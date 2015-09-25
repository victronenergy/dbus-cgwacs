#include "settings.h"

Settings::Settings(QObject *parent) :
	QObject(parent),
	mAcPowerSetPoint(0),
	mMaxChargePercentage(100),
	mMaxDischargePercentage(100),
	mState(Hub4SelfConsumption),
	mMaintenanceInterval(7)
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

int Settings::maintenanceInterval() const
{
	return mMaintenanceInterval;
}

void Settings::setMaintenanceInterval(int v)
{
	if (mMaintenanceInterval == v)
		return;
	mMaintenanceInterval = v;
	emit maintenanceIntervalChanged();
}

int Settings::state() const
{
	return mState;
}

void Settings::setState(int v)
{
	if (mState == v)
		return;
	mState = v;
	emit stateChanged();
}

QDateTime Settings::maintenanceDate() const
{
	return mMaintenanceDate;
}

void Settings::setMaintenanceDate(const QDateTime &v)
{
	if (mMaintenanceDate == v)
		return;
	mMaintenanceDate = v;
	emit maintenanceDateChanged();
}

void Settings::registerDevice(const QString &serial)
{
	if (mDeviceIds.contains(serial))
		return;
	mDeviceIds.append(serial);
	emit deviceIdsChanged();
}
