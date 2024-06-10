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
#include "ui_mainwindow.h"
#include <QMenuBar>
#include "IconProvider.h"

Ui::CMainWindow::CMainWindow()
{
	m_settings.uiTheme = 0;
	m_settings.defaultUnits = 0;
	m_settings.clearUndoOnSave = true;
	m_settings.autoSaveInterval = 600;
	m_settings.loadFEBioConfigFile = true;
	m_settings.febioConfigFileName = "$(FEBioStudioDir)/febio.xml";
}

void Ui::CMainWindow::setupUi(::CMainWindow* wnd)
{
	m_wnd = wnd;

#ifdef WIN32
	m_defaultProjectParent = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
#endif
	m_jobManager = new CFEBioJobManager(wnd);

	m_isAnimating = false;

	// Check if updater is present 
	m_updaterPresent = QFile::exists(m_updateWidget.getUpdaterPath());
	m_updateAvailable = false;
	m_updateOnClose = false;
	m_updateDevChannel = false;

	// initialize current path
	m_currentPath = QDir::currentPath();

	// set the initial window size
//        QRect screenSize = QDesktopWidget().availableGeometry(wnd);
		//wnd->resize(QSize(screenSize.width() * 1.0f, screenSize.height() * 1.0f));
//		wnd->resize(800, 600);

		// build the central widget
	centralWidget = new CMainCentralWidget(wnd);
	wnd->setCentralWidget(centralWidget);

	// build the menu
	buildMenu(wnd);

	// build the dockable windows
	// (must be done after menu is created)
	buildDockWidgets(wnd);

	// build status bar
	statusBar = new QStatusBar(m_wnd);
	m_wnd->setStatusBar(statusBar);

	progressBar = new QProgressBar;
	progressBar->setMaximumWidth(200);
	progressBar->setMaximumHeight(15);

	BuildConfigs();

	QMetaObject::connectSlotsByName(wnd);

	QObject::connect(modelViewer, &::CModelViewer::currentObjectChanged, imageSettingsPanel, &::CImageSettingsPanel::ModelTreeSelectionChanged);
	QObject::connect(modelViewer, &::CModelViewer::currentObjectChanged, centralWidget->sliceView, &::CImageSliceView::ModelTreeSelectionChanged);
	QObject::connect(modelViewer, &::CModelViewer::currentObjectChanged, centralWidget->timeView2D, &::C2DImageTimeView::ModelTreeSelectionChanged);
}

QAction* Ui::CMainWindow::addAction(const QString& title, const QString& name, const QString& iconFile, bool bcheckable)
{
	QAction* pa = new QAction(title, m_wnd);
	pa->setObjectName(name);
	if (iconFile.isEmpty() == false) pa->setIcon(CIconProvider::GetIcon(iconFile));
	if (bcheckable) pa->setCheckable(true);
	return pa;
}

