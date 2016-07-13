#include <qnumeric.h>
#include <QVariant>
#include <velib/qt/v_busitem.h>
#include "vbus_item_battery.h"

VbusItemBattery::VbusItemBattery(const QString &service, QObject *parent):
	Battery(parent),
	mServiceName(service),
	mMaxChargeCurrent(
		createItem("/Info/MaxChargeCurrent", SIGNAL(maxChargeCurrentChanged()))),
	mMaxDischargeCurrent(
		createItem("/Info/MaxDischargeCurrent", SIGNAL(maxDischargeCurrentChanged()))),
	mCellImbalanceAlarm(
		createItem("/Alarms/CellImbalance", SIGNAL(cellImbalanceAlarmChanged()))),
	mHighChargeCurrentAlarm(
		createItem("/Alarms/HighChargeCurrent", SIGNAL(highChargeCurrentAlarmChanged()))),
	mHighDischargeCurrentAlarm(
		createItem("/Alarms/HighDischargeCurrent", SIGNAL(highDischargeCurrentAlarmChanged()))),
	mHighTemperatureAlarm(
		createItem("/Alarms/HighTemperature", SIGNAL(highTemperatureAlarmChanged()))),
	mHighVoltageAlarm(
		createItem("/Alarms/HighVoltage", SIGNAL(highVoltageAlarmChanged()))),
	mInternalError(
		createItem("/Alarms/InternalFailure", SIGNAL(internalErrorChanged()))),
	mLowTemperatureAlarm(
		createItem("/Alarms/LowTemperature", SIGNAL(lowTemperatureAlarmChanged()))),
	mLowVoltageAlarm(
		createItem("/Alarms/LowVoltage", SIGNAL(lowVoltageAlarmChanged())))
{
}

double VbusItemBattery::maxChargeCurrent() const
{
	return getDouble(mMaxChargeCurrent);
}

double VbusItemBattery::maxDischargeCurrent() const
{
	return getDouble(mMaxDischargeCurrent);
}

int VbusItemBattery::cellImbalanceAlarm() const
{
	return mCellImbalanceAlarm->getValue().toInt();
}

int VbusItemBattery::highChargeCurrentAlarm() const
{
	return mHighChargeCurrentAlarm->getValue().toInt();
}

int VbusItemBattery::highDischargeCurrentAlarm() const
{
	return mHighDischargeCurrentAlarm->getValue().toInt();
}

int VbusItemBattery::highTemperatureAlarm() const
{
	return mHighTemperatureAlarm->getValue().toInt();
}

int VbusItemBattery::highVoltageAlarm() const
{
	return mHighVoltageAlarm->getValue().toInt();
}

int VbusItemBattery::lowTemperatureAlarm() const
{
	return mLowTemperatureAlarm->getValue().toInt();
}

int VbusItemBattery::lowVoltageAlarm() const
{
	return mLowVoltageAlarm->getValue().toInt();
}

int VbusItemBattery::internalError() const
{
	return mInternalError->getValue().toInt();
}

QString VbusItemBattery::serviceName() const
{
	return mServiceName;
}

VBusItem *VbusItemBattery::createItem(const QString &path, const char *signal)
{
	VBusItem *result = new VBusItem(this);
	connect(result, SIGNAL(valueChanged()), this, signal);
	result->consume(mServiceName, path);
	result->getValue();
	return result;
}

double VbusItemBattery::getDouble(VBusItem *item)
{
	bool ok = false;
	QVariant v = item->getValue();
	double d = v.toDouble(&ok);
	return ok ? d : qQNaN();
}
