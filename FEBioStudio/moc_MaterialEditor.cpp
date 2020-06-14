/****************************************************************************
** Meta object code from reading C++ file 'MaterialEditor.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "MaterialEditor.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MaterialEditor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CMaterialEditor_t {
    QByteArrayData data[10];
    char stringdata0[135];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CMaterialEditor_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CMaterialEditor_t qt_meta_stringdata_CMaterialEditor = {
    {
QT_MOC_LITERAL(0, 0, 15), // "CMaterialEditor"
QT_MOC_LITERAL(1, 16, 26), // "on_tree_currentItemChanged"
QT_MOC_LITERAL(2, 43, 0), // ""
QT_MOC_LITERAL(3, 44, 16), // "QTreeWidgetItem*"
QT_MOC_LITERAL(4, 61, 7), // "current"
QT_MOC_LITERAL(5, 69, 8), // "previous"
QT_MOC_LITERAL(6, 78, 31), // "on_matClass_currentIndexChanged"
QT_MOC_LITERAL(7, 110, 1), // "n"
QT_MOC_LITERAL(8, 112, 15), // "materialChanged"
QT_MOC_LITERAL(9, 128, 6) // "accept"

    },
    "CMaterialEditor\0on_tree_currentItemChanged\0"
    "\0QTreeWidgetItem*\0current\0previous\0"
    "on_matClass_currentIndexChanged\0n\0"
    "materialChanged\0accept"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CMaterialEditor[] = {

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
       1,    2,   34,    2, 0x08 /* Private */,
       6,    1,   39,    2, 0x08 /* Private */,
       8,    1,   42,    2, 0x08 /* Private */,
       9,    0,   45,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3,    4,    5,
    QMetaType::Void, QMetaType::Int,    7,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,

       0        // eod
};

void CMaterialEditor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CMaterialEditor *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_tree_currentItemChanged((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< QTreeWidgetItem*(*)>(_a[2]))); break;
        case 1: _t->on_matClass_currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->materialChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->accept(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CMaterialEditor::staticMetaObject = { {
    QMetaObject::SuperData::link<CHelpDialog::staticMetaObject>(),
    qt_meta_stringdata_CMaterialEditor.data,
    qt_meta_data_CMaterialEditor,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CMaterialEditor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CMaterialEditor::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CMaterialEditor.stringdata0))
        return static_cast<void*>(this);
    return CHelpDialog::qt_metacast(_clname);
}

int CMaterialEditor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CHelpDialog::qt_metacall(_c, _id, _a);
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
