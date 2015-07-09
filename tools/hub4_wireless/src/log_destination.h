#ifndef LOGDESTINATION_H
#define LOGDESTINATION_H

#include <QPointer>
#include <QsLogDest.h>
#include "hub4_wireless.h"

class LogDestination : public QsLogging::Destination
{
public:
	LogDestination(Hub4Wireless *setup);

	virtual void write(const QString& message, QsLogging::Level level);

	// returns whether the destination was created correctly
	virtual bool isValid();

private:
	QPointer<Hub4Wireless> mSetup;
};

#endif // LOGDESTINATION_H
