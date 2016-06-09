#include <battery_info.h>
#include <mock_battery.h>
#include <multi.h>
#include <settings.h>
#include "battery_info_test.h"

TEST_F(BatteryInfoTest, noBattery)
{
	ASSERT_TRUE(qIsNaN(mBatteryInfo->maxChargePower()));
	ASSERT_TRUE(qIsNaN(mBatteryInfo->maxDischargePower()));
}

TEST_F(BatteryInfoTest, noBatteryChargeLimit)
{
	mSettings->setMaxChargePower(123);
	ASSERT_EQ(mBatteryInfo->maxChargePower(), 123);
	ASSERT_TRUE(qIsNaN(mBatteryInfo->maxDischargePower()));
}

TEST_F(BatteryInfoTest, noBatteryDischargeLimit)
{
	mSettings->setMaxDischargePower(534);
	ASSERT_TRUE(qIsNaN(mBatteryInfo->maxChargePower()));
	ASSERT_EQ(mBatteryInfo->maxDischargePower(), 534);
}

TEST_F(BatteryInfoTest, noBatteryLimits)
{
	mSettings->setMaxDischargePower(3231);
	mSettings->setMaxChargePower(121);
	ASSERT_EQ(mBatteryInfo->maxChargePower(), 121);
	ASSERT_EQ(mBatteryInfo->maxDischargePower(), 3231);
}

TEST_F(BatteryInfoTest, batteryLimitsCharge)
{
	mMulti->setDcVoltage(12);
	mBattery->setMaxDischargeCurrent(50);
	mBatteryInfo->addBattery(mBattery.data());
	ASSERT_TRUE(qIsNaN(mBatteryInfo->maxChargePower()));
	ASSERT_EQ(mBatteryInfo->maxDischargePower(), 12 * (50 - 7) * 0.9);
}

TEST_F(BatteryInfoTest, chargePct)
{
	mSettings->setMaxChargePercentage(70);
	ASSERT_EQ(mBatteryInfo->maxChargePower(), 12 * 45 * 0.70);
	ASSERT_TRUE(qIsNaN(mBatteryInfo->maxDischargePower()));
}

TEST_F(BatteryInfoTest, dischargePctHigh)
{
	mSettings->setMaxDischargePercentage(70);
	ASSERT_TRUE(qIsNaN(mBatteryInfo->maxChargePower()));
	ASSERT_TRUE(qIsNaN(mBatteryInfo->maxDischargePower()));
}

TEST_F(BatteryInfoTest, dischargePctLow)
{
	mSettings->setMaxDischargePercentage(30);
	ASSERT_TRUE(qIsNaN(mBatteryInfo->maxChargePower()));
	ASSERT_EQ(mBatteryInfo->maxDischargePower(), 0);
}

TEST_F(BatteryInfoTest, chargePctLimits1)
{
	mSettings->setMaxChargePercentage(30);
	mSettings->setMaxChargePower(500);
	ASSERT_NEAR(mBatteryInfo->maxChargePower(), 0.3 * 12 * 45, 1e-8);
	ASSERT_TRUE(qIsNaN(mBatteryInfo->maxDischargePower()));
}

TEST_F(BatteryInfoTest, chargePctLimits2)
{
	mSettings->setMaxChargePercentage(30);
	mSettings->setMaxChargePower(100);
	ASSERT_NEAR(mBatteryInfo->maxChargePower(), 100, 1e-8);
	ASSERT_TRUE(qIsNaN(mBatteryInfo->maxDischargePower()));
}

TEST_F(BatteryInfoTest, dischargePctLimitsHigh)
{
	mSettings->setMaxDischargePercentage(85);
	mSettings->setMaxDischargePower(500);
	ASSERT_TRUE(qIsNaN(mBatteryInfo->maxChargePower()));
	ASSERT_EQ(mBatteryInfo->maxDischargePower(), 500);
}

TEST_F(BatteryInfoTest, dischargePctLimitsLow)
{
	mSettings->setMaxDischargePercentage(35);
	mSettings->setMaxDischargePower(500);
	ASSERT_TRUE(qIsNaN(mBatteryInfo->maxChargePower()));
	ASSERT_EQ(mBatteryInfo->maxDischargePower(), 0);
}

TEST_F(BatteryInfoTest, batteryChargeLimit1)
{
	mBatteryInfo->addBattery(mBattery.data());
	mBattery->setMaxChargeCurrent(30);
	mSettings->setMaxChargePower(500);
	ASSERT_EQ(mBatteryInfo->maxChargePower(), 30 * 12 / 0.97);
	ASSERT_TRUE(qIsNaN(mBatteryInfo->maxDischargePower()));
}

