/****************************************************************************
** Meta object code from reading C++ file 'serialport.h'
**
** Created: Tue Jan 8 11:12:03 2013
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "serialport/serialport.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'serialport.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SerialPort[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       9,   54, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      11,   24,   28,   33, 0x0a,
      34,   24,   28,   33, 0x0a,
      47,   62,   28,   33, 0x0a,
      71,   33,   28,   33, 0x2a,
      83,   24,   28,   33, 0x0a,
      98,   33,   28,   33, 0x2a,
     109,  126,   28,   33, 0x0a,
     132,   33,   28,   33, 0x2a,

 // enums: name, flags, count, data
     145, 0x0,    3,   90,
     156, 0x0,    9,   96,
     161, 0x0,    5,  114,
     170, 0x0,    6,  124,
     177, 0x0,    4,  136,
     186, 0x0,    4,  144,
     198, 0x0,    9,  152,
     204, 0x0,    5,  170,
     220, 0x0,   11,  180,

 // enum data: key, value
     230, uint(SerialPort::Input),
     236, uint(SerialPort::Output),
     243, uint(SerialPort::AllDirections),
     257, uint(SerialPort::Rate1200),
     266, uint(SerialPort::Rate2400),
     275, uint(SerialPort::Rate4800),
     284, uint(SerialPort::Rate9600),
     293, uint(SerialPort::Rate19200),
     303, uint(SerialPort::Rate38400),
     313, uint(SerialPort::Rate57600),
     323, uint(SerialPort::Rate115200),
     334, uint(SerialPort::UnknownRate),
     346, uint(SerialPort::Data5),
     352, uint(SerialPort::Data6),
     358, uint(SerialPort::Data7),
     364, uint(SerialPort::Data8),
     370, uint(SerialPort::UnknownDataBits),
     386, uint(SerialPort::NoParity),
     395, uint(SerialPort::EvenParity),
     406, uint(SerialPort::OddParity),
     416, uint(SerialPort::SpaceParity),
     428, uint(SerialPort::MarkParity),
     439, uint(SerialPort::UnknownParity),
     453, uint(SerialPort::OneStop),
     461, uint(SerialPort::OneAndHalfStop),
     476, uint(SerialPort::TwoStop),
     484, uint(SerialPort::UnknownStopBits),
     500, uint(SerialPort::NoFlowControl),
     514, uint(SerialPort::HardwareControl),
     530, uint(SerialPort::SoftwareControl),
     546, uint(SerialPort::UnknownFlowControl),
     565, uint(SerialPort::Le),
     568, uint(SerialPort::Dtr),
     572, uint(SerialPort::Rts),
     576, uint(SerialPort::St),
     579, uint(SerialPort::Sr),
     582, uint(SerialPort::Cts),
     586, uint(SerialPort::Dcd),
     590, uint(SerialPort::Ri),
     593, uint(SerialPort::Dsr),
     597, uint(SerialPort::SkipPolicy),
     608, uint(SerialPort::PassZeroPolicy),
     623, uint(SerialPort::IgnorePolicy),
     636, uint(SerialPort::StopReceivingPolicy),
     656, uint(SerialPort::UnknownPolicy),
     670, uint(SerialPort::NoError),
     678, uint(SerialPort::NoSuchDeviceError),
     696, uint(SerialPort::PermissionDeniedError),
     718, uint(SerialPort::DeviceAlreadyOpenedError),
     743, uint(SerialPort::DeviceIsNotOpenedError),
     766, uint(SerialPort::ParityError),
     778, uint(SerialPort::FramingError),
     791, uint(SerialPort::BreakConditionError),
     811, uint(SerialPort::IoError),
     819, uint(SerialPort::UnsupportedPortOperationError),
     849, uint(SerialPort::UnknownPortError),

       0        // eod
};

static const char qt_meta_stringdata_SerialPort[] = {
    "SerialPort\0setDtr(bool)\0set\0bool\0\0"
    "setRts(bool)\0sendBreak(int)\0duration\0"
    "sendBreak()\0setBreak(bool)\0setBreak()\0"
    "clearBreak(bool)\0clear\0clearBreak()\0"
    "Directions\0Rate\0DataBits\0Parity\0"
    "StopBits\0FlowControl\0Lines\0DataErrorPolicy\0"
    "PortError\0Input\0Output\0AllDirections\0"
    "Rate1200\0Rate2400\0Rate4800\0Rate9600\0"
    "Rate19200\0Rate38400\0Rate57600\0Rate115200\0"
    "UnknownRate\0Data5\0Data6\0Data7\0Data8\0"
    "UnknownDataBits\0NoParity\0EvenParity\0"
    "OddParity\0SpaceParity\0MarkParity\0"
    "UnknownParity\0OneStop\0OneAndHalfStop\0"
    "TwoStop\0UnknownStopBits\0NoFlowControl\0"
    "HardwareControl\0SoftwareControl\0"
    "UnknownFlowControl\0Le\0Dtr\0Rts\0St\0Sr\0"
    "Cts\0Dcd\0Ri\0Dsr\0SkipPolicy\0PassZeroPolicy\0"
    "IgnorePolicy\0StopReceivingPolicy\0"
    "UnknownPolicy\0NoError\0NoSuchDeviceError\0"
    "PermissionDeniedError\0DeviceAlreadyOpenedError\0"
    "DeviceIsNotOpenedError\0ParityError\0"
    "FramingError\0BreakConditionError\0"
    "IoError\0UnsupportedPortOperationError\0"
    "UnknownPortError\0"
};

void SerialPort::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        SerialPort *_t = static_cast<SerialPort *>(_o);
        switch (_id) {
        case 0: { bool _r = _t->setDtr((*reinterpret_cast< bool(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 1: { bool _r = _t->setRts((*reinterpret_cast< bool(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 2: { bool _r = _t->sendBreak((*reinterpret_cast< int(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 3: { bool _r = _t->sendBreak();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 4: { bool _r = _t->setBreak((*reinterpret_cast< bool(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 5: { bool _r = _t->setBreak();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 6: { bool _r = _t->clearBreak((*reinterpret_cast< bool(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 7: { bool _r = _t->clearBreak();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        default: ;
        }
    }
}

const QMetaObjectExtraData SerialPort::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject SerialPort::staticMetaObject = {
    { &QIODevice::staticMetaObject, qt_meta_stringdata_SerialPort,
      qt_meta_data_SerialPort, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SerialPort::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SerialPort::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SerialPort::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SerialPort))
        return static_cast<void*>(const_cast< SerialPort*>(this));
    return QIODevice::qt_metacast(_clname);
}

int SerialPort::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QIODevice::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
