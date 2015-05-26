#ifndef LOGGER_H
#define LOGGER_H

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QMetaType>
#include "modbus_rtu.h"

class QTextStream;
class QTimer;

class Logger : public QCoreApplication
{
	Q_OBJECT
public:
	Logger(int &argc, char **argv);

private slots:
	void onErrorReceived(int errorType, int exception);

	void onReadCompleted(int function, const QList<quint16> &values);

	void onWriteCompleted(int function, quint16 address, quint16 value);

	void onTimeout();

private:
	void startNext();

	QTextStream *mCsvOut;
	QTimer *mTimer;
	QElapsedTimer mStopwatch;
	ModbusRtu *mModbus;

	int mIndex;
};

#endif // LOGGER_H
