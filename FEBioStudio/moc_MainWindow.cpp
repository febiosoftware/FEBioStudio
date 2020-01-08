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
    QByteArrayData data[208];
    char stringdata0[4851];
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
QT_MOC_LITERAL(46, 1254, 32), // "on_actionAddConstraint_triggered"
QT_MOC_LITERAL(47, 1287, 37), // "on_actionAddRigidConstraint_t..."
QT_MOC_LITERAL(48, 1325, 36), // "on_actionAddRigidConnector_tr..."
QT_MOC_LITERAL(49, 1362, 30), // "on_actionAddMaterial_triggered"
QT_MOC_LITERAL(50, 1393, 26), // "on_actionAddStep_triggered"
QT_MOC_LITERAL(51, 1420, 30), // "on_actionAddReaction_triggered"
QT_MOC_LITERAL(52, 1451, 30), // "on_actionSoluteTable_triggered"
QT_MOC_LITERAL(53, 1482, 27), // "on_actionSBMTable_triggered"
QT_MOC_LITERAL(54, 1510, 30), // "on_actionCurveEditor_triggered"
QT_MOC_LITERAL(55, 1541, 32), // "on_actionMeshInspector_triggered"
QT_MOC_LITERAL(56, 1574, 38), // "on_actionElasticityConvertor_..."
QT_MOC_LITERAL(57, 1613, 27), // "on_actionFEBioRun_triggered"
QT_MOC_LITERAL(58, 1641, 28), // "on_actionFEBioStop_triggered"
QT_MOC_LITERAL(59, 1670, 32), // "on_actionFEBioOptimize_triggered"
QT_MOC_LITERAL(60, 1703, 26), // "on_actionOptions_triggered"
QT_MOC_LITERAL(61, 1730, 27), // "on_actionPlaneCut_triggered"
QT_MOC_LITERAL(62, 1758, 30), // "on_actionMirrorPlane_triggered"
QT_MOC_LITERAL(63, 1789, 29), // "on_actionVectorPlot_triggered"
QT_MOC_LITERAL(64, 1819, 29), // "on_actionTensorPlot_triggered"
QT_MOC_LITERAL(65, 1849, 33), // "on_actionIsosurfacePlot_trigg..."
QT_MOC_LITERAL(66, 1883, 28), // "on_actionSlicePlot_triggered"
QT_MOC_LITERAL(67, 1912, 34), // "on_actionDisplacementMap_trig..."
QT_MOC_LITERAL(68, 1947, 33), // "on_actionStreamLinePlot_trigg..."
QT_MOC_LITERAL(69, 1981, 35), // "on_actionParticleFlowPlot_tri..."
QT_MOC_LITERAL(70, 2017, 33), // "on_actionVolumeFlowPlot_trigg..."
QT_MOC_LITERAL(71, 2051, 30), // "on_actionImageSlicer_triggered"
QT_MOC_LITERAL(72, 2082, 31), // "on_actionVolumeRender_triggered"
QT_MOC_LITERAL(73, 2114, 32), // "on_actionMarchingCubes_triggered"
QT_MOC_LITERAL(74, 2147, 24), // "on_actionGraph_triggered"
QT_MOC_LITERAL(75, 2172, 26), // "on_actionSummary_triggered"
QT_MOC_LITERAL(76, 2199, 24), // "on_actionStats_triggered"
QT_MOC_LITERAL(77, 2224, 28), // "on_actionIntegrate_triggered"
QT_MOC_LITERAL(78, 2253, 28), // "on_actionRecordNew_triggered"
QT_MOC_LITERAL(79, 2282, 30), // "on_actionRecordStart_triggered"
QT_MOC_LITERAL(80, 2313, 30), // "on_actionRecordPause_triggered"
QT_MOC_LITERAL(81, 2344, 29), // "on_actionRecordStop_triggered"
QT_MOC_LITERAL(82, 2374, 33), // "on_actionUndoViewChange_trigg..."
QT_MOC_LITERAL(83, 2408, 33), // "on_actionRedoViewChange_trigg..."
QT_MOC_LITERAL(84, 2442, 29), // "on_actionZoomSelect_triggered"
QT_MOC_LITERAL(85, 2472, 30), // "on_actionZoomExtents_triggered"
QT_MOC_LITERAL(86, 2503, 28), // "on_actionViewCapture_toggled"
QT_MOC_LITERAL(87, 2532, 8), // "bchecked"
QT_MOC_LITERAL(88, 2541, 22), // "on_actionOrtho_toggled"
QT_MOC_LITERAL(89, 2564, 1), // "b"
QT_MOC_LITERAL(90, 2566, 25), // "on_actionShowGrid_toggled"
QT_MOC_LITERAL(91, 2592, 30), // "on_actionShowMeshLines_toggled"
QT_MOC_LITERAL(92, 2623, 30), // "on_actionShowEdgeLines_toggled"
QT_MOC_LITERAL(93, 2654, 32), // "on_actionBackfaceCulling_toggled"
QT_MOC_LITERAL(94, 2687, 27), // "on_actionViewSmooth_toggled"
QT_MOC_LITERAL(95, 2715, 28), // "on_actionShowNormals_toggled"
QT_MOC_LITERAL(96, 2744, 27), // "on_actionShowFibers_toggled"
QT_MOC_LITERAL(97, 2772, 29), // "on_actionShowDiscrete_toggled"
QT_MOC_LITERAL(98, 2802, 24), // "on_actionFront_triggered"
QT_MOC_LITERAL(99, 2827, 23), // "on_actionBack_triggered"
QT_MOC_LITERAL(100, 2851, 23), // "on_actionLeft_triggered"
QT_MOC_LITERAL(101, 2875, 24), // "on_actionRight_triggered"
QT_MOC_LITERAL(102, 2900, 22), // "on_actionTop_triggered"
QT_MOC_LITERAL(103, 2923, 25), // "on_actionBottom_triggered"
QT_MOC_LITERAL(104, 2949, 26), // "on_actionWireframe_toggled"
QT_MOC_LITERAL(105, 2976, 25), // "on_actionSnap3D_triggered"
QT_MOC_LITERAL(106, 3002, 22), // "on_actionTrack_toggled"
QT_MOC_LITERAL(107, 3025, 29), // "on_actionOnlineHelp_triggered"
QT_MOC_LITERAL(108, 3055, 27), // "on_actionFEBioURL_triggered"
QT_MOC_LITERAL(109, 3083, 29), // "on_actionFEBioForum_triggered"
QT_MOC_LITERAL(110, 3113, 33), // "on_actionFEBioResources_trigg..."
QT_MOC_LITERAL(111, 3147, 28), // "on_actionFEBioPubs_triggered"
QT_MOC_LITERAL(112, 3176, 24), // "on_actionAbout_triggered"
QT_MOC_LITERAL(113, 3201, 23), // "on_actionSelect_toggled"
QT_MOC_LITERAL(114, 3225, 26), // "on_actionTranslate_toggled"
QT_MOC_LITERAL(115, 3252, 23), // "on_actionRotate_toggled"
QT_MOC_LITERAL(116, 3276, 22), // "on_actionScale_toggled"
QT_MOC_LITERAL(117, 3299, 34), // "on_selectCoord_currentIndexCh..."
QT_MOC_LITERAL(118, 3334, 1), // "n"
QT_MOC_LITERAL(119, 3336, 30), // "on_actionSelectObjects_toggled"
QT_MOC_LITERAL(120, 3367, 28), // "on_actionSelectParts_toggled"
QT_MOC_LITERAL(121, 3396, 31), // "on_actionSelectSurfaces_toggled"
QT_MOC_LITERAL(122, 3428, 29), // "on_actionSelectCurves_toggled"
QT_MOC_LITERAL(123, 3458, 28), // "on_actionSelectNodes_toggled"
QT_MOC_LITERAL(124, 3487, 31), // "on_actionSelectDiscrete_toggled"
QT_MOC_LITERAL(125, 3519, 21), // "on_selectRect_toggled"
QT_MOC_LITERAL(126, 3541, 23), // "on_selectCircle_toggled"
QT_MOC_LITERAL(127, 3565, 21), // "on_selectFree_toggled"
QT_MOC_LITERAL(128, 3587, 33), // "on_selectData_currentValueCha..."
QT_MOC_LITERAL(129, 3621, 1), // "i"
QT_MOC_LITERAL(130, 3623, 21), // "on_actionPlay_toggled"
QT_MOC_LITERAL(131, 3645, 24), // "on_actionFirst_triggered"
QT_MOC_LITERAL(132, 3670, 23), // "on_actionPrev_triggered"
QT_MOC_LITERAL(133, 3694, 23), // "on_actionNext_triggered"
QT_MOC_LITERAL(134, 3718, 23), // "on_actionLast_triggered"
QT_MOC_LITERAL(135, 3742, 31), // "on_actionTimeSettings_triggered"
QT_MOC_LITERAL(136, 3774, 25), // "on_actionColorMap_toggled"
QT_MOC_LITERAL(137, 3800, 26), // "on_selectTime_valueChanged"
QT_MOC_LITERAL(138, 3827, 31), // "on_fontStyle_currentFontChanged"
QT_MOC_LITERAL(139, 3859, 4), // "font"
QT_MOC_LITERAL(140, 3864, 24), // "on_fontSize_valueChanged"
QT_MOC_LITERAL(141, 3889, 19), // "on_fontBold_toggled"
QT_MOC_LITERAL(142, 3909, 7), // "checked"
QT_MOC_LITERAL(143, 3917, 21), // "on_fontItalic_toggled"
QT_MOC_LITERAL(144, 3939, 29), // "on_actionProperties_triggered"
QT_MOC_LITERAL(145, 3969, 21), // "on_tab_currentChanged"
QT_MOC_LITERAL(146, 3991, 24), // "on_tab_tabCloseRequested"
QT_MOC_LITERAL(147, 4016, 35), // "on_modelViewer_currentObjectC..."
QT_MOC_LITERAL(148, 4052, 9), // "FSObject*"
QT_MOC_LITERAL(149, 4062, 2), // "po"
QT_MOC_LITERAL(150, 4065, 9), // "CloseView"
QT_MOC_LITERAL(151, 4075, 9), // "CPostDoc*"
QT_MOC_LITERAL(152, 4085, 7), // "postDoc"
QT_MOC_LITERAL(153, 4093, 15), // "SetCurrentState"
QT_MOC_LITERAL(154, 4109, 10), // "closeEvent"
QT_MOC_LITERAL(155, 4120, 12), // "QCloseEvent*"
QT_MOC_LITERAL(156, 4133, 2), // "ev"
QT_MOC_LITERAL(157, 4136, 13), // "keyPressEvent"
QT_MOC_LITERAL(158, 4150, 10), // "QKeyEvent*"
QT_MOC_LITERAL(159, 4161, 19), // "finishedReadingFile"
QT_MOC_LITERAL(160, 4181, 7), // "success"
QT_MOC_LITERAL(161, 4189, 11), // "errorString"
QT_MOC_LITERAL(162, 4201, 17), // "checkFileProgress"
QT_MOC_LITERAL(163, 4219, 13), // "StopAnimation"
QT_MOC_LITERAL(164, 4233, 7), // "onTimer"
QT_MOC_LITERAL(165, 4241, 21), // "on_glview_pointPicked"
QT_MOC_LITERAL(166, 4263, 5), // "vec3d"
QT_MOC_LITERAL(167, 4269, 1), // "r"
QT_MOC_LITERAL(168, 4271, 26), // "on_glview_selectionChanged"
QT_MOC_LITERAL(169, 4298, 13), // "onRunFinished"
QT_MOC_LITERAL(170, 4312, 8), // "exitCode"
QT_MOC_LITERAL(171, 4321, 20), // "QProcess::ExitStatus"
QT_MOC_LITERAL(172, 4342, 2), // "es"
QT_MOC_LITERAL(173, 4345, 11), // "onReadyRead"
QT_MOC_LITERAL(174, 4357, 15), // "onErrorOccurred"
QT_MOC_LITERAL(175, 4373, 22), // "QProcess::ProcessError"
QT_MOC_LITERAL(176, 4396, 3), // "err"
QT_MOC_LITERAL(177, 4400, 17), // "onExportMaterials"
QT_MOC_LITERAL(178, 4418, 18), // "vector<GMaterial*>"
QT_MOC_LITERAL(179, 4437, 7), // "matList"
QT_MOC_LITERAL(180, 4445, 20), // "onExportAllMaterials"
QT_MOC_LITERAL(181, 4466, 17), // "onImportMaterials"
QT_MOC_LITERAL(182, 4484, 18), // "DeleteAllMaterials"
QT_MOC_LITERAL(183, 4503, 11), // "DeleteAllBC"
QT_MOC_LITERAL(184, 4515, 14), // "DeleteAllLoads"
QT_MOC_LITERAL(185, 4530, 11), // "DeleteAllIC"
QT_MOC_LITERAL(186, 4542, 16), // "DeleteAllContact"
QT_MOC_LITERAL(187, 4559, 20), // "DeleteAllConstraints"
QT_MOC_LITERAL(188, 4580, 25), // "DeleteAllRigidConstraints"
QT_MOC_LITERAL(189, 4606, 24), // "DeleteAllRigidConnectors"
QT_MOC_LITERAL(190, 4631, 14), // "DeleteAllSteps"
QT_MOC_LITERAL(191, 4646, 9), // "GetGLView"
QT_MOC_LITERAL(192, 4656, 8), // "CGLView*"
QT_MOC_LITERAL(193, 4665, 14), // "changeViewMode"
QT_MOC_LITERAL(194, 4680, 9), // "View_Mode"
QT_MOC_LITERAL(195, 4690, 2), // "vm"
QT_MOC_LITERAL(196, 4693, 15), // "GetCurrentModel"
QT_MOC_LITERAL(197, 4709, 15), // "Post::CGLModel*"
QT_MOC_LITERAL(198, 4725, 17), // "UpdateFontToolbar"
QT_MOC_LITERAL(199, 4743, 11), // "RunFEBioJob"
QT_MOC_LITERAL(200, 4755, 10), // "CFEBioJob*"
QT_MOC_LITERAL(201, 4766, 3), // "job"
QT_MOC_LITERAL(202, 4770, 15), // "NextSSHFunction"
QT_MOC_LITERAL(203, 4786, 12), // "CSSHHandler*"
QT_MOC_LITERAL(204, 4799, 15), // "ShowSSHProgress"
QT_MOC_LITERAL(205, 4815, 4), // "show"
QT_MOC_LITERAL(206, 4820, 17), // "UpdateSSHProgress"
QT_MOC_LITERAL(207, 4838, 12) // "DoModelCheck"

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
     173,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,  879,    2, 0x0a /* Public */,
       3,    0,  880,    2, 0x0a /* Public */,
       4,    0,  881,    2, 0x0a /* Public */,
       5,    0,  882,    2, 0x0a /* Public */,
       6,    0,  883,    2, 0x0a /* Public */,
       7,    0,  884,    2, 0x0a /* Public */,
       8,    0,  885,    2, 0x0a /* Public */,
       9,    0,  886,    2, 0x0a /* Public */,
      10,    0,  887,    2, 0x0a /* Public */,
      11,    0,  888,    2, 0x0a /* Public */,
      12,    0,  889,    2, 0x0a /* Public */,
      13,    0,  890,    2, 0x0a /* Public */,
      14,    0,  891,    2, 0x0a /* Public */,
      15,    1,  892,    2, 0x0a /* Public */,
      18,    1,  895,    2, 0x0a /* Public */,
      19,    1,  898,    2, 0x0a /* Public */,
      20,    0,  901,    2, 0x0a /* Public */,
      21,    0,  902,    2, 0x0a /* Public */,
      22,    0,  903,    2, 0x0a /* Public */,
      23,    0,  904,    2, 0x0a /* Public */,
      24,    0,  905,    2, 0x0a /* Public */,
      25,    0,  906,    2, 0x0a /* Public */,
      26,    0,  907,    2, 0x0a /* Public */,
      27,    0,  908,    2, 0x0a /* Public */,
      28,    0,  909,    2, 0x0a /* Public */,
      29,    0,  910,    2, 0x0a /* Public */,
      30,    0,  911,    2, 0x0a /* Public */,
      31,    0,  912,    2, 0x0a /* Public */,
      32,    0,  913,    2, 0x0a /* Public */,
      33,    0,  914,    2, 0x0a /* Public */,
      34,    0,  915,    2, 0x0a /* Public */,
      35,    0,  916,    2, 0x0a /* Public */,
      36,    0,  917,    2, 0x0a /* Public */,
      37,    0,  918,    2, 0x0a /* Public */,
      38,    0,  919,    2, 0x0a /* Public */,
      39,    0,  920,    2, 0x0a /* Public */,
      40,    0,  921,    2, 0x0a /* Public */,
      41,    0,  922,    2, 0x0a /* Public */,
      42,    0,  923,    2, 0x0a /* Public */,
      43,    0,  924,    2, 0x0a /* Public */,
      44,    0,  925,    2, 0x0a /* Public */,
      45,    0,  926,    2, 0x0a /* Public */,
      46,    0,  927,    2, 0x0a /* Public */,
      47,    0,  928,    2, 0x0a /* Public */,
      48,    0,  929,    2, 0x0a /* Public */,
      49,    0,  930,    2, 0x0a /* Public */,
      50,    0,  931,    2, 0x0a /* Public */,
      51,    0,  932,    2, 0x0a /* Public */,
      52,    0,  933,    2, 0x0a /* Public */,
      53,    0,  934,    2, 0x0a /* Public */,
      54,    0,  935,    2, 0x0a /* Public */,
      55,    0,  936,    2, 0x0a /* Public */,
      56,    0,  937,    2, 0x0a /* Public */,
      57,    0,  938,    2, 0x0a /* Public */,
      58,    0,  939,    2, 0x0a /* Public */,
      59,    0,  940,    2, 0x0a /* Public */,
      60,    0,  941,    2, 0x0a /* Public */,
      61,    0,  942,    2, 0x0a /* Public */,
      62,    0,  943,    2, 0x0a /* Public */,
      63,    0,  944,    2, 0x0a /* Public */,
      64,    0,  945,    2, 0x0a /* Public */,
      65,    0,  946,    2, 0x0a /* Public */,
      66,    0,  947,    2, 0x0a /* Public */,
      67,    0,  948,    2, 0x0a /* Public */,
      68,    0,  949,    2, 0x0a /* Public */,
      69,    0,  950,    2, 0x0a /* Public */,
      70,    0,  951,    2, 0x0a /* Public */,
      71,    0,  952,    2, 0x0a /* Public */,
      72,    0,  953,    2, 0x0a /* Public */,
      73,    0,  954,    2, 0x0a /* Public */,
      74,    0,  955,    2, 0x0a /* Public */,
      75,    0,  956,    2, 0x0a /* Public */,
      76,    0,  957,    2, 0x0a /* Public */,
      77,    0,  958,    2, 0x0a /* Public */,
      78,    0,  959,    2, 0x0a /* Public */,
      79,    0,  960,    2, 0x0a /* Public */,
      80,    0,  961,    2, 0x0a /* Public */,
      81,    0,  962,    2, 0x0a /* Public */,
      82,    0,  963,    2, 0x0a /* Public */,
      83,    0,  964,    2, 0x0a /* Public */,
      84,    0,  965,    2, 0x0a /* Public */,
      85,    0,  966,    2, 0x0a /* Public */,
      86,    1,  967,    2, 0x0a /* Public */,
      88,    1,  970,    2, 0x0a /* Public */,
      90,    1,  973,    2, 0x0a /* Public */,
      91,    1,  976,    2, 0x0a /* Public */,
      92,    1,  979,    2, 0x0a /* Public */,
      93,    1,  982,    2, 0x0a /* Public */,
      94,    1,  985,    2, 0x0a /* Public */,
      95,    1,  988,    2, 0x0a /* Public */,
      96,    1,  991,    2, 0x0a /* Public */,
      97,    1,  994,    2, 0x0a /* Public */,
      98,    0,  997,    2, 0x0a /* Public */,
      99,    0,  998,    2, 0x0a /* Public */,
     100,    0,  999,    2, 0x0a /* Public */,
     101,    0, 1000,    2, 0x0a /* Public */,
     102,    0, 1001,    2, 0x0a /* Public */,
     103,    0, 1002,    2, 0x0a /* Public */,
     104,    1, 1003,    2, 0x0a /* Public */,
     105,    0, 1006,    2, 0x0a /* Public */,
     106,    1, 1007,    2, 0x0a /* Public */,
     107,    0, 1010,    2, 0x0a /* Public */,
     108,    0, 1011,    2, 0x0a /* Public */,
     109,    0, 1012,    2, 0x0a /* Public */,
     110,    0, 1013,    2, 0x0a /* Public */,
     111,    0, 1014,    2, 0x0a /* Public */,
     112,    0, 1015,    2, 0x0a /* Public */,
     113,    1, 1016,    2, 0x0a /* Public */,
     114,    1, 1019,    2, 0x0a /* Public */,
     115,    1, 1022,    2, 0x0a /* Public */,
     116,    1, 1025,    2, 0x0a /* Public */,
     117,    1, 1028,    2, 0x0a /* Public */,
     119,    1, 1031,    2, 0x0a /* Public */,
     120,    1, 1034,    2, 0x0a /* Public */,
     121,    1, 1037,    2, 0x0a /* Public */,
     122,    1, 1040,    2, 0x0a /* Public */,
     123,    1, 1043,    2, 0x0a /* Public */,
     124,    1, 1046,    2, 0x0a /* Public */,
     125,    1, 1049,    2, 0x0a /* Public */,
     126,    1, 1052,    2, 0x0a /* Public */,
     127,    1, 1055,    2, 0x0a /* Public */,
     128,    1, 1058,    2, 0x0a /* Public */,
     130,    1, 1061,    2, 0x0a /* Public */,
     131,    0, 1064,    2, 0x0a /* Public */,
     132,    0, 1065,    2, 0x0a /* Public */,
     133,    0, 1066,    2, 0x0a /* Public */,
     134,    0, 1067,    2, 0x0a /* Public */,
     135,    0, 1068,    2, 0x0a /* Public */,
     136,    1, 1069,    2, 0x0a /* Public */,
     137,    1, 1072,    2, 0x0a /* Public */,
     138,    1, 1075,    2, 0x0a /* Public */,
     140,    1, 1078,    2, 0x0a /* Public */,
     141,    1, 1081,    2, 0x0a /* Public */,
     143,    1, 1084,    2, 0x0a /* Public */,
     144,    0, 1087,    2, 0x0a /* Public */,
     145,    1, 1088,    2, 0x0a /* Public */,
     146,    1, 1091,    2, 0x0a /* Public */,
     147,    1, 1094,    2, 0x0a /* Public */,
     150,    1, 1097,    2, 0x0a /* Public */,
     150,    1, 1100,    2, 0x0a /* Public */,
     153,    1, 1103,    2, 0x0a /* Public */,
     154,    1, 1106,    2, 0x0a /* Public */,
     157,    1, 1109,    2, 0x0a /* Public */,
     159,    2, 1112,    2, 0x0a /* Public */,
     162,    0, 1117,    2, 0x0a /* Public */,
     163,    0, 1118,    2, 0x0a /* Public */,
     164,    0, 1119,    2, 0x0a /* Public */,
     165,    1, 1120,    2, 0x0a /* Public */,
     168,    0, 1123,    2, 0x0a /* Public */,
     169,    2, 1124,    2, 0x0a /* Public */,
     173,    0, 1129,    2, 0x0a /* Public */,
     174,    1, 1130,    2, 0x0a /* Public */,
     177,    1, 1133,    2, 0x0a /* Public */,
     180,    0, 1136,    2, 0x0a /* Public */,
     181,    0, 1137,    2, 0x0a /* Public */,
     182,    0, 1138,    2, 0x0a /* Public */,
     183,    0, 1139,    2, 0x0a /* Public */,
     184,    0, 1140,    2, 0x0a /* Public */,
     185,    0, 1141,    2, 0x0a /* Public */,
     186,    0, 1142,    2, 0x0a /* Public */,
     187,    0, 1143,    2, 0x0a /* Public */,
     188,    0, 1144,    2, 0x0a /* Public */,
     189,    0, 1145,    2, 0x0a /* Public */,
     190,    0, 1146,    2, 0x0a /* Public */,
     191,    0, 1147,    2, 0x0a /* Public */,
     193,    1, 1148,    2, 0x0a /* Public */,
     196,    0, 1151,    2, 0x0a /* Public */,
     198,    0, 1152,    2, 0x0a /* Public */,
     199,    1, 1153,    2, 0x0a /* Public */,
     202,    1, 1156,    2, 0x0a /* Public */,
     204,    1, 1159,    2, 0x0a /* Public */,
     206,    1, 1162,    2, 0x0a /* Public */,
     207,    0, 1165,    2, 0x0a /* Public */,

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
    QMetaType::Void, QMetaType::Bool,   87,
    QMetaType::Void, QMetaType::Bool,   89,
    QMetaType::Void, QMetaType::Bool,   89,
    QMetaType::Void, QMetaType::Bool,   89,
    QMetaType::Void, QMetaType::Bool,   89,
    QMetaType::Void, QMetaType::Bool,   89,
    QMetaType::Void, QMetaType::Bool,   87,
    QMetaType::Void, QMetaType::Bool,   89,
    QMetaType::Void, QMetaType::Bool,   89,
    QMetaType::Void, QMetaType::Bool,   89,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   89,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   89,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   89,
    QMetaType::Void, QMetaType::Bool,   89,
    QMetaType::Void, QMetaType::Bool,   89,
    QMetaType::Void, QMetaType::Bool,   89,
    QMetaType::Void, QMetaType::Int,  118,
    QMetaType::Void, QMetaType::Bool,   89,
    QMetaType::Void, QMetaType::Bool,   89,
    QMetaType::Void, QMetaType::Bool,   89,
    QMetaType::Void, QMetaType::Bool,   89,
    QMetaType::Void, QMetaType::Bool,   89,
    QMetaType::Void, QMetaType::Bool,   89,
    QMetaType::Void, QMetaType::Bool,   89,
    QMetaType::Void, QMetaType::Bool,   89,
    QMetaType::Void, QMetaType::Bool,   89,
    QMetaType::Void, QMetaType::Int,  129,
    QMetaType::Void, QMetaType::Bool,   87,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   87,
    QMetaType::Void, QMetaType::Int,  118,
    QMetaType::Void, QMetaType::QFont,  139,
    QMetaType::Void, QMetaType::Int,  129,
    QMetaType::Void, QMetaType::Bool,  142,
    QMetaType::Void, QMetaType::Bool,   87,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,  118,
    QMetaType::Void, QMetaType::Int,  118,
    QMetaType::Void, 0x80000000 | 148,  149,
    QMetaType::Void, QMetaType::Int,  118,
    QMetaType::Void, 0x80000000 | 151,  152,
    QMetaType::Void, QMetaType::Int,  118,
    QMetaType::Void, 0x80000000 | 155,  156,
    QMetaType::Void, 0x80000000 | 158,  156,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,  160,  161,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 166,  167,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 171,  170,  172,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 175,  176,
    QMetaType::Void, 0x80000000 | 178,  179,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    0x80000000 | 192,
    QMetaType::Void, 0x80000000 | 194,  195,
    0x80000000 | 197,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 200,  201,
    QMetaType::Void, 0x80000000 | 203,    2,
    QMetaType::Void, QMetaType::Bool,  205,
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
        case 36: _t->on_actionAddBC_triggered(); break;
        case 37: _t->on_actionAddNodalLoad_triggered(); break;
        case 38: _t->on_actionAddSurfLoad_triggered(); break;
        case 39: _t->on_actionAddBodyLoad_triggered(); break;
        case 40: _t->on_actionAddIC_triggered(); break;
        case 41: _t->on_actionAddContact_triggered(); break;
        case 42: _t->on_actionAddConstraint_triggered(); break;
        case 43: _t->on_actionAddRigidConstraint_triggered(); break;
        case 44: _t->on_actionAddRigidConnector_triggered(); break;
        case 45: _t->on_actionAddMaterial_triggered(); break;
        case 46: _t->on_actionAddStep_triggered(); break;
        case 47: _t->on_actionAddReaction_triggered(); break;
        case 48: _t->on_actionSoluteTable_triggered(); break;
        case 49: _t->on_actionSBMTable_triggered(); break;
        case 50: _t->on_actionCurveEditor_triggered(); break;
        case 51: _t->on_actionMeshInspector_triggered(); break;
        case 52: _t->on_actionElasticityConvertor_triggered(); break;
        case 53: _t->on_actionFEBioRun_triggered(); break;
        case 54: _t->on_actionFEBioStop_triggered(); break;
        case 55: _t->on_actionFEBioOptimize_triggered(); break;
        case 56: _t->on_actionOptions_triggered(); break;
        case 57: _t->on_actionPlaneCut_triggered(); break;
        case 58: _t->on_actionMirrorPlane_triggered(); break;
        case 59: _t->on_actionVectorPlot_triggered(); break;
        case 60: _t->on_actionTensorPlot_triggered(); break;
        case 61: _t->on_actionIsosurfacePlot_triggered(); break;
        case 62: _t->on_actionSlicePlot_triggered(); break;
        case 63: _t->on_actionDisplacementMap_triggered(); break;
        case 64: _t->on_actionStreamLinePlot_triggered(); break;
        case 65: _t->on_actionParticleFlowPlot_triggered(); break;
        case 66: _t->on_actionVolumeFlowPlot_triggered(); break;
        case 67: _t->on_actionImageSlicer_triggered(); break;
        case 68: _t->on_actionVolumeRender_triggered(); break;
        case 69: _t->on_actionMarchingCubes_triggered(); break;
        case 70: _t->on_actionGraph_triggered(); break;
        case 71: _t->on_actionSummary_triggered(); break;
        case 72: _t->on_actionStats_triggered(); break;
        case 73: _t->on_actionIntegrate_triggered(); break;
        case 74: _t->on_actionRecordNew_triggered(); break;
        case 75: _t->on_actionRecordStart_triggered(); break;
        case 76: _t->on_actionRecordPause_triggered(); break;
        case 77: _t->on_actionRecordStop_triggered(); break;
        case 78: _t->on_actionUndoViewChange_triggered(); break;
        case 79: _t->on_actionRedoViewChange_triggered(); break;
        case 80: _t->on_actionZoomSelect_triggered(); break;
        case 81: _t->on_actionZoomExtents_triggered(); break;
        case 82: _t->on_actionViewCapture_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 83: _t->on_actionOrtho_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 84: _t->on_actionShowGrid_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 85: _t->on_actionShowMeshLines_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 86: _t->on_actionShowEdgeLines_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 87: _t->on_actionBackfaceCulling_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 88: _t->on_actionViewSmooth_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 89: _t->on_actionShowNormals_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 90: _t->on_actionShowFibers_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 91: _t->on_actionShowDiscrete_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 92: _t->on_actionFront_triggered(); break;
        case 93: _t->on_actionBack_triggered(); break;
        case 94: _t->on_actionLeft_triggered(); break;
        case 95: _t->on_actionRight_triggered(); break;
        case 96: _t->on_actionTop_triggered(); break;
        case 97: _t->on_actionBottom_triggered(); break;
        case 98: _t->on_actionWireframe_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 99: _t->on_actionSnap3D_triggered(); break;
        case 100: _t->on_actionTrack_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 101: _t->on_actionOnlineHelp_triggered(); break;
        case 102: _t->on_actionFEBioURL_triggered(); break;
        case 103: _t->on_actionFEBioForum_triggered(); break;
        case 104: _t->on_actionFEBioResources_triggered(); break;
        case 105: _t->on_actionFEBioPubs_triggered(); break;
        case 106: _t->on_actionAbout_triggered(); break;
        case 107: _t->on_actionSelect_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 108: _t->on_actionTranslate_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 109: _t->on_actionRotate_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 110: _t->on_actionScale_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 111: _t->on_selectCoord_currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 112: _t->on_actionSelectObjects_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 113: _t->on_actionSelectParts_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 114: _t->on_actionSelectSurfaces_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 115: _t->on_actionSelectCurves_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 116: _t->on_actionSelectNodes_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 117: _t->on_actionSelectDiscrete_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 118: _t->on_selectRect_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 119: _t->on_selectCircle_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 120: _t->on_selectFree_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 121: _t->on_selectData_currentValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 122: _t->on_actionPlay_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 123: _t->on_actionFirst_triggered(); break;
        case 124: _t->on_actionPrev_triggered(); break;
        case 125: _t->on_actionNext_triggered(); break;
        case 126: _t->on_actionLast_triggered(); break;
        case 127: _t->on_actionTimeSettings_triggered(); break;
        case 128: _t->on_actionColorMap_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 129: _t->on_selectTime_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 130: _t->on_fontStyle_currentFontChanged((*reinterpret_cast< const QFont(*)>(_a[1]))); break;
        case 131: _t->on_fontSize_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 132: _t->on_fontBold_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 133: _t->on_fontItalic_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 134: _t->on_actionProperties_triggered(); break;
        case 135: _t->on_tab_currentChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 136: _t->on_tab_tabCloseRequested((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 137: _t->on_modelViewer_currentObjectChanged((*reinterpret_cast< FSObject*(*)>(_a[1]))); break;
        case 138: _t->CloseView((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 139: _t->CloseView((*reinterpret_cast< CPostDoc*(*)>(_a[1]))); break;
        case 140: _t->SetCurrentState((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 141: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        case 142: _t->keyPressEvent((*reinterpret_cast< QKeyEvent*(*)>(_a[1]))); break;
        case 143: _t->finishedReadingFile((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 144: _t->checkFileProgress(); break;
        case 145: _t->StopAnimation(); break;
        case 146: _t->onTimer(); break;
        case 147: _t->on_glview_pointPicked((*reinterpret_cast< const vec3d(*)>(_a[1]))); break;
        case 148: _t->on_glview_selectionChanged(); break;
        case 149: _t->onRunFinished((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QProcess::ExitStatus(*)>(_a[2]))); break;
        case 150: _t->onReadyRead(); break;
        case 151: _t->onErrorOccurred((*reinterpret_cast< QProcess::ProcessError(*)>(_a[1]))); break;
        case 152: _t->onExportMaterials((*reinterpret_cast< const vector<GMaterial*>(*)>(_a[1]))); break;
        case 153: _t->onExportAllMaterials(); break;
        case 154: _t->onImportMaterials(); break;
        case 155: _t->DeleteAllMaterials(); break;
        case 156: _t->DeleteAllBC(); break;
        case 157: _t->DeleteAllLoads(); break;
        case 158: _t->DeleteAllIC(); break;
        case 159: _t->DeleteAllContact(); break;
        case 160: _t->DeleteAllConstraints(); break;
        case 161: _t->DeleteAllRigidConstraints(); break;
        case 162: _t->DeleteAllRigidConnectors(); break;
        case 163: _t->DeleteAllSteps(); break;
        case 164: { CGLView* _r = _t->GetGLView();
            if (_a[0]) *reinterpret_cast< CGLView**>(_a[0]) = std::move(_r); }  break;
        case 165: _t->changeViewMode((*reinterpret_cast< View_Mode(*)>(_a[1]))); break;
        case 166: { Post::CGLModel* _r = _t->GetCurrentModel();
            if (_a[0]) *reinterpret_cast< Post::CGLModel**>(_a[0]) = std::move(_r); }  break;
        case 167: _t->UpdateFontToolbar(); break;
        case 168: _t->RunFEBioJob((*reinterpret_cast< CFEBioJob*(*)>(_a[1]))); break;
        case 169: _t->NextSSHFunction((*reinterpret_cast< CSSHHandler*(*)>(_a[1]))); break;
        case 170: _t->ShowSSHProgress((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 171: _t->UpdateSSHProgress((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 172: { bool _r = _t->DoModelCheck();
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
        if (_id < 173)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 173;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 173)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 173;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
