#ifndef MULTI_H
#define MULTI_H

#include <QMetaType>
#include <QObject>
#include "defines.h"

class MultiPhaseData;

/*!
 * This class stores data from a Multi (or Quattro) needed for the Hub-4 control
 * loop (`ControlLoop`).
 */
class Multi : public QObject
{
	Q_OBJECT
	Q_PROPERTY(double acPowerSetPoint READ acPowerSetPoint WRITE setAcPowerSetPoint NOTIFY acPowerSetPointChanged)
	Q_PROPERTY(bool isSetPointAvailable READ isSetPointAvailable WRITE setIsSetPointAvailable NOTIFY isSetPointAvailableChanged)
public:
	explicit Multi(QObject *parent = 0);

	double acPowerSetPoint() const;

	void setAcPowerSetPoint(double o);

	bool isSetPointAvailable() const;

	void setIsSetPointAvailable(bool s);

	MultiPhaseData *meanData();

	MultiPhaseData *l1Data();

	MultiPhaseData *l2Data();

	MultiPhaseData *l3Data();

	MultiPhaseData *getPhaseData(Phase phase);

signals:
	void acPowerSetPointChanged();

	void isSetPointAvailableChanged();

private:
	MultiPhaseData *mMeanData;
	MultiPhaseData *mL1Data;
	MultiPhaseData *mL2Data;
	MultiPhaseData *mL3Data;
	double mAcPowerSetPoint;
	bool mIsSetPointAvailable;
};

#endif // MULTI_H
