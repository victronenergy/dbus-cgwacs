#include <QsLog.h>
#include <QStringList>
#include <veutil/qt/ve_qitem.hpp>
#include "ac_sensor.h"
#include "ac_sensor_bridge.h"
#include "ac_sensor_mediator.h"
#include "ac_sensor_settings.h"
#include "ac_sensor_settings_bridge.h"
#include "ac_sensor_updater.h"
#include "dbus_bridge.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
#define SkipEmptyParts Qt::SkipEmptyParts
#else
#define SkipEmptyParts QString::SkipEmptyParts
#endif

static const QString DeviceIdsPath = "Settings/CGwacs/DeviceIds";

AcSensorMediator::AcSensorMediator(const QString &portName, int baud,
								   int timeout, bool isZigbee, VeQItem *settingsRoot,
								   QObject *parent) :
	QObject(parent),
	mModbus(new ModbusRtu(portName, baud, timeout, this)),
	mDeviceIdsItem(settingsRoot->itemGetOrCreate(DeviceIdsPath))
{
	DBusBridge settingsBridge(settingsRoot, false);
	settingsBridge.addSetting(DeviceIdsPath, "", 0, 0, false);

	/// @todo EV We assume that this setting is initialized before an AC sensor has been found
	mDeviceIdsItem->getValue();
	connect(mModbus, SIGNAL(serialEvent(const char *)), this, SIGNAL(serialEvent(const char *)));
	for (int i=1; i<=2; ++i) {
		AcSensor *m = new AcSensor(portName, i, this);
		AcSensor *pv = new AcSensor(portName, i, this);
		new AcSensorUpdater(m, pv, mModbus, isZigbee, m);
		mAcSensors.append(m);
		connect(m, SIGNAL(connectionStateChanged()),
				this, SLOT(onConnectionStateChanged()));
	}
}

void AcSensorMediator::onDeviceFound()
{
	AcSensor *m = static_cast<AcSensor *>(sender());
	AcSensorUpdater *mu = m->findChild<AcSensorUpdater *>();
	QLOG_INFO() << "Device found:" << m->serial() << '@' << m->portName();
	AcSensorSettings *settings = mu->settings();
	settings->setIsMultiPhase(m->protocolType() != AcSensor::Et112Protocol);
	settings->setParent(m);
	connect(settings, SIGNAL(classAndVrmInstanceChanged()),
			this, SLOT(onServiceTypeChanged()));
	connect(settings, SIGNAL(l2ClassAndVrmInstanceChanged()),
			this, SLOT(onServiceTypeChanged()));
	connect(settings, SIGNAL(piggyEnabledChanged()),
			this, SLOT(onServiceTypeChanged()));

	AcSensorSettingsBridge *b =
			new AcSensorSettingsBridge(settings, settings);
	connect(b, SIGNAL(initialized()),
			this, SLOT(onDeviceSettingsInitialized()));
	registerDevice(m->serial());
	b->updateIsInitialized();
}

void AcSensorMediator::onDeviceSettingsInitialized()
{
	AcSensorSettingsBridge *b = static_cast<AcSensorSettingsBridge *>(sender());
	AcSensorSettings *s = static_cast<AcSensorSettings *>(b->parent());
	AcSensor *m = static_cast<AcSensor *>(s->parent());
	AcSensorUpdater *mu = m->findChild<AcSensorUpdater *>();
	mu->startMeasurements();
}

void AcSensorMediator::onDeviceInitialized()
{
	AcSensor *acSensor = static_cast<AcSensor *>(sender());
	AcSensorUpdater *mu = acSensor->findChild<AcSensorUpdater *>();
	AcSensorSettings *sensorSettings = mu->settings();
	publishSensor(acSensor, mu->pvSensor(), sensorSettings);
}

void AcSensorMediator::onConnectionLost()
{
	AcSensor *acSensor = static_cast<AcSensor *>(sender());
	AcSensorBridge *bridge = acSensor->findChild<AcSensorBridge *>();
	delete bridge;

	foreach (AcSensor *sensor, mAcSensors) {
		if (sensor->connectionState() != Disconnected)
			return;
	}
	emit connectionLost();
}

void AcSensorMediator::onConnectionStateChanged()
{
	AcSensor *m = static_cast<AcSensor *>(sender());
	switch (m->connectionState()) {
	case Disconnected:
		onConnectionLost();
		break;
	case Searched:
		break;
	case Detected:
		onDeviceFound();
		break;
	case Connected:
		onDeviceInitialized();
		break;
	}
}

void AcSensorMediator::onServiceTypeChanged()
{
	AcSensorSettings *sensorSettings = static_cast<AcSensorSettings *>(sender());
	AcSensor *acSensor = static_cast<AcSensor *>(sensorSettings->parent());
	AcSensorBridge *bridge = acSensor->findChild<AcSensorBridge *>();
	if (bridge == 0) {
		// Settings have not been fully initialized yet. We need to have all
		// settings before creating the D-Bus service.
		return;
	}
	// Deleting and recreating the bridge will force recreation of the D-Bus
	// service with another name.
	delete bridge;
	AcSensorUpdater *updater = acSensor->findChild<AcSensorUpdater *>();
	AcSensor *pvSensor = updater->pvSensor();
	// Just in case pvInverterOnPhase2 was set, in which case we have two bridge objects.
	delete pvSensor->findChild<AcSensorBridge *>();
	publishSensor(acSensor, pvSensor, sensorSettings);
}

void AcSensorMediator::publishSensor(AcSensor *acSensor, AcSensor *pvSensor,
									 AcSensorSettings *acSensorSettings)
{
	new AcSensorBridge(acSensor, acSensorSettings, false, acSensor);
	if (acSensorSettings->piggyEnabled())
		new AcSensorBridge(pvSensor, acSensorSettings, true, pvSensor);
}

void AcSensorMediator::registerDevice(const QString &serial)
{
	if (mDeviceIds.isEmpty()) {
		QString ids = mDeviceIdsItem->getValue().toString();
		mDeviceIds = ids.split(',', SkipEmptyParts);
	}
	if (mDeviceIds.contains(serial))
		return;
	mDeviceIds.append(serial);
	mDeviceIdsItem->setValue(mDeviceIds.join(","));
}
