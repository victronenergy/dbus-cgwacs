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
	Q_PROPERTY(double acPowerIn READ acPowerIn WRITE setAcPowerIn NOTIFY acPowerOutChanged)
public:
	explicit MultiPhaseData(QObject *parent = 0);

	double acPowerIn() const;

	void setAcPowerIn(double o);

signals:
	void acPowerOutChanged();

private:
	double mAcPowerIn;
};

#endif // MULTI_PHASE_DATA_H
