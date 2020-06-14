/****************************************************************************
** Meta object code from reading C++ file 'MaterialPanel.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "MaterialPanel.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MaterialPanel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CMaterialPanel_t {
    QByteArrayData data[10];
    char stringdata0[161];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CMaterialPanel_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CMaterialPanel_t qt_meta_stringdata_CMaterialPanel = {
    {
QT_MOC_LITERAL(0, 0, 14), // "CMaterialPanel"
QT_MOC_LITERAL(1, 15, 33), // "on_materialList_currentRowCha..."
QT_MOC_LITERAL(2, 49, 0), // ""
QT_MOC_LITERAL(3, 50, 4), // "nrow"
QT_MOC_LITERAL(4, 55, 21), // "on_showButton_toggled"
QT_MOC_LITERAL(5, 77, 1), // "b"
QT_MOC_LITERAL(6, 79, 23), // "on_enableButton_toggled"
QT_MOC_LITERAL(7, 103, 27), // "on_editName_editingFinished"
QT_MOC_LITERAL(8, 131, 23), // "on_matprops_dataChanged"
QT_MOC_LITERAL(9, 155, 5) // "nprop"

    },
    "CMaterialPanel\0on_materialList_currentRowChanged\0"
    "\0nrow\0on_showButton_toggled\0b\0"
    "on_enableButton_toggled\0"
    "on_editName_editingFinished\0"
    "on_matprops_dataChanged\0nprop"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CMaterialPanel[] = {

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
       1,    1,   39,    2, 0x08 /* Private */,
       4,    1,   42,    2, 0x08 /* Private */,
       6,    1,   45,    2, 0x08 /* Private */,
       7,    0,   48,    2, 0x08 /* Private */,
       8,    1,   49,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    9,

       0        // eod
};

void CMaterialPanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CMaterialPanel *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_materialList_currentRowChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->on_showButton_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->on_enableButton_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->on_editName_editingFinished(); break;
        case 4: _t->on_matprops_dataChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CMaterialPanel::staticMetaObject = { {
    QMetaObject::SuperData::link<CCommandPanel::staticMetaObject>(),
    qt_meta_stringdata_CMaterialPanel.data,
    qt_meta_data_CMaterialPanel,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CMaterialPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CMaterialPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CMaterialPanel.stringdata0))
        return static_cast<void*>(this);
    return CCommandPanel::qt_metacast(_clname);
}

int CMaterialPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CCommandPanel::qt_metacall(_c, _id, _a);
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
QT_WARNING_POP
QT_END_MOC_NAMESPACE
