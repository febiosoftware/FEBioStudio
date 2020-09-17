/****************************************************************************
** Meta object code from reading C++ file 'TimelineWidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "TimelineWidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TimelineWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CTimelineWidget_t {
    QByteArrayData data[7];
    char stringdata0[55];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CTimelineWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CTimelineWidget_t qt_meta_stringdata_CTimelineWidget = {
    {
QT_MOC_LITERAL(0, 0, 15), // "CTimelineWidget"
QT_MOC_LITERAL(1, 16, 12), // "pointClicked"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 1), // "n"
QT_MOC_LITERAL(4, 32, 12), // "rangeChanged"
QT_MOC_LITERAL(5, 45, 4), // "nmin"
QT_MOC_LITERAL(6, 50, 4) // "nmax"

    },
    "CTimelineWidget\0pointClicked\0\0n\0"
    "rangeChanged\0nmin\0nmax"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CTimelineWidget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x06 /* Public */,
       4,    2,   27,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    5,    6,

       0        // eod
};

void CTimelineWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CTimelineWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->pointClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->rangeChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CTimelineWidget::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CTimelineWidget::pointClicked)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CTimelineWidget::*)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CTimelineWidget::rangeChanged)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CTimelineWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CTimelineWidget.data,
    qt_meta_data_CTimelineWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CTimelineWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CTimelineWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CTimelineWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int CTimelineWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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

// SIGNAL 0
void CTimelineWidget::pointClicked(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void CTimelineWidget::rangeChanged(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
