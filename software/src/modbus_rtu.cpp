#include <QSocketNotifier>
#include <QTimer>
#include <unistd.h>
#include "defines.h"
#include "modbus_rtu.h"

ModbusRtu::ModbusRtu(const QString &portName, int baudrate, int timeout, QObject *parent):
	QObject(parent),
	mSerialPort(veSerialAllocate(portName.toLatin1().data())),
	mTimer(new QTimer(this)),
	mCurrentSlave(0)
{
	veSerialSetBaud(mSerialPort, static_cast<un32>(baudrate));
	veSerialSetKind(mSerialPort, 0); // Requires external event pump
	veSerialOpen(mSerialPort, 0);

	QSocketNotifier *readNotifier =
		new QSocketNotifier(mSerialPort->fh, QSocketNotifier::Read, this);
	connect(readNotifier, SIGNAL(activated(int)), this, SLOT(onReadyRead()));

	QSocketNotifier *errorNotifier =
		new QSocketNotifier(mSerialPort->fh, QSocketNotifier::Exception, this);
	connect(errorNotifier, SIGNAL(activated(int)), this, SLOT(onError()));

	mData.reserve(16);

	resetStateEngine();
	mTimer->setInterval(timeout);
	connect(mTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

ModbusRtu::~ModbusRtu()
{
	veSerialClose(mSerialPort);
	VeSerialPortFree(mSerialPort);
}

void ModbusRtu::readRegisters(FunctionCode function, quint8 slaveAddress, quint16 startReg,
							  quint16 count)
{
	if (mState == Idle) {
		_readRegisters(function, slaveAddress, startReg, count);
	} else {
		Cmd cmd;
		cmd.function = function;
		cmd.slaveAddress = slaveAddress;
		cmd.reg = startReg;
		cmd.value = count;
		mPendingCommands.append(cmd);
	}
}

void ModbusRtu::writeRegister(FunctionCode function, quint8 slaveAddress, quint16 reg,
							  quint16 value)
{
	if (mState == Idle) {
		_writeRegister(function, slaveAddress, reg, value);
	} else {
		Cmd cmd;
		cmd.function = function;
		cmd.slaveAddress = slaveAddress;
		cmd.reg	= reg;
		cmd.value = value;
		mPendingCommands.append(cmd);
	}
}

void ModbusRtu::onTimeout()
{
	if (mState == Idle)
		return;
	emit errorReceived(Timeout, mCurrentSlave, 0);
	resetStateEngine();
	processPending();
}

void ModbusRtu::processPacket()
{
	if (mCrc != mCrcBuilder.getValue()) {
		emit errorReceived(CrcError, mCurrentSlave, 0);
		return;
	}
	if ((mFunction & 0x80) != 0) {
		quint8 errorCode = static_cast<quint8>(mData[0]);
		emit errorReceived(Exception, mCurrentSlave, errorCode);
		return;
	}
	if (mState == Function) {
		emit errorReceived(Unsupported, mCurrentSlave, mFunction);
		return;
	}
	switch (mFunction) {
	case ReadHoldingRegisters:
	case ReadInputRegisters:
	{
		QList<quint16> registers;
		for (int i=0; i<mData.length(); i+=2) {
			registers.append(toUInt16(mData, i));
		}
		emit readCompleted(mFunction, mCurrentSlave, registers);
		return;
	}
	case WriteSingleRegister:
	{
		quint16 value = toUInt16(mData, 0);
		emit writeCompleted(mFunction, mCurrentSlave, mStartAddress, value);
		return;
	}
	default:
		break;
	}
}

void ModbusRtu::onReadyRead()
{
	quint8 buf[64];
	bool first = true;
	for (;;) {
		ssize_t len = read(mSerialPort->fh, buf, sizeof(buf));
		if (len < 0) {
			emit serialEvent("serial read failure");
			return;
		}
		if (first && len == 0) {
			emit serialEvent("Ready for reading but read 0 bytes. Device removed?");
			return;
		}
		for (ssize_t i = 0; i<len; ++i)
			handleByteRead(buf[i]);
		if (len < static_cast<int>(sizeof(buf)))
			break;
		first = false;
	}
}

void ModbusRtu::onError()
{
	emit serialEvent("Serial error");
}

void ModbusRtu::handleByteRead(quint8 b)
{
	if (mAddToCrc)
		mCrcBuilder.add(b);
	switch (mState) {
	case Idle:
		// We received data when we were not expecting any. Ignore the data.
		break;
	case Address:
		if (b == mCurrentSlave)
			mState = Function;
		break;
	case Function:
		mFunction = static_cast<FunctionCode>(b);
		if ((mFunction & 0x80) != 0) {
			// Exception
			mCount = 1;
			mState = Data;
		} else {
			switch (mFunction) {
			case ReadHoldingRegisters:
			case ReadInputRegisters:
				mState = ByteCount;
				break;
			case WriteSingleRegister:
				mState = StartAddressMsb;
				break;
			default:
				mState = Address;
				break;
			}
		}
		break;
	case ByteCount:
		mCount = b;
		if (mCount == 0) {
			mState = CrcMsb;
			mAddToCrc = false;
		} else {
			mState = Data;
		}
		break;
	case StartAddressMsb:
		mStartAddress = static_cast<quint16>(b << 8);
		mState = StartAddressLsb;
		break;
	case StartAddressLsb:
		mStartAddress |= b;
		mCount = 2;
		mState = Data;
		break;
	case Data:
		if (mCount > 0) {
			mData.append(static_cast<char>(b));
			--mCount;
		}
		if (mCount == 0) {
			mState = CrcMsb;
			mAddToCrc = false;
		}
		break;
	case CrcMsb:
		mCrc = static_cast<quint16>(b << 8);
		mState = CrcLsb;
		break;
	case CrcLsb:
		mCrc |= b;
		processPacket();
		resetStateEngine();
		processPending();
		break;
	}
}

void ModbusRtu::resetStateEngine()
{
	mState = Idle;
	mCrcBuilder.reset();
	mAddToCrc = true;
	mCurrentSlave = 0;
	mData.clear();
	mTimer->stop();
}

void ModbusRtu::processPending()
{
	if (mPendingCommands.isEmpty())
		return;
	const Cmd &cmd = mPendingCommands.first();
	switch (cmd.function) {
	case ReadHoldingRegisters:
	case ReadInputRegisters:
		_readRegisters(cmd.function, cmd.slaveAddress, cmd.reg, cmd.value);
		break;
	case WriteSingleRegister:
		_writeRegister(cmd.function, cmd.slaveAddress, cmd.reg, cmd.value);
		break;
	default:
		break;
	}
	mPendingCommands.removeFirst();
}

void ModbusRtu::_readRegisters(ModbusRtu::FunctionCode function, quint8 slaveAddress,
							   quint16 startReg, quint16 count)
{
	Q_ASSERT(mState == Idle);
	QByteArray frame;
	frame.reserve(8);
	frame.append(static_cast<char>(slaveAddress));
	frame.append(static_cast<char>(function));
	frame.append(static_cast<char>(msb(startReg)));
	frame.append(static_cast<char>(lsb(startReg)));
	frame.append(static_cast<char>(msb(count)));
	frame.append(static_cast<char>(lsb(count)));
	send(frame);
}

void ModbusRtu::_writeRegister(ModbusRtu::FunctionCode function, quint8 slaveAddress, quint16 reg,
							   quint16 value)
{
	Q_ASSERT(mState == Idle);
	QByteArray frame;
	frame.reserve(8);
	frame.append(static_cast<char>(slaveAddress));
	frame.append(static_cast<char>(function));
	frame.append(static_cast<char>(msb(reg)));
	frame.append(static_cast<char>(lsb(reg)));
	frame.append(static_cast<char>(msb(value)));
	frame.append(static_cast<char>(lsb(value)));
	send(frame);
}

void ModbusRtu::send(QByteArray &data)
{
	Q_ASSERT(mState == Idle);
	quint16 crc = Crc16::getValue(data);
	data.append(static_cast<char>(msb(crc)));
	data.append(static_cast<char>(lsb(crc)));
	// Modbus requires a pause between sending of 3.5 times the interval needed
	// to send a character. We use 4 characters here, just in case...
	// We also assume 10 bits per caracter (8 data bits, 1 stop bit and 1 parity
	// bit). Keep in mind that overestimating the character time does not hurt
	// (a lot), but underestimating does.
	// Then number of bits devided by the the baudrate (unit: bits/second) gives
	// us the time in seconds. usleep wants time in microseconds, so we have to
	// multiply by 1 million.
	usleep((4 * 10 * 1000 * 1000) / mSerialPort->baudrate);
	veSerialPutBuf(mSerialPort, reinterpret_cast<un8 *>(data.data()),
				   static_cast<un32>(data.size()));
	mTimer->start();
	mState = Address;
	mCurrentSlave = static_cast<quint8>(data[0]);
}
