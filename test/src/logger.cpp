#include <QDebug>
#include <QFile>
#include <QStringList>
#include <QTimer>
#include "logger.h"
#include "modbus_rtu.h"
#include "variables.h"

Logger::Logger(int &argc, char **argv):
	QCoreApplication(argc, argv),
	mCsvOut(0),
	mModbus(0),
	mIndex(0)
{
	qRegisterMetaType<QList<quint16> >();

	QStringList args = arguments();
	QString device = "/dev/ttyUSB0";
	if (args.size() > 1)
		device = arguments()[1];
	qDebug() << "device" << device;

	QFile *csv = new QFile("out.csv", this);
	if (!csv->open(QFile::WriteOnly | QFile::Truncate)) {
		qDebug() << "Cannot open file";
	}
	mCsvOut = new QTextStream(csv);

	mModbus = new ModbusRtu(device, 9600, this);
	connect(mModbus, SIGNAL(readCompleted(int, QList<quint16>)),
			this, SLOT(onReadCompleted(int, QList<quint16>)));
	connect(mModbus, SIGNAL(writeCompleted(int, quint16, quint16)),
			this, SLOT(onWriteCompleted(int, quint16, quint16)));
	connect(mModbus, SIGNAL(errorReceived(int, int)),
			this, SLOT(onErrorReceived(int, int)));
	mStopwatch.start();

	startNext();
}

void Logger::onErrorReceived(int errorType, int exception)
{
	(*mCsvOut) << '\n';
	qDebug() << __FUNCTION__ << errorType << exception;
	++mIndex;
	startNext();
}

void Logger::onReadCompleted(int function,
						   const QList<quint16> &values)
{
	Q_UNUSED(function)
	double elapsed = mStopwatch.elapsed() * 1e-3;
	const Variable *v0 = Variables::get(mIndex);
	double value = v0->getValue(values, 0);
	(*mCsvOut) << "," << elapsed
			   << "," << v0->address
			   << "," << v0->name
			   << "," << value
			   << '\n';
	mCsvOut->flush();
	qDebug() << elapsed << '\t'
			 << mIndex << '\t'
			 << v0->address << '\t'
			 << value << '\t'
			 << v0->name;
	++mIndex;
	startNext();
}

void Logger::onWriteCompleted(int function, quint16 address, quint16 value)
{
	qDebug() << __FUNCTION__ << function << address << value;
	startNext();
}

void Logger::startNext()
{
	const Variable *v0 = Variables::get(mIndex);
	if (v0 == 0) {
		exit(0);
		return;
	}
	(*mCsvOut) << mStopwatch.elapsed() * 1e-3;
	mModbus->readRegisters(ModbusRtu::ReadHoldingRegisters, 1, v0->address, v0->size / 2);
}

extern "C"
{
void pltExit(int ret)
{
	QCoreApplication::exit(ret);
}
}
