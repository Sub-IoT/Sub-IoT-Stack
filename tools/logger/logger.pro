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
        loggerdialog.cpp

HEADERS  += loggerdialog.h

FORMS    += loggerdialog.ui
