#include <QsLog.h>
#include "ac_sensor.h"
#include "ac_sensor_phase.h"
#include "ac_sensor_bridge.h"

AcSensor::AcSensor(const QString &portName, int slaveAddress, QObject *parent) :
	QObject(parent),
	mConnectionState(Disconnected),
	mDeviceType(0),
	mDeviceSubType(0),
	mErrorCode(0),
	mFirmwareVersion(0),
	mPortName(portName),
	mSlaveAddress(slaveAddress),
	mPhaseSequence(-1),
	mTotal(new AcSensorPhase(this)),
	mL1(new AcSensorPhase(this)),
	mL2(new AcSensorPhase(this)),
	mL3(new AcSensorPhase(this))
{
	resetValues();
}

void AcSensor::setConnectionState(ConnectionState state)
{
	if (mConnectionState == state)
		return;
	mConnectionState = state;
	emit connectionStateChanged();
}

void AcSensor::setDeviceType(int t)
{
	if (mDeviceType == t)
		return;
	mDeviceType = t;
	emit deviceTypeChanged();
}

void AcSensor::setDeviceSubType(int t)
{
	if (mDeviceSubType == t)
		return;
	mDeviceSubType = t;
	emit deviceSubTypeChanged();
}

AcSensor::ProtocolTypes AcSensor::protocolType() const
{
	if (mDeviceType >= 71 && mDeviceType <= 73)
		return Em24Protocol;
	if (mDeviceType >= 102 && mDeviceType <= 121)
		return Et112Protocol;
	if (mDeviceType >= 330 && mDeviceType <= 345)
		return Em340Protocol;
	if (mDeviceType >= 1744 && mDeviceType <= 1763)
		return Em540Protocol;
	return Unknown;
}

void AcSensor::setErrorCode(int code)
{
	if (mErrorCode == code)
		return;
	mErrorCode = code;
	emit errorCodeChanged();
}

void AcSensor::setSerial(const QString &s)
{
	if (mSerial == s)
		return;
	mSerial = s;
	emit serialChanged();
}

void AcSensor::setRole(const QString &s)
{
	if (mRole == s)
		return;
	mRole = s;
	emit roleChanged();
}

void AcSensor::setPhaseSequence(int v)
{
	mPhaseSequence = v;
}

void AcSensor::setFirmwareVersion(int v)
{
	if (mFirmwareVersion == v)
		return;
	mFirmwareVersion = v;
	emit firmwareVersionChanged();
}

AcSensorPhase *AcSensor::getPhase(Phase phase)
{
	switch (phase) {
	case MultiPhase:
		return mTotal;
	case PhaseL1:
		return mL1;
	case PhaseL2:
		return mL2;
	case PhaseL3:
		return mL3;
	default:
		QLOG_ERROR() << "Incorrect phase:" << phase;
		return 0;
	}
}

void AcSensor::resetValues()
{
	mTotal->resetValues();
	mL1->resetValues();
	mL2->resetValues();
	mL3->resetValues();
}

void AcSensor::flushValues()
{
	AcSensorBridge *bridge = findChild<AcSensorBridge *>();
	if (bridge != 0)
		bridge->publishPendingChanges();
}

int AcSensor::refreshTime() {
	switch (protocolType()) {
		case Em24Protocol:
			return 600;
		case Et112Protocol:
			return 750;
		case Em540Protocol:
			return 100;
		case Em340Protocol:
		default:
			return 2000;
	}
}
