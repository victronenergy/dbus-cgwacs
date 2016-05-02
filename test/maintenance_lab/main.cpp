#include <qmath.h>
#include <QCoreApplication>
#include <QDateTime>
#include <QtDebug>
#include <QtGlobal>
#include "battery_life.h"
#include "multi.h"
#include "settings.h"
#include "mock_system_calc.h"

double rnd(double min, double max)
{
	return min + ((max - min) * qrand()) / RAND_MAX;
}

class TestClock : public Clock
{
public:
	virtual QDateTime now() const
	{
		return mTime;
	}

	void setTime(const QDateTime &time)
	{
		mTime = time;
	}

	void addSecs(int s)
	{
		mTime = mTime.addSecs(s);
	}

private:
	QDateTime mTime;
};

const double FloatSoc = 99;
const double AbsorptionSoc = 95;
const double SustainSoc = 5;
const double BatteryCapacity = 100;
const double BatteryVoltage = 12;
const double SocToWatt = BatteryCapacity * 3600 * BatteryVoltage / (SocInterval / 1000) / 100;

void run() {
	Multi multi;
	multi.setDcVoltage(BatteryVoltage);
	multi.setMaxChargeCurrent(50);
	MockSystemCalc systemCalc;
	Settings settings;
	settings.setState(BatteryLifeStateRestart);
	TestClock *clock = new TestClock();
	QDateTime now = QDateTime::currentDateTime();
	now.setTime(QTime(0, 0));
	clock->setTime(now);
	BatteryLife mc(&systemCalc, &multi, &settings, clock);

	double soc = 10;
	settings.setSocLimit(10);
	BatteryLifeState prevState = settings.state();
	for (int i=0; i<1000; ++i) {
		double loadBase = 300;
		loadBase *= rnd(0.5, 1.5);
		double pvOffset = 0.9 * (1 + qSin(2 * M_PI * (i % 365)/365.0)) / 2;
		double pvBase = 300;
		pvBase *= rnd(0.5, 1.5);
		double minSoc = 100;
		double maxSoc = 0;
		double minSocLimit = 100;
		double totalPvPower = 0;
		double totalLoad = 0;
		double chargePower = 0;
		for (int j=0; j<SocIntervalsPerDay; ++j) {
			double pvPower = pvBase * qMax(0.0, qSin((2 * M_PI * j) / SocIntervalsPerDay) - pvOffset);
			pvPower *= rnd(0.5, 1.5);
			Q_ASSERT(pvPower >= 0);
			double load = loadBase * qMax(0.1, qSin((4 * M_PI * j) / SocIntervalsPerDay));
			load *= rnd(0.5, 1.5);
			Q_ASSERT(load >= 0);
			if (j % 7 < 2)
				load *= 1.5;
			totalPvPower += pvPower;
			totalLoad += load;
			chargePower += qMax(0.0, pvPower - load);
			double dp = (pvPower - load) / SocToWatt;
			if (dp > 0 && (soc + dp) > AbsorptionSoc)
				dp = qMax(0.0, AbsorptionSoc - soc) + qMax(0.0, soc + dp - AbsorptionSoc) / 8;
			if (settings.state() == BatteryLifeStateForceCharge)
				dp = 5 * multi.dcVoltage() / SocToWatt;
			if (settings.state() == BatteryLifeStateDischarged || multi.isSustainActive())
				dp = qMax(0.0, dp);
			soc += dp;
			bool feedbackDisabled =
				settings.state() == BatteryLifeStateForceCharge ||
				settings.state() == BatteryLifeStateDischarged;
			multi.setIsFeedbackDisabled(feedbackDisabled);
			multi.setDcCurrent(dp * SocToWatt / multi.dcVoltage());
			soc = qMin(soc, 100.0);
			if (soc < SustainSoc) {
				soc = SustainSoc;
				systemCalc.setSoc(soc);
				multi.setIsSustainActive(true);
			} else {
				systemCalc.setSoc(soc);
				multi.setIsSustainActive(false);
			}
			minSoc = qMin(minSoc, soc);
			maxSoc = qMax(maxSoc, soc);
			minSocLimit = qMin(minSocLimit, settings.socLimit());
			if (soc > FloatSoc)
				multi.setState(MultiStateFloat);
			else if (soc > AbsorptionSoc)
				multi.setState(MultiStateAbsorption);
			else
				multi.setState(MultiStateBulk);
			clock->addSecs(SocInterval / 1000);
			mc.onChargeTimer();
//			if (settings.state() != prevState) {
//				qDebug() << i + (double)j/SocIntervalsPerDay
//						 << '\t' << multi.soc()
//						 << '\t' << ((int)settings.state()) * 10
//						 << '\t' << settings.socLimit()
//						 << '\t' << totalLoad
//						 << '\t' << totalLoad
//						 << '\t' << settings.state();
//				prevState = settings.state();
//			}
		}
//		minSoc = qMax(minSoc, settings.socLimit());
//		maxSoc = qMax(maxSoc, settings.socLimit());
		qDebug() << i << '\t' << minSoc << '\t' << maxSoc << '\t' << minSocLimit
				 << 0 << ((int)settings.state()) * 10 << '\t'
				 // << mc.averageSolarYield() * 10 / multi.dcVoltage() * BatteryCapacity
				 << '\t' << totalLoad * SocInterval  / (10 * (multi.dcVoltage() * BatteryCapacity * 3600))
				 << '\t' << chargePower;
	}
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	run();

	// return a.exec();
}
