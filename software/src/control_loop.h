#ifndef CONTROL_LOOP_H
#define CONTROL_LOOP_H

#include <QObject>
#include "defines.h"

class Multi;
class MultiPhaseData;
class PowerInfo;
class Settings;

class ControlLoop : public QObject
{
	Q_OBJECT
protected:
	ControlLoop(Multi *multi, Settings *settings, QObject *parent = 0);

	void setTarget(PowerInfo *source, Phase targetPhase, MultiPhaseData *target, double power);

	bool hasSetpoint(Phase phase) const;

private:
	void checkStep();

	void performStep();

	double computeSetpoint(double pTarget) const;

	Multi *mMulti;
	Settings *mSettings;
};

#endif // CONTROL_LOOP_H
