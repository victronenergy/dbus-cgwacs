#include <cmath>
#include "control_loop_test.h"
#include "maintenance_control.h"
#include "multi.h"
#include "settings.h"

class TestClock : public Clock
{
public:
	virtual QDateTime now() const
	{
		return mTime;
	}

	void setTime(const QDateTime &time)
	{
		mTime = time;
	}

	void setHours(int h)
	{
		QTime time = mTime.time();
		time.setHMS(h, 0, 0);
		QDate date = mTime.date();
		mTime = QDateTime(date, time);
	}

	void addSecs(int s)
	{
		mTime = mTime.addSecs(s);
	}

	void addDays(int d)
	{
		mTime = mTime.addDays(d);
	}

private:
	QDateTime mTime;
};

TEST_F(ControlLoopTest, defaultStartup)
{
	triggerUpdate();
	EXPECT_EQ(mSettings->maintenanceDate(), mClock->now());
	EXPECT_EQ(mSettings->maintenanceInterval(), 7);
	EXPECT_EQ(mSettings->state(), Hub4SelfConsumption);
	// EXPECT_LT(mMulti->acPowerSetPoint(), 0);
}

TEST_F(ControlLoopTest, autoChargeLoop)
{
	// Test the loop where the batteries are not fully charged during self
	// consumption. At the maintenance date, forced charging should start.
	triggerUpdate();
	EXPECT_EQ(mSettings->maintenanceDate(), mClock->now());
	EXPECT_EQ(mSettings->state(), Hub4SelfConsumption);
	mClock->addDays(4);
	mClock->setHours(12);
	triggerUpdate();
	EXPECT_EQ(mSettings->state(), Hub4SelfConsumption);
	mClock->addDays(3);
	mClock->setHours(0);
	triggerUpdate();
	EXPECT_EQ(mSettings->state(), Hub4SelfConsumption);
	mClock->setHours(12);
	triggerUpdate();
	EXPECT_EQ(mSettings->state(), Hub4ChargeFromGrid);
	triggerUpdate();
	// EXPECT_GE(mMulti->acPowerSetPoint(), 12 * 90);
	mClock->addDays(1);
	mClock->setHours(1);
	triggerUpdate();
	EXPECT_EQ(mSettings->state(), Hub4ChargeFromGrid);
	triggerUpdate();
	// EXPECT_GE(mMulti->acPowerSetPoint(), 12 * 90);
	mMulti->setState(MultiStateFloat);
	// mMulti->setStateOfCharge(100);
	triggerUpdate();
	EXPECT_EQ(mSettings->state(), Hub4SelfConsumption);
	mMulti->setState(MultiStateBulk);
	// mMulti->setStateOfCharge(90);
	mClock->addDays(7);
	mClock->setHours(12);
	triggerUpdate();
	EXPECT_EQ(mSettings->state(), Hub4ChargeFromGrid);
}

TEST_F(ControlLoopTest, chargedToSelfConsumption)
{
	// The state changes when battery is fully charged during self consumption.
	// The date of maintenance should be shifted further into the future each
	// time discharging starts after the battery has been fully charged.
	triggerUpdate();
	EXPECT_EQ(mSettings->maintenanceDate(), mClock->now());
	EXPECT_EQ(mSettings->state(), Hub4SelfConsumption);
	mClock->addDays(1);
	mMulti->setState(MultiStateFloat);
	// mMulti->setStateOfCharge(100);
	triggerUpdate();
	EXPECT_EQ(mSettings->state(), Hub4Charged);
	EXPECT_EQ(mSettings->maintenanceDate(), mClock->now().addDays(-1));

	mClock->addDays(1);
	// mMulti->setStateOfCharge(95);
	mMulti->setState(MultiStateAbsorption);
	triggerUpdate();
	EXPECT_EQ(mSettings->state(), Hub4SelfConsumption);
	EXPECT_EQ(mSettings->maintenanceDate(), mClock->now());

	mClock->addDays(1);
	// mMulti->setStateOfCharge(100);
	mMulti->setState(MultiStateFloat);
	triggerUpdate();
	EXPECT_EQ(mSettings->state(), Hub4Charged);
	EXPECT_EQ(mSettings->maintenanceDate(), mClock->now().addDays(-1));

	mClock->addDays(1);
	mMulti->setState(MultiStateAbsorption);
	triggerUpdate();
	EXPECT_EQ(mSettings->state(), Hub4SelfConsumption);
	EXPECT_EQ(mSettings->maintenanceDate(), mClock->now());
}

