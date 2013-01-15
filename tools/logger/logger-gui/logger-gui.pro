QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = logger-gui
TEMPLATE = app


SOURCES += main.cpp\
    mainwindow.cpp \
    qcustomplot/qcustomplot.cpp

HEADERS  += \
    mainwindow.h \
    qcustomplot/qcustomplot.h

FORMS    += \
    mainwindow.ui

RESOURCES += \
    resources.qrc

INCLUDEPATH += ../../../d7aoss/ # TODO remove?

# link with liblogger
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../liblogger/release/ -lliblogger
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../liblogger/debug/ -lliblogger
else:unix: LIBS += -L$$OUT_PWD/../liblogger/ -lliblogger

INCLUDEPATH += $$PWD/../liblogger
DEPENDPATH += $$PWD/../liblogger

# link with serialport TODO remove?
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../external/serialport/release/ -lSerialPort
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../external/serialport/debug/ -lSerialPort
else:unix: LIBS += -L$$OUT_PWD/../external/serialport/ -lSerialPort

INCLUDEPATH += $$PWD/../external/serialport
DEPENDPATH += $$PWD/../external/serialport
