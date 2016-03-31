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

enum MultiState {
	MultiStateOff = 0,
	MultiStateLowPower = 1,
	MultiStateFault = 2,
	MultiStateBulk = 3,
	MultiStateAbsorption = 4,
	MultiStateFloat = 5,
	MultiStateStorage = 6,
	MultiStateEqualize = 7,
	MultiStatePassthrough = 8,
	MultiStateInverting = 9,
	MultiStateAssisting = 10,
	MultiStatePowerSupply = 11,
	MultiStateBlocked = 0xFA,
	MultiStateTest = 0xFB,
	MultiStateHub4 = 0xFC
};

Q_DECLARE_METATYPE(MultiMode)
Q_DECLARE_METATYPE(MultiState)

/*!
 * This class stores data from a Multi (or Quattro) needed for the Hub-4 control
 * loop (`ControlLoop`).
 */
class Multi : public QObject
{
	Q_OBJECT
	Q_PROPERTY(double dcVoltage READ dcVoltage WRITE setDcVoltage NOTIFY dcVoltageChanged)
	Q_PROPERTY(double maxChargeCurrent READ maxChargeCurrent WRITE setMaxChargeCurrent NOTIFY maxChargeCurrentChanged)
	Q_PROPERTY(bool isChargeDisabled READ isChargeDisabled WRITE setIsChargeDisabled NOTIFY isChargeDisabledChanged)
	Q_PROPERTY(bool isFeedbackDisabled READ isFeedbackDisabled WRITE setIsFeedbackDisabled NOTIFY isFeedbackDisabledChanged)
	Q_PROPERTY(MultiMode mode READ mode WRITE setMode NOTIFY modeChanged)
	Q_PROPERTY(MultiState state READ state WRITE setState NOTIFY stateChanged)
	Q_PROPERTY(int firmwareVersion READ firmwareVersion WRITE setFirmwareVersion NOTIFY firmwareVersionChanged)
public:
	explicit Multi(QObject *parent = 0);

	double dcVoltage() const;

	void setDcVoltage(double v);

	double maxChargeCurrent() const;

	void setMaxChargeCurrent(double c);

	bool isChargeDisabled() const;

	void setIsChargeDisabled(bool b);

	bool isFeedbackDisabled() const;

	void setIsFeedbackDisabled(bool b);

	MultiMode mode() const;

	void setMode(MultiMode m);

	MultiState state() const;

	void setState(MultiState m);

	int getSetpointCount() const;

	QList<Phase> getSetpointPhases() const;

	int firmwareVersion() const;

	void setFirmwareVersion(int v);

	MultiPhaseData *meanData();

	MultiPhaseData *l1Data();

	MultiPhaseData *l2Data();

	MultiPhaseData *l3Data();

	MultiPhaseData *getPhaseData(Phase phase) const;

signals:
	void dcVoltageChanged();

	void maxChargeCurrentChanged();

	void modeChanged();

	void stateChanged();

	void isChargeDisabledChanged();

	void isFeedbackDisabledChanged();

	void firmwareVersionChanged();

private:
	MultiPhaseData *mMeanData;
	MultiPhaseData *mL1Data;
	MultiPhaseData *mL2Data;
	MultiPhaseData *mL3Data;
	double mDcVoltage;
	double mMaxChargeCurrent;
	bool mChargeDisabled;
	bool mFeedbackDisabled;
	MultiMode mMode;
	MultiState mState;
	int mFirmwareVersion;
};

#endif // MULTI_H
