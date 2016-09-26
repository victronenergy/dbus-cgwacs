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

	QString serviceName() const;

private:
	VBusItem *createItem(const QString &path, const char *signal);

	static double getDouble(VBusItem *item);

	QString mServiceName;
	VBusItem *mMaxChargeCurrent;
	VBusItem *mMaxDischargeCurrent;
};

#endif // VBUS_ITEM_BATTERY_H
