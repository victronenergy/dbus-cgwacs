#ifndef CONTROLLOOPTEST_H
#define CONTROLLOOPTEST_H

#include <QScopedPointer>
#include <gtest/gtest.h>

class AcSensor;
class ControlLoop;
class TestClock;
class Multi;
class Settings;

class ControlLoopTest : public testing::Test
{
protected:
	virtual void SetUp();

	virtual void TearDown();

	void triggerUpdate();

	TestClock *mClock;
	QScopedPointer<AcSensor> mAcSensor;
	QScopedPointer<ControlLoop> mControlLoop;
	QScopedPointer<Multi> mMulti;
	QScopedPointer<Settings> mSettings;
};

#endif // CONTROLLOOPTEST_H
