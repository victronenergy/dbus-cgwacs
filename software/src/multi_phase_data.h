#ifndef MULTI_PHASE_DATA_H
#define MULTI_PHASE_DATA_H

#include <QObject>
#include <QTime>

/*!
 * Contains power data from a single phase.
 */
class MultiPhaseData : public QObject
{
	Q_OBJECT
	Q_PROPERTY(double acPowerIn READ acPowerIn WRITE setAcPowerIn NOTIFY acPowerInChanged)
	Q_PROPERTY(double acPowerOut READ acPowerOut WRITE setAcPowerOut NOTIFY acPowerOutChanged)
	Q_PROPERTY(double acPowerSetPoint READ acPowerSetPoint WRITE setAcPowerSetPoint NOTIFY acPowerSetPointChanged)
	Q_PROPERTY(bool isSetPointAvailable READ isSetPointAvailable NOTIFY isSetPointAvailableChanged)
public:
	explicit MultiPhaseData(QObject *parent = 0);

	double acPowerIn() const;

	void setAcPowerIn(double o);

	double acPowerOut() const;

	void setAcPowerOut(double o);

	double acPowerSetPoint() const;

	void setAcPowerSetPoint(double o);

	bool isSetPointAvailable() const;

signals:
	void acPowerInChanged();

	void acPowerOutChanged();

	void acPowerSetPointChanged();

	void isSetPointAvailableChanged();

private:
	double mAcPowerIn;
	double mAcPowerOut;
	double mAcPowerSetPoint;
	bool mIsSetPointAvailable;
};

#endif // MULTI_PHASE_DATA_H
