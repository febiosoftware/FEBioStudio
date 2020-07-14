/****************************************************************************
** Meta object code from reading C++ file 'ExportProjectWidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "ExportProjectWidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ExportProjectWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CExportProjectWidget_t {
    QByteArrayData data[15];
    char stringdata0[273];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CExportProjectWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CExportProjectWidget_t qt_meta_stringdata_CExportProjectWidget = {
    {
QT_MOC_LITERAL(0, 0, 20), // "CExportProjectWidget"
QT_MOC_LITERAL(1, 21, 22), // "on_addFolder_triggered"
QT_MOC_LITERAL(2, 44, 0), // ""
QT_MOC_LITERAL(3, 45, 22), // "on_delFolder_triggered"
QT_MOC_LITERAL(4, 68, 21), // "on_addFiles_triggered"
QT_MOC_LITERAL(5, 90, 30), // "on_fileTree_currentItemChanged"
QT_MOC_LITERAL(6, 121, 16), // "QTreeWidgetItem*"
QT_MOC_LITERAL(7, 138, 7), // "current"
QT_MOC_LITERAL(8, 146, 23), // "on_fileTree_itemChanged"
QT_MOC_LITERAL(9, 170, 4), // "item"
QT_MOC_LITERAL(10, 175, 6), // "column"
QT_MOC_LITERAL(11, 182, 29), // "on_fileTree_itemDoubleClicked"
QT_MOC_LITERAL(12, 212, 18), // "descriptionChanged"
QT_MOC_LITERAL(13, 231, 20), // "on_addTagBtn_clicked"
QT_MOC_LITERAL(14, 252, 20) // "on_delTagBtn_clicked"

    },
    "CExportProjectWidget\0on_addFolder_triggered\0"
    "\0on_delFolder_triggered\0on_addFiles_triggered\0"
    "on_fileTree_currentItemChanged\0"
    "QTreeWidgetItem*\0current\0"
    "on_fileTree_itemChanged\0item\0column\0"
    "on_fileTree_itemDoubleClicked\0"
    "descriptionChanged\0on_addTagBtn_clicked\0"
    "on_delTagBtn_clicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CExportProjectWidget[] = {

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
       1,    0,   59,    2, 0x0a /* Public */,
       3,    0,   60,    2, 0x0a /* Public */,
       4,    0,   61,    2, 0x0a /* Public */,
       5,    1,   62,    2, 0x0a /* Public */,
       8,    2,   65,    2, 0x0a /* Public */,
      11,    2,   70,    2, 0x0a /* Public */,
      12,    0,   75,    2, 0x0a /* Public */,
      13,    0,   76,    2, 0x0a /* Public */,
      14,    0,   77,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void, 0x80000000 | 6, QMetaType::Int,    9,   10,
    QMetaType::Void, 0x80000000 | 6, QMetaType::Int,    9,   10,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CExportProjectWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CExportProjectWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_addFolder_triggered(); break;
        case 1: _t->on_delFolder_triggered(); break;
        case 2: _t->on_addFiles_triggered(); break;
        case 3: _t->on_fileTree_currentItemChanged((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1]))); break;
        case 4: _t->on_fileTree_itemChanged((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: _t->on_fileTree_itemDoubleClicked((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 6: _t->descriptionChanged(); break;
        case 7: _t->on_addTagBtn_clicked(); break;
        case 8: _t->on_delTagBtn_clicked(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CExportProjectWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CExportProjectWidget.data,
    qt_meta_data_CExportProjectWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CExportProjectWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CExportProjectWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CExportProjectWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int CExportProjectWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
