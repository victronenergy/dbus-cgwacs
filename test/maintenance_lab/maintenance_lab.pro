QT += core
QT -= gui

TARGET = maintenance_lab
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

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
    $$SRCDIR/battery_life.cpp \
    $$SRCDIR/multi.cpp \
    $$SRCDIR/multi_phase_data.cpp \
    $$SRCDIR/settings.cpp \
    $$SRCDIR/system_calc.cpp \
    $$TESTDIR/mock_system_calc.cpp \
    main.cpp

HEADERS += \
    $$SRCDIR/battery_life.h \
    $$SRCDIR/multi.h \
    $$SRCDIR/multi_phase_data.h \
    $$SRCDIR/settings.h \
    $$SRCDIR/system_calc.h \
    $$TESTDIR/mock_system_calc.h

DISTFILES += \
    plot.py
