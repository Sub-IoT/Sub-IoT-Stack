/****************************************************************************
** Meta object code from reading C++ file 'logparser.h'
**
** Created: Tue Jan 8 11:42:33 2013
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "logparser.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'logparser.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_LogParser[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      10,   31,   38,   38, 0x05,
      39,   67,   38,   38, 0x05,

 // slots: signature, parameters, type, tag, flags
      78,   38,   38,   38, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_LogParser[] = {
    "LogParser\0packetParsed(Packet)\0packet\0"
    "\0logMessageReceived(QString)\0logMessage\0"
    "onDataAvailable()\0"
};

void LogParser::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        LogParser *_t = static_cast<LogParser *>(_o);
        switch (_id) {
        case 0: _t->packetParsed((*reinterpret_cast< Packet(*)>(_a[1]))); break;
        case 1: _t->logMessageReceived((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->onDataAvailable(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData LogParser::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject LogParser::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_LogParser,
      qt_meta_data_LogParser, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &LogParser::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *LogParser::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *LogParser::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_LogParser))
        return static_cast<void*>(const_cast< LogParser*>(this));
    return QObject::qt_metacast(_clname);
}

int LogParser::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void LogParser::packetParsed(Packet _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void LogParser::logMessageReceived(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
