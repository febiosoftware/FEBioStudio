/****************************************************************************
** Meta object code from reading C++ file 'GLControlBar.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "GLControlBar.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GLControlBar.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CGLControlBar_t {
    QByteArrayData data[21];
    char stringdata0[302];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CGLControlBar_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CGLControlBar_t qt_meta_stringdata_CGLControlBar = {
    {
QT_MOC_LITERAL(0, 0, 13), // "CGLControlBar"
QT_MOC_LITERAL(1, 14, 14), // "onPivotChanged"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 14), // "onPivotClicked"
QT_MOC_LITERAL(4, 45, 1), // "b"
QT_MOC_LITERAL(5, 47, 19), // "onSnapToGridClicked"
QT_MOC_LITERAL(6, 67, 19), // "onSnapToNodeClicked"
QT_MOC_LITERAL(7, 87, 22), // "onToggleVisibleClicked"
QT_MOC_LITERAL(8, 110, 19), // "onMeshButtonClicked"
QT_MOC_LITERAL(9, 130, 1), // "n"
QT_MOC_LITERAL(10, 132, 17), // "onSelectConnected"
QT_MOC_LITERAL(11, 150, 19), // "onSelectClosestPath"
QT_MOC_LITERAL(12, 170, 17), // "onMaxAngleChanged"
QT_MOC_LITERAL(13, 188, 1), // "v"
QT_MOC_LITERAL(14, 190, 18), // "onSelectBackfacing"
QT_MOC_LITERAL(15, 209, 16), // "onIgnoreInterior"
QT_MOC_LITERAL(16, 226, 15), // "onHideSelection"
QT_MOC_LITERAL(17, 242, 9), // "onShowAll"
QT_MOC_LITERAL(18, 252, 19), // "onZoomSelectClicked"
QT_MOC_LITERAL(19, 272, 16), // "onZoomAllClicked"
QT_MOC_LITERAL(20, 289, 12) // "onToggleMesh"

    },
    "CGLControlBar\0onPivotChanged\0\0"
    "onPivotClicked\0b\0onSnapToGridClicked\0"
    "onSnapToNodeClicked\0onToggleVisibleClicked\0"
    "onMeshButtonClicked\0n\0onSelectConnected\0"
    "onSelectClosestPath\0onMaxAngleChanged\0"
    "v\0onSelectBackfacing\0onIgnoreInterior\0"
    "onHideSelection\0onShowAll\0onZoomSelectClicked\0"
    "onZoomAllClicked\0onToggleMesh"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CGLControlBar[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   94,    2, 0x08 /* Private */,
       3,    1,   95,    2, 0x08 /* Private */,
       5,    1,   98,    2, 0x08 /* Private */,
       6,    1,  101,    2, 0x08 /* Private */,
       7,    1,  104,    2, 0x08 /* Private */,
       8,    1,  107,    2, 0x08 /* Private */,
      10,    1,  110,    2, 0x08 /* Private */,
      11,    1,  113,    2, 0x08 /* Private */,
      12,    1,  116,    2, 0x08 /* Private */,
      14,    1,  119,    2, 0x08 /* Private */,
      15,    1,  122,    2, 0x08 /* Private */,
      16,    1,  125,    2, 0x08 /* Private */,
      17,    1,  128,    2, 0x08 /* Private */,
      18,    1,  131,    2, 0x08 /* Private */,
      19,    1,  134,    2, 0x08 /* Private */,
      20,    1,  137,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void, QMetaType::Int,    9,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void, QMetaType::Double,   13,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void, QMetaType::Bool,    4,

       0        // eod
};

void CGLControlBar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CGLControlBar *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onPivotChanged(); break;
        case 1: _t->onPivotClicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->onSnapToGridClicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->onSnapToNodeClicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->onToggleVisibleClicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->onMeshButtonClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->onSelectConnected((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 7: _t->onSelectClosestPath((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: _t->onMaxAngleChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 9: _t->onSelectBackfacing((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 10: _t->onIgnoreInterior((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 11: _t->onHideSelection((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 12: _t->onShowAll((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 13: _t->onZoomSelectClicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 14: _t->onZoomAllClicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 15: _t->onToggleMesh((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CGLControlBar::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CGLControlBar.data,
    qt_meta_data_CGLControlBar,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CGLControlBar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CGLControlBar::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CGLControlBar.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int CGLControlBar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 16;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
