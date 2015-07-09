#ifndef ZIGBEESETUP_H
#define ZIGBEESETUP_H

#include <QApplication>
#include <QStringList>

class QQmlApplicationEngine;

class Hub4Wireless : public QApplication
{
	Q_OBJECT
	Q_PROPERTY(QString log READ log NOTIFY logChanged)
	Q_PROPERTY(QString info READ info NOTIFY infoChanged)
public:
	Hub4Wireless(int &argc, char **argv);

	QString log() const;

	QString info() const;

	Q_INVOKABLE void programCoordinator();

	Q_INVOKABLE void programRouter();

	void writeToLog(const QString &message);

signals:
	void logChanged();

	void infoChanged();

private slots:
	void onCoordinatorSetupFinished(bool s);

	void onRouterSetupFinished(bool s);

private:
	void setInfo(const QString &t);

	void setLog(const QString &l);

	QStringList findSerialPorts(const QString &manufacturer,
								const QString &description);

	QString mInfo;
	QString mLog;
};

#endif // ZIGBEESETUP_H
