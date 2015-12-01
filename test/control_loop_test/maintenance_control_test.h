#ifndef CONTROLLOOPTEST_H
#define CONTROLLOOPTEST_H

#include <QScopedPointer>
#include <gtest/gtest.h>
#include <maintenance_control.h>
#include <multi.h>
#include <settings.h>

class MaintenanceControlTest : public testing::Test
{
protected:
	virtual void SetUp();

	virtual void TearDown();

	QScopedPointer<MaintenanceControl> mControlLoop;
	QScopedPointer<Multi> mMulti;
	QScopedPointer<Settings> mSettings;
};

#endif // CONTROLLOOPTEST_H