TEST_F(BatteryInfoTest, batteryChargeLimit2)
{
	mBatteryInfo->addBattery(mBattery.data());
	mBattery->setMaxChargeCurrent(30);
	mSettings->setMaxChargePower(200);
	ASSERT_EQ(mBatteryInfo->maxChargePower(), 200);
	ASSERT_TRUE(qIsNaN(mBatteryInfo->maxDischargePower()));
}

TEST_F(BatteryInfoTest, batteryDischargeLimit1)
{
	mBatteryInfo->addBattery(mBattery.data());
	mBattery->setMaxDischargeCurrent(30);
	mSettings->setMaxDischargePower(500);
	ASSERT_EQ(mBatteryInfo->maxDischargePower(), (30 - 7) * 12 * 0.9);
	ASSERT_TRUE(qIsNaN(mBatteryInfo->maxChargePower()));
}

TEST_F(BatteryInfoTest, batteryDischargeLimit2)
{
	mBatteryInfo->addBattery(mBattery.data());
	mBattery->setMaxDischargeCurrent(30);
	mSettings->setMaxDischargePower(200);
	ASSERT_EQ(mBatteryInfo->maxDischargePower(), 200);
	ASSERT_TRUE(qIsNaN(mBatteryInfo->maxChargePower()));
}

TEST_F(BatteryInfoTest, alltogether)
{
	mBatteryInfo->addBattery(mBattery.data());

	mBattery->setMaxChargeCurrent(55);
	mSettings->setMaxChargePower(200);
	mSettings->setMaxChargePercentage(25);

	mBattery->setMaxDischargeCurrent(35);
	mSettings->setMaxDischargePower(250);
	mSettings->setMaxDischargePercentage(75);

	ASSERT_EQ(mBatteryInfo->maxChargePower(), qMin(200.0, 0.25 * 55 * 12 / 0.97));
	ASSERT_EQ(mBatteryInfo->maxDischargePower(), qMin(250.0, 0.75 * (35-7) * 12 * 0.9));
}

TEST_F(BatteryInfoTest, bmsDisappears)
{
	mBatteryInfo->addBattery(mBattery.data());
	mBattery->setMaxChargeCurrent(55);
	mBattery->setMaxDischargeCurrent(35);

	ASSERT_EQ(mBatteryInfo->maxChargePower(), 55 * 12 / 0.97);
	ASSERT_EQ(mBatteryInfo->maxDischargePower(), (35 - 7) * 12 * 0.9);

	mBatteryInfo->removeBattery(mBattery.data());

	ASSERT_EQ(mBatteryInfo->maxChargePower(), 0);
	ASSERT_EQ(mBatteryInfo->maxDischargePower(), 0);

	mBattery->setMaxChargeCurrent(65);
	mBattery->setMaxDischargeCurrent(40);
	mBatteryInfo->addBattery(mBattery.data());

	ASSERT_EQ(mBatteryInfo->maxChargePower(), 65 * 12 / 0.97);
	ASSERT_EQ(mBatteryInfo->maxDischargePower(), (40 - 7) * 12 * 0.9);
}

TEST_F(BatteryInfoTest, noBmsBatteryDisappears)
{
	mBatteryInfo->addBattery(mBattery.data());

	ASSERT_TRUE(qIsNaN(mBatteryInfo->maxChargePower()));
	ASSERT_TRUE(qIsNaN(mBatteryInfo->maxDischargePower()));

	mBatteryInfo->removeBattery(mBattery.data());

	ASSERT_TRUE(qIsNaN(mBatteryInfo->maxChargePower()));
	ASSERT_TRUE(qIsNaN(mBatteryInfo->maxDischargePower()));

	mBatteryInfo->addBattery(mBattery.data());

	ASSERT_TRUE(qIsNaN(mBatteryInfo->maxChargePower()));
	ASSERT_TRUE(qIsNaN(mBatteryInfo->maxDischargePower()));
}

void BatteryInfoTest::SetUp()
{
	mBattery.reset(new MockBattery);
	mMulti.reset(new Multi);
	mSettings.reset(new Settings);
	mBatteryInfo.reset(new BatteryInfo(mMulti.data(), mSettings.data()));
	mMulti->setDcVoltage(12);
	mMulti->setMaxChargeCurrent(45);
}

void BatteryInfoTest::TearDown()
{
	mBattery.reset();
	mMulti.reset();
	mSettings.reset();
	mBatteryInfo.reset();
}