void Ui::CMainWindow::buildMenu(::CMainWindow* mainWindow)
{
	// --- File menu actions ---
	QAction* actionNewModel = addAction("New Model ...", "actionNewModel", "new");
	QAction* actionNewProject = addAction("New Project ...", "actionNewProject");
	QAction* actionOpen = addAction("Open Model File ...", "actionOpen", "open"); actionOpen->setShortcuts(QKeySequence::Open);
	QAction* actionSave = addAction("Save", "actionSave", "save"); actionSave->setShortcuts(QKeySequence::Save);
	QAction* actionSaveAs = addAction("Save As ...", "actionSaveAs"); actionSaveAs->setShortcuts(QKeySequence::SaveAs);
	QAction* actionSaveAll = addAction("Save All", "actionSaveAll"); actionSaveAll->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_S);
	QAction* actionCloseAll = addAction("Close All", "actionCloseAll");
	actionSnapShot = addAction("Snapshot ...", "actionSnapShot", "snapshot"); actionSnapShot->setShortcut(Qt::AltModifier | Qt::Key_F2);
	QAction* actionSaveProject = addAction("Save Project As ...", "actionSaveProject");
	actionExportFE = addAction("Export FE model ...", "actionExportFEModel");
	actionImportGeom = addAction("Import Geometry ...", "actionImportGeometry");
	actionExportGeom = addAction("Export Geometry ...", "actionExportGeometry");
	QAction* actionOpenProject = addAction("Open Project ...", "actionOpenProject");
	QAction* actionImportProject = addAction("Import Project Archive ...", "actionImportProject");
	QAction* actionExportProject = addAction("Export Project Archive ...", "actionExportProject");
	QAction* actionImportRawImage = addAction("Raw ...", "actionImportRawImage");
	QAction* actionImportDICOMImage = addAction("DICOM/DICOM Sequence ...", "actionImportDICOMImage");
	QAction* actionImportTiffImage = addAction("Tiff ...", "actionImportTiffImage");
	QAction* actionImportOMETiffImage = addAction("OME Tiff ...", "actionImportOMETiffImage");
	QAction* actionImportNrrdImage = addAction("NRRD ...", "actionImportNrrdImage");
	QAction* actionImportImageSequence = addAction("Image Sequence ...", "actionImportImageSequence");
	QAction* actionImportImageOther = addAction("Other ...", "actionImportImageOther");
	QAction* actionConvertFeb = addAction("FEBio Files ...", "actionConvertFeb");
	QAction* actionConvertFeb2Fsm = addAction("FEB to FSM ...", "actionConvertFeb2Fsm");
	QAction* actionConvertFsm2Feb = addAction("FSM to FEB ...", "actionConvertFsm2Feb");
	QAction* actionConvertGeo = addAction("Geometry Files ...", "actionConvertGeo");
	QAction* actionExit = addAction("Exit", "actionExit");

	// --- Edit menu actions ---
	QAction* actionUndo = addAction("Undo", "actionUndo", "undo"); actionUndo->setShortcuts(QKeySequence::Undo);
	QAction* actionRedo = addAction("Redo", "actionRedo", "redo"); actionRedo->setShortcuts(QKeySequence::Redo);
	QAction* actionInvertSelection = addAction("Invert Selection", "actionInvertSelection"); actionInvertSelection->setShortcut(Qt::AltModifier | Qt::Key_I);
	QAction* actionClearSelection = addAction("Clear Selection", "actionClearSelection");
	QAction* actionDeleteSelection = addAction("Delete Selection", "actionDeleteSelection"); actionDeleteSelection->setShortcuts(QKeySequence::Delete);
	QAction* actionNameSelection = addAction("Name Selection ...", "actionNameSelection"); actionNameSelection->setShortcut(Qt::ControlModifier | Qt::Key_G);
	QAction* actionHideSelection = addAction("Hide Selection", "actionHideSelection"); actionHideSelection->setShortcut(Qt::Key_H);
	QAction* actionHideUnselected = addAction("Hide Unselected", "actionHideUnselected"); actionHideUnselected->setShortcut(Qt::ShiftModifier | Qt::Key_H);
	QAction* actionSyncSelection = addAction("Sync selection", "actionSyncSelection"); actionSyncSelection->setShortcut(Qt::AltModifier | Qt::Key_F);
	QAction* actionCopySelection = addAction("Copy selection", "actionCopySelection");
	QAction* actionPasteSelection = addAction("Paste selection", "actionPasteSelection");
	QAction* actionUnhideAll = addAction("Unhide All", "actionUnhideAll");
	QAction* actionFind = addAction("Find ...", "actionFind"); //actionFind->setShortcut(Qt::ControlModifier | Qt::Key_F);
	QAction* actionSelectRange = addAction("Select Range ...", "actionSelectRange");
	QAction* actionToggleVisible = addAction("Toggle Visibility", "actionToggleVisible", "toggle_visible");
	QAction* actionTransform = addAction("Transform ...", "actionTransform"); actionTransform->setShortcut(Qt::ControlModifier | Qt::Key_T);
	QAction* actionCollapseTransform = addAction("Collapse Transform", "actionCollapseTransform");
	QAction* actionClone = addAction("Clone Object ...", "actionClone", "clone"); // actionClone->setShortcut(Qt::ControlModifier | Qt::Key_D);
	QAction* actionCopyObject = addAction("Copy Object", "actionCopyObject");
	QAction* actionPasteObject = addAction("Paste Object", "actionPasteObject");
	QAction* actionCloneGrid = addAction("Clone Grid ...", "actionCloneGrid", "clonegrid");
	QAction* actionCloneRevolve = addAction("Clone Revolve ...", "actionCloneRevolve", "clonerevolve");
	QAction* actionMerge = addAction("Merge Objects ...", "actionMerge", "merge");
	QAction* actionDetach = addAction("Detach Elements", "actionDetach", "detach");
	QAction* actionExtract = addAction("Extract Faces", "actionExtract", "extract");
	QAction* actionPurge = addAction("Purge ...", "actionPurge");

	QAction* actionFace2Elems = addAction("Face to Element Selection", "actionFaceToElem");
	QAction* actionSurfaceToFaces = addAction("Surface to Face Selection", "actionSurfaceToFaces");
	QAction* actionSelectOverlap = addAction("Select surface overlap ...", "actionSelectOverlap");
	QAction* actionSelectIsolatedVertices = addAction("Select isolated vertices", "actionSelectIsolatedVertices");
	QAction* actionGrowSelection = addAction("Grow selection", "actionGrowSelection"); actionGrowSelection->setShortcut(Qt::ControlModifier | Qt::Key_Plus);
	QAction* actionShrinkSelection = addAction("Shrink selection", "actionShrinkSelection"); actionShrinkSelection->setShortcut(Qt::ControlModifier | Qt::Key_Minus);

	// --- Edit (txt) menu actions ---
	QAction* actionFindTxt = addAction("Find ...", "actionFindTxt"); actionFindTxt->setShortcut(Qt::Key_F | Qt::ControlModifier);
	QAction* actionFindAgain = addAction("Find Again", "actionFindAgain"); actionFindAgain->setShortcut(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_F);
	QAction* actionToggleComment = addAction("Toggle Line Comment", "actionToggleComment"); actionToggleComment->setShortcut(Qt::ControlModifier | Qt::Key_Slash);
	QAction* actionDuplicateLine = addAction("Copy Line Down", "actionDuplicateLine"); actionDuplicateLine->setShortcut(Qt::ControlModifier | Qt::Key_D);
	QAction* actionDeleteLine = addAction("Delete Line", "actionDeleteLine"); actionDeleteLine->setShortcut(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_L);

	// -- Edit (xml) menu actions ---
	QAction* actionAddAttribute = addAction("Add Attribute", "actionAddAttribute");
	actionAddAttribute->setIcon(CIconProvider::GetIcon("selectAdd", QColor(Qt::red), Shape::Circle));

	QAction* actionAddElement = addAction("Add Element", "actionAddElement");
	actionAddElement->setIcon(CIconProvider::GetIcon("selectAdd", QColor(Qt::blue), Shape::Circle));

	QAction* actionRemoveRow = addAction("Remove Row", "actionRemoveRow", "selectDel");

	// --- Physics menu actions ---
	QAction* actionAddNodalBC = addAction("Add Nodal BC...", "actionAddNodalBC"); actionAddNodalBC->setShortcut(Qt::ControlModifier | Qt::Key_B);
	QAction* actionAddSurfaceBC = addAction("Add Surface BC...", "actionAddSurfaceBC");
	QAction* actionAddGeneralBC = addAction("Add Linear Constraint ...", "actionAddGeneralBC");
	QAction* actionAddNodalLoad = addAction("Add Nodal Load ...", "actionAddNodalLoad");
	QAction* actionAddSurfLoad = addAction("Add Surface Load ...", "actionAddSurfLoad"); actionAddSurfLoad->setShortcut(Qt::ControlModifier | Qt::Key_L);
	QAction* actionAddBodyLoad = addAction("Add Body Load ...", "actionAddBodyLoad");
	QAction* actionAddIC = addAction("Add Initial Condition ...", "actionAddIC"); actionAddIC->setShortcut(Qt::ControlModifier | Qt::Key_I);
	QAction* actionAddContact = addAction("Add Contact ...", "actionAddContact");
	QAction* actionAddSurfaceNLC = addAction("Add Surface Constraint...", "actionAddSurfaceNLC");
	QAction* actionAddBodyNLC = addAction("Add Body Constraint...", "actionAddBodyNLC");
	QAction* actionAddGenericNLC = addAction("Add General Constraint...", "actionAddGenericNLC");
	QAction* actionAddRigidBC = addAction("Add Rigid Constraint ...", "actionAddRigidBC");
	QAction* actionAddRigidIC = addAction("Add Rigid Initial Condition ...", "actionAddRigidIC");
	QAction* actionAddRigidLoad = addAction("Add Rigid Load ...", "actionAddRigidLoad");
	QAction* actionAddRigidConnector = addAction("Add Rigid Connector ...", "actionAddRigidConnector");
	QAction* actionAddStep = addAction("Add Analysis Step ...", "actionAddStep");
	QAction* actionStepViewer = addAction("Step Viewer ...", "actionStepViewer");
	QAction* actionAddMaterial = addAction("Add Material ...", "actionAddMaterial", "material"); actionAddMaterial->setShortcut(Qt::ControlModifier | Qt::Key_M);
	actionSoluteTable = addAction("Solute Table ...", "actionSoluteTable");
	actionSBMTable = addAction("Solid-bound Molecule Table ...", "actionSBMTable");
	actionAddReaction = addAction("Chemical Reaction Editor ...", "actionAddReaction");
	actionAddMembraneReaction = addAction("Membrane Reaction Editor ...", "actionAddMembraneReaction");
	QAction* actionEditProject = addAction("Edit Physics Modules ...", "actionEditProject");
	QAction* assignSelection = addAction("Assign current selection", "actionAssignSelection"); assignSelection->setShortcut(Qt::ControlModifier | Qt::Key_A);

	// --- FEBio menu actions ---
	actionFEBioRun = addAction("Run FEBio ...", "actionFEBioRun", "febiorun"); actionFEBioRun->setShortcut(Qt::Key_F5);
	actionFEBioStop = addAction("Stop FEBio", "actionFEBioStop");
	actionFEBioCheck = addAction("Model check ...", "actionFEBioCheck");
	QAction* actionFEBioOptimize = addAction("Generate optimization file ...", "actionFEBioOptimize");
	QAction* actionFEBioTangent = addAction("Generate tangent diagnostic ...", "actionFEBioTangent");
	QAction* actionFEBioInfo = addAction("FEBio Info ...", "actionFEBioInfo");
	QAction* actionFEBioPlugins = addAction("Manage FEBio plugins ...", "actionFEBioPlugins");

	// --- Tools menu ---
	actionCurveEditor = addAction("Curve Editor ...", "actionCurveEditor", "curves"); actionCurveEditor->setShortcut(Qt::Key_F9);
	actionMeshInspector = addAction("Mesh Inspector ...", "actionMeshInspector", "inspect"); actionMeshInspector->setShortcut(Qt::Key_F10);
	actionMeshDiagnostic = addAction("Mesh Diagnostic ...", "actionMeshDiagnostic"); actionMeshDiagnostic->setShortcut(Qt::Key_F11);
	QAction* actionElasticityConvertor = addAction("Elasticity Converter ...", "actionElasticityConvertor");
	QAction* actionUnitConverter = addAction("Unit Converter ...", "actionUnitConverter");
	actionMaterialTest = addAction("Material test ...", "actionMaterialTest");
	QAction* actionKinemat = addAction("Kinemat ...", "actionKinemat");
	QAction* actionPlotMix = addAction("Plotmix ...", "actionPlotMix");
	actionOptions = addAction("Options ...", "actionOptions"); actionOptions->setShortcut(Qt::Key_F12);

	QAction* actionLayerInfo = addAction("Print Layer Info", "actionLayerInfo"); actionLayerInfo->setShortcut(Qt::AltModifier | Qt::Key_L);

	// --- Post menu ---
	QAction* actionPlaneCut = addAction("Planecut", "actionPlaneCut", "cut");
	QAction* actionMirrorPlane = addAction("Mirror Plane", "actionMirrorPlane", "mirror");
	QAction* actionVectorPlot = addAction("Vector Plot", "actionVectorPlot", "vectors");
	QAction* actionTensorPlot = addAction("Tensor Plot", "actionTensorPlot", "tensor");
	QAction* actionIsosurfacePlot = addAction("Isosurface Plot", "actionIsosurfacePlot", "isosurface");
	QAction* actionSlicePlot = addAction("Slice Plot", "actionSlicePlot", "sliceplot");
	QAction* actionDisplacementMap = addAction("Displacement Map", "actionDisplacementMap", "distort");
	QAction* actionStreamLinePlot = addAction("Streamlines Plot", "actionStreamLinePlot", "streamlines");
	QAction* actionParticleFlowPlot = addAction("Particleflow Plot", "actionParticleFlowPlot", "particle");
	QAction* actionVolumeFlowPlot = addAction("Volumeflow Plot", "actionVolumeFlowPlot", "flow");

	QAction* actionImageSlicer = addAction("Image Slicer", "actionImageSlicer", "imageslice");
	QAction* actionVolumeRender = addAction("Volume Render", "actionVolumeRender", "volrender");
	QAction* actionMarchingCubes = addAction("Image Isosurface", "actionMarchingCubes", "marching_cubes");
	QAction* actionImageWarp = addAction("Image Warp", "actionImageWarp");

	QAction* actionAddProbe = addAction("Point Probe", "actionAddProbe", "probe");
	QAction* actionAddCurveProbe = addAction("Curve Probe", "actionAddCurveProbe");
	QAction* actionAddRuler = addAction("Ruler", "actionAddRuler", "ruler");
	QAction* actionMusclePath = addAction("Muscle Path", "actionMusclePath", "musclepath");
	QAction* actionPlotGroup = addAction("Plot Group", "actionPlotGroup", "folder");
	QAction* actionGraph = addAction("New Graph ...", "actionGraph", "chart"); actionGraph->setShortcut(Qt::Key_F3);
	QAction* actionScatter = addAction("Scatter plot ...", "actionScatter");
	QAction* actionSummary = addAction("Summary ...", "actionSummary"); actionSummary->setShortcut(Qt::Key_F4);
	QAction* actionStats = addAction("Statistics  ...", "actionStats");
	QAction* actionIntegrate = addAction("Integrate ...", "actionIntegrate", "integrate");
	QAction* actionIntegrateSurface = addAction("Integrate Surface ...", "actionIntegrateSurface");
	QAction* actionImportPoints = addAction("Import Points ...", "actionImportPoints");
	QAction* actionImportLines = addAction("Import Lines ...", "actionImportLines");

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
	QAction* actionRecordNew = addAction("New ...", "actionRecordNew");
	QAction* actionRecordStart = addAction("Start", "actionRecordStart"); actionRecordStart->setShortcut(Qt::Key_F6);
	QAction* actionRecordPause = addAction("Pause", "actionRecordPause"); actionRecordPause->setShortcut(Qt::Key_F7);
	QAction* actionRecordStop = addAction("Stop", "actionRecordStop"); actionRecordStop->setShortcut(Qt::Key_F8);

	actionRecordNew->setWhatsThis("<font color=\"black\">Click this to open a file dialog box and create a new animation file.");
	actionRecordStart->setWhatsThis("<font color=\"black\">Click to start recording an animation. You must create an animation file first before you can start recording.");
	actionRecordPause->setWhatsThis("<font color=\"black\">Click this pause the current recording");
	actionRecordStop->setWhatsThis("<font color=\"black\">Click this to stop the recording. This will finalize and close the animation file as well.");


	// --- View menu ---
	actionUndoViewChange = addAction("Undo View Change", "actionUndoViewChange"); actionUndoViewChange->setShortcut(Qt::ControlModifier | Qt::Key_U);
	actionRedoViewChange = addAction("Redo View Change", "actionRedoViewChange"); actionRedoViewChange->setShortcut(Qt::ControlModifier | Qt::Key_R);
	actionZoomSelect = addAction("Zoom to Selection", "actionZoomSelect"); actionZoomSelect->setShortcut(Qt::Key_F);
	actionZoomExtents = addAction("Zoom to Selection", "actionZoomExtents");
	actionViewCapture = addAction("Show Capture Frame", "actionViewCapture"); actionViewCapture->setCheckable(true); actionViewCapture->setShortcut(Qt::Key_0);
	actionShowGrid = addAction("Show Grid", "actionShowGrid"); actionShowGrid->setCheckable(true); actionShowGrid->setChecked(true); actionShowGrid->setShortcut(Qt::Key_G);
	actionShowMeshLines = addAction("Show Mesh Lines", "actionShowMeshLines", "show_mesh"); actionShowMeshLines->setCheckable(true); actionShowMeshLines->setChecked(true); actionShowMeshLines->setShortcut(Qt::Key_M);
	actionShowEdgeLines = addAction("Show Edge Lines", "actionShowEdgeLines"); actionShowEdgeLines->setCheckable(true); actionShowEdgeLines->setChecked(true); actionShowEdgeLines->setShortcut(Qt::Key_Z);
	actionBackfaceCulling = addAction("Backface Culling", "actionBackfaceCulling"); actionBackfaceCulling->setCheckable(true);
	actionViewSmooth = addAction("Color Smoothing", "actionViewSmooth"); actionViewSmooth->setShortcut(Qt::Key_C); actionViewSmooth->setCheckable(true);
	actionOrtho = addAction("Orthographic Projection", "actionOrtho"); actionOrtho->setCheckable(true); actionOrtho->setShortcut(Qt::Key_P);
	actionShowNormals = addAction("Show Normals", "actionShowNormals"); actionShowNormals->setCheckable(true); actionShowNormals->setShortcut(Qt::Key_N);
	actionRenderMode = addAction("Toggle Render Mode", "actionRenderMode"); actionRenderMode->setCheckable(true); actionRenderMode->setShortcut(Qt::Key_W);
	actionShowFibers = addAction("Show Fibers", "actionShowFibers");
	actionShowMatAxes = addAction("Toggle Material Axes", "actionShowMatAxes"); actionShowMatAxes->setCheckable(true);
	actionShowDiscrete = addAction("Show Discrete Sets", "actionShowDiscrete"); actionShowDiscrete->setCheckable(true);  actionShowDiscrete->setChecked(true);
	actionShowRigidBodies = addAction("Show Rigid Bodies", "actionShowRigidBodies"); actionShowRigidBodies->setCheckable(true);  actionShowRigidBodies->setChecked(true);
	actionShowRigidJoints = addAction("Show Rigid Joints", "actionShowRigidJoints"); actionShowRigidJoints->setCheckable(true);  actionShowRigidJoints->setChecked(true);
	actionShowRigidLabels = addAction("Show Rigid Labels", "actionShowRigidLabels"); actionShowRigidLabels->setCheckable(true);  actionShowRigidLabels->setChecked(true);
	actionToggleTagInfo = addAction("Toggle Tag info", "actionToggleTagInfo"); actionToggleTagInfo->setShortcut(Qt::Key_I);
	QAction* actionSnap3D = addAction("3D Cursor to Selection", "actionSnap3D"); actionSnap3D->setShortcut(Qt::Key_X);
	QAction* actionTrack = addAction("Track Selection", "actionTrack"); actionTrack->setShortcut(Qt::Key_Y);
	QAction* actionToggleConnected = addAction("Toggle select connected", "actionToggleConnected"); actionToggleConnected->setShortcut(Qt::Key_E);
	actionToggleLight = addAction("Toggle Lighting", "actionToggleLight", "light");
	actionFront = addAction("Front", "actionFront"); actionFront->setShortcut(Qt::Key_8 | Qt::KeypadModifier);
	actionBack = addAction("Back", "actionBack"); actionBack->setShortcut(Qt::Key_2 | Qt::KeypadModifier);
	actionRight = addAction("Right", "actionRight"); actionRight->setShortcut(Qt::Key_6 | Qt::KeypadModifier);
	actionLeft = addAction("Left", "actionLeft"); actionLeft->setShortcut(Qt::Key_4 | Qt::KeypadModifier);
	actionTop = addAction("Top", "actionTop"); actionTop->setShortcut(Qt::Key_9 | Qt::KeypadModifier);
	actionBottom = addAction("Bottom", "actionBottom"); actionBottom->setShortcut(Qt::Key_3 | Qt::KeypadModifier);
	actionIsometric = addAction("Isometric", "actionIsometric"); actionIsometric->setShortcut(Qt::Key_5 | Qt::KeypadModifier);
	QAction* actionViewVPSave = addAction("Save Viewpoint", "actionViewVPSave"); actionViewVPSave->setShortcut(Qt::CTRL | Qt::Key_K);
	QAction* actionViewVPPrev = addAction("Prev Viewpoint", "actionViewVPPrev"); actionViewVPPrev->setShortcut(Qt::Key_J);
	QAction* actionViewVPNext = addAction("Next Viewpoint", "actionViewVPNext"); actionViewVPNext->setShortcut(Qt::Key_L);
	QAction* actionSyncViews = addAction("Sync all Views", "actionSyncViews"); actionSyncViews->setShortcut(Qt::Key_S | Qt::AltModifier);

	QAction* actionToggleFPS = addAction("Toggle FPS", "actionToggleFPS"); actionToggleFPS->setShortcut(Qt::Key_F12 | Qt::ControlModifier);

	// --- Help menu ---
	QAction* actionUpdate = addAction("Check for Updates...", "actionUpdate");
	QAction* actionFEBioURL = addAction("FEBio Website", "actionFEBioURL");
	QAction* actionFEBioResources = addAction("FEBio Knowledgebase", "actionFEBioResources");
	QAction* actionFEBioUM = addAction("FEBio User Manual", "actionFEBioUM");
	QAction* actionFEBioTM = addAction("FEBio Theory Manual", "actionFEBioTM");
	QAction* actionFBSManual = addAction("FEBio Studio Manual", "actionFBSManual");
	QAction* actionFEBioForum = addAction("FEBio Forums", "actionFEBioForum");
	QAction* actionFEBioPubs = addAction("FEBio Publications", "actionFEBioPubs");
	QAction* actionWelcome = addAction("Show Welcome Page", "actionWelcome");
	QAction* actionBugReport = addAction("Submit a Bug Report", "actionBugReport");
	QAction* actionAbout = addAction("About FEBio Studio", "actionAbout");

	// other actions
	actionSelectObjects = addAction("Select Objects", "actionSelectObjects", "selectObject", true);
	actionSelectParts = addAction("Select Parts", "actionSelectParts", "selectPart", true);
	actionSelectSurfaces = addAction("Select Surfaces", "actionSelectSurfaces", "selectSurface", true);
	actionSelectCurves = addAction("Select Curves", "actionSelectCurves", "selectCurves", true);
	actionSelectNodes = addAction("Select Nodes", "actionSelectNodes", "selectNodes", true);
	actionSelectDiscrete = addAction("Select Discrete", "actionSelectDiscrete", "discrete", true);

	QAction* actionSelect = addAction("Select", "actionSelect", "select", true); actionSelect->setShortcut(Qt::Key_Q);
	actionTranslate = addAction("Translate", "actionTranslate", "translate", true); actionTranslate->setShortcut(Qt::Key_T);
	actionRotate = addAction("Rotate", "actionRotate", "rotate", true); actionRotate->setShortcut(Qt::Key_R);
	QAction* actionScale = addAction("Scale", "actionScale", "scale", true); actionScale->setShortcut(Qt::Key_S);

	selectRect = addAction("Rectangle", "selectRect", "selectRect", true);
	selectCircle = addAction("Circle", "selectCircle", "selectCircle", true);
	selectFree = addAction("Freehand", "selectFree", "selectFree", true);

	actionMeasureTool = addAction("Measure Tool", "actionMeasureTool", "measure"); actionMeasureTool->setShortcut(Qt::Key_F2);
	actionPlaneCutTool = addAction("Plane Cut", "actionPlaneCutTool", "cut");

	QActionGroup* pag = new QActionGroup(mainWindow);
	pag->addAction(actionSelectObjects);
	pag->addAction(actionSelectParts);
	pag->addAction(actionSelectSurfaces);
	pag->addAction(actionSelectCurves);
	pag->addAction(actionSelectNodes);
	pag->addAction(actionSelectDiscrete);
	actionSelectObjects->setChecked(true);

	pag = new QActionGroup(mainWindow);
	pag->addAction(actionSelect);
	pag->addAction(actionTranslate);
	pag->addAction(actionRotate);
	pag->addAction(actionScale);
	actionSelect->setChecked(true);

	pag = new QActionGroup(mainWindow);
	pag->addAction(selectRect);
	pag->addAction(selectCircle);
	pag->addAction(selectFree);
	selectRect->setChecked(true);

	// Create the menu bar
	QMenuBar* menuBar = m_wnd->menuBar();
	menuFile = new QMenu("File", menuBar);
	menuEdit = new QMenu("Edit", menuBar);
	menuEditPost = new QMenu("Edit", menuBar);
	menuEditTxt = new QMenu("Edit", menuBar);
	menuEditXml = new QMenu("Edit", menuBar);
	menuPhysics = new QMenu("Physics", menuBar);
	menuFEBio = new QMenu("FEBio", menuBar);
	menuPost = new QMenu("Post", menuBar);
	menuRecord = new QMenu("Record", menuBar);
	menuTools = new QMenu("Tools", menuBar);
	menuView = new QMenu("View", menuBar);
	menuHelp = new QMenu("Help", menuBar);

	menuRecentFiles = new QMenu("Open Recent");
	menuRecentProjects = new QMenu("Open Recent Project");
	menuRecentGeomFiles = new QMenu("Import Recent Geometry");

	recentFilesActionGroup = new QActionGroup(mainWindow);
	recentFilesActionGroup->setObjectName("recentFiles");

	recentProjectsActionGroup = new QActionGroup(mainWindow);
	recentProjectsActionGroup->setObjectName("recentProjects");

	recentFEFilesActionGroup = new QActionGroup(mainWindow);
	recentFEFilesActionGroup->setObjectName("recentFEFiles");

	recentGeomFilesActionGroup = new QActionGroup(mainWindow);
	recentGeomFilesActionGroup->setObjectName("recentGeomFiles");

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
	menuImportImage->addAction(actionImportDICOMImage);
	menuImportImage->addAction(actionImportTiffImage);
	menuImportImage->addAction(actionImportNrrdImage);
	//		menuImportImage->addAction(actionImportOMETiffImage); // NOTE: Commented out because this requires Java!
	menuImportImage->addAction(actionImportImageSequence);
	menuImportImage->addAction(actionImportImageOther);


	QMenu* ConvertMenu = new QMenu("Batch convert");
	ConvertMenu->addAction(actionConvertFeb);
	ConvertMenu->addAction(actionConvertFeb2Fsm);
	ConvertMenu->addAction(actionConvertFsm2Feb);
	ConvertMenu->addAction(actionConvertGeo);

	menuFile->addAction(ConvertMenu->menuAction());
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
	menuFEBio->addAction(actionFEBioCheck);
	menuFEBio->addAction(actionFEBioOptimize);
	menuFEBio->addAction(actionFEBioTangent);
	menuFEBio->addAction(actionFEBioInfo);
	menuFEBio->addAction(actionFEBioPlugins);

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
	menuTools->addAction(actionKinemat);
	menuTools->addAction(actionPlotMix);
	menuTools->addAction(actionOptions);

	// View menu
	menuBar->addAction(menuView->menuAction());
	menuView->addAction(actionUndoViewChange);
	menuView->addAction(actionRedoViewChange);
	menuView->addSeparator();
	menuView->addAction(actionZoomSelect);
	menuView->addAction(actionOrtho);
	menuView->addAction(actionShowNormals);
	menuView->addAction(actionViewCapture);
	menuView->addSeparator();
	menuView->addAction(actionShowGrid);
	menuView->addAction(actionShowMeshLines);
	menuView->addAction(actionShowEdgeLines);
	menuView->addAction(actionBackfaceCulling);
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
	if (m_updaterPresent)
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

	// Create the toolbar
	QToolBar* mainToolBar = m_wnd->addToolBar("Main toolbar");
	mainToolBar->setObjectName(QStringLiteral("mainToolBar"));

	mainToolBar->addAction(actionNewModel);
	mainToolBar->addAction(actionOpen);
	mainToolBar->addAction(actionSave);
	mainToolBar->addAction(actionSnapShot);

	// Build tool bar
	coord = new QComboBox;
	coord->setObjectName("selectCoord");
	coord->addItem("Global");
	coord->addItem("Local");

	buildToolBar = new QToolBar(mainWindow);
	buildToolBar->setObjectName(QStringLiteral("buildToolBar"));
	buildToolBar->setWindowTitle("Build Toolbar");

	buildToolBar->addAction(actionUndo);
	buildToolBar->addAction(actionRedo);
	buildToolBar->addSeparator();
	buildToolBar->addAction(selectRect);
	buildToolBar->addAction(selectCircle);
	buildToolBar->addAction(selectFree);
	buildToolBar->addSeparator();
	buildToolBar->addAction(actionMeasureTool);
	buildToolBar->addAction(actionPlaneCutTool);
	buildToolBar->addSeparator();
	buildToolBar->addAction(actionSelect);
	buildToolBar->addAction(actionTranslate);
	buildToolBar->addAction(actionRotate);
	buildToolBar->addAction(actionScale);
	buildToolBar->addWidget(coord);
	buildToolBar->addSeparator();
	buildToolBar->addAction(actionSelectObjects);
	buildToolBar->addAction(actionSelectParts);
	buildToolBar->addAction(actionSelectSurfaces);
	buildToolBar->addAction(actionSelectCurves);
	buildToolBar->addAction(actionSelectNodes);
	buildToolBar->addAction(actionSelectDiscrete);
	buildToolBar->addSeparator();
	buildToolBar->addAction(actionCurveEditor);
	buildToolBar->addAction(actionMeshInspector);
	buildToolBar->addAction(actionFEBioRun);
	buildToolBar->addSeparator();
	buildToolBar->addAction(actionMerge);
	buildToolBar->addAction(actionDetach);
	buildToolBar->addAction(actionExtract);
	buildToolBar->addAction(actionClone);
	buildToolBar->addAction(actionCloneGrid);
	buildToolBar->addAction(actionCloneRevolve);

	mainWindow->addToolBar(Qt::TopToolBarArea, buildToolBar);

	// Post tool bar
	postToolBar = new CPostToolBar(mainWindow);
	postToolBar->setObjectName(QStringLiteral("postToolBar"));
	postToolBar->setWindowTitle("Post Toolbar");
	mainWindow->addToolBar(Qt::TopToolBarArea, postToolBar);
	postToolBar->addAction(actionGraph);
	postToolBar->addAction(actionPlaneCut);
	postToolBar->addAction(actionMirrorPlane);
	postToolBar->addAction(actionVectorPlot);
	postToolBar->addAction(actionTensorPlot);
	postToolBar->addAction(actionIsosurfacePlot);
	postToolBar->addAction(actionSlicePlot);
	postToolBar->addAction(actionStreamLinePlot);
	postToolBar->addAction(actionParticleFlowPlot);
	postToolBar->addAction(actionVolumeFlowPlot);

	postToolBar->setDisabled(true);
	postToolBar->hide();

	// Image tool bar
	imageToolBar = new CImageToolBar(mainWindow);
	imageToolBar->setObjectName("imageToolBar");
	imageToolBar->setWindowTitle("Image Toolbar");
	mainWindow->addToolBar(Qt::TopToolBarArea, imageToolBar);
	imageToolBar->hide();

	// Font tool bar
	pFontToolBar = new QToolBar(mainWindow);
	pFontToolBar->setObjectName("FontToolBar");
	pFontToolBar->setWindowTitle("Font Toolbar");
	mainWindow->addToolBarBreak();
	mainWindow->addToolBar(Qt::TopToolBarArea, pFontToolBar);

	QAction* actionProperties = addAction("Properties ...", "actionProperties", "properties");

	pFontToolBar->addWidget(pFontStyle = new QFontComboBox); pFontStyle->setObjectName("fontStyle");
	pFontToolBar->addWidget(pFontSize = new QSpinBox); pFontSize->setObjectName("fontSize");
	pFontToolBar->addAction(actionFontBold = addAction("Bold", "fontBold", "font_bold")); actionFontBold->setCheckable(true);
	pFontToolBar->addAction(actionFontItalic = addAction("Italic", "fontItalic", "font_italic")); actionFontItalic->setCheckable(true);
	pFontToolBar->addAction(actionProperties);
	pFontToolBar->setEnabled(false);

	// XML toolbar
	xmlToolbar = new QToolBar(mainWindow);
	xmlToolbar->setObjectName("xmlToolbar");
	xmlToolbar->setWindowTitle("XML Toolbar");
	mainWindow->addToolBar(Qt::TopToolBarArea, xmlToolbar);

	actionEditXmlAsText = addAction("Edit as Text", "actionEditXmlAsText", "txt", true);

	xmlToolbar->addAction(actionEditXmlAsText);
	xmlToolbar->addSeparator();
	xmlToolbar->addAction(actionAddAttribute);
	xmlToolbar->addAction(actionAddElement);
	xmlToolbar->addAction(actionRemoveRow);
	xmlToolbar->addSeparator();
	xmlToolbar->addAction(actionUndo);
	xmlToolbar->addAction(actionRedo);
}

