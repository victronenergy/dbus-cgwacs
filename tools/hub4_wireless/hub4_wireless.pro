QT += core serialport qml widgets quick

TARGET = hub4_wireless
CONFIG -= app_bundle

TEMPLATE = app

deploy.commands = windeployqt.exe --dir target.path/deploy --qmldir .\qml hub4_wireless.exe

include(../../software/ext/qslog/QsLog.pri)

INCLUDEPATH += \
    ../../software/src \
    ../../software/ext/qslog

HEADERS += \
    src/drf1600.h \
    src/drf1600_detector.h \
    src/com_test.h \
    src/log_destination.h \
    src/hub4_wireless.h

SOURCES += \
    src/main.cpp \
    src/drf1600.cpp \
    src/drf1600_detector.cpp \
    src/com_test.cpp \
    src/log_destination.cpp \
    src/hub4_wireless.cpp

RESOURCES += qml.qrc

DISTFILES += \
    deploy.bat
