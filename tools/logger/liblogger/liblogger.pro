#-------------------------------------------------
#
# Project created by QtCreator 2012-11-28T14:59:53
#
#-------------------------------------------------

QT       -= gui

TARGET = liblogger
TEMPLATE = lib

DEFINES += LIBLOGGER_LIBRARY

SOURCES += \
    logparser.cpp \
    packet.cpp \
    bytearrayutils.cpp \
    hexdump.cpp 

HEADERS  += \
    logparser.h \
    packet.h \
    bytearrayutils.h \
    hexdump.h \
    bytearrayutils.h

INCLUDEPATH += ../../../d7aoss/


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../external/serialport/release/ -lSerialPort
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../external/serialport/debug/ -lSerialPort
else:unix: LIBS += -L$$OUT_PWD/../external/serialport/ -lSerialPort

INCLUDEPATH += $$PWD/../external/serialport
DEPENDPATH += $$PWD/../external/serialport
