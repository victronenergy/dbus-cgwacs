#include <QsLog.h>
#include "ac_sensor.h"
#include "ac_sensor_bridge.h"
#include "ac_sensor_mediator.h"
#include "ac_sensor_settings.h"
#include "ac_sensor_settings_bridge.h"
#include "ac_sensor_updater.h"
#include "settings.h"

AcSensorMediator::AcSensorMediator(const QString &portName, bool isZigbee, Settings *settings,
								   QObject *parent) :
	QObject(parent),
	mModbus(new ModbusRtu(portName, 9600, isZigbee ? 2000 : 250, this)),
	mGridMeter(0),
	mSettings(settings),
	mHub4Mode(Hub4Disabled),
	mIsMultiPhase(false)
{
	connect(mModbus, SIGNAL(serialEvent(const char *)), this, SIGNAL(onSerialEvent(const char *)));
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
	connect(settings, SIGNAL(serviceTypeChanged()),
			this, SLOT(onServiceTypeChanged()));
	connect(settings, SIGNAL(deviceInstanceChanged()),
			this, SLOT(onServiceTypeChanged()));
	connect(settings, SIGNAL(l2DeviceInstanceChanged()),
			this, SLOT(onServiceTypeChanged()));
	connect(settings, SIGNAL(l2ServiceTypeChanged()),
			this, SLOT(onServiceTypeChanged()));
	AcSensorSettingsBridge *b =
			new AcSensorSettingsBridge(settings, settings);
	connect(b, SIGNAL(initialized()),
			this, SLOT(onDeviceSettingsInitialized()));
	mSettings->registerDevice(m->serial());
}

void AcSensorMediator::onDeviceSettingsInitialized()
{
	AcSensorSettingsBridge *b = static_cast<AcSensorSettingsBridge *>(sender());
	AcSensorSettings *s = static_cast<AcSensorSettings *>(b->parent());
	if (s->deviceInstance() == -1)
		s->setDeviceInstance(mSettings->getDeviceInstance(s->serial(), false));
	if (s->l2DeviceInstance() == -1)
		s->setL2DeviceInstance(mSettings->getDeviceInstance(s->serial(), true));
	// Conversion from v1.0.8 to v1.0.9: Compensating over Phase 1 is no longer used for multi
	// phase systems.
	if (s->isMultiPhase() && s->hub4Mode() == Hub4PhaseL1)
		s->setHub4Mode(Hub4PhaseCompensation);
	AcSensor *m = static_cast<AcSensor *>(s->parent());
	AcSensorUpdater *mu = m->findChild<AcSensorUpdater *>();
	mu->startMeasurements();
	updateGridMeter();
}

void AcSensorMediator::onDeviceInitialized()
{
	AcSensor *acSensor = static_cast<AcSensor *>(sender());
	AcSensorUpdater *mu = acSensor->findChild<AcSensorUpdater *>();
	AcSensorSettings *sensorSettings = mu->settings();
	publishSensor(acSensor, mu->pvSensor(), sensorSettings);
	updateGridMeter();
}

void AcSensorMediator::onConnectionLost()
{
	AcSensor *acSensor = static_cast<AcSensor *>(sender());
	AcSensorBridge *bridge = acSensor->findChild<AcSensorBridge *>();
	delete bridge;

	updateGridMeter();
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
	// Just in case pvInverterOnPhase2 was set, in which case we have to bridge
	// objects.
	delete pvSensor->findChild<AcSensorBridge *>();
	publishSensor(acSensor, pvSensor, sensorSettings);
	updateGridMeter();
}

void AcSensorMediator::onHub4ModeChanged()
{
	Hub4Mode m = Hub4Disabled;
	if (mGridMeter != 0) {
		AcSensorSettings *settings = mGridMeter->findChild<AcSensorSettings *>();
		m = settings->hub4Mode();
	}
	if (mHub4Mode == m)
		return;
	mHub4Mode = m;
	emit hub4ModeChanged();
}

void AcSensorMediator::onIsMultiPhaseChanged()
{
	bool isMultiPhase = false;
	if (mGridMeter != 0) {
		AcSensorSettings *settings = mGridMeter->findChild<AcSensorSettings *>();
		isMultiPhase = settings->isMultiPhase();
	}
	if (mIsMultiPhase == isMultiPhase)
		return;
	mIsMultiPhase = isMultiPhase;
	emit isMultiPhaseChanged();
}

void AcSensorMediator::updateGridMeter()
{
	AcSensor *gridMeter = 0;
	AcSensorSettings *settings = 0;
	foreach (AcSensor *em, mAcSensors) {
		if (em->connectionState() == Connected) {
			AcSensorSettings *s = em->findChild<AcSensorSettings *>();
			if (s != 0 && s->serviceType() == "grid") {
				gridMeter = em;
				settings = s;
				break;
			}
		}
	}
	if (mGridMeter == gridMeter)
		return;
	if (mGridMeter != 0) {
		mGridMeter->disconnect(this, SIGNAL(hub4ModeChanged()));
		mGridMeter->disconnect(this, SIGNAL(isMultiPhaseChanged()));
	}
	mGridMeter = gridMeter;
	if (settings != 0) {
		connect(settings, SIGNAL(hub4ModeChanged()),
				this, SLOT(onHub4ModeChanged()));
		connect(settings, SIGNAL(isMultiPhaseChanged()),
				this, SLOT(onIsMultiPhaseChanged()));
	}
	onHub4ModeChanged();
	onIsMultiPhaseChanged();
	emit gridMeterChanged();
}

void AcSensorMediator::publishSensor(AcSensor *acSensor, AcSensor *pvSensor,
									 AcSensorSettings *acSensorSettings)
{
	new AcSensorBridge(acSensor, acSensorSettings, false, acSensor);
	if (!acSensorSettings->l2ServiceType().isEmpty())
		new AcSensorBridge(pvSensor, acSensorSettings, true, pvSensor);
}
