/****************************************************************************
** Meta object code from reading C++ file 'CurvePicker.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.13.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "CurvePicker.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CurvePicker.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.13.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CCurvePicker_t {
    QByteArrayData data[9];
    char stringdata0[85];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CCurvePicker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CCurvePicker_t qt_meta_stringdata_CCurvePicker = {
    {
QT_MOC_LITERAL(0, 0, 12), // "CCurvePicker"
QT_MOC_LITERAL(1, 13, 12), // "curveChanged"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 11), // "nameChanged"
QT_MOC_LITERAL(4, 39, 13), // "buttonToggled"
QT_MOC_LITERAL(5, 53, 8), // "bchecked"
QT_MOC_LITERAL(6, 62, 10), // "itemPicked"
QT_MOC_LITERAL(7, 73, 6), // "GItem*"
QT_MOC_LITERAL(8, 80, 4) // "item"

    },
    "CCurvePicker\0curveChanged\0\0nameChanged\0"
    "buttonToggled\0bchecked\0itemPicked\0"
    "GItem*\0item"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CCurvePicker[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   34,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    0,   35,    2, 0x08 /* Private */,
       4,    1,   36,    2, 0x08 /* Private */,
       6,    1,   39,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void, 0x80000000 | 7,    8,

       0        // eod
};

void CCurvePicker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CCurvePicker *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->curveChanged(); break;
        case 1: _t->nameChanged(); break;
        case 2: _t->buttonToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->itemPicked((*reinterpret_cast< GItem*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CCurvePicker::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CCurvePicker::curveChanged)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CCurvePicker::staticMetaObject = { {
    &QWidget::staticMetaObject,
    qt_meta_stringdata_CCurvePicker.data,
    qt_meta_data_CCurvePicker,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CCurvePicker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CCurvePicker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CCurvePicker.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int CCurvePicker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void CCurvePicker::curveChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
struct qt_meta_stringdata_CCurveListPicker_t {
    QByteArrayData data[7];
    char stringdata0[107];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CCurveListPicker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CCurveListPicker_t qt_meta_stringdata_CCurveListPicker = {
    {
QT_MOC_LITERAL(0, 0, 16), // "CCurveListPicker"
QT_MOC_LITERAL(1, 17, 12), // "curveChanged"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 18), // "onAddButtonClicked"
QT_MOC_LITERAL(4, 50, 18), // "onSubButtonClicked"
QT_MOC_LITERAL(5, 69, 18), // "onDelButtonClicked"
QT_MOC_LITERAL(6, 88, 18) // "onSelButtonClicked"

    },
    "CCurveListPicker\0curveChanged\0\0"
    "onAddButtonClicked\0onSubButtonClicked\0"
    "onDelButtonClicked\0onSelButtonClicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CCurveListPicker[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    0,   40,    2, 0x08 /* Private */,
       4,    0,   41,    2, 0x08 /* Private */,
       5,    0,   42,    2, 0x08 /* Private */,
       6,    0,   43,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CCurveListPicker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CCurveListPicker *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->curveChanged(); break;
        case 1: _t->onAddButtonClicked(); break;
        case 2: _t->onSubButtonClicked(); break;
        case 3: _t->onDelButtonClicked(); break;
        case 4: _t->onSelButtonClicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CCurveListPicker::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CCurveListPicker::curveChanged)) {
                *result = 0;
                return;
            }
        }
    }
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject CCurveListPicker::staticMetaObject = { {
    &QWidget::staticMetaObject,
    qt_meta_stringdata_CCurveListPicker.data,
    qt_meta_data_CCurveListPicker,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CCurveListPicker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CCurveListPicker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CCurveListPicker.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int CCurveListPicker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void CCurveListPicker::curveChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
