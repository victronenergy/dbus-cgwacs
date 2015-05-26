#ifndef DBUS_ENERGY_METER_H
#define DBUS_ENERGY_METER_H

#include <QObject>
#include <QList>

class AcSensor;
class AcSensorBridge;
class AcSensorUpdater;
class ControlLoop;
class DbusServiceMonitor;
class Hub4ManagerBridge;
class ModbusRtu;
class Multi;
class Settings;

class DBusCGwacs : public QObject
{
	Q_OBJECT
public:
	DBusCGwacs(const QString &portName, QObject *parent = 0);

signals:
	void connectionLost();

private slots:
	void onDeviceFound();

	void onDeviceSettingsInitialized();

	void onDeviceInitialized();

	void onConnectionLost();

	void onSerialEvent(const char *description);

	void onServicesChanged();

	void onServiceTypeChanged();

	void onControlLoopEnabledChanged();

	void onHub4ModeChanged();

	void onMultiPhaseChanged();

	void onIsSetPointAvailableChanged();

private:
	void updateControlLoop();

	void updateMultiBridge();

	DbusServiceMonitor *mServiceMonitor;
	ModbusRtu *mModbus;
	QList<AcSensor *> mAcSensors;
	Settings *mSettings;
	Multi *mMulti;
	QList<ControlLoop *> mControlLoops;
};

#endif // DBUS_ENERGY_METER_H
