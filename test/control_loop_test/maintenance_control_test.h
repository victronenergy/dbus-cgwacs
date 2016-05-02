#ifndef CONTROLLOOPTEST_H
#define CONTROLLOOPTEST_H

#include <QScopedPointer>
#include <gtest/gtest.h>
#include <battery_life.h>
#include <multi.h>
#include <settings.h>
#include <system_calc.h>
#include "mock_system_calc.h"

class MaintenanceControlTest : public testing::Test
{
protected:
	virtual void SetUp();

	virtual void TearDown();

	QScopedPointer<BatteryLife> mControlLoop;
	QScopedPointer<Multi> mMulti;
	QScopedPointer<MockSystemCalc> mSystemCalc;
	QScopedPointer<Settings> mSettings;
};

#endif // CONTROLLOOPTEST_H
