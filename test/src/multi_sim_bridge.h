#ifndef MULTI_SIM_BRIDGE_H
#define MULTI_SIM_BRIDGE_H

#include "dbus_bridge.h"

class MultiSim;

class MultiSimBridge : public DBusBridge
{
	Q_OBJECT
public:
	explicit MultiSimBridge(MultiSim *multi, const QString &deviceName,
							QObject *parent = 0);
};

#endif // MULTI_SIM_BRIDGE_H
