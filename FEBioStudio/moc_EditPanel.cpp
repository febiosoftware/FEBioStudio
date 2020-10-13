/****************************************************************************
** Meta object code from reading C++ file 'EditPanel.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "EditPanel.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'EditPanel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CEditPanel_t {
    QByteArrayData data[9];
    char stringdata0[89];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CEditPanel_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CEditPanel_t qt_meta_stringdata_CEditPanel = {
    {
QT_MOC_LITERAL(0, 0, 10), // "CEditPanel"
QT_MOC_LITERAL(1, 11, 16), // "on_apply_clicked"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 1), // "b"
QT_MOC_LITERAL(4, 31, 17), // "on_menu_triggered"
QT_MOC_LITERAL(5, 49, 8), // "QAction*"
QT_MOC_LITERAL(6, 58, 2), // "pa"
QT_MOC_LITERAL(7, 61, 25), // "on_buttons_buttonSelected"
QT_MOC_LITERAL(8, 87, 1) // "n"

    },
    "CEditPanel\0on_apply_clicked\0\0b\0"
    "on_menu_triggered\0QAction*\0pa\0"
    "on_buttons_buttonSelected\0n"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CEditPanel[] = {

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
       1,    1,   29,    2, 0x08 /* Private */,
       4,    1,   32,    2, 0x08 /* Private */,
       7,    1,   35,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool,    3,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, QMetaType::Int,    8,

       0        // eod
};

void CEditPanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CEditPanel *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_apply_clicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->on_menu_triggered((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        case 2: _t->on_buttons_buttonSelected((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CEditPanel::staticMetaObject = { {
    QMetaObject::SuperData::link<CCommandPanel::staticMetaObject>(),
    qt_meta_stringdata_CEditPanel.data,
    qt_meta_data_CEditPanel,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CEditPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CEditPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CEditPanel.stringdata0))
        return static_cast<void*>(this);
    return CCommandPanel::qt_metacast(_clname);
}

int CEditPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CCommandPanel::qt_metacall(_c, _id, _a);
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
