#ifndef MOCKBATTERY_H
#define MOCKBATTERY_H

#include <battery.h>

class MockBattery : public Battery
{
public:
	MockBattery(QObject *parent = 0);

	virtual double maxChargeCurrent() const
	{
		return mMaxChargeCurrent;
	}

	void setMaxChargeCurrent(double c);

	virtual double maxDischargeCurrent() const
	{
		return mMaxDischargeCurrent;
	}

	void setMaxDischargeCurrent(double c);

	virtual int cellImbalanceAlarm() const
	{
		return mCellImbalanceAlarm;
	}

	virtual int highChargeCurrentAlarm() const
	{
		return mHighChargeCurrentAlarm;
	}

	virtual int highDischargeCurrentAlarm() const
	{
		return mHighDischargeCurrentAlarm;
	}

	virtual int highTemperatureAlarm() const
	{
		return mHighTemperatureAlarm;
	}

	virtual int highVoltageAlarm() const
	{
		return mHighVoltageAlarm;
	}

	virtual int internalError() const
	{
		return mInternalError;
	}

	virtual int lowTemperatureAlarm() const
	{
		return mLowTemperatureAlarm;
	}

	virtual int lowVoltageAlarm() const
	{
		return mLowVoltageAlarm;
	}

private:
	double mMaxChargeCurrent;
	double mMaxDischargeCurrent;
	int mCellImbalanceAlarm;
	int mHighChargeCurrentAlarm;
	int mHighDischargeCurrentAlarm;
	int mHighTemperatureAlarm;
	int mHighVoltageAlarm;
	int mInternalError;
	int mLowTemperatureAlarm;
	int mLowVoltageAlarm;
};

#endif // MOCKBATTERY_H
