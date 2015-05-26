#include <limits>
#include "variables.h"

static const double NaN = std::numeric_limits<double>::quiet_NaN();

static const Variable VariableList[] = {
	{ 0x00, 4, 0.1, "V L1-N" },
	{ 0x02, 4, 0.1, "V L2-N" },
	{ 0x04, 4, 0.1, "V L3-N" },
	{ 0x06, 4, 0.1, "V L1-L2" },
	{ 0x08, 4, 0.1, "V L2-L3" },
	{ 0x0A, 4, 0.1, "V L3-L1" },
	{ 0x0C, 4, 1e-3, "A L1" },
	{ 0x0E, 4, 1e-3, "A L2" },
	{ 0x10, 4, 1e-3, "A L3" },
	{ 0x12, 4, 0.1, "W L1" },
	{ 0x14, 4, 0.1, "W L2" },
	{ 0x16, 4, 0.1, "W L3" },
	{ 0x18, 4, 0.1, "VA L1" },
	{ 0x1A, 4, 0.1, "VA L2" },
	{ 0x1C, 4, 0.1, "VA L3" },
	{ 0x1E, 4, 0.1, "VAR L1" },
	{ 0x20, 4, 0.1, "VAR L2" },
	{ 0x22, 4, 0.1, "VAR L3" },
	{ 0x24, 4, 0.1, "V L-N sum" },
	{ 0x26, 4, 0.1, "V L-L sum" },
	{ 0x28, 4, 0.1, "W sum" },
	{ 0x2A, 4, 0.1, "VA sum" },
	{ 0x2C, 4, 0.1, "VAR sum" },
	{ 0x2E, 4, 0.1, "DMD W sum" },
	{ 0x30, 4, 0.1, "DMD VA sum" },
	{ 0x32, 2, 1e-3, "PF L1" },
	{ 0x33, 2, 1e-3, "PF L2" },
	{ 0x34, 2, 1e-3, "PF L3" },
	{ 0x35, 2, 1e-3, "PF sum" },
	{ 0x36, 2, 1, "Phase sequence" },
	{ 0x37, 2, 0.1, "Hz" },
	{ 0x38, 4, 0.1, "DMD W Sum max" },
	{ 0x3A, 4, 0.1, "DMD VA Sum max" },
	{ 0x3C, 4, 1e-3, "DMD A max" },
	{ 0x3E, 4, 0.1, "kWh (+) tot" },
	{ 0x40, 4, 0.1, "kvarh (+) tot" },
	{ 0x42, 4, 0.1, "KWh (+) par" },
	{ 0x44, 4, 0.1, "KWvarh (+) par" },
	{ 0x46, 4, 0.1, "kWh (+) L1" },
	{ 0x48, 4, 0.1, "kWh (+) L2" },
	{ 0x4A, 4, 0.1, "kWh (+) L3" },
	{ 0x4C, 4, 0.1, "kWh (+) T1" },
	{ 0x4E, 4, 0.1, "kWh (+) T2" },
	{ 0x50, 4, 0.1, "kWh (+) T3" },
	{ 0x52, 4, 0.1, "kWh (+) T4" },
	{ 0x54, 4, 0.1, "kvarh (+) T1" },
	{ 0x56, 4, 0.1, "kvarh (+) T2" },
	{ 0x58, 4, 0.1, "kvarh (+) T3" },
	{ 0x5A, 4, 0.1, "kvarh (+) T4" },
	{ 0x5C, 4, 0.1, "kWh (-) tot" },
	{ 0x5E, 4, 0.1, "kvarh (-) tot" },
	{ 0x60, 4, 0.01, "hour" },
	{ 0x62, 4, 1, "Counter 1" },
	{ 0x64, 4, 1, "Counter 2" },
	{ 0x66, 4, 1, "Counter 3" },
	{ 0x124, 4, 0.1, "W L1 inst" }, // ET 340 only (ET 112?)
	{ 0x1101, 2, 1, "Type of application" }, // EM24
	{ 0x1102, 2, 1, "Measuring system" }, // EM 24
	{ 0x1104, 2, 1, "Connection Check Enable" }, // ET 340 only (ET 112?)
	{ 0x1105, 2, 1, "Connection Check" }, // ET 340 only (ET 112?)
};

static const int VariableCount = sizeof(VariableList)/sizeof(VariableList[0]);

const Variable *Variables::find(const char *name)
{
	for (int i=0; i<VariableCount; ++i) {
		const Variable *v = VariableList + i;
		if (strcmp(name, v->name) == 0)
			return v;
	}
	return 0;
}

const Variable *Variables::get(int index)
{
	if (index < 0 || index >= VariableCount)
		return 0;
	return VariableList + index;
}

double Variables::getValue(const QList<uint16_t> &registers, int offset,
						   const char *name)
{
	const Variable *v = find(name);
	if (v == 0)
		return NaN;
	return v->getValue(registers, offset);
}

double Variable::getValue(const QList<uint16_t> &registers, int offset) const
{
	double value = 0;
	if (size == 4) {
		value = (int32_t)(registers[offset] | registers[offset + 1] << 16);
	} else {
		value = (int16_t)registers[offset];
	}
	return value * factor;
}
