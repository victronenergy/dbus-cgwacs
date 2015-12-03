#ifndef FASTCLOCK_H
#define FASTCLOCK_H

#include "control_loop.h"

/**
 * Simulates a clock that runs faster than real time. Use for testing only.
 */
class FastClock : public Clock
{
public:
	/**
	 * Constructor
	 * @param factor Speed up factor. For 1 second in passed in real time,
	 * `factor` simulated seconds will pass.
	 */
	FastClock(int factor):
		mFactor(factor)
	{
		if (!mStartTime.isValid())
			mStartTime = QDateTime::currentDateTime();
	}

	virtual QDateTime now() const
	{
		QDateTime now = QDateTime::currentDateTime();
		return mStartTime.addMSecs(mStartTime.msecsTo(now) * mFactor);
	}

private:
	int mFactor;
	static QDateTime mStartTime;
};

#endif // FASTCLOCK_H
