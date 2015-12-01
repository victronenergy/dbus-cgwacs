#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDateTime>
#include <QMetaType>
#include <QObject>
#include <QStringList>
#include "defines.h"

Q_DECLARE_METATYPE(BatteryLifeState)

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
	Q_PROPERTY(BatteryLifeState state READ state WRITE setState NOTIFY stateChanged)
	Q_PROPERTY(quint8 flags READ flags WRITE setFlags NOTIFY flagsChanged)
	Q_PROPERTY(double socLimit READ socLimit WRITE setSocLimit NOTIFY socLimitChanged)
	Q_PROPERTY(double minSocLimit READ minSocLimit WRITE setMinSocLimit NOTIFY minSocLimitChanged)
	Q_PROPERTY(QDateTime dischargedTime READ dischargedTime WRITE setDischargedTime NOTIFY dischargedTimeChanged)
public:
	explicit Settings(QObject *parent = 0);

	QStringList deviceIds() const;

	void setDeviceIds(const QStringList &deviceIds);

	double acPowerSetPoint() const;

	void setAcPowerSetPoint(double p);

	double maxChargePercentage() const;

	void setMaxChargePercentage(double p);

	double maxDischargePercentage() const;

	void setMaxDischargePercentage(double p);

	BatteryLifeState state() const;

	void setState(BatteryLifeState state);

	/// This value consists of MaintenanceFlags or'ed together.
	quint8 flags() const;

	void setFlags(quint8 flags);

	double socLimit() const;

	void setSocLimit(double v);

	double minSocLimit() const;

	void setMinSocLimit(double l);

	QDateTime dischargedTime() const;

	void setDischargedTime(const QDateTime &t);

	void registerDevice(const QString &serial);

	int getDeviceInstance(const QString &serial, bool isSecundary) const;

signals:
	void deviceIdsChanged();

	void acPowerSetPointChanged();

	void maxDischargePercentageChanged();

	void maxChargePercentageChanged();

	void stateChanged();

	void flagsChanged();

	void socLimitChanged();

	void minSocLimitChanged();

	void dischargedTimeChanged();

	void isCloudyDayChanged();

	void lastSocChanged();

	void cloudyDayCountChanged();

	void averageSocChanged();

	void averageSocCountChanged();

	void nextDayEventChanged();

private:
	QStringList mDeviceIds;
	double mAcPowerSetPoint;
	double mMaxChargePercentage;
	double mMaxDischargePercentage;
	BatteryLifeState mState;
	quint8 mFlags;
	double mSocLimit;
	double mMinSocLimit;
	QDateTime mDischargedTime;
};

#endif // SETTINGS_H
