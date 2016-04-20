#include <cmath>
#include "maintenance_control_test.h"
#include "battery_life.h"
#include "multi.h"
#include "multi_phase_data.h"
#include "settings.h"

TEST_F(MaintenanceControlTest, chargeToAbsorption)
{
	mSettings->setSocLimit(20);
	mSystemCalc->setSoc(50);
	mMulti->setState(MultiStateBulk);
	EXPECT_EQ(mSettings->socLimit(), 20);
	EXPECT_EQ(mSettings->state(), BatteryLifeStateDefault);
	mSystemCalc->setSoc(85);
	mMulti->setState(MultiStateAbsorption);
	EXPECT_EQ(mSettings->socLimit(), 15);
	EXPECT_EQ(mSettings->state(), BatteryLifeStateAbsorption);
	mSystemCalc->setSoc(75);
	mMulti->setState(MultiStateBulk);
	EXPECT_EQ(mSettings->state(), BatteryLifeStateDefault);
	EXPECT_EQ(mSettings->socLimit(), 15);
	mSystemCalc->setSoc(14);
	EXPECT_EQ(mSettings->state(), BatteryLifeStateDischarged);
	EXPECT_EQ(mSettings->socLimit(), 20);
	mSystemCalc->setSoc(16);
	EXPECT_EQ(mSettings->state(), BatteryLifeStateDischarged);
	EXPECT_EQ(mSettings->socLimit(), 20);
	mSystemCalc->setSoc(26);
	EXPECT_EQ(mSettings->state(), BatteryLifeStateDefault);
	EXPECT_EQ(mSettings->socLimit(), 20);
}

TEST_F(MaintenanceControlTest, chargeToFloat)
{
	mSettings->setSocLimit(20);
	mSystemCalc->setSoc(50);
	mMulti->setState(MultiStateBulk);
	EXPECT_EQ(mSettings->socLimit(), 20);
	EXPECT_EQ(mSettings->state(), BatteryLifeStateDefault);
	mSystemCalc->setSoc(90);
	mMulti->setState(MultiStateAbsorption);
	EXPECT_EQ(mSettings->socLimit(), 15);
	EXPECT_EQ(mSettings->state(), BatteryLifeStateAbsorption);
	EXPECT_EQ(mSettings->flags(), BatteryLifeFlagAbsorption);
	mSystemCalc->setSoc(98);
	mMulti->setState(MultiStateFloat);
	EXPECT_EQ(mSettings->socLimit(), 10);
	EXPECT_EQ(mSettings->state(), BatteryLifeStateFloat);
	EXPECT_EQ(mSettings->flags(), BatteryLifeFlagAbsorption | BatteryLifeFlagFloat);
	mSystemCalc->setSoc(70);
	mMulti->setState(MultiStateBulk);
	EXPECT_EQ(mSettings->state(), BatteryLifeStateDefault);
	EXPECT_EQ(mSettings->socLimit(), 10);
	mSystemCalc->setSoc(9);
	EXPECT_EQ(mSettings->state(), BatteryLifeStateDischarged);
	EXPECT_EQ(mSettings->socLimit(), 15);
	mSystemCalc->setSoc(14);
	EXPECT_EQ(mSettings->state(), BatteryLifeStateDischarged);
	EXPECT_EQ(mSettings->socLimit(), 15);
	mSystemCalc->setSoc(21);
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
	mSystemCalc.reset(new MockSystemCalc);
	mSettings.reset(new Settings);
	mSettings->setState(BatteryLifeStateDefault);
	mControlLoop.reset(new BatteryLife(mSystemCalc.data(), mMulti.data(), mSettings.data()));
}

void MaintenanceControlTest::TearDown()
{
	mControlLoop.reset();
	mMulti.reset();
	mSettings.reset();
}
