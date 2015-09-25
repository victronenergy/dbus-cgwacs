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
    $$SRCDIR/control_loop.cpp \
    $$SRCDIR/multi.cpp \
    $$SRCDIR/power_info.cpp \
    $$SRCDIR/multi_phase_data.cpp \
    $$SRCDIR/settings.cpp \
    main.cpp \
    control_loop_test.cpp

HEADERS += \
    $$SRCDIR/ac_sensor.h \
    $$SRCDIR/control_loop.h \
    $$SRCDIR/multi.h \
    $$SRCDIR/power_info.h \
    $$SRCDIR/multi_phase_data.h \
    $$SRCDIR/settings.h \
    control_loop_test.h
