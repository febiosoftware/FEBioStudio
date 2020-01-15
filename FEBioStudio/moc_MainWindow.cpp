/****************************************************************************
** Meta object code from reading C++ file 'MainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "MainWindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MainWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CMainWindow_t {
    QByteArrayData data[209];
    char stringdata0[4881];
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
QT_MOC_LITERAL(40, 1080, 29), // "on_actionFaceToElem_triggered"
QT_MOC_LITERAL(41, 1110, 24), // "on_actionAddBC_triggered"
QT_MOC_LITERAL(42, 1135, 31), // "on_actionAddNodalLoad_triggered"
QT_MOC_LITERAL(43, 1167, 30), // "on_actionAddSurfLoad_triggered"
QT_MOC_LITERAL(44, 1198, 30), // "on_actionAddBodyLoad_triggered"
QT_MOC_LITERAL(45, 1229, 24), // "on_actionAddIC_triggered"
QT_MOC_LITERAL(46, 1254, 29), // "on_actionAddContact_triggered"
QT_MOC_LITERAL(47, 1284, 32), // "on_actionAddConstraint_triggered"
QT_MOC_LITERAL(48, 1317, 37), // "on_actionAddRigidConstraint_t..."
QT_MOC_LITERAL(49, 1355, 36), // "on_actionAddRigidConnector_tr..."
QT_MOC_LITERAL(50, 1392, 30), // "on_actionAddMaterial_triggered"
QT_MOC_LITERAL(51, 1423, 26), // "on_actionAddStep_triggered"
QT_MOC_LITERAL(52, 1450, 30), // "on_actionAddReaction_triggered"
QT_MOC_LITERAL(53, 1481, 30), // "on_actionSoluteTable_triggered"
QT_MOC_LITERAL(54, 1512, 27), // "on_actionSBMTable_triggered"
QT_MOC_LITERAL(55, 1540, 30), // "on_actionCurveEditor_triggered"
QT_MOC_LITERAL(56, 1571, 32), // "on_actionMeshInspector_triggered"
QT_MOC_LITERAL(57, 1604, 38), // "on_actionElasticityConvertor_..."
QT_MOC_LITERAL(58, 1643, 27), // "on_actionFEBioRun_triggered"
QT_MOC_LITERAL(59, 1671, 28), // "on_actionFEBioStop_triggered"
QT_MOC_LITERAL(60, 1700, 32), // "on_actionFEBioOptimize_triggered"
QT_MOC_LITERAL(61, 1733, 26), // "on_actionOptions_triggered"
QT_MOC_LITERAL(62, 1760, 27), // "on_actionPlaneCut_triggered"
QT_MOC_LITERAL(63, 1788, 30), // "on_actionMirrorPlane_triggered"
QT_MOC_LITERAL(64, 1819, 29), // "on_actionVectorPlot_triggered"
QT_MOC_LITERAL(65, 1849, 29), // "on_actionTensorPlot_triggered"
QT_MOC_LITERAL(66, 1879, 33), // "on_actionIsosurfacePlot_trigg..."
QT_MOC_LITERAL(67, 1913, 28), // "on_actionSlicePlot_triggered"
QT_MOC_LITERAL(68, 1942, 34), // "on_actionDisplacementMap_trig..."
QT_MOC_LITERAL(69, 1977, 33), // "on_actionStreamLinePlot_trigg..."
QT_MOC_LITERAL(70, 2011, 35), // "on_actionParticleFlowPlot_tri..."
QT_MOC_LITERAL(71, 2047, 33), // "on_actionVolumeFlowPlot_trigg..."
QT_MOC_LITERAL(72, 2081, 30), // "on_actionImageSlicer_triggered"
QT_MOC_LITERAL(73, 2112, 31), // "on_actionVolumeRender_triggered"
QT_MOC_LITERAL(74, 2144, 32), // "on_actionMarchingCubes_triggered"
QT_MOC_LITERAL(75, 2177, 24), // "on_actionGraph_triggered"
QT_MOC_LITERAL(76, 2202, 26), // "on_actionSummary_triggered"
QT_MOC_LITERAL(77, 2229, 24), // "on_actionStats_triggered"
QT_MOC_LITERAL(78, 2254, 28), // "on_actionIntegrate_triggered"
QT_MOC_LITERAL(79, 2283, 28), // "on_actionRecordNew_triggered"
QT_MOC_LITERAL(80, 2312, 30), // "on_actionRecordStart_triggered"
QT_MOC_LITERAL(81, 2343, 30), // "on_actionRecordPause_triggered"
QT_MOC_LITERAL(82, 2374, 29), // "on_actionRecordStop_triggered"
QT_MOC_LITERAL(83, 2404, 33), // "on_actionUndoViewChange_trigg..."
QT_MOC_LITERAL(84, 2438, 33), // "on_actionRedoViewChange_trigg..."
QT_MOC_LITERAL(85, 2472, 29), // "on_actionZoomSelect_triggered"
QT_MOC_LITERAL(86, 2502, 30), // "on_actionZoomExtents_triggered"
QT_MOC_LITERAL(87, 2533, 28), // "on_actionViewCapture_toggled"
QT_MOC_LITERAL(88, 2562, 8), // "bchecked"
QT_MOC_LITERAL(89, 2571, 22), // "on_actionOrtho_toggled"
QT_MOC_LITERAL(90, 2594, 1), // "b"
QT_MOC_LITERAL(91, 2596, 25), // "on_actionShowGrid_toggled"
QT_MOC_LITERAL(92, 2622, 30), // "on_actionShowMeshLines_toggled"
QT_MOC_LITERAL(93, 2653, 30), // "on_actionShowEdgeLines_toggled"
QT_MOC_LITERAL(94, 2684, 32), // "on_actionBackfaceCulling_toggled"
QT_MOC_LITERAL(95, 2717, 27), // "on_actionViewSmooth_toggled"
QT_MOC_LITERAL(96, 2745, 28), // "on_actionShowNormals_toggled"
QT_MOC_LITERAL(97, 2774, 27), // "on_actionShowFibers_toggled"
QT_MOC_LITERAL(98, 2802, 29), // "on_actionShowDiscrete_toggled"
QT_MOC_LITERAL(99, 2832, 24), // "on_actionFront_triggered"
QT_MOC_LITERAL(100, 2857, 23), // "on_actionBack_triggered"
QT_MOC_LITERAL(101, 2881, 23), // "on_actionLeft_triggered"
QT_MOC_LITERAL(102, 2905, 24), // "on_actionRight_triggered"
QT_MOC_LITERAL(103, 2930, 22), // "on_actionTop_triggered"
QT_MOC_LITERAL(104, 2953, 25), // "on_actionBottom_triggered"
QT_MOC_LITERAL(105, 2979, 26), // "on_actionWireframe_toggled"
QT_MOC_LITERAL(106, 3006, 25), // "on_actionSnap3D_triggered"
QT_MOC_LITERAL(107, 3032, 22), // "on_actionTrack_toggled"
QT_MOC_LITERAL(108, 3055, 29), // "on_actionOnlineHelp_triggered"
QT_MOC_LITERAL(109, 3085, 27), // "on_actionFEBioURL_triggered"
QT_MOC_LITERAL(110, 3113, 29), // "on_actionFEBioForum_triggered"
QT_MOC_LITERAL(111, 3143, 33), // "on_actionFEBioResources_trigg..."
QT_MOC_LITERAL(112, 3177, 28), // "on_actionFEBioPubs_triggered"
QT_MOC_LITERAL(113, 3206, 24), // "on_actionAbout_triggered"
QT_MOC_LITERAL(114, 3231, 23), // "on_actionSelect_toggled"
QT_MOC_LITERAL(115, 3255, 26), // "on_actionTranslate_toggled"
QT_MOC_LITERAL(116, 3282, 23), // "on_actionRotate_toggled"
QT_MOC_LITERAL(117, 3306, 22), // "on_actionScale_toggled"
QT_MOC_LITERAL(118, 3329, 34), // "on_selectCoord_currentIndexCh..."
QT_MOC_LITERAL(119, 3364, 1), // "n"
QT_MOC_LITERAL(120, 3366, 30), // "on_actionSelectObjects_toggled"
QT_MOC_LITERAL(121, 3397, 28), // "on_actionSelectParts_toggled"
QT_MOC_LITERAL(122, 3426, 31), // "on_actionSelectSurfaces_toggled"
QT_MOC_LITERAL(123, 3458, 29), // "on_actionSelectCurves_toggled"
QT_MOC_LITERAL(124, 3488, 28), // "on_actionSelectNodes_toggled"
QT_MOC_LITERAL(125, 3517, 31), // "on_actionSelectDiscrete_toggled"
QT_MOC_LITERAL(126, 3549, 21), // "on_selectRect_toggled"
QT_MOC_LITERAL(127, 3571, 23), // "on_selectCircle_toggled"
QT_MOC_LITERAL(128, 3595, 21), // "on_selectFree_toggled"
QT_MOC_LITERAL(129, 3617, 33), // "on_selectData_currentValueCha..."
QT_MOC_LITERAL(130, 3651, 1), // "i"
QT_MOC_LITERAL(131, 3653, 21), // "on_actionPlay_toggled"
QT_MOC_LITERAL(132, 3675, 24), // "on_actionFirst_triggered"
QT_MOC_LITERAL(133, 3700, 23), // "on_actionPrev_triggered"
QT_MOC_LITERAL(134, 3724, 23), // "on_actionNext_triggered"
QT_MOC_LITERAL(135, 3748, 23), // "on_actionLast_triggered"
QT_MOC_LITERAL(136, 3772, 31), // "on_actionTimeSettings_triggered"
QT_MOC_LITERAL(137, 3804, 25), // "on_actionColorMap_toggled"
QT_MOC_LITERAL(138, 3830, 26), // "on_selectTime_valueChanged"
QT_MOC_LITERAL(139, 3857, 31), // "on_fontStyle_currentFontChanged"
QT_MOC_LITERAL(140, 3889, 4), // "font"
QT_MOC_LITERAL(141, 3894, 24), // "on_fontSize_valueChanged"
QT_MOC_LITERAL(142, 3919, 19), // "on_fontBold_toggled"
QT_MOC_LITERAL(143, 3939, 7), // "checked"
QT_MOC_LITERAL(144, 3947, 21), // "on_fontItalic_toggled"
QT_MOC_LITERAL(145, 3969, 29), // "on_actionProperties_triggered"
QT_MOC_LITERAL(146, 3999, 21), // "on_tab_currentChanged"
QT_MOC_LITERAL(147, 4021, 24), // "on_tab_tabCloseRequested"
QT_MOC_LITERAL(148, 4046, 35), // "on_modelViewer_currentObjectC..."
QT_MOC_LITERAL(149, 4082, 9), // "FSObject*"
QT_MOC_LITERAL(150, 4092, 2), // "po"
QT_MOC_LITERAL(151, 4095, 9), // "CloseView"
QT_MOC_LITERAL(152, 4105, 9), // "CPostDoc*"
QT_MOC_LITERAL(153, 4115, 7), // "postDoc"
QT_MOC_LITERAL(154, 4123, 15), // "SetCurrentState"
QT_MOC_LITERAL(155, 4139, 10), // "closeEvent"
QT_MOC_LITERAL(156, 4150, 12), // "QCloseEvent*"
QT_MOC_LITERAL(157, 4163, 2), // "ev"
QT_MOC_LITERAL(158, 4166, 13), // "keyPressEvent"
QT_MOC_LITERAL(159, 4180, 10), // "QKeyEvent*"
QT_MOC_LITERAL(160, 4191, 19), // "finishedReadingFile"
QT_MOC_LITERAL(161, 4211, 7), // "success"
QT_MOC_LITERAL(162, 4219, 11), // "errorString"
QT_MOC_LITERAL(163, 4231, 17), // "checkFileProgress"
QT_MOC_LITERAL(164, 4249, 13), // "StopAnimation"
QT_MOC_LITERAL(165, 4263, 7), // "onTimer"
QT_MOC_LITERAL(166, 4271, 21), // "on_glview_pointPicked"
QT_MOC_LITERAL(167, 4293, 5), // "vec3d"
QT_MOC_LITERAL(168, 4299, 1), // "r"
QT_MOC_LITERAL(169, 4301, 26), // "on_glview_selectionChanged"
QT_MOC_LITERAL(170, 4328, 13), // "onRunFinished"
QT_MOC_LITERAL(171, 4342, 8), // "exitCode"
QT_MOC_LITERAL(172, 4351, 20), // "QProcess::ExitStatus"
QT_MOC_LITERAL(173, 4372, 2), // "es"
QT_MOC_LITERAL(174, 4375, 11), // "onReadyRead"
QT_MOC_LITERAL(175, 4387, 15), // "onErrorOccurred"
QT_MOC_LITERAL(176, 4403, 22), // "QProcess::ProcessError"
QT_MOC_LITERAL(177, 4426, 3), // "err"
QT_MOC_LITERAL(178, 4430, 17), // "onExportMaterials"
QT_MOC_LITERAL(179, 4448, 18), // "vector<GMaterial*>"
QT_MOC_LITERAL(180, 4467, 7), // "matList"
QT_MOC_LITERAL(181, 4475, 20), // "onExportAllMaterials"
QT_MOC_LITERAL(182, 4496, 17), // "onImportMaterials"
QT_MOC_LITERAL(183, 4514, 18), // "DeleteAllMaterials"
QT_MOC_LITERAL(184, 4533, 11), // "DeleteAllBC"
QT_MOC_LITERAL(185, 4545, 14), // "DeleteAllLoads"
QT_MOC_LITERAL(186, 4560, 11), // "DeleteAllIC"
QT_MOC_LITERAL(187, 4572, 16), // "DeleteAllContact"
QT_MOC_LITERAL(188, 4589, 20), // "DeleteAllConstraints"
QT_MOC_LITERAL(189, 4610, 25), // "DeleteAllRigidConstraints"
QT_MOC_LITERAL(190, 4636, 24), // "DeleteAllRigidConnectors"
QT_MOC_LITERAL(191, 4661, 14), // "DeleteAllSteps"
QT_MOC_LITERAL(192, 4676, 9), // "GetGLView"
QT_MOC_LITERAL(193, 4686, 8), // "CGLView*"
QT_MOC_LITERAL(194, 4695, 14), // "changeViewMode"
QT_MOC_LITERAL(195, 4710, 9), // "View_Mode"
QT_MOC_LITERAL(196, 4720, 2), // "vm"
QT_MOC_LITERAL(197, 4723, 15), // "GetCurrentModel"
QT_MOC_LITERAL(198, 4739, 15), // "Post::CGLModel*"
QT_MOC_LITERAL(199, 4755, 17), // "UpdateFontToolbar"
QT_MOC_LITERAL(200, 4773, 11), // "RunFEBioJob"
QT_MOC_LITERAL(201, 4785, 10), // "CFEBioJob*"
QT_MOC_LITERAL(202, 4796, 3), // "job"
QT_MOC_LITERAL(203, 4800, 15), // "NextSSHFunction"
QT_MOC_LITERAL(204, 4816, 12), // "CSSHHandler*"
QT_MOC_LITERAL(205, 4829, 15), // "ShowSSHProgress"
QT_MOC_LITERAL(206, 4845, 4), // "show"
QT_MOC_LITERAL(207, 4850, 17), // "UpdateSSHProgress"
QT_MOC_LITERAL(208, 4868, 12) // "DoModelCheck"

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
    "on_actionFEBioRun_triggered\0"
    "on_actionFEBioStop_triggered\0"
    "on_actionFEBioOptimize_triggered\0"
    "on_actionOptions_triggered\0"
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
    "on_actionShowDiscrete_toggled\0"
    "on_actionFront_triggered\0"
    "on_actionBack_triggered\0on_actionLeft_triggered\0"
    "on_actionRight_triggered\0"
    "on_actionTop_triggered\0on_actionBottom_triggered\0"
    "on_actionWireframe_toggled\0"
    "on_actionSnap3D_triggered\0"
    "on_actionTrack_toggled\0"
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
    "on_selectData_currentValueChanged\0i\0"
    "on_actionPlay_toggled\0on_actionFirst_triggered\0"
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
    "on_modelViewer_currentObjectChanged\0"
    "FSObject*\0po\0CloseView\0CPostDoc*\0"
    "postDoc\0SetCurrentState\0closeEvent\0"
    "QCloseEvent*\0ev\0keyPressEvent\0QKeyEvent*\0"
    "finishedReadingFile\0success\0errorString\0"
    "checkFileProgress\0StopAnimation\0onTimer\0"
    "on_glview_pointPicked\0vec3d\0r\0"
    "on_glview_selectionChanged\0onRunFinished\0"
    "exitCode\0QProcess::ExitStatus\0es\0"
    "onReadyRead\0onErrorOccurred\0"
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
    "CSSHHandler*\0ShowSSHProgress\0show\0"
    "UpdateSSHProgress\0DoModelCheck"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CMainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
     174,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,  884,    2, 0x0a /* Public */,
       3,    0,  885,    2, 0x0a /* Public */,
       4,    0,  886,    2, 0x0a /* Public */,
       5,    0,  887,    2, 0x0a /* Public */,
       6,    0,  888,    2, 0x0a /* Public */,
       7,    0,  889,    2, 0x0a /* Public */,
       8,    0,  890,    2, 0x0a /* Public */,
       9,    0,  891,    2, 0x0a /* Public */,
      10,    0,  892,    2, 0x0a /* Public */,
      11,    0,  893,    2, 0x0a /* Public */,
      12,    0,  894,    2, 0x0a /* Public */,
      13,    0,  895,    2, 0x0a /* Public */,
      14,    0,  896,    2, 0x0a /* Public */,
      15,    1,  897,    2, 0x0a /* Public */,
      18,    1,  900,    2, 0x0a /* Public */,
      19,    1,  903,    2, 0x0a /* Public */,
      20,    0,  906,    2, 0x0a /* Public */,
      21,    0,  907,    2, 0x0a /* Public */,
      22,    0,  908,    2, 0x0a /* Public */,
      23,    0,  909,    2, 0x0a /* Public */,
      24,    0,  910,    2, 0x0a /* Public */,
      25,    0,  911,    2, 0x0a /* Public */,
      26,    0,  912,    2, 0x0a /* Public */,
      27,    0,  913,    2, 0x0a /* Public */,
      28,    0,  914,    2, 0x0a /* Public */,
      29,    0,  915,    2, 0x0a /* Public */,
      30,    0,  916,    2, 0x0a /* Public */,
      31,    0,  917,    2, 0x0a /* Public */,
      32,    0,  918,    2, 0x0a /* Public */,
      33,    0,  919,    2, 0x0a /* Public */,
      34,    0,  920,    2, 0x0a /* Public */,
      35,    0,  921,    2, 0x0a /* Public */,
      36,    0,  922,    2, 0x0a /* Public */,
      37,    0,  923,    2, 0x0a /* Public */,
      38,    0,  924,    2, 0x0a /* Public */,
      39,    0,  925,    2, 0x0a /* Public */,
      40,    0,  926,    2, 0x0a /* Public */,
      41,    0,  927,    2, 0x0a /* Public */,
      42,    0,  928,    2, 0x0a /* Public */,
      43,    0,  929,    2, 0x0a /* Public */,
      44,    0,  930,    2, 0x0a /* Public */,
      45,    0,  931,    2, 0x0a /* Public */,
      46,    0,  932,    2, 0x0a /* Public */,
      47,    0,  933,    2, 0x0a /* Public */,
      48,    0,  934,    2, 0x0a /* Public */,
      49,    0,  935,    2, 0x0a /* Public */,
      50,    0,  936,    2, 0x0a /* Public */,
      51,    0,  937,    2, 0x0a /* Public */,
      52,    0,  938,    2, 0x0a /* Public */,
      53,    0,  939,    2, 0x0a /* Public */,
      54,    0,  940,    2, 0x0a /* Public */,
      55,    0,  941,    2, 0x0a /* Public */,
      56,    0,  942,    2, 0x0a /* Public */,
      57,    0,  943,    2, 0x0a /* Public */,
      58,    0,  944,    2, 0x0a /* Public */,
      59,    0,  945,    2, 0x0a /* Public */,
      60,    0,  946,    2, 0x0a /* Public */,
      61,    0,  947,    2, 0x0a /* Public */,
      62,    0,  948,    2, 0x0a /* Public */,
      63,    0,  949,    2, 0x0a /* Public */,
      64,    0,  950,    2, 0x0a /* Public */,
      65,    0,  951,    2, 0x0a /* Public */,
      66,    0,  952,    2, 0x0a /* Public */,
      67,    0,  953,    2, 0x0a /* Public */,
      68,    0,  954,    2, 0x0a /* Public */,
      69,    0,  955,    2, 0x0a /* Public */,
      70,    0,  956,    2, 0x0a /* Public */,
      71,    0,  957,    2, 0x0a /* Public */,
      72,    0,  958,    2, 0x0a /* Public */,
      73,    0,  959,    2, 0x0a /* Public */,
      74,    0,  960,    2, 0x0a /* Public */,
      75,    0,  961,    2, 0x0a /* Public */,
      76,    0,  962,    2, 0x0a /* Public */,
      77,    0,  963,    2, 0x0a /* Public */,
      78,    0,  964,    2, 0x0a /* Public */,
      79,    0,  965,    2, 0x0a /* Public */,
      80,    0,  966,    2, 0x0a /* Public */,
      81,    0,  967,    2, 0x0a /* Public */,
      82,    0,  968,    2, 0x0a /* Public */,
      83,    0,  969,    2, 0x0a /* Public */,
      84,    0,  970,    2, 0x0a /* Public */,
      85,    0,  971,    2, 0x0a /* Public */,
      86,    0,  972,    2, 0x0a /* Public */,
      87,    1,  973,    2, 0x0a /* Public */,
      89,    1,  976,    2, 0x0a /* Public */,
      91,    1,  979,    2, 0x0a /* Public */,
      92,    1,  982,    2, 0x0a /* Public */,
      93,    1,  985,    2, 0x0a /* Public */,
      94,    1,  988,    2, 0x0a /* Public */,
      95,    1,  991,    2, 0x0a /* Public */,
      96,    1,  994,    2, 0x0a /* Public */,
      97,    1,  997,    2, 0x0a /* Public */,
      98,    1, 1000,    2, 0x0a /* Public */,
      99,    0, 1003,    2, 0x0a /* Public */,
     100,    0, 1004,    2, 0x0a /* Public */,
     101,    0, 1005,    2, 0x0a /* Public */,
     102,    0, 1006,    2, 0x0a /* Public */,
     103,    0, 1007,    2, 0x0a /* Public */,
     104,    0, 1008,    2, 0x0a /* Public */,
     105,    1, 1009,    2, 0x0a /* Public */,
     106,    0, 1012,    2, 0x0a /* Public */,
     107,    1, 1013,    2, 0x0a /* Public */,
     108,    0, 1016,    2, 0x0a /* Public */,
     109,    0, 1017,    2, 0x0a /* Public */,
     110,    0, 1018,    2, 0x0a /* Public */,
     111,    0, 1019,    2, 0x0a /* Public */,
     112,    0, 1020,    2, 0x0a /* Public */,
     113,    0, 1021,    2, 0x0a /* Public */,
     114,    1, 1022,    2, 0x0a /* Public */,
     115,    1, 1025,    2, 0x0a /* Public */,
     116,    1, 1028,    2, 0x0a /* Public */,
     117,    1, 1031,    2, 0x0a /* Public */,
     118,    1, 1034,    2, 0x0a /* Public */,
     120,    1, 1037,    2, 0x0a /* Public */,
     121,    1, 1040,    2, 0x0a /* Public */,
     122,    1, 1043,    2, 0x0a /* Public */,
     123,    1, 1046,    2, 0x0a /* Public */,
     124,    1, 1049,    2, 0x0a /* Public */,
     125,    1, 1052,    2, 0x0a /* Public */,
     126,    1, 1055,    2, 0x0a /* Public */,
     127,    1, 1058,    2, 0x0a /* Public */,
     128,    1, 1061,    2, 0x0a /* Public */,
     129,    1, 1064,    2, 0x0a /* Public */,
     131,    1, 1067,    2, 0x0a /* Public */,
     132,    0, 1070,    2, 0x0a /* Public */,
     133,    0, 1071,    2, 0x0a /* Public */,
     134,    0, 1072,    2, 0x0a /* Public */,
     135,    0, 1073,    2, 0x0a /* Public */,
     136,    0, 1074,    2, 0x0a /* Public */,
     137,    1, 1075,    2, 0x0a /* Public */,
     138,    1, 1078,    2, 0x0a /* Public */,
     139,    1, 1081,    2, 0x0a /* Public */,
     141,    1, 1084,    2, 0x0a /* Public */,
     142,    1, 1087,    2, 0x0a /* Public */,
     144,    1, 1090,    2, 0x0a /* Public */,
     145,    0, 1093,    2, 0x0a /* Public */,
     146,    1, 1094,    2, 0x0a /* Public */,
     147,    1, 1097,    2, 0x0a /* Public */,
     148,    1, 1100,    2, 0x0a /* Public */,
     151,    1, 1103,    2, 0x0a /* Public */,
     151,    1, 1106,    2, 0x0a /* Public */,
     154,    1, 1109,    2, 0x0a /* Public */,
     155,    1, 1112,    2, 0x0a /* Public */,
     158,    1, 1115,    2, 0x0a /* Public */,
     160,    2, 1118,    2, 0x0a /* Public */,
     163,    0, 1123,    2, 0x0a /* Public */,
     164,    0, 1124,    2, 0x0a /* Public */,
     165,    0, 1125,    2, 0x0a /* Public */,
     166,    1, 1126,    2, 0x0a /* Public */,
     169,    0, 1129,    2, 0x0a /* Public */,
     170,    2, 1130,    2, 0x0a /* Public */,
     174,    0, 1135,    2, 0x0a /* Public */,
     175,    1, 1136,    2, 0x0a /* Public */,
     178,    1, 1139,    2, 0x0a /* Public */,
     181,    0, 1142,    2, 0x0a /* Public */,
     182,    0, 1143,    2, 0x0a /* Public */,
     183,    0, 1144,    2, 0x0a /* Public */,
     184,    0, 1145,    2, 0x0a /* Public */,
     185,    0, 1146,    2, 0x0a /* Public */,
     186,    0, 1147,    2, 0x0a /* Public */,
     187,    0, 1148,    2, 0x0a /* Public */,
     188,    0, 1149,    2, 0x0a /* Public */,
     189,    0, 1150,    2, 0x0a /* Public */,
     190,    0, 1151,    2, 0x0a /* Public */,
     191,    0, 1152,    2, 0x0a /* Public */,
     192,    0, 1153,    2, 0x0a /* Public */,
     194,    1, 1154,    2, 0x0a /* Public */,
     197,    0, 1157,    2, 0x0a /* Public */,
     199,    0, 1158,    2, 0x0a /* Public */,
     200,    1, 1159,    2, 0x0a /* Public */,
     203,    1, 1162,    2, 0x0a /* Public */,
     205,    1, 1165,    2, 0x0a /* Public */,
     207,    1, 1168,    2, 0x0a /* Public */,
     208,    0, 1171,    2, 0x0a /* Public */,

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
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   88,
    QMetaType::Void, QMetaType::Bool,   90,
    QMetaType::Void, QMetaType::Bool,   90,
    QMetaType::Void, QMetaType::Bool,   90,
    QMetaType::Void, QMetaType::Bool,   90,
    QMetaType::Void, QMetaType::Bool,   90,
    QMetaType::Void, QMetaType::Bool,   88,
    QMetaType::Void, QMetaType::Bool,   90,
    QMetaType::Void, QMetaType::Bool,   90,
    QMetaType::Void, QMetaType::Bool,   90,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   90,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   90,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   90,
    QMetaType::Void, QMetaType::Bool,   90,
    QMetaType::Void, QMetaType::Bool,   90,
    QMetaType::Void, QMetaType::Bool,   90,
    QMetaType::Void, QMetaType::Int,  119,
    QMetaType::Void, QMetaType::Bool,   90,
    QMetaType::Void, QMetaType::Bool,   90,
    QMetaType::Void, QMetaType::Bool,   90,
    QMetaType::Void, QMetaType::Bool,   90,
    QMetaType::Void, QMetaType::Bool,   90,
    QMetaType::Void, QMetaType::Bool,   90,
    QMetaType::Void, QMetaType::Bool,   90,
    QMetaType::Void, QMetaType::Bool,   90,
    QMetaType::Void, QMetaType::Bool,   90,
    QMetaType::Void, QMetaType::Int,  130,
    QMetaType::Void, QMetaType::Bool,   88,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   88,
    QMetaType::Void, QMetaType::Int,  119,
    QMetaType::Void, QMetaType::QFont,  140,
    QMetaType::Void, QMetaType::Int,  130,
    QMetaType::Void, QMetaType::Bool,  143,
    QMetaType::Void, QMetaType::Bool,   88,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,  119,
    QMetaType::Void, QMetaType::Int,  119,
    QMetaType::Void, 0x80000000 | 149,  150,
    QMetaType::Void, QMetaType::Int,  119,
    QMetaType::Void, 0x80000000 | 152,  153,
    QMetaType::Void, QMetaType::Int,  119,
    QMetaType::Void, 0x80000000 | 156,  157,
    QMetaType::Void, 0x80000000 | 159,  157,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,  161,  162,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 167,  168,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 172,  171,  173,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 176,  177,
    QMetaType::Void, 0x80000000 | 179,  180,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    0x80000000 | 193,
    QMetaType::Void, 0x80000000 | 195,  196,
    0x80000000 | 198,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 201,  202,
    QMetaType::Void, 0x80000000 | 204,    2,
    QMetaType::Void, QMetaType::Bool,  206,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Bool,

       0        // eod
};

void CMainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CMainWindow *>(_o);
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
        case 36: _t->on_actionFaceToElem_triggered(); break;
        case 37: _t->on_actionAddBC_triggered(); break;
        case 38: _t->on_actionAddNodalLoad_triggered(); break;
        case 39: _t->on_actionAddSurfLoad_triggered(); break;
        case 40: _t->on_actionAddBodyLoad_triggered(); break;
        case 41: _t->on_actionAddIC_triggered(); break;
        case 42: _t->on_actionAddContact_triggered(); break;
        case 43: _t->on_actionAddConstraint_triggered(); break;
        case 44: _t->on_actionAddRigidConstraint_triggered(); break;
        case 45: _t->on_actionAddRigidConnector_triggered(); break;
        case 46: _t->on_actionAddMaterial_triggered(); break;
        case 47: _t->on_actionAddStep_triggered(); break;
        case 48: _t->on_actionAddReaction_triggered(); break;
        case 49: _t->on_actionSoluteTable_triggered(); break;
        case 50: _t->on_actionSBMTable_triggered(); break;
        case 51: _t->on_actionCurveEditor_triggered(); break;
        case 52: _t->on_actionMeshInspector_triggered(); break;
        case 53: _t->on_actionElasticityConvertor_triggered(); break;
        case 54: _t->on_actionFEBioRun_triggered(); break;
        case 55: _t->on_actionFEBioStop_triggered(); break;
        case 56: _t->on_actionFEBioOptimize_triggered(); break;
        case 57: _t->on_actionOptions_triggered(); break;
        case 58: _t->on_actionPlaneCut_triggered(); break;
        case 59: _t->on_actionMirrorPlane_triggered(); break;
        case 60: _t->on_actionVectorPlot_triggered(); break;
        case 61: _t->on_actionTensorPlot_triggered(); break;
        case 62: _t->on_actionIsosurfacePlot_triggered(); break;
        case 63: _t->on_actionSlicePlot_triggered(); break;
        case 64: _t->on_actionDisplacementMap_triggered(); break;
        case 65: _t->on_actionStreamLinePlot_triggered(); break;
        case 66: _t->on_actionParticleFlowPlot_triggered(); break;
        case 67: _t->on_actionVolumeFlowPlot_triggered(); break;
        case 68: _t->on_actionImageSlicer_triggered(); break;
        case 69: _t->on_actionVolumeRender_triggered(); break;
        case 70: _t->on_actionMarchingCubes_triggered(); break;
        case 71: _t->on_actionGraph_triggered(); break;
        case 72: _t->on_actionSummary_triggered(); break;
        case 73: _t->on_actionStats_triggered(); break;
        case 74: _t->on_actionIntegrate_triggered(); break;
        case 75: _t->on_actionRecordNew_triggered(); break;
        case 76: _t->on_actionRecordStart_triggered(); break;
        case 77: _t->on_actionRecordPause_triggered(); break;
        case 78: _t->on_actionRecordStop_triggered(); break;
        case 79: _t->on_actionUndoViewChange_triggered(); break;
        case 80: _t->on_actionRedoViewChange_triggered(); break;
        case 81: _t->on_actionZoomSelect_triggered(); break;
        case 82: _t->on_actionZoomExtents_triggered(); break;
        case 83: _t->on_actionViewCapture_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 84: _t->on_actionOrtho_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 85: _t->on_actionShowGrid_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 86: _t->on_actionShowMeshLines_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 87: _t->on_actionShowEdgeLines_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 88: _t->on_actionBackfaceCulling_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 89: _t->on_actionViewSmooth_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 90: _t->on_actionShowNormals_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 91: _t->on_actionShowFibers_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 92: _t->on_actionShowDiscrete_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 93: _t->on_actionFront_triggered(); break;
        case 94: _t->on_actionBack_triggered(); break;
        case 95: _t->on_actionLeft_triggered(); break;
        case 96: _t->on_actionRight_triggered(); break;
        case 97: _t->on_actionTop_triggered(); break;
        case 98: _t->on_actionBottom_triggered(); break;
        case 99: _t->on_actionWireframe_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 100: _t->on_actionSnap3D_triggered(); break;
        case 101: _t->on_actionTrack_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 102: _t->on_actionOnlineHelp_triggered(); break;
        case 103: _t->on_actionFEBioURL_triggered(); break;
        case 104: _t->on_actionFEBioForum_triggered(); break;
        case 105: _t->on_actionFEBioResources_triggered(); break;
        case 106: _t->on_actionFEBioPubs_triggered(); break;
        case 107: _t->on_actionAbout_triggered(); break;
        case 108: _t->on_actionSelect_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 109: _t->on_actionTranslate_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 110: _t->on_actionRotate_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 111: _t->on_actionScale_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 112: _t->on_selectCoord_currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 113: _t->on_actionSelectObjects_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 114: _t->on_actionSelectParts_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 115: _t->on_actionSelectSurfaces_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 116: _t->on_actionSelectCurves_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 117: _t->on_actionSelectNodes_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 118: _t->on_actionSelectDiscrete_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 119: _t->on_selectRect_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 120: _t->on_selectCircle_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 121: _t->on_selectFree_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 122: _t->on_selectData_currentValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 123: _t->on_actionPlay_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 124: _t->on_actionFirst_triggered(); break;
        case 125: _t->on_actionPrev_triggered(); break;
        case 126: _t->on_actionNext_triggered(); break;
        case 127: _t->on_actionLast_triggered(); break;
        case 128: _t->on_actionTimeSettings_triggered(); break;
        case 129: _t->on_actionColorMap_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 130: _t->on_selectTime_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 131: _t->on_fontStyle_currentFontChanged((*reinterpret_cast< const QFont(*)>(_a[1]))); break;
        case 132: _t->on_fontSize_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 133: _t->on_fontBold_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 134: _t->on_fontItalic_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 135: _t->on_actionProperties_triggered(); break;
        case 136: _t->on_tab_currentChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 137: _t->on_tab_tabCloseRequested((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 138: _t->on_modelViewer_currentObjectChanged((*reinterpret_cast< FSObject*(*)>(_a[1]))); break;
        case 139: _t->CloseView((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 140: _t->CloseView((*reinterpret_cast< CPostDoc*(*)>(_a[1]))); break;
        case 141: _t->SetCurrentState((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 142: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        case 143: _t->keyPressEvent((*reinterpret_cast< QKeyEvent*(*)>(_a[1]))); break;
        case 144: _t->finishedReadingFile((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 145: _t->checkFileProgress(); break;
        case 146: _t->StopAnimation(); break;
        case 147: _t->onTimer(); break;
        case 148: _t->on_glview_pointPicked((*reinterpret_cast< const vec3d(*)>(_a[1]))); break;
        case 149: _t->on_glview_selectionChanged(); break;
        case 150: _t->onRunFinished((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QProcess::ExitStatus(*)>(_a[2]))); break;
        case 151: _t->onReadyRead(); break;
        case 152: _t->onErrorOccurred((*reinterpret_cast< QProcess::ProcessError(*)>(_a[1]))); break;
        case 153: _t->onExportMaterials((*reinterpret_cast< const vector<GMaterial*>(*)>(_a[1]))); break;
        case 154: _t->onExportAllMaterials(); break;
        case 155: _t->onImportMaterials(); break;
        case 156: _t->DeleteAllMaterials(); break;
        case 157: _t->DeleteAllBC(); break;
        case 158: _t->DeleteAllLoads(); break;
        case 159: _t->DeleteAllIC(); break;
        case 160: _t->DeleteAllContact(); break;
        case 161: _t->DeleteAllConstraints(); break;
        case 162: _t->DeleteAllRigidConstraints(); break;
        case 163: _t->DeleteAllRigidConnectors(); break;
        case 164: _t->DeleteAllSteps(); break;
        case 165: { CGLView* _r = _t->GetGLView();
            if (_a[0]) *reinterpret_cast< CGLView**>(_a[0]) = std::move(_r); }  break;
        case 166: _t->changeViewMode((*reinterpret_cast< View_Mode(*)>(_a[1]))); break;
        case 167: { Post::CGLModel* _r = _t->GetCurrentModel();
            if (_a[0]) *reinterpret_cast< Post::CGLModel**>(_a[0]) = std::move(_r); }  break;
        case 168: _t->UpdateFontToolbar(); break;
        case 169: _t->RunFEBioJob((*reinterpret_cast< CFEBioJob*(*)>(_a[1]))); break;
        case 170: _t->NextSSHFunction((*reinterpret_cast< CSSHHandler*(*)>(_a[1]))); break;
        case 171: _t->ShowSSHProgress((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 172: _t->UpdateSSHProgress((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 173: { bool _r = _t->DoModelCheck();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
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
        if (_id < 174)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 174;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 174)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 174;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
