#include <QDBusConnectionInterface>
#include <QDBusConnection>
#include <velib/qt/v_busitems.h>
#include "dbus_service_monitor.h"

DbusServiceMonitor::DbusServiceMonitor(QObject *parent):
	QObject(parent)
{
}

void DbusServiceMonitor::start()
{
	QDBusConnectionInterface *ci = VBusItems::getConnection().interface();
	connect(ci, SIGNAL(serviceOwnerChanged(QString, QString, QString)),
			this, SLOT(onServiceOwnerChanged(QString, QString, QString)));
	QDBusReply<QStringList> reply = ci->registeredServiceNames();
	foreach (QString s, reply.value())
		processNewService(s);
}

void DbusServiceMonitor::onServiceOwnerChanged(const QString &name,
											   const QString &oldOwner,
											   const QString &newOwner)
{
	if (oldOwner == newOwner)
		return;
	if (!oldOwner.isEmpty())
		processOldService(name);
	if (!newOwner.isEmpty())
		processNewService(name);
}

bool DbusServiceMonitor::isVictronService(const QString &name)
{
	return name.startsWith("com.victronenergy.");
}

void DbusServiceMonitor::processNewService(const QString &name)
{
	if (isVictronService(name))
		emit serviceAdded(name);
}

void DbusServiceMonitor::processOldService(const QString &name)
{
	if (isVictronService(name))
		emit serviceRemoved(name);
}
