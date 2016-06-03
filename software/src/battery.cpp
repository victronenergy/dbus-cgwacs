#include <QsLog.h>
#include <qnumeric.h>
#include "battery.h"

Battery::Battery(QObject *parent) :
	QObject(parent),
	mMaxChargeCurrent(qQNaN()),
	mMaxDischargeCurrent(qQNaN())
{
}

double Battery::maxChargeCurrent() const
{
	return mMaxChargeCurrent;
}

void Battery::setMaxChargeCurrent(double c)
{
	if (mMaxChargeCurrent == c)
		return;
	mMaxChargeCurrent = c;
	emit maxChargeCurrentChanged();
}

double Battery::maxDischargeCurrent() const
{
	return mMaxDischargeCurrent;
}

void Battery::setMaxDischargeCurrent(double c)
{
	if (mMaxDischargeCurrent == c)
		return;
	mMaxDischargeCurrent = c;
	emit maxDischargeCurrentChanged();
}
