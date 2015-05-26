#include <QtGlobal>
#include <QTimer>
#include "multi_sim.h"

const int UpdateInterval = 100; // 100 ms
const double MaxPowerIncrement = 100; // 100 W/sec
const double IncrementPerInverval = (MaxPowerIncrement * UpdateInterval) / 1000;

MultiSim::MultiSim(QObject *parent) :
	QObject(parent),
	mPowerOut(0),
	mPowerOutSetPoint(0),
	mTimer(new QTimer(this)),
	mTimerCount(0)
{
	mTimer->setInterval(UpdateInterval);
	mTimer->start();

	connect(mTimer, SIGNAL(timeout()), this, SLOT(onTimer()));
}

double MultiSim::powerOut() const
{
	return mPowerOut;
}

void MultiSim::setPowerOut(double p)
{
	if (mPowerOut == p)
		return;
	mPowerOut = p;
	emit powerOutChanged();
}

double MultiSim::powerOutSetPoint() const
{
	return mPowerOutSetPoint;
}

void MultiSim::setPowerOutSetPoint(double p)
{
	if (mPowerOutSetPoint == p)
		return;
	mPowerOutSetPoint = p;
	emit powerOutSetPointChanged();
}

void MultiSim::onTimer()
{
//	if (mTimerCount > 50) {
//		setPowerOut(0);
//		return;
//	}
	++mTimerCount;

	const double Alpha = 0.95;
	double newPowerOut = Alpha * mPowerOutSetPoint + (1 - Alpha) * mPowerOut;

//	double diff = 0.8 * (mPowerOutSetPoint - mPowerOut);
//	diff = qBound(-IncrementPerInverval, diff, IncrementPerInverval);
//	double newPowerOut = qRound(mPowerOut + diff);
	setPowerOut(newPowerOut);
}
