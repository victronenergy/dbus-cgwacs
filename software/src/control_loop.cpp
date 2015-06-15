#include <cmath>
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
	MultiPhaseData *mpd = mMulti->getPhaseData(mPhase);
	PowerInfo *pi = mAcSensor->getPowerInfo(mPhase);
	double pNet = pi->power();
	double pMulti = -mpd->acPowerIn();
	if (!std::isfinite(pNet))
		return;
	double pLoad = pNet + pMulti - mSettings->acPowerSetPoint();
	// EV: EM340 seems to work better with Alpha = 0.6. Possibly due to lower
	// update rate.
	const double Alpha = 0.8;
	double pMultiNew = Alpha * pLoad + (1 - Alpha) * pMulti;

	// pMultiNew > 0: battery is discharging
	// pMultiNew < 0: battery is charging
	double maxChargePct = qMax(0.0, qMin(100.0, mSettings->maxChargePercentage()));
	double maxDischargePct = qMax(0.0, qMin(100.0, mSettings->maxDischargePercentage()));
	bool feedbackDisabled = maxDischargePct < 50;
	bool chargeDisabled = maxChargePct <= 0;
	double minPower =
			-maxChargePct *
			mMulti->maxChargeCurrent() *
			mMulti->dcVoltage() / 100;
	pMultiNew = qMax(minPower, pMultiNew);

	// Ugly workaround: the value of pMultiNew must always be sent over the
	// D-Bus, even when it does not change, because the multi will reset its
	// power setpoint if no value has been set during the last 10 seconds.
	// So we add a random value to ensure pMultiNew changes. Other solution
	// would involve changes in velib (VBusItem and friends).
	pMultiNew += qrand() / (100.0 * RAND_MAX);
	QLOG_TRACE() << pNet << '\t' << pLoad << '\t' << pMulti << '\t'
				 << pMultiNew << '\t' << chargeDisabled
				 << '\t' << feedbackDisabled;

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
