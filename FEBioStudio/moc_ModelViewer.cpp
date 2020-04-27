/****************************************************************************
** Meta object code from reading C++ file 'ModelViewer.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "ModelViewer.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ModelViewer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CModelViewer_t {
    QByteArrayData data[78];
    char stringdata0[1280];
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
QT_MOC_LITERAL(9, 110, 30), // "on_modelTree_itemDoubleClicked"
QT_MOC_LITERAL(10, 141, 4), // "item"
QT_MOC_LITERAL(11, 146, 6), // "column"
QT_MOC_LITERAL(12, 153, 23), // "on_selectButton_clicked"
QT_MOC_LITERAL(13, 177, 23), // "on_deleteButton_clicked"
QT_MOC_LITERAL(14, 201, 23), // "on_searchButton_toggled"
QT_MOC_LITERAL(15, 225, 1), // "b"
QT_MOC_LITERAL(16, 227, 21), // "on_syncButton_clicked"
QT_MOC_LITERAL(17, 249, 20), // "on_props_nameChanged"
QT_MOC_LITERAL(18, 270, 3), // "txt"
QT_MOC_LITERAL(19, 274, 25), // "on_props_selectionChanged"
QT_MOC_LITERAL(20, 300, 20), // "on_props_dataChanged"
QT_MOC_LITERAL(21, 321, 12), // "OnDeleteItem"
QT_MOC_LITERAL(22, 334, 13), // "OnAddMaterial"
QT_MOC_LITERAL(23, 348, 18), // "OnUnhideAllObjects"
QT_MOC_LITERAL(24, 367, 20), // "OnCreateNewMeshLayer"
QT_MOC_LITERAL(25, 388, 17), // "OnDeleteMeshLayer"
QT_MOC_LITERAL(26, 406, 16), // "OnUnhideAllParts"
QT_MOC_LITERAL(27, 423, 7), // "OnAddBC"
QT_MOC_LITERAL(28, 431, 16), // "OnAddSurfaceLoad"
QT_MOC_LITERAL(29, 448, 13), // "OnAddBodyLoad"
QT_MOC_LITERAL(30, 462, 21), // "OnAddInitialCondition"
QT_MOC_LITERAL(31, 484, 12), // "OnAddContact"
QT_MOC_LITERAL(32, 497, 15), // "OnAddConstraint"
QT_MOC_LITERAL(33, 513, 20), // "OnAddRigidConstraint"
QT_MOC_LITERAL(34, 534, 19), // "OnAddRigidConnector"
QT_MOC_LITERAL(35, 554, 9), // "OnAddStep"
QT_MOC_LITERAL(36, 564, 12), // "OnHideObject"
QT_MOC_LITERAL(37, 577, 12), // "OnShowObject"
QT_MOC_LITERAL(38, 590, 14), // "OnSelectObject"
QT_MOC_LITERAL(39, 605, 22), // "OnSelectDiscreteObject"
QT_MOC_LITERAL(40, 628, 22), // "OnDetachDiscreteObject"
QT_MOC_LITERAL(41, 651, 10), // "OnHidePart"
QT_MOC_LITERAL(42, 662, 10), // "OnShowPart"
QT_MOC_LITERAL(43, 673, 12), // "OnSelectPart"
QT_MOC_LITERAL(44, 686, 15), // "OnSelectSurface"
QT_MOC_LITERAL(45, 702, 13), // "OnSelectCurve"
QT_MOC_LITERAL(46, 716, 12), // "OnSelectNode"
QT_MOC_LITERAL(47, 729, 14), // "OnCopyMaterial"
QT_MOC_LITERAL(48, 744, 15), // "OnCopyInterface"
QT_MOC_LITERAL(49, 760, 8), // "OnCopyBC"
QT_MOC_LITERAL(50, 769, 8), // "OnCopyIC"
QT_MOC_LITERAL(51, 778, 20), // "OnCopyRigidConnector"
QT_MOC_LITERAL(52, 799, 10), // "OnCopyLoad"
QT_MOC_LITERAL(53, 810, 21), // "OnCopyRigidConstraint"
QT_MOC_LITERAL(54, 832, 10), // "OnCopyStep"
QT_MOC_LITERAL(55, 843, 10), // "OnRerunJob"
QT_MOC_LITERAL(56, 854, 12), // "OnEditOutput"
QT_MOC_LITERAL(57, 867, 15), // "OnEditOutputLog"
QT_MOC_LITERAL(58, 883, 23), // "OnRemoveEmptySelections"
QT_MOC_LITERAL(59, 907, 21), // "OnRemoveAllSelections"
QT_MOC_LITERAL(60, 929, 16), // "OnChangeMaterial"
QT_MOC_LITERAL(61, 946, 19), // "OnMaterialHideParts"
QT_MOC_LITERAL(62, 966, 19), // "OnMaterialShowParts"
QT_MOC_LITERAL(63, 986, 24), // "OnMaterialHideOtherParts"
QT_MOC_LITERAL(64, 1011, 17), // "OnExportMaterials"
QT_MOC_LITERAL(65, 1029, 20), // "OnExportAllMaterials"
QT_MOC_LITERAL(66, 1050, 17), // "OnImportMaterials"
QT_MOC_LITERAL(67, 1068, 20), // "OnDeleteAllMaterials"
QT_MOC_LITERAL(68, 1089, 17), // "OnSwapMasterSlave"
QT_MOC_LITERAL(69, 1107, 13), // "OnGenerateMap"
QT_MOC_LITERAL(70, 1121, 13), // "OnDeleteAllBC"
QT_MOC_LITERAL(71, 1135, 16), // "OnDeleteAllLoads"
QT_MOC_LITERAL(72, 1152, 13), // "OnDeleteAllIC"
QT_MOC_LITERAL(73, 1166, 18), // "OnDeleteAllContact"
QT_MOC_LITERAL(74, 1185, 22), // "OnDeleteAllConstraints"
QT_MOC_LITERAL(75, 1208, 27), // "OnDeleteAllRigidConstraints"
QT_MOC_LITERAL(76, 1236, 26), // "OnDeleteAllRigidConnectors"
QT_MOC_LITERAL(77, 1263, 16) // "OnDeleteAllSteps"

    },
    "CModelViewer\0currentObjectChanged\0\0"
    "FSObject*\0po\0on_modelTree_currentItemChanged\0"
    "QTreeWidgetItem*\0current\0prev\0"
    "on_modelTree_itemDoubleClicked\0item\0"
    "column\0on_selectButton_clicked\0"
    "on_deleteButton_clicked\0on_searchButton_toggled\0"
    "b\0on_syncButton_clicked\0on_props_nameChanged\0"
    "txt\0on_props_selectionChanged\0"
    "on_props_dataChanged\0OnDeleteItem\0"
    "OnAddMaterial\0OnUnhideAllObjects\0"
    "OnCreateNewMeshLayer\0OnDeleteMeshLayer\0"
    "OnUnhideAllParts\0OnAddBC\0OnAddSurfaceLoad\0"
    "OnAddBodyLoad\0OnAddInitialCondition\0"
    "OnAddContact\0OnAddConstraint\0"
    "OnAddRigidConstraint\0OnAddRigidConnector\0"
    "OnAddStep\0OnHideObject\0OnShowObject\0"
    "OnSelectObject\0OnSelectDiscreteObject\0"
    "OnDetachDiscreteObject\0OnHidePart\0"
    "OnShowPart\0OnSelectPart\0OnSelectSurface\0"
    "OnSelectCurve\0OnSelectNode\0OnCopyMaterial\0"
    "OnCopyInterface\0OnCopyBC\0OnCopyIC\0"
    "OnCopyRigidConnector\0OnCopyLoad\0"
    "OnCopyRigidConstraint\0OnCopyStep\0"
    "OnRerunJob\0OnEditOutput\0OnEditOutputLog\0"
    "OnRemoveEmptySelections\0OnRemoveAllSelections\0"
    "OnChangeMaterial\0OnMaterialHideParts\0"
    "OnMaterialShowParts\0OnMaterialHideOtherParts\0"
    "OnExportMaterials\0OnExportAllMaterials\0"
    "OnImportMaterials\0OnDeleteAllMaterials\0"
    "OnSwapMasterSlave\0OnGenerateMap\0"
    "OnDeleteAllBC\0OnDeleteAllLoads\0"
    "OnDeleteAllIC\0OnDeleteAllContact\0"
    "OnDeleteAllConstraints\0"
    "OnDeleteAllRigidConstraints\0"
    "OnDeleteAllRigidConnectors\0OnDeleteAllSteps"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CModelViewer[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      67,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,  349,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    2,  352,    2, 0x08 /* Private */,
       9,    2,  357,    2, 0x08 /* Private */,
      12,    0,  362,    2, 0x08 /* Private */,
      13,    0,  363,    2, 0x08 /* Private */,
      14,    1,  364,    2, 0x08 /* Private */,
      16,    0,  367,    2, 0x08 /* Private */,
      17,    1,  368,    2, 0x08 /* Private */,
      19,    0,  371,    2, 0x08 /* Private */,
      20,    1,  372,    2, 0x08 /* Private */,
      21,    0,  375,    2, 0x0a /* Public */,
      22,    0,  376,    2, 0x0a /* Public */,
      23,    0,  377,    2, 0x0a /* Public */,
      24,    0,  378,    2, 0x0a /* Public */,
      25,    0,  379,    2, 0x0a /* Public */,
      26,    0,  380,    2, 0x0a /* Public */,
      27,    0,  381,    2, 0x0a /* Public */,
      28,    0,  382,    2, 0x0a /* Public */,
      29,    0,  383,    2, 0x0a /* Public */,
      30,    0,  384,    2, 0x0a /* Public */,
      31,    0,  385,    2, 0x0a /* Public */,
      32,    0,  386,    2, 0x0a /* Public */,
      33,    0,  387,    2, 0x0a /* Public */,
      34,    0,  388,    2, 0x0a /* Public */,
      35,    0,  389,    2, 0x0a /* Public */,
      36,    0,  390,    2, 0x0a /* Public */,
      37,    0,  391,    2, 0x0a /* Public */,
      38,    0,  392,    2, 0x0a /* Public */,
      39,    0,  393,    2, 0x0a /* Public */,
      40,    0,  394,    2, 0x0a /* Public */,
      41,    0,  395,    2, 0x0a /* Public */,
      42,    0,  396,    2, 0x0a /* Public */,
      43,    0,  397,    2, 0x0a /* Public */,
      44,    0,  398,    2, 0x0a /* Public */,
      45,    0,  399,    2, 0x0a /* Public */,
      46,    0,  400,    2, 0x0a /* Public */,
      47,    0,  401,    2, 0x0a /* Public */,
      48,    0,  402,    2, 0x0a /* Public */,
      49,    0,  403,    2, 0x0a /* Public */,
      50,    0,  404,    2, 0x0a /* Public */,
      51,    0,  405,    2, 0x0a /* Public */,
      52,    0,  406,    2, 0x0a /* Public */,
      53,    0,  407,    2, 0x0a /* Public */,
      54,    0,  408,    2, 0x0a /* Public */,
      55,    0,  409,    2, 0x0a /* Public */,
      56,    0,  410,    2, 0x0a /* Public */,
      57,    0,  411,    2, 0x0a /* Public */,
      58,    0,  412,    2, 0x0a /* Public */,
      59,    0,  413,    2, 0x0a /* Public */,
      60,    0,  414,    2, 0x0a /* Public */,
      61,    0,  415,    2, 0x0a /* Public */,
      62,    0,  416,    2, 0x0a /* Public */,
      63,    0,  417,    2, 0x0a /* Public */,
      64,    0,  418,    2, 0x0a /* Public */,
      65,    0,  419,    2, 0x0a /* Public */,
      66,    0,  420,    2, 0x0a /* Public */,
      67,    0,  421,    2, 0x0a /* Public */,
      68,    0,  422,    2, 0x0a /* Public */,
      69,    0,  423,    2, 0x0a /* Public */,
      70,    0,  424,    2, 0x0a /* Public */,
      71,    0,  425,    2, 0x0a /* Public */,
      72,    0,  426,    2, 0x0a /* Public */,
      73,    0,  427,    2, 0x0a /* Public */,
      74,    0,  428,    2, 0x0a /* Public */,
      75,    0,  429,    2, 0x0a /* Public */,
      76,    0,  430,    2, 0x0a /* Public */,
      77,    0,  431,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 6, 0x80000000 | 6,    7,    8,
    QMetaType::Void, 0x80000000 | 6, QMetaType::Int,   10,   11,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   15,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   18,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   15,
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
        case 2: _t->on_modelTree_itemDoubleClicked((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->on_selectButton_clicked(); break;
        case 4: _t->on_deleteButton_clicked(); break;
        case 5: _t->on_searchButton_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 6: _t->on_syncButton_clicked(); break;
        case 7: _t->on_props_nameChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->on_props_selectionChanged(); break;
        case 9: _t->on_props_dataChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 10: _t->OnDeleteItem(); break;
        case 11: _t->OnAddMaterial(); break;
        case 12: _t->OnUnhideAllObjects(); break;
        case 13: _t->OnCreateNewMeshLayer(); break;
        case 14: _t->OnDeleteMeshLayer(); break;
        case 15: _t->OnUnhideAllParts(); break;
        case 16: _t->OnAddBC(); break;
        case 17: _t->OnAddSurfaceLoad(); break;
        case 18: _t->OnAddBodyLoad(); break;
        case 19: _t->OnAddInitialCondition(); break;
        case 20: _t->OnAddContact(); break;
        case 21: _t->OnAddConstraint(); break;
        case 22: _t->OnAddRigidConstraint(); break;
        case 23: _t->OnAddRigidConnector(); break;
        case 24: _t->OnAddStep(); break;
        case 25: _t->OnHideObject(); break;
        case 26: _t->OnShowObject(); break;
        case 27: _t->OnSelectObject(); break;
        case 28: _t->OnSelectDiscreteObject(); break;
        case 29: _t->OnDetachDiscreteObject(); break;
        case 30: _t->OnHidePart(); break;
        case 31: _t->OnShowPart(); break;
        case 32: _t->OnSelectPart(); break;
        case 33: _t->OnSelectSurface(); break;
        case 34: _t->OnSelectCurve(); break;
        case 35: _t->OnSelectNode(); break;
        case 36: _t->OnCopyMaterial(); break;
        case 37: _t->OnCopyInterface(); break;
        case 38: _t->OnCopyBC(); break;
        case 39: _t->OnCopyIC(); break;
        case 40: _t->OnCopyRigidConnector(); break;
        case 41: _t->OnCopyLoad(); break;
        case 42: _t->OnCopyRigidConstraint(); break;
        case 43: _t->OnCopyStep(); break;
        case 44: _t->OnRerunJob(); break;
        case 45: _t->OnEditOutput(); break;
        case 46: _t->OnEditOutputLog(); break;
        case 47: _t->OnRemoveEmptySelections(); break;
        case 48: _t->OnRemoveAllSelections(); break;
        case 49: _t->OnChangeMaterial(); break;
        case 50: _t->OnMaterialHideParts(); break;
        case 51: _t->OnMaterialShowParts(); break;
        case 52: _t->OnMaterialHideOtherParts(); break;
        case 53: _t->OnExportMaterials(); break;
        case 54: _t->OnExportAllMaterials(); break;
        case 55: _t->OnImportMaterials(); break;
        case 56: _t->OnDeleteAllMaterials(); break;
        case 57: _t->OnSwapMasterSlave(); break;
        case 58: _t->OnGenerateMap(); break;
        case 59: _t->OnDeleteAllBC(); break;
        case 60: _t->OnDeleteAllLoads(); break;
        case 61: _t->OnDeleteAllIC(); break;
        case 62: _t->OnDeleteAllContact(); break;
        case 63: _t->OnDeleteAllConstraints(); break;
        case 64: _t->OnDeleteAllRigidConstraints(); break;
        case 65: _t->OnDeleteAllRigidConnectors(); break;
        case 66: _t->OnDeleteAllSteps(); break;
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
    QMetaObject::SuperData::link<CCommandPanel::staticMetaObject>(),
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
        if (_id < 67)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 67;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 67)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 67;
    }
    return _id;
}

// SIGNAL 0
void CModelViewer::currentObjectChanged(FSObject * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
