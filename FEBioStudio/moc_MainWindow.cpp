/****************************************************************************
** Meta object code from reading C++ file 'MainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "MainWindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MainWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CMainWindow_t {
    QByteArrayData data[138];
    char stringdata0[3239];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CMainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CMainWindow_t qt_meta_stringdata_CMainWindow = {
    {
QT_MOC_LITERAL(0, 0, 11), // "CMainWindow"
QT_MOC_LITERAL(1, 12, 22), // "on_actionNew_triggered"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 23), // "on_actionOpen_triggered"
QT_MOC_LITERAL(4, 60, 23), // "on_actionSave_triggered"
QT_MOC_LITERAL(5, 84, 25), // "on_actionSaveAs_triggered"
QT_MOC_LITERAL(6, 110, 23), // "on_actionInfo_triggered"
QT_MOC_LITERAL(7, 134, 32), // "on_actionImportFEModel_triggered"
QT_MOC_LITERAL(8, 167, 32), // "on_actionExportFEModel_triggered"
QT_MOC_LITERAL(9, 200, 33), // "on_actionImportGeometry_trigg..."
QT_MOC_LITERAL(10, 234, 33), // "on_actionExportGeometry_trigg..."
QT_MOC_LITERAL(11, 268, 30), // "on_actionImportImage_triggered"
QT_MOC_LITERAL(12, 299, 29), // "on_actionConvertFeb_triggered"
QT_MOC_LITERAL(13, 329, 29), // "on_actionConvertGeo_triggered"
QT_MOC_LITERAL(14, 359, 23), // "on_actionExit_triggered"
QT_MOC_LITERAL(15, 383, 24), // "on_recentFiles_triggered"
QT_MOC_LITERAL(16, 408, 8), // "QAction*"
QT_MOC_LITERAL(17, 417, 6), // "action"
QT_MOC_LITERAL(18, 424, 26), // "on_recentFEFiles_triggered"
QT_MOC_LITERAL(19, 451, 28), // "on_recentGeomFiles_triggered"
QT_MOC_LITERAL(20, 480, 23), // "on_actionUndo_triggered"
QT_MOC_LITERAL(21, 504, 23), // "on_actionRedo_triggered"
QT_MOC_LITERAL(22, 528, 34), // "on_actionInvertSelection_trig..."
QT_MOC_LITERAL(23, 563, 33), // "on_actionClearSelection_trigg..."
QT_MOC_LITERAL(24, 597, 34), // "on_actionDeleteSelection_trig..."
QT_MOC_LITERAL(25, 632, 32), // "on_actionHideSelection_triggered"
QT_MOC_LITERAL(26, 665, 33), // "on_actionHideUnselected_trigg..."
QT_MOC_LITERAL(27, 699, 28), // "on_actionUnhideAll_triggered"
QT_MOC_LITERAL(28, 728, 32), // "on_actionToggleVisible_triggered"
QT_MOC_LITERAL(29, 761, 32), // "on_actionNameSelection_triggered"
QT_MOC_LITERAL(30, 794, 28), // "on_actionTransform_triggered"
QT_MOC_LITERAL(31, 823, 36), // "on_actionCollapseTransform_tr..."
QT_MOC_LITERAL(32, 860, 24), // "on_actionClone_triggered"
QT_MOC_LITERAL(33, 885, 28), // "on_actionCloneGrid_triggered"
QT_MOC_LITERAL(34, 914, 31), // "on_actionCloneRevolve_triggered"
QT_MOC_LITERAL(35, 946, 24), // "on_actionMerge_triggered"
QT_MOC_LITERAL(36, 971, 24), // "on_actionPurge_triggered"
QT_MOC_LITERAL(37, 996, 25), // "on_actionDetach_triggered"
QT_MOC_LITERAL(38, 1022, 26), // "on_actionExtract_triggered"
QT_MOC_LITERAL(39, 1049, 30), // "on_actionEditProject_triggered"
QT_MOC_LITERAL(40, 1080, 24), // "on_actionAddBC_triggered"
QT_MOC_LITERAL(41, 1105, 31), // "on_actionAddNodalLoad_triggered"
QT_MOC_LITERAL(42, 1137, 30), // "on_actionAddSurfLoad_triggered"
QT_MOC_LITERAL(43, 1168, 30), // "on_actionAddBodyLoad_triggered"
QT_MOC_LITERAL(44, 1199, 24), // "on_actionAddIC_triggered"
QT_MOC_LITERAL(45, 1224, 29), // "on_actionAddContact_triggered"
QT_MOC_LITERAL(46, 1254, 37), // "on_actionAddRigidConstraint_t..."
QT_MOC_LITERAL(47, 1292, 36), // "on_actionAddRigidConnector_tr..."
QT_MOC_LITERAL(48, 1329, 30), // "on_actionAddMaterial_triggered"
QT_MOC_LITERAL(49, 1360, 26), // "on_actionAddStep_triggered"
QT_MOC_LITERAL(50, 1387, 30), // "on_actionAddReaction_triggered"
QT_MOC_LITERAL(51, 1418, 30), // "on_actionSoluteTable_triggered"
QT_MOC_LITERAL(52, 1449, 27), // "on_actionSBMTable_triggered"
QT_MOC_LITERAL(53, 1477, 30), // "on_actionCurveEditor_triggered"
QT_MOC_LITERAL(54, 1508, 32), // "on_actionMeshInspector_triggered"
QT_MOC_LITERAL(55, 1541, 38), // "on_actionElasticityConvertor_..."
QT_MOC_LITERAL(56, 1580, 27), // "on_actionFEBioRun_triggered"
QT_MOC_LITERAL(57, 1608, 28), // "on_actionFEBioStop_triggered"
QT_MOC_LITERAL(58, 1637, 32), // "on_actionFEBioOptimize_triggered"
QT_MOC_LITERAL(59, 1670, 26), // "on_actionOptions_triggered"
QT_MOC_LITERAL(60, 1697, 33), // "on_actionUndoViewChange_trigg..."
QT_MOC_LITERAL(61, 1731, 33), // "on_actionRedoViewChange_trigg..."
QT_MOC_LITERAL(62, 1765, 29), // "on_actionZoomSelect_triggered"
QT_MOC_LITERAL(63, 1795, 30), // "on_actionZoomExtents_triggered"
QT_MOC_LITERAL(64, 1826, 28), // "on_actionViewCapture_toggled"
QT_MOC_LITERAL(65, 1855, 8), // "bchecked"
QT_MOC_LITERAL(66, 1864, 22), // "on_actionOrtho_toggled"
QT_MOC_LITERAL(67, 1887, 1), // "b"
QT_MOC_LITERAL(68, 1889, 25), // "on_actionShowGrid_toggled"
QT_MOC_LITERAL(69, 1915, 30), // "on_actionShowMeshLines_toggled"
QT_MOC_LITERAL(70, 1946, 30), // "on_actionShowEdgeLines_toggled"
QT_MOC_LITERAL(71, 1977, 32), // "on_actionBackfaceCulling_toggled"
QT_MOC_LITERAL(72, 2010, 28), // "on_actionShowNormals_toggled"
QT_MOC_LITERAL(73, 2039, 27), // "on_actionShowFibers_toggled"
QT_MOC_LITERAL(74, 2067, 24), // "on_actionFront_triggered"
QT_MOC_LITERAL(75, 2092, 23), // "on_actionBack_triggered"
QT_MOC_LITERAL(76, 2116, 23), // "on_actionLeft_triggered"
QT_MOC_LITERAL(77, 2140, 24), // "on_actionRight_triggered"
QT_MOC_LITERAL(78, 2165, 22), // "on_actionTop_triggered"
QT_MOC_LITERAL(79, 2188, 25), // "on_actionBottom_triggered"
QT_MOC_LITERAL(80, 2214, 26), // "on_actionWireframe_toggled"
QT_MOC_LITERAL(81, 2241, 25), // "on_actionSnap3D_triggered"
QT_MOC_LITERAL(82, 2267, 29), // "on_actionOnlineHelp_triggered"
QT_MOC_LITERAL(83, 2297, 24), // "on_actionAbout_triggered"
QT_MOC_LITERAL(84, 2322, 23), // "on_actionSelect_toggled"
QT_MOC_LITERAL(85, 2346, 26), // "on_actionTranslate_toggled"
QT_MOC_LITERAL(86, 2373, 23), // "on_actionRotate_toggled"
QT_MOC_LITERAL(87, 2397, 22), // "on_actionScale_toggled"
QT_MOC_LITERAL(88, 2420, 34), // "on_selectCoord_currentIndexCh..."
QT_MOC_LITERAL(89, 2455, 1), // "n"
QT_MOC_LITERAL(90, 2457, 30), // "on_actionSelectObjects_toggled"
QT_MOC_LITERAL(91, 2488, 28), // "on_actionSelectParts_toggled"
QT_MOC_LITERAL(92, 2517, 31), // "on_actionSelectSurfaces_toggled"
QT_MOC_LITERAL(93, 2549, 29), // "on_actionSelectCurves_toggled"
QT_MOC_LITERAL(94, 2579, 28), // "on_actionSelectNodes_toggled"
QT_MOC_LITERAL(95, 2608, 31), // "on_actionSelectDiscrete_toggled"
QT_MOC_LITERAL(96, 2640, 21), // "on_selectRect_toggled"
QT_MOC_LITERAL(97, 2662, 23), // "on_selectCircle_toggled"
QT_MOC_LITERAL(98, 2686, 21), // "on_selectFree_toggled"
QT_MOC_LITERAL(99, 2708, 10), // "closeEvent"
QT_MOC_LITERAL(100, 2719, 12), // "QCloseEvent*"
QT_MOC_LITERAL(101, 2732, 2), // "ev"
QT_MOC_LITERAL(102, 2735, 13), // "keyPressEvent"
QT_MOC_LITERAL(103, 2749, 10), // "QKeyEvent*"
QT_MOC_LITERAL(104, 2760, 19), // "finishedReadingFile"
QT_MOC_LITERAL(105, 2780, 7), // "success"
QT_MOC_LITERAL(106, 2788, 11), // "errorString"
QT_MOC_LITERAL(107, 2800, 17), // "checkFileProgress"
QT_MOC_LITERAL(108, 2818, 21), // "on_glview_pointPicked"
QT_MOC_LITERAL(109, 2840, 5), // "vec3d"
QT_MOC_LITERAL(110, 2846, 1), // "r"
QT_MOC_LITERAL(111, 2848, 26), // "on_glview_selectionChanged"
QT_MOC_LITERAL(112, 2875, 13), // "onRunFinished"
QT_MOC_LITERAL(113, 2889, 8), // "exitCode"
QT_MOC_LITERAL(114, 2898, 20), // "QProcess::ExitStatus"
QT_MOC_LITERAL(115, 2919, 2), // "es"
QT_MOC_LITERAL(116, 2922, 11), // "onReadyRead"
QT_MOC_LITERAL(117, 2934, 15), // "onErrorOccurred"
QT_MOC_LITERAL(118, 2950, 22), // "QProcess::ProcessError"
QT_MOC_LITERAL(119, 2973, 3), // "err"
QT_MOC_LITERAL(120, 2977, 17), // "onExportMaterials"
QT_MOC_LITERAL(121, 2995, 18), // "vector<GMaterial*>"
QT_MOC_LITERAL(122, 3014, 7), // "matList"
QT_MOC_LITERAL(123, 3022, 20), // "onExportAllMaterials"
QT_MOC_LITERAL(124, 3043, 17), // "onImportMaterials"
QT_MOC_LITERAL(125, 3061, 18), // "DeleteAllMaterials"
QT_MOC_LITERAL(126, 3080, 11), // "DeleteAllBC"
QT_MOC_LITERAL(127, 3092, 14), // "DeleteAllLoads"
QT_MOC_LITERAL(128, 3107, 11), // "DeleteAllIC"
QT_MOC_LITERAL(129, 3119, 16), // "DeleteAllContact"
QT_MOC_LITERAL(130, 3136, 20), // "DeleteAllConstraints"
QT_MOC_LITERAL(131, 3157, 19), // "DeleteAllConnectors"
QT_MOC_LITERAL(132, 3177, 14), // "DeleteAllSteps"
QT_MOC_LITERAL(133, 3192, 9), // "GetGLView"
QT_MOC_LITERAL(134, 3202, 8), // "CGLView*"
QT_MOC_LITERAL(135, 3211, 14), // "changeViewMode"
QT_MOC_LITERAL(136, 3226, 9), // "View_Mode"
QT_MOC_LITERAL(137, 3236, 2) // "vm"

    },
    "CMainWindow\0on_actionNew_triggered\0\0"
    "on_actionOpen_triggered\0on_actionSave_triggered\0"
    "on_actionSaveAs_triggered\0"
    "on_actionInfo_triggered\0"
    "on_actionImportFEModel_triggered\0"
    "on_actionExportFEModel_triggered\0"
    "on_actionImportGeometry_triggered\0"
    "on_actionExportGeometry_triggered\0"
    "on_actionImportImage_triggered\0"
    "on_actionConvertFeb_triggered\0"
    "on_actionConvertGeo_triggered\0"
    "on_actionExit_triggered\0"
    "on_recentFiles_triggered\0QAction*\0"
    "action\0on_recentFEFiles_triggered\0"
    "on_recentGeomFiles_triggered\0"
    "on_actionUndo_triggered\0on_actionRedo_triggered\0"
    "on_actionInvertSelection_triggered\0"
    "on_actionClearSelection_triggered\0"
    "on_actionDeleteSelection_triggered\0"
    "on_actionHideSelection_triggered\0"
    "on_actionHideUnselected_triggered\0"
    "on_actionUnhideAll_triggered\0"
    "on_actionToggleVisible_triggered\0"
    "on_actionNameSelection_triggered\0"
    "on_actionTransform_triggered\0"
    "on_actionCollapseTransform_triggered\0"
    "on_actionClone_triggered\0"
    "on_actionCloneGrid_triggered\0"
    "on_actionCloneRevolve_triggered\0"
    "on_actionMerge_triggered\0"
    "on_actionPurge_triggered\0"
    "on_actionDetach_triggered\0"
    "on_actionExtract_triggered\0"
    "on_actionEditProject_triggered\0"
    "on_actionAddBC_triggered\0"
    "on_actionAddNodalLoad_triggered\0"
    "on_actionAddSurfLoad_triggered\0"
    "on_actionAddBodyLoad_triggered\0"
    "on_actionAddIC_triggered\0"
    "on_actionAddContact_triggered\0"
    "on_actionAddRigidConstraint_triggered\0"
    "on_actionAddRigidConnector_triggered\0"
    "on_actionAddMaterial_triggered\0"
    "on_actionAddStep_triggered\0"
    "on_actionAddReaction_triggered\0"
    "on_actionSoluteTable_triggered\0"
    "on_actionSBMTable_triggered\0"
    "on_actionCurveEditor_triggered\0"
    "on_actionMeshInspector_triggered\0"
    "on_actionElasticityConvertor_triggered\0"
    "on_actionFEBioRun_triggered\0"
    "on_actionFEBioStop_triggered\0"
    "on_actionFEBioOptimize_triggered\0"
    "on_actionOptions_triggered\0"
    "on_actionUndoViewChange_triggered\0"
    "on_actionRedoViewChange_triggered\0"
    "on_actionZoomSelect_triggered\0"
    "on_actionZoomExtents_triggered\0"
    "on_actionViewCapture_toggled\0bchecked\0"
    "on_actionOrtho_toggled\0b\0"
    "on_actionShowGrid_toggled\0"
    "on_actionShowMeshLines_toggled\0"
    "on_actionShowEdgeLines_toggled\0"
    "on_actionBackfaceCulling_toggled\0"
    "on_actionShowNormals_toggled\0"
    "on_actionShowFibers_toggled\0"
    "on_actionFront_triggered\0"
    "on_actionBack_triggered\0on_actionLeft_triggered\0"
    "on_actionRight_triggered\0"
    "on_actionTop_triggered\0on_actionBottom_triggered\0"
    "on_actionWireframe_toggled\0"
    "on_actionSnap3D_triggered\0"
    "on_actionOnlineHelp_triggered\0"
    "on_actionAbout_triggered\0"
    "on_actionSelect_toggled\0"
    "on_actionTranslate_toggled\0"
    "on_actionRotate_toggled\0on_actionScale_toggled\0"
    "on_selectCoord_currentIndexChanged\0n\0"
    "on_actionSelectObjects_toggled\0"
    "on_actionSelectParts_toggled\0"
    "on_actionSelectSurfaces_toggled\0"
    "on_actionSelectCurves_toggled\0"
    "on_actionSelectNodes_toggled\0"
    "on_actionSelectDiscrete_toggled\0"
    "on_selectRect_toggled\0on_selectCircle_toggled\0"
    "on_selectFree_toggled\0closeEvent\0"
    "QCloseEvent*\0ev\0keyPressEvent\0QKeyEvent*\0"
    "finishedReadingFile\0success\0errorString\0"
    "checkFileProgress\0on_glview_pointPicked\0"
    "vec3d\0r\0on_glview_selectionChanged\0"
    "onRunFinished\0exitCode\0QProcess::ExitStatus\0"
    "es\0onReadyRead\0onErrorOccurred\0"
    "QProcess::ProcessError\0err\0onExportMaterials\0"
    "vector<GMaterial*>\0matList\0"
    "onExportAllMaterials\0onImportMaterials\0"
    "DeleteAllMaterials\0DeleteAllBC\0"
    "DeleteAllLoads\0DeleteAllIC\0DeleteAllContact\0"
    "DeleteAllConstraints\0DeleteAllConnectors\0"
    "DeleteAllSteps\0GetGLView\0CGLView*\0"
    "changeViewMode\0View_Mode\0vm"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CMainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
     114,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,  584,    2, 0x0a /* Public */,
       3,    0,  585,    2, 0x0a /* Public */,
       4,    0,  586,    2, 0x0a /* Public */,
       5,    0,  587,    2, 0x0a /* Public */,
       6,    0,  588,    2, 0x0a /* Public */,
       7,    0,  589,    2, 0x0a /* Public */,
       8,    0,  590,    2, 0x0a /* Public */,
       9,    0,  591,    2, 0x0a /* Public */,
      10,    0,  592,    2, 0x0a /* Public */,
      11,    0,  593,    2, 0x0a /* Public */,
      12,    0,  594,    2, 0x0a /* Public */,
      13,    0,  595,    2, 0x0a /* Public */,
      14,    0,  596,    2, 0x0a /* Public */,
      15,    1,  597,    2, 0x0a /* Public */,
      18,    1,  600,    2, 0x0a /* Public */,
      19,    1,  603,    2, 0x0a /* Public */,
      20,    0,  606,    2, 0x0a /* Public */,
      21,    0,  607,    2, 0x0a /* Public */,
      22,    0,  608,    2, 0x0a /* Public */,
      23,    0,  609,    2, 0x0a /* Public */,
      24,    0,  610,    2, 0x0a /* Public */,
      25,    0,  611,    2, 0x0a /* Public */,
      26,    0,  612,    2, 0x0a /* Public */,
      27,    0,  613,    2, 0x0a /* Public */,
      28,    0,  614,    2, 0x0a /* Public */,
      29,    0,  615,    2, 0x0a /* Public */,
      30,    0,  616,    2, 0x0a /* Public */,
      31,    0,  617,    2, 0x0a /* Public */,
      32,    0,  618,    2, 0x0a /* Public */,
      33,    0,  619,    2, 0x0a /* Public */,
      34,    0,  620,    2, 0x0a /* Public */,
      35,    0,  621,    2, 0x0a /* Public */,
      36,    0,  622,    2, 0x0a /* Public */,
      37,    0,  623,    2, 0x0a /* Public */,
      38,    0,  624,    2, 0x0a /* Public */,
      39,    0,  625,    2, 0x0a /* Public */,
      40,    0,  626,    2, 0x0a /* Public */,
      41,    0,  627,    2, 0x0a /* Public */,
      42,    0,  628,    2, 0x0a /* Public */,
      43,    0,  629,    2, 0x0a /* Public */,
      44,    0,  630,    2, 0x0a /* Public */,
      45,    0,  631,    2, 0x0a /* Public */,
      46,    0,  632,    2, 0x0a /* Public */,
      47,    0,  633,    2, 0x0a /* Public */,
      48,    0,  634,    2, 0x0a /* Public */,
      49,    0,  635,    2, 0x0a /* Public */,
      50,    0,  636,    2, 0x0a /* Public */,
      51,    0,  637,    2, 0x0a /* Public */,
      52,    0,  638,    2, 0x0a /* Public */,
      53,    0,  639,    2, 0x0a /* Public */,
      54,    0,  640,    2, 0x0a /* Public */,
      55,    0,  641,    2, 0x0a /* Public */,
      56,    0,  642,    2, 0x0a /* Public */,
      57,    0,  643,    2, 0x0a /* Public */,
      58,    0,  644,    2, 0x0a /* Public */,
      59,    0,  645,    2, 0x0a /* Public */,
      60,    0,  646,    2, 0x0a /* Public */,
      61,    0,  647,    2, 0x0a /* Public */,
      62,    0,  648,    2, 0x0a /* Public */,
      63,    0,  649,    2, 0x0a /* Public */,
      64,    1,  650,    2, 0x0a /* Public */,
      66,    1,  653,    2, 0x0a /* Public */,
      68,    1,  656,    2, 0x0a /* Public */,
      69,    1,  659,    2, 0x0a /* Public */,
      70,    1,  662,    2, 0x0a /* Public */,
      71,    1,  665,    2, 0x0a /* Public */,
      72,    1,  668,    2, 0x0a /* Public */,
      73,    1,  671,    2, 0x0a /* Public */,
      74,    0,  674,    2, 0x0a /* Public */,
      75,    0,  675,    2, 0x0a /* Public */,
      76,    0,  676,    2, 0x0a /* Public */,
      77,    0,  677,    2, 0x0a /* Public */,
      78,    0,  678,    2, 0x0a /* Public */,
      79,    0,  679,    2, 0x0a /* Public */,
      80,    1,  680,    2, 0x0a /* Public */,
      81,    0,  683,    2, 0x0a /* Public */,
      82,    0,  684,    2, 0x0a /* Public */,
      83,    0,  685,    2, 0x0a /* Public */,
      84,    1,  686,    2, 0x0a /* Public */,
      85,    1,  689,    2, 0x0a /* Public */,
      86,    1,  692,    2, 0x0a /* Public */,
      87,    1,  695,    2, 0x0a /* Public */,
      88,    1,  698,    2, 0x0a /* Public */,
      90,    1,  701,    2, 0x0a /* Public */,
      91,    1,  704,    2, 0x0a /* Public */,
      92,    1,  707,    2, 0x0a /* Public */,
      93,    1,  710,    2, 0x0a /* Public */,
      94,    1,  713,    2, 0x0a /* Public */,
      95,    1,  716,    2, 0x0a /* Public */,
      96,    1,  719,    2, 0x0a /* Public */,
      97,    1,  722,    2, 0x0a /* Public */,
      98,    1,  725,    2, 0x0a /* Public */,
      99,    1,  728,    2, 0x0a /* Public */,
     102,    1,  731,    2, 0x0a /* Public */,
     104,    2,  734,    2, 0x0a /* Public */,
     107,    0,  739,    2, 0x0a /* Public */,
     108,    1,  740,    2, 0x0a /* Public */,
     111,    0,  743,    2, 0x0a /* Public */,
     112,    2,  744,    2, 0x0a /* Public */,
     116,    0,  749,    2, 0x0a /* Public */,
     117,    1,  750,    2, 0x0a /* Public */,
     120,    1,  753,    2, 0x0a /* Public */,
     123,    0,  756,    2, 0x0a /* Public */,
     124,    0,  757,    2, 0x0a /* Public */,
     125,    0,  758,    2, 0x0a /* Public */,
     126,    0,  759,    2, 0x0a /* Public */,
     127,    0,  760,    2, 0x0a /* Public */,
     128,    0,  761,    2, 0x0a /* Public */,
     129,    0,  762,    2, 0x0a /* Public */,
     130,    0,  763,    2, 0x0a /* Public */,
     131,    0,  764,    2, 0x0a /* Public */,
     132,    0,  765,    2, 0x0a /* Public */,
     133,    0,  766,    2, 0x0a /* Public */,
     135,    1,  767,    2, 0x0a /* Public */,

 // slots: parameters
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
    QMetaType::Void, 0x80000000 | 16,   17,
    QMetaType::Void, 0x80000000 | 16,   17,
    QMetaType::Void, 0x80000000 | 16,   17,
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
    QMetaType::Void, QMetaType::Bool,   65,
    QMetaType::Void, QMetaType::Bool,   67,
    QMetaType::Void, QMetaType::Bool,   67,
    QMetaType::Void, QMetaType::Bool,   67,
    QMetaType::Void, QMetaType::Bool,   67,
    QMetaType::Void, QMetaType::Bool,   67,
    QMetaType::Void, QMetaType::Bool,   67,
    QMetaType::Void, QMetaType::Bool,   67,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   67,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   67,
    QMetaType::Void, QMetaType::Bool,   67,
    QMetaType::Void, QMetaType::Bool,   67,
    QMetaType::Void, QMetaType::Bool,   67,
    QMetaType::Void, QMetaType::Int,   89,
    QMetaType::Void, QMetaType::Bool,   67,
    QMetaType::Void, QMetaType::Bool,   67,
    QMetaType::Void, QMetaType::Bool,   67,
    QMetaType::Void, QMetaType::Bool,   67,
    QMetaType::Void, QMetaType::Bool,   67,
    QMetaType::Void, QMetaType::Bool,   67,
    QMetaType::Void, QMetaType::Bool,   67,
    QMetaType::Void, QMetaType::Bool,   67,
    QMetaType::Void, QMetaType::Bool,   67,
    QMetaType::Void, 0x80000000 | 100,  101,
    QMetaType::Void, 0x80000000 | 103,  101,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,  105,  106,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 109,  110,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 114,  113,  115,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 118,  119,
    QMetaType::Void, 0x80000000 | 121,  122,
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
    0x80000000 | 134,
    QMetaType::Void, 0x80000000 | 136,  137,

       0        // eod
};

void CMainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CMainWindow *_t = static_cast<CMainWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_actionNew_triggered(); break;
        case 1: _t->on_actionOpen_triggered(); break;
        case 2: _t->on_actionSave_triggered(); break;
        case 3: _t->on_actionSaveAs_triggered(); break;
        case 4: _t->on_actionInfo_triggered(); break;
        case 5: _t->on_actionImportFEModel_triggered(); break;
        case 6: _t->on_actionExportFEModel_triggered(); break;
        case 7: _t->on_actionImportGeometry_triggered(); break;
        case 8: _t->on_actionExportGeometry_triggered(); break;
        case 9: _t->on_actionImportImage_triggered(); break;
        case 10: _t->on_actionConvertFeb_triggered(); break;
        case 11: _t->on_actionConvertGeo_triggered(); break;
        case 12: _t->on_actionExit_triggered(); break;
        case 13: _t->on_recentFiles_triggered((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        case 14: _t->on_recentFEFiles_triggered((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        case 15: _t->on_recentGeomFiles_triggered((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        case 16: _t->on_actionUndo_triggered(); break;
        case 17: _t->on_actionRedo_triggered(); break;
        case 18: _t->on_actionInvertSelection_triggered(); break;
        case 19: _t->on_actionClearSelection_triggered(); break;
        case 20: _t->on_actionDeleteSelection_triggered(); break;
        case 21: _t->on_actionHideSelection_triggered(); break;
        case 22: _t->on_actionHideUnselected_triggered(); break;
        case 23: _t->on_actionUnhideAll_triggered(); break;
        case 24: _t->on_actionToggleVisible_triggered(); break;
        case 25: _t->on_actionNameSelection_triggered(); break;
        case 26: _t->on_actionTransform_triggered(); break;
        case 27: _t->on_actionCollapseTransform_triggered(); break;
        case 28: _t->on_actionClone_triggered(); break;
        case 29: _t->on_actionCloneGrid_triggered(); break;
        case 30: _t->on_actionCloneRevolve_triggered(); break;
        case 31: _t->on_actionMerge_triggered(); break;
        case 32: _t->on_actionPurge_triggered(); break;
        case 33: _t->on_actionDetach_triggered(); break;
        case 34: _t->on_actionExtract_triggered(); break;
        case 35: _t->on_actionEditProject_triggered(); break;
        case 36: _t->on_actionAddBC_triggered(); break;
        case 37: _t->on_actionAddNodalLoad_triggered(); break;
        case 38: _t->on_actionAddSurfLoad_triggered(); break;
        case 39: _t->on_actionAddBodyLoad_triggered(); break;
        case 40: _t->on_actionAddIC_triggered(); break;
        case 41: _t->on_actionAddContact_triggered(); break;
        case 42: _t->on_actionAddRigidConstraint_triggered(); break;
        case 43: _t->on_actionAddRigidConnector_triggered(); break;
        case 44: _t->on_actionAddMaterial_triggered(); break;
        case 45: _t->on_actionAddStep_triggered(); break;
        case 46: _t->on_actionAddReaction_triggered(); break;
        case 47: _t->on_actionSoluteTable_triggered(); break;
        case 48: _t->on_actionSBMTable_triggered(); break;
        case 49: _t->on_actionCurveEditor_triggered(); break;
        case 50: _t->on_actionMeshInspector_triggered(); break;
        case 51: _t->on_actionElasticityConvertor_triggered(); break;
        case 52: _t->on_actionFEBioRun_triggered(); break;
        case 53: _t->on_actionFEBioStop_triggered(); break;
        case 54: _t->on_actionFEBioOptimize_triggered(); break;
        case 55: _t->on_actionOptions_triggered(); break;
        case 56: _t->on_actionUndoViewChange_triggered(); break;
        case 57: _t->on_actionRedoViewChange_triggered(); break;
        case 58: _t->on_actionZoomSelect_triggered(); break;
        case 59: _t->on_actionZoomExtents_triggered(); break;
        case 60: _t->on_actionViewCapture_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 61: _t->on_actionOrtho_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 62: _t->on_actionShowGrid_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 63: _t->on_actionShowMeshLines_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 64: _t->on_actionShowEdgeLines_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 65: _t->on_actionBackfaceCulling_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 66: _t->on_actionShowNormals_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 67: _t->on_actionShowFibers_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 68: _t->on_actionFront_triggered(); break;
        case 69: _t->on_actionBack_triggered(); break;
        case 70: _t->on_actionLeft_triggered(); break;
        case 71: _t->on_actionRight_triggered(); break;
        case 72: _t->on_actionTop_triggered(); break;
        case 73: _t->on_actionBottom_triggered(); break;
        case 74: _t->on_actionWireframe_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 75: _t->on_actionSnap3D_triggered(); break;
        case 76: _t->on_actionOnlineHelp_triggered(); break;
        case 77: _t->on_actionAbout_triggered(); break;
        case 78: _t->on_actionSelect_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 79: _t->on_actionTranslate_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 80: _t->on_actionRotate_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 81: _t->on_actionScale_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 82: _t->on_selectCoord_currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 83: _t->on_actionSelectObjects_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 84: _t->on_actionSelectParts_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 85: _t->on_actionSelectSurfaces_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 86: _t->on_actionSelectCurves_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 87: _t->on_actionSelectNodes_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 88: _t->on_actionSelectDiscrete_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 89: _t->on_selectRect_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 90: _t->on_selectCircle_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 91: _t->on_selectFree_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 92: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        case 93: _t->keyPressEvent((*reinterpret_cast< QKeyEvent*(*)>(_a[1]))); break;
        case 94: _t->finishedReadingFile((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 95: _t->checkFileProgress(); break;
        case 96: _t->on_glview_pointPicked((*reinterpret_cast< const vec3d(*)>(_a[1]))); break;
        case 97: _t->on_glview_selectionChanged(); break;
        case 98: _t->onRunFinished((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QProcess::ExitStatus(*)>(_a[2]))); break;
        case 99: _t->onReadyRead(); break;
        case 100: _t->onErrorOccurred((*reinterpret_cast< QProcess::ProcessError(*)>(_a[1]))); break;
        case 101: _t->onExportMaterials((*reinterpret_cast< const vector<GMaterial*>(*)>(_a[1]))); break;
        case 102: _t->onExportAllMaterials(); break;
        case 103: _t->onImportMaterials(); break;
        case 104: _t->DeleteAllMaterials(); break;
        case 105: _t->DeleteAllBC(); break;
        case 106: _t->DeleteAllLoads(); break;
        case 107: _t->DeleteAllIC(); break;
        case 108: _t->DeleteAllContact(); break;
        case 109: _t->DeleteAllConstraints(); break;
        case 110: _t->DeleteAllConnectors(); break;
        case 111: _t->DeleteAllSteps(); break;
        case 112: { CGLView* _r = _t->GetGLView();
            if (_a[0]) *reinterpret_cast< CGLView**>(_a[0]) = std::move(_r); }  break;
        case 113: _t->changeViewMode((*reinterpret_cast< View_Mode(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CMainWindow::staticMetaObject = { {
    &QMainWindow::staticMetaObject,
    qt_meta_stringdata_CMainWindow.data,
    qt_meta_data_CMainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CMainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CMainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CMainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int CMainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 114)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 114;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 114)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 114;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
