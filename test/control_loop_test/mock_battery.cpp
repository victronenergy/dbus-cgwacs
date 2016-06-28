#include <qnumeric.h>
#include "mock_battery.h"

MockBattery::MockBattery(QObject *parent):
	Battery(parent),
	mMaxChargeCurrent(qQNaN()),
	mMaxDischargeCurrent(qQNaN()),
	mCellImbalanceAlarm(0),
	mHighChargeCurrentAlarm(0),
	mHighDischargeCurrentAlarm(0),
	mHighTemperatureAlarm(0),
	mHighVoltageAlarm(0),
	mInternalError(0),
	mLowTemperatureAlarm(0),
	mLowVoltageAlarm(0)
{
}

void MockBattery::setMaxChargeCurrent(double c)
{
	if (mMaxChargeCurrent == c)
		return;
	mMaxChargeCurrent = c;
	emit maxChargeCurrentChanged();
}

void MockBattery::setMaxDischargeCurrent(double c)
{
	if (mMaxDischargeCurrent == c)
		return;
	mMaxDischargeCurrent = c;
	emit maxDischargeCurrentChanged();
}
