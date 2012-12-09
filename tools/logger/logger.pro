#-------------------------------------------------
#
# Project created by QtCreator 2012-11-28T14:59:53
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include(serialport/serialport-lib.pri)

TARGET = logger
TEMPLATE = app


SOURCES += main.cpp\
        loggerdialog.cpp \
    logparser.cpp \
    packet.cpp \
    bytearrayutils.cpp \
    hexdump.cpp

HEADERS  += loggerdialog.h \
    logparser.h \
    packet.h \
    bytearrayutils.h \
    hexdump.h

FORMS    += loggerdialog.ui

INCLUDEPATH += ../../d7aoss/
