#include <QsLog.h>
#include <QTimer>
#include "ac_sensor.h"
#include "battery_info.h"
#include "multi.h"
#include "multi_phase_control.h"
#include "multi_phase_data.h"
#include "power_info.h"

MultiPhaseControl::MultiPhaseControl(Multi *multi, AcSensor *acSensor, Settings *settings,
									 QObject *parent):
	ControlLoop(multi, settings, parent),
	mAcSensor(acSensor),
	mTimer(new QTimer(this)),
	mMultiL1Update(false),
	mMultiL2Update(false),
	mMultiL3Update(false),
	mMultiTotalPowerUpdate(false),
	mMeterUpdate(false)
{
	Q_ASSERT(multi != 0);
	Q_ASSERT(acSensor != 0);
	Q_ASSERT(settings != 0);
	mTimer->setInterval(6000);
	mTimer->start();
	connect(multi->getPhaseData(PhaseL1), SIGNAL(acPowerInChanged()),
			this, SLOT(onL1FromMulti()));
	connect(multi->getPhaseData(PhaseL2), SIGNAL(acPowerInChanged()),
			this, SLOT(onL2FromMulti()));
	connect(multi->getPhaseData(PhaseL3), SIGNAL(acPowerInChanged()),
			this, SLOT(onL3FromMulti()));
	connect(multi->getPhaseData(MultiPhase), SIGNAL(acPowerInChanged()),
			this, SLOT(onTotalPowerFromMulti()));
	connect(acSensor, SIGNAL(destroyed()), this, SLOT(deleteLater()));
	// Total power is changed after phase specific data. So if total power changes, L1, L2, and L3
	// have fresh values as well.
	connect(acSensor->getPowerInfo(MultiPhase), SIGNAL(powerChanged()),
			this, SLOT(onPowerFromMeter()));
	connect(mTimer, SIGNAL(timeout()), this, SLOT(onTimer()));
}

void MultiPhaseControl::adjustSetpoints(double setpoints[])
{
	double setpointTotal = 0;
	for (int p=0; p<3; ++p) {
		if (qIsFinite(setpoints[p]))
			setpointTotal += setpoints[p];
	}
	BatteryInfo *bi = batteryInfo();
	double f = bi == 0 ? 1 : bi->applyLimits(setpointTotal) / setpointTotal;
	QLOG_WARN() << __func__ << setpointTotal << f << setpointTotal * f;
	AcSensor *acSensor = this->acSensor();
	Multi *multi = this->multi();
	for (int p=0; p<3; ++p) {
		Phase phase = static_cast<Phase>(PhaseL1 + p);
		MultiPhaseData *mpd = multi->getPhaseData(phase);
		PowerInfo *pi = acSensor->getPowerInfo(phase);
		if (qIsFinite(setpoints[p])) {
			QLOG_WARN() << __func__ << setpoints[p] * f;
			adjustSetpoint(pi, phase, mpd, setpoints[p] * f);
		}
	}
}

void MultiPhaseControl::onTimer()
{
	QLOG_DEBUG() << "Update timeout";
	mMultiL1Update = true;
	mMultiL2Update = true;
	mMultiL3Update = true;
	mMeterUpdate = true;
	checkStep();
}

void MultiPhaseControl::onL1FromMulti()
{
	mMultiL1Update = true;
	checkStep();
}

void MultiPhaseControl::onL2FromMulti()
{
	mMultiL2Update = true;
	checkStep();
}

void MultiPhaseControl::onL3FromMulti()
{
	mMultiL3Update = true;
	checkStep();
}

void MultiPhaseControl::onTotalPowerFromMulti()
{
	mMultiTotalPowerUpdate = true;
	checkStep();
}

void MultiPhaseControl::onPowerFromMeter()
{
	mMeterUpdate = true;
	checkStep();
}

void MultiPhaseControl::checkStep()
{
	if (!mMeterUpdate)
		return;
	if (!mMultiTotalPowerUpdate)
		return;
	if (!mMultiL1Update && hasSetpoint(PhaseL1))
		return;
	if (!mMultiL2Update && hasSetpoint(PhaseL2))
		return;
	if (!mMultiL3Update && hasSetpoint(PhaseL3))
		return;
	performStep();
	mMeterUpdate = false;
	mMultiL1Update = false;
	mMultiL2Update = false;
	mMultiL3Update = false;
	mMultiTotalPowerUpdate = false;
	mTimer->start();
}
