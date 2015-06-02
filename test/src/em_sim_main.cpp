#include <QCoreApplication>
#include <velib/qt/v_busitems.h>
#include "control_loop.h"
#include "ac_sensor.h"
#include "ac_sensor_bridge.h"
#include "ac_sensor_settings.h"
#include "ac_sensor_sim.h"
#include "multi.h"
#include "multi_bridge.h"
#include "settings.h"

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

#if TARGET_ccgx
	VBusItems::setConnectionType(QDBusConnection::SystemBus);
#endif

	Settings settings;

	AcSensor em("/dev/ttyUSB0", 1);
	AcSensorSettings emSettings(0, "B01202");
	AcSensorBridge emBridge(&em, &emSettings, &settings);

	Multi multi;
	AcSensorSim emSim(&em, &multi);

	MultiBridge multiBridge(&multi, "ttyO1");

	ControlLoop cl(&multi, PhaseL1, &em, &settings);

	return app.exec();
}
