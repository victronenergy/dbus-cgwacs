#include "settings.h"

Settings::Settings(QObject *parent) :
	QObject(parent),
	mAcPowerSetPoint(0),
	mMaxChargePercentage(100),
	mMaxDischargePercentage(100)
{
}

const QStringList &Settings::deviceIds() const
{
	return mDeviceIds;
}

void Settings::setDeviceIds(const QStringList &deviceIds)
{
	if (mDeviceIds == deviceIds)
		return;
	mDeviceIds = deviceIds;
	emit deviceIdsChanged();
}

double Settings::acPowerSetPoint() const
{
	return mAcPowerSetPoint;
}

void Settings::setAcPowerSetPoint(double p)
{
	if (mAcPowerSetPoint == p)
		return;
	mAcPowerSetPoint = p;
	emit acPowerSetPointChanged();
}

double Settings::maxChargePercentage() const
{
	return mMaxChargePercentage;
}

void Settings::setMaxChargePercentage(double p)
{
	p = qBound(0.0, p, 100.0);
	if (mMaxChargePercentage == p)
		return;
	mMaxChargePercentage = p;
	emit maxChargePercentageChanged();
}

double Settings::maxDischargePercentage() const
{
	return mMaxDischargePercentage;
}

void Settings::setMaxDischargePercentage(double p)
{
	p = qBound(0.0, p, 100.0);
	if (mMaxDischargePercentage == p)
		return;
	mMaxDischargePercentage = p;
	emit maxDischargePercentageChanged();
}

void Settings::registerDevice(const QString &serial)
{
	if (mDeviceIds.contains(serial))
		return;
	mDeviceIds.append(serial);
	emit deviceIdsChanged();
}
