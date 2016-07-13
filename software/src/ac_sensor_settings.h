#ifndef AC_SENSOR_SETTINGS_H
#define AC_SENSOR_SETTINGS_H

#include <QMetaType>
#include <QObject>
#include "defines.h"

Q_DECLARE_METATYPE(Phase)
Q_DECLARE_METATYPE(Position)

class AcSensorSettings : public QObject
{
	Q_OBJECT
	Q_PROPERTY(int deviceType READ deviceType)
	Q_PROPERTY(QString serial READ serial)
	Q_PROPERTY(QString customName READ customName WRITE setCustomName NOTIFY customNameChanged)
	Q_PROPERTY(QString effectiveCustomName READ effectiveCustomName WRITE setEffectiveCustomName NOTIFY effectiveCustomNameChanged)
	Q_PROPERTY(QString productName READ productName NOTIFY productNameChanged)
	Q_PROPERTY(QString serviceType READ serviceType WRITE setServiceType NOTIFY serviceTypeChanged)
	Q_PROPERTY(bool isMultiPhase READ isMultiPhase WRITE setIsMultiPhase NOTIFY isMultiPhaseChanged)
	Q_PROPERTY(Position position READ position WRITE setPosition NOTIFY positionChanged)
	Q_PROPERTY(int deviceInstance READ deviceInstance WRITE setDeviceInstance NOTIFY deviceInstanceChanged)
	Q_PROPERTY(double l1ReverseEnergy READ l1ReverseEnergy WRITE setL1ReverseEnergy NOTIFY l1ReverseEnergyChanged)
	Q_PROPERTY(double l2ReverseEnergy READ l2ReverseEnergy WRITE setL2ReverseEnergy NOTIFY l2ReverseEnergyChanged)
	Q_PROPERTY(double l3ReverseEnergy READ l3ReverseEnergy WRITE setL3ReverseEnergy NOTIFY l3ReverseEnergyChanged)

	Q_PROPERTY(QString l2CustomName READ l2CustomName WRITE setL2CustomName NOTIFY l2CustomNameChanged)
	Q_PROPERTY(QString l2EffectiveCustomName READ l2EffectiveCustomName WRITE setL2EffectiveCustomName NOTIFY l2EffectiveCustomNameChanged)
	Q_PROPERTY(QString l2ProductName READ l2ProductName NOTIFY l2ProductNameChanged)
	Q_PROPERTY(QString l2ServiceType READ l2ServiceType WRITE setL2ServiceType NOTIFY l2ServiceTypeChanged)
	Q_PROPERTY(Position l2Position READ l2Position WRITE setL2Position NOTIFY l2PositionChanged)
	Q_PROPERTY(int l2DeviceInstance READ l2DeviceInstance WRITE setL2DeviceInstance NOTIFY l2DeviceInstanceChanged)
public:
	AcSensorSettings(int deviceType, const QString &serial, QObject *parent = 0);

	int deviceType() const
	{
		return mDeviceType;
	}

	QString serial() const
	{
		return mSerial;
	}

	/*!
	 * The custom name as entered by the user. Empty when the user did not
	 * enter a custom name.
	 */
	QString customName() const
	{
		return mCustomName;
	}

	void setCustomName(const QString &n);

	QString productName() const;

	/*!
	 * Returns customName if not empty, else productName
	 */
	QString effectiveCustomName() const;

	void setEffectiveCustomName(const QString &n);

	/*!
	 * This string is used to create the D-Bus service.
	 * D-Bus service name will be 'com.victronenergy.[serviceName]'. So
	 * `serviceType` tells us where the device is installed (ie. which power
	 * stream is measured).
	 * Typical values are 'grid', 'genset' (for generator), and 'pvinverter'.
	 */
	QString serviceType() const
	{
		return mServiceType;
	}

	void setServiceType(const QString &t);

	bool isMultiPhase() const
	{
		return mIsMultiPhase;
	}

	void setIsMultiPhase(bool b);

	QString l2CustomName() const;

	void setL2CustomName(const QString &v);

	QString l2ProductName() const;

	QString l2EffectiveCustomName() const;

	void setL2EffectiveCustomName(const QString &n);

	QString l2ServiceType() const;

	void setL2ServiceType(const QString &v);

	Position l2Position() const;

	void setL2Position(Position p);

	Position position();

	void setPosition(Position p);

	int deviceInstance() const;

	void setDeviceInstance(int d);

	int l2DeviceInstance() const;

	void setL2DeviceInstance(int d);

	double l1ReverseEnergy() const;

	void setL1ReverseEnergy(double e);

	double l2ReverseEnergy() const;

	void setL2ReverseEnergy(double e);

	double l3ReverseEnergy() const;

	void setL3ReverseEnergy(double e);

	double getReverseEnergy(Phase phase) const;

	void setReverseEnergy(Phase phase, double value);

signals:
	void customNameChanged();

	void productNameChanged();

	void effectiveCustomNameChanged();

	void serviceTypeChanged();

	void isMultiPhaseChanged();

	void hub4ModeChanged();

	void positionChanged();

	void deviceInstanceChanged();

	void l1ReverseEnergyChanged();

	void l2ReverseEnergyChanged();

	void l3ReverseEnergyChanged();

	void l2CustomNameChanged();

	void l2EffectiveCustomNameChanged();

	void l2ProductNameChanged();

	void l2ServiceTypeChanged();

	void l2PositionChanged();

	void l2DeviceInstanceChanged();

private:
	static QString getProductName(const QString &serviceType, Position position);

	int mDeviceType;
	QString mSerial;
	QString mCustomName;
	QString mServiceType;
	bool mIsMultiPhase;
	Position mPosition;
	int mDeviceInstance;
	double mL1Energy;
	double mL2Energy;
	double mL3Energy;

	QString mL2CustomName;
	QString mL2ServiceType;
	Position mL2Position;
	int mL2DeviceInstance;
};

#endif // AC_SENSOR_SETTINGS_H
