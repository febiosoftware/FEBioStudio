/****************************************************************************
** Meta object code from reading C++ file 'DatabasePanel.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.13.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "DatabasePanel.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DatabasePanel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.13.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CDatabasePanel_t {
    QByteArrayData data[15];
    char stringdata0[320];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CDatabasePanel_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CDatabasePanel_t qt_meta_stringdata_CDatabasePanel = {
    {
QT_MOC_LITERAL(0, 0, 14), // "CDatabasePanel"
QT_MOC_LITERAL(1, 15, 22), // "on_loginButton_clicked"
QT_MOC_LITERAL(2, 38, 0), // ""
QT_MOC_LITERAL(3, 39, 31), // "on_treeWidget_itemDoubleClicked"
QT_MOC_LITERAL(4, 71, 16), // "QTreeWidgetItem*"
QT_MOC_LITERAL(5, 88, 4), // "item"
QT_MOC_LITERAL(6, 93, 6), // "column"
QT_MOC_LITERAL(7, 100, 27), // "on_actionDownload_triggered"
QT_MOC_LITERAL(8, 128, 23), // "on_actionOpen_triggered"
QT_MOC_LITERAL(9, 152, 35), // "on_actionOpenFileLocation_tri..."
QT_MOC_LITERAL(10, 188, 25), // "on_actionDelete_triggered"
QT_MOC_LITERAL(11, 214, 25), // "on_actionUpload_triggered"
QT_MOC_LITERAL(12, 240, 34), // "on_treeWidget_itemSelectionCh..."
QT_MOC_LITERAL(13, 275, 40), // "on_treeWidget_customContextMe..."
QT_MOC_LITERAL(14, 316, 3) // "pos"

    },
    "CDatabasePanel\0on_loginButton_clicked\0"
    "\0on_treeWidget_itemDoubleClicked\0"
    "QTreeWidgetItem*\0item\0column\0"
    "on_actionDownload_triggered\0"
    "on_actionOpen_triggered\0"
    "on_actionOpenFileLocation_triggered\0"
    "on_actionDelete_triggered\0"
    "on_actionUpload_triggered\0"
    "on_treeWidget_itemSelectionChanged\0"
    "on_treeWidget_customContextMenuRequested\0"
    "pos"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CDatabasePanel[] = {

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
       1,    0,   59,    2, 0x08 /* Private */,
       3,    2,   60,    2, 0x08 /* Private */,
       7,    0,   65,    2, 0x08 /* Private */,
       8,    0,   66,    2, 0x08 /* Private */,
       9,    0,   67,    2, 0x08 /* Private */,
      10,    0,   68,    2, 0x08 /* Private */,
      11,    0,   69,    2, 0x08 /* Private */,
      12,    0,   70,    2, 0x08 /* Private */,
      13,    1,   71,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4, QMetaType::Int,    5,    6,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QPoint,   14,

       0        // eod
};

void CDatabasePanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CDatabasePanel *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_loginButton_clicked(); break;
        case 1: _t->on_treeWidget_itemDoubleClicked((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->on_actionDownload_triggered(); break;
        case 3: _t->on_actionOpen_triggered(); break;
        case 4: _t->on_actionOpenFileLocation_triggered(); break;
        case 5: _t->on_actionDelete_triggered(); break;
        case 6: _t->on_actionUpload_triggered(); break;
        case 7: _t->on_treeWidget_itemSelectionChanged(); break;
        case 8: _t->on_treeWidget_customContextMenuRequested((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CDatabasePanel::staticMetaObject = { {
    &QWidget::staticMetaObject,
    qt_meta_stringdata_CDatabasePanel.data,
    qt_meta_data_CDatabasePanel,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CDatabasePanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CDatabasePanel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CDatabasePanel.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int CDatabasePanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