void Ui::CMainWindow::buildDockWidgets(::CMainWindow* wnd)
{
	wnd->setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);

	QDockWidget* dock1 = new QDockWidget("Project", m_wnd); dock1->setObjectName("dockFiles");
	dock1->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	fileViewer = new ::CFileViewer(m_wnd, dock1);
	fileViewer->setObjectName(QStringLiteral("fileViewer"));
	dock1->setWidget(fileViewer);
	m_wnd->addDockWidget(Qt::LeftDockWidgetArea, dock1);
	menuWindows->addAction(dock1->toggleViewAction());

	QDockWidget* dock2 = new QDockWidget("Model", m_wnd); dock2->setObjectName("dockModel");
	dock2->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	modelViewer = new ::CModelViewer(m_wnd, dock2);
	modelViewer->setObjectName("modelViewer");
	dock2->setWidget(modelViewer);
	menuWindows->addAction(dock2->toggleViewAction());
	m_wnd->tabifyDockWidget(dock1, dock2);

	QDockWidget* dock3 = new QDockWidget("Build", m_wnd); dock3->setObjectName("dockBuild");
	dock3->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	buildPanel = new ::CBuildPanel(m_wnd, dock3);
	dock3->setWidget(buildPanel);
	menuWindows->addAction(dock3->toggleViewAction());
	m_wnd->addDockWidget(Qt::RightDockWidgetArea, dock3);
	//		m_wnd->tabifyDockWidget(dock2, dock3);

	QDockWidget* dock4 = new QDockWidget("Output", m_wnd); dock4->setObjectName("dockLog");
	dock4->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
	logPanel = new ::CLogPanel(dock4);
	dock4->setWidget(logPanel);
	menuWindows->addAction(dock4->toggleViewAction());
	m_wnd->addDockWidget(Qt::BottomDockWidgetArea, dock4);

	QDockWidget* dock5 = new QDockWidget("Post", m_wnd); dock5->setObjectName("dockPost");
	dock5->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	postPanel = new ::CPostPanel(m_wnd, dock5);
	dock5->setWidget(postPanel);
	menuWindows->addAction(dock5->toggleViewAction());
	m_wnd->tabifyDockWidget(dock1, dock5);

	QDockWidget* dock6 = new QDockWidget("Notes", m_wnd); dock6->setObjectName("dockInfo");
	infoPanel = new ::CInfoPanel(wnd, dock6);
	dock6->setWidget(infoPanel);
	menuWindows->addAction(dock6->toggleViewAction());
	m_wnd->tabifyDockWidget(dock4, dock6);

