#include <qmath.h>
#include <limits>
#include <QsLog.h>
#include "defines.h"
#include "multi_phase_data.h"

MultiPhaseData::MultiPhaseData(QObject *parent) :
	QObject(parent),
	mAcPowerIn(NaN),
	mAcPowerSetPoint(NaN),
	mIsSetPointAvailable(false)
{
}

double MultiPhaseData::acPowerIn() const
{
	return mAcPowerIn;
}

void MultiPhaseData::setAcPowerIn(double o)
{
//	if (mAcPowerIn == o || (std::isnan(mAcPowerIn) && std::isnan(o)))
//		return;
	mAcPowerIn = o;
	emit acPowerInChanged();
}

double MultiPhaseData::acPowerSetPoint() const
{
	return mAcPowerSetPoint;
}

void MultiPhaseData::setAcPowerSetPoint(double o)
{
//	if (mAcPowerSetPoint == o || (std::isnan(mAcPowerSetPoint) && std::isnan(o)))
//		return;
	mAcPowerSetPoint = o;
	emit acPowerSetPointChanged();
	bool setpointAvailable = !qIsNaN(mAcPowerSetPoint);
	if (setpointAvailable != mIsSetPointAvailable) {
		mIsSetPointAvailable = setpointAvailable;
		emit isSetPointAvailableChanged();
	}
}

bool MultiPhaseData::isSetPointAvailable() const
{
	return mIsSetPointAvailable;
}
