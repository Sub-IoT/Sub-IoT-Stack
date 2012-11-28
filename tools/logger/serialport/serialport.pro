QT = core
DEFINES += QT_SERIALPORT_LIB
VERSION = 1.0.0

include($$PWD/serialport-lib.pri)

greaterThan(QT_MAJOR_VERSION, 4) {
    load(qt_build_config)
    QT += core-private
    TARGET = QtAddOnSerialPort
    load(qt_module)
} else {
    TEMPLATE = lib
    TARGET = $$qtLibraryTarget(SerialPort$$QT_LIBINFIX)
    include($$PWD/qt4support/install-helper.pri)
    CONFIG += module create_prl
    mac:QMAKE_FRAMEWORK_BUNDLE_NAME = $$TARGET
}
