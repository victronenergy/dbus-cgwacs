#include <velib/qt/ve_qitem.hpp>
#include "dbus_service_monitor.h"

DbusServiceMonitor::DbusServiceMonitor(QObject *parent):
	QObject(parent)
{
}

void DbusServiceMonitor::start()
{
	VeQItem *root = VeQItems::getRoot()->itemGetOrCreate("sub");
	connect(root, SIGNAL(childAdded(VeQItem *)), this, SLOT(onChildAdded(VeQItem *)));
	connect(root, SIGNAL(childRemoved(VeQItem *)), this, SLOT(onChildRemoved(VeQItem *)));
	for (int i=0;; ++i) {
		VeQItem *child = root->itemChild(i);
		if (child == 0)
			break;
		onChildAdded(child);
	}
}

void DbusServiceMonitor::onChildAdded(VeQItem *child)
{
	// Note that the service root item will start with state Idle, which will change to Offline
	// if the service disappears.
	QString name = child->id();
	connect(child, SIGNAL(stateChanged(VeQItem *, State)),
			this, SLOT(onChildStateChanged(VeQItem *)));
	onChildStateChanged(child);
}

void DbusServiceMonitor::onChildRemoved(VeQItem *child)
{
	QString name = child->id();
	if (mServices.contains(child)) {
		mServices.removeOne(child);
		emit serviceRemoved(child);
	}
}

void DbusServiceMonitor::onChildStateChanged(VeQItem *child)
{
	VeQItem::State state = child->getState();
	bool connected = state != VeQItem::Offline;
	if (connected && !mServices.contains(child)) {
		mServices.append(child);
		emit serviceAdded(child);
	} else if (!connected && mServices.contains(child)) {
		mServices.removeOne(child);
		emit serviceRemoved(child);
	}
}
