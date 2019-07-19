/****************************************************************************
** Meta object code from reading C++ file 'PlotWidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "PlotWidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PlotWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CPlotWidget_t {
    QByteArrayData data[16];
    char stringdata0[176];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CPlotWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CPlotWidget_t qt_meta_stringdata_CPlotWidget = {
    {
QT_MOC_LITERAL(0, 0, 11), // "CPlotWidget"
QT_MOC_LITERAL(1, 12, 14), // "doneZoomToRect"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 12), // "pointClicked"
QT_MOC_LITERAL(4, 41, 1), // "p"
QT_MOC_LITERAL(5, 43, 6), // "bshift"
QT_MOC_LITERAL(6, 50, 13), // "pointSelected"
QT_MOC_LITERAL(7, 64, 1), // "n"
QT_MOC_LITERAL(8, 66, 12), // "pointDragged"
QT_MOC_LITERAL(9, 79, 13), // "draggingStart"
QT_MOC_LITERAL(10, 93, 11), // "draggingEnd"
QT_MOC_LITERAL(11, 105, 13), // "OnZoomToWidth"
QT_MOC_LITERAL(12, 119, 14), // "OnZoomToHeight"
QT_MOC_LITERAL(13, 134, 11), // "OnZoomToFit"
QT_MOC_LITERAL(14, 146, 11), // "OnShowProps"
QT_MOC_LITERAL(15, 158, 17) // "OnCopyToClipboard"

    },
    "CPlotWidget\0doneZoomToRect\0\0pointClicked\0"
    "p\0bshift\0pointSelected\0n\0pointDragged\0"
    "draggingStart\0draggingEnd\0OnZoomToWidth\0"
    "OnZoomToHeight\0OnZoomToFit\0OnShowProps\0"
    "OnCopyToClipboard"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CPlotWidget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   69,    2, 0x06 /* Public */,
       3,    2,   70,    2, 0x06 /* Public */,
       6,    1,   75,    2, 0x06 /* Public */,
       8,    1,   78,    2, 0x06 /* Public */,
       9,    1,   81,    2, 0x06 /* Public */,
      10,    1,   84,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      11,    0,   87,    2, 0x0a /* Public */,
      12,    0,   88,    2, 0x0a /* Public */,
      13,    0,   89,    2, 0x0a /* Public */,
      14,    0,   90,    2, 0x0a /* Public */,
      15,    0,   91,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QPointF, QMetaType::Bool,    4,    5,
    QMetaType::Void, QMetaType::Int,    7,
    QMetaType::Void, QMetaType::QPoint,    4,
    QMetaType::Void, QMetaType::QPoint,    4,
    QMetaType::Void, QMetaType::QPoint,    4,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CPlotWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CPlotWidget *_t = static_cast<CPlotWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->doneZoomToRect(); break;
        case 1: _t->pointClicked((*reinterpret_cast< QPointF(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 2: _t->pointSelected((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->pointDragged((*reinterpret_cast< QPoint(*)>(_a[1]))); break;
        case 4: _t->draggingStart((*reinterpret_cast< QPoint(*)>(_a[1]))); break;
        case 5: _t->draggingEnd((*reinterpret_cast< QPoint(*)>(_a[1]))); break;
        case 6: _t->OnZoomToWidth(); break;
        case 7: _t->OnZoomToHeight(); break;
        case 8: _t->OnZoomToFit(); break;
        case 9: _t->OnShowProps(); break;
        case 10: _t->OnCopyToClipboard(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CPlotWidget::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CPlotWidget::doneZoomToRect)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CPlotWidget::*)(QPointF , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CPlotWidget::pointClicked)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (CPlotWidget::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CPlotWidget::pointSelected)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (CPlotWidget::*)(QPoint );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CPlotWidget::pointDragged)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (CPlotWidget::*)(QPoint );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CPlotWidget::draggingStart)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (CPlotWidget::*)(QPoint );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CPlotWidget::draggingEnd)) {
                *result = 5;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CPlotWidget::staticMetaObject = { {
    &QWidget::staticMetaObject,
    qt_meta_stringdata_CPlotWidget.data,
    qt_meta_data_CPlotWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CPlotWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CPlotWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CPlotWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int CPlotWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void CPlotWidget::doneZoomToRect()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void CPlotWidget::pointClicked(QPointF _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void CPlotWidget::pointSelected(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void CPlotWidget::pointDragged(QPoint _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void CPlotWidget::draggingStart(QPoint _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void CPlotWidget::draggingEnd(QPoint _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
struct qt_meta_stringdata_CDlgPlotWidgetProps_t {
    QByteArrayData data[1];
    char stringdata0[20];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CDlgPlotWidgetProps_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CDlgPlotWidgetProps_t qt_meta_stringdata_CDlgPlotWidgetProps = {
    {
QT_MOC_LITERAL(0, 0, 19) // "CDlgPlotWidgetProps"

    },
    "CDlgPlotWidgetProps"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CDlgPlotWidgetProps[] = {

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

void CDlgPlotWidgetProps::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject CDlgPlotWidgetProps::staticMetaObject = { {
    &QDialog::staticMetaObject,
    qt_meta_stringdata_CDlgPlotWidgetProps.data,
    qt_meta_data_CDlgPlotWidgetProps,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CDlgPlotWidgetProps::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CDlgPlotWidgetProps::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CDlgPlotWidgetProps.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int CDlgPlotWidgetProps::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
