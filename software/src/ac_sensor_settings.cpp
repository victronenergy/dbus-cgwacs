#include <QsLog.h>
#include "ac_sensor_settings.h"

AcSensorSettings::AcSensorSettings(int deviceType, const QString &serial,
								   QObject *parent) :
	QObject(parent),
	mDeviceType(deviceType),
	mSerial(serial),
	mIsMultiPhase(false),
	mHub4Mode(Hub4PhaseL1),
	mPosition(Input1),
	mDeviceInstance(-1),
	mL1Energy(0),
	mL2Energy(0),
	mL3Energy(0),
	mL2Position(Input1),
	mL2DeviceInstance(-1)
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
	emit effectiveCustomNameChanged();
}

QString AcSensorSettings::productName() const
{
	return getProductName(mServiceType, mPosition);
}

QString AcSensorSettings::effectiveCustomName() const
{
	if (!mCustomName.isEmpty())
		return mCustomName;
	return productName();
}

void AcSensorSettings::setEffectiveCustomName(const QString &n)
{
	setCustomName(n == productName() ? QString() : n);
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
	if (t != "grid")
		setL2ServiceType(QString());
	emit serviceTypeChanged();
	emit productNameChanged();
	if (mCustomName.isEmpty())
		emit effectiveCustomNameChanged();
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
	emit l2EffectiveCustomNameChanged();
}

QString AcSensorSettings::l2ProductName() const
{
	return getProductName(mL2ServiceType, mL2Position);
}

QString AcSensorSettings::l2EffectiveCustomName() const
{
	if (!mL2CustomName.isEmpty())
		return mL2CustomName;
	return l2ProductName();
}

void AcSensorSettings::setL2EffectiveCustomName(const QString &n)
{
	setL2CustomName(n == l2ProductName() ? QString() : n);
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
	if (mL2CustomName.isEmpty())
		emit l2EffectiveCustomNameChanged();
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
	if (mL2CustomName.isEmpty())
		emit l2EffectiveCustomNameChanged();
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
	emit productNameChanged();
	if (mCustomName.isEmpty())
		emit effectiveCustomNameChanged();
}

int AcSensorSettings::deviceInstance() const
{
	return mDeviceInstance;
}

void AcSensorSettings::setDeviceInstance(int d)
{
	qDebug() << __FUNCTION__ << d << mSerial;
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
	qDebug() << __FUNCTION__ << d << mSerial;
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
