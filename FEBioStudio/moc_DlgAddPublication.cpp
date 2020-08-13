/****************************************************************************
** Meta object code from reading C++ file 'DlgAddPublication.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "DlgAddPublication.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DlgAddPublication.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CDlgAddPublication_t {
    QByteArrayData data[18];
    char stringdata0[288];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CDlgAddPublication_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CDlgAddPublication_t qt_meta_stringdata_CDlgAddPublication = {
    {
QT_MOC_LITERAL(0, 0, 18), // "CDlgAddPublication"
QT_MOC_LITERAL(1, 19, 22), // "on_DOILookup_triggered"
QT_MOC_LITERAL(2, 42, 0), // ""
QT_MOC_LITERAL(3, 43, 24), // "on_queryLookup_triggered"
QT_MOC_LITERAL(4, 68, 22), // "on_addAuthor_triggered"
QT_MOC_LITERAL(5, 91, 25), // "on_removeAuthor_triggered"
QT_MOC_LITERAL(6, 117, 31), // "on_authorTree_itemDoubleClicked"
QT_MOC_LITERAL(7, 149, 16), // "QTreeWidgetItem*"
QT_MOC_LITERAL(8, 166, 4), // "item"
QT_MOC_LITERAL(9, 171, 6), // "column"
QT_MOC_LITERAL(10, 178, 17), // "publicationChosen"
QT_MOC_LITERAL(11, 196, 19), // "CPublicationWidget*"
QT_MOC_LITERAL(12, 216, 3), // "pub"
QT_MOC_LITERAL(13, 220, 19), // "manualButtonClicked"
QT_MOC_LITERAL(14, 240, 17), // "backButtonClicked"
QT_MOC_LITERAL(15, 258, 12), // "connFinished"
QT_MOC_LITERAL(16, 271, 14), // "QNetworkReply*"
QT_MOC_LITERAL(17, 286, 1) // "r"

    },
    "CDlgAddPublication\0on_DOILookup_triggered\0"
    "\0on_queryLookup_triggered\0"
    "on_addAuthor_triggered\0on_removeAuthor_triggered\0"
    "on_authorTree_itemDoubleClicked\0"
    "QTreeWidgetItem*\0item\0column\0"
    "publicationChosen\0CPublicationWidget*\0"
    "pub\0manualButtonClicked\0backButtonClicked\0"
    "connFinished\0QNetworkReply*\0r"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CDlgAddPublication[] = {

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
       5,    0,   62,    2, 0x0a /* Public */,
       6,    2,   63,    2, 0x0a /* Public */,
      10,    1,   68,    2, 0x0a /* Public */,
      13,    0,   71,    2, 0x0a /* Public */,
      14,    0,   72,    2, 0x0a /* Public */,
      15,    1,   73,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 7, QMetaType::Int,    8,    9,
    QMetaType::Void, 0x80000000 | 11,   12,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 16,   17,

       0        // eod
};

void CDlgAddPublication::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CDlgAddPublication *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_DOILookup_triggered(); break;
        case 1: _t->on_queryLookup_triggered(); break;
        case 2: _t->on_addAuthor_triggered(); break;
        case 3: _t->on_removeAuthor_triggered(); break;
        case 4: _t->on_authorTree_itemDoubleClicked((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: _t->publicationChosen((*reinterpret_cast< CPublicationWidget*(*)>(_a[1]))); break;
        case 6: _t->manualButtonClicked(); break;
        case 7: _t->backButtonClicked(); break;
        case 8: _t->connFinished((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CDlgAddPublication::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_CDlgAddPublication.data,
    qt_meta_data_CDlgAddPublication,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CDlgAddPublication::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CDlgAddPublication::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CDlgAddPublication.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int CDlgAddPublication::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
