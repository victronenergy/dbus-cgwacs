#include <qnumeric.h>
#include <QsLog.h>
#include "battery.h"
#include "battery_bridge.h"
#include "battery_info.h"
#include "dbus_service_monitor.h"
#include "multi.h"
#include "settings.h"

const double ChargeEfficiency = 0.97;
const double DischargeEfficiency = 0.90;
const double DischargeOffset = 7;

BatteryInfo::BatteryInfo(Multi *multi, Settings *settings, QObject *parent) :
	QObject(parent),
	mMulti(multi),
	mSettings(settings),
	mMaxChargePower(qQNaN()),
	mMaxDischargePower(qQNaN()),
	mHasChargeCurrent(false),
	mHasDischargeCurrent(false)
{
	Q_ASSERT(multi != 0);
	Q_ASSERT(settings != 0);
	connect(multi, SIGNAL(dcVoltageChanged()), this, SLOT(updateBatteryLimits()));
	connect(settings, SIGNAL(maxChargePercentageChanged()), this, SLOT(updateBatteryLimits()));
	connect(settings, SIGNAL(maxDischargePercentageChanged()), this, SLOT(updateBatteryLimits()));
	connect(settings, SIGNAL(maxChargePowerChanged()), this, SLOT(updateBatteryLimits()));
	connect(settings, SIGNAL(maxDischargePowerChanged()), this, SLOT(updateBatteryLimits()));
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

void BatteryInfo::addBattery(Battery *battery)
{
	if (mBatteries.contains(battery))
		return;
	mBatteries.append(battery);
	updateBatteryLimits();
	connect(battery, SIGNAL(maxChargeCurrentChanged()), this, SLOT(updateBatteryLimits()));
	connect(battery, SIGNAL(maxDischargeCurrentChanged()), this, SLOT(updateBatteryLimits()));
	connect(battery, SIGNAL(cellImbalanceAlarmChanged()), this, SLOT(updateBatteryLimits()));
	connect(battery, SIGNAL(highChargeCurrentAlarmChanged()), this, SLOT(updateBatteryLimits()));
	connect(battery, SIGNAL(highDischargeCurrentAlarmChanged()), this, SLOT(updateBatteryLimits()));
	connect(battery, SIGNAL(highTemperatureAlarmChanged()), this, SLOT(updateBatteryLimits()));
	connect(battery, SIGNAL(highVoltageAlarmChanged()), this, SLOT(updateBatteryLimits()));
	connect(battery, SIGNAL(lowTemperatureAlarmChanged()), this, SLOT(updateBatteryLimits()));
	connect(battery, SIGNAL(lowVoltageAlarmChanged()), this, SLOT(updateBatteryLimits()));
	connect(battery, SIGNAL(internalErrorChanged()), this, SLOT(updateBatteryLimits()));
}

void BatteryInfo::removeBattery(Battery *battery)
{
	disconnect(battery);
	mBatteries.removeOne(battery);
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

bool BatteryInfo::isAlarmActive(const Battery *b)
{
	// low voltage alarm is excluded here, because charge should be possible on low voltage
	return
		b->cellImbalanceAlarm() == 2 || b->highChargeCurrentAlarm() == 2 ||
		b->highDischargeCurrentAlarm() == 2 || b->highTemperatureAlarm() == 2 ||
		b->highVoltageAlarm() == 2 ||
		b->lowTemperatureAlarm() == 2 || b->internalError() == 2;
}

void BatteryInfo::updateBatteryLimits()
{
	// Set max charge/discharge current to 0, so they will still be zero if no battery service is
	// present right now, but was present earlier on.
	// The result is that charge/discharge is not allowed when a battery service (with BMS
	// functionality) was present, but disappears later.
	double maxChargeCurrent = 0;
	double maxDischargeCurrent = 0;
	// hasChargeAlarm/hasDischargeAlarm is only set to true if there are alarms on a battery with
	// built in BMS (LG/bmz).
	bool hasChargeAlarm = false;
	bool hasDischargeAlarm = false;
	foreach (Battery *battery, mBatteries) {
		if (qIsFinite(battery->maxChargeCurrent())) {
			hasChargeAlarm = hasChargeAlarm || isAlarmActive(battery);
			maxChargeCurrent += battery->maxChargeCurrent();
			mHasChargeCurrent = true;
		}
		if (qIsFinite(battery->maxDischargeCurrent())) {
			hasDischargeAlarm = hasDischargeAlarm || isAlarmActive(battery) ||
				battery->lowVoltageAlarm() == 2;
			maxDischargeCurrent += battery->maxDischargeCurrent();
			mHasDischargeCurrent = true;
		}
	}

	double maxChargePower = qQNaN();
	if (hasChargeAlarm) {
		maxChargePower = 0;
	} else if (mHasChargeCurrent) {
		maxChargePower = maxChargeCurrent * mMulti->dcVoltage() / ChargeEfficiency;
	} else if (mSettings->maxChargePercentage() <= 99) {
		// Backward compatible behaviour: only compute a max charge power if the max charge
		// percentage has been changed (100 is the default). This is done to keep systems with
		// load on AC-Out running properly.
		maxChargePower = mMulti->maxChargeCurrent() * mMulti->dcVoltage();
	}
	double maxChargePct = qBound(0.0, mSettings->maxChargePercentage(), 100.0);
	maxChargePower *= maxChargePct / 100;
	double userMaxChargePower = mSettings->maxChargePower();
	if (qIsFinite(userMaxChargePower) &&
		(!qIsFinite(maxChargePower) || userMaxChargePower < maxChargePower)) {
		maxChargePower = userMaxChargePower;
	}
	setMaxChargePower(maxChargePower);

	double maxDischargePower = qQNaN();
	double maxDischargePct = qBound(0.0, mSettings->maxDischargePercentage(), 100.0);
	if (hasDischargeAlarm) {
		maxDischargePower = 0;
	} else if (mHasDischargeCurrent) {
		maxDischargePower = qMax(0.0, (maxDischargeCurrent - DischargeOffset) * mMulti->dcVoltage() * DischargeEfficiency);
		maxDischargePower *= maxDischargePct / 100;
	} else if (maxDischargePct < 50) {
		// Backward compatible behaviour: since we have no information from the battery we have no
		// maximum charge current, so we cannot apply maxDischargePower. It was decided to use the
		// percentage as on/off switch. Again, we only change the maxDischargePower if
		// maxDischargeCurrent has been changed in order to keep load on AC-Out running.
		maxDischargePower = 0;
	}
	double userMaxDischargePower = mSettings->maxDischargePower();
	if (qIsFinite(userMaxDischargePower) &&
		(!qIsFinite(maxDischargePower) || userMaxDischargePower < maxDischargePower)) {
		maxDischargePower = userMaxDischargePower;
	}
	setMaxDischargePower(maxDischargePower);
}
