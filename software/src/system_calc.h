#ifndef SYSTEMCALC_H
#define SYSTEMCALC_H

#include <QObject>

class DbusServiceMonitor;
class VBusItem;

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

class VBusItemSystemCalc : public SystemCalc
{
	Q_OBJECT
public:
	VBusItemSystemCalc(DbusServiceMonitor *serviceMonitor, QObject *parent = 0);

	virtual double soc() const;

private slots:
	void onServiceAdded(QString service);

private:
	VBusItem *mSystemSoc;
};

#endif // SYSTEMCALC_H
