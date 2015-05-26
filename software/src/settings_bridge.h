#ifndef SETTINGS_BRIDGE_H
#define SETTINGS_BRIDGE_H

#include <QMetaType>
#include "dbus_bridge.h"
#include "defines.h"

class Settings;

class SettingsBridge : public DBusBridge
{
public:
	SettingsBridge(Settings *settings, QObject *parent = 0);

protected:
	virtual bool toDBus(const QString &path, QVariant &value);

	virtual bool fromDBus(const QString &path, QVariant &value);
};

#endif // SETTINGS_BRIDGE_H
