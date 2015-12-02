#ifndef MULTI_BRIDGE_H
#define MULTI_BRIDGE_H

#include "dbus_bridge.h"

class Multi;
class MultiPhaseData;

/*!
 * Retrieves Multi data from the D-Bus (com.victronenergy.vebus.*) and stores
 * it in a `Multi` object. Changes in the AcPowerSetpoint of the `Multi` object
 * will be sent back to the D-Bus.
 */
class MultiBridge : public DBusBridge
{
	Q_OBJECT
public:
	MultiBridge(Multi *multi, const QString &service, QObject *parent = 0);

protected:
	virtual bool toDBus(const QString &path, QVariant &v);

	virtual bool fromDBus(const QString &path, QVariant &v);

private:
	void consumePhase(const QString &service,
					  const QString &phase, MultiPhaseData *phaseData);

	Multi *mMulti;
};

#endif // MULTI_BRIDGE_H
