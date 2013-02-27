#-------------------------------------------------
#
# Project created by QtCreator 2013-01-15T13:08:17
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = logger-cli
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    clilogger.cpp

INCLUDEPATH += ../../../d7aoss/ # TODO remove?

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../liblogger/release/ -llogger
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../liblogger/debug/ -llogger
else:unix: LIBS += -L$$OUT_PWD/../liblogger/ -llogger

INCLUDEPATH += $$PWD/../liblogger
DEPENDPATH += $$PWD/../liblogger

HEADERS += \
    clilogger.h

# link with serialport TODO remove?
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../external/serialport/release/ -lSerialPort
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../external/serialport/debug/ -lSerialPortd0
else:unix: LIBS += -L$$OUT_PWD/../external/serialport/ -lSerialPort

INCLUDEPATH += $$PWD/../external/serialport
DEPENDPATH += $$PWD/../external/serialport
