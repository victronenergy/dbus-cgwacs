#include <ac_sensor.h>
#include <ac_sensor_settings.h>
#include <battery_info.h>
#include <dbus_service_monitor.h>
#include <charge_phase_control.h>
#include <multi.h>
#include <multi_phase_data.h>
#include <phase_compensation_control.h>
#include <power_info.h>
#include <QCoreApplication>
#include <qmath.h>
#include <QsLog.h>
#include <settings.h>
#include <single_phase_control.h>

double rnd(double min, double max) {
	return ((max - min) * qrand()) / RAND_MAX + min;
}

void run_test()
{
	AcSensor sensor("", 0);
	AcSensorSettings sensorSettings(71, "");
	sensorSettings.setHub4Mode(Hub4PhaseCompensation);
	sensorSettings.setIsMultiPhase(true);
	Multi multi;
	Settings settings;
	DbusServiceMonitor serviceMonitor;
	BatteryInfo bi(&serviceMonitor, &multi, &settings);
	double pLoadL1 = 200;
	double pLoadL2 = -100;
	double pLoadL3 = 400;
	multi.setDcVoltage(12);
	multi.l1Data()->setAcPowerSetPoint(0);
	multi.l2Data()->setAcPowerSetPoint(0);
	multi.l3Data()->setAcPowerSetPoint(0);
	settings.setAcPowerSetPoint(0);

//	SinglePhaseControl controlLoopL1(&multi, &sensor, &settings, PhaseL1, Hub4PhaseSplit);
//	SinglePhaseControl controlLoopL2(&multi, &sensor, &settings, PhaseL2, Hub4PhaseSplit);
//	SinglePhaseControl controlLoopL3(&multi, &sensor, &settings, PhaseL3, Hub4PhaseSplit);
	PhaseCompensationControl controlLoop(&multi, &sensor, &settings);
	controlLoop.setBatteryInfo(&bi);
//	ChargePhaseControl controlLoop(&multi, &sensor, &settings, PhaseL1);
	double pLoad = 0;
	if (qIsFinite(pLoadL1))
		pLoad += pLoadL1;
	if (qIsFinite(pLoadL2))
		pLoad += pLoadL2;
	if (qIsFinite(pLoadL3))
		pLoad += pLoadL3;
	multi.l1Data()->setAcPowerIn(0);
	multi.l2Data()->setAcPowerIn(0);
	multi.l3Data()->setAcPowerIn(0);
	multi.meanData()->setAcPowerIn(0);
	for (int i=0; i<5; ++i) {
		sensor.l1PowerInfo()->setPower(pLoadL1);
		sensor.l2PowerInfo()->setPower(pLoadL2);
		sensor.l3PowerInfo()->setPower(pLoadL3);
		sensor.meanPowerInfo()->setPower(pLoad);
	}
	for (int i=0; i<20; ++i) {
		double pL1 = qRound(0.5 * multi.l1Data()->acPowerIn() + 0.5 * multi.l1Data()->acPowerSetPoint());
		double pL2 = qRound(0.5 * multi.l2Data()->acPowerIn() + 0.5 * multi.l2Data()->acPowerSetPoint());
		double pL3 = qRound(0.5 * multi.l3Data()->acPowerIn() + 0.5 * multi.l3Data()->acPowerSetPoint());
//		pL1 += rnd(-10, 10);
//		pL2 += rnd(-10, 10);
//		pL3 += rnd(-10, 10);
		double pT = 0;
		if (qIsFinite(pL1)) {
			pT += pL1;
			for (int i=0; i<5; ++i)
				sensor.l1PowerInfo()->setPower(pLoadL1 + pL1);
		}
		if (qIsFinite(pL2)) {
			pT += pL2;
			for (int i=0; i<5; ++i)
				sensor.l2PowerInfo()->setPower(pLoadL2 + pL2);
		}
		if (qIsFinite(pL3)) {
			pT += pL3;
			for (int i=0; i<5; ++i)
				sensor.l3PowerInfo()->setPower(pLoadL3 + pL3);
		}
		for (int i=0; i<5; ++i)
			sensor.meanPowerInfo()->setPower(pLoad + pT);
		// QLOG_WARN() << pLoad + pT;
		if (qIsFinite(pL1))
			multi.l1Data()->setAcPowerIn(pL1);
		if (qIsFinite(pL2))
			multi.l2Data()->setAcPowerIn(pL2);
		if (qIsFinite(pL3))
			multi.l3Data()->setAcPowerIn(pL3);
		multi.meanData()->setAcPowerIn(pT);
		multi.setDcCurrent(pT * (pT > 0 ? 0.8 : 1.3) / multi.dcVoltage());
		// controlLoop.onTimer();
	}
}

int main(int argc, char *argv[])
{
	QsLogging::Logger &logger = QsLogging::Logger::instance();
	QsLogging::DestinationPtr debugDestination(
		QsLogging::DestinationFactory::MakeDebugOutputDestination());
	logger.addDestination(debugDestination);
	logger.setLoggingLevel(QsLogging::TraceLevel);

	QCoreApplication app(argc, argv);

	run_test();

	// app.exec();
}
