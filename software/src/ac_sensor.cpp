#include <QsLog.h>
#include "ac_sensor.h"
#include "power_info.h"

AcSensor::AcSensor(const QString &portName, int slaveAddress, QObject *parent) :
	QObject(parent),
	mConnectionState(Disconnected),
	mDeviceType(0),
	mDeviceSubType(0),
	mErrorCode(0),
	mFirmwareVersion(0),
	mPortName(portName),
	mSlaveAddress(slaveAddress),
	mMeanPowerInfo(new PowerInfo(this)),
	mL1PowerInfo(new PowerInfo(this)),
	mL2PowerInfo(new PowerInfo(this)),
	mL3PowerInfo(new PowerInfo(this))
{
	resetValues();
}

ConnectionState AcSensor::connectionState() const
{
	return mConnectionState;
}

void AcSensor::setConnectionState(ConnectionState state)
{
	if (mConnectionState == state)
		return;
	mConnectionState = state;
	emit connectionStateChanged();
}

int AcSensor::deviceType() const
{
	return mDeviceType;
}

void AcSensor::setDeviceType(int t)
{
	if (mDeviceType == t)
		return;
	mDeviceType = t;
	emit deviceTypeChanged();
}

int AcSensor::deviceSubType() const
{
	return mDeviceSubType;
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
	return Em340Protocol;
}

QString AcSensor::productName() const
{
	switch (mDeviceType) {
	case 102:
		// EM112-DIN AV1 1 x S1 PF B
		return "EM112-DIN.AV1.1.X.S1.X";
	case 103:
		return "EM111-DIN.AV8.1.X.S1.X";
	case 104:
		// EM112-DIN AV0 1 x S1 PF B
		return "EM112-DIN.AV0.1.x.S1.X";
	case 120:
		return "ET112-DIN.AV0.1.X.S1.X";
	case 121:
		return "ET112-DIN.AV1.1.X.S1.X";
	case 345:
		return "EM340-DIN.AV2.3.X.S1.X";
	}
	if (protocolType() != Em24Protocol) {
		QLOG_WARN() << "Unkown device type:" << mDeviceType;
		return "???";
	}
	switch (mDeviceSubType) {
	case 75:
		return "EM24-DIN.AV6.3.X.O2.X";
	case 76:
		return "EM24-DIN.AV6.3.X.R2.X";
	case 77:
		return "EM24-DIN.AV6.3.X.IS.X";
	case 78:
		return "EM24-DIN.AV6.3.X.XX.X";
	}
	QString result = "EM24-DIN";
	switch (mDeviceSubType & 0x3F) {
	case 0:
		result += ".AV9.3.X.O2";
		break;
	case 2:
		result += ".AV9.3.X.IS";
		break;
	case 3:
		result += ".AV9.3.X.XX";
		break;
	case 4:
		result += ".AV9.3.X.O2";
		break;
	case 5:
		result += ".AV2.3.X.IS";
		break;
	case 6:
		result += ".AV2.3.X.XX";
		break;
	case 7:
		result += ".AV2.3.X.O2";
		break;
	case 9:
		result += ".AV5.3.X.IS";
		break;
	case 10:
		result += ".AV5.3.X.XX";
		break;
	}
	switch (mDeviceSubType & 0xC0) {
	case 0x00:
		result += ".PFA";
		break;
	case 0x40:
		result += ".X";
		break;
	case 0x80:
		result += ".PFB";
		break;
	}
	return result;
}

int AcSensor::errorCode() const
{
	return mErrorCode;
}

void AcSensor::setErrorCode(int code)
{
	if (mErrorCode == code)
		return;
	mErrorCode = code;
	emit errorCodeChanged();
}

QString AcSensor::portName() const
{
	return mPortName;
}

int AcSensor::slaveAddress() const
{
	return mSlaveAddress;
}

QString AcSensor::serial() const
{
	return mSerial;
}

void AcSensor::setSerial(const QString &s)
{
	if (mSerial == s)
		return;
	mSerial = s;
	emit serialChanged();
}

int AcSensor::firmwareVersion() const
{
	return mFirmwareVersion;
}

void AcSensor::setFirmwareVersion(int v)
{
	if (mFirmwareVersion == v)
		return;
	mFirmwareVersion = v;
	emit firmwareVersionChanged();
}

PowerInfo *AcSensor::meanPowerInfo()
{
	return mMeanPowerInfo;
}

PowerInfo *AcSensor::l1PowerInfo()
{
	return mL1PowerInfo;
}

PowerInfo *AcSensor::l2PowerInfo()
{
	return mL2PowerInfo;
}

PowerInfo *AcSensor::l3PowerInfo()
{
	return mL3PowerInfo;
}

PowerInfo *AcSensor::getPowerInfo(Phase phase)
{
	switch (phase) {
	case MultiPhase:
		return mMeanPowerInfo;
	case PhaseL1:
		return mL1PowerInfo;
	case PhaseL2:
		return mL2PowerInfo;
	case PhaseL3:
		return mL3PowerInfo;
	default:
		QLOG_ERROR() << "Incorrect phase:" << phase;
		return 0;
	}
}

void AcSensor::resetValues()
{
	mMeanPowerInfo->resetValues();
	mL1PowerInfo->resetValues();
	mL2PowerInfo->resetValues();
	mL3PowerInfo->resetValues();
}
