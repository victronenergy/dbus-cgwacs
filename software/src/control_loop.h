#ifndef CONTROL_LOOP_H
#define CONTROL_LOOP_H

#include <QObject>
#include "defines.h"

class BatteryInfo;
class Multi;
class MultiPhaseData;
class PowerInfo;
class Settings;

class ControlLoop : public QObject
{
	Q_OBJECT
public:
	Multi *multi() const
	{
		return mMulti;
	}

	Settings *settings() const
	{
		return mSettings;
	}

	BatteryInfo *batteryInfo() const
	{
		return mBatteryInfo;
	}

	void setBatteryInfo(BatteryInfo *i);

protected:
	ControlLoop(Multi *multi, Settings *settings, QObject *parent = 0);

	void adjustSetpoint(PowerInfo *source, Phase targetPhase, MultiPhaseData *target,
						double setpoint);

	bool hasSetpoint(Phase phase) const;

private:
	void checkStep();

	void performStep();

	double computeSetpoint(double pTarget) const;

	Multi *mMulti;
	Settings *mSettings;
	BatteryInfo *mBatteryInfo;
};

#endif // CONTROL_LOOP_H
