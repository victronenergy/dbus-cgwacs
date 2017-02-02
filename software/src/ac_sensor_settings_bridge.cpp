#include <QsLog.h>
#include <velib/qt/v_busitems.h>
#include "ac_sensor_settings.h"
#include "ac_sensor_settings_bridge.h"

static const QString Service = "sub/com.victronenergy.settings";

AcSensorSettingsBridge::AcSensorSettingsBridge(AcSensorSettings *settings, QObject *parent) :
	DBusBridge(Service, false, parent)
{
	QString path = QString("/Settings/CGwacs/Devices/D%1").
				   arg(settings->serial());
	consume(settings, "deviceType", QVariant(settings->deviceType()),
			path + "/DeviceType", false);
	consume(settings, "customName", QVariant(""),
			path + "/CustomName", false);
	consume(settings, "serviceType", QVariant("grid"),
			path + "/ServiceType", false);
	consume(settings, "isMultiPhase",
			QVariant(static_cast<int>(settings->isMultiPhase())),
			path + "/IsMultiPhase", false);
	consume(settings, "position", QVariant(0),
			path + "/Position", false);
	consume(settings, "deviceInstance", QVariant(-1),
			path + "/DeviceInstance", false);
	consume(settings, "l1ReverseEnergy", 0.0, 0.0, 1e6,
			path + "/L1ReverseEnergy", true);
	consume(settings, "l2ReverseEnergy", 0.0, 0.0, 1e6,
			path + "/L2ReverseEnergy", true);
	consume(settings, "l3ReverseEnergy", 0.0, 0.0, 1e6,
			path + "/L3ReverseEnergy", true);

	consume(settings, "l2CustomName", QVariant(""),
			path + "/L2/CustomName", false);
	consume(settings, "l2ServiceType", QVariant(""),
			path + "/L2/ServiceType", false);
	consume(settings, "l2Position", QVariant(0),
			path + "/L2/Position", false);
	consume(settings, "l2DeviceInstance", QVariant(-1),
			path + "/L2/DeviceInstance", false);
}

bool AcSensorSettingsBridge::toDBus(const QString &path, QVariant &v)
{
	if (path.endsWith("/Position")) {
		v = QVariant(static_cast<int>(v.value<Position>()));
	}
	return true;
}

bool AcSensorSettingsBridge::fromDBus(const QString &path, QVariant &v)
{
	if (path.endsWith("/Position")) {
		v = qVariantFromValue(static_cast<Position>(v.toInt()));
	}
	return true;
}
