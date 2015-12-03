#ifndef SINGLE_PHASE_CONTROL_H
#define SINGLE_PHASE_CONTROL_H

#include "control_loop.h"
#include "defines.h"
#include "settings.h"

class AcSensor;
class Multi;
class MultiPhaseData;
class PowerInfo;
class QTimer;

/*!
 * Implements the Hub-4 control loop.
 */
class SinglePhaseControl : public ControlLoop
{
	Q_OBJECT
public:
	SinglePhaseControl(Multi *multi, AcSensor *acSensor, Settings *settings,
					   Phase phase, Hub4Mode hub4Mode, QObject *parent = 0);

private slots:
	void onTimer();

	void onPowerFromMulti();

	void onPowerFromMeter();

private:
	void checkStep();

	void performStep();

	double computeSetpoint() const;

	Multi *mMulti;
	Settings *mSettings;
	QTimer *mTimer;
	Phase mPhase;
	MultiPhaseData *mMultiTargetPhase;
	PowerInfo *mAcSensorPhase;
	Hub4Mode mHub4Mode;
	bool mMultiUpdate;
	bool mMeterUpdate;
};

#endif // SINGLE_PHASE_CONTROL_H
