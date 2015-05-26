# Application version and revision
VERSION = 1.0.0
REVISION = $$system(git --git-dir $$PWD/../.git --work-tree $$PWD describe --always --dirty --tags)

# suppress the mangling of va_arg has changed for gcc 4.4
QMAKE_CXXFLAGS += -Wno-psabi

# these warnings appear when compiling with QT4.8.3-debug. Problem appears to be
# solved in newer QT versions.
QMAKE_CXXFLAGS += -Wno-unused-local-typedefs

target.path = /opt/color-control/logger
INSTALLS += target

machine=$$(MACHINE)
contains(machine,ccgx) {
    DEFINES += TARGET_ccgx
}

# Note: we need the script module to parse JSON fragments as part of the
# fronius solar API.
QT += core network script xml
QT -= gui

TARGET = logger
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

include(../software/ext/qslog/QsLog.pri)

INCLUDEPATH += \
    ../software/src \
    ../software/ext/qslog \
    ../software/ext/velib/inc \
    ../software/ext/velib/lib/Qvelib \
    ../software/ext/qextserialport/src/extserialport \
    ../software/ext/velib/inc/velib/platform

SOURCES += \
    ../software/src/crc16.cpp \
    ../software/src/modbus_rtu.cpp \
    ../software/src/variables.cpp \
    ../software/ext/velib/src/plt/serial.c \
    ../software/ext/velib/src/plt/posix_serial.c \
    ../software/ext/velib/src/plt/posix_ctx.c \
    ../software/ext/velib/src/types/ve_variant.c \
    src/main.cpp \
    src/logger.cpp

HEADERS += \
    ../software/src/crc16.h \
    ../software/src/modbus_rtu.h \
    ../software/src/variables.h \
    ../software/ext/velib/inc/velib/platform/serial.h \
    src/logger.h
