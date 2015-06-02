#ifndef DBUS_SERVICE_MONITOR_H
#define DBUS_SERVICE_MONITOR_H

#include <QObject>

/*!
 * Monitors the presence of services whose name start with a known prefix (eg.
 * com.victronenergy.vebus).
 * The list of services is stored in the `services` property. The
 * `servicesChanged` signal will be raised whenever the service list changed.
 */
class DbusServiceMonitor : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QList<QString> services READ services NOTIFY servicesChanged)
public:
	explicit DbusServiceMonitor(const QString &prefix, QObject *parent = 0);

	QList<QString> services() const;

signals:
	void servicesChanged();

private slots:
	void onServiceOwnerChanged(const QString &name, const QString &oldOwner,
							   const QString &newOwner);

private:
	void processNewService(const QString &name);

	void processOldService(const QString &name);

	QString mPrefix;
	QList<QString> mServices;
};

#endif // DBUS_SERVICE_MONITOR_H
