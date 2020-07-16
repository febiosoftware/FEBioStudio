/****************************************************************************
** Meta object code from reading C++ file 'GraphWindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "GraphWindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GraphWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_TimeRangeOptions_t {
    QByteArrayData data[4];
    char stringdata0[50];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_TimeRangeOptions_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_TimeRangeOptions_t qt_meta_stringdata_TimeRangeOptions = {
    {
QT_MOC_LITERAL(0, 0, 16), // "TimeRangeOptions"
QT_MOC_LITERAL(1, 17, 14), // "optionsChanged"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 16) // "onOptionsChanged"

    },
    "TimeRangeOptions\0optionsChanged\0\0"
    "onOptionsChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TimeRangeOptions[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   24,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    0,   25,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void TimeRangeOptions::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TimeRangeOptions *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->optionsChanged(); break;
        case 1: _t->onOptionsChanged(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (TimeRangeOptions::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TimeRangeOptions::optionsChanged)) {
                *result = 0;
                return;
            }
        }
    }
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject TimeRangeOptions::staticMetaObject = { {
    QMetaObject::SuperData::link<CPlotTool::staticMetaObject>(),
    qt_meta_stringdata_TimeRangeOptions.data,
    qt_meta_data_TimeRangeOptions,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *TimeRangeOptions::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TimeRangeOptions::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TimeRangeOptions.stringdata0))
        return static_cast<void*>(this);
    return CPlotTool::qt_metacast(_clname);
}

int TimeRangeOptions::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CPlotTool::qt_metacall(_c, _id, _a);
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
void TimeRangeOptions::optionsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
struct qt_meta_stringdata_GraphOptions_t {
    QByteArrayData data[3];
    char stringdata0[31];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_GraphOptions_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_GraphOptions_t qt_meta_stringdata_GraphOptions = {
    {
QT_MOC_LITERAL(0, 0, 12), // "GraphOptions"
QT_MOC_LITERAL(1, 13, 16), // "onOptionsChanged"
QT_MOC_LITERAL(2, 30, 0) // ""

    },
    "GraphOptions\0onOptionsChanged\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_GraphOptions[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   19,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void GraphOptions::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<GraphOptions *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onOptionsChanged(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject GraphOptions::staticMetaObject = { {
    QMetaObject::SuperData::link<CPlotTool::staticMetaObject>(),
    qt_meta_stringdata_GraphOptions.data,
    qt_meta_data_GraphOptions,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *GraphOptions::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GraphOptions::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_GraphOptions.stringdata0))
        return static_cast<void*>(this);
    return CPlotTool::qt_metacast(_clname);
}

int GraphOptions::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CPlotTool::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
struct qt_meta_stringdata_DataOptions_t {
    QByteArrayData data[6];
    char stringdata0[57];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DataOptions_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DataOptions_t qt_meta_stringdata_DataOptions = {
    {
QT_MOC_LITERAL(0, 0, 11), // "DataOptions"
QT_MOC_LITERAL(1, 12, 13), // "onIndexChange"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 1), // "n"
QT_MOC_LITERAL(4, 29, 12), // "onDataChange"
QT_MOC_LITERAL(5, 42, 14) // "onLabelChanged"

    },
    "DataOptions\0onIndexChange\0\0n\0onDataChange\0"
    "onLabelChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DataOptions[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x0a /* Public */,
       4,    1,   32,    2, 0x0a /* Public */,
       5,    0,   35,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void,

       0        // eod
};

