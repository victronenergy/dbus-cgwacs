#ifndef DATA_PROCESSOR_H
#define DATA_PROCESSOR_H

#include <QObject>
#include "defines.h"

class AcSensor;
class AcSensorSettings;

/*!
 * Processes energy meter data from an and stores it in an `AcSensor` object.
 * This class will also compute values needed for the D-Bus service which cannot
 * be obtained from the energy meter directly.
 */
class DataProcessor : public QObject
{
	Q_OBJECT
public:
	explicit DataProcessor(AcSensor *acSensor,
						   AcSensorSettings *settings, QObject *parent = 0);

	void setPower(Phase phase, double value);

	void setVoltage(Phase phase, double value);

	void setCurrent(Phase phase, double value);

	void setPositiveEnergy(Phase phase, double value);

	void setNegativeEnergy(double sum);

	void setNegativeEnergy(Phase phase, double value);

	void updateEnergySettings();

private:
	double getReverseEnergy(Phase phase);

	void setReverseEnergy(Phase phase, double value);

	void updateEnergySettings(Phase phase);

	void setInitialEnergy(Phase phase, double defaultValue);

	AcSensor *mAcSensor;
	AcSensorSettings *mSettings;
	double mNegativePower[4];
};

#endif // DATA_PROCESSOR_H
