QT += core
QT -= gui

TARGET = maintenance_lab
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

include(../../software/ext/qslog/QsLog.pri)

SRCDIR = ../../software/src
EXTDIR = ../../software/ext

INCLUDEPATH += \
    ../../software/ext/velib/inc \
    ../../software/ext/velib/lib/Qvelib \
    ../../software/src

SOURCES += \
    $$SRCDIR/maintenance_control.cpp \
    $$SRCDIR/multi.cpp \
    $$SRCDIR/multi_phase_data.cpp \
    $$SRCDIR/settings.cpp \
    main.cpp

HEADERS += \
    $$SRCDIR/maintenance_control.h \
    $$SRCDIR/multi.h \
    $$SRCDIR/multi_phase_data.h \
    $$SRCDIR/settings.h

DISTFILES += \
    plot.py
