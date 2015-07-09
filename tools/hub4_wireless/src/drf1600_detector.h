#ifndef DRF1600DETECTOR_H
#define DRF1600DETECTOR_H

#include <QObject>
#include "drf1600.h"

class QTimer;

class Drf1600Detector : public QObject
{
	Q_OBJECT
public:
	explicit Drf1600Detector(const QString &comPort, QObject *parent = 0);

	ZigbeeDeviceType deviceType() const;

	void setDeviceType(ZigbeeDeviceType deviceType);

	bool adjustSettings() const;

	void setAdjustSettings(bool a);

	quint16 panId() const;

	void setPanId(quint16 id);

	void start();

	QString portName() const;

signals:
	void finished(bool found);

	void error(QSerialPort::SerialPortError serialPortError);

private slots:
	void onPanIdFound(quint16 id);

	void onPanIdChanged();

	void onTypeFound(ZigbeeDeviceType type);

	void onTypeChanged();

	void onBaudrateSet();

	void onBaudrateOk(quint16 version);

	void onError(Drf1600Error error);

	void onTimer();

private:
	void restart();

	enum State {
		Idle,
		TestBaudRate9600,
		TestBaudRate38400,
		SetBaudrate9600,
		GetDeviceType,
		SetDeviceType,
		GetPanId,
		SetPanId
	};

	Drf1600 *mDrf1600;
	QTimer *mTimer;
	int mRetryCount;
	State mState;
	ZigbeeDeviceType mDeviceType;
	bool mAdjust;
	int mPanId;
};

#endif // DRF1600DETECTOR_H
