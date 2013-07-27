#-------------------------------------------------
#
# Project created by QtCreator 2012-11-28T14:59:53
#
#-------------------------------------------------

QT -= gui
TARGET = logger
TEMPLATE = lib

DEFINES += LOGGER_LIBRARY

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
