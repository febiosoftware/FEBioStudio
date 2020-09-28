/****************************************************************************
** Meta object code from reading C++ file 'SelectionBox.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "SelectionBox.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SelectionBox.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CSelectionBox_t {
    QByteArrayData data[18];
    char stringdata0[293];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CSelectionBox_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CSelectionBox_t qt_meta_stringdata_CSelectionBox = {
    {
QT_MOC_LITERAL(0, 0, 13), // "CSelectionBox"
QT_MOC_LITERAL(1, 14, 16), // "addButtonClicked"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 16), // "subButtonClicked"
QT_MOC_LITERAL(4, 49, 16), // "delButtonClicked"
QT_MOC_LITERAL(5, 66, 16), // "selButtonClicked"
QT_MOC_LITERAL(6, 83, 18), // "clearButtonClicked"
QT_MOC_LITERAL(7, 102, 11), // "nameChanged"
QT_MOC_LITERAL(8, 114, 1), // "t"
QT_MOC_LITERAL(9, 116, 20), // "on_addButton_clicked"
QT_MOC_LITERAL(10, 137, 20), // "on_subButton_clicked"
QT_MOC_LITERAL(11, 158, 20), // "on_delButton_clicked"
QT_MOC_LITERAL(12, 179, 20), // "on_selButton_clicked"
QT_MOC_LITERAL(13, 200, 18), // "on_name_textEdited"
QT_MOC_LITERAL(14, 219, 25), // "on_list_itemDoubleClicked"
QT_MOC_LITERAL(15, 245, 16), // "QListWidgetItem*"
QT_MOC_LITERAL(16, 262, 4), // "item"
QT_MOC_LITERAL(17, 267, 25) // "on_clearSelection_clicked"

    },
    "CSelectionBox\0addButtonClicked\0\0"
    "subButtonClicked\0delButtonClicked\0"
    "selButtonClicked\0clearButtonClicked\0"
    "nameChanged\0t\0on_addButton_clicked\0"
    "on_subButton_clicked\0on_delButton_clicked\0"
    "on_selButton_clicked\0on_name_textEdited\0"
    "on_list_itemDoubleClicked\0QListWidgetItem*\0"
    "item\0on_clearSelection_clicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CSelectionBox[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   79,    2, 0x06 /* Public */,
       3,    0,   80,    2, 0x06 /* Public */,
       4,    0,   81,    2, 0x06 /* Public */,
       5,    0,   82,    2, 0x06 /* Public */,
       6,    0,   83,    2, 0x06 /* Public */,
       7,    1,   84,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       9,    0,   87,    2, 0x08 /* Private */,
      10,    0,   88,    2, 0x08 /* Private */,
      11,    0,   89,    2, 0x08 /* Private */,
      12,    0,   90,    2, 0x08 /* Private */,
      13,    1,   91,    2, 0x08 /* Private */,
      14,    1,   94,    2, 0x08 /* Private */,
      17,    0,   97,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    8,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void, 0x80000000 | 15,   16,
    QMetaType::Void,

       0        // eod
};

void CSelectionBox::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CSelectionBox *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->addButtonClicked(); break;
        case 1: _t->subButtonClicked(); break;
        case 2: _t->delButtonClicked(); break;
        case 3: _t->selButtonClicked(); break;
        case 4: _t->clearButtonClicked(); break;
        case 5: _t->nameChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->on_addButton_clicked(); break;
        case 7: _t->on_subButton_clicked(); break;
        case 8: _t->on_delButton_clicked(); break;
        case 9: _t->on_selButton_clicked(); break;
        case 10: _t->on_name_textEdited((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 11: _t->on_list_itemDoubleClicked((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 12: _t->on_clearSelection_clicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CSelectionBox::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CSelectionBox::addButtonClicked)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CSelectionBox::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CSelectionBox::subButtonClicked)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (CSelectionBox::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CSelectionBox::delButtonClicked)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (CSelectionBox::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CSelectionBox::selButtonClicked)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (CSelectionBox::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CSelectionBox::clearButtonClicked)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (CSelectionBox::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CSelectionBox::nameChanged)) {
                *result = 5;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CSelectionBox::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CSelectionBox.data,
    qt_meta_data_CSelectionBox,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CSelectionBox::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CSelectionBox::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CSelectionBox.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int CSelectionBox::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void CSelectionBox::addButtonClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void CSelectionBox::subButtonClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void CSelectionBox::delButtonClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void CSelectionBox::selButtonClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void CSelectionBox::clearButtonClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void CSelectionBox::nameChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
