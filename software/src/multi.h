#ifndef MULTI_H
#define MULTI_H

#include <QMetaType>
#include <QObject>
#include "defines.h"

class MultiPhaseData;

enum MultiMode {
	MultiChargerOnly = 1,
	MultiInverterOnly = 2,
	MultiOn = 3,
	MultiOff = 4
};

Q_DECLARE_METATYPE(MultiMode)

/*!
 * This class stores data from a Multi (or Quattro) needed for the Hub-4 control
 * loop (`ControlLoop`).
 */
class Multi : public QObject
{
	Q_OBJECT
	Q_PROPERTY(double acPowerSetPoint READ acPowerSetPoint WRITE setAcPowerSetPoint NOTIFY acPowerSetPointChanged)
	Q_PROPERTY(bool isSetPointAvailable READ isSetPointAvailable WRITE setIsSetPointAvailable NOTIFY isSetPointAvailableChanged)
	Q_PROPERTY(double dcVoltage READ dcVoltage WRITE setDcVoltage NOTIFY dcVoltageChanged)
	Q_PROPERTY(double maxChargeCurrent READ maxChargeCurrent WRITE setMaxChargeCurrent NOTIFY maxChargeCurrentChanged)
	Q_PROPERTY(bool isChargeDisabled READ isChargeDisabled WRITE setIsChargeDisabled NOTIFY isChargeDisabledChanged)
	Q_PROPERTY(bool isFeedbackDisabled READ isFeedbackDisabled WRITE setIsFeedbackDisabled NOTIFY isFeedbackDisabledChanged)
	Q_PROPERTY(MultiMode mode READ mode WRITE setMode NOTIFY modeChanged)
public:
	explicit Multi(QObject *parent = 0);

	double acPowerSetPoint() const;

	void setAcPowerSetPoint(double o);

	bool isSetPointAvailable() const;

	void setIsSetPointAvailable(bool s);

	double dcVoltage() const;

	void setDcVoltage(double v);

	double maxChargeCurrent() const;

	void setMaxChargeCurrent(double c);

	MultiMode mode() const;

	void setMode(MultiMode m);

	bool isChargeDisabled() const;

	void setIsChargeDisabled(bool b);

	bool isFeedbackDisabled() const;

	void setIsFeedbackDisabled(bool b);

	MultiPhaseData *meanData();

	MultiPhaseData *l1Data();

	MultiPhaseData *l2Data();

	MultiPhaseData *l3Data();

	MultiPhaseData *getPhaseData(Phase phase);

signals:
	void acPowerSetPointChanged();

	void isSetPointAvailableChanged();

	void dcVoltageChanged();

	void maxChargeCurrentChanged();

	void modeChanged();

	void isChargeDisabledChanged();

	void isFeedbackDisabledChanged();

private:
	MultiPhaseData *mMeanData;
	MultiPhaseData *mL1Data;
	MultiPhaseData *mL2Data;
	MultiPhaseData *mL3Data;
	double mAcPowerSetPoint;
	bool mIsSetPointAvailable;
	double mDcVoltage;
	double mMaxChargeCurrent;
	MultiMode mMode;
	bool mChargeDisabled;
	bool mFeedbackDisabled;
};

#endif // MULTI_H
