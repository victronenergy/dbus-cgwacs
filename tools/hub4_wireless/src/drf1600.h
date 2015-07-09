#ifndef DRF1600_H
#define DRF1600_H

#include <QObject>
#include <QSerialPort>

class QTimer;

enum ZigbeeDeviceType {
	ZigbeeCoordinator,
	ZigbeeRouter,
	ZigbeeEndPoint
};

enum Drf1600Error {
	Drf1600Timeout,
	Drf1600IncorrectReply
};

class Drf1600 : public QObject
{
	Q_OBJECT
public:
	Drf1600(const QString &portName, QObject *parent = 0);

	int baudRate() const;

	void setBaudRate(int r);

	QString portName() const;

	void readShortAddress();

	void readPanId();

	void writePanId(quint16 panId);

	void writeBaudrate(int baudrate);

	void readDeviceType();

	// According to documentation, this will revert the PAN ID to the default
	// value 0x199B.
	// DeviceType EndPoint is not supported.
	void writeDeviceType(ZigbeeDeviceType type);

	void restart();

	void testBaudrate();

signals:
	void baudrateSet();

	void baudrateTested(quint16 firmwareVersion);

	void deviceTypeRetrieved(ZigbeeDeviceType deviceType);

	void deviceTypeSet();

	void panIdRead(quint16 panId);

	void panIdWritten(quint16 panId);

	void shortAddressRead(quint16 address);

	void errorReceived(Drf1600Error errorType);

	void error(QSerialPort::SerialPortError serialPortError);

private slots:
	void onTimeout();

	void onReadyRead();

private:
	enum Command {
		GetDeviceType,
		Idle,
		ReadPanId,
		ReadShortAddress,
		Restart,
		SetBaudRate,
		SetDeviceType,
		TestBaudrate,
		WritePanId,
	};

	void send(Command command, int bytesExpected, const quint8 *buffer,
			  int length);

	static bool checkReply(const QByteArray &data, const quint8 *buffer, int length);

	void processPacket(const QByteArray &dataRead);

	void resetStateEngine();

	Command mCurrentCommand;
	int mBytesExpected;
	QSerialPort *mSerialPort;
	QTimer *mTimer;
};

#endif // DRF1600_H
