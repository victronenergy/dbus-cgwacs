#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QStringList>
#include "defines.h"

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

	void registerDevice(const QString &serial);

signals:
	void deviceIdsChanged();

	void acPowerSetPointChanged();

	void maxDischargePercentageChanged();

	void maxChargePercentageChanged();

private:
	QStringList mDeviceIds;
	double mAcPowerSetPoint;
	double mMaxChargePercentage;
	double mMaxDischargePercentage;
};

#endif // SETTINGS_H
