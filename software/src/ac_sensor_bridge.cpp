#include <cmath>
#include <QCoreApplication>
#include <QsLog.h>
#include <QStringList>
#include <velib/vecan/products.h>
#include "ac_sensor.h"
#include "ac_sensor_bridge.h"
#include "ac_sensor_settings.h"
#include "power_info.h"
#include "settings.h"
#include "version.h"

AcSensorBridge::AcSensorBridge(AcSensor *acSensor, AcSensorSettings *emSettings,
							   Settings *settings, bool isSecundary,
							   QObject *parent) :
	DBusBridge(parent),
	mAcSensor(acSensor)
{
	Q_ASSERT(acSensor != 0);
	Q_ASSERT(emSettings != 0);
	connect(acSensor, SIGNAL(destroyed()), this, SLOT(deleteLater()));
	connect(emSettings, SIGNAL(destroyed()), this, SLOT(deleteLater()));
	setUpdateInterval(1000);

	QString serviceType = isSecundary ?
		emSettings->l2ServiceType() :
		emSettings->serviceType();
	setServiceName(QString("com.victronenergy.%1.%2").
			arg(serviceType).
			arg(acSensor->portName().
				replace("/dev/", "").
				replace("/", "_")));

	produce(acSensor, "connectionState", "/Connected");
	produce(acSensor, "errorCode", "/ErrorCode");

	producePowerInfo(acSensor->meanPowerInfo(), "/Ac");
	producePowerInfo(acSensor->l1PowerInfo(), "/Ac/L1");
	producePowerInfo(acSensor->l2PowerInfo(), "/Ac/L2");
	producePowerInfo(acSensor->l3PowerInfo(), "/Ac/L3");

	produce(emSettings, isSecundary ? "l2Position" : "position", "/Position");
	produce(emSettings, isSecundary ? "l2CustomName" : "customName", "/CustomName");

	if (settings != 0 && serviceType == "grid") {
		produce(settings, "acPowerSetPoint", "/Hub4/AcPowerSetpoint");
		produce(settings, "maxChargePercentage", "/Hub4/MaxChargePercentage");
		produce(settings, "maxDischargePercentage", "/Hub4/MaxDischargePercentage");
		produce(settings, "state", "/Hub4/State");
		produce(settings, "maintenanceInterval", "/Hub4/Maintenance/Interval");
		produce(settings, "maintenanceDate", "/Hub4/Maintenance/Date");
	}

	QString processName = QCoreApplication::arguments()[0];
	// The values of the items below will not change after creation, so we don't
	// need an update mechanism.
	produce("/Mgmt/ProcessName", processName);
	produce("/Mgmt/ProcessVersion", VERSION);
	produce("/FirmwareVersion", acSensor->firmwareVersion());
	produce("/ProductName", acSensor->productName());
	produce("/ProductId", VE_PROD_ID_CARLO_GAVAZZI_EM);
	produce("/DeviceType", acSensor->deviceType());
	produce("/Mgmt/Connection", acSensor->portName());
	int deviceInstance = settings->getDeviceInstance(serviceType,
													 acSensor->serial());
	produce("/DeviceInstance", deviceInstance);
	produce("/Serial", acSensor->serial());

	registerService();
}

bool AcSensorBridge::toDBus(const QString &path, QVariant &value)
{
	if (path == "/Connected") {
		value = QVariant(value.value<ConnectionState>() == Connected ? 1 : 0);
	} else if (path == "/Position") {
		value = QVariant(static_cast<int>(value.value<Position>()));
	} else if (path == "/CustomName") {
		QString name = value.toString();
		if (name.isEmpty())
			value = mAcSensor->productName();
	} else if (path == "/Hub4/State") {
		// value = static_cast<int>(value.value<Hub4State>());
	} else if (path == "/Hub4/Maintenance/Date") {
		QDateTime t = value.value<QDateTime>();
		value = t.isValid() ? t.toTime_t() : 0;
	}
	if (value.type() == QVariant::Double && !std::isfinite(value.toDouble()))
		value = QVariant();
	return true;
}

bool AcSensorBridge::fromDBus(const QString &path, QVariant &value)
{
	if (path == "/CustomName") {
		QString name = value.toString();
		if (name == mAcSensor->productName())
			value = "";
		return true;
	} else if (path == "/Hub4/Maintenance/Date") {
		time_t t = value.value<time_t>();
		value = t == 0 ? QDateTime() : QDateTime::fromTime_t(t);
		return true;
	} else if (path == "/Hub4/State") {
		// value = static_cast<Hub4State>(value.toInt());
		return true;
	} else if (path.startsWith("/Hub4/")) {
		return true;
	}
	return false;
}

void AcSensorBridge::producePowerInfo(PowerInfo *pi, const QString &path)
{
	produce(pi, "current", path + "/Current", "A", 1);
	produce(pi, "voltage", path + "/Voltage", "V", 0);
	produce(pi, "power", path + "/Power", "W", 0);
	produce(pi, "energyForward", path + "/Energy/Forward", "kWh", 1);
	produce(pi, "energyReverse", path + "/Energy/Reverse", "kWh", 1);
}
