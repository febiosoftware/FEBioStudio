/****************************************************************************
** Meta object code from reading C++ file 'DlgEditLConfigs.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.13.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "DlgEditLConfigs.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DlgEditLConfigs.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.13.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CDlgEditPath_t {
    QByteArrayData data[10];
    char stringdata0[133];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CDlgEditPath_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CDlgEditPath_t qt_meta_stringdata_CDlgEditPath = {
    {
QT_MOC_LITERAL(0, 0, 12), // "CDlgEditPath"
QT_MOC_LITERAL(1, 13, 19), // "on_selection_change"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 16), // "QListWidgetItem*"
QT_MOC_LITERAL(4, 51, 7), // "current"
QT_MOC_LITERAL(5, 59, 8), // "previous"
QT_MOC_LITERAL(6, 68, 11), // "on_dblClick"
QT_MOC_LITERAL(7, 80, 4), // "item"
QT_MOC_LITERAL(8, 85, 23), // "on_addConfigBtn_Clicked"
QT_MOC_LITERAL(9, 109, 23) // "on_delConfigBtn_Clicked"

    },
    "CDlgEditPath\0on_selection_change\0\0"
    "QListWidgetItem*\0current\0previous\0"
    "on_dblClick\0item\0on_addConfigBtn_Clicked\0"
    "on_delConfigBtn_Clicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CDlgEditPath[] = {

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
       1,    2,   34,    2, 0x0a /* Public */,
       6,    1,   39,    2, 0x0a /* Public */,
       8,    0,   42,    2, 0x0a /* Public */,
       9,    0,   43,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3,    4,    5,
    QMetaType::Void, 0x80000000 | 3,    7,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CDlgEditPath::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CDlgEditPath *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_selection_change((*reinterpret_cast< QListWidgetItem*(*)>(_a[1])),(*reinterpret_cast< QListWidgetItem*(*)>(_a[2]))); break;
        case 1: _t->on_dblClick((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 2: _t->on_addConfigBtn_Clicked(); break;
        case 3: _t->on_delConfigBtn_Clicked(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CDlgEditPath::staticMetaObject = { {
    &QDialog::staticMetaObject,
    qt_meta_stringdata_CDlgEditPath.data,
    qt_meta_data_CDlgEditPath,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CDlgEditPath::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CDlgEditPath::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CDlgEditPath.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int CDlgEditPath::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
