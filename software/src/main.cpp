#include <QCoreApplication>
#include <QsLog.h>
#include <QStringList>
#include <QDBusMessage>
#include <QEventLoop>
#include <QTimer>
#include <unistd.h>
#include <veutil/qt/ve_qitem.hpp>
#include <veutil/qt/ve_qitems_dbus.hpp>
#include <veutil/qt/ve_qitem_exported_dbus_services.hpp>
#include "dbus_bridge.h"
#include "ac_sensor.h"
#include "ac_sensor_mediator.h"

bool initDBus(QDBusConnection &dbus)
{
	// Calling GetDefault on /Settings should return -1. It's a simple way
	// to see if localsettings is up without relying on a particular path.
	QDBusMessage m = QDBusMessage::createMethodCall("com.victronenergy.settings",
		"/Settings", "org.freedesktop.DBus.Introspectable", "Introspect");

	QLOG_INFO() << "Wait for local settings on DBus... ";

	for (int i=0; i<10; i++) {
		QDBusMessage reply = dbus.call(m);
		if (reply.type() == QDBusMessage::ReplyMessage) {
			QLOG_INFO() << "Local settings found";
			return true;
		}

		QLOG_INFO() << "Waiting...";

		QEventLoop l;
		QTimer::singleShot(2000, &l, SLOT(quit()));
		l.exec();
	}
	return false;
}

void initLogger(QsLogging::Level logLevel)
{
	QsLogging::Logger &logger = QsLogging::Logger::instance();
	QsLogging::DestinationPtr debugDestination(
			QsLogging::DestinationFactory::MakeDebugOutputDestination());
	logger.addDestination(debugDestination);
	logger.setIncludeTimestamp(false);

	QLOG_INFO() << "dbus-cgwacs" << "v" VERSION << "started";
	QLOG_INFO() << "Built with Qt" << QT_VERSION_STR << "running on" << qVersion();
	QLOG_INFO() << "Built on" << __DATE__ << "at" << __TIME__;
	logger.setLoggingLevel(logLevel);
}

extern "C"
{
// This function is called by the serial port API from velib when the device is
// disconnected from the serial port.
void pltExit(int ret)
{
	QCoreApplication::exit(ret);
}
}

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);
	app.setApplicationVersion(VERSION);
	app.setApplicationName(app.arguments()[0]);

	initLogger(QsLogging::InfoLevel);

	bool isZigbee = false;
	QString portName;
	QString dbusAddress = "system";
	int timeout = 250;
	QStringList args = app.arguments();
	args.pop_front();

	while (!args.isEmpty()) {
		QString arg = args.takeFirst();

		if (arg == "-h" || arg == "--help") {
			QLOG_INFO() << app.arguments().first();
			QLOG_INFO() << "\t-h, --help";
			QLOG_INFO() << "\t Show this message.";
			QLOG_INFO() << "\t-V, --version";
			QLOG_INFO() << "\t Show the application version.";
			QLOG_INFO() << "\t-b, --dbus";
			QLOG_INFO() << "\t dbus address or 'session' or 'system'";
			QLOG_INFO() << "\t-d level, --debug level";
			QLOG_INFO() << "\t Set log level";
			QLOG_INFO() << "\t--timeout milliseconds";
			QLOG_INFO() << "\t Timeout in milliseconds for RS485 responses";
			QLOG_INFO() << "\t <Port Name>";
			QLOG_INFO() << "\t Name of communication port (eg. /dev/ttyUSB0)";
			exit(1);
		} else if (arg == "-V" || arg == "--version") {
			QLOG_INFO() << VERSION;
			exit(0);
		} else if (arg == "-d" || arg == "--debug") {
			if (!args.isEmpty()) {
				arg = args.takeFirst();
				QsLogging::Logger &logger = QsLogging::Logger::instance();
				QsLogging::Level logLevel = static_cast<QsLogging::Level>(qBound(
					static_cast<int>(QsLogging::TraceLevel),
					arg.toInt(),
					static_cast<int>(QsLogging::OffLevel)));
				logger.setLoggingLevel(logLevel);
			}
		} else if (arg == "-t" || arg == "--timestamp") {
			QsLogging::Logger &logger = QsLogging::Logger::instance();
			logger.setIncludeTimestamp(true);
		} else if (arg == "--timeout") {
			if (!args.isEmpty())
				timeout = qBound(150, args.takeFirst().toInt(), 10000);
		} else if (arg == "-b" || arg == "--dbus") {
			if (!args.isEmpty())
				dbusAddress = args.takeFirst();
		} else if (arg == "-z" || arg == "--zigbee") {
			timeout = qMax(2000, timeout);
			isZigbee = true;
		} else if (!arg.startsWith('-')) {
			portName = arg;
		}
	}

	if (portName.isEmpty()) {
		QLOG_ERROR() << "No communication port specified on command line";
		exit(2);
	} else {
		QLOG_INFO() << "Connecting to" << portName;
	}

	VeQItemDbusProducer producer(VeQItems::getRoot(), "sub", false, false);
	producer.setAutoCreateItems(false);
	producer.open(dbusAddress);

	BridgeItemProducer dbusExportProducer(VeQItems::getRoot(), "pub");
	VeQItemExportedDbusServices publisher(dbusExportProducer.services());
	publisher.open(dbusAddress);

	qRegisterMetaType<ConnectionState>();

	if (!initDBus(producer.dbusConnection())) {
		return 1; // Not success
	}

	VeQItem *settingsRoot = VeQItems::getRoot()->itemGetOrCreate("sub/com.victronenergy.settings", false);
	AcSensorMediator m(portName, timeout, isZigbee, settingsRoot);

	app.connect(&m, SIGNAL(connectionLost()), &app, SLOT(quit()));
	app.connect(&m, SIGNAL(serialEvent(const char *)), &app, SLOT(quit()));

	return app.exec();
}
