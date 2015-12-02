#include <QsLog.h>
#include <QTimer>
#include "maintenance_control.h"
#include "multi.h"
#include "settings.h"

MaintenanceControl::MaintenanceControl(Multi *multi, Settings *settings,
									   Clock *clock, QObject *parent):
	QObject(parent),
	mMulti(multi),
	mSettings(settings),
	mClock(clock)
{
	Q_ASSERT(multi != 0);
	Q_ASSERT(settings != 0);
	if (mClock == 0)
		mClock.reset(new DefaultClock());
	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
	timer->setInterval(5000);
	timer->start();
}

void MaintenanceControl::update()
{
	if (mSettings->maintenanceInterval() == 0) {
		setHub4State(Hub4External);
	} else if (mSettings->state() == Hub4External) {
		setHub4State(Hub4SelfConsumption);
	}

	switch (mSettings->state()) {
	case Hub4SelfConsumption:
	{
		QDateTime nextCharge = mSettings->maintenanceDate();
		if (!nextCharge.isValid()) {
			updateMaintenanceDate();
			nextCharge = mSettings->maintenanceDate();
		}
		nextCharge = nextCharge.addDays(mSettings->maintenanceInterval());
		QDateTime now = mClock->now();
		if (now >= nextCharge && now.time().hour() == 12) {
			setHub4State(Hub4ChargeFromGrid);
		}

		if (isMultiCharged()) {
			setHub4State(Hub4Charged);
		}
		break;
	}
	case Hub4Charged:
		if (mMulti->state() == MultiStateBulk || mMulti->state() == MultiStateAbsorption) {
			setHub4State(Hub4SelfConsumption);
			updateMaintenanceDate();
		}
		break;
	case Hub4External:
		break;
	case Hub4ChargeFromGrid:
		if (isMultiCharged()) {
			setHub4State(Hub4SelfConsumption);
			updateMaintenanceDate();
		}
		break;
	case Hub4Storage:
		if (isMultiCharged())
			mSettings->setMaintenanceDate(QDateTime());
		break;
	}
}

void MaintenanceControl::onTimer()
{
	update();
}

bool MaintenanceControl::isMultiCharged() const
{
	return mMulti->state() == MultiStateFloat || mMulti->state() == MultiStateStorage;
}

void MaintenanceControl::updateMaintenanceDate()
{
	QDateTime nextCharge = mClock->now();
	nextCharge.setTime(QTime(1, 0, 0));
	mSettings->setMaintenanceDate(nextCharge);
	QLOG_INFO() << "Next maintenance base date set at:" << nextCharge;
}

void MaintenanceControl::setHub4State(Hub4State state)
{
	if (mSettings->state() == state)
		return;
	QLOG_INFO() << "Changing Hub4 state from"
				<< getStateName(mSettings->state())
				<< "to" << getStateName(state);
	mSettings->setState(state);
}

const char *MaintenanceControl::getStateName(int state)
{
	static const char *StateNames[] = { "SelfConsumption", "ChargeFromGrid", "Charged", "Storage", "External" };
	static const int StateNameCount	= static_cast<int>(sizeof(StateNames)/sizeof(StateNames[0]));
	if (state < 0 || state >= StateNameCount)
		return "Unknown";
	return StateNames[state];
}
