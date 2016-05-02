#include <QsLog.h>
#include <QTimer>
#include <qmath.h>
#include "dbus_service_monitor.h"
#include "defines.h"
#include "battery_life.h"
#include "multi.h"
#include "settings.h"
#include "system_calc.h"

class Guard
{
public:
	Guard(bool *busy):
		mBusy(busy)
	{
		Q_ASSERT(!*mBusy);
		*mBusy = true;
	}

	~Guard()
	{
		*mBusy = false;
	}

private:
	bool *mBusy;
};

static const double AbsorptionLevel = 85;
static const double FloatLevel = 95;
static const QString SystemCalcService = "com.victronenergy.system";
static const QString SocPath = "/Dc/Battery/Soc";

BatteryLife::BatteryLife(SystemCalc *systemCalc, Multi *multi,
						 Settings *settings, Clock *clock, QObject *parent):
	QObject(parent),
	mSystemCalc(systemCalc),
	mMulti(multi),
	mSettings(settings),
	mClock(clock == 0 ? new DefaultClock() : clock),
	mUpdateBusy(false)
{
	Q_ASSERT(systemCalc != 0);
	Q_ASSERT(multi != 0);
	Q_ASSERT(settings != 0);
	connect(mSystemCalc, SIGNAL(socChanged()), this, SLOT(update()));
	connect(mMulti, SIGNAL(isSustainActiveChanged()), this, SLOT(update()));
	connect(mSettings, SIGNAL(socLimitChanged()), this, SLOT(update()));
	connect(mSettings, SIGNAL(stateChanged()), this, SLOT(update()));
	connect(mSettings, SIGNAL(minSocLimitChanged()), this, SLOT(update()));

	QTimer *chargeTimer = new QTimer(this);
	chargeTimer->setInterval(SocInterval);
	connect(chargeTimer, SIGNAL(timeout()), this, SLOT(onChargeTimer()));

	update();
	chargeTimer->start();
}

void BatteryLife::onChargeTimer()
{
	QDateTime now = mClock->now();
	if (QTime(0, 0, 0).msecsTo(now.time()) < SocInterval)
		mSettings->setFlags(0);
	if (mSettings->state() == BatteryLifeStateDischarged) {
		QDateTime dischargeTime = mSettings->dischargedTime();
		if (dischargeTime.isValid())
		{
			if (dischargeTime.msecsTo(now) >= ForceChargeInterval) {
				adjustSocLimit(SocSwitchIncrement);
				setState(BatteryLifeStateForceCharge);
			}
		} else {
			// This should only happen if the discharge time in the	settings is changed externally
			// when the state is 'discharged'.
			mSettings->setDischargedTime(mClock->now());
		}
	}
}

void BatteryLife::update()
{
	if (mUpdateBusy)
		return;
	Guard guard(&mUpdateBusy);
	double soc = mSystemCalc->soc();
	if (!qIsFinite(soc)) {
		QLOG_TRACE() << "[Battery life] No active battery or no valid SoC";
		return;
	}
	double socLimit = mSettings->socLimit();
	double minSocLimit = mSettings->minSocLimit();
	if (socLimit < minSocLimit) {
		socLimit = minSocLimit;
		mSettings->setSocLimit(socLimit);
		QLOG_INFO() << "[Battery life] Setting SoC limit to" << socLimit;
	}
	BatteryLifeState state = mSettings->state();
	QLOG_TRACE() << "[Battery life]" << soc << socLimit << getStateName(state);
	bool adjustLimit = true;
	switch (mSettings->state()) {
	case BatteryLifeStateDisabled:
		break;
	case BatteryLifeStateRestart:
		adjustLimit = false;
		// Fall through
	case BatteryLifeStateDefault:
		// Switch off check
		if (mMulti->isSustainActive() || soc < socLimit) {
			onDischarged(adjustLimit);
		} else if (soc >= FloatLevel) {
			onFloat(adjustLimit);
		} else if (soc >= AbsorptionLevel) {
			onAbsorption(adjustLimit);
		} else {
			setState(BatteryLifeStateDefault);
		}
		break;
	case BatteryLifeStateDischarged:
		// Switch on check
		if (!mMulti->isSustainActive() && soc > mSettings->socLimit() + SocSwitchOffset)
			setState(BatteryLifeStateDefault);
		break;
	case BatteryLifeStateForceCharge:
		if (!mMulti->isSustainActive() && soc > mSettings->socLimit()) {
			mSettings->setDischargedTime(mClock->now());
			setState(BatteryLifeStateDischarged);
		}
		break;
	case BatteryLifeStateAbsorption:
		if (soc >= FloatLevel)
			onFloat(adjustLimit);
		else if (soc < AbsorptionLevel - SocSwitchOffset)
			setState(BatteryLifeStateDefault);
		break;
	case BatteryLifeStateFloat:
		if (soc < FloatLevel - SocSwitchOffset)
			setState(BatteryLifeStateDefault);
		break;
	default:
		setState(BatteryLifeStateDefault);
		break;
	}
}

void BatteryLife::onDischarged(bool adjustLimit)
{
	if (adjustLimit) {
		if ((mSettings->flags() & BatteryLifeFlagDischarged) == 0) {
			mSettings->setFlags(mSettings->flags() | BatteryLifeFlagDischarged);
			adjustSocLimit(SocSwitchIncrement);
		}
		mSettings->setDischargedTime(mClock->now());
	}
	setState(BatteryLifeStateDischarged);
}

void BatteryLife::onAbsorption(bool adjustLimit)
{
	if (adjustLimit && (mSettings->flags() & BatteryLifeFlagAbsorption) == 0) {
		mSettings->setFlags(mSettings->flags() | BatteryLifeFlagAbsorption);
		adjustSocLimit(-SocSwitchIncrement);
	}
	setState(BatteryLifeStateAbsorption);
}

void BatteryLife::onFloat(bool adjustLimit)
{
	if (adjustLimit && (mSettings->flags() & BatteryLifeFlagFloat) == 0) {
		mSettings->setFlags(mSettings->flags() | BatteryLifeFlagFloat);
		adjustSocLimit(-SocSwitchIncrement);
	}
	setState(BatteryLifeStateFloat);
}

void BatteryLife::adjustSocLimit(double delta)
{
	double socLimit = mSettings->socLimit() + delta;
	socLimit = qBound(mSettings->minSocLimit(), socLimit, SocSwitchMax);
	mSettings->setSocLimit(socLimit);
	QLOG_INFO() << "[Battery life] Setting SoC limit to" << socLimit;
}

void BatteryLife::setState(BatteryLifeState state)
{
	if (mSettings->state() == state)
		return;
	QLOG_INFO() << "[Battery life] Changing state from"
				<< getStateName(mSettings->state())
				<< "to" << getStateName(state);
	mSettings->setState(state);
}

const char *BatteryLife::getStateName(BatteryLifeState state)
{
	static const char *StateNames[] = { "Disabled", "Restart", "Default", "Absorption", "Float",
										"Discharged", "ForceCharge" };
	static const int StateNameCount = static_cast<int>(sizeof(StateNames)/sizeof(StateNames[0]));
	if (state < 0 || state >= StateNameCount)
		return "Unknown";
	return StateNames[state];
}
