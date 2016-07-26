QT += core
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
TESTDIR = ../src

INCLUDEPATH += \
    $$EXTDIR/velib/inc \
    $$EXTDIR/velib/lib/Qvelib \
    $$SRCDIR \
    $$TESTDIR

SOURCES += \
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
    $$SRCDIR/battery.cpp \
    $$SRCDIR/battery_info.cpp \
    $$SRCDIR/system_calc.cpp \
    $$SRCDIR/charge_phase_control.cpp \
    $$TESTDIR/mock_system_calc.cpp \
    main.cpp

HEADERS += \
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
    $$SRCDIR/battery.h \
    $$SRCDIR/battery_info.h \
    $$SRCDIR/system_calc.h \
    $$SRCDIR/charge_phase_control.h \
    $$TESTDIR/mock_system_calc.h
