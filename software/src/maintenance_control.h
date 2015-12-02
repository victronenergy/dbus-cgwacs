#ifndef MAINTENANCECONTROL_H
#define MAINTENANCECONTROL_H

#include <QDateTime>
#include <QObject>
#include <QScopedPointer>
#include "settings.h"

class Multi;

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

class MaintenanceControl : public QObject
{
	Q_OBJECT
public:
	MaintenanceControl(Multi *multi, Settings *settings, Clock *clock = 0,
					   QObject *parent = 0);

	void update();

private slots:
	void onTimer();

private:
	bool isMultiCharged() const;

	void updateMaintenanceDate();

	void setHub4State(Hub4State state);

	static const char *getStateName(int state);

	Multi *mMulti;
	Settings *mSettings;
	QScopedPointer<Clock> mClock;
};

#endif // MAINTENANCECONTROL_H
