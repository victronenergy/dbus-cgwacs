# Application version and revision
VERSION = 1.0.0
REVISION = $$system(git --git-dir $$PWD/../.git --work-tree $$PWD describe --always --dirty --tags)

# Create a include file with VERION / REVISION
version_rule.target = $$OUT_PWD/version.h
version_rule.commands = @echo \"updating file $$revtarget.target\"; \
    printf \"/* generated file (do not edit) */\\n \
    $${LITERAL_HASH}ifndef VERSION_H\\n \
    $${LITERAL_HASH}define VERSION_H\\n \
    $${LITERAL_HASH}define VERSION \\\"$${VERSION}\\\"\\n \
    $${LITERAL_HASH}define REVISION \\\"$${REVISION}\\\"\\n \
    $${LITERAL_HASH}endif\" > $$version_rule.target
version_rule.depends = FORCE
QMAKE_DISTCLEAN += $$version_rule.target

QMAKE_EXTRA_TARGETS += version_rule
PRE_TARGETDEPS += $$OUT_PWD/version.h

# suppress the mangling of va_arg has changed for gcc 4.4
QMAKE_CXXFLAGS += -Wno-psabi

# these warnings appear when compiling with QT4.8.3-debug. Problem appears to be
# solved in newer QT versions.
QMAKE_CXXFLAGS += -Wno-unused-local-typedefs

target.path = /opt/color-control/em_sim
INSTALLS += target

machine=$$(MACHINE)
contains(machine,ccgx) {
    DEFINES += TARGET_ccgx
}

# Note: we need the script module to parse JSON fragments as part of the
# fronius solar API.
QT += core dbus network script xml
QT -= gui

TARGET = em_sim
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

include(../software/ext/qslog/QsLog.pri)

INCLUDEPATH += \
    ../software/src \
    ../software/ext/qslog \
    ../software/ext/velib/inc \
    ../software/ext/velib/lib/Qvelib \
    ../software/ext/velib/inc/velib/platform

SOURCES += \
    ../software/ext/velib/src/qt/v_busitem.cpp \
    ../software/ext/velib/src/qt/v_busitems.cpp \
    ../software/ext/velib/src/qt/v_busitem_adaptor.cpp \
    ../software/ext/velib/src/qt/v_busitem_private_cons.cpp \
    ../software/ext/velib/src/qt/v_busitem_private_prod.cpp \
    ../software/ext/velib/src/qt/v_busitem_proxy.cpp \
    ../software/src/ac_sensor.cpp \
    ../software/src/ac_sensor_bridge.cpp \
    ../software/src/ac_sensor_settings.cpp \
    ../software/src/control_loop.cpp \
    ../software/src/dbus_bridge.cpp \
    ../software/src/multi.cpp \
    ../software/src/multi_bridge.cpp \
    ../software/src/multi_phase_data.cpp \
    ../software/src/power_info.cpp \
    ../software/src/settings.cpp \
    ../software/src/v_bus_node.cpp \
    src/em_sim_main.cpp \
    src/ac_sensor_sim.cpp

HEADERS += \
    ../software/ext/velib/src/qt/v_busitem_adaptor.h \
    ../software/ext/velib/src/qt/v_busitem_private_cons.h \
    ../software/ext/velib/src/qt/v_busitem_private_prod.h \
    ../software/ext/velib/src/qt/v_busitem_private.h \
    ../software/ext/velib/src/qt/v_busitem_proxy.h \
    ../software/ext/velib/inc/velib/qt/v_busitem.h \
    ../software/ext/velib/inc/velib/qt/v_busitems.h \
    ../software/src/ac_sensor_bridge.h \
    ../software/src/ac_sensor_settings.h \
    ../software/src/control_loop.h \
    ../software/src/dbus_bridge.h \
    ../software/src/multi_bridge.h \
    ../software/src/v_bus_node.h \
    ../software/src/ac_sensor.h \
    ../software/src/multi.h \
    ../software/src/multi_phase_data.h \
    ../software/src/power_info.h \
    ../software/src/settings.h \
    src/em_sim_main.cpp \
    src/ac_sensor_sim.h
