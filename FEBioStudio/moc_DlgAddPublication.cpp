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
    QByteArrayData data[12];
    char stringdata0[178];
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
QT_MOC_LITERAL(4, 68, 17), // "publicationChosen"
QT_MOC_LITERAL(5, 86, 19), // "CPublicationWidget*"
QT_MOC_LITERAL(6, 106, 3), // "pub"
QT_MOC_LITERAL(7, 110, 19), // "manualButtonClicked"
QT_MOC_LITERAL(8, 130, 17), // "backButtonClicked"
QT_MOC_LITERAL(9, 148, 12), // "connFinished"
QT_MOC_LITERAL(10, 161, 14), // "QNetworkReply*"
QT_MOC_LITERAL(11, 176, 1) // "r"

    },
    "CDlgAddPublication\0on_DOILookup_triggered\0"
    "\0on_queryLookup_triggered\0publicationChosen\0"
    "CPublicationWidget*\0pub\0manualButtonClicked\0"
    "backButtonClicked\0connFinished\0"
    "QNetworkReply*\0r"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CDlgAddPublication[] = {

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
       1,    0,   44,    2, 0x0a /* Public */,
       3,    0,   45,    2, 0x0a /* Public */,
       4,    1,   46,    2, 0x0a /* Public */,
       7,    0,   49,    2, 0x0a /* Public */,
       8,    0,   50,    2, 0x0a /* Public */,
       9,    1,   51,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 10,   11,

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
        case 2: _t->publicationChosen((*reinterpret_cast< CPublicationWidget*(*)>(_a[1]))); break;
        case 3: _t->manualButtonClicked(); break;
        case 4: _t->backButtonClicked(); break;
        case 5: _t->connFinished((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
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
