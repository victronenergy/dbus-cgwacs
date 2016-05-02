#include <velib/qt/v_busitem.h>
#include "dbus_service_monitor.h"
#include "vbus_item_system_calc.h"

static const QString SystemCalcService = "com.victronenergy.system";
static const QString SocPath = "/Dc/Battery/Soc";

VBusItemSystemCalc::VBusItemSystemCalc(DbusServiceMonitor *serviceMonitor, QObject *parent):
	SystemCalc(parent),
	mSystemSoc(new VBusItem(this))
{
	connect(serviceMonitor, SIGNAL(serviceAdded(QString)), this, SLOT(onServiceAdded(QString)));
	connect(mSystemSoc, SIGNAL(valueChanged()), this, SIGNAL(socChanged()));
	mSystemSoc->consume(SystemCalcService, SocPath);
	mSystemSoc->getValue();
}

double VBusItemSystemCalc::soc() const
{
	QVariant v = mSystemSoc->getValue();
	return v.isValid() ? v.toDouble() : qQNaN();
}

void VBusItemSystemCalc::onServiceAdded(QString service)
{
	if (service == SystemCalcService)
		mSystemSoc->consume(SystemCalcService, SocPath);
}
