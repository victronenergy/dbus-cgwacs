#include <QsLog.h>
#include <velib/qt/v_busitem.h>
#include "ac_sensor.h"
#include "ac_sensor_mediator.h"
#include "ac_sensor_settings.h"
#include "battery_info.h"
#include "charge_phase_control.h"
#include "dbus_cgwacs.h"
#include "dbus_service_monitor.h"
#include "hub4_control_bridge.h"
#include "maintenance_control.h"
#include "modbus_rtu.h"
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
	mSettings(new Settings(this)),
	mAcSensorMediator(new AcSensorMediator(portName, isZigbee, mSettings, this)),
	mMulti(new Multi(this)),
	mMaintenanceControl(0),
	mControlLoop(0),
	mBatteryInfo(new BatteryInfo(mServiceMonitor, mMulti, mSettings)),
	mHub4ControlBridge(0),
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

	connect(mAcSensorMediator, SIGNAL(gridMeterChanged()),
			this, SLOT(onGridMeterChanged()));
	connect(mAcSensorMediator, SIGNAL(hub4ModeChanged()),
			this, SLOT(onHub4ModeChanged()));
	connect(mAcSensorMediator, SIGNAL(isMultiPhaseChanged()),
			this, SLOT(onMultiPhaseChanged()));
	connect(mAcSensorMediator, SIGNAL(serialEvent(const char *)),
			this, SLOT(onSerialEvent(const char *)));
	connect(mAcSensorMediator, SIGNAL(connectionLost()),
			this, SIGNAL(connectionLost()));

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

void DBusCGwacs::onGridMeterChanged()
{
	if (mAcSensorMediator->gridMeter() == 0 && mHub4ControlBridge != 0) {
		delete mHub4ControlBridge;
		mHub4ControlBridge = 0;
	} else if (mAcSensorMediator->gridMeter() != 0 && mHub4ControlBridge == 0) {
		mHub4ControlBridge = new Hub4ControlBridge(mBatteryInfo, mSettings, this);
	}
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
	AcSensor *gridMeter = mAcSensorMediator->gridMeter();
	if (gridMeter == 0) {
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
		QLOG_INFO() << "Control loop: force charge (maintenance)";
	} else if (mAcSensorMediator->isMultiPhase()) {
		switch (mAcSensorMediator->hub4Mode()) {
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
		switch (mAcSensorMediator->hub4Mode()) {
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
