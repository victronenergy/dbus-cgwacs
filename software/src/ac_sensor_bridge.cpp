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
	produce(emSettings, isSecundary ? "l2ProductName" : "productName", "/ProductName");
	produce(emSettings, isSecundary ? "l2EffectiveCustomName" : "effectiveCustomName", "/CustomName");

	QString processName = QCoreApplication::arguments()[0];
	// The values of the items below will not change after creation, so we don't
	// need an update mechanism.
	produce("/Mgmt/ProcessName", processName);
	produce("/Mgmt/ProcessVersion", QCoreApplication::applicationVersion());
	produce("/FirmwareVersion", acSensor->firmwareVersion());
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
	}
	if (value.type() == QVariant::Double && !std::isfinite(value.toDouble()))
		value = QVariant();
	return true;
}

bool AcSensorBridge::fromDBus(const QString &path, QVariant &value)
{
	Q_UNUSED(value)
	if (path == "/CustomName")
		return true;
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
