# Application version
VERSION = 1.5.1

# suppress the mangling of va_arg has changed for gcc 4.4
QMAKE_CXXFLAGS += -Wno-psabi

# these warnings appear when compiling with QT4.8.3-debug. Problem appears to be
# solved in newer QT versions.
QMAKE_CXXFLAGS += -Wno-unused-local-typedefs

# Add more folders to ship with the application here
unix {
    bindir = $$(bindir)
    DESTDIR = $$(DESTDIR)
    isEmpty(bindir) {
        bindir = /usr/local/bin
    }
    INSTALLS += target
    target.path = $${DESTDIR}$${bindir}
}

MOC_DIR=.moc
OBJECTS_DIR=.obj

QT += core dbus
QT -= gui

TARGET = dbus-cgwacs
CONFIG += console
CONFIG -= app_bundle
DEFINES += VERSION=\\\"$${VERSION}\\\"

TEMPLATE = app

include(ext/qslog/QsLog.pri)

INCLUDEPATH += \
    ext/qslog \
    ext/velib/inc \
    ext/velib/inc/velib/platform \
    src

SOURCES += \
    ext/velib/src/qt/v_busitem.cpp \
    ext/velib/src/qt/v_busitems.cpp \
    ext/velib/src/qt/v_busitem_adaptor.cpp \
    ext/velib/src/qt/v_busitem_private_cons.cpp \
    ext/velib/src/qt/v_busitem_private_prod.cpp \
    ext/velib/src/qt/v_busitem_proxy.cpp \
    ext/velib/src/plt/serial.c \
    ext/velib/src/plt/posix_serial.c \
    ext/velib/src/plt/posix_ctx.c \
    ext/velib/src/types/ve_variant.c \
    src/main.cpp \
    src/dbus_bridge.cpp \
    src/power_info.cpp \
    src/modbus_rtu.cpp \
    src/v_bus_node.cpp \
    src/crc16.cpp \
    src/settings.cpp \
    src/multi.cpp \
    src/multi_phase_data.cpp \
    src/control_loop.cpp \
    src/settings_bridge.cpp \
    src/multi_bridge.cpp \
    src/data_processor.cpp \
    src/dbus_service_monitor.cpp \
    src/ac_sensor.cpp \
    src/ac_sensor_bridge.cpp \
    src/ac_sensor_settings.cpp \
    src/ac_sensor_settings_bridge.cpp \
    src/ac_sensor_updater.cpp \
    src/dbus_cgwacs.cpp \
    src/hub4_control_bridge.cpp \
    src/phase_compensation_control.cpp \
    src/single_phase_control.cpp \
    src/charge_phase_control.cpp \
    src/battery.cpp \
    src/multi_phase_control.cpp \
    src/split_phase_control.cpp \
    src/battery_info.cpp \
    src/ac_sensor_mediator.cpp \
    src/battery_life.cpp \
    src/system_calc.cpp \
    src/vbus_item_system_calc.cpp \
    src/vbus_item_battery.cpp

HEADERS += \
    ext/velib/src/qt/v_busitem_adaptor.h \
    ext/velib/src/qt/v_busitem_private_cons.h \
    ext/velib/src/qt/v_busitem_private_prod.h \
    ext/velib/src/qt/v_busitem_private.h \
    ext/velib/src/qt/v_busitem_proxy.h \
    ext/velib/inc/velib/qt/v_busitem.h \
    ext/velib/inc/velib/qt/v_busitems.h \
    ext/velib/inc/velib/platform/serial.h \
    src/dbus_bridge.h \
    src/power_info.h \
    src/defines.h \
    src/settings.h \
    src/modbus_rtu.h \
    src/v_bus_node.h \
    src/crc16.h \
    src/multi.h \
    src/multi_phase_data.h \
    src/control_loop.h \
    src/settings_bridge.h \
    src/multi_bridge.h \
    src/velib/velib_config_app.h \
    src/data_processor.h \
    src/dbus_service_monitor.h \
    src/ac_sensor.h \
    src/ac_sensor_bridge.h \
    src/ac_sensor_settings.h \
    src/ac_sensor_updater.h \
    src/dbus_cgwacs.h \
    src/ac_sensor_settings_bridge.h \
    src/hub4_control_bridge.h \
    src/fastclock.h \
    src/phase_compensation_control.h \
    src/single_phase_control.h \
    src/charge_phase_control.h \
    src/battery.h \
    src/multi_phase_control.h \
    src/split_phase_control.h \
    src/battery_info.h \
    src/ac_sensor_mediator.h \
    src/battery_life.h \
    src/system_calc.h \
    src/vbus_item_system_calc.h \
    src/vbus_item_battery.h

DISTFILES += \
    ../documents/battery_life.dot \
    ../.gitlab-ci.yml
