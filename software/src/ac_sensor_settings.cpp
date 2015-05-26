#include <QsLog.h>
#include "ac_sensor_settings.h"

AcSensorSettings::AcSensorSettings(int deviceType, const QString &serial,
										 QObject *parent) :
	QObject(parent),
	mDeviceType(deviceType),
	mSerial(serial),
	mHub4Mode(Hub4PhaseL1),
	mPosition(Input1),
	mL1Energy(0),
	mL2Energy(0),
	mL3Energy(0)
{
}

int AcSensorSettings::deviceType() const
{
	return mDeviceType;
}

QString AcSensorSettings::serial() const
{
	return mSerial;
}

QString AcSensorSettings::customName() const
{
	return mCustomName;
}

void AcSensorSettings::setCustomName(const QString &n)
{
	if (mCustomName == n)
		return;
	mCustomName	= n;
	emit customNameChanged();
}

QString AcSensorSettings::serviceType() const
{
	return mServiceType;
}

void AcSensorSettings::setServiceType(const QString &t)
{
	if (mServiceType == t)
		return;
	mServiceType = t;
	emit serviceTypeChanged();
}

bool AcSensorSettings::isMultiPhase() const
{
	return mIsMultiPhase;
}

void AcSensorSettings::setIsMultiPhase(bool b)
{
	if (mIsMultiPhase == b)
		return;
	mIsMultiPhase = b;
	emit isMultiPhaseChanged();
}

Hub4Mode AcSensorSettings::hub4Mode() const
{
	return mHub4Mode;
}

void AcSensorSettings::setHub4Mode(Hub4Mode m)
{
	if (mHub4Mode == m)
		return;
	mHub4Mode = m;
	emit hub4ModeChanged();
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
