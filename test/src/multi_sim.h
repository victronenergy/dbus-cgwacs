#ifndef MULTI_SIM_H
#define MULTI_SIM_H

#include <QObject>

class QTimer;

class MultiSim : public QObject
{
	Q_OBJECT
	Q_PROPERTY(double powerOut READ powerOut WRITE setPowerOut NOTIFY powerOutChanged)
	Q_PROPERTY(double powerOutSetPoint READ powerOutSetPoint WRITE setPowerOutSetPoint NOTIFY powerOutSetPointChanged)
public:
	explicit MultiSim(QObject *parent = 0);

	double powerOut() const;

	void setPowerOut(double p);

	double powerOutSetPoint() const;

	void setPowerOutSetPoint(double p);

signals:
	void powerOutChanged();

	void powerOutSetPointChanged();

private slots:
	void onTimer();

private:
	double mPowerOut;
	double mPowerOutSetPoint;
	QTimer *mTimer;
	int mTimerCount;
};

#endif // MULTI_SIM_H
