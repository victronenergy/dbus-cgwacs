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

enum AccountingMode {
	Individual = 0,
	Sum = 1,
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

/// This value is used to indicate that the correct device instance has not been set yet.
const int InvalidDeviceInstance = -1;
const int MinDeviceInstance = 30;
const int MaxDeviceInstance = 39;

#endif // DEFINES_H
