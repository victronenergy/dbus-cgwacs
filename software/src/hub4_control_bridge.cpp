#include <cmath>
#include "battery_info.h"
#include "hub4_control_bridge.h"
#include "settings.h"

Hub4ControlBridge::Hub4ControlBridge(BatteryInfo *batteryInfo, Settings *settings, QObject *parent):
	DBusBridge("com.victronenergy.hub4", parent)
{
	Q_ASSERT(settings != 0);
	produce(settings, "acPowerSetPoint", "/AcPowerSetpoint");
	produce(settings, "maxChargePercentage", "/MaxChargePercentage");
	produce(settings, "maxDischargePercentage", "/MaxDischargePercentage");
	produce(settings, "state", "/State");
	produce(settings, "maintenanceInterval", "/Maintenance/Interval");
	produce(settings, "maintenanceDate", "/Maintenance/Date");
	produce(batteryInfo, "maxChargePower", "/MaxChargePower", "W", 0);
	produce(batteryInfo, "maxDischargePower", "/MaxDischargePower", "W", 0);
	produce("/DeviceInstance", 0);
	registerService();
}

bool Hub4ControlBridge::toDBus(const QString &path, QVariant &value)
{
	if (path == "/State") {
		// value = static_cast<int>(value.value<Hub4State>());
	} else if (path == "/Maintenance/Date") {
		QDateTime t = value.value<QDateTime>();
		value = t.isValid() ? t.toTime_t() : 0;
	}
	if (value.type() == QVariant::Double && !qIsFinite(value.toDouble()))
		value = QVariant();
	return true;
}

bool Hub4ControlBridge::fromDBus(const QString &path, QVariant &value)
{
	if (path == "/State") {
		// value = static_cast<Hub4State>(value.toInt());
	} else if (path == "/Maintenance/Date") {
		time_t t = value.value<time_t>();
		value = t == 0 ? QDateTime() : QDateTime::fromTime_t(t);
	}
	return true;
}
