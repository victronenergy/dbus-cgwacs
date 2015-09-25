#include <QsLog.h>
#include "ac_sensor.h"
#include "ac_sensor_bridge.h"
#include "ac_sensor_settings.h"
#include "ac_sensor_settings_bridge.h"
#include "ac_sensor_updater.h"
#include "control_loop.h"
#include "dbus_cgwacs.h"
#include "dbus_service_monitor.h"
#include "multi.h"
#include "multi_bridge.h"
#include "settings.h"
#include "settings_bridge.h"

DBusCGwacs::DBusCGwacs(const QString &portName, QObject *parent):
	QObject(parent),
	mServiceMonitor(new DbusServiceMonitor("com.victronenergy.vebus", this)),
	mModbus(new ModbusRtu(portName, 9600, this))
{
	qRegisterMetaType<ConnectionState>();
	qRegisterMetaType<Position>();
	qRegisterMetaType<MultiMode>();
	qRegisterMetaType<Hub4State>();
	qRegisterMetaType<QList<quint16> >();

	for (int i=1; i<=2; ++i) {
		AcSensor *m = new AcSensor(portName, i, this);
		new AcSensorUpdater(m, mModbus, m);
		mAcSensors.append(m);
		connect(m, SIGNAL(connectionStateChanged()),
				this, SLOT(onConnectionStateChanged()));
	}
	mSettings = new Settings(this);
	new SettingsBridge(mSettings, this);

	mMulti = new Multi(this);
	connect(mMulti, SIGNAL(isSetPointAvailableChanged()),
			this, SLOT(onIsSetPointAvailableChanged()));
	connect(mModbus, SIGNAL(serialEvent(const char *)),
			this, SLOT(onSerialEvent(const char *)));
	connect(mServiceMonitor, SIGNAL(servicesChanged()),
			this, SLOT(onServicesChanged()));
	updateMultiBridge();
}

void DBusCGwacs::onConnectionStateChanged()
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

void DBusCGwacs::onDeviceFound()
{
	AcSensor *m = static_cast<AcSensor *>(sender());
	AcSensorUpdater *mu = m->findChild<AcSensorUpdater *>();
	QLOG_INFO() << "Device found:" << m->serial()
				<< '@' << m->portName();
	AcSensorSettings *settings = mu->settings();
	settings->setIsMultiPhase(m->protocolType() != AcSensor::Et112Protocol);
	settings->setParent(m);
	connect(settings, SIGNAL(hub4ModeChanged()),
			this, SLOT(onHub4ModeChanged()));
	connect(settings, SIGNAL(serviceTypeChanged()),
			this, SLOT(onServiceTypeChanged()));
	connect(settings, SIGNAL(isMultiPhaseChanged()),
			this, SLOT(onMultiPhaseChanged()));
	connect(settings, SIGNAL(isControlLoopEnabledChanged()),
			this, SLOT(onControlLoopEnabledChanged()));
	AcSensorSettingsBridge *b =
			new AcSensorSettingsBridge(settings, settings);
	connect(b, SIGNAL(initialized()),
			this, SLOT(onDeviceSettingsInitialized()));
	mSettings->registerDevice(m->serial());
}

void DBusCGwacs::onDeviceSettingsInitialized()
{
	AcSensorSettingsBridge *b = static_cast<AcSensorSettingsBridge *>(sender());
	AcSensorSettings *s = static_cast<AcSensorSettings *>(b->parent());
	AcSensor *m = static_cast<AcSensor *>(s->parent());
	AcSensorUpdater *mu = m->findChild<AcSensorUpdater *>();
	mu->startMeasurements();
	updateControlLoop();
}

void DBusCGwacs::onDeviceInitialized()
{
	AcSensor *m = static_cast<AcSensor *>(sender());
	AcSensorUpdater *mu = m->findChild<AcSensorUpdater *>();
	new AcSensorBridge(m, mu->settings(), mSettings, m);
	updateControlLoop();
}

void DBusCGwacs::onServiceTypeChanged()
{
	AcSensorSettings *s = static_cast<AcSensorSettings *>(sender());
	AcSensor *m = static_cast<AcSensor *>(s->parent());
	AcSensorBridge *bridge = m->findChild<AcSensorBridge *>();
	if (bridge == 0) {
		// Settings have not been fully initialized yet. We need to have all
		// settings before creating the D-Bus service.
		return;
	}
	// Deleting and recreating the bridge will force recreation of the D-Bus
	// service with another name.
	delete bridge;
	new AcSensorBridge(m, s, mSettings, m);
	updateControlLoop();
}

void DBusCGwacs::onControlLoopEnabledChanged()
{
	updateControlLoop();
}

void DBusCGwacs::onHub4ModeChanged()
{
	updateControlLoop();
}

void DBusCGwacs::onMultiPhaseChanged()
{
	updateControlLoop();
}

void DBusCGwacs::onIsSetPointAvailableChanged()
{
	updateControlLoop();
}

void DBusCGwacs::onConnectionLost()
{
	foreach (AcSensor *sensor, mAcSensors) {
		if (sensor->connectionState() != Disconnected)
			return;
	}
	emit connectionLost();
}

void DBusCGwacs::onSerialEvent(const char *description)
{
	QLOG_ERROR() << "Serial event:" << description
				 << "Application will shut down.";
	exit(1);
}

void DBusCGwacs::onServicesChanged()
{
	updateMultiBridge();
}

void DBusCGwacs::updateControlLoop()
{
	foreach (ControlLoop *c, mControlLoops)
		delete c;
	mControlLoops.clear();
	if (!mMulti->isSetPointAvailable())
		return;
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
	if (settings == 0) {
		QLOG_INFO() << "Control loop: no energy meter with service type 'grid'";
		return;
	}
	if (settings->isMultiPhase()) {
		switch (settings->hub4Mode()) {
		case Hub4PhaseL1:
			QLOG_INFO() << "Control loop: multi phase, phase L1";
			mControlLoops.append(new ControlLoop(mMulti, PhaseL1, gridMeter, mSettings, 0, this));
			break;
		case Hub4PhaseCompensation:
			QLOG_INFO() << "Control loop: multi phase, phase compensation";
			mControlLoops.append(new ControlLoop(mMulti, MultiPhase, gridMeter, mSettings, 0, this));
			break;
		case Hub4PhaseSplit:
			QLOG_INFO() << "Control loop: multi phase, each phase has a dedicated control loop";
			mControlLoops.append(new ControlLoop(mMulti, PhaseL1, gridMeter, mSettings, 0, this));
			mControlLoops.append(new ControlLoop(mMulti, PhaseL2, gridMeter, mSettings, 0, this));
			mControlLoops.append(new ControlLoop(mMulti, PhaseL3, gridMeter, mSettings, 0, this));
			break;
		}
	} else {
		QLOG_INFO() << "Control loop: single phase";
		mControlLoops.append(new ControlLoop(mMulti, PhaseL1, gridMeter, mSettings, 0, this));
	}
}

void DBusCGwacs::updateMultiBridge()
{
	QList<QString> services = mServiceMonitor->services();
	MultiBridge *bridge = mMulti->findChild<MultiBridge *>();
	if (bridge != 0 && !services.contains(bridge->serviceName())) {
		QLOG_INFO() << "Multi @" << bridge->serviceName() << "disappeared.";
		delete bridge;
		bridge = 0;
	}
	if (bridge == 0 && !services.isEmpty()) {
		QLOG_INFO() << "Multi found @" << services.first();
		new MultiBridge(mMulti, services.first(), mMulti);
	}
}
