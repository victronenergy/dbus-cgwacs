#ifndef BATTERYINFO_H
#define BATTERYINFO_H

#include <QHash>
#include <QObject>

class Battery;
class DbusServiceMonitor;
class Multi;
class Settings;

class BatteryInfo : public QObject
{
	Q_OBJECT
	Q_PROPERTY(double maxChargePower READ maxChargePower NOTIFY maxChargePowerChanged)
	Q_PROPERTY(double maxDischargePower READ maxDischargePower NOTIFY maxDischargePowerChanged)
public:
	BatteryInfo(DbusServiceMonitor *serviceManager, Multi *multi, Settings *settings,
				QObject *parent = 0);

	double maxChargePower() const
	{
		return mMaxChargePower;
	}

	double maxDischargePower() const
	{
		return mMaxDischargePower;
	}

	double applyLimits(double p) const;

signals:
	void maxChargePowerChanged();

	void maxDischargePowerChanged();

private slots:
	void onServiceAdded(QString service);

	void onServiceRemoved(QString service);

	void updateBatteryLimits();

private:
	void setMaxChargePower(double p);

	void setMaxDischargePower(double p);

	QHash<QString, Battery *> mBatteries;
	Multi *mMulti;
	Settings *mSettings;
	double mMaxChargePower;
	double mMaxDischargePower;
};

#endif // BATTERYINFO_H
