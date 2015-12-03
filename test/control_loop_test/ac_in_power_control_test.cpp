#include <ac_sensor.h>
#include <ac_sensor_settings.h>
#include <multi.h>
#include <multi_phase_data.h>
#include <power_info.h>
#include <phase_compensation_control.h>
#include <qmath.h>
#include <settings.h>
#include <single_phase_control.h>
#include "ac_in_power_control_test.h"

TEST_F(AcInPowerControlTest, singlePhaseInit)
{
	mAcSensorSettings->setIsMultiPhase(false);
	mMulti->l1Data()->setAcPowerSetPoint(0);
	SinglePhaseControl cl(mMulti.data(), mAcSensor.data(), mSettings.data(), PhaseL1, Hub4PhaseL1);
	double load[3] = { 100.0, NaN, NaN };
	runControlLoop(load);
	EXPECT_NEAR(mAcSensor->l1PowerInfo()->power(), 0, 1);
}

TEST_F(AcInPowerControlTest, singlePhaseStep)
{
	mAcSensorSettings->setIsMultiPhase(false);
	mMulti->l1Data()->setAcPowerSetPoint(0);
	SinglePhaseControl cl(mMulti.data(), mAcSensor.data(), mSettings.data(), PhaseL1, Hub4PhaseL1);
	double load[3] = { 100.0, NaN, NaN };
	runControlLoop(load);
	double load2[3] = { 200.0, NaN, NaN };
	runControlLoop(load2);
	EXPECT_NEAR(mAcSensor->l1PowerInfo()->power(), 0, 1);
}

TEST_F(AcInPowerControlTest, splitPhase3Init)
{
	mAcSensorSettings->setIsMultiPhase(true);
	mAcSensorSettings->setHub4Mode(Hub4PhaseSplit);
	mMulti->l1Data()->setAcPowerSetPoint(0);
	mMulti->l2Data()->setAcPowerSetPoint(0);
	mMulti->l3Data()->setAcPowerSetPoint(0);
	SinglePhaseControl cl1(mMulti.data(), mAcSensor.data(), mSettings.data(), PhaseL1, Hub4PhaseSplit);
	SinglePhaseControl cl2(mMulti.data(), mAcSensor.data(), mSettings.data(), PhaseL2, Hub4PhaseSplit);
	SinglePhaseControl cl3(mMulti.data(), mAcSensor.data(), mSettings.data(), PhaseL3, Hub4PhaseSplit);
	double load[3] = { 100.0, -200, 150 };
	runControlLoop(load);
	EXPECT_NEAR(mAcSensor->l1PowerInfo()->power(), 0, 1);
	EXPECT_NEAR(mAcSensor->l2PowerInfo()->power(), 0, 1);
	EXPECT_NEAR(mAcSensor->l3PowerInfo()->power(), 0, 1);
}

TEST_F(AcInPowerControlTest, splitPhase3Step)
{
	mAcSensorSettings->setIsMultiPhase(true);
	mAcSensorSettings->setHub4Mode(Hub4PhaseSplit);
	mMulti->l1Data()->setAcPowerSetPoint(0);
	mMulti->l2Data()->setAcPowerSetPoint(0);
	mMulti->l3Data()->setAcPowerSetPoint(0);
	SinglePhaseControl cl1(mMulti.data(), mAcSensor.data(), mSettings.data(), PhaseL1, Hub4PhaseSplit);
	SinglePhaseControl cl2(mMulti.data(), mAcSensor.data(), mSettings.data(), PhaseL2, Hub4PhaseSplit);
	SinglePhaseControl cl3(mMulti.data(), mAcSensor.data(), mSettings.data(), PhaseL3, Hub4PhaseSplit);
	double load[3] = { 100.0, -200, 150 };
	runControlLoop(load, 2);
	double load2[3] = { -150, -100, 200 };
	runControlLoop(load2);
	EXPECT_NEAR(mAcSensor->l1PowerInfo()->power(), 0, 1);
	EXPECT_NEAR(mAcSensor->l2PowerInfo()->power(), 0, 1);
	EXPECT_NEAR(mAcSensor->l3PowerInfo()->power(), 0, 1);
}

TEST_F(AcInPowerControlTest, splitPhase2Init)
{
	for (int i=0; i<3; ++i) {
		mAcSensorSettings->setIsMultiPhase(true);
		mAcSensorSettings->setHub4Mode(Hub4PhaseSplit);
		for (int j=0; j<3; ++j) {
			Phase phase = static_cast<Phase>(PhaseL1 + j);
			mMulti->getPhaseData(phase)->setAcPowerSetPoint(i == j ? NaN : 0);
		}
		SinglePhaseControl cl1(mMulti.data(), mAcSensor.data(), mSettings.data(), PhaseL1, Hub4PhaseSplit);
		SinglePhaseControl cl2(mMulti.data(), mAcSensor.data(), mSettings.data(), PhaseL2, Hub4PhaseSplit);
		SinglePhaseControl cl3(mMulti.data(), mAcSensor.data(), mSettings.data(), PhaseL3, Hub4PhaseSplit);
		double load[3] = { 100.0, -200, 150 };
		runControlLoop(load);
		for (int j=0; j<3; ++j) {
			Phase phase = static_cast<Phase>(PhaseL1 + j);
			double power = mAcSensor->getPowerInfo(phase)->power();
			if (i != j)
				EXPECT_NEAR(power, 0, 1);
		}
	}
}

