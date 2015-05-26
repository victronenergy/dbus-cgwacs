#ifndef DBUSSERVICEMONITOR_H
#define DBUSSERVICEMONITOR_H

#include <QObject>

class DbusServiceMonitor : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QList<QString> services READ services NOTIFY servicesChanged)
public:
	explicit DbusServiceMonitor(const QString &prefix, QObject *parent = 0);

	QList<QString> services() const;

signals:
	void servicesChanged();

private slots:
	void onServiceOwnerChanged(const QString &name, const QString &oldOwner,
							   const QString &newOwner);

private:
	void processNewService(const QString &name);

	void processOldService(const QString &name);

	QString mPrefix;
	QList<QString> mServices;
};

#endif // DBUSSERVICEMONITOR_H
