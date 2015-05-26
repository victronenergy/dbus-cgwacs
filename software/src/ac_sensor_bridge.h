#ifndef ENERGY_METER_BRIDGE_H
#define ENERGY_METER_BRIDGE_H

#include <QPointer>
#include <QString>
#include "dbus_bridge.h"

class AcSensor;
class AcSensorSettings;
class PowerInfo;
class Settings;

/*!
 * @brief Connects data from `Inverter` to the DBus.
 * This class creates and fills the com.victronenergy.pvinverter_xxx service.
 */
class AcSensorBridge : public DBusBridge
{
	Q_OBJECT
public:
	explicit AcSensorBridge(AcSensor *acSensor,
							AcSensorSettings *emSettings,
							Settings *settings,
							QObject *parent = 0);

protected:
	virtual bool toDBus(const QString &path, QVariant &value);

	virtual bool fromDBus(const QString &path, QVariant &value);

private:
	void producePowerInfo(PowerInfo *pi, const QString &path);

	int getDeviceInstance(const QString &path, const QString &prefix,
						  int instanceBase);

	AcSensor *mAcSensor;
};

#endif // ENERGY_METER_BRIDGE_H
