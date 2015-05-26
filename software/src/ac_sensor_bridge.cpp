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

AcSensorBridge::AcSensorBridge(AcSensor *acSensor,
							   AcSensorSettings *emSettings,
							   Settings *settings,
							   QObject *parent) :
	DBusBridge(parent),
	mAcSensor(acSensor)
{
	Q_ASSERT(acSensor != 0);
	Q_ASSERT(emSettings != 0);
	connect(acSensor, SIGNAL(destroyed()), this, SLOT(deleteLater()));
	connect(emSettings, SIGNAL(destroyed()), this, SLOT(deleteLater()));
	setUpdateInterval(1000);

	setServiceName(QString("com.victronenergy.%1.%2").
			arg(emSettings->serviceType()).
			arg(acSensor->portName().
				replace("/dev/", "").
				replace("/", "_")));

	produce(acSensor, "isConnected", "/Connected");
	produce(acSensor, "errorCode", "/ErrorCode");

	producePowerInfo(acSensor->meanPowerInfo(), "/Ac");
	producePowerInfo(acSensor->l1PowerInfo(), "/Ac/L1");
	producePowerInfo(acSensor->l2PowerInfo(), "/Ac/L2");
	producePowerInfo(acSensor->l3PowerInfo(), "/Ac/L3");

	produce(emSettings, "position", "/Position");
	produce(emSettings, "customName", "/CustomName");

	if (settings != 0 && emSettings->serviceType() == "grid") {
		produce(settings, "acPowerSetPoint", "/Hub4/AcPowerSetpoint");
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
	QString portName = acSensor->portName();
	int deviceInstance = getDeviceInstance(portName, "/dev/ttyUSB", 288);
	if (deviceInstance == -1)
		deviceInstance = getDeviceInstance(portName, "/dev/ttyO", 256);
	produce("/Mgmt/Connection", portName);
	produce("/DeviceInstance", deviceInstance);
	QString serial = acSensor->serial();
	produce("/Serial", serial);

	registerService();
}

bool AcSensorBridge::toDBus(const QString &path, QVariant &value)
{
	if (path == "/Connected") {
		value = QVariant(value.toBool() ? 1 : 0);
	} else if (path == "/Position") {
		value = QVariant(static_cast<int>(value.value<Position>()));
	} else if (path == "/CustomName") {
		QString name = value.toString();
		if (name.isEmpty())
			value = mAcSensor->productName();
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

int AcSensorBridge::getDeviceInstance(const QString &path,
										 const QString &prefix,
										 int instanceBase)
{
	if (path.startsWith(prefix)) {
		return instanceBase + path.mid(prefix.size()).toInt();
	}
	return -1;
}
