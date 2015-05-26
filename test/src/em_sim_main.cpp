#include <QCoreApplication>
#include <velib/qt/v_busitems.h>
#include "control_loop.h"
#include "energy_meter.h"
#include "energy_meter_bridge.h"
#include "energy_meter_settings.h"
#include "energy_meter_sim.h"
#include "multi.h"
#include "multi_bridge.h"
#include "settings.h"

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

#if TARGET_ccgx
	VBusItems::setConnectionType(QDBusConnection::SystemBus);
#endif

	EnergyMeter em("/dev/ttyUSB0");
	EnergyMeterSettings emSettings("B01202");
	EnergyMeterBridge emBridge(&em, &emSettings);

	Multi multi;
	EnergyMeterSim emSim(&em, &multi);

	MultiBridge multiBridge(&multi, "ttyO1");

	Settings settings;
	ControlLoop cl(&multi, &em, &settings);

	return app.exec();
}
