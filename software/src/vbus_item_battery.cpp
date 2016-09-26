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
		createItem("/Info/MaxDischargeCurrent", SIGNAL(maxDischargeCurrentChanged())))
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
