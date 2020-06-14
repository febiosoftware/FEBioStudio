/****************************************************************************
** Meta object code from reading C++ file 'DlgAddChemicalReaction.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "DlgAddChemicalReaction.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DlgAddChemicalReaction.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_QSelectBox_t {
    QByteArrayData data[4];
    char stringdata0[52];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QSelectBox_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QSelectBox_t qt_meta_stringdata_QSelectBox = {
    {
QT_MOC_LITERAL(0, 0, 10), // "QSelectBox"
QT_MOC_LITERAL(1, 11, 19), // "on_toTarget_clicked"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 19) // "on_toSource_clicked"

    },
    "QSelectBox\0on_toTarget_clicked\0\0"
    "on_toSource_clicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QSelectBox[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   24,    2, 0x08 /* Private */,
       3,    0,   25,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void QSelectBox::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<QSelectBox *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_toTarget_clicked(); break;
        case 1: _t->on_toSource_clicked(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject QSelectBox::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_QSelectBox.data,
    qt_meta_data_QSelectBox,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *QSelectBox::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QSelectBox::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_QSelectBox.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int QSelectBox::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}
struct qt_meta_stringdata_CReactionList_t {
    QByteArrayData data[9];
    char stringdata0[89];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CReactionList_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CReactionList_t qt_meta_stringdata_CReactionList = {
    {
QT_MOC_LITERAL(0, 0, 13), // "CReactionList"
QT_MOC_LITERAL(1, 14, 19), // "currentIndexChanged"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 1), // "n"
QT_MOC_LITERAL(4, 37, 5), // "onAdd"
QT_MOC_LITERAL(5, 43, 8), // "onRemove"
QT_MOC_LITERAL(6, 52, 21), // "onCurrentIndexChanged"
QT_MOC_LITERAL(7, 74, 5), // "OnAdd"
QT_MOC_LITERAL(8, 80, 8) // "OnRemove"

    },
    "CReactionList\0currentIndexChanged\0\0n\0"
    "onAdd\0onRemove\0onCurrentIndexChanged\0"
    "OnAdd\0OnRemove"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CReactionList[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x06 /* Public */,
       4,    0,   47,    2, 0x06 /* Public */,
       5,    0,   48,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    1,   49,    2, 0x08 /* Private */,
       7,    0,   52,    2, 0x08 /* Private */,
       8,    0,   53,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CReactionList::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CReactionList *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->onAdd(); break;
        case 2: _t->onRemove(); break;
        case 3: _t->onCurrentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->OnAdd(); break;
        case 5: _t->OnRemove(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CReactionList::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CReactionList::currentIndexChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CReactionList::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CReactionList::onAdd)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (CReactionList::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CReactionList::onRemove)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CReactionList::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CReactionList.data,
    qt_meta_data_CReactionList,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CReactionList::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CReactionList::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CReactionList.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int CReactionList::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void CReactionList::currentIndexChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void CReactionList::onAdd()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void CReactionList::onRemove()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}
struct qt_meta_stringdata_CDlgAddChemicalReaction_t {
    QByteArrayData data[13];
    char stringdata0[159];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CDlgAddChemicalReaction_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CDlgAddChemicalReaction_t qt_meta_stringdata_CDlgAddChemicalReaction = {
    {
QT_MOC_LITERAL(0, 0, 23), // "CDlgAddChemicalReaction"
QT_MOC_LITERAL(1, 24, 17), // "onMaterialChanged"
QT_MOC_LITERAL(2, 42, 0), // ""
QT_MOC_LITERAL(3, 43, 1), // "n"
QT_MOC_LITERAL(4, 45, 17), // "onReactionChanged"
QT_MOC_LITERAL(5, 63, 14), // "onReactionType"
QT_MOC_LITERAL(6, 78, 13), // "onAddReaction"
QT_MOC_LITERAL(7, 92, 16), // "onRemoveReaction"
QT_MOC_LITERAL(8, 109, 13), // "onNameChanged"
QT_MOC_LITERAL(9, 123, 1), // "t"
QT_MOC_LITERAL(10, 125, 9), // "onClicked"
QT_MOC_LITERAL(11, 135, 16), // "QAbstractButton*"
QT_MOC_LITERAL(12, 152, 6) // "button"

    },
    "CDlgAddChemicalReaction\0onMaterialChanged\0"
    "\0n\0onReactionChanged\0onReactionType\0"
    "onAddReaction\0onRemoveReaction\0"
    "onNameChanged\0t\0onClicked\0QAbstractButton*\0"
    "button"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CDlgAddChemicalReaction[] = {

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
       1,    1,   49,    2, 0x09 /* Protected */,
       4,    1,   52,    2, 0x09 /* Protected */,
       5,    1,   55,    2, 0x09 /* Protected */,
       6,    0,   58,    2, 0x09 /* Protected */,
       7,    0,   59,    2, 0x09 /* Protected */,
       8,    1,   60,    2, 0x09 /* Protected */,
      10,    1,   63,    2, 0x09 /* Protected */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    9,
    QMetaType::Void, 0x80000000 | 11,   12,

       0        // eod
};

void CDlgAddChemicalReaction::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CDlgAddChemicalReaction *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onMaterialChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->onReactionChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->onReactionType((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->onAddReaction(); break;
        case 4: _t->onRemoveReaction(); break;
        case 5: _t->onNameChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->onClicked((*reinterpret_cast< QAbstractButton*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CDlgAddChemicalReaction::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_CDlgAddChemicalReaction.data,
    qt_meta_data_CDlgAddChemicalReaction,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CDlgAddChemicalReaction::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CDlgAddChemicalReaction::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CDlgAddChemicalReaction.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int CDlgAddChemicalReaction::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
