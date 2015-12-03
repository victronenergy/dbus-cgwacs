#include <cmath>
#include <QsLog.h>
#include "ac_sensor.h"
#include "ac_sensor_settings.h"
#include "data_processor.h"
#include "power_info.h"

DataProcessor::DataProcessor(AcSensor *acSensor,
							 AcSensorSettings *settings, QObject *parent) :
	QObject(parent),
	mAcSensor(acSensor),
	mSettings(settings)
{
	Q_ASSERT(acSensor != 0);
	Q_ASSERT(settings != 0);
	memset(mNegativePower, 0, sizeof(mNegativePower));
}

void DataProcessor::setPower(Phase phase, double value)
{
	PowerInfo *pi = mAcSensor->getPowerInfo(phase);
	double prev = pi->power();
	const double Fc = 0.25;
	const double Dt = 0.5;
	const double Alpha = Fc/(Fc + 1/(2 * M_PI * Dt));
	value = qIsNaN(prev) ? value : (1 - Alpha) * prev + Alpha * value;
	pi->setPower(value);
	if (value < 0)
		mNegativePower[phase] -= value;
}

void DataProcessor::setVoltage(Phase phase, double value)
{
	PowerInfo *pi = mAcSensor->getPowerInfo(phase);
	pi->setVoltage(value);
}

void DataProcessor::setCurrent(Phase phase, double value)
{
	PowerInfo *pi = mAcSensor->getPowerInfo(phase);
	pi->setCurrent(value);
}

void DataProcessor::setPositiveEnergy(Phase phase, double value)
{
	PowerInfo *pi = mAcSensor->getPowerInfo(phase);
	pi->setEnergyForward(value);
}

void DataProcessor::setNegativeEnergy(double sum)
{
	double e1 = getReverseEnergy(PhaseL1);
	if (!qIsFinite(e1)) {
		setInitialEnergy(PhaseL1, sum / 3);
		setInitialEnergy(PhaseL2, sum / 3);
		setInitialEnergy(PhaseL3, sum / 3);
		setReverseEnergy(MultiPhase, sum);
		memset(mNegativePower, 0, sizeof(mNegativePower));
		return;
	}
	double prevEnergy = getReverseEnergy(MultiPhase);
	double delta = sum - prevEnergy;
	if (delta <= 0)
		return;
	setReverseEnergy(MultiPhase, sum);
	double f = delta / mNegativePower[MultiPhase];
	if (qIsFinite(f)) {
		setReverseEnergy(PhaseL1, getReverseEnergy(PhaseL1) + f * mNegativePower[PhaseL1]);
		setReverseEnergy(PhaseL2, getReverseEnergy(PhaseL2) + f * mNegativePower[PhaseL2]);
		setReverseEnergy(PhaseL3, getReverseEnergy(PhaseL3) + f * mNegativePower[PhaseL3]);
	}
	memset(mNegativePower, 0, sizeof(mNegativePower));
}

void DataProcessor::setNegativeEnergy(Phase phase, double value)
{
	setReverseEnergy(phase, value);
}

void DataProcessor::updateEnergySettings()
{
	updateEnergySettings(PhaseL1);
	updateEnergySettings(PhaseL2);
	updateEnergySettings(PhaseL3);
}

void DataProcessor::updateEnergySettings(Phase phase)
{
	PowerInfo *pi = mAcSensor->getPowerInfo(phase);
	if (pi == 0)
		return;
	double e = getReverseEnergy(phase);
	if (qIsFinite(e))
		mSettings->setReverseEnergy(phase, e);
}

void DataProcessor::setInitialEnergy(Phase phase, double defaultValue)
{
	double v = mSettings->getReverseEnergy(phase);
	if (v == 0)
		v = defaultValue;
	setReverseEnergy(phase, v);
}

double DataProcessor::getReverseEnergy(Phase phase)
{
	PowerInfo *pi = mAcSensor->getPowerInfo(phase);
	if (pi == 0)
		return 0;
	return pi->energyReverse();
}

void DataProcessor::setReverseEnergy(Phase phase, double value)
{
	PowerInfo *pi = mAcSensor->getPowerInfo(phase);
	if (pi == 0)
		return;
	pi->setEnergyReverse(value);
}
