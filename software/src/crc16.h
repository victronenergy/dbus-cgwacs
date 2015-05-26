#ifndef CRC16_H
#define CRC16_H

#include <QByteArray>
#include <stdint.h>

class Crc16
{
public:
	Crc16();

	uint16_t getValue() const;

	void add(uint8_t byte);

	void add(const QByteArray &bytes);

	void reset();

	static uint16_t getValue(const QByteArray &bytes);

private:
	uint8_t mCrcLo;
	uint8_t mCrcHi;
};

#endif // CRC16_H
