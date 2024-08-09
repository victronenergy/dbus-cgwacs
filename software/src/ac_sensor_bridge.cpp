#include <cmath>
#include <QCoreApplication>
#include <QStringList>
#include <QsLog.h>
#include <velib/vecan/products.h>
#include "ac_sensor.h"
#include "ac_sensor_bridge.h"
#include "ac_sensor_settings.h"
#include "ac_sensor_phase.h"

static bool roleFromDBus(DBusBridge*, QVariant &v)
{
	QString s = v.toString();
	return (QStringList() << "grid" << "pvinverter" << "genset" << "acload").contains(s);
}

static bool accountingModeFromDBus(DBusBridge*, QVariant &v)
{
	int n = v.toInt();
	return (n >= 0) && (n < 2);
}

static bool positionFromDBus(DBusBridge*, QVariant &v)
{
	int n = v.toInt();
	return (n >= 0) && (n < 3);
}

static bool customNameFromDBus(DBusBridge*, QVariant &v)
{
	Q_UNUSED(v)
	return true;
}

AcSensorBridge::AcSensorBridge(AcSensor *acSensor, AcSensorSettings *settings,
							   bool isSecondary, QObject *parent) :
	DBusBridge(getServiceName(acSensor, settings, isSecondary), true, parent)
{
	Q_ASSERT(acSensor != 0);
	Q_ASSERT(settings != 0);
	connect(acSensor, SIGNAL(destroyed()), this, SLOT(deleteLater()));
	connect(settings, SIGNAL(destroyed()), this, SLOT(deleteLater()));

	bool isGridmeter =
		(isSecondary ? settings->l2ServiceType() : settings->serviceType()) == "grid";
	// Changes on dbus will happen at least this often, but may be flushed
	// sooner (for fast ESS loop).
	setUpdateInterval(isGridmeter ? 1000 : 2500);

	produce(acSensor, "connectionState", "/Connected");
	produce(acSensor, "errorCode", "/ErrorCode");

	producePowerInfo(acSensor->total(), "/Ac", isGridmeter);
	producePowerInfo(acSensor->l1(), "/Ac/L1", isGridmeter);
	producePowerInfo(acSensor->l2(), "/Ac/L2", isGridmeter);
	producePowerInfo(acSensor->l3(), "/Ac/L3", isGridmeter);

	if (isSecondary || settings->serviceType() == "pvinverter")
		produce(settings, isSecondary ? "l2Position" : "position", "/Position",
			QString(), -1, false, positionFromDBus);
	produce(settings, isSecondary ? "l2ProductName" : "productName", "/ProductName");
	produce(settings, isSecondary ? "l2CustomName" : "customName", "/CustomName",
			QString(), -1, false, customNameFromDBus);
	produce(settings, isSecondary ? "l2ServiceType" : "serviceType", "/Role",
		QString(), -1, false, roleFromDBus);

	QString processName = QCoreApplication::applicationName();
	// The values of the items below will not change after creation, so we don't
	// need an update mechanism.
	produce("/Mgmt/ProcessName", processName);
	produce("/Mgmt/ProcessVersion", QCoreApplication::applicationVersion());
	produce("/FirmwareVersion", acSensor->firmwareVersion());
	produce("/PhaseSequence", settings->piggyEnabled() || acSensor->phaseSequence() < 0 ? QVariant() : acSensor->phaseSequence());
	int productId = 0;
	switch (acSensor->protocolType()) {
	case AcSensor::Em24Protocol:
		productId = isSecondary ?
			VE_PROD_ID_CARLO_GAVAZZI_EM24_PIGGY_BACK :
			VE_PROD_ID_CARLO_GAVAZZI_EM24;
		break;
	case AcSensor::Et112Protocol:
		productId = VE_PROD_ID_CARLO_GAVAZZI_ET112;
		break;
	case AcSensor::Et340Protocol:
		productId = isSecondary ?
			VE_PROD_ID_CARLO_GAVAZZI_ET340_PIGGY_BACK :
			VE_PROD_ID_CARLO_GAVAZZI_ET340;
		break;
	case AcSensor::Em540Protocol:
		productId = isSecondary ?
			VE_PROD_ID_CARLO_GAVAZZI_EM540_PIGGY_BACK :
			VE_PROD_ID_CARLO_GAVAZZI_EM540;
		break;
	case AcSensor::Em300S27Protocol:
		productId = isSecondary ?
			VE_PROD_ID_CARLO_GAVAZZI_EM300_SPEC27_PIGGY_BACK :
			VE_PROD_ID_CARLO_GAVAZZI_EM300_SPEC27;
		break;
	case AcSensor::Em300Protocol:
		productId = isSecondary ?
			VE_PROD_ID_CARLO_GAVAZZI_EM300_PIGGY_BACK :
			VE_PROD_ID_CARLO_GAVAZZI_EM300;
		break;
	default:
		Q_ASSERT(false);
		break;
	}
	produce("/ProductId", productId);
	produce("/DeviceType", acSensor->deviceType());
	produce("/Mgmt/Connection", acSensor->portName());

	int deviceInstance = isSecondary ?
		settings->l2DeviceInstance() :
		settings->deviceInstance();
	produce("/DeviceInstance", deviceInstance);
	produce("/Serial", isSecondary ? QString("%1_S").arg(acSensor->serial()) : acSensor->serial());
	produce("/AllowedRoles", isSecondary ? (QStringList() << "pvinverter" << "acload") : (QStringList() << "grid" << "pvinverter" << "genset" << "acload"));
	if (isGridmeter) {
		produce("/AccountingMode", QStringList() << "individual" << "sum");
	}

	// Publish some information about the known refresh time of these devices.
	// This is used by hub4control to know how fast it can make its control
	// loop.
	produce("/RefreshTime", acSensor->refreshTime());

	registerService();
}

