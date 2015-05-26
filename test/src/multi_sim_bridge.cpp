#include <QDBusConnection>
#include <QsLog.h>
#include <velib/qt/v_busitems.h>
#include "multi_sim_bridge.h"
#include "multi_sim.h"

MultiSimBridge::MultiSimBridge(MultiSim *multi,
							   const QString &deviceName,
							   QObject *parent) :
	DBusBridge("com.victronenergy.vebus." + deviceName, parent)
{
	produce(multi, "powerOut", "/Ac/Out/L1/P");
	produce(multi, "powerOut", "/Ac/Out/L2/P");
	produce(multi, "powerOut", "/Ac/Out/L3/P");
	produce(multi, "powerOutSetPoint", "/Hub4/AcPowerSetpoint");
	registerService();
}
