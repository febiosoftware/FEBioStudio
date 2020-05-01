/****************************************************************************
** Meta object code from reading C++ file 'MainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "MainWindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MainWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CMainWindow_t {
    QByteArrayData data[243];
    char stringdata0[5748];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CMainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CMainWindow_t qt_meta_stringdata_CMainWindow = {
    {
QT_MOC_LITERAL(0, 0, 11), // "CMainWindow"
QT_MOC_LITERAL(1, 12, 29), // "on_actionNewProject_triggered"
QT_MOC_LITERAL(2, 42, 0), // ""
QT_MOC_LITERAL(3, 43, 27), // "on_actionNewModel_triggered"
QT_MOC_LITERAL(4, 71, 30), // "on_actionOpenProject_triggered"
QT_MOC_LITERAL(5, 102, 23), // "on_actionOpen_triggered"
QT_MOC_LITERAL(6, 126, 23), // "on_actionSave_triggered"
QT_MOC_LITERAL(7, 150, 25), // "on_actionSaveAs_triggered"
QT_MOC_LITERAL(8, 176, 26), // "on_actionSaveAll_triggered"
QT_MOC_LITERAL(9, 203, 27), // "on_actionSnapShot_triggered"
QT_MOC_LITERAL(10, 231, 32), // "on_actionExportFEModel_triggered"
QT_MOC_LITERAL(11, 264, 33), // "on_actionImportGeometry_trigg..."
QT_MOC_LITERAL(12, 298, 33), // "on_actionExportGeometry_trigg..."
QT_MOC_LITERAL(13, 332, 32), // "on_actionImportProject_triggered"
QT_MOC_LITERAL(14, 365, 32), // "on_actionExportProject_triggered"
QT_MOC_LITERAL(15, 398, 30), // "on_actionImportImage_triggered"
QT_MOC_LITERAL(16, 429, 29), // "on_actionConvertFeb_triggered"
QT_MOC_LITERAL(17, 459, 29), // "on_actionConvertGeo_triggered"
QT_MOC_LITERAL(18, 489, 23), // "on_actionExit_triggered"
QT_MOC_LITERAL(19, 513, 24), // "on_recentFiles_triggered"
QT_MOC_LITERAL(20, 538, 8), // "QAction*"
QT_MOC_LITERAL(21, 547, 6), // "action"
QT_MOC_LITERAL(22, 554, 26), // "on_recentFEFiles_triggered"
QT_MOC_LITERAL(23, 581, 28), // "on_recentGeomFiles_triggered"
QT_MOC_LITERAL(24, 610, 23), // "on_actionUndo_triggered"
QT_MOC_LITERAL(25, 634, 23), // "on_actionRedo_triggered"
QT_MOC_LITERAL(26, 658, 34), // "on_actionInvertSelection_trig..."
QT_MOC_LITERAL(27, 693, 33), // "on_actionClearSelection_trigg..."
QT_MOC_LITERAL(28, 727, 34), // "on_actionDeleteSelection_trig..."
QT_MOC_LITERAL(29, 762, 32), // "on_actionHideSelection_triggered"
QT_MOC_LITERAL(30, 795, 33), // "on_actionHideUnselected_trigg..."
QT_MOC_LITERAL(31, 829, 28), // "on_actionUnhideAll_triggered"
QT_MOC_LITERAL(32, 858, 23), // "on_actionFind_triggered"
QT_MOC_LITERAL(33, 882, 30), // "on_actionSelectRange_triggered"
QT_MOC_LITERAL(34, 913, 32), // "on_actionToggleVisible_triggered"
QT_MOC_LITERAL(35, 946, 32), // "on_actionNameSelection_triggered"
QT_MOC_LITERAL(36, 979, 28), // "on_actionTransform_triggered"
QT_MOC_LITERAL(37, 1008, 36), // "on_actionCollapseTransform_tr..."
QT_MOC_LITERAL(38, 1045, 24), // "on_actionClone_triggered"
QT_MOC_LITERAL(39, 1070, 28), // "on_actionCloneGrid_triggered"
QT_MOC_LITERAL(40, 1099, 31), // "on_actionCloneRevolve_triggered"
QT_MOC_LITERAL(41, 1131, 24), // "on_actionMerge_triggered"
QT_MOC_LITERAL(42, 1156, 24), // "on_actionPurge_triggered"
QT_MOC_LITERAL(43, 1181, 25), // "on_actionDetach_triggered"
QT_MOC_LITERAL(44, 1207, 26), // "on_actionExtract_triggered"
QT_MOC_LITERAL(45, 1234, 30), // "on_actionEditProject_triggered"
QT_MOC_LITERAL(46, 1265, 29), // "on_actionFaceToElem_triggered"
QT_MOC_LITERAL(47, 1295, 24), // "on_actionAddBC_triggered"
QT_MOC_LITERAL(48, 1320, 31), // "on_actionAddNodalLoad_triggered"
QT_MOC_LITERAL(49, 1352, 30), // "on_actionAddSurfLoad_triggered"
QT_MOC_LITERAL(50, 1383, 30), // "on_actionAddBodyLoad_triggered"
QT_MOC_LITERAL(51, 1414, 24), // "on_actionAddIC_triggered"
QT_MOC_LITERAL(52, 1439, 29), // "on_actionAddContact_triggered"
QT_MOC_LITERAL(53, 1469, 32), // "on_actionAddConstraint_triggered"
QT_MOC_LITERAL(54, 1502, 37), // "on_actionAddRigidConstraint_t..."
QT_MOC_LITERAL(55, 1540, 36), // "on_actionAddRigidConnector_tr..."
QT_MOC_LITERAL(56, 1577, 30), // "on_actionAddMaterial_triggered"
QT_MOC_LITERAL(57, 1608, 26), // "on_actionAddStep_triggered"
QT_MOC_LITERAL(58, 1635, 30), // "on_actionAddReaction_triggered"
QT_MOC_LITERAL(59, 1666, 30), // "on_actionSoluteTable_triggered"
QT_MOC_LITERAL(60, 1697, 27), // "on_actionSBMTable_triggered"
QT_MOC_LITERAL(61, 1725, 30), // "on_actionCurveEditor_triggered"
QT_MOC_LITERAL(62, 1756, 32), // "on_actionMeshInspector_triggered"
QT_MOC_LITERAL(63, 1789, 38), // "on_actionElasticityConvertor_..."
QT_MOC_LITERAL(64, 1828, 26), // "on_actionKinemat_triggered"
QT_MOC_LITERAL(65, 1855, 26), // "on_actionPlotMix_triggered"
QT_MOC_LITERAL(66, 1882, 27), // "on_actionFEBioRun_triggered"
QT_MOC_LITERAL(67, 1910, 28), // "on_actionFEBioStop_triggered"
QT_MOC_LITERAL(68, 1939, 32), // "on_actionFEBioOptimize_triggered"
QT_MOC_LITERAL(69, 1972, 26), // "on_actionOptions_triggered"
QT_MOC_LITERAL(70, 1999, 28), // "on_actionLayerInfo_triggered"
QT_MOC_LITERAL(71, 2028, 27), // "on_actionPlaneCut_triggered"
QT_MOC_LITERAL(72, 2056, 30), // "on_actionMirrorPlane_triggered"
QT_MOC_LITERAL(73, 2087, 29), // "on_actionVectorPlot_triggered"
QT_MOC_LITERAL(74, 2117, 29), // "on_actionTensorPlot_triggered"
QT_MOC_LITERAL(75, 2147, 33), // "on_actionIsosurfacePlot_trigg..."
QT_MOC_LITERAL(76, 2181, 28), // "on_actionSlicePlot_triggered"
QT_MOC_LITERAL(77, 2210, 34), // "on_actionDisplacementMap_trig..."
QT_MOC_LITERAL(78, 2245, 33), // "on_actionStreamLinePlot_trigg..."
QT_MOC_LITERAL(79, 2279, 35), // "on_actionParticleFlowPlot_tri..."
QT_MOC_LITERAL(80, 2315, 33), // "on_actionVolumeFlowPlot_trigg..."
QT_MOC_LITERAL(81, 2349, 30), // "on_actionImageSlicer_triggered"
QT_MOC_LITERAL(82, 2380, 31), // "on_actionVolumeRender_triggered"
QT_MOC_LITERAL(83, 2412, 32), // "on_actionMarchingCubes_triggered"
QT_MOC_LITERAL(84, 2445, 24), // "on_actionGraph_triggered"
QT_MOC_LITERAL(85, 2470, 26), // "on_actionSummary_triggered"
QT_MOC_LITERAL(86, 2497, 24), // "on_actionStats_triggered"
QT_MOC_LITERAL(87, 2522, 28), // "on_actionIntegrate_triggered"
QT_MOC_LITERAL(88, 2551, 31), // "on_actionImportPoints_triggered"
QT_MOC_LITERAL(89, 2583, 30), // "on_actionImportLines_triggered"
QT_MOC_LITERAL(90, 2614, 28), // "on_actionRecordNew_triggered"
QT_MOC_LITERAL(91, 2643, 30), // "on_actionRecordStart_triggered"
QT_MOC_LITERAL(92, 2674, 30), // "on_actionRecordPause_triggered"
QT_MOC_LITERAL(93, 2705, 29), // "on_actionRecordStop_triggered"
QT_MOC_LITERAL(94, 2735, 33), // "on_actionUndoViewChange_trigg..."
QT_MOC_LITERAL(95, 2769, 33), // "on_actionRedoViewChange_trigg..."
QT_MOC_LITERAL(96, 2803, 29), // "on_actionZoomSelect_triggered"
QT_MOC_LITERAL(97, 2833, 30), // "on_actionZoomExtents_triggered"
QT_MOC_LITERAL(98, 2864, 28), // "on_actionViewCapture_toggled"
QT_MOC_LITERAL(99, 2893, 8), // "bchecked"
QT_MOC_LITERAL(100, 2902, 22), // "on_actionOrtho_toggled"
QT_MOC_LITERAL(101, 2925, 1), // "b"
QT_MOC_LITERAL(102, 2927, 25), // "on_actionShowGrid_toggled"
QT_MOC_LITERAL(103, 2953, 30), // "on_actionShowMeshLines_toggled"
QT_MOC_LITERAL(104, 2984, 30), // "on_actionShowEdgeLines_toggled"
QT_MOC_LITERAL(105, 3015, 32), // "on_actionBackfaceCulling_toggled"
QT_MOC_LITERAL(106, 3048, 27), // "on_actionViewSmooth_toggled"
QT_MOC_LITERAL(107, 3076, 28), // "on_actionShowNormals_toggled"
QT_MOC_LITERAL(108, 3105, 27), // "on_actionShowFibers_toggled"
QT_MOC_LITERAL(109, 3133, 28), // "on_actionShowMatAxes_toggled"
QT_MOC_LITERAL(110, 3162, 29), // "on_actionShowDiscrete_toggled"
QT_MOC_LITERAL(111, 3192, 30), // "on_actionToggleLight_triggered"
QT_MOC_LITERAL(112, 3223, 24), // "on_actionFront_triggered"
QT_MOC_LITERAL(113, 3248, 23), // "on_actionBack_triggered"
QT_MOC_LITERAL(114, 3272, 23), // "on_actionLeft_triggered"
QT_MOC_LITERAL(115, 3296, 24), // "on_actionRight_triggered"
QT_MOC_LITERAL(116, 3321, 22), // "on_actionTop_triggered"
QT_MOC_LITERAL(117, 3344, 25), // "on_actionBottom_triggered"
QT_MOC_LITERAL(118, 3370, 26), // "on_actionWireframe_toggled"
QT_MOC_LITERAL(119, 3397, 25), // "on_actionSnap3D_triggered"
QT_MOC_LITERAL(120, 3423, 22), // "on_actionTrack_toggled"
QT_MOC_LITERAL(121, 3446, 29), // "on_actionViewVPSave_triggered"
QT_MOC_LITERAL(122, 3476, 29), // "on_actionViewVPPrev_triggered"
QT_MOC_LITERAL(123, 3506, 29), // "on_actionViewVPNext_triggered"
QT_MOC_LITERAL(124, 3536, 28), // "on_actionSyncViews_triggered"
QT_MOC_LITERAL(125, 3565, 29), // "on_actionOnlineHelp_triggered"
QT_MOC_LITERAL(126, 3595, 27), // "on_actionFEBioURL_triggered"
QT_MOC_LITERAL(127, 3623, 29), // "on_actionFEBioForum_triggered"
QT_MOC_LITERAL(128, 3653, 33), // "on_actionFEBioResources_trigg..."
QT_MOC_LITERAL(129, 3687, 28), // "on_actionFEBioPubs_triggered"
QT_MOC_LITERAL(130, 3716, 24), // "on_actionAbout_triggered"
QT_MOC_LITERAL(131, 3741, 23), // "on_actionSelect_toggled"
QT_MOC_LITERAL(132, 3765, 26), // "on_actionTranslate_toggled"
QT_MOC_LITERAL(133, 3792, 23), // "on_actionRotate_toggled"
QT_MOC_LITERAL(134, 3816, 22), // "on_actionScale_toggled"
QT_MOC_LITERAL(135, 3839, 34), // "on_selectCoord_currentIndexCh..."
QT_MOC_LITERAL(136, 3874, 1), // "n"
QT_MOC_LITERAL(137, 3876, 30), // "on_actionSelectObjects_toggled"
QT_MOC_LITERAL(138, 3907, 28), // "on_actionSelectParts_toggled"
QT_MOC_LITERAL(139, 3936, 31), // "on_actionSelectSurfaces_toggled"
QT_MOC_LITERAL(140, 3968, 29), // "on_actionSelectCurves_toggled"
QT_MOC_LITERAL(141, 3998, 28), // "on_actionSelectNodes_toggled"
QT_MOC_LITERAL(142, 4027, 31), // "on_actionSelectDiscrete_toggled"
QT_MOC_LITERAL(143, 4059, 21), // "on_selectRect_toggled"
QT_MOC_LITERAL(144, 4081, 23), // "on_selectCircle_toggled"
QT_MOC_LITERAL(145, 4105, 21), // "on_selectFree_toggled"
QT_MOC_LITERAL(146, 4127, 30), // "on_actionMeasureTool_triggered"
QT_MOC_LITERAL(147, 4158, 25), // "on_postSelectRect_toggled"
QT_MOC_LITERAL(148, 4184, 27), // "on_postSelectCircle_toggled"
QT_MOC_LITERAL(149, 4212, 25), // "on_postSelectFree_toggled"
QT_MOC_LITERAL(150, 4238, 34), // "on_postActionMeasureTool_trig..."
QT_MOC_LITERAL(151, 4273, 33), // "on_selectData_currentValueCha..."
QT_MOC_LITERAL(152, 4307, 1), // "i"
QT_MOC_LITERAL(153, 4309, 21), // "on_actionPlay_toggled"
QT_MOC_LITERAL(154, 4331, 26), // "on_actionRefresh_triggered"
QT_MOC_LITERAL(155, 4358, 24), // "on_actionFirst_triggered"
QT_MOC_LITERAL(156, 4383, 23), // "on_actionPrev_triggered"
QT_MOC_LITERAL(157, 4407, 23), // "on_actionNext_triggered"
QT_MOC_LITERAL(158, 4431, 23), // "on_actionLast_triggered"
QT_MOC_LITERAL(159, 4455, 31), // "on_actionTimeSettings_triggered"
QT_MOC_LITERAL(160, 4487, 25), // "on_actionColorMap_toggled"
QT_MOC_LITERAL(161, 4513, 26), // "on_selectTime_valueChanged"
QT_MOC_LITERAL(162, 4540, 31), // "on_fontStyle_currentFontChanged"
QT_MOC_LITERAL(163, 4572, 4), // "font"
QT_MOC_LITERAL(164, 4577, 24), // "on_fontSize_valueChanged"
QT_MOC_LITERAL(165, 4602, 19), // "on_fontBold_toggled"
QT_MOC_LITERAL(166, 4622, 7), // "checked"
QT_MOC_LITERAL(167, 4630, 21), // "on_fontItalic_toggled"
QT_MOC_LITERAL(168, 4652, 29), // "on_actionProperties_triggered"
QT_MOC_LITERAL(169, 4682, 21), // "on_tab_currentChanged"
QT_MOC_LITERAL(170, 4704, 24), // "on_tab_tabCloseRequested"
QT_MOC_LITERAL(171, 4729, 24), // "OnPostObjectStateChanged"
QT_MOC_LITERAL(172, 4754, 24), // "OnPostObjectPropsChanged"
QT_MOC_LITERAL(173, 4779, 9), // "FSObject*"
QT_MOC_LITERAL(174, 4789, 2), // "po"
QT_MOC_LITERAL(175, 4792, 35), // "on_modelViewer_currentObjectC..."
QT_MOC_LITERAL(176, 4828, 17), // "OnSelectMeshLayer"
QT_MOC_LITERAL(177, 4846, 2), // "ac"
QT_MOC_LITERAL(178, 4849, 30), // "OnSelectObjectTransparencyMode"
QT_MOC_LITERAL(179, 4880, 9), // "CloseView"
QT_MOC_LITERAL(180, 4890, 10), // "CDocument*"
QT_MOC_LITERAL(181, 4901, 3), // "doc"
QT_MOC_LITERAL(182, 4905, 15), // "SetCurrentState"
QT_MOC_LITERAL(183, 4921, 10), // "closeEvent"
QT_MOC_LITERAL(184, 4932, 12), // "QCloseEvent*"
QT_MOC_LITERAL(185, 4945, 2), // "ev"
QT_MOC_LITERAL(186, 4948, 13), // "keyPressEvent"
QT_MOC_LITERAL(187, 4962, 10), // "QKeyEvent*"
QT_MOC_LITERAL(188, 4973, 22), // "on_finishedReadingFile"
QT_MOC_LITERAL(189, 4996, 7), // "success"
QT_MOC_LITERAL(190, 5004, 11), // "errorString"
QT_MOC_LITERAL(191, 5016, 19), // "finishedReadingFile"
QT_MOC_LITERAL(192, 5036, 11), // "QueuedFile&"
QT_MOC_LITERAL(193, 5048, 5), // "qfile"
QT_MOC_LITERAL(194, 5054, 17), // "checkFileProgress"
QT_MOC_LITERAL(195, 5072, 13), // "StopAnimation"
QT_MOC_LITERAL(196, 5086, 7), // "onTimer"
QT_MOC_LITERAL(197, 5094, 21), // "on_glview_pointPicked"
QT_MOC_LITERAL(198, 5116, 5), // "vec3d"
QT_MOC_LITERAL(199, 5122, 1), // "r"
QT_MOC_LITERAL(200, 5124, 26), // "on_glview_selectionChanged"
QT_MOC_LITERAL(201, 5151, 13), // "onRunFinished"
QT_MOC_LITERAL(202, 5165, 8), // "exitCode"
QT_MOC_LITERAL(203, 5174, 20), // "QProcess::ExitStatus"
QT_MOC_LITERAL(204, 5195, 2), // "es"
QT_MOC_LITERAL(205, 5198, 11), // "onReadyRead"
QT_MOC_LITERAL(206, 5210, 15), // "onErrorOccurred"
QT_MOC_LITERAL(207, 5226, 22), // "QProcess::ProcessError"
QT_MOC_LITERAL(208, 5249, 3), // "err"
QT_MOC_LITERAL(209, 5253, 17), // "onExportMaterials"
QT_MOC_LITERAL(210, 5271, 18), // "vector<GMaterial*>"
QT_MOC_LITERAL(211, 5290, 7), // "matList"
QT_MOC_LITERAL(212, 5298, 20), // "onExportAllMaterials"
QT_MOC_LITERAL(213, 5319, 17), // "onImportMaterials"
QT_MOC_LITERAL(214, 5337, 18), // "DeleteAllMaterials"
QT_MOC_LITERAL(215, 5356, 11), // "DeleteAllBC"
QT_MOC_LITERAL(216, 5368, 14), // "DeleteAllLoads"
QT_MOC_LITERAL(217, 5383, 11), // "DeleteAllIC"
QT_MOC_LITERAL(218, 5395, 16), // "DeleteAllContact"
QT_MOC_LITERAL(219, 5412, 20), // "DeleteAllConstraints"
QT_MOC_LITERAL(220, 5433, 25), // "DeleteAllRigidConstraints"
QT_MOC_LITERAL(221, 5459, 24), // "DeleteAllRigidConnectors"
QT_MOC_LITERAL(222, 5484, 14), // "DeleteAllSteps"
QT_MOC_LITERAL(223, 5499, 9), // "GetGLView"
QT_MOC_LITERAL(224, 5509, 8), // "CGLView*"
QT_MOC_LITERAL(225, 5518, 14), // "changeViewMode"
QT_MOC_LITERAL(226, 5533, 9), // "View_Mode"
QT_MOC_LITERAL(227, 5543, 2), // "vm"
QT_MOC_LITERAL(228, 5546, 15), // "GetCurrentModel"
QT_MOC_LITERAL(229, 5562, 15), // "Post::CGLModel*"
QT_MOC_LITERAL(230, 5578, 17), // "UpdateFontToolbar"
QT_MOC_LITERAL(231, 5596, 11), // "RunFEBioJob"
QT_MOC_LITERAL(232, 5608, 10), // "CFEBioJob*"
QT_MOC_LITERAL(233, 5619, 3), // "job"
QT_MOC_LITERAL(234, 5623, 15), // "NextSSHFunction"
QT_MOC_LITERAL(235, 5639, 12), // "CSSHHandler*"
QT_MOC_LITERAL(236, 5652, 12), // "ShowProgress"
QT_MOC_LITERAL(237, 5665, 4), // "show"
QT_MOC_LITERAL(238, 5670, 7), // "message"
QT_MOC_LITERAL(239, 5678, 25), // "ShowIndeterminateProgress"
QT_MOC_LITERAL(240, 5704, 14), // "UpdateProgress"
QT_MOC_LITERAL(241, 5719, 12), // "DoModelCheck"
QT_MOC_LITERAL(242, 5732, 15) // "CModelDocument*"

    },
    "CMainWindow\0on_actionNewProject_triggered\0"
    "\0on_actionNewModel_triggered\0"
    "on_actionOpenProject_triggered\0"
    "on_actionOpen_triggered\0on_actionSave_triggered\0"
    "on_actionSaveAs_triggered\0"
    "on_actionSaveAll_triggered\0"
    "on_actionSnapShot_triggered\0"
    "on_actionExportFEModel_triggered\0"
    "on_actionImportGeometry_triggered\0"
    "on_actionExportGeometry_triggered\0"
    "on_actionImportProject_triggered\0"
    "on_actionExportProject_triggered\0"
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
    "on_actionFind_triggered\0"
    "on_actionSelectRange_triggered\0"
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
    "on_actionFaceToElem_triggered\0"
    "on_actionAddBC_triggered\0"
    "on_actionAddNodalLoad_triggered\0"
    "on_actionAddSurfLoad_triggered\0"
    "on_actionAddBodyLoad_triggered\0"
    "on_actionAddIC_triggered\0"
    "on_actionAddContact_triggered\0"
    "on_actionAddConstraint_triggered\0"
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
    "on_actionKinemat_triggered\0"
    "on_actionPlotMix_triggered\0"
    "on_actionFEBioRun_triggered\0"
    "on_actionFEBioStop_triggered\0"
    "on_actionFEBioOptimize_triggered\0"
    "on_actionOptions_triggered\0"
    "on_actionLayerInfo_triggered\0"
    "on_actionPlaneCut_triggered\0"
    "on_actionMirrorPlane_triggered\0"
    "on_actionVectorPlot_triggered\0"
    "on_actionTensorPlot_triggered\0"
    "on_actionIsosurfacePlot_triggered\0"
    "on_actionSlicePlot_triggered\0"
    "on_actionDisplacementMap_triggered\0"
    "on_actionStreamLinePlot_triggered\0"
    "on_actionParticleFlowPlot_triggered\0"
    "on_actionVolumeFlowPlot_triggered\0"
    "on_actionImageSlicer_triggered\0"
    "on_actionVolumeRender_triggered\0"
    "on_actionMarchingCubes_triggered\0"
    "on_actionGraph_triggered\0"
    "on_actionSummary_triggered\0"
    "on_actionStats_triggered\0"
    "on_actionIntegrate_triggered\0"
    "on_actionImportPoints_triggered\0"
    "on_actionImportLines_triggered\0"
    "on_actionRecordNew_triggered\0"
    "on_actionRecordStart_triggered\0"
    "on_actionRecordPause_triggered\0"
    "on_actionRecordStop_triggered\0"
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
    "on_actionViewSmooth_toggled\0"
    "on_actionShowNormals_toggled\0"
    "on_actionShowFibers_toggled\0"
    "on_actionShowMatAxes_toggled\0"
    "on_actionShowDiscrete_toggled\0"
    "on_actionToggleLight_triggered\0"
    "on_actionFront_triggered\0"
    "on_actionBack_triggered\0on_actionLeft_triggered\0"
    "on_actionRight_triggered\0"
    "on_actionTop_triggered\0on_actionBottom_triggered\0"
    "on_actionWireframe_toggled\0"
    "on_actionSnap3D_triggered\0"
    "on_actionTrack_toggled\0"
    "on_actionViewVPSave_triggered\0"
    "on_actionViewVPPrev_triggered\0"
    "on_actionViewVPNext_triggered\0"
    "on_actionSyncViews_triggered\0"
    "on_actionOnlineHelp_triggered\0"
    "on_actionFEBioURL_triggered\0"
    "on_actionFEBioForum_triggered\0"
    "on_actionFEBioResources_triggered\0"
    "on_actionFEBioPubs_triggered\0"
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
    "on_selectFree_toggled\0"
    "on_actionMeasureTool_triggered\0"
    "on_postSelectRect_toggled\0"
    "on_postSelectCircle_toggled\0"
    "on_postSelectFree_toggled\0"
    "on_postActionMeasureTool_triggered\0"
    "on_selectData_currentValueChanged\0i\0"
    "on_actionPlay_toggled\0on_actionRefresh_triggered\0"
    "on_actionFirst_triggered\0"
    "on_actionPrev_triggered\0on_actionNext_triggered\0"
    "on_actionLast_triggered\0"
    "on_actionTimeSettings_triggered\0"
    "on_actionColorMap_toggled\0"
    "on_selectTime_valueChanged\0"
    "on_fontStyle_currentFontChanged\0font\0"
    "on_fontSize_valueChanged\0on_fontBold_toggled\0"
    "checked\0on_fontItalic_toggled\0"
    "on_actionProperties_triggered\0"
    "on_tab_currentChanged\0on_tab_tabCloseRequested\0"
    "OnPostObjectStateChanged\0"
    "OnPostObjectPropsChanged\0FSObject*\0"
    "po\0on_modelViewer_currentObjectChanged\0"
    "OnSelectMeshLayer\0ac\0"
    "OnSelectObjectTransparencyMode\0CloseView\0"
    "CDocument*\0doc\0SetCurrentState\0"
    "closeEvent\0QCloseEvent*\0ev\0keyPressEvent\0"
    "QKeyEvent*\0on_finishedReadingFile\0"
    "success\0errorString\0finishedReadingFile\0"
    "QueuedFile&\0qfile\0checkFileProgress\0"
    "StopAnimation\0onTimer\0on_glview_pointPicked\0"
    "vec3d\0r\0on_glview_selectionChanged\0"
    "onRunFinished\0exitCode\0QProcess::ExitStatus\0"
    "es\0onReadyRead\0onErrorOccurred\0"
    "QProcess::ProcessError\0err\0onExportMaterials\0"
    "vector<GMaterial*>\0matList\0"
    "onExportAllMaterials\0onImportMaterials\0"
    "DeleteAllMaterials\0DeleteAllBC\0"
    "DeleteAllLoads\0DeleteAllIC\0DeleteAllContact\0"
    "DeleteAllConstraints\0DeleteAllRigidConstraints\0"
    "DeleteAllRigidConnectors\0DeleteAllSteps\0"
    "GetGLView\0CGLView*\0changeViewMode\0"
    "View_Mode\0vm\0GetCurrentModel\0"
    "Post::CGLModel*\0UpdateFontToolbar\0"
    "RunFEBioJob\0CFEBioJob*\0job\0NextSSHFunction\0"
    "CSSHHandler*\0ShowProgress\0show\0message\0"
    "ShowIndeterminateProgress\0UpdateProgress\0"
    "DoModelCheck\0CModelDocument*"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CMainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
     205,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0, 1039,    2, 0x0a /* Public */,
       3,    0, 1040,    2, 0x0a /* Public */,
       4,    0, 1041,    2, 0x0a /* Public */,
       5,    0, 1042,    2, 0x0a /* Public */,
       6,    0, 1043,    2, 0x0a /* Public */,
       7,    0, 1044,    2, 0x0a /* Public */,
       8,    0, 1045,    2, 0x0a /* Public */,
       9,    0, 1046,    2, 0x0a /* Public */,
      10,    0, 1047,    2, 0x0a /* Public */,
      11,    0, 1048,    2, 0x0a /* Public */,
      12,    0, 1049,    2, 0x0a /* Public */,
      13,    0, 1050,    2, 0x0a /* Public */,
      14,    0, 1051,    2, 0x0a /* Public */,
      15,    0, 1052,    2, 0x0a /* Public */,
      16,    0, 1053,    2, 0x0a /* Public */,
      17,    0, 1054,    2, 0x0a /* Public */,
      18,    0, 1055,    2, 0x0a /* Public */,
      19,    1, 1056,    2, 0x0a /* Public */,
      22,    1, 1059,    2, 0x0a /* Public */,
      23,    1, 1062,    2, 0x0a /* Public */,
      24,    0, 1065,    2, 0x0a /* Public */,
      25,    0, 1066,    2, 0x0a /* Public */,
      26,    0, 1067,    2, 0x0a /* Public */,
      27,    0, 1068,    2, 0x0a /* Public */,
      28,    0, 1069,    2, 0x0a /* Public */,
      29,    0, 1070,    2, 0x0a /* Public */,
      30,    0, 1071,    2, 0x0a /* Public */,
      31,    0, 1072,    2, 0x0a /* Public */,
      32,    0, 1073,    2, 0x0a /* Public */,
      33,    0, 1074,    2, 0x0a /* Public */,
      34,    0, 1075,    2, 0x0a /* Public */,
      35,    0, 1076,    2, 0x0a /* Public */,
      36,    0, 1077,    2, 0x0a /* Public */,
      37,    0, 1078,    2, 0x0a /* Public */,
      38,    0, 1079,    2, 0x0a /* Public */,
      39,    0, 1080,    2, 0x0a /* Public */,
      40,    0, 1081,    2, 0x0a /* Public */,
      41,    0, 1082,    2, 0x0a /* Public */,
      42,    0, 1083,    2, 0x0a /* Public */,
      43,    0, 1084,    2, 0x0a /* Public */,
      44,    0, 1085,    2, 0x0a /* Public */,
      45,    0, 1086,    2, 0x0a /* Public */,
      46,    0, 1087,    2, 0x0a /* Public */,
      47,    0, 1088,    2, 0x0a /* Public */,
      48,    0, 1089,    2, 0x0a /* Public */,
      49,    0, 1090,    2, 0x0a /* Public */,
      50,    0, 1091,    2, 0x0a /* Public */,
      51,    0, 1092,    2, 0x0a /* Public */,
      52,    0, 1093,    2, 0x0a /* Public */,
      53,    0, 1094,    2, 0x0a /* Public */,
      54,    0, 1095,    2, 0x0a /* Public */,
      55,    0, 1096,    2, 0x0a /* Public */,
      56,    0, 1097,    2, 0x0a /* Public */,
      57,    0, 1098,    2, 0x0a /* Public */,
      58,    0, 1099,    2, 0x0a /* Public */,
      59,    0, 1100,    2, 0x0a /* Public */,
      60,    0, 1101,    2, 0x0a /* Public */,
      61,    0, 1102,    2, 0x0a /* Public */,
      62,    0, 1103,    2, 0x0a /* Public */,
      63,    0, 1104,    2, 0x0a /* Public */,
      64,    0, 1105,    2, 0x0a /* Public */,
      65,    0, 1106,    2, 0x0a /* Public */,
      66,    0, 1107,    2, 0x0a /* Public */,
      67,    0, 1108,    2, 0x0a /* Public */,
      68,    0, 1109,    2, 0x0a /* Public */,
      69,    0, 1110,    2, 0x0a /* Public */,
      70,    0, 1111,    2, 0x0a /* Public */,
      71,    0, 1112,    2, 0x0a /* Public */,
      72,    0, 1113,    2, 0x0a /* Public */,
      73,    0, 1114,    2, 0x0a /* Public */,
      74,    0, 1115,    2, 0x0a /* Public */,
      75,    0, 1116,    2, 0x0a /* Public */,
      76,    0, 1117,    2, 0x0a /* Public */,
      77,    0, 1118,    2, 0x0a /* Public */,
      78,    0, 1119,    2, 0x0a /* Public */,
      79,    0, 1120,    2, 0x0a /* Public */,
      80,    0, 1121,    2, 0x0a /* Public */,
      81,    0, 1122,    2, 0x0a /* Public */,
      82,    0, 1123,    2, 0x0a /* Public */,
      83,    0, 1124,    2, 0x0a /* Public */,
      84,    0, 1125,    2, 0x0a /* Public */,
      85,    0, 1126,    2, 0x0a /* Public */,
      86,    0, 1127,    2, 0x0a /* Public */,
      87,    0, 1128,    2, 0x0a /* Public */,
      88,    0, 1129,    2, 0x0a /* Public */,
      89,    0, 1130,    2, 0x0a /* Public */,
      90,    0, 1131,    2, 0x0a /* Public */,
      91,    0, 1132,    2, 0x0a /* Public */,
      92,    0, 1133,    2, 0x0a /* Public */,
      93,    0, 1134,    2, 0x0a /* Public */,
      94,    0, 1135,    2, 0x0a /* Public */,
      95,    0, 1136,    2, 0x0a /* Public */,
      96,    0, 1137,    2, 0x0a /* Public */,
      97,    0, 1138,    2, 0x0a /* Public */,
      98,    1, 1139,    2, 0x0a /* Public */,
     100,    1, 1142,    2, 0x0a /* Public */,
     102,    1, 1145,    2, 0x0a /* Public */,
     103,    1, 1148,    2, 0x0a /* Public */,
     104,    1, 1151,    2, 0x0a /* Public */,
     105,    1, 1154,    2, 0x0a /* Public */,
     106,    1, 1157,    2, 0x0a /* Public */,
     107,    1, 1160,    2, 0x0a /* Public */,
     108,    1, 1163,    2, 0x0a /* Public */,
     109,    1, 1166,    2, 0x0a /* Public */,
     110,    1, 1169,    2, 0x0a /* Public */,
     111,    0, 1172,    2, 0x0a /* Public */,
     112,    0, 1173,    2, 0x0a /* Public */,
     113,    0, 1174,    2, 0x0a /* Public */,
     114,    0, 1175,    2, 0x0a /* Public */,
     115,    0, 1176,    2, 0x0a /* Public */,
     116,    0, 1177,    2, 0x0a /* Public */,
     117,    0, 1178,    2, 0x0a /* Public */,
     118,    1, 1179,    2, 0x0a /* Public */,
     119,    0, 1182,    2, 0x0a /* Public */,
     120,    1, 1183,    2, 0x0a /* Public */,
     121,    0, 1186,    2, 0x0a /* Public */,
     122,    0, 1187,    2, 0x0a /* Public */,
     123,    0, 1188,    2, 0x0a /* Public */,
     124,    0, 1189,    2, 0x0a /* Public */,
     125,    0, 1190,    2, 0x0a /* Public */,
     126,    0, 1191,    2, 0x0a /* Public */,
     127,    0, 1192,    2, 0x0a /* Public */,
     128,    0, 1193,    2, 0x0a /* Public */,
     129,    0, 1194,    2, 0x0a /* Public */,
     130,    0, 1195,    2, 0x0a /* Public */,
     131,    1, 1196,    2, 0x0a /* Public */,
     132,    1, 1199,    2, 0x0a /* Public */,
     133,    1, 1202,    2, 0x0a /* Public */,
     134,    1, 1205,    2, 0x0a /* Public */,
     135,    1, 1208,    2, 0x0a /* Public */,
     137,    1, 1211,    2, 0x0a /* Public */,
     138,    1, 1214,    2, 0x0a /* Public */,
     139,    1, 1217,    2, 0x0a /* Public */,
     140,    1, 1220,    2, 0x0a /* Public */,
     141,    1, 1223,    2, 0x0a /* Public */,
     142,    1, 1226,    2, 0x0a /* Public */,
     143,    1, 1229,    2, 0x0a /* Public */,
     144,    1, 1232,    2, 0x0a /* Public */,
     145,    1, 1235,    2, 0x0a /* Public */,
     146,    0, 1238,    2, 0x0a /* Public */,
     147,    1, 1239,    2, 0x0a /* Public */,
     148,    1, 1242,    2, 0x0a /* Public */,
     149,    1, 1245,    2, 0x0a /* Public */,
     150,    0, 1248,    2, 0x0a /* Public */,
     151,    1, 1249,    2, 0x0a /* Public */,
     153,    1, 1252,    2, 0x0a /* Public */,
     154,    0, 1255,    2, 0x0a /* Public */,
     155,    0, 1256,    2, 0x0a /* Public */,
     156,    0, 1257,    2, 0x0a /* Public */,
     157,    0, 1258,    2, 0x0a /* Public */,
     158,    0, 1259,    2, 0x0a /* Public */,
     159,    0, 1260,    2, 0x0a /* Public */,
     160,    1, 1261,    2, 0x0a /* Public */,
     161,    1, 1264,    2, 0x0a /* Public */,
     162,    1, 1267,    2, 0x0a /* Public */,
     164,    1, 1270,    2, 0x0a /* Public */,
     165,    1, 1273,    2, 0x0a /* Public */,
     167,    1, 1276,    2, 0x0a /* Public */,
     168,    0, 1279,    2, 0x0a /* Public */,
     169,    1, 1280,    2, 0x0a /* Public */,
     170,    1, 1283,    2, 0x0a /* Public */,
     171,    0, 1286,    2, 0x0a /* Public */,
     172,    1, 1287,    2, 0x0a /* Public */,
     175,    1, 1290,    2, 0x0a /* Public */,
     176,    1, 1293,    2, 0x0a /* Public */,
     178,    1, 1296,    2, 0x0a /* Public */,
     179,    1, 1299,    2, 0x0a /* Public */,
     179,    1, 1302,    2, 0x0a /* Public */,
     182,    1, 1305,    2, 0x0a /* Public */,
     183,    1, 1308,    2, 0x0a /* Public */,
     186,    1, 1311,    2, 0x0a /* Public */,
     188,    2, 1314,    2, 0x0a /* Public */,
     191,    3, 1319,    2, 0x0a /* Public */,
     194,    0, 1326,    2, 0x0a /* Public */,
     195,    0, 1327,    2, 0x0a /* Public */,
     196,    0, 1328,    2, 0x0a /* Public */,
     197,    1, 1329,    2, 0x0a /* Public */,
     200,    0, 1332,    2, 0x0a /* Public */,
     201,    2, 1333,    2, 0x0a /* Public */,
     205,    0, 1338,    2, 0x0a /* Public */,
     206,    1, 1339,    2, 0x0a /* Public */,
     209,    1, 1342,    2, 0x0a /* Public */,
     212,    0, 1345,    2, 0x0a /* Public */,
     213,    0, 1346,    2, 0x0a /* Public */,
     214,    0, 1347,    2, 0x0a /* Public */,
     215,    0, 1348,    2, 0x0a /* Public */,
     216,    0, 1349,    2, 0x0a /* Public */,
     217,    0, 1350,    2, 0x0a /* Public */,
     218,    0, 1351,    2, 0x0a /* Public */,
     219,    0, 1352,    2, 0x0a /* Public */,
     220,    0, 1353,    2, 0x0a /* Public */,
     221,    0, 1354,    2, 0x0a /* Public */,
     222,    0, 1355,    2, 0x0a /* Public */,
     223,    0, 1356,    2, 0x0a /* Public */,
     225,    1, 1357,    2, 0x0a /* Public */,
     228,    0, 1360,    2, 0x0a /* Public */,
     230,    0, 1361,    2, 0x0a /* Public */,
     231,    1, 1362,    2, 0x0a /* Public */,
     234,    1, 1365,    2, 0x0a /* Public */,
     236,    2, 1368,    2, 0x0a /* Public */,
     236,    1, 1373,    2, 0x2a /* Public | MethodCloned */,
     239,    2, 1376,    2, 0x0a /* Public */,
     239,    1, 1381,    2, 0x2a /* Public | MethodCloned */,
     240,    1, 1384,    2, 0x0a /* Public */,
     241,    1, 1387,    2, 0x0a /* Public */,

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
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 20,   21,
    QMetaType::Void, 0x80000000 | 20,   21,
    QMetaType::Void, 0x80000000 | 20,   21,
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
    QMetaType::Void, QMetaType::Bool,   99,
    QMetaType::Void, QMetaType::Bool,  101,
    QMetaType::Void, QMetaType::Bool,  101,
    QMetaType::Void, QMetaType::Bool,  101,
    QMetaType::Void, QMetaType::Bool,  101,
    QMetaType::Void, QMetaType::Bool,  101,
    QMetaType::Void, QMetaType::Bool,   99,
    QMetaType::Void, QMetaType::Bool,  101,
    QMetaType::Void, QMetaType::Bool,  101,
    QMetaType::Void, QMetaType::Bool,  101,
    QMetaType::Void, QMetaType::Bool,  101,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,  101,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,  101,
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
    QMetaType::Void, QMetaType::Bool,  101,
    QMetaType::Void, QMetaType::Bool,  101,
    QMetaType::Void, QMetaType::Bool,  101,
    QMetaType::Void, QMetaType::Bool,  101,
    QMetaType::Void, QMetaType::Int,  136,
    QMetaType::Void, QMetaType::Bool,  101,
    QMetaType::Void, QMetaType::Bool,  101,
    QMetaType::Void, QMetaType::Bool,  101,
    QMetaType::Void, QMetaType::Bool,  101,
    QMetaType::Void, QMetaType::Bool,  101,
    QMetaType::Void, QMetaType::Bool,  101,
    QMetaType::Void, QMetaType::Bool,  101,
    QMetaType::Void, QMetaType::Bool,  101,
    QMetaType::Void, QMetaType::Bool,  101,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,  101,
    QMetaType::Void, QMetaType::Bool,  101,
    QMetaType::Void, QMetaType::Bool,  101,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,  152,
    QMetaType::Void, QMetaType::Bool,   99,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   99,
    QMetaType::Void, QMetaType::Int,  136,
    QMetaType::Void, QMetaType::QFont,  163,
    QMetaType::Void, QMetaType::Int,  152,
    QMetaType::Void, QMetaType::Bool,  166,
    QMetaType::Void, QMetaType::Bool,   99,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,  136,
    QMetaType::Void, QMetaType::Int,  136,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 173,  174,
    QMetaType::Void, 0x80000000 | 173,  174,
    QMetaType::Void, 0x80000000 | 20,  177,
    QMetaType::Void, 0x80000000 | 20,  177,
    QMetaType::Void, QMetaType::Int,  136,
    QMetaType::Void, 0x80000000 | 180,  181,
    QMetaType::Void, QMetaType::Int,  136,
    QMetaType::Void, 0x80000000 | 184,  185,
    QMetaType::Void, 0x80000000 | 187,  185,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,  189,  190,
    QMetaType::Void, QMetaType::Bool, 0x80000000 | 192, QMetaType::QString,  189,  193,  190,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 198,  199,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 203,  202,  204,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 207,  208,
    QMetaType::Void, 0x80000000 | 210,  211,
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
    0x80000000 | 224,
    QMetaType::Void, 0x80000000 | 226,  227,
    0x80000000 | 229,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 232,  233,
    QMetaType::Void, 0x80000000 | 235,    2,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,  237,  238,
    QMetaType::Void, QMetaType::Bool,  237,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,  237,  238,
    QMetaType::Void, QMetaType::Bool,  237,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Bool, 0x80000000 | 242,  181,

       0        // eod
};

void CMainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CMainWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_actionNewProject_triggered(); break;
        case 1: _t->on_actionNewModel_triggered(); break;
        case 2: _t->on_actionOpenProject_triggered(); break;
        case 3: _t->on_actionOpen_triggered(); break;
        case 4: _t->on_actionSave_triggered(); break;
        case 5: _t->on_actionSaveAs_triggered(); break;
        case 6: _t->on_actionSaveAll_triggered(); break;
        case 7: _t->on_actionSnapShot_triggered(); break;
        case 8: _t->on_actionExportFEModel_triggered(); break;
        case 9: _t->on_actionImportGeometry_triggered(); break;
        case 10: _t->on_actionExportGeometry_triggered(); break;
        case 11: _t->on_actionImportProject_triggered(); break;
        case 12: _t->on_actionExportProject_triggered(); break;
        case 13: _t->on_actionImportImage_triggered(); break;
        case 14: _t->on_actionConvertFeb_triggered(); break;
        case 15: _t->on_actionConvertGeo_triggered(); break;
        case 16: _t->on_actionExit_triggered(); break;
        case 17: _t->on_recentFiles_triggered((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        case 18: _t->on_recentFEFiles_triggered((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        case 19: _t->on_recentGeomFiles_triggered((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        case 20: _t->on_actionUndo_triggered(); break;
        case 21: _t->on_actionRedo_triggered(); break;
        case 22: _t->on_actionInvertSelection_triggered(); break;
        case 23: _t->on_actionClearSelection_triggered(); break;
        case 24: _t->on_actionDeleteSelection_triggered(); break;
        case 25: _t->on_actionHideSelection_triggered(); break;
        case 26: _t->on_actionHideUnselected_triggered(); break;
        case 27: _t->on_actionUnhideAll_triggered(); break;
        case 28: _t->on_actionFind_triggered(); break;
        case 29: _t->on_actionSelectRange_triggered(); break;
        case 30: _t->on_actionToggleVisible_triggered(); break;
        case 31: _t->on_actionNameSelection_triggered(); break;
        case 32: _t->on_actionTransform_triggered(); break;
        case 33: _t->on_actionCollapseTransform_triggered(); break;
        case 34: _t->on_actionClone_triggered(); break;
        case 35: _t->on_actionCloneGrid_triggered(); break;
        case 36: _t->on_actionCloneRevolve_triggered(); break;
        case 37: _t->on_actionMerge_triggered(); break;
        case 38: _t->on_actionPurge_triggered(); break;
        case 39: _t->on_actionDetach_triggered(); break;
        case 40: _t->on_actionExtract_triggered(); break;
        case 41: _t->on_actionEditProject_triggered(); break;
        case 42: _t->on_actionFaceToElem_triggered(); break;
        case 43: _t->on_actionAddBC_triggered(); break;
        case 44: _t->on_actionAddNodalLoad_triggered(); break;
        case 45: _t->on_actionAddSurfLoad_triggered(); break;
        case 46: _t->on_actionAddBodyLoad_triggered(); break;
        case 47: _t->on_actionAddIC_triggered(); break;
        case 48: _t->on_actionAddContact_triggered(); break;
        case 49: _t->on_actionAddConstraint_triggered(); break;
        case 50: _t->on_actionAddRigidConstraint_triggered(); break;
        case 51: _t->on_actionAddRigidConnector_triggered(); break;
        case 52: _t->on_actionAddMaterial_triggered(); break;
        case 53: _t->on_actionAddStep_triggered(); break;
        case 54: _t->on_actionAddReaction_triggered(); break;
        case 55: _t->on_actionSoluteTable_triggered(); break;
        case 56: _t->on_actionSBMTable_triggered(); break;
        case 57: _t->on_actionCurveEditor_triggered(); break;
        case 58: _t->on_actionMeshInspector_triggered(); break;
        case 59: _t->on_actionElasticityConvertor_triggered(); break;
        case 60: _t->on_actionKinemat_triggered(); break;
        case 61: _t->on_actionPlotMix_triggered(); break;
        case 62: _t->on_actionFEBioRun_triggered(); break;
        case 63: _t->on_actionFEBioStop_triggered(); break;
        case 64: _t->on_actionFEBioOptimize_triggered(); break;
        case 65: _t->on_actionOptions_triggered(); break;
        case 66: _t->on_actionLayerInfo_triggered(); break;
        case 67: _t->on_actionPlaneCut_triggered(); break;
        case 68: _t->on_actionMirrorPlane_triggered(); break;
        case 69: _t->on_actionVectorPlot_triggered(); break;
        case 70: _t->on_actionTensorPlot_triggered(); break;
        case 71: _t->on_actionIsosurfacePlot_triggered(); break;
        case 72: _t->on_actionSlicePlot_triggered(); break;
        case 73: _t->on_actionDisplacementMap_triggered(); break;
        case 74: _t->on_actionStreamLinePlot_triggered(); break;
        case 75: _t->on_actionParticleFlowPlot_triggered(); break;
        case 76: _t->on_actionVolumeFlowPlot_triggered(); break;
        case 77: _t->on_actionImageSlicer_triggered(); break;
        case 78: _t->on_actionVolumeRender_triggered(); break;
        case 79: _t->on_actionMarchingCubes_triggered(); break;
        case 80: _t->on_actionGraph_triggered(); break;
        case 81: _t->on_actionSummary_triggered(); break;
        case 82: _t->on_actionStats_triggered(); break;
        case 83: _t->on_actionIntegrate_triggered(); break;
        case 84: _t->on_actionImportPoints_triggered(); break;
        case 85: _t->on_actionImportLines_triggered(); break;
        case 86: _t->on_actionRecordNew_triggered(); break;
        case 87: _t->on_actionRecordStart_triggered(); break;
        case 88: _t->on_actionRecordPause_triggered(); break;
        case 89: _t->on_actionRecordStop_triggered(); break;
        case 90: _t->on_actionUndoViewChange_triggered(); break;
        case 91: _t->on_actionRedoViewChange_triggered(); break;
        case 92: _t->on_actionZoomSelect_triggered(); break;
        case 93: _t->on_actionZoomExtents_triggered(); break;
        case 94: _t->on_actionViewCapture_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 95: _t->on_actionOrtho_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 96: _t->on_actionShowGrid_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 97: _t->on_actionShowMeshLines_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 98: _t->on_actionShowEdgeLines_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 99: _t->on_actionBackfaceCulling_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 100: _t->on_actionViewSmooth_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 101: _t->on_actionShowNormals_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 102: _t->on_actionShowFibers_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 103: _t->on_actionShowMatAxes_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 104: _t->on_actionShowDiscrete_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 105: _t->on_actionToggleLight_triggered(); break;
        case 106: _t->on_actionFront_triggered(); break;
        case 107: _t->on_actionBack_triggered(); break;
        case 108: _t->on_actionLeft_triggered(); break;
        case 109: _t->on_actionRight_triggered(); break;
        case 110: _t->on_actionTop_triggered(); break;
        case 111: _t->on_actionBottom_triggered(); break;
        case 112: _t->on_actionWireframe_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 113: _t->on_actionSnap3D_triggered(); break;
        case 114: _t->on_actionTrack_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 115: _t->on_actionViewVPSave_triggered(); break;
        case 116: _t->on_actionViewVPPrev_triggered(); break;
        case 117: _t->on_actionViewVPNext_triggered(); break;
        case 118: _t->on_actionSyncViews_triggered(); break;
        case 119: _t->on_actionOnlineHelp_triggered(); break;
        case 120: _t->on_actionFEBioURL_triggered(); break;
        case 121: _t->on_actionFEBioForum_triggered(); break;
        case 122: _t->on_actionFEBioResources_triggered(); break;
        case 123: _t->on_actionFEBioPubs_triggered(); break;
        case 124: _t->on_actionAbout_triggered(); break;
        case 125: _t->on_actionSelect_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 126: _t->on_actionTranslate_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 127: _t->on_actionRotate_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 128: _t->on_actionScale_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 129: _t->on_selectCoord_currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 130: _t->on_actionSelectObjects_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 131: _t->on_actionSelectParts_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 132: _t->on_actionSelectSurfaces_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 133: _t->on_actionSelectCurves_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 134: _t->on_actionSelectNodes_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 135: _t->on_actionSelectDiscrete_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 136: _t->on_selectRect_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 137: _t->on_selectCircle_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 138: _t->on_selectFree_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 139: _t->on_actionMeasureTool_triggered(); break;
        case 140: _t->on_postSelectRect_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 141: _t->on_postSelectCircle_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 142: _t->on_postSelectFree_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 143: _t->on_postActionMeasureTool_triggered(); break;
        case 144: _t->on_selectData_currentValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 145: _t->on_actionPlay_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 146: _t->on_actionRefresh_triggered(); break;
        case 147: _t->on_actionFirst_triggered(); break;
        case 148: _t->on_actionPrev_triggered(); break;
        case 149: _t->on_actionNext_triggered(); break;
        case 150: _t->on_actionLast_triggered(); break;
        case 151: _t->on_actionTimeSettings_triggered(); break;
        case 152: _t->on_actionColorMap_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 153: _t->on_selectTime_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 154: _t->on_fontStyle_currentFontChanged((*reinterpret_cast< const QFont(*)>(_a[1]))); break;
        case 155: _t->on_fontSize_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 156: _t->on_fontBold_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 157: _t->on_fontItalic_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 158: _t->on_actionProperties_triggered(); break;
        case 159: _t->on_tab_currentChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 160: _t->on_tab_tabCloseRequested((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 161: _t->OnPostObjectStateChanged(); break;
        case 162: _t->OnPostObjectPropsChanged((*reinterpret_cast< FSObject*(*)>(_a[1]))); break;
        case 163: _t->on_modelViewer_currentObjectChanged((*reinterpret_cast< FSObject*(*)>(_a[1]))); break;
        case 164: _t->OnSelectMeshLayer((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        case 165: _t->OnSelectObjectTransparencyMode((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        case 166: _t->CloseView((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 167: _t->CloseView((*reinterpret_cast< CDocument*(*)>(_a[1]))); break;
        case 168: _t->SetCurrentState((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 169: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        case 170: _t->keyPressEvent((*reinterpret_cast< QKeyEvent*(*)>(_a[1]))); break;
        case 171: _t->on_finishedReadingFile((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 172: _t->finishedReadingFile((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< QueuedFile(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3]))); break;
        case 173: _t->checkFileProgress(); break;
        case 174: _t->StopAnimation(); break;
        case 175: _t->onTimer(); break;
        case 176: _t->on_glview_pointPicked((*reinterpret_cast< const vec3d(*)>(_a[1]))); break;
        case 177: _t->on_glview_selectionChanged(); break;
        case 178: _t->onRunFinished((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QProcess::ExitStatus(*)>(_a[2]))); break;
        case 179: _t->onReadyRead(); break;
        case 180: _t->onErrorOccurred((*reinterpret_cast< QProcess::ProcessError(*)>(_a[1]))); break;
        case 181: _t->onExportMaterials((*reinterpret_cast< const vector<GMaterial*>(*)>(_a[1]))); break;
        case 182: _t->onExportAllMaterials(); break;
        case 183: _t->onImportMaterials(); break;
        case 184: _t->DeleteAllMaterials(); break;
        case 185: _t->DeleteAllBC(); break;
        case 186: _t->DeleteAllLoads(); break;
        case 187: _t->DeleteAllIC(); break;
        case 188: _t->DeleteAllContact(); break;
        case 189: _t->DeleteAllConstraints(); break;
        case 190: _t->DeleteAllRigidConstraints(); break;
        case 191: _t->DeleteAllRigidConnectors(); break;
        case 192: _t->DeleteAllSteps(); break;
        case 193: { CGLView* _r = _t->GetGLView();
            if (_a[0]) *reinterpret_cast< CGLView**>(_a[0]) = std::move(_r); }  break;
        case 194: _t->changeViewMode((*reinterpret_cast< View_Mode(*)>(_a[1]))); break;
        case 195: { Post::CGLModel* _r = _t->GetCurrentModel();
            if (_a[0]) *reinterpret_cast< Post::CGLModel**>(_a[0]) = std::move(_r); }  break;
        case 196: _t->UpdateFontToolbar(); break;
        case 197: _t->RunFEBioJob((*reinterpret_cast< CFEBioJob*(*)>(_a[1]))); break;
        case 198: _t->NextSSHFunction((*reinterpret_cast< CSSHHandler*(*)>(_a[1]))); break;
        case 199: _t->ShowProgress((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 200: _t->ShowProgress((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 201: _t->ShowIndeterminateProgress((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 202: _t->ShowIndeterminateProgress((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 203: _t->UpdateProgress((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 204: { bool _r = _t->DoModelCheck((*reinterpret_cast< CModelDocument*(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CMainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
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
        if (_id < 205)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 205;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 205)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 205;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
