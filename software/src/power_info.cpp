#include <qmath.h>
#include "defines.h"
#include "power_info.h"

PowerInfo::PowerInfo(QObject *parent) :
	QObject(parent),
	mCurrent(NaN),
	mVoltage(NaN),
	mPower(NaN),
	mEnergyForward(NaN)
{
}

double PowerInfo::current() const
{
	return mCurrent;
}

void PowerInfo::setCurrent(double c)
{
	if (valuesEqual(mCurrent, c))
		return;
	mCurrent = c;
	emit currentChanged();
}

double PowerInfo::voltage() const
{
	return mVoltage;
}

void PowerInfo::setVoltage(double v)
{
	if (valuesEqual(mVoltage, v))
		return;
	mVoltage = v;
	emit voltageChanged();
}

double PowerInfo::power() const
{
	return mPower;
}

void PowerInfo::setPower(double p)
{
	if (valuesEqual(mPower, p))
		return;
	mPower = p;
	emit powerChanged();
}

double PowerInfo::energyForward() const
{
	return mEnergyForward;
}

void PowerInfo::setEnergyForward(double e)
{
	if (valuesEqual(mEnergyForward, e))
		return;
	mEnergyForward = e;
	emit energyForwardChanged();
}

double PowerInfo::energyReverse() const
{
	return mEnergyReverse;
}

void PowerInfo::setEnergyReverse(double e)
{
	if (valuesEqual(mEnergyReverse, e))
		return;
	mEnergyReverse = e;
	emit energyReverseChanged();
}

void PowerInfo::resetValues()
{
	setCurrent(NaN);
	setPower(NaN);
	setVoltage(NaN);
	setEnergyForward(NaN);
	setEnergyReverse(NaN);
}

bool PowerInfo::valuesEqual(double v1, double v2)
{
	return (qIsNaN(v1) && qIsNaN(v2)) || (v1 == v2);
}
