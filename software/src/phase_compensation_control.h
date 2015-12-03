#ifndef PHASE_COMPENSATION_CONTROL_H
#define PHASE_COMPENSATION_CONTROL_H

#include "defines.h"
#include "control_loop.h"
#include "settings.h"

class AcSensor;
class AcSensorSettings;
class Multi;
class QTimer;

/*!
 * Implements the multi phase compensation control loop
 *
 * The algorithm will try to make the total power on grid 0, and minimize the
 * power flowing from phase to phase through the multi. This means that there
 * is never a multi charging while another is discharging.
 */
class PhaseCompensationControl : public ControlLoop
{
	Q_OBJECT
public:
	PhaseCompensationControl(Multi *multi, AcSensor *AcSensor,
							 Settings *settings, QObject *parent = 0);

private slots:
	void onTimer();

	void onL1FromMulti();

	void onL2FromMulti();

	void onL3FromMulti();

	void onTotalPowerFromMulti();

	void onPowerFromMeter();

private:
	void checkStep();

	void performStep();

	Multi *mMulti;
	AcSensor *mAcSensor;
	Settings *mSettings;
	QTimer *mTimer;
	bool mMultiL1Update;
	bool mMultiL2Update;
	bool mMultiL3Update;
	bool mMultiTotalPowerUpdate;
	bool mMeterUpdate;
};

#endif // PHASE_COMPENSATION_CONTROL_H
