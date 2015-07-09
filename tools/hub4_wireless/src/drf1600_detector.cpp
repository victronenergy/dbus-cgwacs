#include <QsLog.h>
#include <QTimer>
#include "drf1600.h"
#include "drf1600_detector.h"

// static const quint16 DefaultCoordinatorPanId = 0x5324; // 0x199B;
static const quint16 DefaultPanId = 0xFFFF;
static const int RetryCount = 5;

Drf1600Detector::Drf1600Detector(const QString &comPort,
								 QObject *parent):
	QObject(parent),
	mDrf1600(new Drf1600(comPort, this)),
	mTimer(new QTimer(this)),
	mRetryCount(0),
	mState(Idle),
	mDeviceType(ZigbeeRouter),
	mAdjust(false),
	mPanId(DefaultPanId)
{
	mTimer->setSingleShot(true);
	connect(mTimer, SIGNAL(timeout()), this, SLOT(onTimer()));
	connect(mDrf1600, SIGNAL(deviceTypeRetrieved(ZigbeeDeviceType)),
			this, SLOT(onTypeFound(ZigbeeDeviceType)));
	connect(mDrf1600, SIGNAL(deviceTypeSet()),
			this, SLOT(onTypeChanged()));
	connect(mDrf1600, SIGNAL(panIdRead(quint16)),
			this, SLOT(onPanIdFound(quint16)));
	connect(mDrf1600, SIGNAL(panIdWritten(quint16)),
			this, SLOT(onPanIdChanged()));
	connect(mDrf1600, SIGNAL(baudrateTested(quint16)),
			this, SLOT(onBaudrateOk(quint16)));
	connect(mDrf1600, SIGNAL(baudrateSet()),
			this, SLOT(onBaudrateSet()));
	connect(mDrf1600, SIGNAL(errorReceived(Drf1600Error)),
			this, SLOT(onError(Drf1600Error)));
}

ZigbeeDeviceType Drf1600Detector::deviceType() const
{
	return mDeviceType;
}

void Drf1600Detector::setDeviceType(ZigbeeDeviceType deviceType)
{
	Q_ASSERT(mState == Idle);
	// Never set the PanID of the coordinator as the PanID of the router.
	// This will cause undefined behavior: coordinator will automatically
	// try to change its PanID because it thinks another coordinator with
	// the same PanID is present. Set 0xFFFF instead which will force the
	// device to look for any coordinator after restart.
	if (deviceType == ZigbeeRouter)
		mPanId = DefaultPanId;
	mDeviceType = deviceType;
}

bool Drf1600Detector::adjustSettings() const
{
	return mAdjust;
}

void Drf1600Detector::setAdjustSettings(bool a)
{
	Q_ASSERT(mState == Idle);
	mAdjust = a;
}

quint16 Drf1600Detector::panId() const
{
	return mPanId;
}

void Drf1600Detector::setPanId(quint16 id)
{
	Q_ASSERT(mState == Idle);
	mPanId = id;
}

void Drf1600Detector::start()
{
	if (mDrf1600 == 0)
		return;
	mRetryCount = RetryCount;
	mDrf1600->setBaudRate(9600);
	mState = TestBaudRate9600;
	mDrf1600->testBaudrate();
}

QString Drf1600Detector::portName() const
{
	return mDrf1600->portName();
}

void Drf1600Detector::onPanIdFound(quint16 id)
{
	if (mDrf1600 == 0)
		return;
	QLOG_INFO() << "PanID:" << QString::number(id, 16);
	// PanID of 0xFFFE: router has not found coordinator yet. Otherwise PanID
	// will be equal to PanID of coordinator.
	bool adjust = mAdjust;
	if (adjust) {
		switch (mDeviceType) {
		case ZigbeeCoordinator:
			adjust = mPanId != DefaultPanId && id != 0x199B;
			break;
		case ZigbeeRouter:
			adjust = id != 0xFFFE;
			break;
		}
	}
	if (!adjust) {
		QLOG_INFO() << "Zigbee found and config checked";
		emit finished(true);
	} else {
		QLOG_INFO() << "Trying to change Pan ID to"
					<< QString::number(mPanId, 16);
		mState = SetPanId;
		mDrf1600->writePanId(mPanId);
	}
}

void Drf1600Detector::onPanIdChanged()
{
	restart();
}

void Drf1600Detector::onTypeFound(ZigbeeDeviceType type)
{
	if (mDrf1600 == 0)
		return;
	QLOG_INFO() <<  "Zigbee device type:" << type;
	if (!mAdjust || type == mDeviceType) {
		mState = GetPanId;
		mDrf1600->readPanId();
	} else {
		QLOG_INFO() << "Trying to change device type";
		mState = SetDeviceType;
		mDrf1600->writeDeviceType(mDeviceType);
	}
}

void Drf1600Detector::onTypeChanged()
{
	restart();
}

void Drf1600Detector::onBaudrateSet()
{
	restart();
}

void Drf1600Detector::onBaudrateOk(quint16 version)
{
	QLOG_INFO() << "Drf1600 Found, version:" << QString::number(version, 16)
				<< "baudrate:" << mDrf1600->baudRate();
	if (!mAdjust || mState == TestBaudRate9600) {
		mState = GetDeviceType;
		mDrf1600->readDeviceType();
	} else if (mState == TestBaudRate38400) {
		mState = SetBaudrate9600;
		mDrf1600->writeBaudrate(9600);
	}
}

void Drf1600Detector::onError(Drf1600Error error)
{
	QLOG_DEBUG() << "Drf1600 error" << error << mState << mRetryCount;
	--mRetryCount;
	if (mRetryCount <= 0) {
		if (mState == TestBaudRate9600) {
			QLOG_INFO() << "Trying 38400 baud";
			mDrf1600->setBaudRate(38400);
			mState = TestBaudRate38400;
			mRetryCount = RetryCount;
			mDrf1600->testBaudrate();
			return;
		}
		QLOG_INFO() << "Drf1600 not found";
		emit finished(false);
		return;
	}
	mTimer->setInterval(250);
	mTimer->start();
}

void Drf1600Detector::onTimer()
{
	if (mDrf1600 == 0)
		return;
	if (mState != TestBaudRate38400) {
		mState = TestBaudRate9600;
		mDrf1600->setBaudRate(9600);
	}
	mDrf1600->testBaudrate();
}

void Drf1600Detector::restart()
{
	if (mDrf1600 == 0)
		return;
	mDrf1600->restart();
	mTimer->setInterval(5000);
	mTimer->start();
}