#ifdef MODEL_REPO
	QDockWidget* dock7 = new QDockWidget("Repository", m_wnd); dock7->setObjectName("dockDatabase");
	databasePanel = new ::CRepositoryPanel(wnd, dock7);
	dock7->setWidget(databasePanel);
	menuWindows->addAction(dock7->toggleViewAction());
	m_wnd->tabifyDockWidget(dock1, dock7);
#endif

	QDockWidget* dock8 = new QDockWidget("Timeline", m_wnd); dock8->setObjectName("dockTime");
	timePanel = new ::CTimelinePanel(wnd, dock8);
	dock8->setWidget(timePanel);
	menuWindows->addAction(dock8->toggleViewAction());
	m_wnd->tabifyDockWidget(dock4, dock8);

	QDockWidget* dock9 = new QDockWidget("3D Image Settings", m_wnd); dock9->setObjectName("dockImageSettings");
	imageSettingsPanel = new ::CImageSettingsPanel(wnd, dock9);
	dock9->setWidget(imageSettingsPanel);
	menuWindows->addAction(dock9->toggleViewAction());
	m_wnd->tabifyDockWidget(dock4, dock9);

#ifdef HAS_PYTHON
		QDockWidget* dock10 = new QDockWidget("Python", m_wnd); dock10->setObjectName("dockPython");
		pythonToolsPanel = new ::CPythonToolsPanel(wnd, dock10);
		pythonToolsPanel->initPython();
		dock10->setWidget(pythonToolsPanel);
		menuWindows->addAction(dock10->toggleViewAction());
		m_wnd->tabifyDockWidget(dock3, dock10);
