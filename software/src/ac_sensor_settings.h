#ifndef ENERGY_METER_SETTINGS_H
#define ENERGY_METER_SETTINGS_H

#include <QMetaType>
#include <QObject>
#include "defines.h"

enum Hub4Mode {
	Hub4PhaseL1 = 0,
	Hub4PhaseCompensation = 1,
	Hub4PhaseSplit = 2
};

Q_DECLARE_METATYPE(Hub4Mode)
Q_DECLARE_METATYPE(Phase)
Q_DECLARE_METATYPE(Position)

class AcSensorSettings : public QObject
{
	Q_OBJECT
	Q_PROPERTY(int deviceType READ deviceType)
	Q_PROPERTY(QString serial READ serial)
	Q_PROPERTY(QString customName READ customName WRITE setCustomName NOTIFY customNameChanged)
	Q_PROPERTY(QString serviceType READ serviceType WRITE setServiceType NOTIFY serviceTypeChanged)
	Q_PROPERTY(bool isMultiPhase READ isMultiPhase WRITE setIsMultiPhase NOTIFY isMultiPhaseChanged)
	Q_PROPERTY(Hub4Mode hub4Mode READ hub4Mode WRITE setHub4Mode NOTIFY hub4ModeChanged)
	Q_PROPERTY(Position position READ position WRITE setPosition NOTIFY positionChanged)
	Q_PROPERTY(double l1ReverseEnergy READ l1ReverseEnergy WRITE setL1ReverseEnergy NOTIFY l1ReverseEnergyChanged)
	Q_PROPERTY(double l2ReverseEnergy READ l2ReverseEnergy WRITE setL2ReverseEnergy NOTIFY l2ReverseEnergyChanged)
	Q_PROPERTY(double l3ReverseEnergy READ l3ReverseEnergy WRITE setL3ReverseEnergy NOTIFY l3ReverseEnergyChanged)
public:
	AcSensorSettings(int deviceType, const QString &serial, QObject *parent = 0);

	int deviceType() const;

	QString serial() const;

	QString customName() const;

	void setCustomName(const QString &n);

	/*!
	 * This string is used to create the D-Bus service.
	 * D-Bus service name will be 'com.victronenergy.[serviceName]'. So
	 * `serviceType` tells us where the device is installed (ie. which power
	 * stream is measured).
	 * Typical values are 'grid', 'genset' (for generator), and 'pvinverter'.
	 */
	QString serviceType() const;

	void setServiceType(const QString &t);

	bool isMultiPhase() const;

	void setIsMultiPhase(bool b);

	Hub4Mode hub4Mode() const;

	void setHub4Mode(Hub4Mode m);

	Position position();

	void setPosition(Position p);

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

	void serviceTypeChanged();

	void isMultiPhaseChanged();

	void isControlLoopEnabledChanged();

	void hub4ModeChanged();

	void positionChanged();

	void l1ReverseEnergyChanged();

	void l2ReverseEnergyChanged();

	void l3ReverseEnergyChanged();

private:
	int mDeviceType;
	QString mSerial;
	QString mCustomName;
	QString mServiceType;
	bool mIsMultiPhase;
	Hub4Mode mHub4Mode;
	Position mPosition;
	double mL1Energy;
	double mL2Energy;
	double mL3Energy;
};

#endif // ENERGY_METER_SETTINGS_H
