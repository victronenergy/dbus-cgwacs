#include <QsLog.h>
#include "multi.h"
#include "multi_bridge.h"
#include "multi_phase_data.h"

MultiBridge::MultiBridge(Multi *multi, VeQItem *service, QObject *parent) :
	DBusBridge(service, false, parent),
	mMulti(multi)
{
	Q_ASSERT(mMulti != 0);
	consumePhase("L1", multi->l1Data());
	consumePhase("L2", multi->l2Data());
	consumePhase("L3", multi->l3Data());
	consume(multi->meanData(), "acPowerIn", "/Ac/ActiveIn/P");
	consume(multi, "dcVoltage", "/Dc/0/Voltage");
	consume(multi, "dcCurrent", "/Dc/0/Current");
	consume(multi, "maxChargeCurrent", "/Dc/0/MaxChargeCurrent");
	consume(multi, "isSustainActive", "/Hub4/Sustain");
	consume(multi, "mode", "/Mode");
	consume(multi, "state", "/State");
	consume(multi, "isChargeDisabled", "/Hub4/DisableCharge");
	consume(multi, "isFeedbackDisabled", "/Hub4/DisableFeedback");
	consume(multi, "firmwareVersion", "/FirmwareVersion");
}

bool MultiBridge::toDBus(const QString &path, QVariant &v)
{
	if (path.endsWith("/AcPowerSetpoint")) {
		v = QVariant::fromValue(static_cast<short>(v.toDouble()));
		return true;
	} else if (path == "/Mode") {
		v = QVariant(static_cast<int>(v.value<MultiMode>()));
		return true;
	} else if (path == "/Hub4/DisableCharge" || path == "/Hub4/DisableFeedback") {
		v = v.toBool() ? 1 : 0;
		return true;
	}
	return false;
}

bool MultiBridge::fromDBus(const QString &path, QVariant &v)
{
	if (path.endsWith("/AcPowerSetpoint")) {
		v = static_cast<double>(v.toInt());
	} else if (path == "/Mode") {
		v = QVariant::fromValue(static_cast<MultiMode>(v.toInt()));
	} else if (path == "/State") {
		v = QVariant::fromValue(static_cast<MultiState>(v.toInt()));
	} else if (path.endsWith("/DisableCharge") || path.endsWith("/DisableFeedback")) {
		v = v.toInt() != 0;
	}
	return true;
}

void MultiBridge::consumePhase(const QString &phase, MultiPhaseData *phaseData)
{
	consume(phaseData, "acPowerIn", "/Ac/ActiveIn/" + phase + "/P");
	consume(phaseData, "acPowerOut", "/Ac/Out/" + phase + "/P");
	consume(phaseData, "acPowerSetPoint", "/Hub4/" + phase + "/AcPowerSetpoint");
}