#endif

	// make sure the file viewer is the visible tab
	dock1->raise();
}

void Ui::CMainWindow::BuildConfigs()
{
	// Insert in same order as Ui::Config !
	m_configs.push_back(new CEmptyConfig(this));
	m_configs.push_back(new CHTMLConfig(this));
	m_configs.push_back(new CModelConfig(this));
	m_configs.push_back(new CPostConfig(this));
	m_configs.push_back(new CTextConfig(this));
	m_configs.push_back(new CXMLConfig(this));

	setUIConfig(Ui::Config::EMPTY_CONFIG);
}

void Ui::CMainWindow::SetSelectionMode(int nselect)
{
	switch (nselect)
	{
	case SELECT_OBJECT  : actionSelectObjects->trigger(); break;
	case SELECT_PART    : actionSelectParts->trigger(); break;
	case SELECT_FACE    : actionSelectSurfaces->trigger(); break;
	case SELECT_EDGE    : actionSelectCurves->trigger(); break;
	case SELECT_NODE    : actionSelectNodes->trigger(); break;
	case SELECT_DISCRETE: actionSelectDiscrete->trigger(); break;
	default:
		assert(false);
	}
}

void Ui::CMainWindow::showCurveEditor()
{
	if (curveWnd == 0) curveWnd = new CCurveEditor(m_wnd);

	curveWnd->Update();

	curveWnd->show();
	curveWnd->raise();
	curveWnd->activateWindow();
}

