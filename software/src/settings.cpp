#include "settings.h"

Settings::Settings(QObject *parent) :
	QObject(parent),
	mAcPowerSetPoint(0)
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

void Settings::registerDevice(const QString &serial)
{
	if (mDeviceIds.contains(serial))
		return;
	mDeviceIds.append(serial);
	emit deviceIdsChanged();
}
