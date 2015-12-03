#include <QsLog.h>
#include <QTimer>
#include "ac_sensor.h"
#include "defines.h"
#include "multi.h"
#include "multi_phase_data.h"
#include "phase_compensation_control.h"
#include "power_info.h"
#include "settings.h"

PhaseCompensationControl::PhaseCompensationControl(Multi *multi, AcSensor *acSensor,
												   Settings *settings, QObject *parent):
	ControlLoop(multi, settings, parent),
	mMulti(multi),
	mAcSensor(acSensor),
	mSettings(settings),
	mTimer(new QTimer(this)),
	mMultiL1Update(false),
	mMultiL2Update(false),
	mMultiL3Update(false),
	mMultiTotalPowerUpdate(false),
	mMeterUpdate(false)
{
	Q_ASSERT(multi != 0);
	Q_ASSERT(acSensor != 0);
	Q_ASSERT(settings != 0);
	mTimer->setInterval(6000);
	mTimer->start();
	connect(mMulti, SIGNAL(destroyed()), this, SLOT(deleteLater()));
	connect(mMulti->getPhaseData(PhaseL1), SIGNAL(acPowerInChanged()),
			this, SLOT(onL1FromMulti()));
	connect(mMulti->getPhaseData(PhaseL2), SIGNAL(acPowerInChanged()),
			this, SLOT(onL2FromMulti()));
	connect(mMulti->getPhaseData(PhaseL3), SIGNAL(acPowerInChanged()),
			this, SLOT(onL3FromMulti()));
	connect(mMulti->getPhaseData(MultiPhase), SIGNAL(acPowerInChanged()),
			this, SLOT(onTotalPowerFromMulti()));
	connect(mAcSensor, SIGNAL(destroyed()), this, SLOT(deleteLater()));
	/// @todo EV Total power is changed after phase specific data. So if
	/// total power changes, L1, L2, and L3 have fresh values as well.
	connect(mAcSensor->getPowerInfo(MultiPhase), SIGNAL(powerChanged()),
			this, SLOT(onPowerFromMeter()));
	connect(mTimer, SIGNAL(timeout()), this, SLOT(onTimer()));
}

void PhaseCompensationControl::onTimer()
{
	QLOG_DEBUG() << "Update timeout";
	mMultiL1Update = true;
	mMultiL2Update = true;
	mMultiL3Update = true;
	mMeterUpdate = true;
	checkStep();
}

void PhaseCompensationControl::onL1FromMulti()
{
	mMultiL1Update = true;
	checkStep();
}

void PhaseCompensationControl::onL2FromMulti()
{
	mMultiL2Update = true;
	checkStep();
}

void PhaseCompensationControl::onL3FromMulti()
{
	mMultiL3Update = true;
	checkStep();
}

void PhaseCompensationControl::onTotalPowerFromMulti()
{
	mMultiTotalPowerUpdate = true;
	checkStep();
}

void PhaseCompensationControl::onPowerFromMeter()
{
	mMeterUpdate = true;
	checkStep();
}

void PhaseCompensationControl::checkStep()
{
	if (!mMeterUpdate)
		return;
	if (!mMultiTotalPowerUpdate)
		return;
	if (!mMultiL1Update && hasSetpoint(PhaseL1))
		return;
	if (!mMultiL2Update && hasSetpoint(PhaseL2))
		return;
	if (!mMultiL3Update && hasSetpoint(PhaseL3))
		return;
	performStep();
	mMeterUpdate = false;
	mMultiL1Update = false;
	mMultiL2Update = false;
	mMultiL3Update = false;
	mMultiTotalPowerUpdate = false;
	mTimer->start();
}

void PhaseCompensationControl::performStep()
{
	PowerInfo *pi = mAcSensor->getPowerInfo(MultiPhase);
	double pNet = pi->power() - mMulti->meanData()->acPowerIn();
	if (!qIsFinite(pNet))
		return;
	int multiPhaseCount = 0;
	double powerOnSetpointPhases = 0;
	for (int p=0; p<3; ++p) {
		Phase phase = static_cast<Phase>(PhaseL1 + p);
		MultiPhaseData *mpd = mMulti->getPhaseData(phase);
		if (mpd->isSetPointAvailable()) {
			++multiPhaseCount;
			double pNetp = mAcSensor->getPowerInfo(phase)->power() - mpd->acPowerIn();
			if (qIsFinite(pNetp))
				powerOnSetpointPhases += pNetp;
		}
	}
	if (multiPhaseCount == 0)
		return;
	pNet -= mSettings->acPowerSetPoint();
	double extraPhasePower = (pNet - powerOnSetpointPhases) / multiPhaseCount;
	double pNetLeft = pNet;
	for (int p=0; p<3; ++p) {
		Phase phase = static_cast<Phase>(PhaseL1 + p);
		PowerInfo *pi = mAcSensor->getPowerInfo(phase);
		MultiPhaseData *mpd = mMulti->getPhaseData(phase);
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
			setTarget(pi, phase, mMulti->getPhaseData(phase), - mpd->acPowerIn() - pTarget);
		}
	}
}
