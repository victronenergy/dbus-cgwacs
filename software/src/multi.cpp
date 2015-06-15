#include <limits>
#include <QsLog.h>
#include "multi.h"
#include "multi_phase_data.h"

static const double NaN = std::numeric_limits<double>::quiet_NaN();

Multi::Multi(QObject *parent) :
	QObject(parent),
	mMeanData(new MultiPhaseData(this)),
	mL1Data(new MultiPhaseData(this)),
	mL2Data(new MultiPhaseData(this)),
	mL3Data(new MultiPhaseData(this)),
	mAcPowerSetPoint(0),
	mIsSetPointAvailable(false),
	mDcVoltage(NaN),
	mMaxChargeCurrent(NaN),
	mMode(MultiOff),
	mChargeDisabled(false),
	mFeedbackDisabled(false)
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

bool Multi::isChargeDisabled() const
{
	return mChargeDisabled;
}

void Multi::setIsChargeDisabled(bool b)
{
	if (mChargeDisabled == b)
		return;
	mChargeDisabled = b;
	emit isChargeDisabledChanged();
}

bool Multi::isFeedbackDisabled() const
{
	return mFeedbackDisabled;
}

void Multi::setIsFeedbackDisabled(bool b)
{
	if (mFeedbackDisabled == b)
		return;
	mFeedbackDisabled = b;
	emit isFeedbackDisabledChanged();
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
