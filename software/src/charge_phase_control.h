#ifndef CHARGE_PHASE_CONTROL_H
#define CHARGE_PHASE_CONTROL_H

#include "control_loop.h"

class AcSensor;
class PowerInfo;
class MultiPhaseData;
class QTimer;

class ChargePhaseControl : public ControlLoop
{
	Q_OBJECT
public:
	ChargePhaseControl(Multi *multi, AcSensor *acSensor, Settings *settings,
					   Phase phase, QObject *parent = 0);

private slots:
	void onTimer();

private:
	QTimer *mTimer;
	MultiPhaseData *mMultiTargetPhase;
	PowerInfo *mAcSensorPhase;
	Phase mPhase;
};

#endif // CHARGE_PHASE_CONTROL_H
