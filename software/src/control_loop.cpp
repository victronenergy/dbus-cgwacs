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
						 Settings *settings, QObject *parent):
	QObject(parent),
	mMulti(multi),
	mAcSensor(AcSensor),
	mSettings(settings),
	mTimer(new QTimer(this)),
	mPhase(phase),
	mMultiUpdate(false),
	mMeterUpdate(false)
{
	Q_ASSERT(multi != 0);
	Q_ASSERT(multi->getPhaseData(phase) != 0);
	Q_ASSERT(AcSensor != 0);
	Q_ASSERT(settings != 0);
	mTimer->setInterval(5000);
	mTimer->start();
	connect(mMulti, SIGNAL(destroyed()), this, SLOT(onDestroyed()));
	connect(mMulti->getPhaseData(mPhase), SIGNAL(acPowerInChanged()),
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
	// pMultiNew < 0: battery is discharging
	// pMultiNew > 0: battery is charging
	const double MaxMultiPower = 32700;
	const double MinMultiPower = -MaxMultiPower;
	double pMultiNew = 0;
	double maxChargePct = qMax(0.0, qMin(100.0, mSettings->maxChargePercentage()));
	double maxDischargePct = qMax(0.0, qMin(100.0, mSettings->maxDischargePercentage()));
	double maxPower = maxChargePct <= 99 ?
			maxChargePct *
			mMulti->maxChargeCurrent() *
			mMulti->dcVoltage() / 100:
			MaxMultiPower;

	switch (mSettings->state()) {
	case Hub4SelfConsumption:
	case Hub4Charged:
	case Hub4External:
		pMultiNew = computeSetpoint();
		break;
	case Hub4ChargeFromGrid:
	case Hub4Storage:
		pMultiNew = maxPower;
		break;
	}

	bool feedbackDisabled = maxDischargePct < 50;
	// It seems that enabling the the ChargeDisabled flag disables both charge
	// and discharge. So we're only setting the flags when we are not
	// discharging.
	bool chargeDisabled = maxChargePct <= 0 && (
		feedbackDisabled || (
			std::isfinite(pMultiNew) &&
			pMultiNew > 30));
	if (std::isfinite(pMultiNew))
		pMultiNew = qBound(MinMultiPower, pMultiNew, maxPower);

	// Ugly workaround: the value of pMultiNew must always be sent over the
	// D-Bus, even when it does not change, because the multi will reset its
	// power setpoint if no value has been set during the last 10 seconds.
	// So we add a random value to ensure pMultiNew changes. Other solution
	// would involve changes in velib (VBusItem and friends).
	pMultiNew -= qrand() / (10.0 * RAND_MAX);

	MultiPhaseData *mpd = mMulti->getPhaseData(mPhase);
	mMulti->setIsChargeDisabled(chargeDisabled);
	mMulti->setIsFeedbackDisabled(feedbackDisabled);
	// If feedback is disabled and the multi is 'on', it will start inverting
	// when the grid drops ways (emergency power). If we set the multi to
	// 'charge only', it will not start inverting so the system on AC-In and
	// AC-Out will lose power.
	// mMulti->setMode(feedbackDisabled ? MultiChargerOnly : MultiOn);
	QLOG_TRACE() << mAcSensor->getPowerInfo(mPhase)->power()
				 << '\t' << mpd->acPowerIn()
				 << '\t' << pMultiNew
				 << '\t' << chargeDisabled
				 << '\t' << feedbackDisabled;
	if (std::isfinite(pMultiNew) && std::isfinite(mpd->acPowerSetPoint()))
		mpd->setAcPowerSetPoint(pMultiNew);
}

double ControlLoop::computeSetpoint() const
{
	PowerInfo *pi = mAcSensor->getPowerInfo(mPhase);
	double pNet = pi->power();
	if (!std::isfinite(pNet))
		return NaN;
	MultiPhaseData *mpd = mMulti->getPhaseData(mPhase);
	double pMulti = mpd->acPowerIn();
	double pTarget = mSettings->acPowerSetPoint();
	// EV: EM340 seems to work better with Alpha = 0.6. Possibly due to lower
	// update rate.
	const double Alpha = 0.8;
	return pMulti + Alpha * (pTarget - pNet);
}
