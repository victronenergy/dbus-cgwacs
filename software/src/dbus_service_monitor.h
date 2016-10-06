#ifndef DBUS_SERVICE_MONITOR_H
#define DBUS_SERVICE_MONITOR_H

#include <QObject>
#include <QList>

class VeQItem;

/// Emits signals when D-Bus services appear/disappears to/from the D-Bus
class DbusServiceMonitor : public QObject
{
	Q_OBJECT
public:
	explicit DbusServiceMonitor(QObject *parent = 0);

	void start();

signals:
	void serviceAdded(VeQItem *root);

	void serviceRemoved(VeQItem *root);

private slots:
	void onChildAdded(VeQItem *child);

	void onChildRemoved(VeQItem *child);

	void onChildStateChanged(VeQItem *child);

private:
	QList<VeQItem *> mServices;
};

#endif // DBUS_SERVICE_MONITOR_H
