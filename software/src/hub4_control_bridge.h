#ifndef HUB4CONTROLBRIDGE_H
#define HUB4CONTROLBRIDGE_H

#include "dbus_bridge.h"

class Settings;

class Hub4ControlBridge : public DBusBridge
{
public:
	explicit Hub4ControlBridge(Settings *settings, QObject *parent = 0);

protected:
	virtual bool toDBus(const QString &path, QVariant &value);

	virtual bool fromDBus(const QString &path, QVariant &value);
};

#endif // HUB4CONTROLBRIDGE_H
