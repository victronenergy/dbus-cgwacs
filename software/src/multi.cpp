#include <QsLog.h>
#include "multi.h"
#include "multi_phase_data.h"

Multi::Multi(QObject *parent) :
	QObject(parent),
	mMeanData(new MultiPhaseData(this)),
	mL1Data(new MultiPhaseData(this)),
	mL2Data(new MultiPhaseData(this)),
	mL3Data(new MultiPhaseData(this)),
	mAcPowerSetPoint(0),
	mIsSetPointAvailable(false)
{
}

double Multi::acPowerSetPoint() const
{
	return mAcPowerSetPoint;
}

void Multi::setAcPowerSetPoint(double o)
{
	if (mAcPowerSetPoint == o)
		return;
	mAcPowerSetPoint = o;
	emit acPowerSetPointChanged();
}

bool Multi::isSetPointAvailable() const
{
	return mIsSetPointAvailable;
}

void Multi::setIsSetPointAvailable(bool s)
{
	if (mIsSetPointAvailable == s)
		return;
	mIsSetPointAvailable = s;
	emit isSetPointAvailableChanged();
}

MultiPhaseData *Multi::meanData()
{
	return mMeanData;
}

MultiPhaseData *Multi::l1Data()
{
	return mL1Data;
}

MultiPhaseData *Multi::l2Data()
{
	return mL2Data;
}

MultiPhaseData *Multi::l3Data()
{
	return mL3Data;
}

MultiPhaseData *Multi::getPhaseData(Phase phase)
{
	switch (phase) {
	case MultiPhase:
		return mMeanData;
	case PhaseL1:
		return mL1Data;
	case PhaseL2:
		return mL2Data;
	case PhaseL3:
		return mL3Data;
	default:
		QLOG_ERROR() << "Incorrect phase:" << phase;
		return 0;
	}
}