void Ui::CMainWindow::showMeshInspector()
{
	if (meshWnd == 0) meshWnd = new ::CMeshInspector(m_wnd);

	meshWnd->Update(true);

	meshWnd->show();
	meshWnd->raise();
	meshWnd->activateWindow();
}

void Ui::CMainWindow::updateMeshInspector()
{
	if (meshWnd && meshWnd->isVisible())
	{
		meshWnd->Update(true);
	}
}

void Ui::CMainWindow::setRecentFiles(QStringList& recentFiles)
{
	setRecentFileList(m_recentFiles, recentFiles, menuRecentFiles, recentFilesActionGroup);
}

void Ui::CMainWindow::setRecentProjects(QStringList& recentFiles)
{
	setRecentFileList(m_recentProjects, recentFiles, menuRecentProjects, recentProjectsActionGroup);
}

void Ui::CMainWindow::setRecentPlugins(QStringList& recentPlugins)
{
	setRecentFileList(m_recentPlugins, recentPlugins, nullptr, nullptr);
}

void Ui::CMainWindow::setRecentGeomFiles(QStringList& recentFiles)
{
	setRecentFileList(m_recentGeomFiles, recentFiles, menuRecentGeomFiles, recentGeomFilesActionGroup);
}

void Ui::CMainWindow::addToRecentFiles(const QString& file)
{
	addToRecentFilesList(m_recentFiles, file, menuRecentFiles, recentFilesActionGroup);
}

