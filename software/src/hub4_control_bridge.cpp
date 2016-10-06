#include <cmath>
#include "battery_info.h"
#include "hub4_control_bridge.h"
#include "settings.h"

Hub4ControlBridge::Hub4ControlBridge(BatteryInfo *batteryInfo, Settings *settings, QObject *parent):
	DBusBridge("pub/com.victronenergy.hub4", true, parent)
{
	Q_ASSERT(settings != 0);
	produce(settings, "acPowerSetPoint", "/AcPowerSetpoint");
	produce(settings, "maxChargePercentage", "/MaxChargePercentage");
	produce(settings, "maxDischargePercentage", "/MaxDischargePercentage");
	produce(settings, "state", "/State");
	produce(batteryInfo, "maxChargePower", "/MaxChargePower", "W", 0);
	produce(batteryInfo, "maxDischargePower", "/MaxDischargePower", "W", 0);
	produce("/DeviceInstance", 0);
	registerService();
}

bool Hub4ControlBridge::toDBus(const QString &path, QVariant &value)
{
	if (path == "/State") {
		value = static_cast<int>(value.value<BatteryLifeState>());
	}
	if (value.type() == QVariant::Double && !qIsFinite(value.toDouble()))
		value = QVariant();
	return true;
}

bool Hub4ControlBridge::fromDBus(const QString &path, QVariant &value)
{
	if (path == "/State") {
		value = static_cast<BatteryLifeState>(value.toInt());
	} else if (path == "/DeviceInstance") {
		return false;
	}
	return true;
}
