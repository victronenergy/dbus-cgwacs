#include <QDebug>
#include <QFile>
#include <QStringList>
#include <QTimer>
#include "logger.h"
#include "variables.h"

static const char *Monitors[] = {
	// "W L1", // "V L1-N", "A L1" // EM24
	"W L1 inst" // EM 340
	// /* "V L2-N", */ "V L3-N" // A1 and W1 (?) on EM 112
};

static const int MonitorCount = sizeof(Monitors)/sizeof(Monitors[0]);

Logger::Logger(int &argc, char **argv):
	QCoreApplication(argc, argv),
	mCsvOut(0),
	mTimer(new QTimer(this)),
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

	mModbus = new ModbusRtu(device, 9600, 1, this);
	connect(mModbus, SIGNAL(readCompleted(int, QList<quint16>)),
			this, SLOT(onReadCompleted(int, QList<quint16>)));
	connect(mModbus, SIGNAL(writeCompleted(int, quint16, quint16)),
			this, SLOT(onWriteCompleted(int, quint16, quint16)));
	connect(mModbus, SIGNAL(errorReceived(int, int)),
			this, SLOT(onErrorReceived(int, int)));
	connect(mTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
	// mTimer->setInterval(200);
	// mTimer->start();
	mStopwatch.start();

	startNext();
}

void Logger::onErrorReceived(int errorType, int exception)
{
	(*mCsvOut) << '\n';
	qDebug() << __FUNCTION__ << errorType << exception;
	// printf("<\n");
	++mIndex;
	// mIndex = 0;
	startNext();
}

void Logger::onReadCompleted(int function,
						   const QList<quint16> &values)
{
	Q_UNUSED(function)
	double elapsed = mStopwatch.elapsed() * 1e-3;
	const Variable *v0 = Variables::get(mIndex);
	// const Variable *v0 = Variables::find(Monitors[mIndex]);
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

void Logger::onTimeout()
{
	// startNext();
	// (*mCsvOut) << mStopwatch.elapsed() * 1e-3;
	// 0x00: V L1-N
	// 0x12: W L1
	// 0x32: PF L1 (power factor)

	// 0x0302 Version (type) code
	// 0x1300 - 0x1306 serial number (13 characters)
	// mModbus->readHoldingRegisters(0x12, 0x2);
}

void Logger::startNext()
{
//	if (mIndex == MonitorCount)
//	{
//		mIndex = 0;
//		// putchar('\n');
//		// qDebug().nospace() << '\n';
//	}

	const Variable *v0 = Variables::get(mIndex);
	// const Variable *v0 = Variables::find(Monitors[mIndex]);
	if (v0 == 0) {
		exit(0);
		return;
	}
	(*mCsvOut) << mStopwatch.elapsed() * 1e-3;
	// qDebug() << __FUNCTION__ << mIndex;
	mModbus->readRegisters(ReadHoldingRegisters, v0->address, v0->size / 2);
}

extern "C"
{
void pltExit(int ret)
{
	QCoreApplication::exit(ret);
}
}
