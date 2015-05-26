#include <QDBusConnectionInterface>
#include <QDBusConnection>
#include <velib/qt/v_busitems.h>
#include "dbus_service_monitor.h"

DbusServiceMonitor::DbusServiceMonitor(const QString &prefix, QObject *parent):
	QObject(parent),
	mPrefix(prefix + '.')
{
	QDBusConnectionInterface *ci = VBusItems::getConnection().interface();
	connect(ci, SIGNAL(serviceOwnerChanged(QString, QString, QString)),
			this, SLOT(onServiceOwnerChanged(QString, QString, QString)));
	QDBusReply<QStringList> reply = ci->registeredServiceNames();
	foreach (QString s, reply.value())
		processNewService(s);
}

QList<QString> DbusServiceMonitor::services() const
{
	return mServices;
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

void DbusServiceMonitor::processNewService(const QString &name)
{
	if (name.startsWith(mPrefix) && !mServices.contains(name)) {
		mServices.append(name);
		emit servicesChanged();
	}
}

void DbusServiceMonitor::processOldService(const QString &name)
{
	if (mServices.removeOne(name))
		emit servicesChanged();
}
