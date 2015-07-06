#include <QtGlobal>
#include <QTimer>
#include "ac_sensor.h"
#include "ac_sensor_sim.h"
#include "multi.h"
#include "multi_phase_data.h"
#include "power_info.h"

AcSensorSim::AcSensorSim(AcSensor *m, Multi *multi, QObject *parent):
	QObject(parent),
	mAcSensor(m),
	mMulti(multi),
	mTimer(new QTimer(this)),
	mTimerCount(0)
{
	Q_ASSERT(m != 0);
	Q_ASSERT(multi != 0);
	mTimer->setInterval(500);
	mTimer->start();
	connect(mTimer, SIGNAL(timeout()), this, SLOT(onTimer()));
}

void AcSensorSim::onTimer()
{
	double totalPower = 0;
	if (mTimerCount < 20) {
		totalPower = 1000;
		++mTimerCount;
	} else {
		totalPower = -500;
	}
	totalPower *= 1 + (qrand() % 10000) / (100 * 10000.0);
	totalPower -= mMulti->l1Data()->acPowerIn();
	mAcSensor->l1PowerInfo()->setPower(totalPower);
}