QT += core
QT -= gui

TARGET = control_loop_test
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
    $$EXTDIR/googletest/googletest/include \
    $$EXTDIR/googletest/googletest \
    $$SRCDIR \
    $$TESTDIR

SOURCES += \
    $$EXTDIR/googletest/googletest/src/gtest-all.cc \
    $$SRCDIR/ac_sensor.cpp \
    $$SRCDIR/ac_sensor_settings.cpp \
    $$SRCDIR/battery.cpp \
    $$SRCDIR/battery_info.cpp \
    $$SRCDIR/control_loop.cpp \
    $$SRCDIR/battery_life.cpp \
    $$SRCDIR/multi.cpp \
    $$SRCDIR/multi_phase_data.cpp \
    $$SRCDIR/power_info.cpp \
    $$SRCDIR/phase_compensation_control.cpp \
    $$SRCDIR/settings.cpp \
    $$SRCDIR/single_phase_control.cpp \
    $$SRCDIR/multi_phase_control.cpp \
    $$SRCDIR/system_calc.cpp \
    $$TESTDIR/mock_system_calc.cpp \
    main.cpp \
    ac_in_power_control_test.cpp \
    maintenance_control_test.cpp \
    battery_info_test.cpp \
    mock_battery.cpp

HEADERS += \
    $$SRCDIR/ac_sensor.h \
    $$SRCDIR/ac_sensor_settings.h \
    $$SRCDIR/battery.h \
    $$SRCDIR/battery_info.h \
    $$SRCDIR/control_loop.h \
    $$SRCDIR/battery_life.h \
    $$SRCDIR/multi.h \
    $$SRCDIR/multi_phase_data.h \
    $$SRCDIR/power_info.h \
    $$SRCDIR/phase_compensation_control.h \
    $$SRCDIR/settings.h \
    $$SRCDIR/single_phase_control.h \
    $$SRCDIR/multi_phase_control.h \
    $$SRCDIR/system_calc.h \
    $$TESTDIR/mock_system_calc.h \
    ac_in_power_control_test.h \
    maintenance_control_test.h \
    battery_info_test.h \
    mock_battery.h
