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
public:
	explicit DbusServiceMonitor(QObject *parent = 0);

	void start();

signals:
	void serviceAdded(QString service);

	void serviceRemoved(QString service);

private slots:
	void onServiceOwnerChanged(const QString &name, const QString &oldOwner,
							   const QString &newOwner);

private:
	void processNewService(const QString &name);

	void processOldService(const QString &name);

	static bool isVictronService(const QString &name);
};

#endif // DBUS_SERVICE_MONITOR_H