TEST_F(AcInPowerControlTest, phaseCompensation1Phase)
{
	mAcSensorSettings->setIsMultiPhase(true);
	mAcSensorSettings->setHub4Mode(Hub4PhaseCompensation);
	mMulti->l1Data()->setAcPowerSetPoint(0);
	SinglePhaseControl cl1(mMulti.data(), mAcSensor.data(), mSettings.data(), PhaseL1, Hub4PhaseCompensation);
	double load[3] = { 100.0, -200, 150 };
	runControlLoop(load);
	EXPECT_NEAR(mAcSensor->l1PowerInfo()->power(), 50, 1);
}

TEST_F(AcInPowerControlTest, phaseCompensation2Phase)
{
	mAcSensorSettings->setIsMultiPhase(true);
	mAcSensorSettings->setHub4Mode(Hub4PhaseCompensation);
	mMulti->l1Data()->setAcPowerSetPoint(0);
	mMulti->l2Data()->setAcPowerSetPoint(0);
	PhaseCompensationControl cl(mMulti.data(), mAcSensor.data(), mSettings.data());
	double load[3] = { -300, 750, 200 };
	runControlLoop(load);
	EXPECT_NEAR(mAcSensor->l1PowerInfo()->power(), -300, 1);
	EXPECT_NEAR(mAcSensor->l2PowerInfo()->power(), 100, 1);
}

TEST_F(AcInPowerControlTest, phaseCompensation3Phase)
{
	mAcSensorSettings->setIsMultiPhase(true);
	mAcSensorSettings->setHub4Mode(Hub4PhaseCompensation);
	mMulti->l1Data()->setAcPowerSetPoint(0);
	mMulti->l2Data()->setAcPowerSetPoint(0);
	mMulti->l3Data()->setAcPowerSetPoint(0);
	PhaseCompensationControl cl(mMulti.data(), mAcSensor.data(), mSettings.data());
	double load[3] = { 600, -100, 200 };
	runControlLoop(load);
	EXPECT_NEAR(mAcSensor->l1PowerInfo()->power(), 0, 1);
	EXPECT_NEAR(mAcSensor->l2PowerInfo()->power(), -100, 1);
	EXPECT_NEAR(mAcSensor->l3PowerInfo()->power(), 100, 1);
}

TEST_F(AcInPowerControlTest, phaseCompensation3PhasePositive)
{
	mAcSensorSettings->setIsMultiPhase(true);
	mAcSensorSettings->setHub4Mode(Hub4PhaseCompensation);
	mMulti->l1Data()->setAcPowerSetPoint(0);
	mMulti->l2Data()->setAcPowerSetPoint(0);
	mMulti->l3Data()->setAcPowerSetPoint(0);
	PhaseCompensationControl cl(mMulti.data(), mAcSensor.data(), mSettings.data());
	double load[3] = { 300, 400, 200 };
	runControlLoop(load);
	EXPECT_NEAR(mAcSensor->l1PowerInfo()->power(), 0, 1);
	EXPECT_NEAR(mAcSensor->l2PowerInfo()->power(), 0, 1);
	EXPECT_NEAR(mAcSensor->l3PowerInfo()->power(), 0, 1);
}

void AcInPowerControlTest::SetUp()
{
	mAcSensor.reset(new AcSensor("", 0));
	mAcSensorSettings.reset(new AcSensorSettings(71, ""));
	mMulti.reset(new Multi);
	mSettings.reset(new Settings);
}

void AcInPowerControlTest::TearDown()
{
	mAcSensor.reset();
	mAcSensorSettings.reset();
	mMulti.reset();
	mSettings.reset();
}

void AcInPowerControlTest::runControlLoop(double load[3], int stepCount)
{
	double pLoad = 0;
	for (int i=0; i<3; ++i)
	{
		if (qIsFinite(load[i]))
			pLoad += load[i];
	}
	for (int i=0; i<stepCount; ++i) {
		double pT = 0;
		for (int j=0; j<3; ++j) {
			Phase phase = static_cast<Phase>(PhaseL1 + j);
			MultiPhaseData *mpd = mMulti->getPhaseData(phase);
			double setpoint = mpd->acPowerSetPoint();
			if (qIsFinite(setpoint)) {
				setpoint = qRound(setpoint);
				pT += setpoint;
				mpd->setAcPowerIn(setpoint);
				PowerInfo *pi = mAcSensor->getPowerInfo(phase);
				pi->setPower(load[j] + setpoint);
			}
		}
		mMulti->meanData()->setAcPowerIn(pT);
		mAcSensor->meanPowerInfo()->setPower(pLoad + pT);
	}
}
