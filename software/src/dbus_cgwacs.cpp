#include <QsLog.h>
#include <velib/qt/v_busitem.h>
#include "ac_sensor.h"
#include "ac_sensor_bridge.h"
#include "ac_sensor_settings.h"
#include "ac_sensor_settings_bridge.h"
#include "ac_sensor_updater.h"
#include "battery_info.h"
#include "charge_phase_control.h"
#include "dbus_cgwacs.h"
#include "dbus_service_monitor.h"
#include "hub4_control_bridge.h"
#include "maintenance_control.h"
#include "multi.h"
#include "multi_bridge.h"
#include "multi_phase_data.h"
#include "phase_compensation_control.h"
#include "settings.h"
#include "settings_bridge.h"
#include "single_phase_control.h"
#include "split_phase_control.h"

DBusCGwacs::DBusCGwacs(const QString &portName, bool isZigbee, QObject *parent):
	QObject(parent),
	mServiceMonitor(new DbusServiceMonitor(this)),
	mModbus(new ModbusRtu(portName, 9600, isZigbee ? 2000 : 250, this)),
	mSettings(new Settings(this)),
	mMulti(new Multi(this)),
	mMaintenanceControl(0),
	mControlLoop(0),
	mBatteryInfo(new BatteryInfo(mServiceMonitor, mMulti, mSettings)),
	mTimeZone(new VBusItem(this))
{
	qRegisterMetaType<ConnectionState>();
	qRegisterMetaType<Position>();
	qRegisterMetaType<MultiMode>();
	qRegisterMetaType<Hub4State>();
	qRegisterMetaType<QList<quint16> >();

	connect(mTimeZone, SIGNAL(valueChanged()), this, SLOT(onTimeZoneChanged()));
	mTimeZone->consume("com.victronenergy.settings", "/Settings/System/TimeZone");
	mTimeZone->getValue();

	connect(mModbus, SIGNAL(serialEvent(const char *)), this, SLOT(onSerialEvent(const char *)));
	for (int i=1; i<=2; ++i) {
		AcSensor *m = new AcSensor(portName, i, this);
		AcSensor *pv = new AcSensor(portName, i, this);
		new AcSensorUpdater(m, pv, mModbus, isZigbee, m);
		mAcSensors.append(m);
		connect(m, SIGNAL(connectionStateChanged()),
				this, SLOT(onConnectionStateChanged()));
	}

	connect(mSettings, SIGNAL(stateChanged()), this, SLOT(onHub4StateChanged()));
	new SettingsBridge(mSettings, this);

	for (int i=0; i<3; ++i) {
		Phase phase = static_cast<Phase>(PhaseL1 + i);
		connect(mMulti->getPhaseData(phase), SIGNAL(isSetPointAvailableChanged()),
				this, SLOT(onIsSetPointAvailableChanged()));
	}

	connect(mServiceMonitor, SIGNAL(serviceAdded(QString)), this, SLOT(onServiceAdded(QString)));
	connect(mServiceMonitor, SIGNAL(serviceRemoved(QString)), this, SLOT(onServiceRemoved(QString)));
	mServiceMonitor->start();
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
	connect(settings, SIGNAL(deviceInstanceChanged()),
			this, SLOT(onServiceTypeChanged()));
	connect(settings, SIGNAL(l2DeviceInstanceChanged()),
			this, SLOT(onServiceTypeChanged()));
	connect(settings, SIGNAL(isMultiPhaseChanged()),
			this, SLOT(onMultiPhaseChanged()));
	connect(settings, SIGNAL(l2ServiceTypeChanged()),
			this, SLOT(onServiceTypeChanged()));
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
	updateControlLoop();
}

void DBusCGwacs::onDeviceInitialized()
{
	AcSensor *acSensor = static_cast<AcSensor *>(sender());
	AcSensorUpdater *mu = acSensor->findChild<AcSensorUpdater *>();
	AcSensorSettings *sensorSettings = mu->settings();
	publishSensor(acSensor, mu->pvSensor(), sensorSettings);
	updateControlLoop();
}

void DBusCGwacs::onServiceTypeChanged()
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
	updateControlLoop();
}

void DBusCGwacs::onHub4ModeChanged()
{
	updateControlLoop();
}

void DBusCGwacs::onHub4StateChanged()
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

