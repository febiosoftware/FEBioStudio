/****************************************************************************
** Meta object code from reading C++ file 'CreateSpringPane.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "CreateSpringPane.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CreateSpringPane.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CCreateSpringPane_t {
    QByteArrayData data[13];
    char stringdata0[277];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CCreateSpringPane_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CCreateSpringPane_t qt_meta_stringdata_CCreateSpringPane = {
    {
QT_MOC_LITERAL(0, 0, 17), // "CCreateSpringPane"
QT_MOC_LITERAL(1, 18, 25), // "on_node1_addButtonClicked"
QT_MOC_LITERAL(2, 44, 0), // ""
QT_MOC_LITERAL(3, 45, 25), // "on_node1_subButtonClicked"
QT_MOC_LITERAL(4, 71, 25), // "on_node1_selButtonClicked"
QT_MOC_LITERAL(5, 97, 25), // "on_node1_delButtonClicked"
QT_MOC_LITERAL(6, 123, 25), // "on_node2_addButtonClicked"
QT_MOC_LITERAL(7, 149, 25), // "on_node2_subButtonClicked"
QT_MOC_LITERAL(8, 175, 25), // "on_node2_selButtonClicked"
QT_MOC_LITERAL(9, 201, 25), // "on_node2_delButtonClicked"
QT_MOC_LITERAL(10, 227, 17), // "on_newSet_clicked"
QT_MOC_LITERAL(11, 245, 29), // "on_method_currentIndexChanged"
QT_MOC_LITERAL(12, 275, 1) // "n"

    },
    "CCreateSpringPane\0on_node1_addButtonClicked\0"
    "\0on_node1_subButtonClicked\0"
    "on_node1_selButtonClicked\0"
    "on_node1_delButtonClicked\0"
    "on_node2_addButtonClicked\0"
    "on_node2_subButtonClicked\0"
    "on_node2_selButtonClicked\0"
    "on_node2_delButtonClicked\0on_newSet_clicked\0"
    "on_method_currentIndexChanged\0n"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CCreateSpringPane[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   64,    2, 0x09 /* Protected */,
       3,    0,   65,    2, 0x09 /* Protected */,
       4,    0,   66,    2, 0x09 /* Protected */,
       5,    0,   67,    2, 0x09 /* Protected */,
       6,    0,   68,    2, 0x09 /* Protected */,
       7,    0,   69,    2, 0x09 /* Protected */,
       8,    0,   70,    2, 0x09 /* Protected */,
       9,    0,   71,    2, 0x09 /* Protected */,
      10,    0,   72,    2, 0x09 /* Protected */,
      11,    1,   73,    2, 0x09 /* Protected */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   12,

       0        // eod
};

void CCreateSpringPane::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CCreateSpringPane *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_node1_addButtonClicked(); break;
        case 1: _t->on_node1_subButtonClicked(); break;
        case 2: _t->on_node1_selButtonClicked(); break;
        case 3: _t->on_node1_delButtonClicked(); break;
        case 4: _t->on_node2_addButtonClicked(); break;
        case 5: _t->on_node2_subButtonClicked(); break;
        case 6: _t->on_node2_selButtonClicked(); break;
        case 7: _t->on_node2_delButtonClicked(); break;
        case 8: _t->on_newSet_clicked(); break;
        case 9: _t->on_method_currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CCreateSpringPane::staticMetaObject = { {
    QMetaObject::SuperData::link<CCreatePane::staticMetaObject>(),
    qt_meta_stringdata_CCreateSpringPane.data,
    qt_meta_data_CCreateSpringPane,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CCreateSpringPane::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CCreateSpringPane::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CCreateSpringPane.stringdata0))
        return static_cast<void*>(this);
    return CCreatePane::qt_metacast(_clname);
}

int CCreateSpringPane::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CCreatePane::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
