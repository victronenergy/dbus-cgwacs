#ifndef DBUS_CGWACS_H
#define DBUS_CGWACS_H

#include <QObject>
#include <QList>

class AcSensor;
class AcSensorSettings;
class ControlLoop;
class DbusServiceMonitor;
class MaintenanceControl;
class ModbusRtu;
class Multi;
class Settings;
class VBusItem;

/*!
 * Main object which ties everything together.
 * This class will coordinate the search for new AC sensors, and start
 * acquisition when found. The class `AcSensorUpdater` is responsible for
 * communication with the sensors. Sensor data is stored in `AcSensor`.
 *
 * The class will also make sure that the Hub-4 control loop is started when
 * apropriate. The control loop itself is implemented in `ControlLoop`.
 *
 * Finally, this class will detect the presence of a multi within the setup,
 * which we need for the Hub-4 control loop.
 */
class DBusCGwacs : public QObject
{
	Q_OBJECT
public:
	DBusCGwacs(const QString &portName, bool isZigbee, QObject *parent = 0);

signals:
	void connectionLost();

private slots:
	void onDeviceFound();

	void onDeviceSettingsInitialized();

	void onDeviceInitialized();

	void onConnectionLost();

	void onConnectionStateChanged();

	void onSerialEvent(const char *description);

	void onServicesChanged();

	void onServiceTypeChanged();

	void onHub4ModeChanged();

	void onHub4StateChanged();

	void onMultiPhaseChanged();

	void onIsSetPointAvailableChanged();

	void onTimeZoneChanged();

private:
	void updateControlLoop();

	void updateMultiBridge();

	void publishSensor(AcSensor *sensor, AcSensor *pvSensor,
					   AcSensorSettings *sensorSettings);

	DbusServiceMonitor *mServiceMonitor;
	ModbusRtu *mModbus;
	QList<AcSensor *> mAcSensors;
	Settings *mSettings;
	Multi *mMulti;
	MaintenanceControl *mMaintenanceControl;
	QList<ControlLoop *> mControlLoops;
	VBusItem *mTimeZone;
};

#endif // DBUS_CGWACS_H
