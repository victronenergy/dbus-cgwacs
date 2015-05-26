#ifndef VARIABLES_H
#define VARIABLES_H

#include <stdint.h>
#include <QList>

struct Variable {
	int address;
	int size;
	double factor;
	const char *name;

	double getValue(const QList<uint16_t> &registers, int offset) const;
};

class Variables
{
public:
	static const Variable *find(const char *name);

	static const Variable *get(int index);

	static double getValue(const QList<uint16_t> &registers, int offset,
						   const char *name);
};

#endif // VARIABLES_H