TEST_F(ControlLoopTest, storageFullCharge)
{
	// Check storage mode. We should not get to another mode unless the state
	// is changed manually.
	triggerUpdate();
	EXPECT_EQ(mSettings->maintenanceDate(), mClock->now());
	EXPECT_EQ(mSettings->state(), Hub4SelfConsumption);
	mSettings->setState(Hub4Storage);
	triggerUpdate();
	EXPECT_EQ(mSettings->state(), Hub4Storage);
	mClock->addDays(200);
	mMulti->setState(MultiStateStorage);
	// mMulti->setStateOfCharge(100);
	triggerUpdate();
	mSettings->setState(Hub4SelfConsumption);
	triggerUpdate();
	EXPECT_EQ(mSettings->state(), Hub4Charged);
	EXPECT_EQ(mSettings->maintenanceDate(), mClock->now());
}

TEST_F(ControlLoopTest, storageNoFullCharge)
{
	// Check if the maintenance date is not changed when enabling storage mode
	// for a short time (before the battery is fully charged).
	triggerUpdate();
	QDateTime md = mClock->now();
	EXPECT_EQ(mSettings->maintenanceDate(), md);
	EXPECT_EQ(mSettings->state(), Hub4SelfConsumption);
	mSettings->setState(Hub4Storage);
	triggerUpdate();
	EXPECT_EQ(mSettings->state(), Hub4Storage);
	mClock->addDays(1);
	triggerUpdate();
	mSettings->setState(Hub4SelfConsumption);
	triggerUpdate();
	EXPECT_EQ(mSettings->state(), Hub4SelfConsumption);
	EXPECT_EQ(mSettings->maintenanceDate(), md);
}

TEST_F(ControlLoopTest, restartSelfConsumption)
{
	// Simulate a restart of the control loop. Before shutdown state was
	// 'self consumption'.
	mSettings->setMaintenanceInterval(20);
	mSettings->setMaintenanceDate(mClock->now().addDays(-5));
	mSettings->setState(Hub4SelfConsumption);
	triggerUpdate();
	EXPECT_EQ(mSettings->state(), Hub4SelfConsumption);
	EXPECT_EQ(mSettings->maintenanceDate(), mClock->now().addDays(-5));
	// Also check if we start charging when the maintenance date has been
	// reached.
	mClock->addDays(15);
	mClock->setHours(12);
	triggerUpdate();
	EXPECT_EQ(mSettings->state(), Hub4ChargeFromGrid);
}

TEST_F(ControlLoopTest, adjustInterval)
{
	// Test the loop where the batteries are not fully charged during self
	// consumption. At the maintenance date, forced charging should start.
	triggerUpdate();
	EXPECT_EQ(mSettings->maintenanceDate(), mClock->now());
	EXPECT_EQ(mSettings->state(), Hub4SelfConsumption);

	mSettings->setMaintenanceInterval(30);
	mClock->addDays(7);
	mClock->setHours(12);
	triggerUpdate();
	EXPECT_EQ(mSettings->state(), Hub4SelfConsumption);
	triggerUpdate();
	mClock->addDays(23);
	mClock->setHours(12);
	triggerUpdate();
	EXPECT_EQ(mSettings->state(), Hub4ChargeFromGrid);
	triggerUpdate();
	EXPECT_EQ(mSettings->state(), Hub4ChargeFromGrid);
	// EXPECT_GE(mMulti->acPowerSetPoint(), 12 * 90);
}

