/****************************************************************************
** Meta object code from reading C++ file 'GraphWindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "GraphWindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GraphWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_OptionsUi_t {
    QByteArrayData data[4];
    char stringdata0[43];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_OptionsUi_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_OptionsUi_t qt_meta_stringdata_OptionsUi = {
    {
QT_MOC_LITERAL(0, 0, 9), // "OptionsUi"
QT_MOC_LITERAL(1, 10, 14), // "optionsChanged"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 16) // "onOptionsChanged"

    },
    "OptionsUi\0optionsChanged\0\0onOptionsChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_OptionsUi[] = {

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

void OptionsUi::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        OptionsUi *_t = static_cast<OptionsUi *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->optionsChanged(); break;
        case 1: _t->onOptionsChanged(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (OptionsUi::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&OptionsUi::optionsChanged)) {
                *result = 0;
                return;
            }
        }
    }
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject OptionsUi::staticMetaObject = { {
    &CPlotTool::staticMetaObject,
    qt_meta_stringdata_OptionsUi.data,
    qt_meta_data_OptionsUi,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *OptionsUi::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *OptionsUi::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_OptionsUi.stringdata0))
        return static_cast<void*>(this);
    return CPlotTool::qt_metacast(_clname);
}

int OptionsUi::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void OptionsUi::optionsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
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
        RegressionUi *_t = static_cast<RegressionUi *>(_o);
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
    &CPlotTool::staticMetaObject,
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
        MathPlot *_t = static_cast<MathPlot *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onCalculate(); break;
        case 1: _t->onColorChanged((*reinterpret_cast< QColor(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MathPlot::staticMetaObject = { {
    &CPlotTool::staticMetaObject,
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
    QByteArrayData data[15];
    char stringdata0[360];
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
QT_MOC_LITERAL(6, 134, 28), // "on_actionClipboard_triggered"
QT_MOC_LITERAL(7, 163, 24), // "on_actionProps_triggered"
QT_MOC_LITERAL(8, 188, 28), // "on_actionZoomWidth_triggered"
QT_MOC_LITERAL(9, 217, 29), // "on_actionZoomHeight_triggered"
QT_MOC_LITERAL(10, 247, 26), // "on_actionZoomFit_triggered"
QT_MOC_LITERAL(11, 274, 27), // "on_actionZoomSelect_toggled"
QT_MOC_LITERAL(12, 302, 8), // "bchecked"
QT_MOC_LITERAL(13, 311, 22), // "on_plot_doneZoomToRect"
QT_MOC_LITERAL(14, 334, 25) // "on_options_optionsChanged"

    },
    "CGraphWindow\0on_selectX_currentValueChanged\0"
    "\0on_selectY_currentValueChanged\0"
    "on_selectPlot_currentIndexChanged\0"
    "on_actionSave_triggered\0"
    "on_actionClipboard_triggered\0"
    "on_actionProps_triggered\0"
    "on_actionZoomWidth_triggered\0"
    "on_actionZoomHeight_triggered\0"
    "on_actionZoomFit_triggered\0"
    "on_actionZoomSelect_toggled\0bchecked\0"
    "on_plot_doneZoomToRect\0on_options_optionsChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CGraphWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   74,    2, 0x08 /* Private */,
       3,    1,   77,    2, 0x08 /* Private */,
       4,    1,   80,    2, 0x08 /* Private */,
       5,    0,   83,    2, 0x08 /* Private */,
       6,    0,   84,    2, 0x08 /* Private */,
       7,    0,   85,    2, 0x08 /* Private */,
       8,    0,   86,    2, 0x08 /* Private */,
       9,    0,   87,    2, 0x08 /* Private */,
      10,    0,   88,    2, 0x08 /* Private */,
      11,    1,   89,    2, 0x08 /* Private */,
      13,    0,   92,    2, 0x08 /* Private */,
      14,    0,   93,    2, 0x08 /* Private */,

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
    QMetaType::Void, QMetaType::Bool,   12,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CGraphWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CGraphWindow *_t = static_cast<CGraphWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_selectX_currentValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->on_selectY_currentValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->on_selectPlot_currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->on_actionSave_triggered(); break;
        case 4: _t->on_actionClipboard_triggered(); break;
        case 5: _t->on_actionProps_triggered(); break;
        case 6: _t->on_actionZoomWidth_triggered(); break;
        case 7: _t->on_actionZoomHeight_triggered(); break;
        case 8: _t->on_actionZoomFit_triggered(); break;
        case 9: _t->on_actionZoomSelect_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 10: _t->on_plot_doneZoomToRect(); break;
        case 11: _t->on_options_optionsChanged(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CGraphWindow::staticMetaObject = { {
    &QMainWindow::staticMetaObject,
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
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 12;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
