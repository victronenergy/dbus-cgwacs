#include <qnumeric.h>
#include "mock_system_calc.h"

MockSystemCalc::MockSystemCalc(QObject *parent):
	SystemCalc(parent),
	mSoc(qQNaN())
{
}

void MockSystemCalc::setSoc(double s)
{
	if (mSoc == s)
		return;
	mSoc = s;
	emit socChanged();
}
