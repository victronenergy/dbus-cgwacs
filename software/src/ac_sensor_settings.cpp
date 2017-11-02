#include <QsLog.h>
#include "ac_sensor_settings.h"

AcSensorSettings::AcSensorSettings(int deviceType, const QString &serial,
								   QObject *parent) :
	QObject(parent),
	mDeviceType(deviceType),
	mSerial(serial),
	mIsMultiPhase(false),
	mPosition(Input1),
	mDeviceInstance(-1),
	mL1Energy(0),
	mL2Energy(0),
	mL3Energy(0),
	mL2Position(Input1),
	mL2DeviceInstance(-1)
{
}

void AcSensorSettings::setCustomName(const QString &n)
{
	if (mCustomName == n)
		return;
	mCustomName	= n;
	emit customNameChanged();
}

QString AcSensorSettings::productName() const
{
	return getProductName(mServiceType, mPosition);
}

void AcSensorSettings::setServiceType(const QString &t)
{
	if (mServiceType == t)
		return;
	mServiceType = t;
	if (t != "grid")
		setL2ServiceType(QString());
	emit serviceTypeChanged();
	emit productNameChanged();
}

void AcSensorSettings::setIsMultiPhase(bool b)
{
	if (mIsMultiPhase == b)
		return;
	mIsMultiPhase = b;
	if (b)
		setL2ServiceType(QString());
	emit isMultiPhaseChanged();
}

QString AcSensorSettings::l2CustomName() const
{
	return mL2CustomName;
}

void AcSensorSettings::setL2CustomName(const QString &v)
{
	if (mL2CustomName == v)
		return;
	mL2CustomName = v;
	emit l2CustomNameChanged();
}

QString AcSensorSettings::l2ProductName() const
{
	return getProductName(mL2ServiceType, mL2Position);
}

QString AcSensorSettings::l2ServiceType() const
{
	return mL2ServiceType;
}

void AcSensorSettings::setL2ServiceType(const QString &v)
{
	if (mL2ServiceType == v)
		return;
	mL2ServiceType = v;
	if (!v.isEmpty())
		setIsMultiPhase(false);
	emit l2ServiceTypeChanged();
	emit l2ProductNameChanged();
}

Position AcSensorSettings::l2Position() const
{
	return mL2Position;
}

void AcSensorSettings::setL2Position(Position v)
{
	if (mL2Position == v)
		return;
	mL2Position = v;
	emit l2PositionChanged();
	emit l2ProductNameChanged();
}

Position AcSensorSettings::position()
{
	return mPosition;
}

void AcSensorSettings::setPosition(Position p)
{
	if (mPosition == p)
		return;
	mPosition = p;
	emit positionChanged();
	emit productNameChanged();
}

int AcSensorSettings::deviceInstance() const
{
	return mDeviceInstance;
}

void AcSensorSettings::setDeviceInstance(int d)
{
	if (mDeviceInstance == d)
		return;
	mDeviceInstance = d;
	emit deviceInstanceChanged();
}

int AcSensorSettings::l2DeviceInstance() const
{
	return mL2DeviceInstance;
}

void AcSensorSettings::setL2DeviceInstance(int d)
{
	if (mL2DeviceInstance == d)
		return;
	mL2DeviceInstance = d;
	emit l2DeviceInstanceChanged();
}

double AcSensorSettings::l1ReverseEnergy() const
{
	return mL1Energy;
}

void AcSensorSettings::setL1ReverseEnergy(double e)
{
	if (mL1Energy == e)
		return;
	mL1Energy = e;
	emit l1ReverseEnergyChanged();
}

double AcSensorSettings::l2ReverseEnergy() const
{
	return mL2Energy;
}

void AcSensorSettings::setL2ReverseEnergy(double e)
{
	if (mL2Energy == e)
		return;
	mL2Energy = e;
	emit l2ReverseEnergyChanged();
}

double AcSensorSettings::l3ReverseEnergy() const
{
	return mL3Energy;
}

void AcSensorSettings::setL3ReverseEnergy(double e)
{
	if (mL3Energy == e)
		return;
	mL3Energy = e;
	emit l3ReverseEnergyChanged();
}

double AcSensorSettings::getReverseEnergy(Phase phase) const
{
	switch (phase) {
	case PhaseL1:
		return mL1Energy;
	case PhaseL2:
		return mL2Energy;
	case PhaseL3:
		return mL3Energy;
	default:
		QLOG_ERROR() << "Incorrect phase:" << phase;
		return 0;
	}
}

void AcSensorSettings::setReverseEnergy(Phase phase, double value)
{
	switch (phase) {
	case PhaseL1:
		setL1ReverseEnergy(value);
		break;
	case PhaseL2:
		setL2ReverseEnergy(value);
		break;
	case PhaseL3:
		setL3ReverseEnergy(value);
		break;
	default:
		QLOG_ERROR() << "Incorrect phase:" << phase;
		break;
	}
}

QString AcSensorSettings::getProductName(const QString &serviceType, Position position)
{
	if (serviceType == "grid")
		return "Grid meter";
	if (serviceType == "genset")
		return "Generator";
	if (serviceType == "pvinverter") {
		QString productName = "PV inverter";
		QString p;
		switch (position) {
		case Input1:
			p = "input 1";
			break;
		case Input2:
			p = "input 2";
			break;
		case Output:
			p = "output";
			break;
		default:
			break;
		}
		if (!p.isEmpty())
			productName.append(" on ").append(p);
		return productName;
	}
	return "AC sensor";
}
