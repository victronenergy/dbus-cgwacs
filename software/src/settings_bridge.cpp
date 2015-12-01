#include <QStringList>
#include <QsLog.h>
#include <QVariant>
#include "settings.h"
#include "settings_bridge.h"

static const QString Service = "com.victronenergy.settings";
static const QString CGwacsPath = "/Settings/CGwacs";

static const QString DeviceIdsPath = CGwacsPath + "/DeviceIds";
static const QString AcPowerSetPointPath = CGwacsPath + "/AcPowerSetPoint";
static const QString MaxChargePercentagePath = CGwacsPath + "/MaxChargePercentage";
static const QString MaxDischargePercentagePath = CGwacsPath + "/MaxDischargePercentage";

static const QString BatteryLifePath = CGwacsPath + "/BatteryLife";
static const QString StatePath = BatteryLifePath + "/State";
static const QString FlagsPath = BatteryLifePath + "/Flags";
static const QString SocLimitPath = BatteryLifePath + "/SocLimit";
static const QString MinSocLimitPath = BatteryLifePath + "/MinimumSocLimit";
static const QString DischargedTimePath = BatteryLifePath + "/DischargedTime";

SettingsBridge::SettingsBridge(Settings *settings, QObject *parent):
	DBusBridge(parent)
{
	consume(Service, settings, "deviceIds", QVariant(""), DeviceIdsPath);
	consume(Service, settings, "acPowerSetPoint", 0.0, -1e5, 1e5, AcPowerSetPointPath);
	consume(Service, settings, "maxChargePercentage", 100, 0, 100, MaxChargePercentagePath);
	consume(Service, settings, "maxDischargePercentage", 100, 0, 100, MaxDischargePercentagePath);
	consume(Service, settings, "state", static_cast<int>(MaintenanceStateRestart), StatePath);
	consume(Service, settings, "flags", 0, FlagsPath);
	consume(Service, settings, "socLimit", SocSwitchDefaultMin, 0, 100, SocLimitPath);
	consume(Service, settings, "minSocLimit", SocSwitchDefaultMin, 0, 100, MinSocLimitPath);
	consume(Service, settings, "dischargedTime", QVariant(0), DischargedTimePath);
}

bool SettingsBridge::toDBus(const QString &path, QVariant &value)
{
	if (path == DeviceIdsPath) {
		value = value.value<QStringList>().join(",");
	} else if (path == StatePath) {
		value = static_cast<int>(value.value<MaintenanceState>());
	} else if (path == DischargedTimePath) {
		QDateTime dt = qvariant_cast<QDateTime>(value);
		value = dt.isValid() ? dt.toTime_t() : 0;
	}
	return true;
}

bool SettingsBridge::fromDBus(const QString &path, QVariant &value)
{
	if (path == DeviceIdsPath) {
		value = value.toString().split(',', QString::SkipEmptyParts);
	} else if (path == StatePath) {
		value = QVariant::fromValue(static_cast<MaintenanceState>(value.toInt()));
	} else if (path == DischargedTimePath) {
		unsigned int t = value.toUInt();
		value = t == 0 ? QDateTime() : QVariant(QDateTime::fromTime_t(t));
	}
	return true;
}
