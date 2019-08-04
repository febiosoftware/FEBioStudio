/****************************************************************************
** Meta object code from reading C++ file 'ModelPropsPanel.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "ModelPropsPanel.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ModelPropsPanel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CObjectPropsPanel_t {
    QByteArrayData data[8];
    char stringdata0[93];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CObjectPropsPanel_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CObjectPropsPanel_t qt_meta_stringdata_CObjectPropsPanel = {
    {
QT_MOC_LITERAL(0, 0, 17), // "CObjectPropsPanel"
QT_MOC_LITERAL(1, 18, 11), // "nameChanged"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 7), // "newName"
QT_MOC_LITERAL(4, 39, 12), // "colorChanged"
QT_MOC_LITERAL(5, 52, 1), // "c"
QT_MOC_LITERAL(6, 54, 18), // "on_name_textEdited"
QT_MOC_LITERAL(7, 73, 19) // "on_col_colorChanged"

    },
    "CObjectPropsPanel\0nameChanged\0\0newName\0"
    "colorChanged\0c\0on_name_textEdited\0"
    "on_col_colorChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CObjectPropsPanel[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   34,    2, 0x06 /* Public */,
       4,    1,   37,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    1,   40,    2, 0x09 /* Protected */,
       7,    1,   43,    2, 0x09 /* Protected */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QColor,    5,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::QColor,    5,

       0        // eod
};

void CObjectPropsPanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CObjectPropsPanel *_t = static_cast<CObjectPropsPanel *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->nameChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->colorChanged((*reinterpret_cast< const QColor(*)>(_a[1]))); break;
        case 2: _t->on_name_textEdited((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->on_col_colorChanged((*reinterpret_cast< QColor(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CObjectPropsPanel::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CObjectPropsPanel::nameChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CObjectPropsPanel::*)(const QColor & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CObjectPropsPanel::colorChanged)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CObjectPropsPanel::staticMetaObject = { {
    &QWidget::staticMetaObject,
    qt_meta_stringdata_CObjectPropsPanel.data,
    qt_meta_data_CObjectPropsPanel,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CObjectPropsPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CObjectPropsPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CObjectPropsPanel.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int CObjectPropsPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void CObjectPropsPanel::nameChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void CObjectPropsPanel::colorChanged(const QColor & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
struct qt_meta_stringdata_CBCObjectPropsPanel_t {
    QByteArrayData data[8];
    char stringdata0[102];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CBCObjectPropsPanel_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CBCObjectPropsPanel_t qt_meta_stringdata_CBCObjectPropsPanel = {
    {
QT_MOC_LITERAL(0, 0, 19), // "CBCObjectPropsPanel"
QT_MOC_LITERAL(1, 20, 11), // "nameChanged"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 7), // "newName"
QT_MOC_LITERAL(4, 41, 11), // "stepChanged"
QT_MOC_LITERAL(5, 53, 1), // "n"
QT_MOC_LITERAL(6, 55, 18), // "on_name_textEdited"
QT_MOC_LITERAL(7, 74, 27) // "on_list_currentIndexChanged"

    },
    "CBCObjectPropsPanel\0nameChanged\0\0"
    "newName\0stepChanged\0n\0on_name_textEdited\0"
    "on_list_currentIndexChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CBCObjectPropsPanel[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   34,    2, 0x06 /* Public */,
       4,    1,   37,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    1,   40,    2, 0x09 /* Protected */,
       7,    1,   43,    2, 0x09 /* Protected */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::Int,    5,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::Int,    5,

       0        // eod
};

void CBCObjectPropsPanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CBCObjectPropsPanel *_t = static_cast<CBCObjectPropsPanel *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->nameChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->stepChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->on_name_textEdited((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->on_list_currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CBCObjectPropsPanel::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CBCObjectPropsPanel::nameChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CBCObjectPropsPanel::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CBCObjectPropsPanel::stepChanged)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CBCObjectPropsPanel::staticMetaObject = { {
    &QWidget::staticMetaObject,
    qt_meta_stringdata_CBCObjectPropsPanel.data,
    qt_meta_data_CBCObjectPropsPanel,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CBCObjectPropsPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CBCObjectPropsPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CBCObjectPropsPanel.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int CBCObjectPropsPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void CBCObjectPropsPanel::nameChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void CBCObjectPropsPanel::stepChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
struct qt_meta_stringdata_CModelPropsPanel_t {
    QByteArrayData data[27];
    char stringdata0[490];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CModelPropsPanel_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CModelPropsPanel_t qt_meta_stringdata_CModelPropsPanel = {
    {
QT_MOC_LITERAL(0, 0, 16), // "CModelPropsPanel"
QT_MOC_LITERAL(1, 17, 11), // "nameChanged"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 3), // "txt"
QT_MOC_LITERAL(4, 34, 16), // "selectionChanged"
QT_MOC_LITERAL(5, 51, 11), // "dataChanged"
QT_MOC_LITERAL(6, 63, 1), // "b"
QT_MOC_LITERAL(7, 65, 27), // "on_select1_addButtonClicked"
QT_MOC_LITERAL(8, 93, 27), // "on_select1_subButtonClicked"
QT_MOC_LITERAL(9, 121, 27), // "on_select1_delButtonClicked"
QT_MOC_LITERAL(10, 149, 27), // "on_select1_selButtonClicked"
QT_MOC_LITERAL(11, 177, 27), // "on_select2_addButtonClicked"
QT_MOC_LITERAL(12, 205, 27), // "on_select2_subButtonClicked"
QT_MOC_LITERAL(13, 233, 27), // "on_select2_delButtonClicked"
QT_MOC_LITERAL(14, 261, 27), // "on_select2_selButtonClicked"
QT_MOC_LITERAL(15, 289, 22), // "on_select1_nameChanged"
QT_MOC_LITERAL(16, 312, 1), // "t"
QT_MOC_LITERAL(17, 314, 22), // "on_select2_nameChanged"
QT_MOC_LITERAL(18, 337, 21), // "on_object_nameChanged"
QT_MOC_LITERAL(19, 359, 23), // "on_bcobject_nameChanged"
QT_MOC_LITERAL(20, 383, 22), // "on_object_colorChanged"
QT_MOC_LITERAL(21, 406, 3), // "col"
QT_MOC_LITERAL(22, 410, 20), // "on_props_dataChanged"
QT_MOC_LITERAL(23, 431, 19), // "on_form_dataChanged"
QT_MOC_LITERAL(24, 451, 12), // "itemModified"
QT_MOC_LITERAL(25, 464, 23), // "on_bcobject_stepChanged"
QT_MOC_LITERAL(26, 488, 1) // "n"

    },
    "CModelPropsPanel\0nameChanged\0\0txt\0"
    "selectionChanged\0dataChanged\0b\0"
    "on_select1_addButtonClicked\0"
    "on_select1_subButtonClicked\0"
    "on_select1_delButtonClicked\0"
    "on_select1_selButtonClicked\0"
    "on_select2_addButtonClicked\0"
    "on_select2_subButtonClicked\0"
    "on_select2_delButtonClicked\0"
    "on_select2_selButtonClicked\0"
    "on_select1_nameChanged\0t\0"
    "on_select2_nameChanged\0on_object_nameChanged\0"
    "on_bcobject_nameChanged\0on_object_colorChanged\0"
    "col\0on_props_dataChanged\0on_form_dataChanged\0"
    "itemModified\0on_bcobject_stepChanged\0"
    "n"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CModelPropsPanel[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      19,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,  109,    2, 0x06 /* Public */,
       4,    0,  112,    2, 0x06 /* Public */,
       5,    1,  113,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    0,  116,    2, 0x08 /* Private */,
       8,    0,  117,    2, 0x08 /* Private */,
       9,    0,  118,    2, 0x08 /* Private */,
      10,    0,  119,    2, 0x08 /* Private */,
      11,    0,  120,    2, 0x08 /* Private */,
      12,    0,  121,    2, 0x08 /* Private */,
      13,    0,  122,    2, 0x08 /* Private */,
      14,    0,  123,    2, 0x08 /* Private */,
      15,    1,  124,    2, 0x08 /* Private */,
      17,    1,  127,    2, 0x08 /* Private */,
      18,    1,  130,    2, 0x08 /* Private */,
      19,    1,  133,    2, 0x08 /* Private */,
      20,    1,  136,    2, 0x08 /* Private */,
      22,    0,  139,    2, 0x08 /* Private */,
      23,    1,  140,    2, 0x08 /* Private */,
      25,    1,  143,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    6,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   16,
    QMetaType::Void, QMetaType::QString,   16,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::QColor,   21,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   24,
    QMetaType::Void, QMetaType::Int,   26,

       0        // eod
};

void CModelPropsPanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CModelPropsPanel *_t = static_cast<CModelPropsPanel *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->nameChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->selectionChanged(); break;
        case 2: _t->dataChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->on_select1_addButtonClicked(); break;
        case 4: _t->on_select1_subButtonClicked(); break;
        case 5: _t->on_select1_delButtonClicked(); break;
        case 6: _t->on_select1_selButtonClicked(); break;
        case 7: _t->on_select2_addButtonClicked(); break;
        case 8: _t->on_select2_subButtonClicked(); break;
        case 9: _t->on_select2_delButtonClicked(); break;
        case 10: _t->on_select2_selButtonClicked(); break;
        case 11: _t->on_select1_nameChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 12: _t->on_select2_nameChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 13: _t->on_object_nameChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 14: _t->on_bcobject_nameChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 15: _t->on_object_colorChanged((*reinterpret_cast< const QColor(*)>(_a[1]))); break;
        case 16: _t->on_props_dataChanged(); break;
        case 17: _t->on_form_dataChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 18: _t->on_bcobject_stepChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CModelPropsPanel::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CModelPropsPanel::nameChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CModelPropsPanel::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CModelPropsPanel::selectionChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (CModelPropsPanel::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CModelPropsPanel::dataChanged)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CModelPropsPanel::staticMetaObject = { {
    &QWidget::staticMetaObject,
    qt_meta_stringdata_CModelPropsPanel.data,
    qt_meta_data_CModelPropsPanel,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CModelPropsPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CModelPropsPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CModelPropsPanel.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int CModelPropsPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 19)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 19;
    }
    return _id;
}

// SIGNAL 0
void CModelPropsPanel::nameChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void CModelPropsPanel::selectionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void CModelPropsPanel::dataChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
