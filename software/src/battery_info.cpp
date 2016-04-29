#include <qnumeric.h>
#include <QsLog.h>
#include "battery.h"
#include "battery_bridge.h"
#include "battery_info.h"
#include "dbus_service_monitor.h"
#include "multi.h"
#include "settings.h"

const double ChargeEfficiency = 0.95;
const double DischargeEfficiency = 0.70;

BatteryInfo::BatteryInfo(DbusServiceMonitor *serviceMonitor, Multi *multi, Settings *settings,
						 QObject *parent) :
	QObject(parent),
	mMulti(multi),
	mSettings(settings),
	mMaxChargePower(qQNaN()),
	mMaxDischargePower(qQNaN())
{
	// Q_ASSERT(serviceMonitor != 0);
	Q_ASSERT(multi != 0);
	Q_ASSERT(settings != 0);
	if (serviceMonitor != 0) {
		connect(serviceMonitor, SIGNAL(serviceAdded(QString)), this, SLOT(onServiceAdded(QString)));
		connect(serviceMonitor, SIGNAL(serviceRemoved(QString)), this, SLOT(onServiceRemoved(QString)));
	}
	connect(multi, SIGNAL(dcVoltageChanged()), this, SLOT(updateBatteryLimits()));
	connect(settings, SIGNAL(maxChargePercentageChanged()), this, SLOT(updateBatteryLimits()));
	connect(settings, SIGNAL(maxDischargePercentageChanged()), this, SLOT(updateBatteryLimits()));
}

bool BatteryInfo::canCharge() const
{
	return !qIsFinite(mMaxChargePower) || mMaxChargePower > 0;
}

bool BatteryInfo::canDischarge() const
{
	return !qIsFinite(mMaxDischargePower) || mMaxDischargePower > 0;
}

double BatteryInfo::applyLimits(double power) const
{
	if (qIsFinite(mMaxDischargePower) && -mMaxDischargePower > power)
		power = -mMaxDischargePower;
	if (qIsFinite(mMaxChargePower) && mMaxChargePower < power)
		power = mMaxChargePower;
	return power;
}

void BatteryInfo::onServiceAdded(QString service)
{
	if (!service.startsWith("com.victronenergy.battery."))
		return;
	if (mBatteries.contains(service))
		return;
	QLOG_INFO() << "Battery found @" << service;
	Battery *battery = new Battery(this);
	new BatteryBridge(service, battery, battery);
	mBatteries.insert(service, battery);
	connect(battery, SIGNAL(maxChargeCurrentChanged()),
			this, SLOT(updateBatteryLimits()));
	connect(battery, SIGNAL(maxDischargeCurrentChanged()),
			this, SLOT(updateBatteryLimits()));
}

void BatteryInfo::onServiceRemoved(QString service)
{
	if (!service.startsWith("com.victronenergy.battery."))
		return;
	QHash<QString, Battery *>::Iterator it = mBatteries.find(service);
	if (it == mBatteries.end())
		return;
	QLOG_INFO() << "Battery @" << service << "disappeared.";
	it.value()->deleteLater();
	mBatteries.erase(it);
	updateBatteryLimits();
}

void BatteryInfo::setMaxChargePower(double p)
{
	if (mMaxChargePower == p)
		return;
	mMaxChargePower = p;
	emit maxChargePowerChanged();
}

void BatteryInfo::setMaxDischargePower(double p)
{
	if (mMaxDischargePower == p)
		return;
	mMaxDischargePower = p;
	emit maxDischargePowerChanged();
}

void BatteryInfo::updateBatteryLimits()
{
	double maxChargeCurrent = 0;
	double maxDischargeCurrent = 0;
	bool hasChargeCurrent = false;
	bool hasDischargeCurrent = false;
	foreach (Battery *battery, mBatteries.values()) {
		if (qIsFinite(battery->maxChargeCurrent())) {
			maxChargeCurrent += battery->maxChargeCurrent();
			hasChargeCurrent = true;
		}
		if (qIsFinite(battery->maxDischargeCurrent())) {
			maxDischargeCurrent += battery->maxDischargeCurrent();
			hasDischargeCurrent = true;
		}
	}
	/// @todo EV Keep old values if no batteries present? If so, we'd have to store the values in
	/// localsettings??
	double maxChargePower = qQNaN();
	double maxChargePct = qBound(0.0, mSettings->maxChargePercentage(), 100.0);
	if (hasChargeCurrent) {
		maxChargePower = maxChargeCurrent * mMulti->dcVoltage() / ChargeEfficiency;
	} else if (mSettings->maxChargePercentage() <= 99) {
		// Backward compatible behaviour: only compute a max charge power if the max charge
		// percentage has been changed (100 is the default). This is done to keep systems with
		// load on AC-Out running properly.
		maxChargePower = mMulti->maxChargeCurrent() * mMulti->dcVoltage();
	}
	maxChargePower *= maxChargePct / 100;
	setMaxChargePower(maxChargePower);

	double maxDischargePower = qQNaN();
	double maxDischargePct = qBound(0.0, mSettings->maxDischargePercentage(), 100.0);
	if (hasDischargeCurrent) {
		maxDischargePower = maxDischargeCurrent * mMulti->dcVoltage() * DischargeEfficiency;
		maxDischargePower *= maxDischargePct / 100;
	} else if (maxDischargePct < 50) {
		// Backward compatible behaviour: since we have no information from the battery we have no
		// maximum charge current, so we cannot apply maxDischargePower. It was decided to use the
		// percentage as on/off switch. Again, we only change the maxDischargePower if
		// maxDischargeCurrent has been changed in order to keep load on AC-Out running.
		maxDischargePower = 0;
	}
	setMaxDischargePower(maxDischargePower);
}
