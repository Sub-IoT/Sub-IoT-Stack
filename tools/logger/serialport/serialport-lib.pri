INCLUDEPATH += $$PWD

linux*:DEFINES += HAVE_LIBUDEV

PUBLIC_HEADERS += \
    $$PWD/serialport-global.h \
    $$PWD/serialport.h \
    $$PWD/serialportinfo.h

PRIVATE_HEADERS += \
    $$PWD/serialport_p.h \
    $$PWD/serialportinfo_p.h

SOURCES += \
    $$PWD/serialport.cpp \
    $$PWD/serialportinfo.cpp

win32 {
    PRIVATE_HEADERS += \
        $$PWD/serialport_win_p.h

    SOURCES += \
        $$PWD/serialport_win.cpp \
        $$PWD/serialportinfo_win.cpp

    !wince*: {
        LIBS += -lsetupapi -ladvapi32
    } else {
        SOURCES += \
            $$PWD/serialport_wince.cpp \
            $$PWD/serialportinfo_wince.cpp
    }
}

symbian {
    MMP_RULES += EXPORTUNFROZEN
    #MMP_RULES += DEBUGGABLE_UDEBONLY
    TARGET.UID3 = 0xE7E62DFD
    TARGET.CAPABILITY =
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = SerialPort.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles

    # FIXME !!!
    #INCLUDEPATH += c:/Nokia/devices/Nokia_Symbian3_SDK_v1.0/epoc32/include/platform
    INCLUDEPATH += c:/QtSDK/Symbian/SDKs/Symbian3Qt473/epoc32/include/platform

    PRIVATE_HEADERS += \
        $$PWD/serialport_symbian_p.h

    SOURCES += \
        $$PWD/serialport_symbian.cpp \
        $$PWD/serialportinfo_symbian.cpp

    LIBS += -leuser -lefsrv -lc32
}

unix:!symbian {
    PRIVATE_HEADERS += \
        $$PWD/ttylocker_unix_p.h \
        $$PWD/serialport_unix_p.h

    SOURCES += \
        $$PWD/ttylocker_unix.cpp \
        $$PWD/serialport_unix.cpp \
        $$PWD/serialportinfo_unix.cpp

    macx {
        SOURCES += $$PWD/serialportinfo_mac.cpp

        LIBS += -framework IOKit -framework CoreFoundation
    } else {
        linux*:contains( DEFINES, HAVE_LIBUDEV ) {
            LIBS += -ludev
        }
    }
}

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
