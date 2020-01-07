/****************************************************************************
** Meta object code from reading C++ file 'ModelViewer.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "ModelViewer.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ModelViewer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CModelViewer_t {
    QByteArrayData data[71];
    char stringdata0[1159];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CModelViewer_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CModelViewer_t qt_meta_stringdata_CModelViewer = {
    {
QT_MOC_LITERAL(0, 0, 12), // "CModelViewer"
QT_MOC_LITERAL(1, 13, 20), // "currentObjectChanged"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 9), // "FSObject*"
QT_MOC_LITERAL(4, 45, 2), // "po"
QT_MOC_LITERAL(5, 48, 31), // "on_modelTree_currentItemChanged"
QT_MOC_LITERAL(6, 80, 16), // "QTreeWidgetItem*"
QT_MOC_LITERAL(7, 97, 7), // "current"
QT_MOC_LITERAL(8, 105, 4), // "prev"
QT_MOC_LITERAL(9, 110, 23), // "on_selectButton_clicked"
QT_MOC_LITERAL(10, 134, 23), // "on_deleteButton_clicked"
QT_MOC_LITERAL(11, 158, 23), // "on_searchButton_toggled"
QT_MOC_LITERAL(12, 182, 1), // "b"
QT_MOC_LITERAL(13, 184, 21), // "on_syncButton_clicked"
QT_MOC_LITERAL(14, 206, 20), // "on_props_nameChanged"
QT_MOC_LITERAL(15, 227, 3), // "txt"
QT_MOC_LITERAL(16, 231, 25), // "on_props_selectionChanged"
QT_MOC_LITERAL(17, 257, 20), // "on_props_dataChanged"
QT_MOC_LITERAL(18, 278, 12), // "OnDeleteItem"
QT_MOC_LITERAL(19, 291, 13), // "OnAddMaterial"
QT_MOC_LITERAL(20, 305, 18), // "OnUnhideAllObjects"
QT_MOC_LITERAL(21, 324, 16), // "OnUnhideAllParts"
QT_MOC_LITERAL(22, 341, 7), // "OnAddBC"
QT_MOC_LITERAL(23, 349, 16), // "OnAddSurfaceLoad"
QT_MOC_LITERAL(24, 366, 13), // "OnAddBodyLoad"
QT_MOC_LITERAL(25, 380, 21), // "OnAddInitialCondition"
QT_MOC_LITERAL(26, 402, 12), // "OnAddContact"
QT_MOC_LITERAL(27, 415, 20), // "OnAddRigidConstraint"
QT_MOC_LITERAL(28, 436, 19), // "OnAddRigidConnector"
QT_MOC_LITERAL(29, 456, 9), // "OnAddStep"
QT_MOC_LITERAL(30, 466, 12), // "OnHideObject"
QT_MOC_LITERAL(31, 479, 12), // "OnShowObject"
QT_MOC_LITERAL(32, 492, 14), // "OnSelectObject"
QT_MOC_LITERAL(33, 507, 22), // "OnSelectDiscreteObject"
QT_MOC_LITERAL(34, 530, 22), // "OnDetachDiscreteObject"
QT_MOC_LITERAL(35, 553, 10), // "OnHidePart"
QT_MOC_LITERAL(36, 564, 10), // "OnShowPart"
QT_MOC_LITERAL(37, 575, 12), // "OnSelectPart"
QT_MOC_LITERAL(38, 588, 15), // "OnSelectSurface"
QT_MOC_LITERAL(39, 604, 13), // "OnSelectCurve"
QT_MOC_LITERAL(40, 618, 12), // "OnSelectNode"
QT_MOC_LITERAL(41, 631, 14), // "OnCopyMaterial"
QT_MOC_LITERAL(42, 646, 15), // "OnCopyInterface"
QT_MOC_LITERAL(43, 662, 8), // "OnCopyBC"
QT_MOC_LITERAL(44, 671, 8), // "OnCopyIC"
QT_MOC_LITERAL(45, 680, 20), // "OnCopyRigidConnector"
QT_MOC_LITERAL(46, 701, 10), // "OnCopyLoad"
QT_MOC_LITERAL(47, 712, 21), // "OnCopyRigidConstraint"
QT_MOC_LITERAL(48, 734, 10), // "OnCopyStep"
QT_MOC_LITERAL(49, 745, 10), // "OnRerunJob"
QT_MOC_LITERAL(50, 756, 12), // "OnEditOutput"
QT_MOC_LITERAL(51, 769, 15), // "OnEditOutputLog"
QT_MOC_LITERAL(52, 785, 23), // "OnRemoveEmptySelections"
QT_MOC_LITERAL(53, 809, 21), // "OnRemoveAllSelections"
QT_MOC_LITERAL(54, 831, 16), // "OnChangeMaterial"
QT_MOC_LITERAL(55, 848, 19), // "OnMaterialHideParts"
QT_MOC_LITERAL(56, 868, 19), // "OnMaterialShowParts"
QT_MOC_LITERAL(57, 888, 24), // "OnMaterialHideOtherParts"
QT_MOC_LITERAL(58, 913, 17), // "OnExportMaterials"
QT_MOC_LITERAL(59, 931, 20), // "OnExportAllMaterials"
QT_MOC_LITERAL(60, 952, 17), // "OnImportMaterials"
QT_MOC_LITERAL(61, 970, 20), // "OnDeleteAllMaterials"
QT_MOC_LITERAL(62, 991, 17), // "OnSwapMasterSlave"
QT_MOC_LITERAL(63, 1009, 13), // "OnGenerateMap"
QT_MOC_LITERAL(64, 1023, 13), // "OnDeleteAllBC"
QT_MOC_LITERAL(65, 1037, 16), // "OnDeleteAllLoads"
QT_MOC_LITERAL(66, 1054, 13), // "OnDeleteAllIC"
QT_MOC_LITERAL(67, 1068, 18), // "OnDeleteAllContact"
QT_MOC_LITERAL(68, 1087, 27), // "OnDeleteAllRigidConstraints"
QT_MOC_LITERAL(69, 1115, 26), // "OnDeleteAllRigidConnectors"
QT_MOC_LITERAL(70, 1142, 16) // "OnDeleteAllSteps"

    },
    "CModelViewer\0currentObjectChanged\0\0"
    "FSObject*\0po\0on_modelTree_currentItemChanged\0"
    "QTreeWidgetItem*\0current\0prev\0"
    "on_selectButton_clicked\0on_deleteButton_clicked\0"
    "on_searchButton_toggled\0b\0"
    "on_syncButton_clicked\0on_props_nameChanged\0"
    "txt\0on_props_selectionChanged\0"
    "on_props_dataChanged\0OnDeleteItem\0"
    "OnAddMaterial\0OnUnhideAllObjects\0"
    "OnUnhideAllParts\0OnAddBC\0OnAddSurfaceLoad\0"
    "OnAddBodyLoad\0OnAddInitialCondition\0"
    "OnAddContact\0OnAddRigidConstraint\0"
    "OnAddRigidConnector\0OnAddStep\0"
    "OnHideObject\0OnShowObject\0OnSelectObject\0"
    "OnSelectDiscreteObject\0OnDetachDiscreteObject\0"
    "OnHidePart\0OnShowPart\0OnSelectPart\0"
    "OnSelectSurface\0OnSelectCurve\0"
    "OnSelectNode\0OnCopyMaterial\0OnCopyInterface\0"
    "OnCopyBC\0OnCopyIC\0OnCopyRigidConnector\0"
    "OnCopyLoad\0OnCopyRigidConstraint\0"
    "OnCopyStep\0OnRerunJob\0OnEditOutput\0"
    "OnEditOutputLog\0OnRemoveEmptySelections\0"
    "OnRemoveAllSelections\0OnChangeMaterial\0"
    "OnMaterialHideParts\0OnMaterialShowParts\0"
    "OnMaterialHideOtherParts\0OnExportMaterials\0"
    "OnExportAllMaterials\0OnImportMaterials\0"
    "OnDeleteAllMaterials\0OnSwapMasterSlave\0"
    "OnGenerateMap\0OnDeleteAllBC\0"
    "OnDeleteAllLoads\0OnDeleteAllIC\0"
    "OnDeleteAllContact\0OnDeleteAllRigidConstraints\0"
    "OnDeleteAllRigidConnectors\0OnDeleteAllSteps"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CModelViewer[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      62,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,  324,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    2,  327,    2, 0x08 /* Private */,
       9,    0,  332,    2, 0x08 /* Private */,
      10,    0,  333,    2, 0x08 /* Private */,
      11,    1,  334,    2, 0x08 /* Private */,
      13,    0,  337,    2, 0x08 /* Private */,
      14,    1,  338,    2, 0x08 /* Private */,
      16,    0,  341,    2, 0x08 /* Private */,
      17,    1,  342,    2, 0x08 /* Private */,
      18,    0,  345,    2, 0x08 /* Private */,
      19,    0,  346,    2, 0x08 /* Private */,
      20,    0,  347,    2, 0x08 /* Private */,
      21,    0,  348,    2, 0x08 /* Private */,
      22,    0,  349,    2, 0x08 /* Private */,
      23,    0,  350,    2, 0x08 /* Private */,
      24,    0,  351,    2, 0x08 /* Private */,
      25,    0,  352,    2, 0x08 /* Private */,
      26,    0,  353,    2, 0x08 /* Private */,
      27,    0,  354,    2, 0x08 /* Private */,
      28,    0,  355,    2, 0x08 /* Private */,
      29,    0,  356,    2, 0x08 /* Private */,
      30,    0,  357,    2, 0x08 /* Private */,
      31,    0,  358,    2, 0x08 /* Private */,
      32,    0,  359,    2, 0x08 /* Private */,
      33,    0,  360,    2, 0x08 /* Private */,
      34,    0,  361,    2, 0x08 /* Private */,
      35,    0,  362,    2, 0x08 /* Private */,
      36,    0,  363,    2, 0x08 /* Private */,
      37,    0,  364,    2, 0x08 /* Private */,
      38,    0,  365,    2, 0x08 /* Private */,
      39,    0,  366,    2, 0x08 /* Private */,
      40,    0,  367,    2, 0x08 /* Private */,
      41,    0,  368,    2, 0x08 /* Private */,
      42,    0,  369,    2, 0x08 /* Private */,
      43,    0,  370,    2, 0x08 /* Private */,
      44,    0,  371,    2, 0x08 /* Private */,
      45,    0,  372,    2, 0x08 /* Private */,
      46,    0,  373,    2, 0x08 /* Private */,
      47,    0,  374,    2, 0x08 /* Private */,
      48,    0,  375,    2, 0x08 /* Private */,
      49,    0,  376,    2, 0x08 /* Private */,
      50,    0,  377,    2, 0x08 /* Private */,
      51,    0,  378,    2, 0x08 /* Private */,
      52,    0,  379,    2, 0x08 /* Private */,
      53,    0,  380,    2, 0x08 /* Private */,
      54,    0,  381,    2, 0x08 /* Private */,
      55,    0,  382,    2, 0x08 /* Private */,
      56,    0,  383,    2, 0x08 /* Private */,
      57,    0,  384,    2, 0x08 /* Private */,
      58,    0,  385,    2, 0x08 /* Private */,
      59,    0,  386,    2, 0x08 /* Private */,
      60,    0,  387,    2, 0x08 /* Private */,
      61,    0,  388,    2, 0x08 /* Private */,
      62,    0,  389,    2, 0x08 /* Private */,
      63,    0,  390,    2, 0x08 /* Private */,
      64,    0,  391,    2, 0x08 /* Private */,
      65,    0,  392,    2, 0x08 /* Private */,
      66,    0,  393,    2, 0x08 /* Private */,
      67,    0,  394,    2, 0x08 /* Private */,
      68,    0,  395,    2, 0x08 /* Private */,
      69,    0,  396,    2, 0x08 /* Private */,
      70,    0,  397,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 6, 0x80000000 | 6,    7,    8,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   12,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   15,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   12,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CModelViewer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CModelViewer *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->currentObjectChanged((*reinterpret_cast< FSObject*(*)>(_a[1]))); break;
        case 1: _t->on_modelTree_currentItemChanged((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< QTreeWidgetItem*(*)>(_a[2]))); break;
        case 2: _t->on_selectButton_clicked(); break;
        case 3: _t->on_deleteButton_clicked(); break;
        case 4: _t->on_searchButton_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->on_syncButton_clicked(); break;
        case 6: _t->on_props_nameChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->on_props_selectionChanged(); break;
        case 8: _t->on_props_dataChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: _t->OnDeleteItem(); break;
        case 10: _t->OnAddMaterial(); break;
        case 11: _t->OnUnhideAllObjects(); break;
        case 12: _t->OnUnhideAllParts(); break;
        case 13: _t->OnAddBC(); break;
        case 14: _t->OnAddSurfaceLoad(); break;
        case 15: _t->OnAddBodyLoad(); break;
        case 16: _t->OnAddInitialCondition(); break;
        case 17: _t->OnAddContact(); break;
        case 18: _t->OnAddRigidConstraint(); break;
        case 19: _t->OnAddRigidConnector(); break;
        case 20: _t->OnAddStep(); break;
        case 21: _t->OnHideObject(); break;
        case 22: _t->OnShowObject(); break;
        case 23: _t->OnSelectObject(); break;
        case 24: _t->OnSelectDiscreteObject(); break;
        case 25: _t->OnDetachDiscreteObject(); break;
        case 26: _t->OnHidePart(); break;
        case 27: _t->OnShowPart(); break;
        case 28: _t->OnSelectPart(); break;
        case 29: _t->OnSelectSurface(); break;
        case 30: _t->OnSelectCurve(); break;
        case 31: _t->OnSelectNode(); break;
        case 32: _t->OnCopyMaterial(); break;
        case 33: _t->OnCopyInterface(); break;
        case 34: _t->OnCopyBC(); break;
        case 35: _t->OnCopyIC(); break;
        case 36: _t->OnCopyRigidConnector(); break;
        case 37: _t->OnCopyLoad(); break;
        case 38: _t->OnCopyRigidConstraint(); break;
        case 39: _t->OnCopyStep(); break;
        case 40: _t->OnRerunJob(); break;
        case 41: _t->OnEditOutput(); break;
        case 42: _t->OnEditOutputLog(); break;
        case 43: _t->OnRemoveEmptySelections(); break;
        case 44: _t->OnRemoveAllSelections(); break;
        case 45: _t->OnChangeMaterial(); break;
        case 46: _t->OnMaterialHideParts(); break;
        case 47: _t->OnMaterialShowParts(); break;
        case 48: _t->OnMaterialHideOtherParts(); break;
        case 49: _t->OnExportMaterials(); break;
        case 50: _t->OnExportAllMaterials(); break;
        case 51: _t->OnImportMaterials(); break;
        case 52: _t->OnDeleteAllMaterials(); break;
        case 53: _t->OnSwapMasterSlave(); break;
        case 54: _t->OnGenerateMap(); break;
        case 55: _t->OnDeleteAllBC(); break;
        case 56: _t->OnDeleteAllLoads(); break;
        case 57: _t->OnDeleteAllIC(); break;
        case 58: _t->OnDeleteAllContact(); break;
        case 59: _t->OnDeleteAllRigidConstraints(); break;
        case 60: _t->OnDeleteAllRigidConnectors(); break;
        case 61: _t->OnDeleteAllSteps(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CModelViewer::*)(FSObject * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CModelViewer::currentObjectChanged)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CModelViewer::staticMetaObject = { {
    &CCommandPanel::staticMetaObject,
    qt_meta_stringdata_CModelViewer.data,
    qt_meta_data_CModelViewer,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CModelViewer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CModelViewer::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CModelViewer.stringdata0))
        return static_cast<void*>(this);
    return CCommandPanel::qt_metacast(_clname);
}

int CModelViewer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CCommandPanel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 62)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 62;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 62)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 62;
    }
    return _id;
}

// SIGNAL 0
void CModelViewer::currentObjectChanged(FSObject * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
