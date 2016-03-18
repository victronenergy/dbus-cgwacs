#ifndef BATTERYBRIDGE_H
#define BATTERYBRIDGE_H

#include "dbus_bridge.h"

class Battery;

class BatteryBridge : public DBusBridge
{
	Q_OBJECT
public:
	BatteryBridge(const QString &serviceName, Battery *battery, QObject *parent = 0);
};

#endif // BATTERYBRIDGE_H
