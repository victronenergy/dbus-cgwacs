#ifndef VBUS_ITEM_BATTERY_H
#define VBUS_ITEM_BATTERY_H

#include "battery.h"

class VBusItem;

class VbusItemBattery : public Battery
{
public:
	VbusItemBattery(const QString &service, QObject *parent = 0);

	virtual double maxChargeCurrent() const;

	virtual double maxDischargeCurrent() const;

	virtual int cellImbalanceAlarm() const;

	virtual int highChargeCurrentAlarm() const;

	virtual int highDischargeCurrentAlarm() const;

	virtual int highTemperatureAlarm() const;

	virtual int highVoltageAlarm() const;

	virtual int internalError() const;

	virtual int lowTemperatureAlarm() const;

	virtual int lowVoltageAlarm() const;

	QString serviceName() const;

private:
	VBusItem *createItem(const QString &path, const char *signal);

	static double getDouble(VBusItem *item);

	QString mServiceName;
	VBusItem *mMaxChargeCurrent;
	VBusItem *mMaxDischargeCurrent;
	VBusItem *mCellImbalanceAlarm;
	VBusItem *mHighChargeCurrentAlarm;
	VBusItem *mHighDischargeCurrentAlarm;
	VBusItem *mHighTemperatureAlarm;
	VBusItem *mHighVoltageAlarm;
	VBusItem *mInternalError;
	VBusItem *mLowTemperatureAlarm;
	VBusItem *mLowVoltageAlarm;
};

#endif // VBUS_ITEM_BATTERY_H
