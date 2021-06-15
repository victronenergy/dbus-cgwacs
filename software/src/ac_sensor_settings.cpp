#include <QStringList>
#include <QsLog.h>
#include "ac_sensor_settings.h"

AcSensorSettings::AcSensorSettings(int deviceType, const QString &serial,
								   QObject *parent) :
	QObject(parent),
	mDeviceType(deviceType),
	mSerial(serial),
	mIsMultiPhase(false),
	mPiggyEnabled(false),
	mPosition(Input1),
	mL1Energy(0),
	mL2Energy(0),
	mL3Energy(0),
	mL2Position(Input1)
{
}

bool AcSensorSettings::supportMultiphase() const
{
	return (mDeviceType >= 71 && mDeviceType <= 73) ||
		(mDeviceType >= 340 && mDeviceType <= 345);
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
	return getProductName(serviceType(), mPosition);
}

QString AcSensorSettings::serviceType() const
{
	return mClassAndVrmInstance.split(":").first();
}

void AcSensorSettings::setServiceType(const QString &t)
{
	setClassAndVrmInstance(
		QString("%1:%2").arg(t).arg(deviceInstance()));
}

void AcSensorSettings::setIsMultiPhase(bool b)
{
	if (mIsMultiPhase == b)
		return;
	mIsMultiPhase = b;
	if (b)
		setPiggyEnabled(false);
	emit isMultiPhaseChanged();
}

void AcSensorSettings::setPiggyEnabled(bool b)
{
	if (mPiggyEnabled == b)
		return;
	mPiggyEnabled = b;
	if (b)
		setIsMultiPhase(false);
	emit piggyEnabledChanged();
}

const QString AcSensorSettings::l2CustomName() const
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

const QString AcSensorSettings::l2ProductName() const
{
	return getProductName(l2ServiceType(), mL2Position);
}

const QString AcSensorSettings::l2ServiceType() const
{
	return mL2ClassAndVrmInstance.split(":").first();
}

void AcSensorSettings::setL2ServiceType(const QString &t)
{
	setL2ClassAndVrmInstance(
		QString("%1:%2").arg(t).arg(l2DeviceInstance()));
}

void AcSensorSettings::setClassAndVrmInstance(const QString &s)
{
	if (mClassAndVrmInstance == s)
		return;
	mClassAndVrmInstance = s;
	emit productNameChanged();
	emit classAndVrmInstanceChanged();
}

void AcSensorSettings::setL2ClassAndVrmInstance(const QString &s)
{
	if (mL2ClassAndVrmInstance == s)
		return;
	mL2ClassAndVrmInstance = s;
	emit l2ClassAndVrmInstanceChanged();
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
	bool ok;
	int di = mClassAndVrmInstance.split(":").last().toInt(&ok);
	if (ok)
		return di;
	return -1;
}

int AcSensorSettings::l2DeviceInstance() const
{
	bool ok;
	int di = mL2ClassAndVrmInstance.split(":").last().toInt(&ok);
	if (ok)
		return di;
	return -1;
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
	if (serviceType == "acmeter")
		return "AC meter";
	return "AC sensor";
}
