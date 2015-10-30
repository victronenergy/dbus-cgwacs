#ifndef AC_SENSOR_BRIDGE_H
#define AC_SENSOR_BRIDGE_H

#include "dbus_bridge.h"

class AcSensor;
class AcSensorSettings;
class PowerInfo;
class Settings;

/*!
 * @brief Connects data from `AcSensor` to the DBus.
 * This class creates and fills the com.victronenergy.xxx.yyy service.
 * Where xxx is grid, pvinverter or shore depending on the `AcSensorSettings`,
 * and yyy is the communication port (eg /dev/ttyUSB0).
 */
class AcSensorBridge : public DBusBridge
{
	Q_OBJECT
public:
	AcSensorBridge(AcSensor *acSensor,
				   AcSensorSettings *emSettings,
				   Settings *settings,
				   bool isSecundary,
				   QObject *parent = 0);

protected:
	virtual bool toDBus(const QString &path, QVariant &value);

	virtual bool fromDBus(const QString &path, QVariant &value);

private:
	void producePowerInfo(PowerInfo *pi, const QString &path);

	AcSensor *mAcSensor;
};

#endif // AC_SENSOR_BRIDGE_H
