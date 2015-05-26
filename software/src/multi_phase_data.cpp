#include <cmath>
#include <limits>
#include "multi_phase_data.h"

static const double NaN = std::numeric_limits<double>::quiet_NaN();

MultiPhaseData::MultiPhaseData(QObject *parent) :
	QObject(parent),
	mAcPowerIn(NaN)
{
}

double MultiPhaseData::acPowerIn() const
{
	return mAcPowerIn;
}

void MultiPhaseData::setAcPowerIn(double o)
{
	if (mAcPowerIn == o || (std::isnan(mAcPowerIn) && std::isnan(o)))
		return;
	mAcPowerIn = o;
	emit acPowerOutChanged();
}
