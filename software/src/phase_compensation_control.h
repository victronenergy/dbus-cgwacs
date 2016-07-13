#ifndef PHASE_COMPENSATION_CONTROL_H
#define PHASE_COMPENSATION_CONTROL_H

#include <QVector>
#include "defines.h"
#include "multi_phase_control.h"

class GridMeter;
class Multi;
class QTimer;
class Settings;

/*!
 * Implements the multi phase compensation control loop
 *
 * The algorithm will try to make the total power on grid 0, and minimize the
 * power flowing from phase to phase through the multi. This means that there
 * is never a multi charging while another is discharging.
 */
class PhaseCompensationControl : public MultiPhaseControl
{
	Q_OBJECT
public:
	PhaseCompensationControl(SystemCalc *systemCalc, Multi *multi, GridMeter *gridMeter,
							 Settings *settings, QObject *parent = 0);

protected:
	virtual void performStep();

private:
	QVector<double> mSetpoints;
};

#endif // PHASE_COMPENSATION_CONTROL_H
