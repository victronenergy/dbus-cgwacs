#ifndef SYSTEMCALC_H
#define SYSTEMCALC_H

#include <QObject>

class SystemCalc: public QObject
{
	Q_OBJECT
	Q_PROPERTY(double soc READ soc NOTIFY socChanged)
public:
	SystemCalc(QObject *parent = 0);

	virtual double soc() const = 0;

signals:
	void socChanged();
};

#endif // SYSTEMCALC_H
