/****************************************************************************
** Meta object code from reading C++ file 'FileViewer.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "FileViewer.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FileViewer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CFileViewer_t {
    QByteArrayData data[13];
    char stringdata0[167];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CFileViewer_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CFileViewer_t qt_meta_stringdata_CFileViewer = {
    {
QT_MOC_LITERAL(0, 0, 11), // "CFileViewer"
QT_MOC_LITERAL(1, 12, 29), // "on_fileList_itemDoubleClicked"
QT_MOC_LITERAL(2, 42, 0), // ""
QT_MOC_LITERAL(3, 43, 16), // "QTreeWidgetItem*"
QT_MOC_LITERAL(4, 60, 4), // "item"
QT_MOC_LITERAL(5, 65, 6), // "column"
QT_MOC_LITERAL(6, 72, 13), // "onCreateGroup"
QT_MOC_LITERAL(7, 86, 13), // "onMoveToGroup"
QT_MOC_LITERAL(8, 100, 1), // "i"
QT_MOC_LITERAL(9, 102, 13), // "onRemoveGroup"
QT_MOC_LITERAL(10, 116, 13), // "onRenameGroup"
QT_MOC_LITERAL(11, 130, 16), // "onShowInExplorer"
QT_MOC_LITERAL(12, 147, 19) // "onRemoveFromProject"

    },
    "CFileViewer\0on_fileList_itemDoubleClicked\0"
    "\0QTreeWidgetItem*\0item\0column\0"
    "onCreateGroup\0onMoveToGroup\0i\0"
    "onRemoveGroup\0onRenameGroup\0"
    "onShowInExplorer\0onRemoveFromProject"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CFileViewer[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,   49,    2, 0x08 /* Private */,
       6,    0,   54,    2, 0x08 /* Private */,
       7,    1,   55,    2, 0x08 /* Private */,
       9,    0,   58,    2, 0x08 /* Private */,
      10,    0,   59,    2, 0x08 /* Private */,
      11,    0,   60,    2, 0x08 /* Private */,
      12,    0,   61,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,    5,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    8,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CFileViewer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CFileViewer *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_fileList_itemDoubleClicked((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->onCreateGroup(); break;
        case 2: _t->onMoveToGroup((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->onRemoveGroup(); break;
        case 4: _t->onRenameGroup(); break;
        case 5: _t->onShowInExplorer(); break;
        case 6: _t->onRemoveFromProject(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CFileViewer::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CFileViewer.data,
    qt_meta_data_CFileViewer,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CFileViewer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CFileViewer::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CFileViewer.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int CFileViewer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
