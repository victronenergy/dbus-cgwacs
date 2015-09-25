#ifndef CONTROL_LOOP_H
#define CONTROL_LOOP_H

#include <QDateTime>
#include <QObject>
#include "defines.h"
#include "settings.h"

class AcSensor;
class Multi;
class QTimer;

class Clock
{
public:
	virtual ~Clock() {}

	virtual QDateTime now() const = 0;
};

class DefaultClock : public Clock
{
public:
	virtual QDateTime now() const
	{
		return QDateTime::currentDateTime();
	}
};

/*!
 * Implements the Hub-4 control loop.
 */
class ControlLoop : public QObject
{
	Q_OBJECT
public:
	ControlLoop(Multi *multi, Phase phase, AcSensor *AcSensor,
				Settings *settings, Clock *clock = 0, QObject *parent = 0);

	Phase phase() const;

private slots:
	void onDestroyed();

	void onTimer();

	void onPowerFromMulti();

	void onPowerFromMeter();

private:
	void checkStep();

	void performStep();

	double computeSetpoint() const;

	bool isMultiCharged() const;

	void updateMaintenanceDate();

	void setHub4State(Hub4State state);

	Multi *mMulti;
	AcSensor *mAcSensor;
	Settings *mSettings;
	QTimer *mTimer;
	QScopedPointer<Clock> mClock;
	Phase mPhase;
	bool mMultiUpdate;
	bool mMeterUpdate;
};

#endif // CONTROL_LOOP_H
