#ifndef POWER_INFO_H
#define POWER_INFO_H

#include <QObject>
#include <QTime>

/*!
 * Contains measurement data from an single AC sensor phase.
 */
class AcSensorPhase : public QObject
{
	Q_OBJECT
	Q_PROPERTY(double current READ current WRITE setCurrent NOTIFY currentChanged)
	Q_PROPERTY(double voltage READ voltage WRITE setVoltage NOTIFY voltageChanged)
	Q_PROPERTY(double power READ power WRITE setPower NOTIFY powerChanged)
	Q_PROPERTY(double energyForward READ energyForward WRITE setEnergyForward NOTIFY energyForwardChanged)
	Q_PROPERTY(double energyReverse READ energyReverse WRITE setEnergyReverse NOTIFY energyReverseChanged)
	Q_PROPERTY(double powerFactor READ powerFactor WRITE setPowerFactor NOTIFY powerFactorChanged)
public:
	explicit AcSensorPhase(QObject *parent = 0);

	double current() const
	{
		return mCurrent;
	}

	void setCurrent(double c);

	double voltage() const
	{
		return mVoltage;
	}

	void setVoltage(double v);

	double power() const
	{
		return mPower;
	}

	void setPower(double p);

	double energyForward() const
	{
		return mEnergyForward;
	}

	void setEnergyForward(double e);

	double energyReverse() const
	{
		return mEnergyReverse;
	}

	void setEnergyReverse(double e);

	double powerFactor() const
	{
		return mPowerFactor;
	}

	void setPowerFactor(double f);

	/*!
	 * @brief Reset all measured values to NaN
	 */
	void resetValues();

signals:
	void currentChanged();

	void voltageChanged();

	void powerChanged();

	void energyForwardChanged();

	void energyReverseChanged();

	void powerFactorChanged();

private:
	static bool valuesEqual(double v1, double v2);

	double mCurrent;
	double mVoltage;
	double mPower;
	double mEnergyForward;
	double mEnergyReverse;
	double mPowerFactor;
};

#endif // POWER_INFO_H
