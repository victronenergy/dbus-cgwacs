#include <QDBusVariant>
#include <QDBusMessage>
#include <QsLog.h>
#include <QTimer>
#include <veutil/qt/ve_qitem.hpp>
#include <veutil/qt/ve_qitems_dbus.hpp>
#include "dbus_bridge.h"

DBusBridge::DBusBridge(const QString &serviceName, bool isProducer, QObject *parent):
	QObject(parent),
	mUpdateTimer(0),
	mIsProducer(isProducer),
	mIsInitialized(false)
{
	mServiceRoot = VeQItems::getRoot()->itemGetOrCreate(serviceName);
}

DBusBridge::DBusBridge(VeQItem *serviceRoot, bool isProducer, QObject *parent):
	QObject(parent),
	mServiceRoot(serviceRoot),
	mUpdateTimer(0),
	mIsProducer(isProducer)
{
}

DBusBridge::~DBusBridge()
{
	if (mServiceRoot == 0 || !mIsProducer)
		return;
	mServiceRoot->produceValue(QVariant(), VeQItem::Offline);
	QLOG_INFO() << "Unregistering service" << mServiceRoot->id();
}

void DBusBridge::setUpdateInterval(int interval)
{
	if (interval <= 0) {
		if (mUpdateTimer != 0) {
			delete mUpdateTimer;
			mUpdateTimer = 0;
		}
		return;
	}
	if (mUpdateTimer == 0) {
		mUpdateTimer = new QTimer(this);
		connect(mUpdateTimer, SIGNAL(timeout()), this, SLOT(onUpdateTimer()));
	}
	mUpdateTimer->setInterval(interval);
	mUpdateTimer->start();
}

void DBusBridge::produce(QObject *src, const char *property, const QString &path,
						 const QString &unit, int precision, bool alwaysNotify,
						 dbus_transform_t _fromDBus, dbus_transform_t _toDBus)
{
	Q_ASSERT(mIsProducer);
	VeQItem *vbi = mServiceRoot->itemGetOrCreate(path);
	BusItemBridge &b = connectItem(vbi, src, property, path, unit, precision, true, alwaysNotify, _fromDBus, _toDBus);
	publishValue(b);
}

void DBusBridge::produce(const QString &path, const QVariant &value,
						 const QString &unit, int precision,
						 dbus_transform_t _fromDBus, dbus_transform_t _toDBus)
{
	Q_ASSERT(mIsProducer);
	VeQItem *vbi = mServiceRoot->itemGetOrCreate(path);
	BusItemBridge &b = connectItem(vbi, 0, 0, path, unit, precision, true, false, _fromDBus, _toDBus);
	publishValue(b, value);
}

void DBusBridge::consume(QObject *src, const char *property, const QString &path,
						 dbus_transform_t _fromDBus, dbus_transform_t _toDBus)
{
	Q_ASSERT(!mIsProducer);
	VeQItem *vbi = mServiceRoot->itemGetOrCreate(path);
	BusItemBridge &b = connectItem(vbi, src, property, path, QString(), 0, false, false, _fromDBus, _toDBus);
	connect(vbi, SIGNAL(valueChanged(QVariant)),
			this, SLOT(onVBusItemChanged()));
	QVariant v = vbi->getValue(); // force value retrieval
	if (v.isValid())
		setValue(b, v);
}

void DBusBridge::consume(QObject *src, const char *property, const QVariant &defaultValue,
						 const QString &path, bool silentSetting,
						 dbus_transform_t _fromDBus, dbus_transform_t _toDBus)
{
	addSetting(path, defaultValue, QVariant(0), QVariant(0), silentSetting);
	consume(src, property, path, _fromDBus, _toDBus);
}

void DBusBridge::consume(QObject *src, const char *property, double defaultValue,
						 double minValue, double maxValue, const QString &path, bool silentSetting,
						 dbus_transform_t _fromDBus, dbus_transform_t _toDBus)
{
	addSetting(path, QVariant(defaultValue), QVariant(minValue), QVariant(maxValue), silentSetting);
	consume(src, property, path, _fromDBus, _toDBus);
}

void DBusBridge::registerService()
{
	Q_ASSERT(mIsProducer);
	if (mServiceRoot->getState() == VeQItem::Requested)
		return;
	QLOG_INFO() << "Registering service" << mServiceRoot->id();
	mServiceRoot->produceValue(QVariant(), VeQItem::Synchronized);
}

int DBusBridge::updateValue(BridgeItem *item, QVariant &value)
{
	BusItemBridge *bridge = findBridge(item);
	if (bridge == 0)
		return -1;
	setValue(*bridge, value);
	return 0;
}

