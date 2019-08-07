/****************************************************************************
** Meta object code from reading C++ file 'GLControlBar.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "GLControlBar.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GLControlBar.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CGLControlBar_t {
    QByteArrayData data[21];
    char stringdata0[312];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CGLControlBar_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CGLControlBar_t qt_meta_stringdata_CGLControlBar = {
    {
QT_MOC_LITERAL(0, 0, 13), // "CGLControlBar"
QT_MOC_LITERAL(1, 14, 18), // "currentViewChanged"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 1), // "n"
QT_MOC_LITERAL(4, 36, 14), // "onPivotChanged"
QT_MOC_LITERAL(5, 51, 14), // "onPivotClicked"
QT_MOC_LITERAL(6, 66, 1), // "b"
QT_MOC_LITERAL(7, 68, 19), // "onSnapToGridClicked"
QT_MOC_LITERAL(8, 88, 19), // "onSnapToNodeClicked"
QT_MOC_LITERAL(9, 108, 22), // "onToggleVisibleClicked"
QT_MOC_LITERAL(10, 131, 19), // "onMeshButtonClicked"
QT_MOC_LITERAL(11, 151, 17), // "onSelectConnected"
QT_MOC_LITERAL(12, 169, 19), // "onSelectClosestPath"
QT_MOC_LITERAL(13, 189, 17), // "onMaxAngleChanged"
QT_MOC_LITERAL(14, 207, 1), // "v"
QT_MOC_LITERAL(15, 209, 18), // "onSelectBackfacing"
QT_MOC_LITERAL(16, 228, 15), // "onHideSelection"
QT_MOC_LITERAL(17, 244, 9), // "onShowAll"
QT_MOC_LITERAL(18, 254, 19), // "onZoomSelectClicked"
QT_MOC_LITERAL(19, 274, 16), // "onZoomAllClicked"
QT_MOC_LITERAL(20, 291, 20) // "onCurrentViewChanged"

    },
    "CGLControlBar\0currentViewChanged\0\0n\0"
    "onPivotChanged\0onPivotClicked\0b\0"
    "onSnapToGridClicked\0onSnapToNodeClicked\0"
    "onToggleVisibleClicked\0onMeshButtonClicked\0"
    "onSelectConnected\0onSelectClosestPath\0"
    "onMaxAngleChanged\0v\0onSelectBackfacing\0"
    "onHideSelection\0onShowAll\0onZoomSelectClicked\0"
    "onZoomAllClicked\0onCurrentViewChanged"
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
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   94,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    0,   97,    2, 0x08 /* Private */,
       5,    1,   98,    2, 0x08 /* Private */,
       7,    1,  101,    2, 0x08 /* Private */,
       8,    1,  104,    2, 0x08 /* Private */,
       9,    1,  107,    2, 0x08 /* Private */,
      10,    1,  110,    2, 0x08 /* Private */,
      11,    1,  113,    2, 0x08 /* Private */,
      12,    1,  116,    2, 0x08 /* Private */,
      13,    1,  119,    2, 0x08 /* Private */,
      15,    1,  122,    2, 0x08 /* Private */,
      16,    1,  125,    2, 0x08 /* Private */,
      17,    1,  128,    2, 0x08 /* Private */,
      18,    1,  131,    2, 0x08 /* Private */,
      19,    1,  134,    2, 0x08 /* Private */,
      20,    1,  137,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void, QMetaType::Double,   14,
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void, QMetaType::Int,    3,

       0        // eod
};

void CGLControlBar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CGLControlBar *_t = static_cast<CGLControlBar *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->currentViewChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->onPivotChanged(); break;
        case 2: _t->onPivotClicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->onSnapToGridClicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->onSnapToNodeClicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->onToggleVisibleClicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 6: _t->onMeshButtonClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->onSelectConnected((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: _t->onSelectClosestPath((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: _t->onMaxAngleChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 10: _t->onSelectBackfacing((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 11: _t->onHideSelection((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 12: _t->onShowAll((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 13: _t->onZoomSelectClicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 14: _t->onZoomAllClicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 15: _t->onCurrentViewChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CGLControlBar::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CGLControlBar::currentViewChanged)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CGLControlBar::staticMetaObject = { {
    &QWidget::staticMetaObject,
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

// SIGNAL 0
void CGLControlBar::currentViewChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
