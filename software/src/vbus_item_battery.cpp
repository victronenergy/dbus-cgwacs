#include <qnumeric.h>
#include <QVariant>
#include <velib/qt/ve_qitem.hpp>
#include "vbus_item_battery.h"

VbusItemBattery::VbusItemBattery(VeQItem *service, QObject *parent):
	Battery(parent),
	mService(service),
	mMaxChargeCurrent(
		createItem("Info/MaxChargeCurrent", SIGNAL(maxChargeCurrentChanged()))),
	mMaxDischargeCurrent(
		createItem("Info/MaxDischargeCurrent", SIGNAL(maxDischargeCurrentChanged())))
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

VeQItem *VbusItemBattery::createItem(const QString &path, const char *signal)
{
	VeQItem *result = mService->itemGetOrCreate(path);
	connect(result, SIGNAL(valueChanged(VeQItem *, QVariant)), this, signal);
	result->getValue();
	return result;
}

double VbusItemBattery::getDouble(VeQItem *item)
{
	bool ok = false;
	QVariant v = item->getValue();
	double d = v.toDouble(&ok);
	return ok ? d : qQNaN();
}
