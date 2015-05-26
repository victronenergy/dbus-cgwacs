#include <QCoreApplication>
#include <QDebug>
#include <QsLog.h>
#include <velib/qt/v_busitems.h>
#include "control_loop.h"
#include "multi_bridge.h"
#include "multi_sim_bridge.h"
#include "multi.h"
#include "multi_sim.h"

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

#if TARGET_ccgx
	VBusItems::setConnectionType(QDBusConnection::SystemBus);
#endif

	QsLogging::Logger &logger = QsLogging::Logger::instance();
	logger.setLoggingLevel(QsLogging::DebugLevel);

	MultiSim multiSim;
	MultiSimBridge multiSimBridge(&multiSim, "ttyO1");

	Multi multi;

	MultiBridge multiBridge(&multi, "ttyO1");

	return app.exec();
}
