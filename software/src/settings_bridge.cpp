#include <QStringList>
#include <QVariant>
#include "settings.h"
#include "settings_bridge.h"

static const QString Service = "com.victronenergy.settings";
static const QString DeviceIdsPath = "/Settings/CGwacs/DeviceIds";
static const QString AcPowerSetPointPath = "/Settings/CGwacs/AcPowerSetPoint";
static const QString MaxChargePercentagePath = "/Settings/CGwacs/MaxChargePercentage";
static const QString MaxDischargePercentagePath = "/Settings/CGwacs/MaxDischargePercentage";

SettingsBridge::SettingsBridge(Settings *settings, QObject *parent):
	DBusBridge(parent)
{
	consume(Service, settings, "deviceIds", QVariant(""), DeviceIdsPath);
	consume(Service, settings, "acPowerSetPoint", 0.0, -1e5, 1e5, AcPowerSetPointPath);
	consume(Service, settings, "maxChargePercentage", 100, 0, 100, MaxChargePercentagePath);
	consume(Service, settings, "maxDischargePercentage", 100, 0, 100, MaxDischargePercentagePath);
}

bool SettingsBridge::toDBus(const QString &path, QVariant &value)
{
	if (path == DeviceIdsPath) {
		value = value.value<QStringList>().join(",");
	}
	return true;
}

bool SettingsBridge::fromDBus(const QString &path, QVariant &value)
{
	if (path == DeviceIdsPath) {
		value = value.toString().split(',', QString::SkipEmptyParts);
	}
	return true;
}
