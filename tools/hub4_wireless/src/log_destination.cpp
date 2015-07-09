#include "log_destination.h"
#include "hub4_wireless.h"

LogDestination::LogDestination(Hub4Wireless *setup):
	mSetup(setup)
{
}

void LogDestination::write(const QString &message, QsLogging::Level level)
{
	Q_UNUSED(level);
	if (!mSetup.isNull())
		mSetup->writeToLog(message);
}

bool LogDestination::isValid()
{
	return !mSetup.isNull();
}

