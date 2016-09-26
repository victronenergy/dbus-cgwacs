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

signals:
	void maxChargeCurrentChanged();

	void maxDischargeCurrentChanged();
};

#endif // BATTERY_H
