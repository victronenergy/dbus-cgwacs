#ifndef ENERGY_METER_SETTINGS_BRIDGE_H
#define ENERGY_METER_SETTINGS_BRIDGE_H

#include "dbus_bridge.h"

class AcSensorSettings;

class AcSensorSettingsBridge : public DBusBridge
{
	Q_OBJECT
public:
	AcSensorSettingsBridge(AcSensorSettings *settings,
							  QObject *parent = 0);

protected:
	virtual bool toDBus(const QString &path, QVariant &v);

	virtual bool fromDBus(const QString &path, QVariant &v);
};

#endif // ENERGY_METER_SETTINGS_BRIDGE_H
