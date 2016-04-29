#ifndef SPLITPHASECONTROL_H
#define SPLITPHASECONTROL_H

#include <QVector>
#include "multi_phase_control.h"

class SplitPhaseControl: public MultiPhaseControl
{
	Q_OBJECT
public:
	SplitPhaseControl(Multi *multi, AcSensor *acSensor, Settings *settings, QObject *parent = 0);

protected:
	virtual void performStep();

private:
	QVector<double> mSetpoints;
};

#endif // SPLITPHASECONTROL_H
