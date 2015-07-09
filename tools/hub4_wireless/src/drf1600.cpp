#include <QTimer>
#include <unistd.h>
#include "defines.h"
#include "drf1600.h"

Drf1600::Drf1600(const QString &portName, QObject *parent):
	QObject(parent),
	mCurrentCommand(Idle),
	mBytesExpected(0),
	mSerialPort(new QSerialPort(this)),
	mTimer(new QTimer(this))
{
	mSerialPort->setPortName(portName);
	mSerialPort->open(QSerialPort::ReadWrite);
	connect(mSerialPort, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
	connect(mSerialPort, SIGNAL(error(QSerialPort::SerialPortError)),
			this, SIGNAL(error(QSerialPort::SerialPortError)));

	mTimer->setInterval(250);
	mTimer->setSingleShot(true);
	connect(mTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

int Drf1600::baudRate() const
{
	return mSerialPort->baudRate();
}

QString Drf1600::portName() const
{
	return mSerialPort->portName();
}

void Drf1600::setBaudRate(int r)
{
	mSerialPort->setBaudRate(r);
}

void Drf1600::readShortAddress()
{
	static const quint8 Msg[] = { 0xFC, 0x00, 0x91, 0x04, 0xC4, 0xD4 };
	send(ReadShortAddress, 2, Msg, sizeof(Msg)/sizeof(Msg[0]));
}

void Drf1600::readPanId()
{
	static const quint8 Msg[] = { 0xFC, 0x00, 0x91, 0x03, 0xA3, 0xB3 };
	send(ReadPanId, 2, Msg, sizeof(Msg)/sizeof(Msg[0]));
}

void Drf1600::writePanId(quint16 panId)
{
	quint8 Msg[6] = { 0xFC, 0x02, 0x91, 0x01 };
	Msg[4] = msb(panId);
	Msg[5] = lsb(panId);
	send(WritePanId, 2, Msg, sizeof(Msg)/sizeof(Msg[0]));
}

void Drf1600::writeBaudrate(int baudrate)
{
	quint8 b = 0;
	switch (baudrate) {
	case 9600:
		b = 1;
		break;
	case 19200:
		b = 2;
		break;
	case 38400:
		b = 3;
		break;
	case 57600:
		b = 4;
		break;
	case 115200:
		b = 5;
		break;
	default:
		b = 0;
		break;
	}
	quint8 Msg[6] = { 0xFC, 0x01, 0x91, 0x06, 0x00, 0xF6 };
	Msg[4] = b;
	send(SetBaudRate, 6, Msg, sizeof(Msg)/sizeof(Msg[0]));
}

void Drf1600::readDeviceType()
{
	quint8 Msg[6] = { 0xFC, 0x00, 0x91, 0x0B, 0xCB, 0xEB };
	send(GetDeviceType, 6, Msg, sizeof(Msg)/sizeof(Msg[0]));
}

void Drf1600::writeDeviceType(ZigbeeDeviceType type)
{
	switch (type) {
	case ZigbeeCoordinator:
	{
		quint8 Msg[6] = { 0xFC, 0x00, 0x91, 0x09, 0xA9, 0xC9 };
		send(SetDeviceType, 8, Msg, sizeof(Msg)/sizeof(Msg[0]));
		break;
	}
	case ZigbeeRouter:
	{
		quint8 Msg[6] = { 0xFC, 0x00, 0x91, 0x0A, 0xBA, 0xDA };
		send(SetDeviceType, 8, Msg, sizeof(Msg)/sizeof(Msg[0]));
		break;
	}
	default:
		Q_ASSERT_X(false, "setDeviceType", "Device type not supported");
		break;
	}
}

void Drf1600::restart()
{
	quint8 Msg[6] = { 0xFC, 0x00, 0x91, 0x87, 0x6A, 0x35 };
	send(Restart, 0, Msg, sizeof(Msg)/sizeof(Msg[0]));
}

void Drf1600::testBaudrate()
{
	quint8 Msg[6] = { 0xFC, 0x00, 0x91, 0x07, 0x97, 0xA7 };
	send(TestBaudrate, 8, Msg, sizeof(Msg)/sizeof(Msg[0]));
}

void Drf1600::onTimeout()
{
	resetStateEngine();
	emit errorReceived(Drf1600Timeout);
}

void Drf1600::onReadyRead()
{
	if (mCurrentCommand == Idle) {
		mSerialPort->read(mSerialPort->bytesAvailable());
		return;
	}
	if (mSerialPort->bytesAvailable() < mBytesExpected)
		return;
	processPacket(mSerialPort->read(mSerialPort->bytesAvailable()));
}

void Drf1600::send(Command command, int bytesExpected, const quint8 *buffer,
				   int length)
{
	Q_ASSERT(mCurrentCommand == Idle);
	mCurrentCommand = command;
	mBytesExpected = bytesExpected;
	quint8 total = 0;
	for (int i=0; i<length; ++i)
		total += buffer[i];
	usleep(10000);
	mSerialPort->write((const char *)buffer, length);
	mSerialPort->write((const char *)&total, 1);
	if (bytesExpected == 0)
		mCurrentCommand = Idle;
	else
		mTimer->start();
}

bool Drf1600::checkReply(const QByteArray &data, const quint8 *buffer, int length)
{
	if (data.size() != length)
		return false;
	for (int i=0; i<length; ++i) {
		if (data[i] != buffer[i])
			return false;
	}
	return true;
}

void Drf1600::resetStateEngine()
{
	mCurrentCommand = Idle;
	mSerialPort->read(mSerialPort->bytesAvailable());
	mTimer->stop();
}

void Drf1600::processPacket(const QByteArray &dataRead)
{
	Command cmd = mCurrentCommand;
	resetStateEngine();
	switch (cmd) {
	case GetDeviceType:
	{
		int c = toUInt16(dataRead[0], dataRead[2]);
		switch (c)
		{
		case 0x436F:
			emit deviceTypeRetrieved(ZigbeeCoordinator);
			break;
		case 0x5275:
			emit deviceTypeRetrieved(ZigbeeRouter);
			break;
		default:
			emit errorReceived(Drf1600IncorrectReply);
			break;
		}
		break;
	}
	case ReadPanId:
		emit panIdRead(toUInt16(dataRead[0], dataRead[1]));
		break;
	case ReadShortAddress:
		emit shortAddressRead(toUInt16(dataRead[0], dataRead[1]));
		break;
	case SetBaudRate:
		/// @todo EV Check retrieved data
		emit baudrateSet();
		break;
	case SetDeviceType:
		static quint8 MsgCoord[] = { 0x43, 0x6F, 0x6F, 0x72, 0x64, 0x3B, 0x00, 0x19 };
		static quint8 MsgRouter[] = { 0x52, 0x6F, 0x75, 0x74, 0x65, 0x3B, 0x00, 0x19 };
		if (checkReply(dataRead, MsgCoord, sizeof(MsgCoord)/sizeof(MsgCoord[0])) ||
			checkReply(dataRead, MsgRouter, sizeof(MsgRouter)/sizeof(MsgRouter[0]))) {
			emit deviceTypeSet();
		} else {
			emit errorReceived(Drf1600IncorrectReply);
		}
		break;
	case TestBaudrate:
		emit baudrateTested(toUInt16(dataRead[6], dataRead[7]));
		break;
	case WritePanId:
		emit panIdWritten(toUInt16(dataRead[0], dataRead[1]));
		break;
	default:
		break;
	}
}
