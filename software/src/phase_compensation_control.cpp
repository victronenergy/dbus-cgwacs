#include "grid_meter.h"
#include "multi.h"
#include "multi_phase_data.h"
#include "phase_compensation_control.h"
#include "power_info.h"
#include "settings.h"

PhaseCompensationControl::PhaseCompensationControl(SystemCalc *systemCalc, Multi *multi,
												   GridMeter *gridMeter, Settings *settings,
												   QObject *parent):
	MultiPhaseControl(systemCalc, multi, gridMeter, settings, parent),
	mSetpoints(3)
{
}

void PhaseCompensationControl::performStep()
{
	GridMeter *acSensor = this->acSensor();
	Multi *multi = this->multi();
	double pNet = acSensor->getTotalPower() - multi->meanData()->acPowerIn();
	if (!qIsFinite(pNet))
		return;
	int multiPhaseCount = 0;
	double powerOnSetpointPhases = 0;
	for (int p=0; p<3; ++p) {
		Phase phase = static_cast<Phase>(PhaseL1 + p);
		MultiPhaseData *mpd = multi->getPhaseData(phase);
		if (mpd->isSetPointAvailable()) {
			++multiPhaseCount;
			double pNetp = acSensor->getPower(phase) - mpd->acPowerIn();
			if (qIsFinite(pNetp))
				powerOnSetpointPhases += pNetp;
		}
	}
	if (multiPhaseCount == 0)
		return;
	Settings *settings = this->settings();
	pNet -= pvPowerSetpointOffset() + settings->acPowerSetPoint();
	double extraPhasePower = (pNet - powerOnSetpointPhases) / multiPhaseCount;
	double pNetLeft = pNet;
	for (int p=0; p<3; ++p) {
		Phase phase = static_cast<Phase>(PhaseL1 + p);
		MultiPhaseData *mpd = multi->getPhaseData(phase);
		double pNetp = acSensor->getPower(phase) - mpd->acPowerIn() + extraPhasePower;
		if (qIsFinite(pNetp) && qIsFinite(pNet) && mpd->isSetPointAvailable()) {
			double pTarget = 0;
			if (pNet < 0 && pNetp < 0) {
				pTarget = qMax(pNetLeft, pNetp);
				pNetLeft -= pTarget;
			} else if (pNet > 0 && pNetp > 0) {
				pTarget = qMin(pNetLeft, pNetp);
				pNetLeft -= pTarget;
			}
			mSetpoints[p] = -pTarget;
		} else {
			mSetpoints[p] = qQNaN();
		}
	}
	adjustSetpoints(mSetpoints);
}