void Ui::CMainWindow::addToRecentProjects(const QString& file)
{
	addToRecentFilesList(m_recentProjects, file, menuRecentProjects, recentProjectsActionGroup);
}

void Ui::CMainWindow::addToRecentPlugins(const QString& file)
{
	addToRecentFilesList(m_recentPlugins, file);
}

void Ui::CMainWindow::addToRecentGeomFiles(const QString& file)
{
	addToRecentFilesList(m_recentGeomFiles, file, menuRecentGeomFiles, recentGeomFilesActionGroup);
}

void Ui::CMainWindow::showFileViewer()
{
	fileViewer->parentWidget()->raise();
}

void Ui::CMainWindow::showModelViewer()
{
	modelViewer->parentWidget()->raise();
}

void Ui::CMainWindow::showBuildPanel()
{
	buildPanel->parentWidget()->raise();
}

void Ui::CMainWindow::showPostPanel()
{
	postPanel->parentWidget()->raise();
}

void Ui::CMainWindow::showDatabasePanel()
{
	databasePanel->parentWidget()->raise();
}

void Ui::CMainWindow::showTimeline()
{
	timePanel->parentWidget()->raise();
}

void Ui::CMainWindow::stopAnimation()
{
	m_isAnimating = false;
	postToolBar->CheckPlayButton(false);
}