TEST_F(ControlLoopTest, restartChargeFromGrid)
{
	// Simulate a restart of the control loop. Before shutdown state was
	// 'charge from grid'.
	mSettings->setMaintenanceDate(mClock->now().addSecs(-3600 * 12));
	mSettings->setState(Hub4ChargeFromGrid);
	triggerUpdate();
	EXPECT_EQ(mSettings->state(), Hub4ChargeFromGrid);
	// EXPECT_GE(mMulti->acPowerSetPoint(), 12 * 90);
	mMulti->setState(MultiStateFloat);
	// mMulti->setStateOfCharge(100);
	// Check if we return to self consumption are charge has been completed.
	triggerUpdate();
	EXPECT_EQ(mSettings->state(), Hub4SelfConsumption);
}

TEST_F(ControlLoopTest, restartStorageFromGrid)
{
	// Simulate a restart of the control loop. Before shutdown state was
	// 'storage'.
	QDateTime md = mClock->now().addSecs(-3600 * 12);
	mSettings->setMaintenanceDate(md);
	mSettings->setState(Hub4Storage);
	triggerUpdate();
	EXPECT_EQ(md, mSettings->maintenanceDate());
	// EXPECT_GE(mMulti->acPowerSetPoint(), 12 * 90);
	EXPECT_EQ(mSettings->state(), Hub4Storage);
	mMulti->setState(MultiStateFloat);
	// mMulti->setStateOfCharge(100);
	triggerUpdate();
	EXPECT_FALSE(mSettings->maintenanceDate().isValid());
	EXPECT_EQ(mSettings->state(), Hub4Storage);
	mSettings->setState(Hub4Storage);
	mClock->addDays(1);
	triggerUpdate();
	EXPECT_FALSE(mSettings->maintenanceDate().isValid());
	EXPECT_EQ(mSettings->state(), Hub4Storage);
}

TEST_F(ControlLoopTest, externalControl)
{
	// Make sure we don't end up in self consumption or charge mode when we are
	// manually put in HubExternal mode.
	triggerUpdate();
	EXPECT_EQ(mSettings->maintenanceDate(), mClock->now());
	EXPECT_EQ(mSettings->state(), Hub4SelfConsumption);
	mSettings->setMaintenanceInterval(0);
	triggerUpdate();
	EXPECT_EQ(mSettings->state(), Hub4External);
	mMulti->setState(MultiStateFloat);
	// mMulti->setStateOfCharge(100);
	mClock->addDays(1);
	triggerUpdate();
	EXPECT_EQ(mSettings->state(), Hub4External);
	mMulti->setState(MultiStateAbsorption);
	// mMulti->setStateOfCharge(80);
	mClock->addDays(1);
	mClock->addDays(100);
	triggerUpdate();
	EXPECT_EQ(mSettings->state(), Hub4External);
	mSettings->setMaintenanceInterval(7);
	triggerUpdate();
	EXPECT_EQ(mSettings->state(), Hub4SelfConsumption);
	mClock->addDays(8);
	mClock->setHours(12);
	triggerUpdate();
	EXPECT_EQ(mSettings->state(), Hub4ChargeFromGrid);
}

void ControlLoopTest::SetUp()
{
	mMulti.reset(new Multi);
	mMulti->setDcVoltage(12);
	mMulti->setAcPowerSetPoint(0);
	mMulti->setMaxChargeCurrent(90);
	mMulti->setMode(MultiOn);
	mMulti->setIsChargeDisabled(false);
	mMulti->setIsFeedbackDisabled(false);
	mMulti->setState(MultiStateBulk);
	mSettings.reset(new Settings);
	mClock = new TestClock();
	mClock->setTime(QDateTime::currentDateTime());
	mClock->setHours(1);
	mControlLoop.reset(new MaintenanceControl(mMulti.data(), mSettings.data(), mClock));
}

void ControlLoopTest::TearDown()
{
	mControlLoop.reset();
	mMulti.reset();
	mSettings.reset();
}

void ControlLoopTest::triggerUpdate()
{
	mControlLoop->update();
}
