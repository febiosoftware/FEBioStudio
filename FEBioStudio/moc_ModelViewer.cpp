/****************************************************************************
** Meta object code from reading C++ file 'ModelViewer.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "ModelViewer.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ModelViewer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CModelViewer_t {
    QByteArrayData data[64];
    char stringdata0[1019];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CModelViewer_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CModelViewer_t qt_meta_stringdata_CModelViewer = {
    {
QT_MOC_LITERAL(0, 0, 12), // "CModelViewer"
QT_MOC_LITERAL(1, 13, 31), // "on_modelTree_currentItemChanged"
QT_MOC_LITERAL(2, 45, 0), // ""
QT_MOC_LITERAL(3, 46, 16), // "QTreeWidgetItem*"
QT_MOC_LITERAL(4, 63, 7), // "current"
QT_MOC_LITERAL(5, 71, 4), // "prev"
QT_MOC_LITERAL(6, 76, 23), // "on_selectButton_clicked"
QT_MOC_LITERAL(7, 100, 23), // "on_deleteButton_clicked"
QT_MOC_LITERAL(8, 124, 23), // "on_searchButton_toggled"
QT_MOC_LITERAL(9, 148, 1), // "b"
QT_MOC_LITERAL(10, 150, 21), // "on_syncButton_clicked"
QT_MOC_LITERAL(11, 172, 20), // "on_props_nameChanged"
QT_MOC_LITERAL(12, 193, 3), // "txt"
QT_MOC_LITERAL(13, 197, 25), // "on_props_selectionChanged"
QT_MOC_LITERAL(14, 223, 20), // "on_props_dataChanged"
QT_MOC_LITERAL(15, 244, 12), // "OnDeleteItem"
QT_MOC_LITERAL(16, 257, 13), // "OnAddMaterial"
QT_MOC_LITERAL(17, 271, 18), // "OnUnhideAllObjects"
QT_MOC_LITERAL(18, 290, 16), // "OnUnhideAllParts"
QT_MOC_LITERAL(19, 307, 7), // "OnAddBC"
QT_MOC_LITERAL(20, 315, 16), // "OnAddSurfaceLoad"
QT_MOC_LITERAL(21, 332, 13), // "OnAddBodyLoad"
QT_MOC_LITERAL(22, 346, 21), // "OnAddInitialCondition"
QT_MOC_LITERAL(23, 368, 12), // "OnAddContact"
QT_MOC_LITERAL(24, 381, 15), // "OnAddConstraint"
QT_MOC_LITERAL(25, 397, 14), // "OnAddConnector"
QT_MOC_LITERAL(26, 412, 9), // "OnAddStep"
QT_MOC_LITERAL(27, 422, 12), // "OnHideObject"
QT_MOC_LITERAL(28, 435, 12), // "OnShowObject"
QT_MOC_LITERAL(29, 448, 14), // "OnSelectObject"
QT_MOC_LITERAL(30, 463, 22), // "OnSelectDiscreteObject"
QT_MOC_LITERAL(31, 486, 22), // "OnDetachDiscreteObject"
QT_MOC_LITERAL(32, 509, 10), // "OnHidePart"
QT_MOC_LITERAL(33, 520, 10), // "OnShowPart"
QT_MOC_LITERAL(34, 531, 12), // "OnSelectPart"
QT_MOC_LITERAL(35, 544, 15), // "OnSelectSurface"
QT_MOC_LITERAL(36, 560, 13), // "OnSelectCurve"
QT_MOC_LITERAL(37, 574, 12), // "OnSelectNode"
QT_MOC_LITERAL(38, 587, 14), // "OnCopyMaterial"
QT_MOC_LITERAL(39, 602, 15), // "OnCopyInterface"
QT_MOC_LITERAL(40, 618, 8), // "OnCopyBC"
QT_MOC_LITERAL(41, 627, 8), // "OnCopyIC"
QT_MOC_LITERAL(42, 636, 15), // "OnCopyConnector"
QT_MOC_LITERAL(43, 652, 10), // "OnCopyLoad"
QT_MOC_LITERAL(44, 663, 16), // "OnCopyConstraint"
QT_MOC_LITERAL(45, 680, 10), // "OnCopyStep"
QT_MOC_LITERAL(46, 691, 12), // "OnEditOutput"
QT_MOC_LITERAL(47, 704, 15), // "OnEditOutputLog"
QT_MOC_LITERAL(48, 720, 23), // "OnRemoveEmptySelections"
QT_MOC_LITERAL(49, 744, 21), // "OnRemoveAllSelections"
QT_MOC_LITERAL(50, 766, 16), // "OnChangeMaterial"
QT_MOC_LITERAL(51, 783, 17), // "OnExportMaterials"
QT_MOC_LITERAL(52, 801, 20), // "OnExportAllMaterials"
QT_MOC_LITERAL(53, 822, 17), // "OnImportMaterials"
QT_MOC_LITERAL(54, 840, 20), // "OnDeleteAllMaterials"
QT_MOC_LITERAL(55, 861, 17), // "OnSwapMasterSlave"
QT_MOC_LITERAL(56, 879, 13), // "OnGenerateMap"
QT_MOC_LITERAL(57, 893, 13), // "OnDeleteAllBC"
QT_MOC_LITERAL(58, 907, 16), // "OnDeleteAllLoads"
QT_MOC_LITERAL(59, 924, 13), // "OnDeleteAllIC"
QT_MOC_LITERAL(60, 938, 18), // "OnDeleteAllContact"
QT_MOC_LITERAL(61, 957, 22), // "OnDeleteAllConstraints"
QT_MOC_LITERAL(62, 980, 21), // "OnDeleteAllConnectors"
QT_MOC_LITERAL(63, 1002, 16) // "OnDeleteAllSteps"

    },
    "CModelViewer\0on_modelTree_currentItemChanged\0"
    "\0QTreeWidgetItem*\0current\0prev\0"
    "on_selectButton_clicked\0on_deleteButton_clicked\0"
    "on_searchButton_toggled\0b\0"
    "on_syncButton_clicked\0on_props_nameChanged\0"
    "txt\0on_props_selectionChanged\0"
    "on_props_dataChanged\0OnDeleteItem\0"
    "OnAddMaterial\0OnUnhideAllObjects\0"
    "OnUnhideAllParts\0OnAddBC\0OnAddSurfaceLoad\0"
    "OnAddBodyLoad\0OnAddInitialCondition\0"
    "OnAddContact\0OnAddConstraint\0"
    "OnAddConnector\0OnAddStep\0OnHideObject\0"
    "OnShowObject\0OnSelectObject\0"
    "OnSelectDiscreteObject\0OnDetachDiscreteObject\0"
    "OnHidePart\0OnShowPart\0OnSelectPart\0"
    "OnSelectSurface\0OnSelectCurve\0"
    "OnSelectNode\0OnCopyMaterial\0OnCopyInterface\0"
    "OnCopyBC\0OnCopyIC\0OnCopyConnector\0"
    "OnCopyLoad\0OnCopyConstraint\0OnCopyStep\0"
    "OnEditOutput\0OnEditOutputLog\0"
    "OnRemoveEmptySelections\0OnRemoveAllSelections\0"
    "OnChangeMaterial\0OnExportMaterials\0"
    "OnExportAllMaterials\0OnImportMaterials\0"
    "OnDeleteAllMaterials\0OnSwapMasterSlave\0"
    "OnGenerateMap\0OnDeleteAllBC\0"
    "OnDeleteAllLoads\0OnDeleteAllIC\0"
    "OnDeleteAllContact\0OnDeleteAllConstraints\0"
    "OnDeleteAllConnectors\0OnDeleteAllSteps"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CModelViewer[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      57,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,  299,    2, 0x08 /* Private */,
       6,    0,  304,    2, 0x08 /* Private */,
       7,    0,  305,    2, 0x08 /* Private */,
       8,    1,  306,    2, 0x08 /* Private */,
      10,    0,  309,    2, 0x08 /* Private */,
      11,    1,  310,    2, 0x08 /* Private */,
      13,    0,  313,    2, 0x08 /* Private */,
      14,    1,  314,    2, 0x08 /* Private */,
      15,    0,  317,    2, 0x08 /* Private */,
      16,    0,  318,    2, 0x08 /* Private */,
      17,    0,  319,    2, 0x08 /* Private */,
      18,    0,  320,    2, 0x08 /* Private */,
      19,    0,  321,    2, 0x08 /* Private */,
      20,    0,  322,    2, 0x08 /* Private */,
      21,    0,  323,    2, 0x08 /* Private */,
      22,    0,  324,    2, 0x08 /* Private */,
      23,    0,  325,    2, 0x08 /* Private */,
      24,    0,  326,    2, 0x08 /* Private */,
      25,    0,  327,    2, 0x08 /* Private */,
      26,    0,  328,    2, 0x08 /* Private */,
      27,    0,  329,    2, 0x08 /* Private */,
      28,    0,  330,    2, 0x08 /* Private */,
      29,    0,  331,    2, 0x08 /* Private */,
      30,    0,  332,    2, 0x08 /* Private */,
      31,    0,  333,    2, 0x08 /* Private */,
      32,    0,  334,    2, 0x08 /* Private */,
      33,    0,  335,    2, 0x08 /* Private */,
      34,    0,  336,    2, 0x08 /* Private */,
      35,    0,  337,    2, 0x08 /* Private */,
      36,    0,  338,    2, 0x08 /* Private */,
      37,    0,  339,    2, 0x08 /* Private */,
      38,    0,  340,    2, 0x08 /* Private */,
      39,    0,  341,    2, 0x08 /* Private */,
      40,    0,  342,    2, 0x08 /* Private */,
      41,    0,  343,    2, 0x08 /* Private */,
      42,    0,  344,    2, 0x08 /* Private */,
      43,    0,  345,    2, 0x08 /* Private */,
      44,    0,  346,    2, 0x08 /* Private */,
      45,    0,  347,    2, 0x08 /* Private */,
      46,    0,  348,    2, 0x08 /* Private */,
      47,    0,  349,    2, 0x08 /* Private */,
      48,    0,  350,    2, 0x08 /* Private */,
      49,    0,  351,    2, 0x08 /* Private */,
      50,    0,  352,    2, 0x08 /* Private */,
      51,    0,  353,    2, 0x08 /* Private */,
      52,    0,  354,    2, 0x08 /* Private */,
      53,    0,  355,    2, 0x08 /* Private */,
      54,    0,  356,    2, 0x08 /* Private */,
      55,    0,  357,    2, 0x08 /* Private */,
      56,    0,  358,    2, 0x08 /* Private */,
      57,    0,  359,    2, 0x08 /* Private */,
      58,    0,  360,    2, 0x08 /* Private */,
      59,    0,  361,    2, 0x08 /* Private */,
      60,    0,  362,    2, 0x08 /* Private */,
      61,    0,  363,    2, 0x08 /* Private */,
      62,    0,  364,    2, 0x08 /* Private */,
      63,    0,  365,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3,    4,    5,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    9,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   12,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    9,
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
        CModelViewer *_t = static_cast<CModelViewer *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_modelTree_currentItemChanged((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< QTreeWidgetItem*(*)>(_a[2]))); break;
        case 1: _t->on_selectButton_clicked(); break;
        case 2: _t->on_deleteButton_clicked(); break;
        case 3: _t->on_searchButton_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->on_syncButton_clicked(); break;
        case 5: _t->on_props_nameChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->on_props_selectionChanged(); break;
        case 7: _t->on_props_dataChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: _t->OnDeleteItem(); break;
        case 9: _t->OnAddMaterial(); break;
        case 10: _t->OnUnhideAllObjects(); break;
        case 11: _t->OnUnhideAllParts(); break;
        case 12: _t->OnAddBC(); break;
        case 13: _t->OnAddSurfaceLoad(); break;
        case 14: _t->OnAddBodyLoad(); break;
        case 15: _t->OnAddInitialCondition(); break;
        case 16: _t->OnAddContact(); break;
        case 17: _t->OnAddConstraint(); break;
        case 18: _t->OnAddConnector(); break;
        case 19: _t->OnAddStep(); break;
        case 20: _t->OnHideObject(); break;
        case 21: _t->OnShowObject(); break;
        case 22: _t->OnSelectObject(); break;
        case 23: _t->OnSelectDiscreteObject(); break;
        case 24: _t->OnDetachDiscreteObject(); break;
        case 25: _t->OnHidePart(); break;
        case 26: _t->OnShowPart(); break;
        case 27: _t->OnSelectPart(); break;
        case 28: _t->OnSelectSurface(); break;
        case 29: _t->OnSelectCurve(); break;
        case 30: _t->OnSelectNode(); break;
        case 31: _t->OnCopyMaterial(); break;
        case 32: _t->OnCopyInterface(); break;
        case 33: _t->OnCopyBC(); break;
        case 34: _t->OnCopyIC(); break;
        case 35: _t->OnCopyConnector(); break;
        case 36: _t->OnCopyLoad(); break;
        case 37: _t->OnCopyConstraint(); break;
        case 38: _t->OnCopyStep(); break;
        case 39: _t->OnEditOutput(); break;
        case 40: _t->OnEditOutputLog(); break;
        case 41: _t->OnRemoveEmptySelections(); break;
        case 42: _t->OnRemoveAllSelections(); break;
        case 43: _t->OnChangeMaterial(); break;
        case 44: _t->OnExportMaterials(); break;
        case 45: _t->OnExportAllMaterials(); break;
        case 46: _t->OnImportMaterials(); break;
        case 47: _t->OnDeleteAllMaterials(); break;
        case 48: _t->OnSwapMasterSlave(); break;
        case 49: _t->OnGenerateMap(); break;
        case 50: _t->OnDeleteAllBC(); break;
        case 51: _t->OnDeleteAllLoads(); break;
        case 52: _t->OnDeleteAllIC(); break;
        case 53: _t->OnDeleteAllContact(); break;
        case 54: _t->OnDeleteAllConstraints(); break;
        case 55: _t->OnDeleteAllConnectors(); break;
        case 56: _t->OnDeleteAllSteps(); break;
        default: ;
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
        if (_id < 57)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 57;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 57)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 57;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
