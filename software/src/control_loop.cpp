#include <QsLog.h>
#include <QTimer>
#include "control_loop.h"
#include "defines.h"
#include "multi.h"
#include "multi_phase_data.h"
#include "power_info.h"
#include "settings.h"

ControlLoop::ControlLoop(Multi *multi, Settings *settings, QObject *parent):
	QObject(parent),
	mMulti(multi),
	mSettings(settings)
{
	Q_ASSERT(multi != 0);
	Q_ASSERT(settings != 0);
}

void ControlLoop::setTarget(PowerInfo *source, Phase targetPhase, MultiPhaseData *target, double power)
{
	if (!qIsFinite(target->acPowerSetPoint()))
		return;
	double maxChargePct = qMax(0.0, qMin(100.0, mSettings->maxChargePercentage()));
	double maxDischargePct = qMax(0.0, qMin(100.0, mSettings->maxDischargePercentage()));
	// Compute the maximum charge power of the battery. We use it here to limit
	// the setpoint. Obviously, this will only work when a single ContolLoop
	// instance is active. This means a single multi with hub-4 assistant.
	double maxPower = maxChargePct <= 99 ?
			maxChargePct *
			mMulti->maxChargeCurrent() *
			mMulti->dcVoltage() / 100:
			MaxMultiPower;

	// EV: EM340 seems to work better with Alpha = 0.6. Possibly due to lower
	// update rate.
	const double Alpha = 1.0;
	// pMultiNew < 0: battery is discharging
	// pMultiNew > 0: battery is charging
	double pMultiNew = target->acPowerIn() + Alpha * power;
	bool feedbackDisabled = maxDischargePct < 50;
	// It seems that enabling the the ChargeDisabled flag disables both charge
	// and discharge. So we're only setting the flags when we are not
	// discharging.
	bool chargeDisabled = maxChargePct <= 0 && (
		feedbackDisabled || (
			qIsFinite(pMultiNew) &&
			pMultiNew > 30));
	if (qIsFinite(pMultiNew)) {
		// Ugly workaround: the value of pMultiNew must always be sent over the
		// D-Bus, even when it does not change, because the multi will reset its
		// power setpoint if no value has been set during the last 10 seconds.
		pMultiNew = qRound(pMultiNew);
		if (target->acPowerSetPoint() == pMultiNew)
			pMultiNew -= 1;

		// Make sure we do not send illegal values and honor the max power
		// derived from the max charge percentage.
		pMultiNew = qBound(MinMultiPower, pMultiNew, maxPower);
	}

	mMulti->setIsChargeDisabled(chargeDisabled);
	mMulti->setIsFeedbackDisabled(feedbackDisabled);
	// If feedback is disabled and the multi is 'on', it will start inverting
	// when the grid drops ways (emergency power). If we set the multi to
	// 'charge only', it will not start inverting so the system on AC-In and
	// AC-Out will lose power.
	// mMulti->setMode(feedbackDisabled ? MultiChargerOnly : MultiOn);
	QLOG_TRACE() << QString("L%1").arg(targetPhase)
				 << source->power()
				 << '\t' << target->acPowerIn()
				 << '\t' << target->acPowerSetPoint()
				 << '\t' << pMultiNew
				 << '\t' << chargeDisabled
				 << '\t' << feedbackDisabled;
	if (qIsFinite(pMultiNew))
		target->setAcPowerSetPoint(pMultiNew);
}

bool ControlLoop::hasSetpoint(Phase phase) const
{
	return mMulti->getPhaseData(phase)->isSetPointAvailable();
}
