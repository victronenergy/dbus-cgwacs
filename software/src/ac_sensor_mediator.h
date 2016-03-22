#ifndef ACSENSORMEDIATOR_H
#define ACSENSORMEDIATOR_H

#include <QObject>
#include "defines.h"

class AcSensor;
class AcSensorSettings;
class ModbusRtu;
class Settings;

class AcSensorMediator : public QObject
{
	Q_OBJECT
public:
	explicit AcSensorMediator(const QString &portName, bool isZigbee, Settings *settings,
							  QObject *parent = 0);

	AcSensor *gridMeter() const
	{
		return mGridMeter;
	}

	Hub4Mode hub4Mode() const
	{
		return mHub4Mode;
	}

	bool isMultiPhase() const
	{
		return mIsMultiPhase;
	}

signals:
	void gridMeterChanged();

	void serialEvent(const char *);

	void connectionLost();

	void hub4ModeChanged();

	void isMultiPhaseChanged();

private slots:
	void onDeviceFound();

	void onDeviceSettingsInitialized();

	void onDeviceInitialized();

	void onConnectionLost();

	void onConnectionStateChanged();

	void onServiceTypeChanged();

	void onHub4ModeChanged();

	void onIsMultiPhaseChanged();

private:
	void updateGridMeter();

	void publishSensor(AcSensor *acSensor, AcSensor *pvSensor, AcSensorSettings *acSensorSettings);

	QList<AcSensor *> mAcSensors;
	ModbusRtu *mModbus;
	AcSensor *mGridMeter;
	Settings *mSettings;
	Hub4Mode mHub4Mode;
	bool mIsMultiPhase;
};

#endif // ACSENSORMEDIATOR_H
