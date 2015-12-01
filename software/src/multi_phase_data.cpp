#include <qmath.h>
#include <QsLog.h>
#include "defines.h"
#include "multi_phase_data.h"

MultiPhaseData::MultiPhaseData(QObject *parent) :
	QObject(parent),
	mAcPowerIn(NaN),
	mAcPowerOut(NaN),
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
//	if (mAcPowerIn == o || (qIsNaN(mAcPowerIn) && qIsNaN(o)))
//		return;
	mAcPowerIn = o;
	emit acPowerInChanged();
}

double MultiPhaseData::acPowerOut() const
{
	return mAcPowerOut;
}

void MultiPhaseData::setAcPowerOut(double o)
{
	mAcPowerOut = o;
	emit acPowerOutChanged();
}

double MultiPhaseData::acPowerSetPoint() const
{
	return mAcPowerSetPoint;
}

void MultiPhaseData::setAcPowerSetPoint(double o)
{
//	if (mAcPowerSetPoint == o || (qIsNaN(mAcPowerSetPoint) && qIsNaN(o)))
//		return;
	mAcPowerSetPoint = o;
	emit acPowerSetPointChanged();
	bool setpointAvailable = qIsFinite(mAcPowerSetPoint);
	if (setpointAvailable != mIsSetPointAvailable) {
		mIsSetPointAvailable = setpointAvailable;
		emit isSetPointAvailableChanged();
	}
}

bool MultiPhaseData::isSetPointAvailable() const
{
	return mIsSetPointAvailable;
}
