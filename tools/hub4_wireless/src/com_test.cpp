#include <QsLog.h>
#include <QSerialPort>
#include <QTimer>
#include "com_test.h"

static const QString Request = "Ping";
static const QString Reply = "Pong";

ComTest::ComTest(const QString &serialPort1, const QString serialPort2,
				 QObject *parent) :
	QObject(parent),
	mPort1(new QSerialPort(this)),
	mPort2(new QSerialPort(this)),
	mTimer(new QTimer(this)),
	mRetryCount(0),
	mMessageCount(0)
{
	qDebug() << __FUNCTION__ << serialPort1 << serialPort2;

	mPort1->setPortName(serialPort1);
	mPort1->setBaudRate(9600);
	connect(mPort1, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

	mPort2->setPortName(serialPort2);
	mPort2->setBaudRate(9600);
	connect(mPort2, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

	mTimer = new QTimer(this);
	mTimer->setInterval(1000);
	connect(mTimer, SIGNAL(timeout()), this, SLOT(onTimer()));
}

void ComTest::start()
{
	QLOG_INFO() << "Start";
	mRetryCount = 0;
	mPort1->open(QSerialPort::ReadWrite);
	mPort2->open(QSerialPort::ReadWrite);
	mPort1->write(Request.toLatin1());
	mTimer->start();
}

void ComTest::onReadyRead()
{
	qDebug() << __FUNCTION__;
	if (sender() == mPort1) {
		if (mPort1->bytesAvailable() >= Reply.size()) {
			QByteArray r = mPort1->read(mPort1->bytesAvailable());
			if (r == Reply.toLatin1()) {
				mTimer->stop();
				QLOG_INFO() << "Roundtrip" << mMessageCount << "OK";
				++mMessageCount;
				if (mMessageCount == 10) {
					emit finished(true);
				} else {
					start();
				}
			} else {
				start();
			}
		}
	} else {
		if (mPort2->bytesAvailable() >= Request.size()) {
			QByteArray r = mPort2->read(mPort2->bytesAvailable());
			if (r == Request.toLatin1())
				mPort2->write(Reply.toLatin1());
			else
				start();
		}
	}
}

void ComTest::onTimer()
{
	QLOG_INFO() << "timeout";
	++mRetryCount;
	if (mRetryCount == 5) {
		mTimer->stop();
		emit finished(false);
	} else {
		mPort1->close();
		mPort2->close();
		start();
	}
}
