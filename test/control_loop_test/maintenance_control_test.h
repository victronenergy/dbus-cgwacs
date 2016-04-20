#ifndef CONTROLLOOPTEST_H
#define CONTROLLOOPTEST_H

#include <QScopedPointer>
#include <gtest/gtest.h>
#include <battery_life.h>
#include <multi.h>
#include <settings.h>
#include <system_calc.h>

class MockSystemCalc : public SystemCalc
{
	Q_OBJECT
public:
	MockSystemCalc(QObject *parent = 0):
		SystemCalc(parent),
		mSoc(qQNaN())
	{}

	virtual double soc() const
	{
		return mSoc;
	}

	void setSoc(double s)
	{
		if (mSoc == s)
			return;
		mSoc = s;
		emit socChanged();
	}

private:
	double mSoc;
};

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
