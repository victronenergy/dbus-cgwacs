#ifndef CONTROL_LOOP_H
#define CONTROL_LOOP_H

#include <QObject>
#include "defines.h"

class AcSensor;
class Multi;
class QTimer;
class Settings;

/*!
 * Implements the Hub-4 control loop.
 */
class ControlLoop : public QObject
{
	Q_OBJECT
public:
	ControlLoop(Multi *multi, Phase phase, AcSensor *AcSensor,
				Settings *settings, QObject *parent = 0);

	Phase phase() const;

private slots:
	void onDestroyed();

	void onTimer();

	void onPowerFromMulti();

	void onPowerFromMeter();

private:
	void checkStep();

	void performStep();

	Multi *mMulti;
	AcSensor *mAcSensor;
	Settings *mSettings;
	QTimer *mTimer;
	Phase mPhase;
	bool mMultiUpdate;
	bool mMeterUpdate;
};

#endif // CONTROL_LOOP_H
