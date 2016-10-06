#ifndef VBUS_ITEM_BATTERY_H
#define VBUS_ITEM_BATTERY_H

#include "battery.h"

class VeQItem;

class VbusItemBattery : public Battery
{
public:
	explicit VbusItemBattery(VeQItem *service, QObject *parent = 0);

	virtual double maxChargeCurrent() const;

	virtual double maxDischargeCurrent() const;

	VeQItem *service() const
	{
		return mService;
	}

private:
	VeQItem *createItem(const QString &path, const char *signal);

	static double getDouble(VeQItem *item);

	VeQItem *mService;
	VeQItem *mMaxChargeCurrent;
	VeQItem *mMaxDischargeCurrent;
};

#endif // VBUS_ITEM_BATTERY_H