bool AcSensorBridge::toDBus(const QString &path, QVariant &value)
{
	if (path == "/Connected") {
		value = QVariant(value.value<ConnectionState>() == Connected ? 1 : 0);
	} else if (path == "/Position") {
		value = QVariant(static_cast<int>(value.value<Position>()));
	}
	if (value.type() == QVariant::Double && !qIsFinite(value.toDouble()))
		value = QVariant();
	return true;
}

bool AcSensorBridge::fromDBus(const QString &path, QVariant &value)
{
	Q_UNUSED(path)
	Q_UNUSED(value)
	return false;
}

QString AcSensorBridge::toText(const QString &path, const QVariant &value, const QString &unit,
							   int precision)
{
	if (path == "/ProductId")
		return QString::number(value.toInt(), 16);
	return DBusBridge::toText(path, value, unit, precision);
}

QString AcSensorBridge::getServiceName(AcSensor *acSensor, AcSensorSettings *settings,
									   bool isSecondary)
{
	QString serviceType = isSecondary ?
		settings->l2ServiceType() :
		settings->serviceType();
	QString portId = acSensor->portName().
			replace("/dev/", "").
			replace("/", "_");
	QString serviceName = QString("pub/com.victronenergy.%1.cgwacs_%2_mb%3").
			arg(serviceType).
			arg(portId).
			arg(acSensor->slaveAddress());
	if (isSecondary)
		serviceName += "_l2";
	return serviceName;
}

void AcSensorBridge::producePowerInfo(AcSensorPhase *pi, const QString &path, bool isGridmeter)
{
	produce(pi, "current", path + "/Current", "A", 1);
	produce(pi, "voltage", path + "/Voltage", "V", 0);
	produce(pi, "power", path + "/Power", "W", 0);
	produce(pi, "energyForward", path + "/Energy/Forward", "kWh", 1);
	if (isGridmeter)
		produce(pi, "energyReverse", path + "/Energy/Reverse", "kWh", 1);
}
