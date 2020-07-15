/****************************************************************************
** Meta object code from reading C++ file 'WzdUpload.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "WzdUpload.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'WzdUpload.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CWzdUpload_t {
    QByteArrayData data[18];
    char stringdata0[338];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CWzdUpload_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CWzdUpload_t qt_meta_stringdata_CWzdUpload = {
    {
QT_MOC_LITERAL(0, 0, 10), // "CWzdUpload"
QT_MOC_LITERAL(1, 11, 20), // "on_addTagBtn_clicked"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 20), // "on_delTagBtn_clicked"
QT_MOC_LITERAL(4, 54, 22), // "on_addFolder_triggered"
QT_MOC_LITERAL(5, 77, 21), // "on_addFiles_triggered"
QT_MOC_LITERAL(6, 99, 19), // "on_rename_triggered"
QT_MOC_LITERAL(7, 119, 30), // "on_fileTree_currentItemChanged"
QT_MOC_LITERAL(8, 150, 16), // "QTreeWidgetItem*"
QT_MOC_LITERAL(9, 167, 7), // "current"
QT_MOC_LITERAL(10, 175, 23), // "on_fileTree_itemChanged"
QT_MOC_LITERAL(11, 199, 4), // "item"
QT_MOC_LITERAL(12, 204, 6), // "column"
QT_MOC_LITERAL(13, 211, 29), // "on_fileTree_itemDoubleClicked"
QT_MOC_LITERAL(14, 241, 23), // "on_fileTree_itemClicked"
QT_MOC_LITERAL(15, 265, 22), // "fileDescriptionChanged"
QT_MOC_LITERAL(16, 288, 24), // "on_addFileTagBtn_clicked"
QT_MOC_LITERAL(17, 313, 24) // "on_delFileTagBtn_clicked"

    },
    "CWzdUpload\0on_addTagBtn_clicked\0\0"
    "on_delTagBtn_clicked\0on_addFolder_triggered\0"
    "on_addFiles_triggered\0on_rename_triggered\0"
    "on_fileTree_currentItemChanged\0"
    "QTreeWidgetItem*\0current\0"
    "on_fileTree_itemChanged\0item\0column\0"
    "on_fileTree_itemDoubleClicked\0"
    "on_fileTree_itemClicked\0fileDescriptionChanged\0"
    "on_addFileTagBtn_clicked\0"
    "on_delFileTagBtn_clicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CWzdUpload[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   74,    2, 0x0a /* Public */,
       3,    0,   75,    2, 0x0a /* Public */,
       4,    0,   76,    2, 0x0a /* Public */,
       5,    0,   77,    2, 0x0a /* Public */,
       6,    0,   78,    2, 0x0a /* Public */,
       7,    1,   79,    2, 0x0a /* Public */,
      10,    2,   82,    2, 0x0a /* Public */,
      13,    2,   87,    2, 0x0a /* Public */,
      14,    2,   92,    2, 0x0a /* Public */,
      15,    0,   97,    2, 0x0a /* Public */,
      16,    0,   98,    2, 0x0a /* Public */,
      17,    0,   99,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, 0x80000000 | 8, QMetaType::Int,   11,   12,
    QMetaType::Void, 0x80000000 | 8, QMetaType::Int,   11,   12,
    QMetaType::Void, 0x80000000 | 8, QMetaType::Int,   11,   12,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CWzdUpload::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CWzdUpload *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_addTagBtn_clicked(); break;
        case 1: _t->on_delTagBtn_clicked(); break;
        case 2: _t->on_addFolder_triggered(); break;
        case 3: _t->on_addFiles_triggered(); break;
        case 4: _t->on_rename_triggered(); break;
        case 5: _t->on_fileTree_currentItemChanged((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1]))); break;
        case 6: _t->on_fileTree_itemChanged((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 7: _t->on_fileTree_itemDoubleClicked((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 8: _t->on_fileTree_itemClicked((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 9: _t->fileDescriptionChanged(); break;
        case 10: _t->on_addFileTagBtn_clicked(); break;
        case 11: _t->on_delFileTagBtn_clicked(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CWzdUpload::staticMetaObject = { {
    QMetaObject::SuperData::link<QWizard::staticMetaObject>(),
    qt_meta_stringdata_CWzdUpload.data,
    qt_meta_data_CWzdUpload,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CWzdUpload::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CWzdUpload::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CWzdUpload.stringdata0))
        return static_cast<void*>(this);
    return QWizard::qt_metacast(_clname);
}

int CWzdUpload::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWizard::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 12;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
