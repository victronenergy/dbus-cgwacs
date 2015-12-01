#include <cmath>
#include "maintenance_control_test.h"
#include "battery_life.h"
#include "multi.h"
#include "multi_phase_data.h"
#include "settings.h"

TEST_F(MaintenanceControlTest, chargeToAbsorption)
{
	mSettings->setSocLimit(20);
	mMulti->setSoc(50);
	mMulti->setState(MultiStateBulk);
	EXPECT_EQ(mSettings->socLimit(), 20);
	EXPECT_EQ(mSettings->state(), BatteryLifeStateDefault);
	mMulti->setSoc(90);
	mMulti->setState(MultiStateAbsorption);
	EXPECT_EQ(mSettings->socLimit(), 15);
	EXPECT_EQ(mSettings->state(), BatteryLifeStateAbsorption);
	mMulti->setSoc(80);
	mMulti->setState(MultiStateBulk);
	EXPECT_EQ(mSettings->state(), BatteryLifeStateDefault);
	EXPECT_EQ(mSettings->socLimit(), 15);
	mMulti->setSoc(14);
	EXPECT_EQ(mSettings->state(), BatteryLifeStateDischarged);
	EXPECT_EQ(mSettings->socLimit(), 20);
	mMulti->setSoc(16);
	EXPECT_EQ(mSettings->state(), BatteryLifeStateDischarged);
	EXPECT_EQ(mSettings->socLimit(), 20);
	mMulti->setSoc(26);
	EXPECT_EQ(mSettings->state(), BatteryLifeStateDefault);
	EXPECT_EQ(mSettings->socLimit(), 20);
}

TEST_F(MaintenanceControlTest, chargeToFloat)
{
	mSettings->setSocLimit(20);
	mMulti->setSoc(50);
	mMulti->setState(MultiStateBulk);
	EXPECT_EQ(mSettings->socLimit(), 20);
	EXPECT_EQ(mSettings->state(), BatteryLifeStateDefault);
	mMulti->setSoc(90);
	mMulti->setState(MultiStateAbsorption);
	EXPECT_EQ(mSettings->socLimit(), 15);
	EXPECT_EQ(mSettings->state(), BatteryLifeStateAbsorption);
	mMulti->setSoc(98);
	mMulti->setState(MultiStateFloat);
	EXPECT_EQ(mSettings->socLimit(), 10);
	EXPECT_EQ(mSettings->state(), BatteryLifeStateFloat);
	mMulti->setState(MultiStateBulk);
	EXPECT_EQ(mSettings->state(), BatteryLifeStateDefault);
	EXPECT_EQ(mSettings->socLimit(), 10);
	mMulti->setSoc(9);
	EXPECT_EQ(mSettings->state(), BatteryLifeStateDischarged);
	EXPECT_EQ(mSettings->socLimit(), 15);
	mMulti->setSoc(14);
	EXPECT_EQ(mSettings->state(), BatteryLifeStateDischarged);
	EXPECT_EQ(mSettings->socLimit(), 15);
	mMulti->setSoc(21);
	EXPECT_EQ(mSettings->state(), BatteryLifeStateDefault);
	EXPECT_EQ(mSettings->socLimit(), 15);
}

void MaintenanceControlTest::SetUp()
{
	mMulti.reset(new Multi);
	mMulti->setDcVoltage(12);
	mMulti->l1Data()->setAcPowerSetPoint(0);
	mMulti->setMaxChargeCurrent(90);
	mMulti->setMode(MultiOn);
	mMulti->setIsChargeDisabled(false);
	mMulti->setIsFeedbackDisabled(false);
	mMulti->setState(MultiStateBulk);
	mSettings.reset(new Settings);
	mControlLoop.reset(new BatteryLife(0, mMulti.data(), mSettings.data()));
}

void MaintenanceControlTest::TearDown()
{
	mControlLoop.reset();
	mMulti.reset();
	mSettings.reset();
}
