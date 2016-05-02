#ifndef VBUSITEMSYSTEMCALC_H
#define VBUSITEMSYSTEMCALC_H

#include "system_calc.h"

class DbusServiceMonitor;
class VBusItem;

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

#endif // VBUSITEMSYSTEMCALC_H
