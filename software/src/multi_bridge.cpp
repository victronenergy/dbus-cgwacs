#include <QsLog.h>
#include "multi.h"
#include "multi_bridge.h"
#include "multi_phase_data.h"

MultiBridge::MultiBridge(Multi *multi, const QString &service, QObject *parent) :
	DBusBridge(service, parent),
	mMulti(multi)
{
	Q_ASSERT(mMulti != 0);
	consumePhase(service, "L1", multi->l1Data());
	consumePhase(service, "L2", multi->l2Data());
	consumePhase(service, "L3", multi->l3Data());
	consume(service, multi->meanData(), "acPowerIn", "/Ac/ActiveIn/P");
	consume(service, multi, "dcVoltage", "/Dc/0/Voltage");
	consume(service, multi, "dcCurrent", "/Dc/0/Current");
	consume(service, multi, "maxChargeCurrent", "/Dc/0/MaxChargeCurrent");
	consume(service, multi, "isSustainActive", "/Hub4/Sustain");
	consume(service, multi, "mode", "/Mode");
	consume(service, multi, "state", "/State");
	consume(service, multi, "isChargeDisabled", "/Hub4/DisableCharge");
	consume(service, multi, "isFeedbackDisabled", "/Hub4/DisableFeedback");
	consume(service, multi, "firmwareVersion", "/FirmwareVersion");
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

void MultiBridge::consumePhase(const QString &service, const QString &phase,
							   MultiPhaseData *phaseData)
{
	consume(service, phaseData, "acPowerIn", "/Ac/ActiveIn/" + phase + "/P");
	consume(service, phaseData, "acPowerOut", "/Ac/Out/" + phase + "/P");
	consume(service, phaseData, "acPowerSetPoint", "/Hub4/" + phase + "/AcPowerSetpoint");
}
