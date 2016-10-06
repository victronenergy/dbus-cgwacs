#ifndef VBUSITEMSYSTEMCALC_H
#define VBUSITEMSYSTEMCALC_H

#include "system_calc.h"

class VeQItem;

class VBusItemSystemCalc : public SystemCalc
{
	Q_OBJECT
public:
	VBusItemSystemCalc(QObject *parent = 0);

	virtual double soc() const;

private:
	VeQItem *mSystemSoc;
};

#endif // VBUSITEMSYSTEMCALC_H
