#ifndef DEFINES_H
#define DEFINES_H

#include <QtGlobal>

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

#endif // DEFINES_H
