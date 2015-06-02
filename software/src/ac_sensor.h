#ifndef AC_SENSOR_H
#define AC_SENSOR_H

#include <QObject>
#include "defines.h"

class PowerInfo;
struct FroniusDeviceInfo;

/*!
 * This class contains properties and measurements retrieved from Carlo Gavazzi
 * energy meters.
 */
class AcSensor : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool isConnected READ isConnected WRITE setIsConnected NOTIFY isConnectedChanged)
	Q_PROPERTY(int deviceType READ deviceType WRITE setDeviceType NOTIFY deviceTypeChanged)
	Q_PROPERTY(int deviceSubType READ deviceSubType WRITE setDeviceSubType NOTIFY deviceSubTypeChanged)
	Q_PROPERTY(QString productName READ productName)
	Q_PROPERTY(QString serial READ serial WRITE setSerial NOTIFY serialChanged)
	Q_PROPERTY(int firmwareVersion READ firmwareVersion WRITE setFirmwareVersion NOTIFY firmwareVersionChanged)
	Q_PROPERTY(int errorCode READ errorCode WRITE setErrorCode NOTIFY errorCodeChanged)
	Q_PROPERTY(QString portName READ portName)
public:
	AcSensor(const QString &portName, int slaveAddress, QObject *parent = 0);

	bool isConnected() const;

	void setIsConnected(bool v);

	/*!
	 * Returns the device type as reported by the energy meter.
	 */
	int deviceType() const;

	void setDeviceType(int t);

	/*!
	 * Returns the device sub type as reported by the energy meter. This value
	 * is non-zero for EM24 energy meters only.
	 */
	int deviceSubType() const;

	void setDeviceSubType(int t);

	enum ProtocolTypes {
		Em24Protocol,
		Et112Protocol,
		Em340Protocol
	};

	/*!
	 * Returns which set of registers is used by the energy meter.
	 * The return value is based on the value of `deviceType`.
	 */
	ProtocolTypes protocolType() const;

	/*!
	 * Returns the full official producet name of the energy meter.
	 * For example: EM24=DIN.AV9.3.X.IS.X.
	 * The return value is based on the value of `deviceType` and
	 * `subDeviceType`.
	 */
	QString productName() const;

	/*!
	 * Returned the serial number of the energy meter.
	 */
	QString serial() const;

	void setSerial(const QString &s);

	int firmwareVersion() const;

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
	int errorCode() const;

	void setErrorCode(int code);

	/*!
	 * Returns the logical name of the communication port. (eg. /dev/ttyUSB1).
	 */
	QString portName() const;

	int slaveAddress() const;

	PowerInfo *meanPowerInfo();

	PowerInfo *l1PowerInfo();

	PowerInfo *l2PowerInfo();

	PowerInfo *l3PowerInfo();

	PowerInfo *getPowerInfo(Phase phase);

	/*!
	 * Reset all measured values to NaN
	 */
	void resetValues();

signals:
	void isConnectedChanged();

	void deviceTypeChanged();

	void deviceSubTypeChanged();

	void serialChanged();

	void firmwareVersionChanged();

	void errorCodeChanged();

private:
	bool mIsConnected;
	int mDeviceType;
	int mDeviceSubType;
	int mErrorCode;
	int mFirmwareVersion;
	QString mPortName;
	int mSlaveAddress;
	QString mSerial;
	PowerInfo *mMeanPowerInfo;
	PowerInfo *mL1PowerInfo;
	PowerInfo *mL2PowerInfo;
	PowerInfo *mL3PowerInfo;
};

#endif // AC_SENSOR_H
