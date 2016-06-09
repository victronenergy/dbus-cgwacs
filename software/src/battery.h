#ifndef BATTERY_H
#define BATTERY_H

#include <QObject>

class Battery : public QObject
{
	Q_OBJECT
public:
	explicit Battery(QObject *parent = 0);

	virtual double maxChargeCurrent() const = 0;

	virtual double maxDischargeCurrent() const = 0;

	virtual int cellImbalanceAlarm() const = 0;

	virtual int highChargeCurrentAlarm() const = 0;

	virtual int highDischargeCurrentAlarm() const = 0;

	virtual int highTemperatureAlarm() const = 0;

	virtual int highVoltageAlarm() const = 0;

	virtual int internalError() const = 0;

	virtual int lowTemperatureAlarm() const = 0;

	virtual int lowVoltageAlarm() const = 0;

signals:
	void maxChargeCurrentChanged();

	void maxDischargeCurrentChanged();

	void cellImbalanceAlarmChanged();

	void highChargeCurrentAlarmChanged();

	void highDischargeCurrentAlarmChanged();

	void highTemperatureAlarmChanged();

	void highVoltageAlarmChanged();

	void internalErrorChanged();

	void lowTemperatureAlarmChanged();

	void lowVoltageAlarmChanged();
};

#endif // BATTERY_H
