#ifndef HUB4_CONTROL_BRIDGE_H
#define HUB4_CONTROL_BRIDGE_H

#include "dbus_bridge.h"

class BatteryInfo;
class Settings;

class Hub4ControlBridge : public DBusBridge
{
public:
	Hub4ControlBridge(BatteryInfo *batteryInfo, Settings *settings, QObject *parent = 0);

protected:
	virtual bool toDBus(const QString &path, QVariant &value);

	virtual bool fromDBus(const QString &path, QVariant &value);
};

#endif // HUB4_CONTROL_BRIDGE_H
