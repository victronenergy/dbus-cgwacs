#ifndef AC_SENSOR_SETTINGS_H
#define AC_SENSOR_SETTINGS_H

#include <QMetaType>
#include <QObject>
#include "defines.h"

Q_DECLARE_METATYPE(Phase)
Q_DECLARE_METATYPE(Position)
Q_DECLARE_METATYPE(AccountingMode)

class AcSensorSettings : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool supportMultiphase READ supportMultiphase)
	Q_PROPERTY(QString serial READ serial)
	Q_PROPERTY(QString customName READ customName WRITE setCustomName NOTIFY customNameChanged)
	Q_PROPERTY(QString productName READ productName NOTIFY productNameChanged)
	Q_PROPERTY(QString serviceType READ serviceType WRITE setServiceType)
	Q_PROPERTY(QString classAndVrmInstance READ classAndVrmInstance WRITE setClassAndVrmInstance NOTIFY classAndVrmInstanceChanged)
	Q_PROPERTY(QString l2ClassAndVrmInstance READ l2ClassAndVrmInstance WRITE setL2ClassAndVrmInstance NOTIFY l2ClassAndVrmInstanceChanged)
	Q_PROPERTY(bool isMultiPhase READ isMultiPhase WRITE setIsMultiPhase NOTIFY isMultiPhaseChanged)
	Q_PROPERTY(bool piggyEnabled READ piggyEnabled WRITE setPiggyEnabled NOTIFY piggyEnabledChanged)
	Q_PROPERTY(Position position READ position WRITE setPosition NOTIFY positionChanged)
	Q_PROPERTY(AccountingMode accountingMode READ accountingMode WRITE setaccountingMode NOTIFY accountingModeChanged)
	Q_PROPERTY(int deviceInstance READ deviceInstance)
	Q_PROPERTY(double l1ReverseEnergy READ l1ReverseEnergy WRITE setL1ReverseEnergy NOTIFY l1ReverseEnergyChanged)
	Q_PROPERTY(double l2ReverseEnergy READ l2ReverseEnergy WRITE setL2ReverseEnergy NOTIFY l2ReverseEnergyChanged)
	Q_PROPERTY(double l3ReverseEnergy READ l3ReverseEnergy WRITE setL3ReverseEnergy NOTIFY l3ReverseEnergyChanged)

	Q_PROPERTY(QString l2CustomName READ l2CustomName WRITE setL2CustomName NOTIFY l2CustomNameChanged)
	Q_PROPERTY(QString l2ProductName READ l2ProductName NOTIFY l2ProductNameChanged)
	Q_PROPERTY(QString l2ServiceType READ l2ServiceType WRITE setL2ServiceType)
	Q_PROPERTY(Position l2Position READ l2Position WRITE setL2Position NOTIFY l2PositionChanged)
	Q_PROPERTY(int l2DeviceInstance READ l2DeviceInstance NOTIFY l2DeviceInstanceChanged)
public:
	AcSensorSettings(bool supportMultiphase, const QString &serial, QObject *parent = 0);

	bool supportMultiphase() const;

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
	 * This string is used to create the D-Bus service.
	 * D-Bus service name will be 'com.victronenergy.[serviceName]'. So
	 * `serviceType` tells us where the device is installed (ie. which power
	 * stream is measured).
	 * Typical values are 'grid', 'genset' (for generator), and 'pvinverter'.
	 */
	QString serviceType() const;

	void setServiceType(const QString &t);

	QString classAndVrmInstance() const
	{
		return mClassAndVrmInstance;
	}

	void setClassAndVrmInstance(const QString &s);

	QString l2ClassAndVrmInstance() const
	{
		return mL2ClassAndVrmInstance;
	}

	void setL2ClassAndVrmInstance(const QString &s);

	bool isMultiPhase() const
	{
		return mIsMultiPhase;
	}

	bool piggyEnabled() const
	{
		return mPiggyEnabled;
	}

	void setPiggyEnabled(bool b);

	void setIsMultiPhase(bool b);

	const QString l2CustomName() const;

	void setL2CustomName(const QString &v);

	const QString l2ProductName() const;

	const QString l2ServiceType() const;

	void setL2ServiceType(const QString &t);

	Position l2Position() const;

	void setL2Position(Position p);

	Position position();

	void setPosition(Position p);

	AccountingMode accountingMode();

	void setAccountingMode(AccountingMode a);

	int deviceInstance() const;

	int l2DeviceInstance() const;

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

	void classAndVrmInstanceChanged();

	void l2ClassAndVrmInstanceChanged();

	void isMultiPhaseChanged();

	void piggyEnabledChanged();

	void hub4ModeChanged();

	void positionChanged();

	void accountingModeChanged();

	void l1ReverseEnergyChanged();

	void l2ReverseEnergyChanged();

	void l3ReverseEnergyChanged();

	void l2CustomNameChanged();

	void l2ProductNameChanged();

	void l2PositionChanged();

	void l2DeviceInstanceChanged();

private:
	static QString getProductName(const QString &serviceType, Position position);

	bool mSupportMultiphase;
	QString mSerial;
	QString mCustomName;
	QString mClassAndVrmInstance;
	QString mL2ClassAndVrmInstance;
	bool mIsMultiPhase;
	bool mPiggyEnabled;
	Position mPosition;
	AccountingMode mAccountingMode;
	double mL1Energy;
	double mL2Energy;
	double mL3Energy;

	QString mL2CustomName;
	Position mL2Position;
};

#endif // AC_SENSOR_SETTINGS_H
