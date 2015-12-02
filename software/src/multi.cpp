#include <QsLog.h>
#include "multi.h"
#include "multi_phase_data.h"

Multi::Multi(QObject *parent) :
	QObject(parent),
	mMeanData(new MultiPhaseData(this)),
	mL1Data(new MultiPhaseData(this)),
	mL2Data(new MultiPhaseData(this)),
	mL3Data(new MultiPhaseData(this)),
	mDcVoltage(NaN),
	mMaxChargeCurrent(NaN),
	mChargeDisabled(false),
	mFeedbackDisabled(false),
	mMode(MultiOff)
{
}

double Multi::dcVoltage() const
{
	return mDcVoltage;
}

void Multi::setDcVoltage(double v)
{
	if (mDcVoltage == v)
		return;
	mDcVoltage = v;
	emit dcVoltageChanged();
}

double Multi::maxChargeCurrent() const
{
	return mMaxChargeCurrent;
}

void Multi::setMaxChargeCurrent(double c)
{
	if (mMaxChargeCurrent == c)
		return;
	mMaxChargeCurrent = c;
	emit maxChargeCurrentChanged();
}

bool Multi::isChargeDisabled() const
{
	return mChargeDisabled;
}

void Multi::setIsChargeDisabled(bool b)
{
//	if (mChargeDisabled == b)
//		return;
	mChargeDisabled = b;
	emit isChargeDisabledChanged();
}

bool Multi::isFeedbackDisabled() const
{
	return mFeedbackDisabled;
}

void Multi::setIsFeedbackDisabled(bool b)
{
//	if (mFeedbackDisabled == b)
//		return;
	mFeedbackDisabled = b;
	emit isFeedbackDisabledChanged();
}

MultiMode Multi::mode() const
{
	return mMode;
}

void Multi::setMode(MultiMode m)
{
	if (mMode == m)
		return;
	mMode = m;
	emit modeChanged();
}

MultiState Multi::state() const
{
	return mState;
}

void Multi::setState(MultiState m)
{
	if (mState == m)
		return;
	mState = m;
	emit stateChanged();
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
