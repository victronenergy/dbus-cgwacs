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
public:
	explicit Settings(QObject *parent = 0);

	const QStringList &deviceIds() const;

	void setDeviceIds(const QStringList &deviceIds);

	double acPowerSetPoint() const;

	void setAcPowerSetPoint(double p);

	void registerDevice(const QString &serial);

signals:
	void deviceIdsChanged();

	void acPowerSetPointChanged();

private:
	QStringList mDeviceIds;
	double mAcPowerSetPoint;
};

#endif // SETTINGS_H
