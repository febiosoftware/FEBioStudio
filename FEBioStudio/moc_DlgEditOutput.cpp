/****************************************************************************
** Meta object code from reading C++ file 'DlgEditOutput.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "DlgEditOutput.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DlgEditOutput.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CDlgAddDomain_t {
    QByteArrayData data[1];
    char stringdata0[14];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CDlgAddDomain_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CDlgAddDomain_t qt_meta_stringdata_CDlgAddDomain = {
    {
QT_MOC_LITERAL(0, 0, 13) // "CDlgAddDomain"

    },
    "CDlgAddDomain"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CDlgAddDomain[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

void CDlgAddDomain::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject CDlgAddDomain::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_CDlgAddDomain.data,
    qt_meta_data_CDlgAddDomain,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CDlgAddDomain::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CDlgAddDomain::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CDlgAddDomain.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int CDlgAddDomain::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    return _id;
}
struct qt_meta_stringdata_CDlgEditOutput_t {
    QByteArrayData data[15];
    char stringdata0[168];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CDlgEditOutput_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CDlgEditOutput_t qt_meta_stringdata_CDlgEditOutput = {
    {
QT_MOC_LITERAL(0, 0, 14), // "CDlgEditOutput"
QT_MOC_LITERAL(1, 15, 11), // "OnAddDomain"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 14), // "OnRemoveDomain"
QT_MOC_LITERAL(4, 43, 13), // "OnNewVariable"
QT_MOC_LITERAL(5, 57, 10), // "OnVariable"
QT_MOC_LITERAL(6, 68, 4), // "nrow"
QT_MOC_LITERAL(7, 73, 13), // "OnItemClicked"
QT_MOC_LITERAL(8, 87, 16), // "QListWidgetItem*"
QT_MOC_LITERAL(9, 104, 4), // "item"
QT_MOC_LITERAL(10, 109, 15), // "onFilterChanged"
QT_MOC_LITERAL(11, 125, 3), // "txt"
QT_MOC_LITERAL(12, 129, 8), // "onLogAdd"
QT_MOC_LITERAL(13, 138, 11), // "onLogRemove"
QT_MOC_LITERAL(14, 150, 17) // "UpdateLogItemList"

    },
    "CDlgEditOutput\0OnAddDomain\0\0OnRemoveDomain\0"
    "OnNewVariable\0OnVariable\0nrow\0"
    "OnItemClicked\0QListWidgetItem*\0item\0"
    "onFilterChanged\0txt\0onLogAdd\0onLogRemove\0"
    "UpdateLogItemList"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CDlgEditOutput[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   59,    2, 0x09 /* Protected */,
       3,    0,   60,    2, 0x09 /* Protected */,
       4,    0,   61,    2, 0x09 /* Protected */,
       5,    1,   62,    2, 0x09 /* Protected */,
       7,    1,   65,    2, 0x09 /* Protected */,
      10,    1,   68,    2, 0x09 /* Protected */,
      12,    0,   71,    2, 0x09 /* Protected */,
      13,    0,   72,    2, 0x09 /* Protected */,
      14,    0,   73,    2, 0x09 /* Protected */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, QMetaType::QString,   11,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CDlgEditOutput::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CDlgEditOutput *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->OnAddDomain(); break;
        case 1: _t->OnRemoveDomain(); break;
        case 2: _t->OnNewVariable(); break;
        case 3: _t->OnVariable((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->OnItemClicked((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 5: _t->onFilterChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->onLogAdd(); break;
        case 7: _t->onLogRemove(); break;
        case 8: _t->UpdateLogItemList(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CDlgEditOutput::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_CDlgEditOutput.data,
    qt_meta_data_CDlgEditOutput,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CDlgEditOutput::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CDlgEditOutput::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CDlgEditOutput.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int CDlgEditOutput::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
