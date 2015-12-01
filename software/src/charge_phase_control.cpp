#include <QtGlobal>
#include <qmath.h>
#include <QsLog.h>
#include <QTimer>
#include "ac_sensor.h"
#include "battery_info.h"
#include "charge_phase_control.h"
#include "defines.h"
#include "multi.h"
#include "multi_phase_data.h"

ChargePhaseControl::ChargePhaseControl(Multi *multi, AcSensor *acSensor, Settings *settings,
									   QObject *parent):
	ControlLoop(multi, settings, parent),
	mAcSensor(acSensor)
{
	QTimer *timer = new QTimer(this);
	timer->setInterval(2500);
	timer->start();
	connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
}

void ChargePhaseControl::onTimer()
{
	int spcount = qMax(1, multi()->getSetpointCount());
	double power = batteryInfo()->maxChargePower() / spcount;
	for (int p=0; p<3; ++p) {
		Phase phase = static_cast<Phase>(PhaseL1 + p);
		PowerInfo *pi = mAcSensor->getPowerInfo(phase);
		MultiPhaseData *mpd = multi()->getPhaseData(phase);
		if (mpd->isSetPointAvailable())
			adjustSetpoint(pi, phase, mpd, power);
	}
}
