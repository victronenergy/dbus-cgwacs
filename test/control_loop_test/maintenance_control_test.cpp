#include <cmath>
#include "maintenance_control_test.h"
#include "maintenance_control.h"
#include "multi.h"
#include "settings.h"

TEST_F(MaintenanceControlTest, chargeToAbsorption)
{
	mSettings->setSocLimit(20);
	mMulti->setSoc(50);
	mMulti->setState(MultiStateBulk);
	EXPECT_EQ(mSettings->socLimit(), 20);
	EXPECT_EQ(mSettings->state(), MaintenanceStateDefault);
	mMulti->setSoc(90);
	mMulti->setState(MultiStateAbsorption);
	EXPECT_EQ(mSettings->socLimit(), 15);
	EXPECT_EQ(mSettings->state(), MaintenanceStateAbsorption);
	mMulti->setSoc(80);
	mMulti->setState(MultiStateBulk);
	EXPECT_EQ(mSettings->state(), MaintenanceStateDefault);
	EXPECT_EQ(mSettings->socLimit(), 15);
	mMulti->setSoc(14);
	EXPECT_EQ(mSettings->state(), MaintenanceStateDischarged);
	EXPECT_EQ(mSettings->socLimit(), 20);
	mMulti->setSoc(16);
	EXPECT_EQ(mSettings->state(), MaintenanceStateDischarged);
	EXPECT_EQ(mSettings->socLimit(), 20);
	mMulti->setSoc(26);
	EXPECT_EQ(mSettings->state(), MaintenanceStateDefault);
	EXPECT_EQ(mSettings->socLimit(), 20);
}

TEST_F(MaintenanceControlTest, chargeToFloat)
{
	mSettings->setSocLimit(20);
	mMulti->setSoc(50);
	mMulti->setState(MultiStateBulk);
	EXPECT_EQ(mSettings->socLimit(), 20);
	EXPECT_EQ(mSettings->state(), MaintenanceStateDefault);
	mMulti->setSoc(90);
	mMulti->setState(MultiStateAbsorption);
	EXPECT_EQ(mSettings->socLimit(), 15);
	EXPECT_EQ(mSettings->state(), MaintenanceStateAbsorption);
	mMulti->setSoc(98);
	mMulti->setState(MultiStateFloat);
	EXPECT_EQ(mSettings->socLimit(), 10);
	EXPECT_EQ(mSettings->state(), MaintenanceStateFloat);
	mMulti->setState(MultiStateBulk);
	EXPECT_EQ(mSettings->state(), MaintenanceStateDefault);
	EXPECT_EQ(mSettings->socLimit(), 10);
	mMulti->setSoc(9);
	EXPECT_EQ(mSettings->state(), MaintenanceStateDischarged);
	EXPECT_EQ(mSettings->socLimit(), 15);
	mMulti->setSoc(14);
	EXPECT_EQ(mSettings->state(), MaintenanceStateDischarged);
	EXPECT_EQ(mSettings->socLimit(), 15);
	mMulti->setSoc(21);
	EXPECT_EQ(mSettings->state(), MaintenanceStateDefault);
	EXPECT_EQ(mSettings->socLimit(), 15);
}

void MaintenanceControlTest::SetUp()
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
	mControlLoop.reset(new MaintenanceControl(mMulti.data(), mSettings.data()));
}

void MaintenanceControlTest::TearDown()
{
	mControlLoop.reset();
	mMulti.reset();
	mSettings.reset();
}
