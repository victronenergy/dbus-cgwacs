#ifndef MULTI_BRIDGE_H
#define MULTI_BRIDGE_H

#include "dbus_bridge.h"

class Multi;
class MultiPhaseData;

class MultiBridge : public DBusBridge
{
	Q_OBJECT
public:
	/*!
	 * DBusMultiBridge
	 * @param deviceName The (last part of) the device name (eg. ttyO1)
	 * @param parent
	 */
	explicit MultiBridge(Multi *multi, const QString &service,
						 QObject *parent = 0);

protected:
	virtual bool fromDBus(const QString &path, QVariant &v);

private:
	void consumePhase(const QString &service,
					  const QString &path, MultiPhaseData *phaseData);

	Multi *mMulti;
};

#endif // MULTI_BRIDGE_H
