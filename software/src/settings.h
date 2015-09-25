#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDateTime>
#include <QMetaType>
#include <QObject>
#include <QStringList>
#include "defines.h"

enum Hub4State {
	/// Will run power control algorithm and wait until the next maintenance
	/// date is reached.
	Hub4SelfConsumption = 0,
	/// Battery will be charged (from grid if no PV power available) until
	/// fully charged.
	Hub4ChargeFromGrid = 1,
	/// Battery is currently charged, but power control algorithm is running.
	/// We get here after leaving storage, after leaving charge from grid, or
	/// when battery gets fully charged while self consuming.
	Hub4Charged = 2,
	/// Battery put in storage mode by user.
	Hub4Storage = 3,
	/// Set if maintenance charge has been disabled. Always runs power control
	/// algorithm. This mode is intended for external control over Modbus TCP.
	/// In this case maintenance charge could be implemented by setting the
	/// /Hub4/AcPowerSetpoint.
	Hub4External = 4
};

Q_DECLARE_METATYPE(Hub4State)

/*!
 * Contains the global (sensor independent) settings.
 */
class Settings : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QStringList deviceIds READ deviceIds WRITE setDeviceIds NOTIFY deviceIdsChanged)
	Q_PROPERTY(double acPowerSetPoint READ acPowerSetPoint WRITE setAcPowerSetPoint NOTIFY acPowerSetPointChanged)
	Q_PROPERTY(double maxChargePercentage READ maxChargePercentage WRITE setMaxChargePercentage NOTIFY maxChargePercentageChanged)
	Q_PROPERTY(double maxDischargePercentage READ maxDischargePercentage WRITE setMaxDischargePercentage NOTIFY maxDischargePercentageChanged)
	/// @todo EV Type of state property should be Hub4State, but if we use that
	/// changed from the D-Bus are not propagated somehow.
	Q_PROPERTY(int state READ state WRITE setState NOTIFY stateChanged)
	Q_PROPERTY(int maintenanceInterval READ maintenanceInterval WRITE setMaintenanceInterval NOTIFY maintenanceIntervalChanged)
	/// Date when last maintenance cycle was completed. Next maintance charge
	/// will start at date maintenanceDate + maintenanceInterval
	Q_PROPERTY(QDateTime maintenanceDate READ maintenanceDate WRITE setMaintenanceDate NOTIFY maintenanceDateChanged)
public:
	explicit Settings(QObject *parent = 0);

	const QStringList &deviceIds() const;

	void setDeviceIds(const QStringList &deviceIds);

	double acPowerSetPoint() const;

	void setAcPowerSetPoint(double p);

	double maxChargePercentage() const;

	void setMaxChargePercentage(double p);

	double maxDischargePercentage() const;

	void setMaxDischargePercentage(double p);

	int maintenanceInterval() const;

	int state() const;

	void setState(int state);

	void setMaintenanceInterval(int v);

	QDateTime maintenanceDate() const;

	void setMaintenanceDate(const QDateTime &v);

	void registerDevice(const QString &serial);

signals:
	void deviceIdsChanged();

	void acPowerSetPointChanged();

	void maxDischargePercentageChanged();

	void maxChargePercentageChanged();

	void stateChanged();

	void maintenanceIntervalChanged();

	void maintenanceDateChanged();

private:
	QStringList mDeviceIds;
	double mAcPowerSetPoint;
	double mMaxChargePercentage;
	double mMaxDischargePercentage;
	int mState;
	int mMaintenanceInterval;
	QDateTime mMaintenanceDate;
};

#endif // SETTINGS_H
