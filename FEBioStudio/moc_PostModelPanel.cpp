/****************************************************************************
** Meta object code from reading C++ file 'PostModelPanel.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "PostModelPanel.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PostModelPanel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CPostModelPanel_t {
    QByteArrayData data[14];
    char stringdata0[226];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CPostModelPanel_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CPostModelPanel_t qt_meta_stringdata_CPostModelPanel = {
    {
QT_MOC_LITERAL(0, 0, 15), // "CPostModelPanel"
QT_MOC_LITERAL(1, 16, 31), // "on_modelTree_currentItemChanged"
QT_MOC_LITERAL(2, 48, 0), // ""
QT_MOC_LITERAL(3, 49, 16), // "QTreeWidgetItem*"
QT_MOC_LITERAL(4, 66, 7), // "current"
QT_MOC_LITERAL(5, 74, 4), // "prev"
QT_MOC_LITERAL(6, 79, 30), // "on_modelTree_itemDoubleClicked"
QT_MOC_LITERAL(7, 110, 4), // "item"
QT_MOC_LITERAL(8, 115, 6), // "column"
QT_MOC_LITERAL(9, 122, 27), // "on_nameEdit_editingFinished"
QT_MOC_LITERAL(10, 150, 23), // "on_deleteButton_clicked"
QT_MOC_LITERAL(11, 174, 20), // "on_props_dataChanged"
QT_MOC_LITERAL(12, 195, 23), // "on_enabled_stateChanged"
QT_MOC_LITERAL(13, 219, 6) // "nstate"

    },
    "CPostModelPanel\0on_modelTree_currentItemChanged\0"
    "\0QTreeWidgetItem*\0current\0prev\0"
    "on_modelTree_itemDoubleClicked\0item\0"
    "column\0on_nameEdit_editingFinished\0"
    "on_deleteButton_clicked\0on_props_dataChanged\0"
    "on_enabled_stateChanged\0nstate"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CPostModelPanel[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,   44,    2, 0x08 /* Private */,
       6,    2,   49,    2, 0x08 /* Private */,
       9,    0,   54,    2, 0x08 /* Private */,
      10,    0,   55,    2, 0x08 /* Private */,
      11,    0,   56,    2, 0x08 /* Private */,
      12,    1,   57,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    7,    8,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   13,

       0        // eod
};

void CPostModelPanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CPostModelPanel *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_modelTree_currentItemChanged((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< QTreeWidgetItem*(*)>(_a[2]))); break;
        case 1: _t->on_modelTree_itemDoubleClicked((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->on_nameEdit_editingFinished(); break;
        case 3: _t->on_deleteButton_clicked(); break;
        case 4: _t->on_props_dataChanged(); break;
        case 5: _t->on_enabled_stateChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CPostModelPanel::staticMetaObject = { {
    &CCommandPanel::staticMetaObject,
    qt_meta_stringdata_CPostModelPanel.data,
    qt_meta_data_CPostModelPanel,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CPostModelPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CPostModelPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CPostModelPanel.stringdata0))
        return static_cast<void*>(this);
    return CCommandPanel::qt_metacast(_clname);
}

int CPostModelPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CCommandPanel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
