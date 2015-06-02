#ifndef AC_SENSOR_SIM_H
#define AC_SENSOR_SIM_H

#include <QObject>

class AcSensor;
class Multi;
class QTimer;

class AcSensorSim : public QObject
{
	Q_OBJECT
public:
	AcSensorSim(AcSensor *m, Multi *multi, QObject *parent = 0);

private slots:
	void onTimer();

private:
	AcSensor *mAcSensor;
	Multi *mMulti;
	QTimer *mTimer;
	int mTimerCount;
};

#endif // AC_SENSOR_SIM_H
