#include <QsLog.h>
#include <velib/qt/v_busitems.h>
#include "ac_sensor_settings.h"
#include "ac_sensor_settings_bridge.h"

static const QString Service = "sub/com.victronenergy.settings";

static bool positionFromDBus(DBusBridge*, QVariant &v)
{
	v = qVariantFromValue(static_cast<Position>(v.toInt()));
	return true;
}

static bool positionToDBus(DBusBridge*, QVariant &v)
{
	v = QVariant(static_cast<int>(v.value<Position>()));
	return true;
}

AcSensorSettingsBridge::AcSensorSettingsBridge(AcSensorSettings *settings, QObject *parent) :
	DBusBridge(Service, false, parent)
{
	QString primaryId = settings->serial();
	QString secondaryId = QString("%1_S").arg(settings->serial());
	QString primaryPath = QString("/Settings/Devices/%1").arg(primaryId);
	QString secondaryPath = QString("/Settings/Devices/%1").arg(secondaryId);

	consume(settings, "customName", QVariant(""),
			primaryPath + "/CustomName", false);
	consume(settings, "classAndVrmInstance",
			primaryPath + "/ClassAndVrmInstance");
	consume(settings, "isMultiPhase",
			QVariant(static_cast<int>(settings->isMultiPhase())),
			primaryPath + "/IsMultiphase", false);
	consume(settings, "position", QVariant(0),
			primaryPath + "/Position", false, positionFromDBus, positionToDBus);
	consume(settings, "l1ReverseEnergy", 0.0, 0.0, 1e6,
			primaryPath + "/L1ReverseEnergy", true);
	consume(settings, "l2ReverseEnergy", 0.0, 0.0, 1e6,
			primaryPath + "/L2ReverseEnergy", true);
	consume(settings, "l3ReverseEnergy", 0.0, 0.0, 1e6,
			primaryPath + "/L3ReverseEnergy", true);
	consume(settings, "supportMultiphase", QVariant(static_cast<int>(settings->supportMultiphase())),
			primaryPath + "/SupportMultiphase", false);

	consume(settings, "l2ClassAndVrmInstance",
			secondaryPath + "/ClassAndVrmInstance");
	consume(settings, "l2CustomName", QVariant(""),
			secondaryPath + "/CustomName", false);
	consume(settings, "l2Position", QVariant(0),
			secondaryPath + "/Position", false);
	consume(settings, "piggyEnabled", QVariant(0),
			secondaryPath + "/Enabled", false);

	// Allocate deviceinstances
	initDeviceInstance(primaryId, "grid", MinDeviceInstance);
	initDeviceInstance(secondaryId, "pvinverter", MinDeviceInstance);
}
