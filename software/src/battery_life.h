#ifndef MAINTENANCE_CONTROL_H
#define MAINTENANCE_CONTROL_H

#include <QDateTime>
#include <QObject>
#include <QScopedPointer>
#include "settings.h"

class DbusServiceMonitor;
class Multi;
class QTimer;
class VBusItem;

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

class BatteryLife : public QObject
{
	Q_OBJECT
public:
	BatteryLife(DbusServiceMonitor *serviceMonitor, Multi *multi, Settings *settings,
				Clock *clock = 0, QObject *parent = 0);

public slots:
	void onChargeTimer();

private slots:
	void update();

	void onServiceAdded(QString service);

private:
	void onDischarged(bool adjustLimit);

	void onAbsorption(bool adjustSocLimit);

	void onFloat(bool adjustLimit);

	void adjustSocLimit(double delta);

	void setState(BatteryLifeState state);

	static const char *getStateName(BatteryLifeState state);

	Multi *mMulti;
	Settings *mSettings;
	VBusItem *mSystemSoc;
	QScopedPointer<Clock> mClock;
	bool mUpdateBusy;
};

#endif // MAINTENANCE_CONTROL_H
