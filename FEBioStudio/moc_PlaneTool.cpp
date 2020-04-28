/****************************************************************************
** Meta object code from reading C++ file 'PlaneTool.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "PlaneTool.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PlaneTool.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CPlaneTool_t {
    QByteArrayData data[6];
    char stringdata0[72];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CPlaneTool_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CPlaneTool_t qt_meta_stringdata_CPlaneTool = {
    {
QT_MOC_LITERAL(0, 0, 10), // "CPlaneTool"
QT_MOC_LITERAL(1, 11, 15), // "on_change_node1"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 15), // "on_change_node2"
QT_MOC_LITERAL(4, 44, 15), // "on_change_node3"
QT_MOC_LITERAL(5, 60, 11) // "onAlignView"

    },
    "CPlaneTool\0on_change_node1\0\0on_change_node2\0"
    "on_change_node3\0onAlignView"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CPlaneTool[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   34,    2, 0x08 /* Private */,
       3,    0,   35,    2, 0x08 /* Private */,
       4,    0,   36,    2, 0x08 /* Private */,
       5,    0,   37,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CPlaneTool::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CPlaneTool *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_change_node1(); break;
        case 1: _t->on_change_node2(); break;
        case 2: _t->on_change_node3(); break;
        case 3: _t->onAlignView(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject CPlaneTool::staticMetaObject = { {
    QMetaObject::SuperData::link<CAbstractTool::staticMetaObject>(),
    qt_meta_stringdata_CPlaneTool.data,
    qt_meta_data_CPlaneTool,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CPlaneTool::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CPlaneTool::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CPlaneTool.stringdata0))
        return static_cast<void*>(this);
    return CAbstractTool::qt_metacast(_clname);
}

int CPlaneTool::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CAbstractTool::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
