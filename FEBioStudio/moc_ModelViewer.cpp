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
    QByteArrayData data[84];
    char stringdata0[1374];
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
QT_MOC_LITERAL(21, 321, 29), // "on_filter_currentIndexChanged"
QT_MOC_LITERAL(22, 351, 1), // "n"
QT_MOC_LITERAL(23, 353, 12), // "OnDeleteItem"
QT_MOC_LITERAL(24, 366, 13), // "OnAddMaterial"
QT_MOC_LITERAL(25, 380, 18), // "OnUnhideAllObjects"
QT_MOC_LITERAL(26, 399, 20), // "OnCreateNewMeshLayer"
QT_MOC_LITERAL(27, 420, 17), // "OnDeleteMeshLayer"
QT_MOC_LITERAL(28, 438, 16), // "OnUnhideAllParts"
QT_MOC_LITERAL(29, 455, 7), // "OnAddBC"
QT_MOC_LITERAL(30, 463, 16), // "OnAddSurfaceLoad"
QT_MOC_LITERAL(31, 480, 13), // "OnAddBodyLoad"
QT_MOC_LITERAL(32, 494, 21), // "OnAddInitialCondition"
QT_MOC_LITERAL(33, 516, 12), // "OnAddContact"
QT_MOC_LITERAL(34, 529, 15), // "OnAddConstraint"
QT_MOC_LITERAL(35, 545, 20), // "OnAddRigidConstraint"
QT_MOC_LITERAL(36, 566, 19), // "OnAddRigidConnector"
QT_MOC_LITERAL(37, 586, 9), // "OnAddStep"
QT_MOC_LITERAL(38, 596, 12), // "OnHideObject"
QT_MOC_LITERAL(39, 609, 12), // "OnShowObject"
QT_MOC_LITERAL(40, 622, 14), // "OnSelectObject"
QT_MOC_LITERAL(41, 637, 22), // "OnSelectDiscreteObject"
QT_MOC_LITERAL(42, 660, 22), // "OnDetachDiscreteObject"
QT_MOC_LITERAL(43, 683, 10), // "OnHidePart"
QT_MOC_LITERAL(44, 694, 20), // "OnSelectPartElements"
QT_MOC_LITERAL(45, 715, 10), // "OnShowPart"
QT_MOC_LITERAL(46, 726, 12), // "OnSelectPart"
QT_MOC_LITERAL(47, 739, 15), // "OnSelectSurface"
QT_MOC_LITERAL(48, 755, 13), // "OnSelectCurve"
QT_MOC_LITERAL(49, 769, 12), // "OnSelectNode"
QT_MOC_LITERAL(50, 782, 14), // "OnCopyMaterial"
QT_MOC_LITERAL(51, 797, 15), // "OnCopyInterface"
QT_MOC_LITERAL(52, 813, 8), // "OnCopyBC"
QT_MOC_LITERAL(53, 822, 8), // "OnCopyIC"
QT_MOC_LITERAL(54, 831, 20), // "OnCopyRigidConnector"
QT_MOC_LITERAL(55, 852, 10), // "OnCopyLoad"
QT_MOC_LITERAL(56, 863, 16), // "OnCopyConstraint"
QT_MOC_LITERAL(57, 880, 21), // "OnCopyRigidConstraint"
QT_MOC_LITERAL(58, 902, 10), // "OnCopyStep"
QT_MOC_LITERAL(59, 913, 12), // "OnStepMoveUp"
QT_MOC_LITERAL(60, 926, 14), // "OnStepMoveDown"
QT_MOC_LITERAL(61, 941, 10), // "OnRerunJob"
QT_MOC_LITERAL(62, 952, 9), // "OnOpenJob"
QT_MOC_LITERAL(63, 962, 12), // "OnEditOutput"
QT_MOC_LITERAL(64, 975, 15), // "OnEditOutputLog"
QT_MOC_LITERAL(65, 991, 23), // "OnRemoveEmptySelections"
QT_MOC_LITERAL(66, 1015, 21), // "OnRemoveAllSelections"
QT_MOC_LITERAL(67, 1037, 16), // "OnChangeMaterial"
QT_MOC_LITERAL(68, 1054, 19), // "OnMaterialHideParts"
QT_MOC_LITERAL(69, 1074, 19), // "OnMaterialShowParts"
QT_MOC_LITERAL(70, 1094, 24), // "OnMaterialHideOtherParts"
QT_MOC_LITERAL(71, 1119, 17), // "OnExportMaterials"
QT_MOC_LITERAL(72, 1137, 20), // "OnExportAllMaterials"
QT_MOC_LITERAL(73, 1158, 17), // "OnImportMaterials"
QT_MOC_LITERAL(74, 1176, 20), // "OnDeleteAllMaterials"
QT_MOC_LITERAL(75, 1197, 17), // "OnSwapMasterSlave"
QT_MOC_LITERAL(76, 1215, 13), // "OnDeleteAllBC"
QT_MOC_LITERAL(77, 1229, 16), // "OnDeleteAllLoads"
QT_MOC_LITERAL(78, 1246, 13), // "OnDeleteAllIC"
QT_MOC_LITERAL(79, 1260, 18), // "OnDeleteAllContact"
QT_MOC_LITERAL(80, 1279, 22), // "OnDeleteAllConstraints"
QT_MOC_LITERAL(81, 1302, 27), // "OnDeleteAllRigidConstraints"
QT_MOC_LITERAL(82, 1330, 26), // "OnDeleteAllRigidConnectors"
QT_MOC_LITERAL(83, 1357, 16) // "OnDeleteAllSteps"

    },
    "CModelViewer\0currentObjectChanged\0\0"
    "FSObject*\0po\0on_modelTree_currentItemChanged\0"
    "QTreeWidgetItem*\0current\0prev\0"
    "on_modelTree_itemDoubleClicked\0item\0"
    "column\0on_selectButton_clicked\0"
    "on_deleteButton_clicked\0on_searchButton_toggled\0"
    "b\0on_syncButton_clicked\0on_props_nameChanged\0"
    "txt\0on_props_selectionChanged\0"
    "on_props_dataChanged\0on_filter_currentIndexChanged\0"
    "n\0OnDeleteItem\0OnAddMaterial\0"
    "OnUnhideAllObjects\0OnCreateNewMeshLayer\0"
    "OnDeleteMeshLayer\0OnUnhideAllParts\0"
    "OnAddBC\0OnAddSurfaceLoad\0OnAddBodyLoad\0"
    "OnAddInitialCondition\0OnAddContact\0"
    "OnAddConstraint\0OnAddRigidConstraint\0"
    "OnAddRigidConnector\0OnAddStep\0"
    "OnHideObject\0OnShowObject\0OnSelectObject\0"
    "OnSelectDiscreteObject\0OnDetachDiscreteObject\0"
    "OnHidePart\0OnSelectPartElements\0"
    "OnShowPart\0OnSelectPart\0OnSelectSurface\0"
    "OnSelectCurve\0OnSelectNode\0OnCopyMaterial\0"
    "OnCopyInterface\0OnCopyBC\0OnCopyIC\0"
    "OnCopyRigidConnector\0OnCopyLoad\0"
    "OnCopyConstraint\0OnCopyRigidConstraint\0"
    "OnCopyStep\0OnStepMoveUp\0OnStepMoveDown\0"
    "OnRerunJob\0OnOpenJob\0OnEditOutput\0"
    "OnEditOutputLog\0OnRemoveEmptySelections\0"
    "OnRemoveAllSelections\0OnChangeMaterial\0"
    "OnMaterialHideParts\0OnMaterialShowParts\0"
    "OnMaterialHideOtherParts\0OnExportMaterials\0"
    "OnExportAllMaterials\0OnImportMaterials\0"
    "OnDeleteAllMaterials\0OnSwapMasterSlave\0"
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
      72,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,  374,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    2,  377,    2, 0x08 /* Private */,
       9,    2,  382,    2, 0x08 /* Private */,
      12,    0,  387,    2, 0x08 /* Private */,
      13,    0,  388,    2, 0x08 /* Private */,
      14,    1,  389,    2, 0x08 /* Private */,
      16,    0,  392,    2, 0x08 /* Private */,
      17,    1,  393,    2, 0x08 /* Private */,
      19,    0,  396,    2, 0x08 /* Private */,
      20,    1,  397,    2, 0x08 /* Private */,
      21,    1,  400,    2, 0x08 /* Private */,
      23,    0,  403,    2, 0x0a /* Public */,
      24,    0,  404,    2, 0x0a /* Public */,
      25,    0,  405,    2, 0x0a /* Public */,
      26,    0,  406,    2, 0x0a /* Public */,
      27,    0,  407,    2, 0x0a /* Public */,
      28,    0,  408,    2, 0x0a /* Public */,
      29,    0,  409,    2, 0x0a /* Public */,
      30,    0,  410,    2, 0x0a /* Public */,
      31,    0,  411,    2, 0x0a /* Public */,
      32,    0,  412,    2, 0x0a /* Public */,
      33,    0,  413,    2, 0x0a /* Public */,
      34,    0,  414,    2, 0x0a /* Public */,
      35,    0,  415,    2, 0x0a /* Public */,
      36,    0,  416,    2, 0x0a /* Public */,
      37,    0,  417,    2, 0x0a /* Public */,
      38,    0,  418,    2, 0x0a /* Public */,
      39,    0,  419,    2, 0x0a /* Public */,
      40,    0,  420,    2, 0x0a /* Public */,
      41,    0,  421,    2, 0x0a /* Public */,
      42,    0,  422,    2, 0x0a /* Public */,
      43,    0,  423,    2, 0x0a /* Public */,
      44,    0,  424,    2, 0x0a /* Public */,
      45,    0,  425,    2, 0x0a /* Public */,
      46,    0,  426,    2, 0x0a /* Public */,
      47,    0,  427,    2, 0x0a /* Public */,
      48,    0,  428,    2, 0x0a /* Public */,
      49,    0,  429,    2, 0x0a /* Public */,
      50,    0,  430,    2, 0x0a /* Public */,
      51,    0,  431,    2, 0x0a /* Public */,
      52,    0,  432,    2, 0x0a /* Public */,
      53,    0,  433,    2, 0x0a /* Public */,
      54,    0,  434,    2, 0x0a /* Public */,
      55,    0,  435,    2, 0x0a /* Public */,
      56,    0,  436,    2, 0x0a /* Public */,
      57,    0,  437,    2, 0x0a /* Public */,
      58,    0,  438,    2, 0x0a /* Public */,
      59,    0,  439,    2, 0x0a /* Public */,
      60,    0,  440,    2, 0x0a /* Public */,
      61,    0,  441,    2, 0x0a /* Public */,
      62,    0,  442,    2, 0x0a /* Public */,
      63,    0,  443,    2, 0x0a /* Public */,
      64,    0,  444,    2, 0x0a /* Public */,
      65,    0,  445,    2, 0x0a /* Public */,
      66,    0,  446,    2, 0x0a /* Public */,
      67,    0,  447,    2, 0x0a /* Public */,
      68,    0,  448,    2, 0x0a /* Public */,
      69,    0,  449,    2, 0x0a /* Public */,
      70,    0,  450,    2, 0x0a /* Public */,
      71,    0,  451,    2, 0x0a /* Public */,
      72,    0,  452,    2, 0x0a /* Public */,
      73,    0,  453,    2, 0x0a /* Public */,
      74,    0,  454,    2, 0x0a /* Public */,
      75,    0,  455,    2, 0x0a /* Public */,
      76,    0,  456,    2, 0x0a /* Public */,
      77,    0,  457,    2, 0x0a /* Public */,
      78,    0,  458,    2, 0x0a /* Public */,
      79,    0,  459,    2, 0x0a /* Public */,
      80,    0,  460,    2, 0x0a /* Public */,
      81,    0,  461,    2, 0x0a /* Public */,
      82,    0,  462,    2, 0x0a /* Public */,
      83,    0,  463,    2, 0x0a /* Public */,

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
    QMetaType::Void, QMetaType::Int,   22,
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
        case 10: _t->on_filter_currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: _t->OnDeleteItem(); break;
        case 12: _t->OnAddMaterial(); break;
        case 13: _t->OnUnhideAllObjects(); break;
        case 14: _t->OnCreateNewMeshLayer(); break;
        case 15: _t->OnDeleteMeshLayer(); break;
        case 16: _t->OnUnhideAllParts(); break;
        case 17: _t->OnAddBC(); break;
        case 18: _t->OnAddSurfaceLoad(); break;
        case 19: _t->OnAddBodyLoad(); break;
        case 20: _t->OnAddInitialCondition(); break;
        case 21: _t->OnAddContact(); break;
        case 22: _t->OnAddConstraint(); break;
        case 23: _t->OnAddRigidConstraint(); break;
        case 24: _t->OnAddRigidConnector(); break;
        case 25: _t->OnAddStep(); break;
        case 26: _t->OnHideObject(); break;
        case 27: _t->OnShowObject(); break;
        case 28: _t->OnSelectObject(); break;
        case 29: _t->OnSelectDiscreteObject(); break;
        case 30: _t->OnDetachDiscreteObject(); break;
        case 31: _t->OnHidePart(); break;
        case 32: _t->OnSelectPartElements(); break;
        case 33: _t->OnShowPart(); break;
        case 34: _t->OnSelectPart(); break;
        case 35: _t->OnSelectSurface(); break;
        case 36: _t->OnSelectCurve(); break;
        case 37: _t->OnSelectNode(); break;
        case 38: _t->OnCopyMaterial(); break;
        case 39: _t->OnCopyInterface(); break;
        case 40: _t->OnCopyBC(); break;
        case 41: _t->OnCopyIC(); break;
        case 42: _t->OnCopyRigidConnector(); break;
        case 43: _t->OnCopyLoad(); break;
        case 44: _t->OnCopyConstraint(); break;
        case 45: _t->OnCopyRigidConstraint(); break;
        case 46: _t->OnCopyStep(); break;
        case 47: _t->OnStepMoveUp(); break;
        case 48: _t->OnStepMoveDown(); break;
        case 49: _t->OnRerunJob(); break;
        case 50: _t->OnOpenJob(); break;
        case 51: _t->OnEditOutput(); break;
        case 52: _t->OnEditOutputLog(); break;
        case 53: _t->OnRemoveEmptySelections(); break;
        case 54: _t->OnRemoveAllSelections(); break;
        case 55: _t->OnChangeMaterial(); break;
        case 56: _t->OnMaterialHideParts(); break;
        case 57: _t->OnMaterialShowParts(); break;
        case 58: _t->OnMaterialHideOtherParts(); break;
        case 59: _t->OnExportMaterials(); break;
        case 60: _t->OnExportAllMaterials(); break;
        case 61: _t->OnImportMaterials(); break;
        case 62: _t->OnDeleteAllMaterials(); break;
        case 63: _t->OnSwapMasterSlave(); break;
        case 64: _t->OnDeleteAllBC(); break;
        case 65: _t->OnDeleteAllLoads(); break;
        case 66: _t->OnDeleteAllIC(); break;
        case 67: _t->OnDeleteAllContact(); break;
        case 68: _t->OnDeleteAllConstraints(); break;
        case 69: _t->OnDeleteAllRigidConstraints(); break;
        case 70: _t->OnDeleteAllRigidConnectors(); break;
        case 71: _t->OnDeleteAllSteps(); break;
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
        if (_id < 72)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 72;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 72)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 72;
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
