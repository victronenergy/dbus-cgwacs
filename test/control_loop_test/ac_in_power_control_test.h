#ifndef SINGLEPHASECONTROLTEST_H
#define SINGLEPHASECONTROLTEST_H

#include <QScopedPointer>
#include <gtest/gtest.h>

class AcSensor;
class AcSensorSettings;
class ControlLoop;
class Multi;
class Settings;
class SystemCalc;

class AcInPowerControlTest : public testing::Test
{
protected:
	virtual void SetUp();

	virtual void TearDown();

	void runControlLoop(double load[3], int stepCount=20);

	QScopedPointer<AcSensor> mAcSensor;
	QScopedPointer<AcSensorSettings> mAcSensorSettings;
	QScopedPointer<ControlLoop> mControlLoop;
	QScopedPointer<Multi> mMulti;
	QScopedPointer<Settings> mSettings;
	QScopedPointer<SystemCalc> mSystemCalc;
};

#endif // SINGLEPHASECONTROLTEST_H
