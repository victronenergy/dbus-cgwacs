#include <QsLog.h>
#include <QSerialPortInfo>
#include <QtQml>
#include "drf1600_detector.h"
#include "log_destination.h"
#include "hub4_wireless.h"

static const QString SetupCoordinator = "SetupCoordinator";
static const QString SetupRouter = "SetupRouter";
static const QString TestCommunication = "TestComs";
static const QString ComTestFailed = "ComTestFailed";
static const QString ComTestOk = "ComTestOk";

Hub4Wireless::Hub4Wireless(int &argc, char **argv) :
	QApplication(argc, argv)
{
	QsLogging::Logger &logger = QsLogging::Logger::instance();
	QsLogging::DestinationPtr debugDestination(
			QsLogging::DestinationFactory::MakeDebugOutputDestination());
	logger.addDestination(debugDestination);
	logger.addDestination(QsLogging::DestinationPtr(new LogDestination(this)));
	logger.setIncludeTimestamp(false);
}

QString Hub4Wireless::log() const
{
	return mLog;
}

QString Hub4Wireless::info() const
{
	return mInfo;
}

void Hub4Wireless::programCoordinator()
{
	QStringList serialPorts = findSerialPorts("Prolific",
											  "Prolific USB-to-Serial Comm Port");
	if (serialPorts.isEmpty()) {
		setInfo("CoordinatorNotFound");
		return;
	}
	foreach (QString serialPort, serialPorts) {
		Drf1600Detector *d = new Drf1600Detector(serialPort, this);
		d->setDeviceType(ZigbeeCoordinator);
		d->setAdjustSettings(true);
		connect(d, SIGNAL(finished(bool)),
				this, SLOT(onCoordinatorSetupFinished(bool)));
		d->start();
	}
	setInfo("Busy");
}

void Hub4Wireless::programRouter()
{
	QStringList serialPorts = findSerialPorts("FTDI", "USB Serial Port");
	if (serialPorts.isEmpty()) {
		setInfo("RouterNotFound");
		return;
	}
	foreach (QString serialPort, serialPorts) {
		Drf1600Detector *d = new Drf1600Detector(serialPort, this);
		d->setDeviceType(ZigbeeRouter);
		d->setAdjustSettings(true);
		connect(d, SIGNAL(finished(bool)),
				this, SLOT(onRouterSetupFinished(bool)));
		d->start();
	}
	setInfo("Busy");
}

void Hub4Wireless::writeToLog(const QString &message)
{
	setLog(log() + message + '\n');
}

void Hub4Wireless::onCoordinatorSetupFinished(bool s)
{
	Drf1600Detector *d = static_cast<Drf1600Detector *>(sender());
	d->deleteLater();
	if (findChildren<Drf1600Detector *>().size() <= 1) {
		if (s) {
			setInfo("CoordinatorOk");
		} else {
			setInfo("CoordinatorFailed");
		}
	}
}

void Hub4Wireless::onRouterSetupFinished(bool s)
{
	Drf1600Detector *d = static_cast<Drf1600Detector *>(sender());
	d->deleteLater();
	if (findChildren<Drf1600Detector *>().size() <= 1) {
		if (s) {
			setInfo("RouterOk");
		} else {
			setInfo("RouterFailed");
		}
	}
}

void Hub4Wireless::setInfo(const QString &t)
{
	if (mInfo == t)
		return;
	mInfo = t;
	emit infoChanged();
}

void Hub4Wireless::setLog(const QString &l)
{
	if (mLog == l)
		return;
	mLog = l;
	emit logChanged();
}

QStringList Hub4Wireless::findSerialPorts(const QString &manufacturer,
										 const QString &description)
{
	QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
	QStringList result;
	foreach (QSerialPortInfo p, ports) {
		QLOG_INFO() << p.portName() << p.description() << p.manufacturer();
		if (p.manufacturer() == manufacturer && p.description() == description) {
			result.append(p.portName());
		}
	}
	return result;
}
