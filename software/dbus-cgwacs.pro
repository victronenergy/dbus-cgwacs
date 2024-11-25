# Application version
VERSION = 2.0.24

# suppress the mangling of va_arg has changed for gcc 4.4
QMAKE_CXXFLAGS += -Wno-psabi

# these warnings appear when compiling with QT4.8.3-debug. Problem appears to be
# solved in newer QT versions.
QMAKE_CXXFLAGS += -Wno-unused-local-typedefs

equals(QT_MAJOR_VERSION, 6): QMAKE_CXXFLAGS += -std=c++17

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

QT += core dbus xml
QT -= gui

TARGET = dbus-cgwacs
CONFIG += console
CONFIG -= app_bundle
DEFINES += VERSION=\\\"$${VERSION}\\\"

TEMPLATE = app

include(ext/qslog/QsLog.pri)
include(ext/veutil/src/qt/veqitem.pri)

INCLUDEPATH += \
    ext/qslog \
    ext/velib/inc \
    ext/velib/inc/velib/platform \
    src

SOURCES += \
    ext/velib/src/plt/serial.c \
    ext/velib/src/plt/posix_serial.c \
    ext/velib/src/plt/posix_ctx.c \
    ext/velib/src/types/ve_variant.c \
    src/ac_sensor.cpp \
    src/ac_sensor_bridge.cpp \
    src/ac_sensor_mediator.cpp \
    src/ac_sensor_settings.cpp \
    src/ac_sensor_settings_bridge.cpp \
    src/ac_sensor_updater.cpp \
    src/crc16.cpp \
    src/data_processor.cpp \
    src/dbus_bridge.cpp \
    src/main.cpp \
    src/modbus_rtu.cpp \
    src/ac_sensor_phase.cpp

HEADERS += \
    ext/velib/inc/velib/platform/serial.h \
    src/ac_sensor.h \
    src/ac_sensor_bridge.h \
    src/ac_sensor_mediator.h \
    src/ac_sensor_settings.h \
    src/ac_sensor_settings_bridge.h \
    src/ac_sensor_updater.h \
    src/crc16.h \
    src/data_processor.h \
    src/dbus_bridge.h \
    src/defines.h \
    src/modbus_rtu.h \
    src/velib/velib_config_app.h \
    src/ac_sensor_phase.h

DISTFILES += \
    ../README.md

*g++* {
    QMAKE_CXX += -Wno-class-memaccess -Wno-deprecated-copy
}
