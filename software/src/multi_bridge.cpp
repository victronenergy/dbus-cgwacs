#include <QsLog.h>
#include "multi.h"
#include "multi_bridge.h"
#include "multi_phase_data.h"

MultiBridge::MultiBridge(Multi *multi, const QString &service,
						 QObject *parent) :
	DBusBridge(service, parent),
	mMulti(multi)
{
	Q_ASSERT(mMulti != 0);
	consumePhase(service, "/Ac/ActiveIn", multi->meanData());
	consumePhase(service, "/Ac/ActiveIn/L1", multi->l1Data());
	// /Ac/ActiveIn/L2 tree is invalid for single phase system, which means that
	// the initialized signal will never fire.
	consumePhase(service, "/Ac/ActiveIn/L2", multi->l2Data());
	consumePhase(service, "/Ac/ActiveIn/L3", multi->l3Data());
	// Order matters here. Retrieve acPowerSetPoint as last item, so we know all
	// items are synced once IsSetPointAvailable is set.
	consume(service, multi, "acPowerSetPoint", "/Hub4/AcPowerSetpoint");
}

bool MultiBridge::fromDBus(const QString &path, QVariant &v)
{
	if (path == "/Hub4/AcPowerSetpoint") {
		mMulti->setIsSetPointAvailable(v.isValid());
	}
	return true;
}

void MultiBridge::consumePhase(const QString &service, const QString &path,
							   MultiPhaseData *phaseData)
{
	consume(service, phaseData, "acPowerIn", path + "/P");
}
