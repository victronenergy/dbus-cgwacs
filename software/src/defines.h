#ifndef DEFINES_H
#define DEFINES_H

#include <QByteArray>
#include <QtGlobal>
#include <qnumeric.h>

enum Phase {
	MultiPhase = 0,
	PhaseL1 = 1,
	PhaseL2 = 2,
	PhaseL3 = 3
};

enum Position {
	Input1 = 0,
	Output = 1,
	Input2 = 2
};

enum Hub4Mode {
	/// Use L1 for control loop only. No longer used. AcSensorSettingsBridge will change this
	/// mode to Hub4PhaseCompensation
	Hub4PhaseL1 = 0,
	/// Apply grid AC setpoint on total power to grid. Individual phases are controlled so that
	/// power flow between Multis is minimized: all multi's in the system are either inverting or
	/// charging or inactive.
	Hub4PhaseCompensation = 1,
	/// Each phase is controlled so that each individual phase has the same power: 0 Watt by
	/// default, or the grid AC setpoint devided by the number of phases.
	Hub4PhaseSplit = 2,
	/// No control loop active. This should be set by developers who want to create their own
	/// controll loop and the Multi AC power setpoint, to prevent this service to change it as well.
	Hub4Disabled = 3,
	/// Old fashioned phase compensation: one phase (usually L1) will compensate so that total grid
	/// power equals the grid AC setpoint. There is no control loop on the other phases. This option
	/// is provided for backward compatibility.
	Hub4SinglePhaseCompensation = 4
};

inline quint8 msb(quint16 d)
{
	return d >> 8;
}

inline quint8 lsb(quint16 d)
{
	return d & 0xFF;
}

inline quint16 toUInt16(quint8 msb, quint8 lsb)
{
	return static_cast<quint16>((msb << 8) | lsb);
}

inline quint16 toUInt16(const QByteArray &a, int offset)
{
	return toUInt16(static_cast<quint8>(a[offset]), static_cast<quint8>(a[offset + 1]));
}

/// This value is used to indicate that the correct device instance has not
/// been set yet.
const int InvalidDeviceInstance = -1;
const int MinDeviceInstance = 30;
const int MaxDeviceInstance = 39;

const double MaxMultiPower = 32700;
const double MinMultiPower = -MaxMultiPower;

#endif // DEFINES_H
