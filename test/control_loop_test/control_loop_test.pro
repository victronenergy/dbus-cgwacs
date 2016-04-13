QT += core dbus
QT -= gui

TARGET = control_loop_test
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

include(../../software/ext/qslog/QsLog.pri)

SRCDIR = ../../software/src
EXTDIR = ../../software/ext

INCLUDEPATH += \
    ../../software/ext/velib/inc \
    ../../software/ext/velib/lib/Qvelib \
    ../../software/ext/googletest/googletest/include \
    ../../software/ext/googletest/googletest \
    ../../software/src

SOURCES += \
    $$EXTDIR/velib/src/qt/v_busitem.cpp \
    $$EXTDIR/velib/src/qt/v_busitems.cpp \
    $$EXTDIR/velib/src/qt/v_busitem_adaptor.cpp \
    $$EXTDIR/velib/src/qt/v_busitem_private_cons.cpp \
    $$EXTDIR/velib/src/qt/v_busitem_private_prod.cpp \
    $$EXTDIR/velib/src/qt/v_busitem_proxy.cpp \
    $$EXTDIR/googletest/googletest/src/gtest-all.cc \
    $$SRCDIR/ac_sensor.cpp \
    $$SRCDIR/ac_sensor_settings.cpp \
    $$SRCDIR/battery.cpp \
    $$SRCDIR/battery_bridge.cpp \
    $$SRCDIR/battery_info.cpp \
    $$SRCDIR/control_loop.cpp \
    $$SRCDIR/dbus_bridge.cpp \
    $$SRCDIR/maintenance_control.cpp \
    $$SRCDIR/multi.cpp \
    $$SRCDIR/multi_phase_data.cpp \
    $$SRCDIR/power_info.cpp \
    $$SRCDIR/phase_compensation_control.cpp \
    $$SRCDIR/settings.cpp \
    $$SRCDIR/single_phase_control.cpp \
    $$SRCDIR/multi_phase_control.cpp \
    $$SRCDIR/v_bus_node.cpp \
    main.cpp \
    ac_in_power_control_test.cpp \
    control_loop_test.cpp

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
    $$SRCDIR/battery.h \
    $$SRCDIR/battery_bridge.h \
    $$SRCDIR/battery_info.h \
    $$SRCDIR/control_loop.h \
    $$SRCDIR/dbus_bridge.h \
    $$SRCDIR/maintenance_control.h \
    $$SRCDIR/multi.h \
    $$SRCDIR/multi_phase_data.h \
    $$SRCDIR/power_info.h \
    $$SRCDIR/phase_compensation_control.h \
    $$SRCDIR/settings.h \
    $$SRCDIR/single_phase_control.h \
    $$SRCDIR/multi_phase_control.h \
    $$SRCDIR/v_bus_node.h \
    ac_in_power_control_test.h \
    control_loop_test.h
