#ifndef MOCKSYSTEMCALC_H
#define MOCKSYSTEMCALC_H

#include "system_calc.h"

class MockSystemCalc : public SystemCalc
{
	Q_OBJECT
public:
	MockSystemCalc(QObject *parent = 0);

	virtual double soc() const
	{
		return mSoc;
	}

	void setSoc(double s);

private:
	double mSoc;
};

#endif // MOCKSYSTEMCALC_H
