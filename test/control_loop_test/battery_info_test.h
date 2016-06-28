#ifndef BATTERYINFOTEST_H
#define BATTERYINFOTEST_H

#include <QScopedPointer>
#include <gtest/gtest.h>

class BatteryInfo;
class MockBattery;
class Multi;
class Settings;

class BatteryInfoTest : public testing::Test
{
protected:
	virtual void SetUp();

	virtual void TearDown();

	QScopedPointer<BatteryInfo> mBatteryInfo;
	QScopedPointer<MockBattery> mBattery;
	QScopedPointer<Multi> mMulti;
	QScopedPointer<Settings> mSettings;
};

#endif // BATTERYINFOTEST_H
