#include "ac_sensor.h"
#include "multi.h"
#include "multi_phase_data.h"
#include "phase_compensation_control.h"
#include "power_info.h"
#include "settings.h"

PhaseCompensationControl::PhaseCompensationControl(Multi *multi, AcSensor *acSensor,
												   Settings *settings, QObject *parent):
	MultiPhaseControl (multi, acSensor, settings, parent)
{
}

void PhaseCompensationControl::performStep()
{
	AcSensor *acSensor = this->acSensor();
	Multi *multi = this->multi();
	PowerInfo *pi = acSensor->getPowerInfo(MultiPhase);
	double pNet = pi->power() - multi->meanData()->acPowerIn();
	if (!qIsFinite(pNet))
		return;
	int multiPhaseCount = 0;
	double powerOnSetpointPhases = 0;
	for (int p=0; p<3; ++p) {
		Phase phase = static_cast<Phase>(PhaseL1 + p);
		MultiPhaseData *mpd = multi->getPhaseData(phase);
		if (mpd->isSetPointAvailable()) {
			++multiPhaseCount;
			double pNetp = acSensor->getPowerInfo(phase)->power() - mpd->acPowerIn();
			if (qIsFinite(pNetp))
				powerOnSetpointPhases += pNetp;
		}
	}
	if (multiPhaseCount == 0)
		return;
	Settings *settings = this->settings();
	pNet -= settings->acPowerSetPoint();
	double extraPhasePower = (pNet - powerOnSetpointPhases) / multiPhaseCount;
	double pNetLeft = pNet;
	double setpoints[3];
	for (int p=0; p<3; ++p) {
		Phase phase = static_cast<Phase>(PhaseL1 + p);
		PowerInfo *pi = acSensor->getPowerInfo(phase);
		MultiPhaseData *mpd = multi->getPhaseData(phase);
		double pNetp = pi->power() - mpd->acPowerIn() + extraPhasePower;
		if (qIsFinite(pNetp) && qIsFinite(pNet) && mpd->isSetPointAvailable()) {
			double pTarget = 0;
			if (pNet < 0 && pNetp < 0) {
				pTarget = qMax(pNetLeft, pNetp);
				pNetLeft -= pTarget;
			} else if (pNet > 0 && pNetp > 0) {
				pTarget = qMin(pNetLeft, pNetp);
				pNetLeft -= pTarget;
			}
			setpoints[p] = -pTarget;
		}
	}
	adjustSetpoints(setpoints);
}
