#ifndef MULTIPHASECONTROL_H
#define MULTIPHASECONTROL_H

#include "control_loop.h"

class AcSensor;
class QTimer;

class MultiPhaseControl: public ControlLoop
{
	Q_OBJECT
public:
	AcSensor *acSensor() const
	{
		return mAcSensor;
	}

protected:
	MultiPhaseControl(SystemCalc *systemCalc, Multi *multi, AcSensor *acSensor, Settings *settings,
					  QObject *parent = 0);

	virtual void performStep() = 0;

	void adjustSetpoints(const QVector<double> &setpoints);

private slots:
	void onTimer();

	void onL1FromMulti();

	void onL2FromMulti();

	void onL3FromMulti();

	void onTotalPowerFromMulti();

	void onPowerFromMeter();

private:
	void checkStep();

	AcSensor *mAcSensor;
	QTimer *mTimer;
	bool mMultiL1Update;
	bool mMultiL2Update;
	bool mMultiL3Update;
	bool mMultiTotalPowerUpdate;
	bool mMeterUpdate;
};

#endif // MULTIPHASECONTROL_H
