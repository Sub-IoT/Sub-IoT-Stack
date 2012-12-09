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
    logparser.cpp \
    packet.cpp \
    bytearrayutils.cpp \
    hexdump.cpp \
    mainwindow.cpp

HEADERS  += \
    logparser.h \
    packet.h \
    bytearrayutils.h \
    hexdump.h \
    bytearrayutils.h \
    mainwindow.h

FORMS    += \
    mainwindow.ui

INCLUDEPATH += ../../d7aoss/

RESOURCES += \
    resources.qrc
