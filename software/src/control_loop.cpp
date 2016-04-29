#include <QsLog.h>
#include <QTimer>
#include "battery_info.h"
#include "control_loop.h"
#include "defines.h"
#include "multi.h"
#include "multi_phase_data.h"
#include "power_info.h"
#include "settings.h"

ControlLoop::ControlLoop(Multi *multi, Settings *settings, QObject *parent):
	QObject(parent),
	mMulti(multi),
	mSettings(settings),
	mBatteryInfo(0)
{
	Q_ASSERT(multi != 0);
	Q_ASSERT(settings != 0);
}

void ControlLoop::setBatteryInfo(BatteryInfo *c)
{
	mBatteryInfo = c;
}

void ControlLoop::adjustSetpoint(PowerInfo *source, Phase targetPhase, MultiPhaseData *target,
								 double setpoint)
{
	if (!qIsFinite(target->acPowerSetPoint()))
		return;
	if (mMulti->firmwareVersion() < 0x405) {
		// Older vebus firmware versions had a faster control loop on the AC-In power setpoint.
		// In order to prevent overshoot we reduce the difference between the new setpoint and the
		// current AC-In power.
		// Old code was: setpoint = 0.8 * change + acPowerIn. with change = setpoint - acPowerIn
		setpoint = 0.2 * target->acPowerIn() + 0.8 * setpoint;
	}

	bool feedbackDisabled = !mBatteryInfo->canDischarge();

	switch (mSettings->state()) {
	case BatteryLifeStateForceCharge:
	{
		double dcCurrent = mMulti->dcCurrent();
		double dcVoltage = mMulti->dcVoltage();
		if (qIsFinite(dcCurrent) && qIsFinite(dcCurrent)) {
			double dcPower = dcCurrent * dcVoltage;
			double acPower = 0;
			double acPhaseOut = 0;
			for (int i=0; i<3; ++i) {
				Phase phase = static_cast<Phase>(PhaseL1 + i);
				MultiPhaseData *mpd = mMulti->getPhaseData(phase);
				double acPowerIn = mpd->acPowerIn();
				if (qIsFinite(acPowerIn))
					acPower += acPowerIn;
				double acPowerOut = mpd->acPowerOut();
				if (qIsFinite(acPowerOut)) {
					acPhaseOut = acPowerOut;
					acPower -= acPowerOut;
				}
			}
			double minAcPower = dcVoltage * ForceChargeCurrent;
			if (dcPower > 0.1 && acPower > 0)
				minAcPower *= qBound(1.0, acPower / dcPower, 2.0);
			minAcPower += acPhaseOut;
			setpoint = qMax(minAcPower, setpoint);
			setpoint = mBatteryInfo->applyLimits(setpoint);
		}
		// Fall through
	}
	case BatteryLifeStateDischarged:
		feedbackDisabled = true;
		break;
	default:
		break;
	}

	// It seems that enabling the the ChargeDisabled flag disables both charge and discharge. So
	// we're only setting the flags when we are not discharging.
	bool chargeDisabled = !mBatteryInfo->canCharge() && (
		feedbackDisabled || (
			qIsFinite(setpoint) &&
			setpoint > 30));
	if (qIsFinite(setpoint)) {
		setpoint = qBound(MinMultiPower, setpoint, MaxMultiPower);

		// Ugly workaround: the value of pMultiNew must always be sent over the
		// D-Bus, even when it does not change, because the multi will reset its
		// power setpoint if no value has been set during the last 10 seconds.
		setpoint = qRound(setpoint);
		if (qRound(target->acPowerSetPoint()) == setpoint)
			setpoint -= 1;
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
				 << '\t' << setpoint
				 << '\t' << chargeDisabled
				 << '\t' << feedbackDisabled;
	if (qIsFinite(setpoint))
		target->setAcPowerSetPoint(setpoint);
}

bool ControlLoop::hasSetpoint(Phase phase) const
{
	return mMulti->getPhaseData(phase)->isSetPointAvailable();
}
