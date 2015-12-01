#include <QsLog.h>
#include <QTimer>
#include <qmath.h>
#include <velib/qt/v_busitem.h>
#include "dbus_service_monitor.h"
#include "defines.h"
#include "maintenance_control.h"
#include "multi.h"
#include "settings.h"

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

MaintenanceControl::MaintenanceControl(DbusServiceMonitor *serviceMonitor, Multi *multi,
									   Settings *settings, Clock *clock, QObject *parent):
	QObject(parent),
	mMulti(multi),
	mSettings(settings),
	mSystemSoc(new VBusItem(this)),
	mClock(clock == 0 ? new DefaultClock() : clock),
	mUpdateBusy(false)
{
	Q_ASSERT(serviceMonitor != 0);
	Q_ASSERT(multi != 0);
	Q_ASSERT(settings != 0);
	connect(serviceMonitor, SIGNAL(serviceAdded(QString)), this, SLOT(onServiceAdded(QString)));
	connect(mMulti, SIGNAL(isSustainActiveChanged()), this, SLOT(update()));
	connect(mSettings, SIGNAL(socLimitChanged()), this, SLOT(update()));
	connect(mSettings, SIGNAL(stateChanged()), this, SLOT(update()));
	connect(mSettings, SIGNAL(minSocLimitChanged()), this, SLOT(update()));

	mSystemSoc->consume(SystemCalcService, SocPath);
	mSystemSoc->getValue();
	connect(mSystemSoc, SIGNAL(valueChanged()), this, SLOT(update()));

	QTimer *chargeTimer = new QTimer(this);
	chargeTimer->setInterval(SocInterval);
	connect(chargeTimer, SIGNAL(timeout()), this, SLOT(onChargeTimer()));

	update();
	chargeTimer->start();
}

void MaintenanceControl::onChargeTimer()
{
	QDateTime now = mClock->now();
	if (QTime(0, 0, 0).msecsTo(now.time()) < SocInterval)
		mSettings->setFlags(0);
	if (mSettings->state() == MaintenanceStateDischarged) {
		QDateTime dischargeTime = mSettings->dischargedTime();
		if (dischargeTime.isValid())
		{
			if (dischargeTime.msecsTo(now) >= ForceChargeInterval) {
				adjustSocLimit(SocSwitchIncrement);
				setState(MaintenanceStateForceCharge);
			}
		} else {
			// This should only happen if the discharge time in the	settings is changed externally
			// when the state is 'discharged'.
			mSettings->setDischargedTime(mClock->now());
		}
	}
}

void MaintenanceControl::update()
{
	if (mUpdateBusy)
		return;
	Guard guard(&mUpdateBusy);
	QVariant v = mSystemSoc->getValue();
	if (!v.isValid()) {
		QLOG_TRACE() << "[Battery life] No active battery or no valid SoC";
		return;
	}
	double soc = v.toDouble();
	double socLimit = mSettings->socLimit();
	double minSocLimit = mSettings->minSocLimit();
	if (socLimit < minSocLimit) {
		socLimit = minSocLimit;
		mSettings->setSocLimit(socLimit);
		QLOG_INFO() << "[Battery life] Setting SoC limit to" << socLimit;
	}
	MaintenanceState state = mSettings->state();
	QLOG_TRACE() << "[Battery life]" << soc << socLimit << getStateName(state);
	bool adjustLimit = true;
	switch (mSettings->state()) {
	case MaintenanceStateDisabled:
		break;
	case MaintenanceStateRestart:
		adjustLimit = false;
		// Fall through
	case MaintenanceStateDefault:
		// Switch off check
		if (mMulti->isSustainActive() || soc < socLimit) {
			onDischarged(adjustLimit);
		} else if (soc >= AbsorptionLevel) {
			onAbsorption(adjustLimit);
		} else if (soc >= FloatLevel) {
			onFloat(adjustLimit);
		} else {
			setState(MaintenanceStateDefault);
		}
		break;
	case MaintenanceStateDischarged:
		// Switch on check
		if (!mMulti->isSustainActive() && soc > mSettings->socLimit() + SocSwitchOffset)
			setState(MaintenanceStateDefault);
		break;
	case MaintenanceStateForceCharge:
		if (!mMulti->isSustainActive() && soc > mSettings->socLimit()) {
			mSettings->setDischargedTime(mClock->now());
			setState(MaintenanceStateDischarged);
		}
		break;
	case MaintenanceStateAbsorption:
		if (soc >= FloatLevel)
			onFloat(adjustLimit);
		else if (soc < AbsorptionLevel - SocSwitchOffset)
			setState(MaintenanceStateDefault);
		break;
	case MaintenanceStateFloat:
		if (soc < FloatLevel - SocSwitchOffset)
			setState(MaintenanceStateDefault);
		break;
	default:
		setState(MaintenanceStateDefault);
		break;
	}
}

void MaintenanceControl::onServiceAdded(QString service)
{
	if (service == SystemCalcService)
		mSystemSoc->consume(SystemCalcService, SocPath);
}

void MaintenanceControl::onDischarged(bool adjustLimit)
{
	if (adjustLimit) {
		if ((mSettings->flags() & MaintenanceFlagDischarged) == 0) {
			mSettings->setFlags(mSettings->flags() | MaintenanceFlagDischarged);
			adjustSocLimit(SocSwitchIncrement);
		}
		mSettings->setDischargedTime(mClock->now());
	}
	setState(MaintenanceStateDischarged);
}

void MaintenanceControl::onAbsorption(bool adjustLimit)
{
	if (adjustLimit && (mSettings->flags() & MaintenanceFlagAbsorption) == 0) {
		mSettings->setFlags(mSettings->flags() | MaintenanceFlagAbsorption);
		adjustSocLimit(-SocSwitchIncrement);
	}
	setState(MaintenanceStateAbsorption);
}

void MaintenanceControl::onFloat(bool adjustLimit)
{
	if (adjustLimit && (mSettings->flags() & MaintenanceFlagFloat) == 0) {
		mSettings->setFlags(mSettings->flags() | MaintenanceFlagFloat);
		adjustSocLimit(-SocSwitchIncrement);
	}
	setState(MaintenanceStateFloat);
}

void MaintenanceControl::adjustSocLimit(double delta)
{
	double socLimit = mSettings->socLimit() + delta;
	socLimit = qBound(mSettings->minSocLimit(), socLimit, SocSwitchMax);
	mSettings->setSocLimit(socLimit);
	QLOG_INFO() << "[Battery life] Setting SoC limit to" << socLimit;
}

void MaintenanceControl::setState(MaintenanceState state)
{
	if (mSettings->state() == state)
		return;
	QLOG_INFO() << "[Battery life] Changing maintenance state from"
				<< getStateName(mSettings->state())
				<< "to" << getStateName(state);
	mSettings->setState(state);
}

const char *MaintenanceControl::getStateName(MaintenanceState state)
{
	static const char *StateNames[] = { "Disabled", "Restart", "Default", "Absorption", "Float",
										"Discharged", "ForceCharge" };
	static const int StateNameCount = static_cast<int>(sizeof(StateNames)/sizeof(StateNames[0]));
	if (state < 0 || state >= StateNameCount)
		return "Unknown";
	return StateNames[state];
}