void DataOptions::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DataOptions *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onIndexChange((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->onDataChange((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->onLabelChanged(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DataOptions::staticMetaObject = { {
    QMetaObject::SuperData::link<CPlotTool::staticMetaObject>(),
    qt_meta_stringdata_DataOptions.data,
    qt_meta_data_DataOptions,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DataOptions::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DataOptions::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DataOptions.stringdata0))
        return static_cast<void*>(this);
    return CPlotTool::qt_metacast(_clname);
}

int DataOptions::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CPlotTool::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}
struct qt_meta_stringdata_RegressionUi_t {
    QByteArrayData data[7];
    char stringdata0[63];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_RegressionUi_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_RegressionUi_t qt_meta_stringdata_RegressionUi = {
    {
QT_MOC_LITERAL(0, 0, 12), // "RegressionUi"
QT_MOC_LITERAL(1, 13, 11), // "onCalculate"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 17), // "onFunctionChanged"
QT_MOC_LITERAL(4, 44, 1), // "n"
QT_MOC_LITERAL(5, 46, 14), // "onColorChanged"
QT_MOC_LITERAL(6, 61, 1) // "c"

    },
    "RegressionUi\0onCalculate\0\0onFunctionChanged\0"
    "n\0onColorChanged\0c"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_RegressionUi[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   29,    2, 0x0a /* Public */,
       3,    1,   30,    2, 0x0a /* Public */,
       5,    1,   33,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::QColor,    6,

       0        // eod
};

void RegressionUi::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<RegressionUi *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onCalculate(); break;
        case 1: _t->onFunctionChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->onColorChanged((*reinterpret_cast< QColor(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject RegressionUi::staticMetaObject = { {
    QMetaObject::SuperData::link<CPlotTool::staticMetaObject>(),
    qt_meta_stringdata_RegressionUi.data,
    qt_meta_data_RegressionUi,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *RegressionUi::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RegressionUi::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_RegressionUi.stringdata0))
        return static_cast<void*>(this);
    return CPlotTool::qt_metacast(_clname);
}

int RegressionUi::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CPlotTool::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}
struct qt_meta_stringdata_MathPlot_t {
    QByteArrayData data[5];
    char stringdata0[39];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MathPlot_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MathPlot_t qt_meta_stringdata_MathPlot = {
    {
QT_MOC_LITERAL(0, 0, 8), // "MathPlot"
QT_MOC_LITERAL(1, 9, 11), // "onCalculate"
QT_MOC_LITERAL(2, 21, 0), // ""
QT_MOC_LITERAL(3, 22, 14), // "onColorChanged"
QT_MOC_LITERAL(4, 37, 1) // "c"

    },
    "MathPlot\0onCalculate\0\0onColorChanged\0"
    "c"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MathPlot[] = {

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
       1,    0,   24,    2, 0x0a /* Public */,
       3,    1,   25,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QColor,    4,

       0        // eod
};

void MathPlot::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MathPlot *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onCalculate(); break;
        case 1: _t->onColorChanged((*reinterpret_cast< QColor(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MathPlot::staticMetaObject = { {
    QMetaObject::SuperData::link<CPlotTool::staticMetaObject>(),
    qt_meta_stringdata_MathPlot.data,
    qt_meta_data_MathPlot,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MathPlot::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MathPlot::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MathPlot.stringdata0))
        return static_cast<void*>(this);
    return CPlotTool::qt_metacast(_clname);
}

int MathPlot::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CPlotTool::qt_metacall(_c, _id, _a);
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
struct qt_meta_stringdata_CGraphWindow_t {
    QByteArrayData data[18];
    char stringdata0[450];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CGraphWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CGraphWindow_t qt_meta_stringdata_CGraphWindow = {
    {
QT_MOC_LITERAL(0, 0, 12), // "CGraphWindow"
QT_MOC_LITERAL(1, 13, 30), // "on_selectX_currentValueChanged"
QT_MOC_LITERAL(2, 44, 0), // ""
QT_MOC_LITERAL(3, 45, 30), // "on_selectY_currentValueChanged"
QT_MOC_LITERAL(4, 76, 33), // "on_selectPlot_currentIndexCha..."
QT_MOC_LITERAL(5, 110, 23), // "on_actionSave_triggered"
QT_MOC_LITERAL(6, 134, 29), // "on_actionAddToModel_triggered"
QT_MOC_LITERAL(7, 164, 28), // "on_actionClipboard_triggered"
QT_MOC_LITERAL(8, 193, 27), // "on_actionSnapshot_triggered"
QT_MOC_LITERAL(9, 221, 24), // "on_actionProps_triggered"
QT_MOC_LITERAL(10, 246, 28), // "on_actionZoomWidth_triggered"
QT_MOC_LITERAL(11, 275, 29), // "on_actionZoomHeight_triggered"
QT_MOC_LITERAL(12, 305, 26), // "on_actionZoomFit_triggered"
QT_MOC_LITERAL(13, 332, 27), // "on_actionZoomSelect_toggled"
QT_MOC_LITERAL(14, 360, 8), // "bchecked"
QT_MOC_LITERAL(15, 369, 22), // "on_plot_doneZoomToRect"
QT_MOC_LITERAL(16, 392, 23), // "on_range_optionsChanged"
QT_MOC_LITERAL(17, 416, 33) // "on_dataSource_currentIndexCha..."

    },
    "CGraphWindow\0on_selectX_currentValueChanged\0"
    "\0on_selectY_currentValueChanged\0"
    "on_selectPlot_currentIndexChanged\0"
    "on_actionSave_triggered\0"
    "on_actionAddToModel_triggered\0"
    "on_actionClipboard_triggered\0"
    "on_actionSnapshot_triggered\0"
    "on_actionProps_triggered\0"
    "on_actionZoomWidth_triggered\0"
    "on_actionZoomHeight_triggered\0"
    "on_actionZoomFit_triggered\0"
    "on_actionZoomSelect_toggled\0bchecked\0"
    "on_plot_doneZoomToRect\0on_range_optionsChanged\0"
    "on_dataSource_currentIndexChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CGraphWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      15,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   89,    2, 0x08 /* Private */,
       3,    1,   92,    2, 0x08 /* Private */,
       4,    1,   95,    2, 0x08 /* Private */,
       5,    0,   98,    2, 0x08 /* Private */,
       6,    0,   99,    2, 0x08 /* Private */,
       7,    0,  100,    2, 0x08 /* Private */,
       8,    0,  101,    2, 0x08 /* Private */,
       9,    0,  102,    2, 0x08 /* Private */,
      10,    0,  103,    2, 0x08 /* Private */,
      11,    0,  104,    2, 0x08 /* Private */,
      12,    0,  105,    2, 0x08 /* Private */,
      13,    1,  106,    2, 0x08 /* Private */,
      15,    0,  109,    2, 0x08 /* Private */,
      16,    0,  110,    2, 0x08 /* Private */,
      17,    1,  111,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   14,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,

       0        // eod
};

void CGraphWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CGraphWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_selectX_currentValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->on_selectY_currentValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->on_selectPlot_currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->on_actionSave_triggered(); break;
        case 4: _t->on_actionAddToModel_triggered(); break;
        case 5: _t->on_actionClipboard_triggered(); break;
        case 6: _t->on_actionSnapshot_triggered(); break;
        case 7: _t->on_actionProps_triggered(); break;
        case 8: _t->on_actionZoomWidth_triggered(); break;
        case 9: _t->on_actionZoomHeight_triggered(); break;
        case 10: _t->on_actionZoomFit_triggered(); break;
        case 11: _t->on_actionZoomSelect_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 12: _t->on_plot_doneZoomToRect(); break;
        case 13: _t->on_range_optionsChanged(); break;
        case 14: _t->on_dataSource_currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CGraphWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_CGraphWindow.data,
    qt_meta_data_CGraphWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CGraphWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CGraphWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CGraphWindow.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "CDocObserver"))
        return static_cast< CDocObserver*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int CGraphWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 15)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 15;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