void Ui::CMainWindow::ShowDefaultBackground()
{
	centralWidget->htmlViewer->setDocument(nullptr);
	centralWidget->htmlViewer->setHtml(QString(" \
					<!DOCTYPE html> \
					<html> \
					<body style = \"background-color:#808080;\"> \
					</body> \
					</html>"\
	));
}

void Ui::CMainWindow::setUIConfig(Ui::Config newConfig)
{
	m_activeConfig = newConfig;
	assert(m_configs[newConfig]->GetConfig() == newConfig);
	m_configs[newConfig]->Apply();
}

void Ui::CMainWindow::setRecentFileList(QStringList& dstList, const QStringList& fileList, QMenu* menu, QActionGroup* actionGroup)
{
	dstList = fileList;

	int N = dstList.count();
	if (N > MAX_RECENT_FILES) N = MAX_RECENT_FILES;

	for (int i = 0; i < N; ++i)
	{
		QString file = dstList.at(i);

		if (menu)
		{
			QAction* pa = menu->addAction(file);
			if (actionGroup) actionGroup->addAction(pa);
		}
	}
}

void Ui::CMainWindow::addToRecentFilesList(QStringList& dstList, const QString& file, QMenu* menu, QActionGroup* actionGroup)
{
	QString fileName = file;

#ifdef WIN32
	// on windows, make sure that all filenames use backslashes
	fileName.replace('/', '\\');
#endif

	QList<QAction*> actionList = menu->actions();
	if (actionList.isEmpty())
	{
		dstList.append(fileName);
		QAction* action = menu->addAction(fileName);
		actionGroup->addAction(action);
	}
	else
	{
		// we need the first action so that we can insert before it
		QAction* firstAction = actionList.at(0);

		// see if the file already exists or not
		int n = dstList.indexOf(fileName);
		if (n >= 0)
		{
			// if the file exists, we move it to the top
			if (n != 0)
			{
				QAction* action = actionList.at(n);
				menu->removeAction(action);
				menu->insertAction(firstAction, action);

				dstList.removeAt(n);
				dstList.push_front(fileName);
			}
		}
		else
		{
			int N = dstList.count();
			if (N >= MAX_RECENT_FILES)
			{
				// remove the last one
				dstList.removeLast();
				menu->removeAction(actionList.last());
			}

			// add a new file item
			dstList.push_front(fileName);
			QAction* pa = new QAction(fileName);
			menu->insertAction(firstAction, pa);
			actionGroup->addAction(pa);
		}
	}
}

void Ui::CMainWindow::addToRecentFilesList(QStringList& dstList, const QString& file)
{
	QString fileName = file;

#ifdef WIN32
	// on windows, make sure that all filenames use backslashes
	fileName.replace('/', '\\');
#endif

	if (dstList.isEmpty())
	{
		dstList.append(fileName);
	}
	else
	{
		// see if the file already exists or not
		int n = dstList.indexOf(fileName);
		if (n >= 0)
		{
			// if the file exists, we move it to the top
			if (n != 0)
			{
				dstList.removeAt(n);
				dstList.push_front(fileName);
			}
		}
		else
		{
			int N = dstList.count();
			if (N >= MAX_RECENT_FILES)
			{
				// remove the last one
				dstList.removeLast();
			}

			// add a new file item
			dstList.push_front(fileName);
		}
	}
}

void Ui::CMainWindow::showPartSelector(CModelDocument* doc)
{
	if (partSelector == nullptr) partSelector = new CDlgPartSelector(m_wnd);
	partSelector->SetDocument(doc);
	partSelector->show();
}
