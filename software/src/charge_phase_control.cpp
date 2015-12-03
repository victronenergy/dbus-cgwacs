#include <QTimer>
#include "ac_sensor.h"
#include "charge_phase_control.h"
#include "defines.h"
#include "multi.h"

ChargePhaseControl::ChargePhaseControl(Multi *multi, AcSensor *acSensor,
									   Settings *settings, Phase phase,
									   QObject *parent):
	ControlLoop(multi, settings, parent),
	mMultiTargetPhase(multi->getPhaseData(phase)),
	mAcSensorPhase(acSensor->getPowerInfo(phase)),
	mPhase(phase)
{
	QTimer *timer = new QTimer(this);
	timer->setInterval(5000);
	timer->start();
	connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
}

void ChargePhaseControl::onTimer()
{
	setTarget(mAcSensorPhase, mPhase, mMultiTargetPhase, MaxMultiPower);
}
