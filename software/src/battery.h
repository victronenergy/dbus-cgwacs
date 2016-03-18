#ifndef BATTERY_H
#define BATTERY_H

#include <QObject>

class Battery : public QObject
{
	Q_OBJECT
	Q_PROPERTY(double maxChargeCurrent READ maxChargeCurrent WRITE setMaxChargeCurrent NOTIFY maxChargeCurrentChanged)
	Q_PROPERTY(double maxDischargeCurrent READ maxDischargeCurrent WRITE setMaxDischargeCurrent NOTIFY maxDischargeCurrentChanged)
public:
	explicit Battery(QObject *parent = 0);

	double maxChargeCurrent() const;

	void setMaxChargeCurrent(double c);

	double maxDischargeCurrent() const;

	void setMaxDischargeCurrent(double c);

signals:
	void maxChargeCurrentChanged();

	void maxDischargeCurrentChanged();

private:
	double mMaxChargeCurrent;
	double mMaxDischargeCurrent;
};

#endif // BATTERY_H
