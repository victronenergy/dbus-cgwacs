#include <qnumeric.h>
#include <velib/qt/ve_qitem.hpp>
#include "vbus_item_system_calc.h"

static const QString SystemCalcService = "sub/com.victronenergy.system/Dc/Battery/Soc";

VBusItemSystemCalc::VBusItemSystemCalc(QObject *parent):
	SystemCalc(parent),
	mSystemSoc(VeQItems::getRoot()->itemGetOrCreate(SystemCalcService))
{
	connect(mSystemSoc, SIGNAL(valueChanged(VeQItem *, QVariant)), this, SIGNAL(socChanged()));
	mSystemSoc->getValue();
}

double VBusItemSystemCalc::soc() const
{
	switch (mSystemSoc->getState()) {
	case VeQItem::Idle:
	case VeQItem::Offline:
	case VeQItem::Requested:
		return qQNaN();
	default:
		QVariant v = mSystemSoc->getValue();
		return v.isValid() ? v.toDouble() : qQNaN();
	}
}
