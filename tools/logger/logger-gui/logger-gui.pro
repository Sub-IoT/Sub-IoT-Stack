QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = logger-gui
TEMPLATE = app


SOURCES += main.cpp\
    mainwindow.cpp \
    qcustomplot/qcustomplot.cpp \
    connectdialog.cpp

HEADERS  += \
    mainwindow.h \
    qcustomplot/qcustomplot.h \
    connectdialog.h

FORMS    += \
    mainwindow.ui \
    connectdialog.ui

RESOURCES += \
    resources.qrc

INCLUDEPATH += ../../../d7aoss/ # TODO remove?

# link with liblogger
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../liblogger/release/ -llogger
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../liblogger/debug/ -llogger
else:unix: LIBS += -L$$OUT_PWD/../liblogger/ -llogger

INCLUDEPATH += $$PWD/../liblogger
DEPENDPATH += $$PWD/../liblogger

