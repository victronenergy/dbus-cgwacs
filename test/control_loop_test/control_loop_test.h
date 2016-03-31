#ifndef CONTROLLOOPTEST_H
#define CONTROLLOOPTEST_H

#include <QScopedPointer>
#include <gtest/gtest.h>
#include <maintenance_control.h>
#include <multi.h>
#include <settings.h>

class TestClock;

class ControlLoopTest : public testing::Test
{
protected:
	virtual void SetUp();

	virtual void TearDown();

	void triggerUpdate();

	TestClock *mClock;
	QScopedPointer<MaintenanceControl> mControlLoop;
	QScopedPointer<Multi> mMulti;
	QScopedPointer<Settings> mSettings;
};

#endif // CONTROLLOOPTEST_H
