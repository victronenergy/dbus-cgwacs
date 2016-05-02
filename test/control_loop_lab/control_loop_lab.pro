QT += core dbus
QT -= gui

TARGET = control_loop_lab
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

# suppress the mangling of va_arg has changed for gcc 4.4
QMAKE_CXXFLAGS += -Wno-psabi

# these warnings appear when compiling with QT4.8.3-debug. Problem appears to be
# solved in newer QT versions.
QMAKE_CXXFLAGS += -Wno-unused-local-typedefs

include(../../software/ext/qslog/QsLog.pri)

SRCDIR = ../../software/src
EXTDIR = ../../software/ext

INCLUDEPATH += \
    $$EXTDIR/velib/inc \
    $$EXTDIR/velib/lib/Qvelib \
    $$SRCDIR

SOURCES += \
    $$EXTDIR/velib/src/qt/v_busitem.cpp \
    $$EXTDIR/velib/src/qt/v_busitems.cpp \
    $$EXTDIR/velib/src/qt/v_busitem_adaptor.cpp \
    $$EXTDIR/velib/src/qt/v_busitem_private_cons.cpp \
    $$EXTDIR/velib/src/qt/v_busitem_private_prod.cpp \
    $$EXTDIR/velib/src/qt/v_busitem_proxy.cpp \
    $$SRCDIR/ac_sensor.cpp \
    $$SRCDIR/ac_sensor_settings.cpp \
    $$SRCDIR/control_loop.cpp \
    $$SRCDIR/multi.cpp \
    $$SRCDIR/power_info.cpp \
    $$SRCDIR/phase_compensation_control.cpp \
    $$SRCDIR/multi_phase_data.cpp \
    $$SRCDIR/settings.cpp \
    $$SRCDIR/single_phase_control.cpp \
    $$SRCDIR/multi_phase_control.cpp \
    $$SRCDIR/split_phase_control.cpp \
    $$SRCDIR/dbus_bridge.cpp \
    $$SRCDIR/battery.cpp \
    $$SRCDIR/battery_bridge.cpp \
    $$SRCDIR/battery_info.cpp \
    $$SRCDIR/v_bus_node.cpp \
    $$SRCDIR/charge_phase_control.cpp \
    main.cpp

HEADERS += \
    $$EXTDIR/velib/src/qt/v_busitem_adaptor.h \
    $$EXTDIR/velib/src/qt/v_busitem_private_cons.h \
    $$EXTDIR/velib/src/qt/v_busitem_private_prod.h \
    $$EXTDIR/velib/src/qt/v_busitem_private.h \
    $$EXTDIR/velib/src/qt/v_busitem_proxy.h \
    $$EXTDIR/velib/inc/velib/qt/v_busitem.h \
    $$EXTDIR/velib/inc/velib/qt/v_busitems.h \
    $$SRCDIR/ac_sensor.h \
    $$SRCDIR/ac_sensor_settings.h \
    $$SRCDIR/control_loop.h \
    $$SRCDIR/multi.h \
    $$SRCDIR/power_info.h \
    $$SRCDIR/phase_compensation_control.h \
    $$SRCDIR/multi_phase_data.h \
    $$SRCDIR/settings.h \
    $$SRCDIR/single_phase_control.h \
    $$SRCDIR/multi_phase_control.h \
    $$SRCDIR/split_phase_control.h \
    $$SRCDIR/dbus_bridge.h \
    $$SRCDIR/battery.h \
    $$SRCDIR/battery_bridge.h \
    $$SRCDIR/battery_info.h \
    $$SRCDIR/v_bus_node.h \
    $$SRCDIR/charge_phase_control.h
