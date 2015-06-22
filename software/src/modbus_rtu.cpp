#include <QMutexLocker>
#include <QTimer>
#include <unistd.h>
#include "defines.h"
#include "modbus_rtu.h"

ModbusRtu::ModbusRtu(const QString &portName, int baudrate,
					 QObject *parent):
	QObject(parent),
	mPortName(portName.toLatin1()),
	mTimer(new QTimer(this)),
	mCurrentSlave(0)
{
	memset(&mSerialPort, 0, sizeof(mSerialPort));
	// The pointer returned by mPortName.data() will remain valid as long as
	// mPortName exists and is not changed.
	mSerialPort.dev = mPortName.data();
	mSerialPort.baudrate = baudrate;
	mSerialPort.intLevel = 2;
	mSerialPort.rxCallback = onDataRead;
	mSerialPort.eventCallback = onSerialEvent;
	veSerialOpen(&mSerialPort, this);

	mData.reserve(16);

	resetStateEngine();
	mTimer->setInterval(250);
	connect(mTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

ModbusRtu::~ModbusRtu()
{
	veSerialClose(&mSerialPort);
}

void ModbusRtu::readRegisters(FunctionCode function, quint8 slaveAddress,
							  quint16 startReg, quint16 count)
{
	QMutexLocker lock(&mMutex);
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

void ModbusRtu::writeRegister(FunctionCode function, quint8 slaveAddress,
							  quint16 reg, quint16 value)
{
	QMutexLocker lock(&mMutex);
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
	QMutexLocker lock(&mMutex);
	if (mState == Idle || mState == Process)
		return;
	int cs = mCurrentSlave;
	resetStateEngine();
	processPending();
	mMutex.unlockInline();
	emit errorReceived(Timeout, cs, 0);
}

void ModbusRtu::processPacket()
{
	QMutexLocker lock(&mMutex);
	int cs = mCurrentSlave;
	if (mCrc != mCrcBuilder.getValue()) {
		resetStateEngine();
		processPending();
		mMutex.unlockInline();
		emit errorReceived(CrcError, cs, 0);
		return;
	}
	if ((mFunction & 0x80) != 0) {
		quint8 errorCode = mData[0];
		resetStateEngine();
		processPending();
		mMutex.unlockInline();
		emit errorReceived(Exception, cs, errorCode);
		return;
	}
	if (mState == Function) {
		FunctionCode function = mFunction;
		resetStateEngine();
		processPending();
		mMutex.unlockInline();
		emit errorReceived(Unsupported, cs, function);
		return;
	}
	FunctionCode function = mFunction;
	switch (function) {
	case ReadHoldingRegisters:
	case ReadInputRegisters:
	{
		QList<quint16> registers;
		for (int i=0; i<mData.length(); i+=2) {
			registers.append(toUInt16(mData[i], mData[i + 1]));
		}
		resetStateEngine();
		processPending();
		mMutex.unlockInline();
		emit readCompleted(function, cs, registers);
		return;
	}
	case WriteSingleRegister:
	{
		quint16 value = toUInt16(mData[0], mData[1]);
		quint16 startAddress = mStartAddress;
		resetStateEngine();
		processPending();
		mMutex.unlockInline();
		emit writeCompleted(function, cs, startAddress, value);
		return;
	}
	default:
		break;
	}
}

void ModbusRtu::handleByteRead(quint8 b)
{
	if (mAddToCrc)
		mCrcBuilder.add(b);
	switch (mState) {
	case Idle:
	case Process:
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
		mStartAddress = b << 8;
		mState = StartAddressLsb;
		break;
	case StartAddressLsb:
		mStartAddress |= b;
		mCount = 2;
		mState = Data;
		break;
	case Data:
		if (mCount > 0) {
			mData.append(b);
			--mCount;
		}
		if (mCount == 0) {
			mState = CrcMsb;
			mAddToCrc = false;
		}
		break;
	case CrcMsb:
		mCrc = b << 8;
		mState = CrcLsb;
		break;
	case CrcLsb:
		mCrc |= b;
		mState = Process;
		QMetaObject::invokeMethod(this, "processPacket");
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

void ModbusRtu::_readRegisters(ModbusRtu::FunctionCode function,
							   quint8 slaveAddress, quint16 startReg,
							   quint16 count)
{
	Q_ASSERT(mState == Idle);
	QByteArray frame;
	frame.reserve(8);
	frame.append(slaveAddress);
	frame.append(function);
	frame.append(msb(startReg));
	frame.append(lsb(startReg));
	frame.append(msb(count));
	frame.append(lsb(count));
	send(frame);
}

void ModbusRtu::_writeRegister(ModbusRtu::FunctionCode function,
							   quint8 slaveAddress, quint16 reg, quint16 value)
{
	Q_ASSERT(mState == Idle);
	QByteArray frame;
	frame.reserve(8);
	frame.append(slaveAddress);
	frame.append(function);
	frame.append(msb(reg));
	frame.append(lsb(reg));
	frame.append(msb(value));
	frame.append(lsb(value));
	send(frame);
}

void ModbusRtu::send(QByteArray &data)
{
	Q_ASSERT(mState == Idle);
	quint16 crc = Crc16::getValue(data);
	data.append(msb(crc));
	data.append(lsb(crc));
	// Modbus requires a pause between sending of 3.5 times the interval needed
	// to send a character. We use 4 characters here, just in case...
	// We also assume 10 bits per caracter (8 data bits, 1 stop bit and 1 parity
	// bit). Keep in mind that overestimating the character time does not hurt
	// (a lot), but underestimating does.
	// Then number of bits devided by the the baudrate (unit: bits/second) gives
	// us the time in seconds. usleep wants time in microseconds, so we have to
	// multiply by 1 million.
	usleep((4 * 10 * 1000 * 1000) / mSerialPort.baudrate);
	veSerialPutBuf(&mSerialPort, (quint8 *)data.data(), data.size());
	mTimer->start();
	mState = Address;
	mCurrentSlave = static_cast<int>(data[0]);
}

void ModbusRtu::onDataRead(VeSerialPortS *port, const quint8 *buffer,
						   quint32 length)
{
	ModbusRtu *rtu = reinterpret_cast<ModbusRtu *>(port->ctx);
	QMutexLocker lock(&rtu->mMutex);
	for (quint32 i=0; i<length; ++i)
		rtu->handleByteRead(buffer[i]);
}

void ModbusRtu::onSerialEvent(VeSerialPortS *port, VeSerialEvent event,
							  const char *desc)
{
	Q_UNUSED(event);
	ModbusRtu *rtu = reinterpret_cast<ModbusRtu *>(port->ctx);
	emit rtu->serialEvent(desc);
}
