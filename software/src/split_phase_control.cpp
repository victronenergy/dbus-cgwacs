#include "ac_sensor.h"
#include "power_info.h"
#include "multi.h"
#include "multi_phase_data.h"
#include "settings.h"
#include "split_phase_control.h"

SplitPhaseControl::SplitPhaseControl(SystemCalc *systemCalc, Multi *multi, AcSensor *acSensor,
									 Settings *settings, QObject *parent):
	MultiPhaseControl(systemCalc, multi, acSensor, settings, parent),
	mSetpoints(3)
{
}

void SplitPhaseControl::performStep()
{
	AcSensor *acSensor = this->acSensor();
	Multi *multi = this->multi();
	Settings *settings = this->settings();

	double gridSetpoint = settings->acPowerSetPoint();
	int phaseCount = multi->getSetpointCount();
	if (phaseCount > 0)
		gridSetpoint /= phaseCount;

	for (int p=0; p<3; ++p) {
		Phase phase = static_cast<Phase>(PhaseL1 + p);
		MultiPhaseData *mpd = multi->getPhaseData(phase);
		PowerInfo *pi = acSensor->getPowerInfo(phase);
		double pNet = pi->power();
		mSetpoints[p] = mpd->acPowerIn() + gridSetpoint - pNet;
	}
	adjustSetpoints(mSetpoints);
}
