#include <qmath.h>
#include "defines.h"
#include "ac_sensor_phase.h"

AcSensorPhase::AcSensorPhase(QObject *parent) :
	QObject(parent),
	mCurrent(qQNaN()),
	mVoltage(qQNaN()),
	mPower(qQNaN()),
	mEnergyForward(qQNaN())
{
}

void AcSensorPhase::setCurrent(double c)
{
	if (valuesEqual(mCurrent, c))
		return;
	mCurrent = c;
	emit currentChanged();
}

void AcSensorPhase::setVoltage(double v)
{
	if (valuesEqual(mVoltage, v))
		return;
	mVoltage = v;
	emit voltageChanged();
}

void AcSensorPhase::setPower(double p)
{
	if (valuesEqual(mPower, p))
		return;
	mPower = p;
	emit powerChanged();
}

void AcSensorPhase::setEnergyForward(double e)
{
	if (valuesEqual(mEnergyForward, e))
		return;
	mEnergyForward = e;
	emit energyForwardChanged();
}

void AcSensorPhase::setEnergyReverse(double e)
{
	if (valuesEqual(mEnergyReverse, e))
		return;
	mEnergyReverse = e;
	emit energyReverseChanged();
}

void AcSensorPhase::resetValues()
{
	setCurrent(qQNaN());
	setPower(qQNaN());
	setVoltage(qQNaN());
	setEnergyForward(qQNaN());
	setEnergyReverse(qQNaN());
}

bool AcSensorPhase::valuesEqual(double v1, double v2)
{
	return (qIsNaN(v1) && qIsNaN(v2)) || (v1 == v2);
}
