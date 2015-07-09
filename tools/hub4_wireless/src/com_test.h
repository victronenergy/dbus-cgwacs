#ifndef COMTEST_H
#define COMTEST_H

#include <QObject>

class QSerialPort;
class QTimer;

class ComTest : public QObject
{
	Q_OBJECT
public:
	ComTest(const QString &serialPort1, const QString serialPort2, QObject *parent = 0);

	void start();

signals:
	void finished(bool ok);

private slots:
	void onReadyRead();

	void onTimer();

private:
	QSerialPort *mPort1;
	QSerialPort *mPort2;
	QTimer *mTimer;
	int mRetryCount;
	int mMessageCount;
};

#endif // COMTEST_H
