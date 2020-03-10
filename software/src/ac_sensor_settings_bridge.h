#ifndef AC_SENSOR_SETTINGS_BRIDGE_H
#define AC_SENSOR_SETTINGS_BRIDGE_H

#include "dbus_bridge.h"

class AcSensorSettings;

/*!
 * Bridge between the settings of a single AC sensor and the D-Bus.
 *
 * D-Bus path: com.victronenergy.settings/Settings/CGwacs/Devices/D<serial>
 * The serial is prefixed with a 'D', because the serial may start with a
 * digit, which is not allowed by the localsettings as start of a settings
 * names (causes problems when storing settings data as xml file).
 */
class AcSensorSettingsBridge : public DBusBridge
{
	Q_OBJECT
public:
	AcSensorSettingsBridge(AcSensorSettings *settings,
							  QObject *parent = 0);
};

#endif // AC_SENSOR_SETTINGS_BRIDGE_H