void DBusCGwacs::onTimeZoneChanged()
{
	QString timeZone = mTimeZone->getValue().toString();
	setenv("TZ", timeZone.toLatin1(), 1);
	QDateTime time = QDateTime::currentDateTime();
	QLOG_INFO() << "Changed time zone to:" << timeZone
				<< "current local time is:" << time.toString();
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

void DBusCGwacs::onServiceAdded(QString service)
{
	if (service.startsWith("com.victronenergy.vebus.")) {
		MultiBridge *bridge = mMulti->findChild<MultiBridge *>();
		if (bridge == 0) {
			QLOG_INFO() << "Multi found @" << service;
			new MultiBridge(mMulti, service, mMulti);
		}
	}
}

void DBusCGwacs::onServiceRemoved(QString service)
{
	if (service.startsWith("com.victronenergy.vebus.")) {
		MultiBridge *bridge = mMulti->findChild<MultiBridge *>();
		if (bridge != 0 && bridge->serviceName() == service) {
			QLOG_INFO() << "Multi @" << bridge->serviceName() << "disappeared.";
			delete bridge;
			bridge = 0;
		}
	}
}

void DBusCGwacs::updateControlLoop()
{
	delete mControlLoop;
	mControlLoop = 0;
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
		delete mMaintenanceControl;
		mMaintenanceControl = 0;
		return;
	}
	if (mMaintenanceControl == 0)
		mMaintenanceControl = new MaintenanceControl(mMulti, mSettings, 0, mMulti);
	bool charge = false;
	switch (mSettings->state()) {
	case Hub4ChargeFromGrid:
	case Hub4Storage:
		charge = true;
		break;
	default:
		charge = false;
		break;
	}
	// Keep in mind that:
	// 1) It may take some time for the vebus service to report all setpoint, so
	//    at this moment, there may be some missing setpoints.
	// 2) If a control loop is started on a phase without setpoint, it won't do
	//    anything until the setpoint is reported.
	// So it is safer to create a control loop on a phase the may not have a
	// setpoint. Of cource, the ControlLoop classes should check for the
	// presence of the setpoint regularly.
	if (charge) {
		mControlLoop = new ChargePhaseControl(mMulti, gridMeter, mSettings);
	} else if (settings->isMultiPhase()) {
		switch (settings->hub4Mode()) {
		case Hub4SinglePhaseCompensation:
		{
			QList<Phase> setpointPhases = mMulti->getSetpointPhases();
			Phase phase = setpointPhases.isEmpty() ? PhaseL1 : setpointPhases.first();
			QLOG_INFO() << QString("Control loop: multi phase, phase L%1").arg(phase);
			mControlLoop = new SinglePhaseControl(mMulti, gridMeter, mSettings, phase,
												  Hub4PhaseCompensation, this);
			break;
		}
		case Hub4PhaseCompensation:
			QLOG_INFO() << "Control loop: multi phase, phase compensation";
			mControlLoop = new PhaseCompensationControl(mMulti, gridMeter, mSettings, this);
			break;
		case Hub4PhaseSplit:
			QLOG_INFO() << "Control loop: split phase control";
			mControlLoop = new SplitPhaseControl(mMulti, gridMeter, mSettings, this);
			break;
		case Hub4PhaseL1:
		case Hub4Disabled:
		default:
			QLOG_INFO() << "Control loop: disabled";
			break;
		}
	} else {
		switch (settings->hub4Mode()) {
		case Hub4Disabled:
			QLOG_INFO() << "Control loop: disabled";
			break;
		default:
			QLOG_INFO() << "Control loop: single phase";
			mControlLoop = new SinglePhaseControl(mMulti, gridMeter, mSettings, PhaseL1,
												  Hub4PhaseL1, this);
			break;
		}
	}
	mControlLoop->setBatteryInfo(mBatteryInfo);
}

void DBusCGwacs::publishSensor(AcSensor *acSensor, AcSensor *pvSensor,
							   AcSensorSettings *acSensorSettings)
{
	AcSensorBridge *bridge = new AcSensorBridge(acSensor, acSensorSettings, false, acSensor);
	if (acSensorSettings->serviceType() == "grid")
		new Hub4ControlBridge(mBatteryInfo, mSettings, bridge);
	if (!acSensorSettings->l2ServiceType().isEmpty())
		new AcSensorBridge(pvSensor, acSensorSettings, true, pvSensor);
}
