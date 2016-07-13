#include <QsLog.h>
#include <qnumeric.h>
#include "defines.h"
#include "settings.h"

Settings::Settings(QObject *parent) :
	QObject(parent),
	mAcPowerSetPoint(0),
	mMaxChargePercentage(100),
	mMaxDischargePercentage(100),
	mState(BatteryLifeStateDisabled),
	mFlags(0),
	mSocLimit(SocSwitchDefaultMin),
	mMinSocLimit(SocSwitchDefaultMin),
	mMaxChargePower(qQNaN()),
	mMaxDischargePower(qQNaN()),
	mPreventFeedback(false)
{
}

QStringList Settings::deviceIds() const
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

BatteryLifeState Settings::state() const
{
	return mState;
}

void Settings::setState(BatteryLifeState v)
{
	if (mState == v)
		return;
	mState = v;
	emit stateChanged();
}

quint8 Settings::flags() const
{
	return mFlags;
}

void Settings::setFlags(quint8 flags)
{
	if (mFlags == flags)
		return;
	mFlags = flags;
	emit flagsChanged();
}

double Settings::socLimit() const
{
	return mSocLimit;
}

void Settings::setSocLimit(double v)
{
	if (mSocLimit == v)
		return;
	mSocLimit = v;
	emit socLimitChanged();
}

double Settings::minSocLimit() const
{
	return mMinSocLimit;
}

void Settings::setMinSocLimit(double l)
{
	if (mMinSocLimit == l)
		return;
	mMinSocLimit = l;
	emit minSocLimitChanged();
}

double Settings::maxChargePower() const
{
	return mMaxChargePower;
}

void Settings::setMaxChargePower(double p)
{
	if (mMaxChargePower == p)
		return;
	mMaxChargePower = p;
	emit maxChargePowerChanged();
}

double Settings::maxDischargePower() const
{
	return mMaxDischargePower;
}

void Settings::setMaxDischargePower(double p)
{
	if (mMaxDischargePower == p)
		return;
	mMaxDischargePower = p;
	emit maxDischargePowerChanged();
}

void Settings::setPreventFeedback(bool b)
{
	if (mPreventFeedback == b)
		return;
	mPreventFeedback = b;
	emit preventFeedbackChanged();
}

QDateTime Settings::dischargedTime() const
{
	return mDischargedTime;
}

void Settings::setDischargedTime(const QDateTime &t)
{
	if (mDischargedTime == t)
		return;
	mDischargedTime = t;
	emit dischargedTimeChanged();
}

void Settings::registerDevice(const QString &serial)
{
	if (mDeviceIds.contains(serial))
		return;
	mDeviceIds.append(serial);
	emit deviceIdsChanged();
}

int Settings::getDeviceInstance(const QString &serial, bool isSecundary) const
{
	int i = mDeviceIds.indexOf(serial);
	if (i == -1)
		return InvalidDeviceInstance;
	return MinDeviceInstance + (2 * i + (isSecundary ? 1 : 0)) % (MaxDeviceInstance - MinDeviceInstance + 1);
}
