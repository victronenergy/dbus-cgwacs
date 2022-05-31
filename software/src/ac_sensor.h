#ifndef AC_SENSOR_H
#define AC_SENSOR_H

#include <QMetaType>
#include <QObject>
#include "defines.h"

class AcSensorPhase;

enum ConnectionState {
	Disconnected,
	Searched,
	Detected,
	Connected
};

Q_DECLARE_METATYPE(ConnectionState)

/*!
 * This class contains properties and measurements retrieved from Carlo Gavazzi
 * energy meters.
 */
class AcSensor : public QObject
{
	Q_OBJECT
	Q_PROPERTY(ConnectionState connectionState READ connectionState WRITE setConnectionState NOTIFY connectionStateChanged)
	Q_PROPERTY(int deviceType READ deviceType WRITE setDeviceType NOTIFY deviceTypeChanged)
	Q_PROPERTY(int deviceSubType READ deviceSubType WRITE setDeviceSubType NOTIFY deviceSubTypeChanged)
	Q_PROPERTY(QString serial READ serial WRITE setSerial NOTIFY serialChanged)
	Q_PROPERTY(QString role READ role WRITE setRole NOTIFY roleChanged)
	Q_PROPERTY(int firmwareVersion READ firmwareVersion WRITE setFirmwareVersion NOTIFY firmwareVersionChanged)
	Q_PROPERTY(int errorCode READ errorCode WRITE setErrorCode NOTIFY errorCodeChanged)
	Q_PROPERTY(QString portName READ portName)
public:
	AcSensor(const QString &portName, int slaveAddress, QObject *parent = 0);

	ConnectionState connectionState() const
	{
		return mConnectionState;
	}

	void setConnectionState(ConnectionState state);

	/*!
	 * Returns the device type as reported by the energy meter.
	 */
	int deviceType() const
	{
		return mDeviceType;
	}

	void setDeviceType(int t);

	/*!
	 * Returns the device sub type as reported by the energy meter. This value
	 * is non-zero for EM24 energy meters only.
	 */
	int deviceSubType() const
	{
		return mDeviceSubType;
	}

	void setDeviceSubType(int t);

	enum ProtocolTypes {
		Em24Protocol,
		Et112Protocol,
		Em340Protocol,
		Em540Protocol,
		Em300S27Protocol,
		Unknown
	};

	/*!
	 * Returns which set of registers is used by the energy meter.
	 * The return value is based on the value of `deviceType`.
	 */
	ProtocolTypes protocolType() const;

	/*!
	 * Returns whether this meter supports multiphase operation.
	 */
	bool supportMultiphase() const;

	/*!
	 * Returned the serial number of the energy meter.
	 */
	QString serial() const
	{
		return mSerial;
	}

	void setSerial(const QString &s);

	QString role() const
	{
		return mRole;
	}

	void setRole(const QString &s);

	int phaseSequence() const
	{
		return mPhaseSequence;
	}

	void setPhaseSequence(int v);

	int firmwareVersion() const
	{
		return mFirmwareVersion;
	}

	void setFirmwareVersion(int v);

	/*!
	 * Returns the error code.
	 * Possible values:
	 * - 0: no error
	 * - 1: front selector is locked (EM24 only). When the selector is locked,
	 * in is not possible to change settings in the energy meter. Relevant
	 * settings are: power returned to grid should be negative (application H)
	 * and (optionnally) change between single phase and multi phase
	 * measurement.
	 */
	int errorCode() const
	{
		return mErrorCode;
	}

	void setErrorCode(int code);

	/*!
	 * Returns the logical name of the communication port. (eg. /dev/ttyUSB1).
	 */
	QString portName() const
	{
		return mPortName;
	}

	int slaveAddress() const
	{
		return mSlaveAddress;
	}

	AcSensorPhase *total()
	{
		return mTotal;
	}

	AcSensorPhase *l1()
	{
		return mL1;
	}

	AcSensorPhase *l2()
	{
		return mL2;
	}

	AcSensorPhase *l3()
	{
		return mL3;
	}

	AcSensorPhase *getPhase(Phase phase);

	/*!
	 * Reset all measured values to NaN
	 */
	void resetValues();

	/*
	 * Send values out on dbus
	 */
	void flushValues();

	/*
	 * How often the modbus registers are updated, a static value based on what
	 * we know about the meter.
	 */
	int refreshTime();

signals:
	void connectionStateChanged();

	void deviceTypeChanged();

	void deviceSubTypeChanged();

	void serialChanged();

	void roleChanged();

	void firmwareVersionChanged();

	void errorCodeChanged();

private:
	ConnectionState mConnectionState;
	int mDeviceType;
	int mDeviceSubType;
	int mErrorCode;
	int mFirmwareVersion;
	QString mPortName;
	int mSlaveAddress;
	QString mSerial;
	QString mRole;
	int mPhaseSequence;
	AcSensorPhase *mTotal;
	AcSensorPhase *mL1;
	AcSensorPhase *mL2;
	AcSensorPhase *mL3;
};

#endif // AC_SENSOR_H
