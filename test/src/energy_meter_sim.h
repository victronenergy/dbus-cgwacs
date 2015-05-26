#ifndef ENERGY_METER_SIM_H
#define ENERGY_METER_SIM_H

#include <QObject>

class EnergyMeter;
class Multi;
class QTimer;

class EnergyMeterSim : public QObject
{
	Q_OBJECT
public:
	EnergyMeterSim(EnergyMeter *m, Multi *multi, QObject *parent = 0);

private slots:
	void onTimer();

private:
	EnergyMeter *mEnergyMeter;
	Multi *mMulti;
	QTimer *mTimer;
	int mTimerCount;
};

#endif // ENERGY_METER_SIM_H
