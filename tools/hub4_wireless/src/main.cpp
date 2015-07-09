#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "hub4_wireless.h"

int main(int argc, char *argv[])
{
	Hub4Wireless a(argc, argv);

	QQmlApplicationEngine engine;
	QQmlContext *ctx = engine.rootContext();
	ctx->setContextProperty("model", &a);
	engine.load(QUrl("qrc:/qml/main.qml"));

	return a.exec();
}
