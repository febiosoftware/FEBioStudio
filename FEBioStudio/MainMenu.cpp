/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/
#include "MainMenu.h"
#include "MainWindow.h"
#include <QMenu>
#include <QMenuBar>
#include <QActionGroup>
#include "IconProvider.h"

CMainMenu::CMainMenu(CMainWindow* wnd) : m_wnd(wnd)
{
	QMenuBar* menuBar = wnd->menuBar();

	// --- File menu actions ---
	actionNewModel = createAction("New Model ...", "actionNewModel", "new");
	QAction* actionNewProject = createAction("New Project ...", "actionNewProject");
	actionOpen = createAction("Open Model File ...", "actionOpen", "open"); actionOpen->setShortcuts(QKeySequence::Open);
	actionSave = createAction("Save", "actionSave", "save"); actionSave->setShortcuts(QKeySequence::Save);
	QAction* actionSaveAs = createAction("Save As ...", "actionSaveAs"); actionSaveAs->setShortcuts(QKeySequence::SaveAs);
	QAction* actionSaveAll = createAction("Save All", "actionSaveAll");
	QAction* actionCloseAll = createAction("Close All", "actionCloseAll");
	actionSnapShot = createAction("Snapshot ...", "actionSnapShot", "snapshot"); actionSnapShot->setShortcut(Qt::AltModifier | Qt::Key_F2);
	actionRayTrace = createAction("Ray Tracer ...", "actionRayTrace", "teapot");
	QAction* actionSaveProject = createAction("Save Project As ...", "actionSaveProject");
	actionExportFE = createAction("Export FE model ...", "actionExportFEModel");
	actionImportGeom = createAction("Import Geometry ...", "actionImportGeometry");
	actionExportGeom = createAction("Export Geometry ...", "actionExportGeometry");
	QAction* actionOpenProject = createAction("Open Project ...", "actionOpenProject");
	QAction* actionImportProject = createAction("Import Project Archive ...", "actionImportProject");
	QAction* actionExportProject = createAction("Export Project Archive ...", "actionExportProject");
	QAction* actionImportRawImage = createAction("Raw ...", "actionImportRawImage");
	QAction* actionImportDICOMImage = createAction("DICOM/DICOM Sequence ...", "actionImportDICOMImage");
	QAction* actionImportTiffImage = createAction("Tiff ...", "actionImportTiffImage");
	QAction* actionImportNrrdImage = createAction("NRRD ...", "actionImportNrrdImage");
	QAction* actionImportImageSequence = createAction("Image Sequence ...", "actionImportImageSequence");
	QAction* actionImportImageOther = createAction("Other ...", "actionImportImageOther");
	QAction* actionConvertFeb = createAction("FEBio Files ...", "actionConvertFeb");
	QAction* actionConvertFeb2Fsm = createAction("FEB to FSM ...", "actionConvertFeb2Fsm");
	QAction* actionConvertFsm2Feb = createAction("FSM to FEB ...", "actionConvertFsm2Feb");
	QAction* actionConvertGeo = createAction("Geometry Files ...", "actionConvertGeo");
	QAction* actionExit = createAction("Exit", "actionExit");
	
	QAction* actionNewBatch  = createAction("New Batch ...", "actionNewBatch");
	QAction* actionOpenBatch = createAction("Open Batch File ...", "actionOpenBatch");

	// --- Edit menu actions ---
	actionUndo = createAction("Undo", "actionUndo", "undo"); actionUndo->setShortcuts(QKeySequence::Undo);
	actionRedo = createAction("Redo", "actionRedo", "redo"); actionRedo->setShortcuts(QKeySequence::Redo);
	QAction* actionChangeLog = createAction("Change log ...", "actionChangeLog");
	QAction* actionInvertSelection = createAction("Invert Selection", "actionInvertSelection"); actionInvertSelection->setShortcut(Qt::AltModifier | Qt::Key_I);
	QAction* actionClearSelection = createAction("Clear Selection", "actionClearSelection");
	QAction* actionDeleteSelection = createAction("Delete Selection", "actionDeleteSelection"); actionDeleteSelection->setShortcuts(QKeySequence::Delete);
	QAction* actionNameSelection = createAction("Name Selection ...", "actionNameSelection"); actionNameSelection->setShortcut(Qt::ControlModifier | Qt::Key_G);
	QAction* actionHideSelection = createAction("Hide Selection", "actionHideSelection"); actionHideSelection->setShortcut(Qt::Key_H);
	QAction* actionHideUnselected = createAction("Hide Unselected", "actionHideUnselected"); actionHideUnselected->setShortcut(Qt::ShiftModifier | Qt::Key_H);
	QAction* actionSyncSelection = createAction("Sync selection", "actionSyncSelection"); actionSyncSelection->setShortcut(Qt::AltModifier | Qt::Key_F);
	QAction* actionCopySelection = createAction("Copy selection", "actionCopySelection");
	QAction* actionPasteSelection = createAction("Paste selection", "actionPasteSelection");
	QAction* actionUnhideAll = createAction("Unhide All", "actionUnhideAll");
	QAction* actionFind = createAction("Find ...", "actionFind"); //actionFind->setShortcut(Qt::ControlModifier | Qt::Key_F);
	QAction* actionSelectRange = createAction("Select Range ...", "actionSelectRange");
	QAction* actionToggleVisible = createAction("Toggle Visibility", "actionToggleVisible", "toggle_visible");
	QAction* actionTransform = createAction("Transform ...", "actionTransform"); actionTransform->setShortcut(Qt::ControlModifier | Qt::Key_T);
	QAction* actionCollapseTransform = createAction("Collapse Transform", "actionCollapseTransform");
	QAction* actionCopyObject = createAction("Copy Object", "actionCopyObject");
	QAction* actionPasteObject = createAction("Paste Object", "actionPasteObject");

	actionMerge = createAction("Merge Objects ...", "actionMerge", "merge");
	actionDetach = createAction("Detach Elements", "actionDetach", "detach");
	actionExtract = createAction("Extract Faces", "actionExtract", "extract");
	actionClone = createAction("Clone Object ...", "actionClone", "clone"); // actionClone->setShortcut(Qt::ControlModifier | Qt::Key_D);
	actionCloneGrid = createAction("Clone Grid ...", "actionCloneGrid", "clonegrid");
	actionCloneRevolve = createAction("Clone Revolve ...", "actionCloneRevolve", "clonerevolve");
	actionPurge = createAction("Purge ...", "actionPurge");

	QAction* actionFace2Elems = createAction("Face to Element Selection", "actionFaceToElem");
	QAction* actionSurfaceToFaces = createAction("Surface to Face Selection", "actionSurfaceToFaces");
	QAction* actionSelectOverlap = createAction("Select surface overlap ...", "actionSelectOverlap");
	QAction* actionSelectIsolatedVertices = createAction("Select isolated vertices", "actionSelectIsolatedVertices");
	QAction* actionGrowSelection = createAction("Grow selection", "actionGrowSelection"); actionGrowSelection->setShortcut(Qt::ControlModifier | Qt::Key_Plus);
	QAction* actionShrinkSelection = createAction("Shrink selection", "actionShrinkSelection"); actionShrinkSelection->setShortcut(Qt::ControlModifier | Qt::Key_Minus);

	// --- Edit (txt) menu actions ---
	QAction* actionFindTxt = createAction("Find ...", "actionFindTxt"); actionFindTxt->setShortcut(Qt::Key_F | Qt::ControlModifier);
	QAction* actionFindAgain = createAction("Find Again", "actionFindAgain"); actionFindAgain->setShortcut(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_F);
	QAction* actionToggleComment = createAction("Toggle Line Comment", "actionToggleComment"); actionToggleComment->setShortcut(Qt::ControlModifier | Qt::Key_Slash);
	QAction* actionDuplicateLine = createAction("Copy Line Down", "actionDuplicateLine"); actionDuplicateLine->setShortcut(Qt::ControlModifier | Qt::Key_D);
	QAction* actionDeleteLine = createAction("Delete Line", "actionDeleteLine"); actionDeleteLine->setShortcut(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_L);

	// -- Edit (xml) menu actions ---
	actionAddAttribute = createAction("Add Attribute", "actionAddAttribute");
	actionAddAttribute->setIcon(CIconProvider::GetIcon("selectAdd", QColor(Qt::red), Shape::Circle));

	actionAddElement = createAction("Add Element", "actionAddElement");
	actionAddElement->setIcon(CIconProvider::GetIcon("selectAdd", QColor(Qt::blue), Shape::Circle));

	actionRemoveRow = createAction("Remove Row", "actionRemoveRow", "selectDel");

	// --- Physics menu actions ---
	QAction* actionAddNodalBC = createAction("Add Nodal BC...", "actionAddNodalBC"); actionAddNodalBC->setShortcut(Qt::ControlModifier | Qt::Key_B);
	QAction* actionAddSurfaceBC = createAction("Add Surface BC...", "actionAddSurfaceBC");
	QAction* actionAddGeneralBC = createAction("Add Linear Constraint ...", "actionAddGeneralBC");
	QAction* actionAddNodalLoad = createAction("Add Nodal Load ...", "actionAddNodalLoad");
	QAction* actionAddSurfLoad = createAction("Add Surface Load ...", "actionAddSurfLoad"); actionAddSurfLoad->setShortcut(Qt::ControlModifier | Qt::Key_L);
	QAction* actionAddBodyLoad = createAction("Add Body Load ...", "actionAddBodyLoad");
	QAction* actionAddIC = createAction("Add Initial Condition ...", "actionAddIC"); actionAddIC->setShortcut(Qt::ControlModifier | Qt::Key_I);
	QAction* actionAddContact = createAction("Add Contact ...", "actionAddContact");
	QAction* actionAddSurfaceNLC = createAction("Add Surface Constraint...", "actionAddSurfaceNLC");
	QAction* actionAddBodyNLC = createAction("Add Body Constraint...", "actionAddBodyNLC");
	QAction* actionAddGenericNLC = createAction("Add General Constraint...", "actionAddGenericNLC");
	QAction* actionAddRigidBC = createAction("Add Rigid Constraint ...", "actionAddRigidBC");
	QAction* actionAddRigidIC = createAction("Add Rigid Initial Condition ...", "actionAddRigidIC");
	QAction* actionAddRigidLoad = createAction("Add Rigid Load ...", "actionAddRigidLoad");
	QAction* actionAddRigidConnector = createAction("Add Rigid Connector ...", "actionAddRigidConnector");
	QAction* actionAddStep = createAction("Add Analysis Step ...", "actionAddStep");
	QAction* actionStepViewer = createAction("Step Viewer ...", "actionStepViewer");
	QAction* actionAddMaterial = createAction("Add Material ...", "actionAddMaterial", "material"); actionAddMaterial->setShortcut(Qt::ControlModifier | Qt::Key_M);
	actionSoluteTable = createAction("Solute Table ...", "actionSoluteTable");
	actionSBMTable = createAction("Solid-bound Molecule Table ...", "actionSBMTable");
	actionAddReaction = createAction("Chemical Reaction Editor ...", "actionAddReaction");
	actionAddMembraneReaction = createAction("Membrane Reaction Editor ...", "actionAddMembraneReaction");
	QAction* actionEditProject = createAction("Edit Physics Modules ...", "actionEditProject");
	QAction* assignSelection = createAction("Assign current selection", "actionAssignSelection"); assignSelection->setShortcut(Qt::ControlModifier | Qt::Key_A);

	// --- FEBio menu actions ---
	actionFEBioRun = createAction("Run FEBio ...", "actionFEBioRun", "febiorun"); actionFEBioRun->setShortcut(Qt::Key_F5);
	actionFEBioCheck = createAction("Model check ...", "actionFEBioCheck");
	actionFEBioStop = createAction("Stop FEBio", "actionFEBioStop", "stop");
	actionFEBioMonitor = createAction("Run FEBio Monitor ...", "actionFEBioMonitor", "febiomonitor");
	actionFEBioMonitorSettings = createAction("FEBio Monitor Settings ...", "actionFEBioMonitorSettings", "febiomonitor");
	actionFEBioContinue = createAction("Pause FEBio", "actionFEBioContinue", "play"); actionFEBioContinue->setShortcut(Qt::ControlModifier | Qt::Key_F5);
	actionFEBioPause = createAction("Pause FEBio", "actionFEBioPause", "pause"); actionFEBioPause->setShortcut(Qt::ShiftModifier | Qt::Key_F5);
	actionFEBioNext = createAction("Advance FEBio", "actionFEBioNext", "next"); actionFEBioNext->setShortcut(Qt::AltModifier | Qt::Key_F5);
	QAction* actionFEBioOptimize = createAction("Create optimization Study ...", "actionFEBioOptimize");
	QAction* actionFEBioTangent = createAction("Generate tangent diagnostic ...", "actionFEBioTangent");
	QAction* actionFEBioInfo = createAction("FEBio Info ...", "actionFEBioInfo");
    actionPluginRepo = createAction("Plugin Repository ...", "actionPluginRepo", "plugin"); actionPluginRepo->setToolTip("Open the FEBio Plugin Repository");
	QAction* actionCreatePlugin = createAction("Create FEBio plugin ...", "actionCreatePlugin");

	// --- Tools menu ---
	actionCurveEditor = createAction("Curve Editor ...", "actionCurveEditor", "curves"); actionCurveEditor->setShortcut(Qt::Key_F9);
	actionMeshInspector = createAction("Mesh Inspector ...", "actionMeshInspector", "inspect"); actionMeshInspector->setShortcut(Qt::Key_F10);
	actionMeshDiagnostic = createAction("Mesh Diagnostic ...", "actionMeshDiagnostic"); actionMeshDiagnostic->setShortcut(Qt::Key_F11);
	QAction* actionElasticityConvertor = createAction("Elasticity Converter ...", "actionElasticityConvertor");
	QAction* actionUnitConverter = createAction("Unit Converter ...", "actionUnitConverter");
	QAction* actionRotationConverter = createAction("Rotation Converter ...", "actionRotationConverter");
	actionMaterialTest = createAction("Material test ...", "actionMaterialTest");
	actionDistroVisual = createAction("Distribution visualizer ...", "actionDistroVisual");
	QAction* actionKinemat = createAction("Kinemat ...", "actionKinemat");
	QAction* actionPlotMix = createAction("Plotmix ...", "actionPlotMix");
	QAction* actionPython = createAction("Python editor ...", "actionEditPython");
	actionSettings = createAction("Settings ...", "actionSettings"); actionSettings->setShortcut(Qt::Key_F12);

	QAction* actionLayerInfo = createAction("Print Layer Info", "actionLayerInfo"); actionLayerInfo->setShortcut(Qt::AltModifier | Qt::Key_L);

	// --- Post menu ---
	actionPlaneCut = createAction("Planecut", "actionPlaneCut", "cut");
	actionMirrorPlane = createAction("Mirror Plane", "actionMirrorPlane", "mirror");
	actionVectorPlot = createAction("Vector Plot", "actionVectorPlot", "vectors");
	actionTensorPlot = createAction("Tensor Plot", "actionTensorPlot", "tensor");
	actionIsosurfacePlot = createAction("Isosurface Plot", "actionIsosurfacePlot", "isosurface");
	actionSlicePlot = createAction("Slice Plot", "actionSlicePlot", "sliceplot");
	actionDisplacementMap = createAction("Displacement Map", "actionDisplacementMap", "distort");
	actionStreamLinePlot = createAction("Streamlines Plot", "actionStreamLinePlot", "streamlines");
	actionParticleFlowPlot = createAction("Particleflow Plot", "actionParticleFlowPlot", "particle");
	actionVolumeFlowPlot = createAction("Volumeflow Plot", "actionVolumeFlowPlot", "flow");
	actionVectorGlyph = createAction("Vector Glyph", "actionVectorGlyph");
	actionHelicalAxis = createAction("Helical Axis", "actionHelicalAxis", "helix");
	actionStaticMesh = createAction("Static Mesh", "actionStaticMesh", "mesh");

	actionImageSlicer = createAction("Image Slicer", "actionImageSlicer", "imageslice");
	actionVolumeRender = createAction("Volume Render", "actionVolumeRender", "volrender");
	actionMarchingCubes = createAction("Image Isosurface", "actionMarchingCubes", "marching_cubes");
	actionImageWarp = createAction("Image Warp", "actionImageWarp");

	actionAddProbe = createAction("Point Probe", "actionAddProbe", "probe");
	actionAddCurveProbe = createAction("Curve Probe", "actionAddCurveProbe");
	actionAddRuler = createAction("Ruler", "actionAddRuler", "ruler");
	actionMusclePath = createAction("Muscle Path", "actionMusclePath", "musclepath");
	actionPlotGroup = createAction("Plot Group", "actionPlotGroup", "folder");
	actionGraph = createAction("New Graph ...", "actionGraph", "chart"); actionGraph->setShortcut(Qt::Key_F3);
	actionScatter = createAction("Scatter plot ...", "actionScatter");
	actionSummary = createAction("Summary ...", "actionSummary");
	actionStats = createAction("Statistics  ...", "actionStats");
	actionIntegrate = createAction("Integrate ...", "actionIntegrate", "integrate");
	actionIntegrateSurface = createAction("Integrate Surface ...", "actionIntegrateSurface");
	actionImportPoints = createAction("Import Points ...", "actionImportPoints");
	actionImportLines = createAction("Import Lines ...", "actionImportLines");

	actionPlaneCut->setWhatsThis("<font color=\"black\"><h3>Plane cut</h3>Add a plane cut plot to the model. A plane cut plot allows users to create a cross section of the mesh.</font>");
	actionMirrorPlane->setWhatsThis("<font color=\"black\"><h3>Mirror plane</h3>Renders a mirrorred version of the model.</font>");
	actionVectorPlot->setWhatsThis("<font color=\"black\"><h3>Vector plot</h3>Add a vector plot to the model. Vectors plots can show vector data in the model");
	actionTensorPlot->setWhatsThis("<font color=\"black\"><h3>Tensor plot</h3>Add a tensor plot to the model. Tensor plots can show 2nd order tensor data in the model");
	actionIsosurfacePlot->setWhatsThis("<font color=\"black\"><h3>Iso-surface plot</h3>Add an iso-surface plot to the model. An iso-surface plot shows surfaces that have the same value. You may need to make the model transparent in order to see the iso surfaces.");
	actionSlicePlot->setWhatsThis("<font color=\"black\"><h3>Slice plot</h3>Add a slice plot. This plot adds several cross sections to the model. You may need to make the model transparent to see the slices.");
	actionVolumeFlowPlot->setWhatsThis("<font color=\"black\"><h3>Volume flow plot</h3>Add a volume flow plot.");
	actionDisplacementMap->setWhatsThis("<font color=\"black\"><h3>Displacement map</h3>Adds a displacement map. A displacement map will deform the model as a function of time.");
	actionGraph->setWhatsThis("<font color=\"black\">Create a new Graph window");
	actionSummary->setWhatsThis("<font color=\"black\">Shows the Summary window.The Summary window shows the min, max, and average values of a user-selected data field");
	actionStats->setWhatsThis("<font color=\"black\">Shows the Statistics window. This window shows the distribution of the current nodal values at the current time step");
	actionIntegrate->setWhatsThis("<font color=\"black\">Shows a graph that plots the integral of the values of the current selection as a function of time. Note that for a surface select it calculates a surface integral and for an element section, it shows a volume integral. For a node selection, the nodal values are summed.");
	actionIntegrateSurface->setWhatsThis("<font color=\"black\">Shows a graph of the vector surface integral of the values of the current face selection.");

	// --- Record menu ---
	QAction* actionRecordNew = createAction("New ...", "actionRecordNew");
	QAction* actionRecordStart = createAction("Start", "actionRecordStart"); actionRecordStart->setShortcut(Qt::Key_F6);
	QAction* actionRecordPause = createAction("Pause", "actionRecordPause"); actionRecordPause->setShortcut(Qt::Key_F7);
	QAction* actionRecordStop = createAction("Stop", "actionRecordStop"); actionRecordStop->setShortcut(Qt::Key_F8);

	actionRecordNew->setWhatsThis("<font color=\"black\">Click this to open a file dialog box and create a new animation file.");
	actionRecordStart->setWhatsThis("<font color=\"black\">Click to start recording an animation. You must create an animation file first before you can start recording.");
	actionRecordPause->setWhatsThis("<font color=\"black\">Click this pause the current recording");
	actionRecordStop->setWhatsThis("<font color=\"black\">Click this to stop the recording. This will finalize and close the animation file as well.");


	// --- View menu ---
	actionUndoViewChange = createAction("Undo View Change", "actionUndoViewChange"); actionUndoViewChange->setShortcut(Qt::ControlModifier | Qt::Key_U);
	actionRedoViewChange = createAction("Redo View Change", "actionRedoViewChange"); actionRedoViewChange->setShortcut(Qt::ControlModifier | Qt::Key_R);
	actionShowGVContext = createAction("Show GV context menu", "actionShowGVContext");  actionShowGVContext->setShortcut(Qt::Key_V);
	actionZoomSelect = createAction("Zoom to Selection", "actionZoomSelect"); actionZoomSelect->setShortcut(Qt::Key_F);
	actionZoomExtents = createAction("Zoom to Selection", "actionZoomExtents");
	actionViewCapture = createAction("Show Capture Frame", "actionViewCapture"); actionViewCapture->setCheckable(true); actionViewCapture->setShortcut(Qt::Key_0);
	actionShowGrid = createAction("Show Grid", "actionShowGrid"); actionShowGrid->setCheckable(true); actionShowGrid->setChecked(true); actionShowGrid->setShortcut(Qt::Key_G);
	actionShowOverlay = createAction("Toggle Overlay", "actionToggleOverlay"); actionShowOverlay->setCheckable(true); actionShowOverlay->setChecked(true); actionShowOverlay->setShortcut(Qt::Key_O);
	actionShowMeshLines = createAction("Show Mesh Lines", "actionShowMeshLines", "show_mesh"); actionShowMeshLines->setCheckable(true); actionShowMeshLines->setChecked(true); actionShowMeshLines->setShortcut(Qt::Key_M);
	actionShowEdgeLines = createAction("Show Edge Lines", "actionShowEdgeLines"); actionShowEdgeLines->setCheckable(true); actionShowEdgeLines->setChecked(true); actionShowEdgeLines->setShortcut(Qt::Key_Z);
	actionViewSmooth = createAction("Color Smoothing", "actionViewSmooth"); actionViewSmooth->setShortcut(Qt::Key_C); actionViewSmooth->setCheckable(true);
	actionOrtho = createAction("Orthographic Projection", "actionOrtho"); actionOrtho->setCheckable(true); actionOrtho->setShortcut(Qt::Key_P);
	actionShowNormals = createAction("Show Normals", "actionShowNormals"); actionShowNormals->setCheckable(true); actionShowNormals->setShortcut(Qt::Key_N);
	actionRenderMode = createAction("Toggle Render Mode", "actionRenderMode"); actionRenderMode->setCheckable(true); actionRenderMode->setShortcut(Qt::Key_W);
	actionShowFibers = createAction("Show Fibers", "actionShowFibers");
	actionShowMatAxes = createAction("Toggle Material Axes", "actionShowMatAxes"); actionShowMatAxes->setCheckable(true);
	actionShowDiscrete = createAction("Show Discrete Sets", "actionShowDiscrete"); actionShowDiscrete->setCheckable(true);  actionShowDiscrete->setChecked(true);
	actionShowRigidBodies = createAction("Show Rigid Bodies", "actionShowRigidBodies"); actionShowRigidBodies->setCheckable(true);  actionShowRigidBodies->setChecked(true);
	actionShowRigidJoints = createAction("Show Rigid Joints", "actionShowRigidJoints"); actionShowRigidJoints->setCheckable(true);  actionShowRigidJoints->setChecked(true);
	actionShowRigidLabels = createAction("Show Rigid Labels", "actionShowRigidLabels"); actionShowRigidLabels->setCheckable(true);  actionShowRigidLabels->setChecked(true);
	actionToggleTagInfo = createAction("Toggle Tag info", "actionToggleTagInfo"); actionToggleTagInfo->setShortcut(Qt::Key_I);
	QAction* actionSnap3D = createAction("3D Cursor to Selection", "actionSnap3D"); actionSnap3D->setShortcut(Qt::Key_X);
	QAction* actionTrack = createAction("Track Selection", "actionTrack"); actionTrack->setShortcut(Qt::Key_Y);
	QAction* actionToggleConnected = createAction("Toggle select connected", "actionToggleConnected"); actionToggleConnected->setShortcut(Qt::Key_E);
	actionToggleLight = createAction("Toggle Lighting", "actionToggleLight", "light");
	actionFront = createAction("Front", "actionFront"); actionFront->setShortcut(Qt::Key_8 | Qt::KeypadModifier);
	actionBack = createAction("Back", "actionBack"); actionBack->setShortcut(Qt::Key_2 | Qt::KeypadModifier);
	actionRight = createAction("Right", "actionRight"); actionRight->setShortcut(Qt::Key_6 | Qt::KeypadModifier);
	actionLeft = createAction("Left", "actionLeft"); actionLeft->setShortcut(Qt::Key_4 | Qt::KeypadModifier);
	actionTop = createAction("Top", "actionTop"); actionTop->setShortcut(Qt::Key_9 | Qt::KeypadModifier);
	actionBottom = createAction("Bottom", "actionBottom"); actionBottom->setShortcut(Qt::Key_3 | Qt::KeypadModifier);
	actionIsometric = createAction("Isometric", "actionIsometric"); actionIsometric->setShortcut(Qt::Key_5 | Qt::KeypadModifier);
	QAction* actionViewVPSave = createAction("Save Viewpoint", "actionViewVPSave"); actionViewVPSave->setShortcut(Qt::CTRL | Qt::Key_K);
	QAction* actionViewVPPrev = createAction("Prev Viewpoint", "actionViewVPPrev"); actionViewVPPrev->setShortcut(Qt::Key_J);
	QAction* actionViewVPNext = createAction("Next Viewpoint", "actionViewVPNext"); actionViewVPNext->setShortcut(Qt::Key_L);
	QAction* actionSyncViews = createAction("Sync all Views", "actionSyncViews"); actionSyncViews->setShortcut(Qt::Key_S | Qt::AltModifier);

	QAction* actionToggleFPS = createAction("Toggle FPS", "actionToggleFPS"); actionToggleFPS->setShortcut(Qt::Key_F12 | Qt::ControlModifier);

	// --- Help menu ---
	QAction* actionUpdate = createAction("Check for Updates...", "actionUpdate");
	QAction* actionFEBioURL = createAction("FEBio Website", "actionFEBioURL");
	QAction* actionFEBioResources = createAction("FEBio Knowledgebase", "actionFEBioResources");
	QAction* actionFEBioUM = createAction("FEBio User Manual", "actionFEBioUM");
	QAction* actionFEBioTM = createAction("FEBio Theory Manual", "actionFEBioTM");
	QAction* actionFBSManual = createAction("FEBio Studio Manual", "actionFBSManual");
	QAction* actionFEBioForum = createAction("FEBio Forums", "actionFEBioForum");
	QAction* actionFEBioPubs = createAction("FEBio Publications", "actionFEBioPubs");
	QAction* actionWelcome = createAction("Show Welcome Page", "actionWelcome");
	QAction* actionBugReport = createAction("Submit a Bug Report", "actionBugReport");
	QAction* actionAbout = createAction("About FEBio Studio", "actionAbout");

	// other actions
	actionSelectObjects = createAction("Select Objects", "actionSelectObjects", "selectObject", true);
	actionSelectParts = createAction("Select Parts", "actionSelectParts", "selectPart", true);
	actionSelectSurfaces = createAction("Select Surfaces", "actionSelectSurfaces", "selectSurface", true);
	actionSelectCurves = createAction("Select Curves", "actionSelectCurves", "selectCurves", true);
	actionSelectNodes = createAction("Select Nodes", "actionSelectNodes", "selectNodes", true);
	actionSelectDiscrete = createAction("Select Discrete", "actionSelectDiscrete", "discrete", true);

	actionSelect = createAction("Select", "actionSelect", "select", true); actionSelect->setShortcut(Qt::Key_Q);
	actionTranslate = createAction("Translate", "actionTranslate", "translate", true); actionTranslate->setShortcut(Qt::Key_T);
	actionRotate = createAction("Rotate", "actionRotate", "rotate", true); actionRotate->setShortcut(Qt::Key_R);
	actionScale = createAction("Scale", "actionScale", "scale", true); actionScale->setShortcut(Qt::Key_S);

	selectRect = createAction("Rectangle", "selectRect", "selectRect", true);
	selectCircle = createAction("Circle", "selectCircle", "selectCircle", true);
	selectFree = createAction("Freehand", "selectFree", "selectFree", true);

	actionMeasureTool = createAction("Measure Tool", "actionMeasureTool", "measure"); actionMeasureTool->setShortcut(Qt::Key_F2);
	actionPlaneCutTool = createAction("Plane Cut", "actionPlaneCutTool", "cut");
	actionPickColor = createAction("Pick Color", "actionPickColor", "pickcolor");
	actionExplodedView = createAction("Exploded View", "actionExplodedView", "explode");

	QActionGroup* pag = new QActionGroup(m_wnd);
	pag->addAction(actionSelectObjects);
	pag->addAction(actionSelectParts);
	pag->addAction(actionSelectSurfaces);
	pag->addAction(actionSelectCurves);
	pag->addAction(actionSelectNodes);
	pag->addAction(actionSelectDiscrete);
	actionSelectObjects->setChecked(true);

	pag = new QActionGroup(m_wnd);
	pag->addAction(actionSelect);
	pag->addAction(actionTranslate);
	pag->addAction(actionRotate);
	pag->addAction(actionScale);
	actionSelect->setChecked(true);

	pag = new QActionGroup(m_wnd);
	pag->addAction(selectRect);
	pag->addAction(selectCircle);
	pag->addAction(selectFree);
	selectRect->setChecked(true);

	// Create the menu bar
	menuFile     = new QMenu("File", menuBar);
	menuEdit     = new QMenu("Edit", menuBar);
	menuEditPost = new QMenu("Edit", menuBar);
	menuEditTxt  = new QMenu("Edit", menuBar);
	menuEditXml  = new QMenu("Edit", menuBar);
	menuPhysics  = new QMenu("Physics", menuBar);
	menuFEBio    = new QMenu("FEBio", menuBar);
	menuPost     = new QMenu("Post", menuBar);
	menuRecord   = new QMenu("Record", menuBar);
	menuTools    = new QMenu("Tools", menuBar);
	menuView     = new QMenu("View", menuBar);
	menuHelp     = new QMenu("Help", menuBar);

	menuRecentFiles     = new QMenu("Open Recent");
	menuRecentProjects  = new QMenu("Open Recent Project");
	menuRecentGeomFiles = new QMenu("Import Recent Geometry");
	menuRecentImages    = new QMenu("Recent Images");

	recentFilesActionGroup = new QActionGroup(m_wnd);
	recentFilesActionGroup->setObjectName("recentFiles");

	recentProjectsActionGroup = new QActionGroup(m_wnd);
	recentProjectsActionGroup->setObjectName("recentProjects");

	recentFEFilesActionGroup = new QActionGroup(m_wnd);
	recentFEFilesActionGroup->setObjectName("recentFEFiles");

	recentGeomFilesActionGroup = new QActionGroup(m_wnd);
	recentGeomFilesActionGroup->setObjectName("recentGeomFiles");

	recentImageFilesActionGroup = new QActionGroup(m_wnd);
	recentImageFilesActionGroup->setObjectName("recentImages");

	menuImportImage = new QMenu("Import Image");

	// File menu
	menuBar->addAction(menuFile->menuAction());

	menuFile->addAction(actionNewModel);
	menuFile->addAction(actionNewProject);
	menuFile->addSeparator();
	menuFile->addAction(actionOpen);
	menuFile->addAction(menuRecentFiles->menuAction());
	menuFile->addAction(actionOpenProject);
	menuFile->addAction(menuRecentProjects->menuAction());
	menuFile->addAction(actionImportGeom);
	menuFile->addAction(menuRecentGeomFiles->menuAction());
	menuFile->addAction(actionCloseAll);
	menuFile->addSeparator();
	menuFile->addAction(actionSave);
	menuFile->addAction(actionSaveAs);
	menuFile->addAction(actionSaveAll);
	menuFile->addAction(actionSaveProject);
	menuFile->addAction(actionExportFE);
	menuFile->addAction(actionExportGeom);
#ifdef HAS_LIBZIP
	menuFile->addSeparator();
	menuFile->addAction(actionImportProject);
	menuFile->addAction(actionExportProject);
#endif
	menuFile->addSeparator();
	menuFile->addAction(menuImportImage->menuAction());
	menuImportImage->addAction(actionImportRawImage);
	menuImportImage->addAction(actionImportTiffImage);
#ifdef HAS_ITK
	menuImportImage->addAction(actionImportDICOMImage);
	menuImportImage->addAction(actionImportNrrdImage);
	menuImportImage->addAction(actionImportImageSequence);
	menuImportImage->addAction(actionImportImageOther);
#endif
	menuImportImage->addSeparator();
	menuImportImage->addAction(menuRecentImages->menuAction());

	QMenu* ConvertMenu = new QMenu("Batch convert");
	ConvertMenu->addAction(actionConvertFeb);
	ConvertMenu->addAction(actionConvertFeb2Fsm);
	ConvertMenu->addAction(actionConvertFsm2Feb);
	ConvertMenu->addAction(actionConvertGeo);

	menuFile->addAction(ConvertMenu->menuAction());
	menuFile->addSeparator();

	QMenu* BatchMenu = new QMenu("Batch Run");
	BatchMenu->addAction(actionNewBatch);
	BatchMenu->addAction(actionOpenBatch);
	menuFile->addAction(BatchMenu->menuAction());
	menuFile->addSeparator();

	menuFile->addAction(actionExit);

	QMenu* moreSelection = new QMenu("More selection options");
	moreSelection->addAction(actionFace2Elems);
	moreSelection->addAction(actionSurfaceToFaces);
	moreSelection->addAction(actionSelectOverlap);
	moreSelection->addAction(actionSelectIsolatedVertices);
	moreSelection->addAction(actionGrowSelection);
	moreSelection->addAction(actionShrinkSelection);
	moreSelection->addAction(actionSyncSelection);
	moreSelection->addAction(actionCopySelection);
	moreSelection->addAction(actionPasteSelection);

	// Edit menu
	menuBar->addAction(menuEdit->menuAction());
	menuEdit->addAction(actionUndo);
	menuEdit->addAction(actionRedo);
	menuEdit->addAction(actionChangeLog);
	menuEdit->addSeparator();
	menuEdit->addAction(actionInvertSelection);
	menuEdit->addAction(actionClearSelection);
	menuEdit->addAction(actionDeleteSelection);
	menuEdit->addAction(actionNameSelection);
	menuEdit->addSeparator();
	menuEdit->addAction(actionHideSelection);
	menuEdit->addAction(actionHideUnselected);
	menuEdit->addAction(actionUnhideAll);
	menuEdit->addAction(actionToggleVisible);
	menuEdit->addAction(moreSelection->menuAction());
	menuEdit->addSeparator();
	menuEdit->addAction(actionFind);
	menuEdit->addSeparator();
	menuEdit->addAction(actionTransform);
	menuEdit->addAction(actionCollapseTransform);
	menuEdit->addSeparator();
	menuEdit->addAction(actionClone);
	menuEdit->addAction(actionMerge);
	menuEdit->addAction(actionCopyObject);
	menuEdit->addAction(actionPasteObject);
	menuEdit->addSeparator();
	menuEdit->addAction(actionPurge);

	// Edit Post menu
	menuBar->addAction(menuEditPost->menuAction());
	menuEditPost->addAction(actionUndo);
	menuEditPost->addAction(actionRedo);
	menuEditPost->addSeparator();
	menuEditPost->addAction(actionInvertSelection);
	menuEditPost->addAction(actionClearSelection);
	menuEditPost->addAction(actionNameSelection);
	menuEditPost->addSeparator();
	menuEditPost->addAction(actionHideSelection);
	menuEditPost->addAction(actionHideUnselected);
	menuEditPost->addAction(actionUnhideAll);
	menuEditPost->addAction(actionToggleVisible);
	menuEditPost->addAction(moreSelection->menuAction());
	menuEditPost->addSeparator();
	menuEditPost->addAction(actionFind);
	menuEditPost->addAction(actionSelectRange);
	menuEditPost->addSeparator();
	menuEditPost->addAction(actionCopyObject);

	// Edit (txt) menu
	menuBar->addAction(menuEditTxt->menuAction());
	menuEditTxt->addAction(actionFindTxt);
	menuEditTxt->addAction(actionFindAgain);
	menuEditTxt->addAction(actionToggleComment);
	menuEditTxt->addAction(actionDuplicateLine);
	menuEditTxt->addAction(actionDeleteLine);

	// Edit (xml) menu
	menuBar->addAction(menuEditXml->menuAction());
	menuEditXml->addAction(actionUndo);
	menuEditXml->addAction(actionRedo);
	menuEditXml->addSeparator();
	menuEditXml->addAction(actionAddAttribute);
	menuEditXml->addAction(actionAddElement);
	menuEditXml->addAction(actionRemoveRow);

	// Physics menu
	menuBar->addAction(menuPhysics->menuAction());
	menuPhysics->addAction(assignSelection);
	menuPhysics->addAction(actionAddNodalBC);
	menuPhysics->addAction(actionAddSurfaceBC);
	menuPhysics->addAction(actionAddGeneralBC);
	menuPhysics->addAction(actionAddNodalLoad);
	menuPhysics->addAction(actionAddSurfLoad);
	menuPhysics->addAction(actionAddBodyLoad);
	menuPhysics->addAction(actionAddIC);
	menuPhysics->addAction(actionAddContact);
	menuPhysics->addAction(actionAddSurfaceNLC);
	menuPhysics->addAction(actionAddBodyNLC);
	menuPhysics->addAction(actionAddGenericNLC);
	menuPhysics->addAction(actionAddRigidBC);
	menuPhysics->addAction(actionAddRigidIC);
	menuPhysics->addAction(actionAddRigidLoad);
	menuPhysics->addAction(actionAddRigidConnector);
	menuPhysics->addAction(actionAddMaterial);
	menuPhysics->addAction(actionAddStep);
	menuPhysics->addAction(actionStepViewer);
	menuPhysics->addSeparator();
	menuPhysics->addAction(actionSoluteTable);
	menuPhysics->addAction(actionSBMTable);
	menuPhysics->addAction(actionAddReaction);
	menuPhysics->addAction(actionAddMembraneReaction);
	menuPhysics->addSeparator();
	menuPhysics->addAction(actionEditProject);

	// FEBio menu
	menuBar->addAction(menuFEBio->menuAction());
	menuFEBio->addAction(actionFEBioRun);
	menuFEBio->addAction(actionFEBioStop);
	menuFEBio->addAction(actionFEBioMonitor);
	menuFEBio->addAction(actionFEBioOptimize);
	menuFEBio->addAction(actionFEBioTangent);
	menuFEBio->addAction(actionFEBioInfo);
    menuFEBio->addAction(actionPluginRepo);
	menuFEBio->addAction(actionCreatePlugin);

	// Post menu
	menuBar->addAction(menuPost->menuAction());
	menuPost->addAction(actionPlaneCut);
	menuPost->addAction(actionMirrorPlane);
	menuPost->addAction(actionVectorPlot);
	menuPost->addAction(actionTensorPlot);
	menuPost->addAction(actionIsosurfacePlot);
	menuPost->addAction(actionSlicePlot);
	menuPost->addAction(actionDisplacementMap);
	menuPost->addAction(actionStreamLinePlot);
	menuPost->addAction(actionParticleFlowPlot);
	menuPost->addAction(actionVolumeFlowPlot);
	menuPost->addAction(actionAddProbe);
	menuPost->addAction(actionAddCurveProbe);
	menuPost->addAction(actionAddRuler);
	menuPost->addAction(actionMusclePath);
	menuPost->addAction(actionVectorGlyph);
	menuPost->addAction(actionHelicalAxis);
	menuPost->addAction(actionStaticMesh);
	menuPost->addAction(actionPlotGroup);
	menuPost->addSeparator();
	menuPost->addAction(actionImageSlicer);
	menuPost->addAction(actionVolumeRender);
	menuPost->addAction(actionMarchingCubes);
	menuPost->addAction(actionImageWarp);
	menuPost->addSeparator();
	menuPost->addAction(actionGraph);
	menuPost->addAction(actionScatter);
	menuPost->addAction(actionSummary);
	menuPost->addAction(actionStats);
	menuPost->addAction(actionIntegrate);
	menuPost->addAction(actionIntegrateSurface);
	menuPost->addSeparator();
	menuPost->addAction(actionImportPoints);
	menuPost->addAction(actionImportLines);

	// Record menu
	menuBar->addAction(menuRecord->menuAction());
	menuRecord->addAction(actionRecordNew);
	menuRecord->addSeparator();
	menuRecord->addAction(actionRecordStart);
	menuRecord->addAction(actionRecordPause);
	menuRecord->addAction(actionRecordStop);

	// Tools menu
	menuBar->addAction(menuTools->menuAction());
	menuTools->addAction(actionCurveEditor);
	menuTools->addAction(actionMeshInspector);
	menuTools->addAction(actionMeshDiagnostic);
	menuTools->addAction(actionUnitConverter);
	menuTools->addAction(actionElasticityConvertor);
	menuTools->addAction(actionMaterialTest);
	menuTools->addAction(actionDistroVisual);
	menuTools->addAction(actionKinemat);
	menuTools->addAction(actionPlotMix);
	menuTools->addAction(actionPython);
	menuTools->addAction(actionSettings);

	// View menu
	menuBar->addAction(menuView->menuAction());
	menuView->addAction(actionUndoViewChange);
	menuView->addAction(actionRedoViewChange);
	menuView->addAction(actionShowGVContext);
	menuView->addSeparator();
	menuView->addAction(actionZoomSelect);
	menuView->addAction(actionOrtho);
	menuView->addAction(actionShowNormals);
	menuView->addAction(actionViewCapture);
	menuView->addAction(actionShowOverlay);
	menuView->addSeparator();
	menuView->addAction(actionShowGrid);
	menuView->addAction(actionShowMeshLines);
	menuView->addAction(actionShowEdgeLines);
	menuView->addAction(actionViewSmooth);
	menuView->addAction(actionRenderMode);
	menuView->addAction(actionShowFibers);
	menuView->addAction(actionShowMatAxes);
	menuView->addAction(actionShowDiscrete);
	menuView->addAction(actionToggleTagInfo);
	menuView->addAction(actionSnap3D);
	menuView->addAction(actionTrack);
	menuView->addAction(actionToggleLight);
	menuView->addAction(actionToggleConnected);
	menuView->addAction(actionToggleFPS);
	menuView->addSeparator();

	menuViews = menuView->addMenu("Standard views");
	menuViews->addAction(actionFront);
	menuViews->addAction(actionBack);
	menuViews->addAction(actionRight);
	menuViews->addAction(actionLeft);
	menuViews->addAction(actionTop);
	menuViews->addAction(actionBottom);
	menuViews->addAction(actionIsometric);

	menuViews->addSeparator();
	menuView->addAction(actionViewVPSave);
	menuView->addAction(actionViewVPPrev);
	menuView->addAction(actionViewVPNext);
	menuView->addAction(actionSyncViews);
	menuView->addSeparator();

	menuWindows = menuView->addMenu("Windows");

#ifndef NDEBUG
	menuView->addAction(actionLayerInfo);
#endif

	// Help menu
	menuBar->addAction(menuHelp->menuAction());
	if (wnd->updaterPresent())
	{
		menuHelp->addAction(actionUpdate);
		menuHelp->addSeparator();
	}
	menuHelp->addAction(actionWelcome);
	menuHelp->addSeparator();
	menuHelp->addAction(actionFEBioURL);
	menuHelp->addAction(actionFEBioResources);
	menuHelp->addAction(actionFEBioUM);
	menuHelp->addAction(actionFEBioTM);
	menuHelp->addAction(actionFBSManual);
	menuHelp->addAction(actionFEBioForum);
	menuHelp->addAction(actionFEBioPubs);
	menuHelp->addSeparator();
	menuHelp->addAction(actionBugReport);
	menuHelp->addSeparator();
	menuHelp->addAction(actionAbout);
}

QAction* CMainMenu::createAction(const QString& title, const QString& name, const QString& iconFile, bool bcheckable, const QKeySequence& key)
{
	QAction* pa = new QAction(title, m_wnd);
	pa->setObjectName(name);
	if (iconFile.isEmpty() == false) pa->setIcon(CIconProvider::GetIcon(iconFile));
	if (bcheckable) pa->setCheckable(true);
	if (key != QKeySequence::UnknownKey) pa->setShortcut(key);
	return pa;
}