bool DBusBridge::alwaysNotify(BridgeItem *item)
{
	BusItemBridge *bridge = findBridge(item);
	return bridge != 0 && bridge->alwaysNotify;
}

bool DBusBridge::toDBus(const QString &, QVariant &)
{
	return true;
}

bool DBusBridge::fromDBus(const QString &, QVariant &)
{
	return true;
}

QString DBusBridge::toText(const QString &path, const QVariant &value, const QString &unit,
						   int precision)
{
	Q_UNUSED(path)
	QString text;
	if (precision >= 0 && value.typeId() == QVariant::Double) {
		text.setNum(value.toDouble(), 'f', precision);
	} else {
		text = value.toString();
	}
	if (!text.isEmpty())
		text += unit;
	return text;
}

bool DBusBridge::addSetting(const QString &path,
							const QVariant &defaultValue,
							const QVariant &minValue,
							const QVariant &maxValue,
							bool silent)
{
	/// This will call the AddSetting function on com.victronenergy.settings. It should not be done
	/// here, because this class is supposed to be independent from VeQItem type. But since it is
	/// not implemented as part of the VeQItem framework, so it is better to do it here, than to
	/// shift the burden to the users of this class.
	int pos = path.startsWith('/') ? 1 : 0;
	if (path.mid(pos, 8) != "Settings") {
		QLOG_ERROR() << "Settings path should start with Settings: " << path;
		return false;
	}
	int groupStart = path.indexOf('/', pos);
	if (groupStart == -1) {
		QLOG_ERROR() << "Settings path should contain group name: " << path;
		return false;
	}
	int nameStart = path.lastIndexOf('/');
	if (nameStart <= groupStart) {
		QLOG_ERROR() << "Settings path should contain name: " << path;
		return false;
	}
	QChar type;
	switch (defaultValue.typeId()) {
	case QVariant::Int:
		type = 'i';
		break;
	case QVariant::Double:
		type = 'f';
		break;
	case QVariant::String:
		type = 's';
		break;
	default:
		return false;
	}
	VeQItemDbusProducer *p = qobject_cast<VeQItemDbusProducer *>(mServiceRoot->producer());
	if (p == 0) {
		QLOG_ERROR() << "No D-Bus producer found";
		return false;
	}
	QString group = path.mid(groupStart + 1, nameStart - groupStart - 1);
	QString name = path.mid(nameStart + 1);
	QDBusConnection &connection = p->dbusConnection();
	QDBusMessage m = QDBusMessage::createMethodCall(
						 "com.victronenergy.settings",
						 "/Settings",
						 "com.victronenergy.Settings",
						 silent ? "AddSilentSetting" : "AddSetting")
					 << group
					 << name
					 << QVariant::fromValue(QDBusVariant(defaultValue))
					 << QString(type)
					 << QVariant::fromValue(QDBusVariant(minValue))
					 << QVariant::fromValue(QDBusVariant(maxValue));
	QDBusMessage reply = connection.call(m);
	return reply.type() == QDBusMessage::ReplyMessage;
}

void DBusBridge::initDeviceInstance(const QString &uniqueId, const QString &deviceClass, int defaultValue)
{
	QString value = QString("%1:%2").arg(deviceClass).arg(defaultValue);
	QVariantMap inner;
	inner.insert("path",
		QVariant(QString("%1/ClassAndVrmInstance").arg(uniqueId)));
	inner.insert("default", QVariant(value));

	QDBusArgument argument;
	argument.beginArray(QVariant::Map);
	argument << inner;
	argument.endArray();

	QDBusConnection &connection = qobject_cast<VeQItemDbusProducer *>(mServiceRoot->producer())->dbusConnection();
	QDBusMessage m = QDBusMessage::createMethodCall(
				"com.victronenergy.settings", "/Settings/Devices",
				"com.victronenergy.Settings", "AddSettings")
		<< QVariant::fromValue(argument);
	connection.asyncCall(m);
}

void DBusBridge::onPropertyChanged()
{
	QObject *src = sender();
	int signalIndex = senderSignalIndex();
	for (QList<BusItemBridge>::iterator it = mBusItems.begin(); it != mBusItems.end(); ++it) {
		if (it->src == src && it->property.isValid() &&
				it->property.notifySignalIndex() == signalIndex) {
			if (mUpdateTimer == 0)
				publishValue(*it);
			else
				it->changed = true;
			break;
		}
	}
}

void DBusBridge::onVBusItemChanged()
{
	VeQItem *item = static_cast<VeQItem *>(sender());
	BusItemBridge *bridge = findBridge(item);
	if (bridge == 0)
		return;
	QVariant value = item->getValue();
	setValue(*bridge, value);
	updateIsInitialized();
}

