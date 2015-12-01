QT += core
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
    $$EXTDIR/googletest/googletest/src/gtest-all.cc \
    $$SRCDIR/ac_sensor.cpp \
    $$SRCDIR/ac_sensor_settings.cpp \
    $$SRCDIR/control_loop.cpp \
    $$SRCDIR/maintenance_control.cpp \
    $$SRCDIR/multi.cpp \
    $$SRCDIR/multi_phase_data.cpp \
    $$SRCDIR/power_info.cpp \
    $$SRCDIR/phase_compensation_control.cpp \
    $$SRCDIR/settings.cpp \
    $$SRCDIR/single_phase_control.cpp \
    main.cpp \
    ac_in_power_control_test.cpp \
    control_loop_test.cpp \
    maintenance_control_test.cpp

HEADERS += \
    $$SRCDIR/ac_sensor.h \
    $$SRCDIR/ac_sensor_settings.h \
    $$SRCDIR/control_loop.h \
    $$SRCDIR/maintenance_control.h \
    $$SRCDIR/multi.h \
    $$SRCDIR/multi_phase_data.h \
    $$SRCDIR/power_info.h \
    $$SRCDIR/phase_compensation_control.h \
    $$SRCDIR/settings.h \
    $$SRCDIR/single_phase_control.h \
    ac_in_power_control_test.h \
    control_loop_test.h \
    maintenance_control_test.h
