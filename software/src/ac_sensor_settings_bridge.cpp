#include <QsLog.h>
#include <QDBusConnection>
#include <QDBusVariant>
#include <velib/qt/v_busitems.h>
#include "ac_sensor_settings.h"
#include "ac_sensor_settings_bridge.h"

static const QString Service = "com.victronenergy.settings";

AcSensorSettingsBridge::AcSensorSettingsBridge(
	AcSensorSettings *settings, QObject *parent) :
	DBusBridge(parent)
{
	QString path = QString("/Settings/CGwacs/Devices/D%1").
				   arg(settings->serial());
	consume(Service, settings, "deviceType", QVariant(settings->deviceType()),
			path + "/DeviceType");
	consume(Service, settings, "customName", QVariant(""),
			path + "/CustomName");
	consume(Service, settings, "serviceType", QVariant("grid"),
			path + "/ServiceType");
	consume(Service, settings, "isMultiPhase",
			QVariant(static_cast<int>(settings->isMultiPhase())),
			path + "/IsMultiPhase");
	consume(Service, settings, "hub4Mode", QVariant(0),
			path + "/Hub4Mode");
	consume(Service, settings, "position", QVariant(0),
			path + "/Position");
	consume(Service, settings, "l1ReverseEnergy", 0.0, 0.0, 1e6,
			path + "/L1ReverseEnergy");
	consume(Service, settings, "l2ReverseEnergy", 0.0, 0.0, 1e6,
			path + "/L2ReverseEnergy");
	consume(Service, settings, "l3ReverseEnergy", 0.0, 0.0, 1e6,
			path + "/L3ReverseEnergy");

	consume(Service, settings, "l2CustomName", QVariant(""),
			path + "/L2/CustomName");
	consume(Service, settings, "l2ServiceType", QVariant(""),
			path + "/L2/ServiceType");
	consume(Service, settings, "l2Position", QVariant(0),
			path + "/L2/Position");
}

bool AcSensorSettingsBridge::toDBus(const QString &path, QVariant &v)
{
	if (path.endsWith("/Position")) {
		v = QVariant(static_cast<int>(v.value<Position>()));
	} else if (path.endsWith("/Hub4Mode")) {
		v = QVariant(static_cast<int>(v.value<Hub4Mode>()));
	}
	return true;
}

bool AcSensorSettingsBridge::fromDBus(const QString &path, QVariant &v)
{
	if (path.endsWith("/Position")) {
		v = qVariantFromValue(static_cast<Position>(v.toInt()));
	} else if (path.endsWith("/Hub4Mode")) {
		v = qVariantFromValue(static_cast<Hub4Mode>(v.toInt()));
	}
	return true;
}
