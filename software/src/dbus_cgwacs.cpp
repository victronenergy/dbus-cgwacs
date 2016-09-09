#include <QsLog.h>
#include <velib/qt/v_busitem.h>
#include "ac_sensor.h"
#include "ac_sensor_mediator.h"
#include "ac_sensor_settings.h"
#include "ac_sensor_settings_bridge.h"
#include "ac_sensor_updater.h"
#include "vbus_item_battery.h"
#include "battery_info.h"
#include "dbus_cgwacs.h"
#include "dbus_service_monitor.h"
#include "hub4_control_bridge.h"
#include "battery_life.h"
#include "modbus_rtu.h"
#include "multi.h"
#include "multi_bridge.h"
#include "multi_phase_data.h"
#include "phase_compensation_control.h"
#include "settings.h"
#include "settings_bridge.h"
#include "single_phase_control.h"
#include "split_phase_control.h"
#include "vbus_item_system_calc.h"

DBusCGwacs::DBusCGwacs(const QString &portName, bool isZigbee, QObject *parent):
	QObject(parent),
	mServiceMonitor(new DbusServiceMonitor(this)),
	mSettings(new Settings(this)),
	mAcSensorMediator(new AcSensorMediator(portName, isZigbee, mSettings, this)),
	mMulti(new Multi(this)),
	mSystemCalc(new VBusItemSystemCalc(mServiceMonitor, this)),
	mMaintenanceControl(0),
	mControlLoop(0),
	mBatteryInfo(new BatteryInfo(mMulti, mSettings)),
	mHub4ControlBridge(0),
	mTimeZone(new VBusItem(this))
{
	qRegisterMetaType<ConnectionState>();
	qRegisterMetaType<BatteryLifeState>();
	qRegisterMetaType<MultiMode>();
	qRegisterMetaType<Position>();
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
	} else if (service.startsWith("com.victronenergy.battery.")) {
		QLOG_INFO() << "Battery found @" << service;
		Battery *battery = new VbusItemBattery(service, this);
		mBatteryInfo->addBattery(battery);
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
	} else if (service.startsWith("com.victronenergy.battery.")) {
		QLOG_INFO() << "Battery @" << service << "disappeared.";
		foreach (VbusItemBattery *b, findChildren<VbusItemBattery *>()) {
			if (b->serviceName() == service) {
				mBatteryInfo->removeBattery(b);
				delete b;
				break;
			}
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
		mMaintenanceControl = new BatteryLife(mSystemCalc, mMulti, mSettings, 0, mMulti);
	if (mAcSensorMediator->isMultiPhase()) {
		switch (mAcSensorMediator->hub4Mode()) {
		case Hub4SinglePhaseCompensation:
		{
			QList<Phase> setpointPhases = mMulti->getSetpointPhases();
			Phase phase = setpointPhases.isEmpty() ? PhaseL1 : setpointPhases.first();
			QLOG_INFO() << QString("Control loop: multi phase, phase L%1").arg(phase);
			mControlLoop = new SinglePhaseControl(mSystemCalc, mMulti, gridMeter, mSettings, phase,
												  Hub4PhaseCompensation, this);
			break;
		}
		case Hub4PhaseCompensation:
			QLOG_INFO() << "Control loop: multi phase, phase compensation";
			mControlLoop = new PhaseCompensationControl(mSystemCalc, mMulti, gridMeter, mSettings, this);
			break;
		case Hub4PhaseSplit:
			QLOG_INFO() << "Control loop: split phase control";
			mControlLoop = new SplitPhaseControl(mSystemCalc, mMulti, gridMeter, mSettings, this);
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
			mControlLoop = new SinglePhaseControl(mSystemCalc, mMulti, gridMeter, mSettings,
												  PhaseL1, Hub4PhaseL1, this);
			break;
		}
	}
	if (mControlLoop != 0)
		mControlLoop->setBatteryInfo(mBatteryInfo);
}
