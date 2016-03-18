#include "battery.h"
#include "battery_bridge.h"

BatteryBridge::BatteryBridge(const QString &serviceName, Battery *battery, QObject *parent):
	DBusBridge(serviceName, parent)
{
	consume(serviceName, battery, "maxChargeCurrent", "/Info/MaxChargeCurrent");
	consume(serviceName, battery, "maxDischargeCurrent", "/Info/MaxDischargeCurrent");
}