void DBusBridge::onUpdateTimer()
{
	for (QList<BusItemBridge>::iterator it = mBusItems.begin(); it != mBusItems.end(); ++it) {
		if (it->changed) {
			publishValue(*it);
			it->changed = false;
		}
	}
}

DBusBridge::BusItemBridge & DBusBridge::connectItem(VeQItem *busItem, QObject *src,
													const char *property, const QString &path,
													const QString &unit, int precision,
													bool produce, bool alwaysNotify,
													dbus_transform_t _fromDBus, dbus_transform_t _toDBus)
{
	Q_ASSERT(produce || !alwaysNotify);
	BusItemBridge bib;
	bib.item = busItem;
	bib.src = src;
	bib.path = path;
	bib.changed = false;
	bib.unit = unit;
	bib.precision = precision;
	bib.busy = false;
	bib.alwaysNotify = alwaysNotify;
	bib.fromDBus = _fromDBus;
	bib.toDBus = _toDBus;
	if (produce) {
		BridgeItem *bi = qobject_cast<BridgeItem *>(busItem);
		if (bi != 0)
			bi->setBridge(this);
	}
	if (src == 0) {
		if (property != 0) {
			QLOG_ERROR() << "Property specified (" << property
						 << "), but source omitted in DBusBridge. Path was"
						 << path;
		}
	} else {
		if (property == 0) {
			QLOG_ERROR() << "Source specified, but property omitted in"
						 << "DBusBridge. Path was"
						 << path;
		} else {
			const QMetaObject *mo = src->metaObject();
			int i = mo->indexOfProperty(property);
			if (i == -1) {
				QLOG_ERROR() << "DBusBridge could not find property" << property
							 << "Path was" << path;
			} else {
				QMetaProperty mp = mo->property(i);
				if (mp.hasNotifySignal()) {
					QMetaMethod signal = mp.notifySignal();
					int index = metaObject()->indexOfSlot("onPropertyChanged()");
					QMetaMethod slot = metaObject()->method(index);
					connect(src, signal, this, slot);
				}
				bib.property = mp;
			}
		}
	}
	mBusItems.push_back(bib);
	return mBusItems.last();
}

void DBusBridge::publishValue(BusItemBridge &item)
{
	QVariant value = item.src->property(item.property.name());
	publishValue(item, value);
}

void DBusBridge::publishValue(DBusBridge::BusItemBridge &item, QVariant value)
{
	Q_ASSERT(!item.busy);
	if (item.busy)
		return;
	if (item.toDBus && !item.toDBus(this, value))
		return;
	if (!toDBus(item.path, value))
		return;
	item.busy = true;
	if (mIsProducer) {
		item.item->produceValue(value);
		QString text = toText(item.path, value, item.unit, item.precision);
		item.item->produceText(text);
	} else {
		item.item->setValue(value);
	}
	item.busy = false;
}

void DBusBridge::setValue(BusItemBridge &bridge, QVariant &value)
{
	Q_ASSERT(!bridge.busy);
	if (bridge.src == 0) {
		QLOG_WARN() << "Value changed on D-Bus could not be stored in QT-property";
	} else if (bridge.property.isValid()) {
		if (bridge.fromDBus && bridge.fromDBus(this, value))
			bridge.src->setProperty(bridge.property.name(), value);
		else if (fromDBus(bridge.path, value))
			bridge.src->setProperty(bridge.property.name(), value);
	}
}

void DBusBridge::updateIsInitialized()
{
	if (mIsInitialized)
		return;
	foreach (const BusItemBridge &bib, mBusItems) {
		Q_ASSERT(bib.item->getState() != VeQItem::Idle);
		if (bib.item->getState() == VeQItem::Requested)
			return;
	}
	mIsInitialized = true;
	emit initialized();
}

DBusBridge::BusItemBridge *DBusBridge::findBridge(VeQItem *item)
{
	for (QList<BusItemBridge>::iterator it = mBusItems.begin(); it != mBusItems.end(); ++it) {
		if (it->item == item)
			return &*it;
	}
	return 0;
}

void BridgeItem::produceValue(QVariant value, VeQItem::State state)
{
	bool stateIsChanged = mState != state;
	bool valueIsChanged = mValue != value || (mBridge != 0 && mBridge->alwaysNotify(this));

	mState = state;
	mValue = value;

	if (stateIsChanged)
		emit stateChanged(state);
	if (valueIsChanged)
		emit valueChanged(value);
}

void DBusBridge::publishPendingChanges()
{
	if (mUpdateTimer != 0) {
		onUpdateTimer();
		mUpdateTimer->start(); // Restart the timer
	}
}
