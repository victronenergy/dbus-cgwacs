#include <cmath>
#include <QDateTime>
#include <QsLog.h>
#include <QTimer>
#include "ac_sensor.h"
#include "control_loop.h"
#include "multi.h"
#include "multi_phase_data.h"
#include "power_info.h"
#include "settings.h"

ControlLoop::ControlLoop(Multi *multi, Phase phase, AcSensor *AcSensor,
						 Settings *settings, Clock *clock, QObject *parent):
	QObject(parent),
	mMulti(multi),
	mAcSensor(AcSensor),
	mSettings(settings),
	mTimer(new QTimer(this)),
	mClock(clock),
	mPhase(phase),
	mMultiUpdate(false),
	mMeterUpdate(false)
{
	Q_ASSERT(multi != 0);
	Q_ASSERT(multi->getPhaseData(phase) != 0);
	Q_ASSERT(AcSensor != 0);
	Q_ASSERT(settings != 0);
	if (mClock == 0)
		mClock.reset(new DefaultClock());
	mTimer->setInterval(5000);
	mTimer->start();
	connect(mMulti, SIGNAL(destroyed()), this, SLOT(onDestroyed()));
	connect(mMulti->getPhaseData(mPhase), SIGNAL(acPowerOutChanged()),
			this, SLOT(onPowerFromMulti()));
	connect(mAcSensor, SIGNAL(destroyed()), this, SLOT(onDestroyed()));
	connect(mAcSensor->getPowerInfo(mPhase), SIGNAL(powerChanged()),
			this, SLOT(onPowerFromMeter()));
	connect(mTimer, SIGNAL(timeout()), this, SLOT(onTimer()));
}

Phase ControlLoop::phase() const
{
	return mPhase;
}

void ControlLoop::onDestroyed()
{
	deleteLater();
}

void ControlLoop::onTimer()
{
	QLOG_DEBUG() << "Update timeout";
	mMultiUpdate = true;
	mMeterUpdate = true;
	checkStep();
}

void ControlLoop::onPowerFromMulti()
{
	mMultiUpdate = true;
	checkStep();
}

void ControlLoop::onPowerFromMeter()
{
	mMeterUpdate = true;
	checkStep();
}

void ControlLoop::checkStep()
{
	if (!mMeterUpdate || !mMultiUpdate)
		return;
	performStep();
	mMeterUpdate = false;
	mMultiUpdate = false;
	mTimer->start();
}

void ControlLoop::performStep()
{
	// pMultiNew > 0: battery is discharging
	// pMultiNew < 0: battery is charging
	double pMultiNew = 0;
	double maxChargePct = qMax(0.0, qMin(100.0, mSettings->maxChargePercentage()));
	double maxDischargePct = qMax(0.0, qMin(100.0, mSettings->maxDischargePercentage()));
	double minPower =
			-maxChargePct *
			mMulti->maxChargeCurrent() *
			mMulti->dcVoltage() / 100;

	if (mSettings->maintenanceInterval() == 0) {
		setHub4State(Hub4External);
	} else if (mSettings->state() == Hub4External) {
		setHub4State(Hub4SelfConsumption);
	}

	switch (mSettings->state()) {
	case Hub4SelfConsumption:
	{
		pMultiNew = computeSetpoint();
		QDateTime nextCharge = mSettings->maintenanceDate();
		if (!nextCharge.isValid()) {
			updateMaintenanceDate();
			nextCharge = mSettings->maintenanceDate();
		}
		nextCharge = nextCharge.addDays(mSettings->maintenanceInterval());
		QDateTime now = mClock->now();
		if (now >= nextCharge && now.time().hour() == 12) {
			setHub4State(Hub4ChargeFromGrid);
		}

//		QLOG_WARN() << __FUNCTION__ << "Days to next charge:"
//					<< now.secsTo(nextCharge)/(24 * 3600.0);

		if (isMultiCharged()) {
			setHub4State(Hub4Charged);
		}
		break;
	}
	case Hub4Charged:
		pMultiNew = computeSetpoint();
		if (!isMultiCharged()) {
			setHub4State(Hub4SelfConsumption);
			updateMaintenanceDate();
		}
		break;
	case Hub4External:
		pMultiNew = computeSetpoint();
		break;
	case Hub4ChargeFromGrid:
		pMultiNew = minPower;
		if (isMultiCharged()) {
			setHub4State(Hub4SelfConsumption);
			updateMaintenanceDate();
		}
		break;
	case Hub4Storage:
		pMultiNew = minPower;
		if (isMultiCharged())
			mSettings->setMaintenanceDate(QDateTime());
		break;
	}

	bool feedbackDisabled = maxDischargePct < 50;
	bool chargeDisabled = maxChargePct <= 0;
	pMultiNew = qMax(minPower, pMultiNew);

	// Ugly workaround: the value of pMultiNew must always be sent over the
	// D-Bus, even when it does not change, because the multi will reset its
	// power setpoint if no value has been set during the last 10 seconds.
	// So we add a random value to ensure pMultiNew changes. Other solution
	// would involve changes in velib (VBusItem and friends).
	pMultiNew += qrand() / (100.0 * RAND_MAX);

	mMulti->setIsChargeDisabled(chargeDisabled);
	mMulti->setIsFeedbackDisabled(feedbackDisabled);
	// If feedback is disabled and the multi is 'on', it will start inverting
	// when the grid drops ways (emergency power). If we set the multi to
	// 'charge only', it will not start inverting so the system on AC-In and
	// AC-Out will lose power.
	// mMulti->setMode(feedbackDisabled ? MultiChargerOnly : MultiOn);
	if (std::isfinite(pMultiNew))
		mMulti->setAcPowerSetPoint(-pMultiNew);
}

double ControlLoop::computeSetpoint() const
{
	PowerInfo *pi = mAcSensor->getPowerInfo(mPhase);
	double pNet = pi->power();
	MultiPhaseData *mpd = mMulti->getPhaseData(mPhase);
	double pMulti = -mpd->acPowerIn();
	double pLoad = pNet + pMulti - mSettings->acPowerSetPoint();
	if (!std::isfinite(pNet))
		return NaN;
	// EV: EM340 seems to work better with Alpha = 0.6. Possibly due to lower
	// update rate.
	const double Alpha = 0.8;
	double pMultiNew = Alpha * pLoad + (1 - Alpha) * pMulti;
	QLOG_TRACE() << pNet << '\t' << pLoad << '\t' << pMulti << '\t' << pMultiNew;
	return pMultiNew;
}

bool ControlLoop::isMultiCharged() const
{
	return (mMulti->state() == MultiStateFloat || mMulti->state() == MultiStateStorage) &&
			mMulti->stateOfCharge() == 100;
}

void ControlLoop::updateMaintenanceDate()
{
	QDateTime nextCharge = mClock->now();
	nextCharge.setTime(QTime(1, 0, 0));
	mSettings->setMaintenanceDate(nextCharge);
	QLOG_INFO() << "Next maintenance date set at:" << nextCharge;
}

void ControlLoop::setHub4State(Hub4State state)
{
	if (mSettings->state() == state)
		return;
	static const char *StateNames[] = { "SelfConsumption", "ChargeFromGrid", "Charged", "Storage", "External" };
	QLOG_INFO() << "Changing Hub4 state from" << StateNames[mSettings->state()]
				<< "to" << StateNames[state];
	mSettings->setState(state);
}
