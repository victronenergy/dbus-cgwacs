#include <QStringList>
#include <QsLog.h>
#include <QVariant>
#include "settings.h"
#include "settings_bridge.h"

static const QString Service = "com.victronenergy.settings";
static const QString DeviceIdsPath = "/Settings/CGwacs/DeviceIds";
static const QString AcPowerSetPointPath = "/Settings/CGwacs/AcPowerSetPoint";
static const QString MaxChargePercentagePath = "/Settings/CGwacs/MaxChargePercentage";
static const QString MaxDischargePercentagePath = "/Settings/CGwacs/MaxDischargePercentage";
static const QString StatePath = "/Settings/CGwacs/State";
static const QString MaintenanceIntervalPath = "/Settings/CGwacs/Maintenance/Interval";
static const QString MaintenanceDatePath = "/Settings/CGwacs/Maintenance/Date";

SettingsBridge::SettingsBridge(Settings *settings, QObject *parent):
	DBusBridge(parent)
{
	consume(Service, settings, "deviceIds", QVariant(""), DeviceIdsPath);
	consume(Service, settings, "acPowerSetPoint", 0.0, -1e5, 1e5, AcPowerSetPointPath);
	consume(Service, settings, "maxChargePercentage", 100, 0, 100, MaxChargePercentagePath);
	consume(Service, settings, "maxDischargePercentage", 100, 0, 100, MaxDischargePercentagePath);
	consume(Service, settings, "state", 0, StatePath);
	consume(Service, settings, "maintenanceInterval", 7, MaintenanceIntervalPath);
	consume(Service, settings, "maintenanceDate", 0, MaintenanceDatePath);
}

bool SettingsBridge::toDBus(const QString &path, QVariant &value)
{
	if (path == DeviceIdsPath) {
		value = value.value<QStringList>().join(",");
	} else if (path == StatePath) {
		// value = static_cast<int>(value.value<Hub4State>());
	} else if (path == MaintenanceDatePath) {
		QDateTime t = value.value<QDateTime>();
		value = t.isValid() ? t.toTime_t() : 0;
	}
	return true;
}

bool SettingsBridge::fromDBus(const QString &path, QVariant &value)
{
	if (path == DeviceIdsPath) {
		value = value.toString().split(',', QString::SkipEmptyParts);
	} else if (path == StatePath) {
		// value = static_cast<Hub4State>(value.toInt());
	} else if (path == MaintenanceDatePath) {
		time_t t = value.value<time_t>();
		value = t == 0 ? QDateTime() : QDateTime::fromTime_t(t);
	}
	return true;
}
