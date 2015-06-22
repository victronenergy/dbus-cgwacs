#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include <QByteArray>
#include <QList>
#include <QMetaType>
#include <QMutex>
#include <QObject>
extern "C" {
	#include <velib/platform/serial.h>
}
#include "crc16.h"

class QTimer;

Q_DECLARE_METATYPE(QList<quint16>)

/*!
 * Partial implementation of the Modbus RTU protocol.
 *
 * Supported functions: `ReadHoldingRegisters`, `ReadInputRegisters`,
 * and `WriteSingleRegister`.
 *
 * Communication is implemented asynchronously. It is allowed to add multiple
 * request at once. They will be queued and sent to the device whenever it is
 * ready (ie. all previous requests have been handled).
 */
class ModbusRtu : public QObject
{
	Q_OBJECT
public:
	enum FunctionCode
	{
		ReadCoils						= 1,
		ReadDiscreteInputs				= 2,
		ReadHoldingRegisters			= 3,
		ReadInputRegisters				= 4,
		WriteSingleCoil					= 5,
		WriteSingleRegister				= 6,
		WriteMultipleCoils				= 15,
		WriteMultipleRegisters			= 16,
		ReadFileRecord					= 20,
		WriteFileRecord					= 21,
		MaskWriteRegister				= 22,
		ReadWriteMultipleRegisters		= 23,
		ReadFIFOQueue					= 24,
		EncapsulatedInterfaceTransport	= 43,
	};

	enum ExceptionCode
	{
		NoExeption							= 0,
		IllegalFunction						= 1,
		IllegalDataAddress					= 2,
		IllegalDataValue					= 3,
		SlaveDeviceFailure					= 4,
		Acknowledge							= 5,
		SlaveDeviceBusy						= 6,
		MemoryParityError					= 7,
		GatewayPathUnavailable				= 10,
		GatewayTargetDeviceFailedToRespond	= 11
	};

	enum ErrorType {
		CrcError,
		Timeout,
		Exception,
		Unsupported
	};

	ModbusRtu(const QString &portName, int baudrate, QObject *parent = 0);

	~ModbusRtu();

	void readRegisters(FunctionCode function, quint8 slaveAddress,
					   quint16 startReg, quint16 count);

	void writeRegister(FunctionCode function, quint8 slaveAddress,
					   quint16 reg, quint16 value);

signals:
	void readCompleted(int function, quint8 slaveAddress, const QList<quint16> &values);

	void writeCompleted(int function, quint8 slaveAddress, quint16 address, quint16 value);

	void errorReceived(int errorType, quint8 slaveAddress, int exception);

	void serialEvent(const char *description);

private slots:
	void onTimeout();

	void processPacket();

private:
	void handleByteRead(quint8 b);

	void resetStateEngine();

	void processPending();

	void _readRegisters(FunctionCode function, quint8 slaveAddress,
						quint16 startReg, quint16 count);

	void _writeRegister(FunctionCode function, quint8 slaveAddress,
						quint16 reg, quint16 value);

	void send(QByteArray &data);

	static void onDataRead(struct VeSerialPortS *port, const quint8 *buffer,
						   quint32 length);

	static void onSerialEvent(struct VeSerialPortS *port, VeSerialEvent event,
							  char const *desc);

	enum ReadState {
		Idle,
		Address,
		Function,
		ByteCount,
		StartAddressMsb,
		StartAddressLsb,
		Data,
		CrcMsb,
		CrcLsb,
		Process
	};

	VeSerialPort mSerialPort;
	QByteArray mPortName;
	QTimer *mTimer;
	QMutex mMutex;
	struct Cmd {
		ModbusRtu::FunctionCode function;
		quint8 slaveAddress;
		quint16 reg;
		quint16 value;
	};
	QList<Cmd> mPendingCommands;
	uint8_t mCurrentSlave;

	// State engine
	ReadState mState;
	FunctionCode mFunction;
	uint8_t mCount;
	quint16 mStartAddress;
	quint16 mCrc;
	Crc16 mCrcBuilder;
	bool mAddToCrc;
	QByteArray mData;
};

#endif // MODBUS_RTU_H
