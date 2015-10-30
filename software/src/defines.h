#ifndef DEFINES_H
#define DEFINES_H

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
	return (msb << 8) | lsb;
}

const double NaN = qQNaN();

/// This value is used to indicate that the correct device instance has not
/// been set yet.
const int InvalidDeviceInstance = -1;
const int MinDeviceInstance = 32;
const int MaxDeviceInstance = 39;
const int GridDeviceInstance = 30;
const int GensetDeviceInstance = 31;

#endif // DEFINES_H
