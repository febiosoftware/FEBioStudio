/****************************************************************************
** Meta object code from reading C++ file 'PlotMixTool.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "PlotMixTool.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PlotMixTool.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CPlotMixTool_t {
    QByteArrayData data[7];
    char stringdata0[60];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CPlotMixTool_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CPlotMixTool_t qt_meta_stringdata_CPlotMixTool = {
    {
QT_MOC_LITERAL(0, 0, 12), // "CPlotMixTool"
QT_MOC_LITERAL(1, 13, 8), // "OnBrowse"
QT_MOC_LITERAL(2, 22, 0), // ""
QT_MOC_LITERAL(3, 23, 8), // "OnRemove"
QT_MOC_LITERAL(4, 32, 8), // "OnMoveUp"
QT_MOC_LITERAL(5, 41, 10), // "OnMoveDown"
QT_MOC_LITERAL(6, 52, 7) // "OnApply"

    },
    "CPlotMixTool\0OnBrowse\0\0OnRemove\0"
    "OnMoveUp\0OnMoveDown\0OnApply"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CPlotMixTool[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x08 /* Private */,
       3,    0,   40,    2, 0x08 /* Private */,
       4,    0,   41,    2, 0x08 /* Private */,
       5,    0,   42,    2, 0x08 /* Private */,
       6,    0,   43,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CPlotMixTool::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CPlotMixTool *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->OnBrowse(); break;
        case 1: _t->OnRemove(); break;
        case 2: _t->OnMoveUp(); break;
        case 3: _t->OnMoveDown(); break;
        case 4: _t->OnApply(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject CPlotMixTool::staticMetaObject = { {
    QMetaObject::SuperData::link<CAbstractTool::staticMetaObject>(),
    qt_meta_stringdata_CPlotMixTool.data,
    qt_meta_data_CPlotMixTool,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CPlotMixTool::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CPlotMixTool::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CPlotMixTool.stringdata0))
        return static_cast<void*>(this);
    return CAbstractTool::qt_metacast(_clname);
}

int CPlotMixTool::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CAbstractTool::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}
struct qt_meta_stringdata_CKinematTool_t {
    QByteArrayData data[5];
    char stringdata0[42];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CKinematTool_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CKinematTool_t qt_meta_stringdata_CKinematTool = {
    {
QT_MOC_LITERAL(0, 0, 12), // "CKinematTool"
QT_MOC_LITERAL(1, 13, 9), // "OnBrowse1"
QT_MOC_LITERAL(2, 23, 0), // ""
QT_MOC_LITERAL(3, 24, 9), // "OnBrowse2"
QT_MOC_LITERAL(4, 34, 7) // "OnApply"

    },
    "CKinematTool\0OnBrowse1\0\0OnBrowse2\0"
    "OnApply"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CKinematTool[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   29,    2, 0x08 /* Private */,
       3,    0,   30,    2, 0x08 /* Private */,
       4,    0,   31,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CKinematTool::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CKinematTool *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->OnBrowse1(); break;
        case 1: _t->OnBrowse2(); break;
        case 2: _t->OnApply(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject CKinematTool::staticMetaObject = { {
    QMetaObject::SuperData::link<CAbstractTool::staticMetaObject>(),
    qt_meta_stringdata_CKinematTool.data,
    qt_meta_data_CKinematTool,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CKinematTool::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CKinematTool::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CKinematTool.stringdata0))
        return static_cast<void*>(this);
    return CAbstractTool::qt_metacast(_clname);
}

int CKinematTool::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CAbstractTool::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
