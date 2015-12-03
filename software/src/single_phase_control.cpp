#include <QsLog.h>
#include <QTimer>
#include "ac_sensor.h"
#include "defines.h"
#include "multi.h"
#include "multi_phase_data.h"
#include "power_info.h"
#include "settings.h"
#include "single_phase_control.h"

SinglePhaseControl::SinglePhaseControl(Multi *multi, AcSensor *acSensor,
									   Settings *settings, Phase phase,
									   Hub4Mode hub4Mode, QObject *parent):
	ControlLoop(multi, settings, parent),
	mMulti(multi),
	mSettings(settings),
	mTimer(new QTimer(this)),
	mPhase(phase),
	mMultiTargetPhase(multi->getPhaseData(phase)),
	mAcSensorPhase(acSensor->getPowerInfo(hub4Mode == Hub4PhaseCompensation ? MultiPhase : phase)),
	mHub4Mode(hub4Mode),
	mMultiUpdate(false),
	mMeterUpdate(false)
{
	Q_ASSERT(multi != 0);
	Q_ASSERT(multi->getPhaseData(phase) != 0);
	Q_ASSERT(acSensor != 0);
	Q_ASSERT(settings != 0);
	mTimer->setInterval(5000);
	mTimer->start();
	connect(mMulti, SIGNAL(destroyed()), this, SLOT(deleteLater()));
	connect(mMultiTargetPhase, SIGNAL(acPowerInChanged()),
			this, SLOT(onPowerFromMulti()));
	connect(acSensor, SIGNAL(destroyed()), this, SLOT(deleteLater()));
	connect(mAcSensorPhase, SIGNAL(powerChanged()),
			this, SLOT(onPowerFromMeter()));
	connect(mTimer, SIGNAL(timeout()), this, SLOT(onTimer()));
}

void SinglePhaseControl::onTimer()
{
	QLOG_DEBUG() << "Update timeout";
	mMultiUpdate = true;
	mMeterUpdate = true;
	checkStep();
}

void SinglePhaseControl::onPowerFromMulti()
{
	mMultiUpdate = true;
	checkStep();
}

void SinglePhaseControl::onPowerFromMeter()
{
	mMeterUpdate = true;
	checkStep();
}

void SinglePhaseControl::checkStep()
{
	if (!mMeterUpdate || !mMultiUpdate)
		return;
	performStep();
	mMeterUpdate = false;
	mMultiUpdate = false;
	mTimer->start();
}

void SinglePhaseControl::performStep()
{
	double pNet = mAcSensorPhase->power();
	double gridSetpoint = mSettings->acPowerSetPoint();
	if (mHub4Mode == Hub4PhaseSplit && gridSetpoint != 0) {
		int phaseCount = mMulti->getSetpointCount();
		if (phaseCount > 0)
			gridSetpoint /= phaseCount;
	}
	double pTarget = gridSetpoint - pNet;
	setTarget(mAcSensorPhase, mPhase, mMultiTargetPhase, pTarget);
}
