#include "GLView.h"
#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QDockWidget>
#include <QDesktopWidget>
#include <QStatusBar>
#include <QToolBar>
#include <QProgressBar>
#include <QComboBox>
#include <QBoxLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QtCore/QDir>
#include "FileViewer.h"
#include "ModelViewer.h"
#include "CurveEditor.h"
#include "MeshInspector.h"
#include "LogPanel.h"
#include "BuildPanel.h"
#include "GLControlBar.h"
#include "Document.h"
#include "DataFieldSelector.h"
#include "PostPanel.h"
#include "InfoPanel.h"
#include "DatabasePanel.h"
#include <QFontComboBox>
#include <vector>
#include "LaunchConfig.h"
#include <QtCore/QStandardPaths>
#include "MainTabBar.h"

class QProcess;

class Ui::CMainWindow
{
	enum
	{
		MAX_RECENT_FILES = 15		// max number of recent files
	};

public:
	CMainTabBar*	tab;
	CGLView*	glview;
	CGLControlBar* glc;
	::CMainWindow*	m_wnd;

	QMenu* menuFile;
	QMenu* menuEdit;
	QMenu* menuPhysics;
	QMenu* menuTools;
	QMenu* menuPost;
	QMenu* menuFEBio;
	QMenu* menuRecord;
	QMenu* menuView;
	QMenu* menuHelp;
	QMenu* menuRecentFiles;
	QMenu* menuRecentFEFiles;
	QMenu* menuRecentGeomFiles;
	QMenu* menuWindows;
	QMenu* menuViews;

	QActionGroup* recentFilesActionGroup;
	QActionGroup* recentFEFilesActionGroup;
	QActionGroup* recentGeomFilesActionGroup;

	::CFileViewer*	fileViewer;
	::CModelViewer* modelViewer;
	::CBuildPanel*  buildPanel;
	::CLogPanel*	logPanel;
	::CCurveEditor*	curveWnd;
	::CMeshInspector* meshWnd;
	::CPostPanel*	postPanel;
	::CInfoPanel*	infoPanel;
	::CDatabasePanel*	databasePanel;

	QToolBar*	mainToolBar;
	QStatusBar*	statusBar;
	QProgressBar*	fileProgress;
	QProgressBar*	indeterminateProgress;

	CDataFieldSelector*	selectData;
	QSpinBox*	pspin;
	QToolBar*	postToolBar;
	QToolBar*	buildToolBar;

	QToolBar* pFontToolBar;
	QFontComboBox*	pFontStyle;
	QSpinBox*		pFontSize;
	QAction*		actionFontBold;
	QAction*		actionFontItalic;

	QAction* actionSelectObjects;
	QAction* actionSelectParts;
	QAction* actionSelectSurfaces;
	QAction* actionSelectCurves;
	QAction* actionSelectNodes;
	QAction* actionSelectDiscrete;

	QAction* actionAddBC;
	QAction* actionAddNodalLoad;
	QAction* actionAddSurfLoad;
	QAction* actionAddBodyLoad;
	QAction* actionAddIC;
	QAction* actionAddContact;
	QAction* actionAddConstraint;
	QAction* actionAddRigidConstraint;
	QAction* actionAddRigidConnector;
	QAction* actionAddStep;
	QAction* actionAddMaterial;
	QAction* actionSoluteTable;
	QAction* actionSBMTable;
	QAction* actionAddReaction;

	QComboBox* coord;

	QString currentPath;

	QStringList	m_recentFiles;
	QStringList	m_recentFEFiles;
	QStringList	m_recentGeomFiles;

	QAction* actionUndoViewChange;
	QAction* actionRedoViewChange;
	QAction* actionZoomSelect;
	QAction* actionZoomExtents;
	QAction* actionViewCapture;
	QAction* actionShowGrid;
	QAction* actionShowMeshLines;
	QAction* actionShowEdgeLines;
	QAction* actionBackfaceCulling;
	QAction* actionViewSmooth;
	QAction* actionShowNormals;
	QAction* actionOrtho;
	QAction* actionFront;
	QAction* actionBack;
	QAction* actionRight;
	QAction* actionLeft;
	QAction* actionTop;
	QAction* actionBottom;
	QAction* actionOptions;
	QAction* actionWireframe;
	QAction* actionShowFibers;
	QAction* actionShowMatAxes;
	QAction* actionShowDiscrete;
	QAction* actionToggleLight;

	QAction* actionColorMap;
	QAction* actionPlay;

	QAction* selectRect;
	QAction* selectCircle;
	QAction* selectFree;

public:
	vector<CLaunchConfig>		m_launch_configs;

	QString		m_defaultProjectFolder;
	QString		m_repositoryFolder;

	QProcess*	m_process;
	bool		m_bkillProcess;

	int			m_theme;	// 0 = default, 1 = dark

	QString	m_old_title;

	bool	m_isAnimating;

	QList<::CGraphWindow*>	graphList;

public:
	CMainWindow()
	{
		m_theme = 0;
	}

	void setupUi(::CMainWindow* wnd)
	{
		m_wnd = wnd;

		m_launch_configs.push_back(CLaunchConfig());
		m_launch_configs.back().type = LOCAL;
		m_launch_configs.back().name = std::string("FEBio 3.0");

#ifdef WIN32
		m_launch_configs.back().path = std::string("$(FEBioStudioDir)/febio3.exe");
#else
		m_launch_configs.back().path = std::string("$(FEBioStudioDir)/febio3");
#endif

#ifdef WIN32
		m_defaultProjectFolder = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
#endif

		m_process = 0;
		m_bkillProcess = false;

		m_isAnimating = false;

		curveWnd = 0;
		meshWnd = 0;

		// initialize current path
		currentPath = QDir::currentPath();

		// set the initial window size
//        QRect screenSize = QDesktopWidget().availableGeometry(wnd);
        //wnd->resize(QSize(screenSize.width() * 1.0f, screenSize.height() * 1.0f));
//		wnd->resize(800, 600);

		tab = new CMainTabBar(wnd);
		tab->setObjectName("tab");
		tab->addView("Model");

		// create the central widget
		QWidget* w = new QWidget;

		// create the layout for the central widget
		QVBoxLayout* l = new QVBoxLayout;
		l->setMargin(0);

		// create the GL view
		glview = new CGLView(wnd); glview->setObjectName("glview");

		// create the GL control bar
		glc = new CGLControlBar(wnd);
		glc->setObjectName("glbar");
		glc->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Policy::Fixed);

		// add it all to the layout
		l->addWidget(tab);
		l->addWidget(glview);
		l->addWidget(glc);
		w->setLayout(l);

		// set the central widget
		wnd->setCentralWidget(w);

		// build the menu
		buildMenu(wnd);

		// build the dockable windows
		// (must be done after menu is created)
		buildDockWidgets(wnd);

		// build status bar
		statusBar = new QStatusBar(m_wnd);
		m_wnd->setStatusBar(statusBar);

		fileProgress = new QProgressBar;
		fileProgress->setRange(0, 100);
		fileProgress->setMaximumWidth(200);
		fileProgress->setMaximumHeight(15);

		indeterminateProgress = new QProgressBar;
		indeterminateProgress->setRange(0, 0);
		indeterminateProgress->setMaximumWidth(200);
		indeterminateProgress->setMaximumHeight(15);

		QMetaObject::connectSlotsByName(wnd);
	}

	QAction* addAction(const QString& title, const QString& name, const QString& iconFile = QString(), bool bcheckable = false)
	{
		QAction* pa = new QAction(title, m_wnd);
		pa->setObjectName(name);
		if (iconFile.isEmpty() == false) pa->setIcon( m_wnd->GetResourceIcon(iconFile));
		if (bcheckable) pa->setCheckable(true);
		return pa;
	}

	// create actions and menu
	void buildMenu(::CMainWindow* mainWindow)
	{
		// --- File menu ---
		QAction* actionNew        = addAction("New ..."    , "actionNew"   , "new" ); actionNew ->setShortcuts(QKeySequence::New );
		QAction* actionOpen       = addAction("Open ..."   , "actionOpen"  , "open"); actionOpen->setShortcuts(QKeySequence::Open);
		QAction* actionSave       = addAction("Save"       , "actionSave"  , "save"); actionSave->setShortcuts(QKeySequence::Save);
		QAction* actionSaveAs     = addAction("Save as ...", "actionSaveAs"); actionSaveAs->setShortcuts(QKeySequence::SaveAs);
		QAction* actionSnapShot   = addAction("Snapshot ...", "actionSnapShot", "snapshot");
		QAction* actionInfo       = addAction("Model info ...", "actionInfo");
		QAction* actionImportFE   = addAction("Import FE model ..." , "actionImportFEModel");
		QAction* actionExportFE   = addAction("Export FE model ..." , "actionExportFEModel");
		QAction* actionImportGeom = addAction("Import geometry ...", "actionImportGeometry");
		QAction* actionExportGeom = addAction("Export geometry ...", "actionExportGeometry");
		QAction* actionImportProject = addAction("Import project ...", "actionImportProject");
		QAction* actionExportProject = addAction("Export project ...", "actionExportProject");
		QAction* actionImportImg  = addAction("Import image ...", "actionImportImage");
		QAction* actionConvertFeb    = addAction("FEBio files ...", "actionConvertFeb");
		QAction* actionConvertGeo = addAction("Geometry files ...", "actionConvertGeo");
		QAction* actionExit       = addAction("Exit"       , "actionExit"  );

		// --- Edit menu ---
		QAction* actionUndo              = addAction("Undo", "actionUndo", "undo"); actionUndo->setShortcuts(QKeySequence::Undo);
		QAction* actionRedo              = addAction("Redo", "actionRedo", "redo"); actionRedo->setShortcuts(QKeySequence::Redo);
		QAction* actionInvertSelection   = addAction("Invert selection"  , "actionInvertSelection"  ); actionInvertSelection->setShortcut(Qt::AltModifier + Qt::Key_I);
		QAction* actionClearSelection    = addAction("Clear selection"   , "actionClearSelection"   );
		QAction* actionDeleteSelection   = addAction("Delete selection"  , "actionDeleteSelection"  ); actionDeleteSelection->setShortcuts(QKeySequence::Delete);
		QAction* actionNameSelection     = addAction("Name selection ...", "actionNameSelection"    ); actionNameSelection->setShortcut(Qt::ControlModifier + Qt::Key_G);
		QAction* actionHideSelection     = addAction("Hide selection"    , "actionHideSelection"    ); actionHideSelection->setShortcut(Qt::Key_H);
		QAction* actionHideUnselected    = addAction("Hide Unselected"   , "actionHideUnselected"   ); actionHideUnselected->setShortcut(Qt::ShiftModifier + Qt::Key_H);
		QAction* actionUnhideAll         = addAction("Unhide all"        , "actionUnhideAll"        );
		QAction* actionFind              = addAction("Find ..."          , "actionFind"             ); actionFind->setShortcut(Qt::ControlModifier + Qt::Key_F);
		QAction* actionSelectRange       = addAction("Select in range ...", "actionSelectRange"     );
		QAction* actionToggleVisible     = addAction("Toggle visibility" , "actionToggleVisible"    , "toggle_visible");
		QAction* actionTransform         = addAction("Transform ..."     , "actionTransform"        ); actionTransform->setShortcut(Qt::ControlModifier + Qt::Key_T);
		QAction* actionCollapseTransform = addAction("Collapse transform", "actionCollapseTransform");
		QAction* actionClone             = addAction("Clone object ...", "actionClone"            , "clone"); actionClone->setShortcut(Qt::ControlModifier + Qt::Key_D);
		QAction* actionCloneGrid         = addAction("Clone grid ..."    , "actionCloneGrid"        , "clonegrid");
		QAction* actionCloneRevolve      = addAction("Clone revolve ..." , "actionCloneRevolve"     , "clonerevolve");
		QAction* actionMerge             = addAction("Merge objects ..." , "actionMerge"            , "merge");
		QAction* actionDetach            = addAction("Detach Elements"   , "actionDetach"           , "detach");
		QAction* actionExtract           = addAction("Extract Faces"     , "actionExtract"          , "extract");
		QAction* actionPurge             = addAction("Purge ..."         , "actionPurge"            );
		QAction* actionEditProject       = addAction("Edit Project Settings ...", "actionEditProject");

		QAction* actionFace2Elems        = addAction("Face to Element selection", "actionFaceToElem");

		// --- Physics menu ---
		actionAddBC              = addAction("Add Boundary Condition ..."    , "actionAddBC"       ); actionAddBC->setShortcut(Qt::ControlModifier + Qt::Key_B);
		actionAddNodalLoad       = addAction("Add Nodal Load ..."            , "actionAddNodalLoad"); 
		actionAddSurfLoad        = addAction("Add Surface Load ..."          , "actionAddSurfLoad"); actionAddSurfLoad->setShortcut(Qt::ControlModifier + Qt::Key_L);
		actionAddBodyLoad        = addAction("Add Body Load ..."             , "actionAddBodyLoad");
		actionAddIC              = addAction("Add Initial Condition ..."     , "actionAddIC"); actionAddIC->setShortcut(Qt::ControlModifier + Qt::Key_I);
		actionAddContact         = addAction("Add Contact ..."               , "actionAddContact");
		actionAddConstraint      = addAction("Add Constraint..."             , "actionAddConstraint");
		actionAddRigidConstraint = addAction("Add Rigid Constraint ..."      , "actionAddRigidConstraint");
		actionAddRigidConnector  = addAction("Add Rigid Connector ..."       , "actionAddRigidConnector");
		actionAddStep            = addAction("Add Analysis Step ..."         , "actionAddStep");
		actionAddMaterial        = addAction("Add Material ..."              , "actionAddMaterial", "material"); actionAddMaterial->setShortcut(Qt::ControlModifier + Qt::Key_M);
		actionSoluteTable        = addAction("Solute Table ..."              , "actionSoluteTable");
		actionSBMTable           = addAction("Solid-bound Molecule Table ...", "actionSBMTable");
		actionAddReaction        = addAction("Chemical Reaction Editor ..."  , "actionAddReaction");

		// --- Tools menu ---
		QAction* actionCurveEditor = addAction("Curve Editor ...", "actionCurveEditor", "curves"); actionCurveEditor->setShortcut(Qt::Key_F9);
		QAction* actionMeshInspector = addAction("Mesh Inspector ...", "actionMeshInspector", "inspect");
		QAction* actionElasticityConvertor = addAction("Elasticity Converter ...", "actionElasticityConvertor");
		QAction* actionFEBioRun  = addAction("Run FEBio ...", "actionFEBioRun", "febiorun"); actionFEBioRun->setShortcut(Qt::Key_F5);
		QAction* actionFEBioStop = addAction("Stop FEBio", "actionFEBioStop");
		QAction* actionFEBioOptimize = addAction("Generate optimization file ...", "actionFEBioOptimize");
		actionOptions = addAction("Options ...", "actionOptions"); actionOptions->setShortcut(Qt::Key_F12);

#ifdef _DEBUG
		QAction* actionLayerInfo = addAction("Print Layer Info", "actionLayerInfo"); actionLayerInfo->setShortcut(Qt::Key_F11);
#endif

		// --- Post menu ---
		QAction* actionPlaneCut = addAction("Plane cut", "actionPlaneCut", "cut");
		QAction* actionMirrorPlane = addAction("Mirror plane", "actionMirrorPlane", "mirror");
		QAction* actionVectorPlot = addAction("Vector plot", "actionVectorPlot", "vectors");
		QAction* actionTensorPlot = addAction("Tensor plot", "actionTensorPlot", "tensor");
		QAction* actionIsosurfacePlot = addAction("Isosurface plot", "actionIsosurfacePlot", "isosurface");
		QAction* actionSlicePlot = addAction("Slice plot", "actionSlicePlot", "sliceplot");
		QAction* actionDisplacementMap = addAction("Displacement map", "actionDisplacementMap", "distort");
		QAction* actionStreamLinePlot = addAction("Stream lines plot", "actionStreamLinePlot", "streamlines");
		QAction* actionParticleFlowPlot = addAction("Particle flow plot", "actionParticleFlowPlot", "particle");
		QAction* actionVolumeFlowPlot = addAction("Volume flow plot", "actionVolumeFlowPlot", "flow");
		QAction* actionImageSlicer = addAction("Image slicer", "actionImageSlicer", "imageslice");
		QAction* actionVolumeRender = addAction("Volume render", "actionVolumeRender", "volrender");
		QAction* actionMarchingCubes = addAction("Image isosurface", "actionMarchingCubes", "marching_cubes");
		QAction* actionGraph = addAction("New Graph ...", "actionGraph", "chart"); actionGraph->setShortcut(Qt::Key_F3);
		QAction* actionSummary = addAction("Summary ...", "actionSummary"); actionSummary->setShortcut(Qt::Key_F4);
		QAction* actionStats = addAction("Statistics  ...", "actionStats");
		QAction* actionIntegrate = addAction("Integrate ...", "actionIntegrate", "integrate");

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
		actionUndoViewChange  = addAction("Undo View Change", "actionUndoViewChange"); actionUndoViewChange->setShortcut(Qt::ControlModifier + Qt::Key_U);
		actionRedoViewChange  = addAction("Redo View Change", "actionRedoViewChange"); actionRedoViewChange->setShortcut(Qt::ControlModifier + Qt::Key_R);
		actionZoomSelect      = addAction("Zoom to selection", "actionZoomSelect"); actionZoomSelect->setShortcut(Qt::Key_F);
		actionZoomExtents     = addAction("Zoom to selection", "actionZoomExtents");
		actionViewCapture     = addAction("Show capture Frame", "actionViewCapture"); actionViewCapture->setCheckable(true); actionViewCapture->setShortcut(Qt::Key_0);
		actionShowGrid        = addAction("Show Grid", "actionShowGrid"); actionShowGrid->setCheckable(true); actionShowGrid->setChecked(true); actionShowGrid->setShortcut(Qt::Key_G);
		actionShowMeshLines   = addAction("Show Mesh Lines", "actionShowMeshLines", "show_mesh"); actionShowMeshLines->setCheckable(true); actionShowMeshLines->setShortcut(Qt::Key_M);
		actionShowEdgeLines   = addAction("Show Edge Lines", "actionShowEdgeLines"); actionShowEdgeLines->setCheckable(true); actionShowEdgeLines->setShortcut(Qt::Key_Z);
		actionBackfaceCulling = addAction("Backface culling", "actionBackfaceCulling"); actionBackfaceCulling->setCheckable(true);
		actionViewSmooth      = addAction("Color smoothing", "actionViewSmooth"); actionViewSmooth->setShortcut(Qt::Key_C); actionViewSmooth->setCheckable(true);
		actionOrtho           = addAction("Orthographic Projection", "actionOrtho"); actionOrtho->setCheckable(true); actionOrtho->setShortcut(Qt::Key_P);
		actionShowNormals     = addAction("Show Normals", "actionShowNormals"); actionShowNormals->setCheckable(true); actionShowNormals->setShortcut(Qt::Key_N);
		actionWireframe		  = addAction("Toggle wireframe", "actionWireframe"); actionWireframe->setCheckable(true); actionWireframe->setShortcut(Qt::Key_W);
		actionShowFibers      = addAction("Toggle Fibers", "actionShowFibers"); actionShowFibers->setCheckable(true); 
		actionShowMatAxes     = addAction("Toggle material axes", "actionShowMatAxes"); actionShowMatAxes->setCheckable(true);
		actionShowDiscrete    = addAction("Show Discrete sets", "actionShowDiscrete"); actionShowDiscrete->setCheckable(true);  actionShowDiscrete->setChecked(true);
		QAction* actionSnap3D = addAction("3D cursor to selection", "actionSnap3D"); actionSnap3D->setShortcut(Qt::Key_X);
		QAction* actionTrack  = addAction("Track Selection", "actionTrack"); actionTrack->setCheckable(true); actionTrack->setShortcut(Qt::Key_Y);
		actionToggleLight     = addAction("Toggle Lighting", "actionToggleLight");
		actionFront           = addAction("Front", "actionFront");
		actionBack            = addAction("Back" , "actionBack");
		actionRight           = addAction("Right", "actionRight");
		actionLeft            = addAction("Left" , "actionLeft");
		actionTop             = addAction("Top"  , "actionTop");
		actionBottom          = addAction("Bottom", "actionBottom");
		QAction* actionViewVPSave = addAction("Save viewpoint", "actionViewVPSave"); actionViewVPSave->setShortcut(Qt::CTRL + Qt::Key_K);
		QAction* actionViewVPPrev = addAction("Prev viewpoint", "actionViewVPPrev"); actionViewVPPrev->setShortcut(Qt::Key_J);
		QAction* actionViewVPNext = addAction("Next viewpoint", "actionViewVPNext"); actionViewVPNext->setShortcut(Qt::Key_L);
		QAction* actionSyncViews  = addAction("Sync all views", "actionSyncViews");

		// --- Help menu ---
		QAction* actionFEBioURL = addAction("FEBio Website", "actionFEBioURL");
		QAction* actionOnlineHelp = addAction("Online Manuals", "actionOnlineHelp");
		QAction* actionFEBioForum = addAction("FEBio Forums", "actionFEBioForum");
		QAction* actionFEBioResources = addAction("Learning Resources", "actionFEBioResources");
		QAction* actionFEBioPubs = addAction("FEBio Publications", "actionFEBioPubs");
		QAction* actionAbout = addAction("About FEBio Studio", "actionAbout");
	
		// other actions
		actionSelectObjects  = addAction("Select Objects" , "actionSelectObjects" , "selectObject" , true);
		actionSelectParts    = addAction("Select Parts"   , "actionSelectParts"   , "selectPart"   , true);
		actionSelectSurfaces = addAction("Select Surfaces", "actionSelectSurfaces", "selectSurface", true);
		actionSelectCurves   = addAction("Select Curves"  , "actionSelectCurves"  , "selectCurves", true );
		actionSelectNodes    = addAction("Select Nodes"   , "actionSelectNodes"   , "selectNodes", true  );
		actionSelectDiscrete = addAction("Select Discrete", "actionSelectDiscrete", "discrete", true);

		QAction* actionSelect    = addAction("Select"   , "actionSelect"   , "select"   , true); actionSelect->setShortcut(Qt::Key_Q);
		QAction* actionTranslate = addAction("Translate", "actionTranslate", "translate", true); actionTranslate->setShortcut(Qt::Key_T);
		QAction* actionRotate    = addAction("Rotate"   , "actionRotate"   , "rotate"   , true); actionRotate->setShortcut(Qt::Key_R);
		QAction* actionScale     = addAction("Scale"    , "actionScale"    , "scale"    , true); actionScale->setShortcut(Qt::Key_S);

		selectRect   = addAction("Rectangle", "selectRect"  , "selectRect"  , true);
		selectCircle = addAction("Circle"   , "selectCircle", "selectCircle", true);
		selectFree   = addAction("Freehand" , "selectFree"  , "selectFree"  , true);

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
		menuFile   = new QMenu("File", menuBar);
		menuEdit   = new QMenu("Edit", menuBar);
		menuPhysics= new QMenu("Physics", menuBar);
		menuFEBio  = new QMenu("FEBio", menuBar);
		menuPost   = new QMenu("Post", menuBar);
		menuRecord = new QMenu("Record", menuBar);
		menuTools  = new QMenu("Tools", menuBar);
		menuView   = new QMenu("View", menuBar);
		menuHelp   = new QMenu("Help", menuBar);

		menuRecentFiles = new QMenu("Recent Files");
		menuRecentFEFiles = new QMenu("Recent FE model Files");
		menuRecentGeomFiles = new QMenu("Recent geometry Files");

		recentFilesActionGroup = new QActionGroup(mainWindow);
		recentFilesActionGroup->setObjectName("recentFiles");

		recentFEFilesActionGroup = new QActionGroup(mainWindow);
		recentFEFilesActionGroup->setObjectName("recentFEFiles");

		recentGeomFilesActionGroup = new QActionGroup(mainWindow);
		recentGeomFilesActionGroup->setObjectName("recentGeomFiles");

		// File menu
		menuBar->addAction(menuFile->menuAction());
		menuFile->addAction(actionNew);
		menuFile->addAction(actionOpen);
		menuFile->addAction(actionSave);
		menuFile->addAction(actionSaveAs);
		menuFile->addAction(menuRecentFiles->menuAction());
		menuFile->addAction(actionInfo);
#ifdef HAS_QUAZIP
		menuFile->addSeparator();
		menuFile->addAction(actionImportProject);
		menuFile->addAction(actionExportProject);
#endif
		menuFile->addSeparator();
		menuFile->addAction(actionImportFE);
		menuFile->addAction(actionExportFE);
		menuFile->addAction(menuRecentFEFiles->menuAction());
		menuFile->addSeparator();
		menuFile->addAction(actionImportGeom);
		menuFile->addAction(actionExportGeom);
		menuFile->addAction(menuRecentGeomFiles->menuAction());
		menuFile->addSeparator();
		menuFile->addAction(actionImportImg);

		QMenu* ConvertMenu = new QMenu("Batch convert");
		ConvertMenu->addAction(actionConvertFeb);
		ConvertMenu->addAction(actionConvertGeo);

		menuFile->addAction(ConvertMenu->menuAction());
		menuFile->addSeparator();
		menuFile->addAction(actionExit);

		QMenu* moreSelection = new QMenu("More selection options");
		moreSelection->addAction(actionFace2Elems);


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
		menuEdit->addAction(actionSelectRange);
		menuEdit->addSeparator();
		menuEdit->addAction(actionTransform);
		menuEdit->addAction(actionCollapseTransform);
		menuEdit->addSeparator();
		menuEdit->addAction(actionClone);
		menuEdit->addAction(actionMerge);
		menuEdit->addSeparator();
		menuEdit->addAction(actionPurge);
		menuEdit->addSeparator();
		menuEdit->addAction(actionEditProject);

		// Physics menu
		menuBar->addAction(menuPhysics->menuAction());
		menuPhysics->addAction(actionAddBC);
		menuPhysics->addAction(actionAddNodalLoad);
		menuPhysics->addAction(actionAddSurfLoad);
		menuPhysics->addAction(actionAddBodyLoad);
		menuPhysics->addAction(actionAddIC);
		menuPhysics->addAction(actionAddContact);
		menuPhysics->addAction(actionAddConstraint);
		menuPhysics->addAction(actionAddRigidConstraint);
		menuPhysics->addAction(actionAddRigidConnector);
		menuPhysics->addAction(actionAddMaterial);
		menuPhysics->addAction(actionAddStep);
		menuPhysics->addSeparator();
		menuPhysics->addAction(actionSoluteTable);
		menuPhysics->addAction(actionSBMTable);
		menuPhysics->addAction(actionAddReaction);

		// FEBio menu
		menuBar->addAction(menuFEBio->menuAction());
		menuFEBio->addAction(actionFEBioRun);
		menuFEBio->addAction(actionFEBioStop);
		menuFEBio->addAction(actionFEBioOptimize);

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
		menuPost->addSeparator();
		menuPost->addAction(actionImageSlicer);
		menuPost->addAction(actionVolumeRender);
		menuPost->addAction(actionMarchingCubes);
		menuPost->addSeparator();
		menuPost->addAction(actionGraph);
		menuPost->addSeparator();
		menuPost->addAction(actionSummary);
		menuPost->addAction(actionStats);
		menuPost->addAction(actionIntegrate);

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
		menuTools->addAction(actionElasticityConvertor);
		menuTools->addAction(actionOptions);
#ifdef _DEBUG
		menuTools->addAction(actionLayerInfo);
#endif
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
		menuView->addAction(actionWireframe);
		menuView->addAction(actionShowFibers);
		menuView->addAction(actionShowMatAxes);
		menuView->addAction(actionShowDiscrete);
		menuView->addAction(actionSnap3D);
		menuView->addAction(actionTrack);
		menuView->addAction(actionToggleLight);
		menuView->addSeparator();

		menuViews = menuView->addMenu("Standard views");
		menuViews->addAction(actionFront);
		menuViews->addAction(actionBack);
		menuViews->addAction(actionRight);
		menuViews->addAction(actionLeft);
		menuViews->addAction(actionTop);
		menuViews->addAction(actionBottom);

		menuViews->addSeparator();
		menuView->addAction(actionViewVPSave);
		menuView->addAction(actionViewVPPrev);
		menuView->addAction(actionViewVPNext);
		menuView->addAction(actionSyncViews);
		menuView->addSeparator();

		menuWindows = menuView->addMenu("Windows");

		// Help menu
		menuBar->addAction(menuHelp->menuAction());
		menuHelp->addAction(actionFEBioURL);
		menuHelp->addAction(actionOnlineHelp);
		menuHelp->addAction(actionFEBioForum);
		menuHelp->addAction(actionFEBioResources);
		menuHelp->addAction(actionFEBioPubs);
		menuHelp->addSeparator();
		menuHelp->addAction(actionAbout);

		// Create the toolbar
		QToolBar* mainToolBar = m_wnd->addToolBar("Main toolbar");
		mainToolBar->setObjectName(QStringLiteral("mainToolBar"));

		coord = new QComboBox;
		coord->setObjectName("selectCoord");
		coord->addItem("Global");
		coord->addItem("Local");

		mainToolBar->addAction(actionNew );
		mainToolBar->addAction(actionOpen);
		mainToolBar->addAction(actionSave);
		mainToolBar->addAction(actionSnapShot);
		mainToolBar->addSeparator();
		mainToolBar->addAction(actionUndo);
		mainToolBar->addAction(actionRedo);
		mainToolBar->addSeparator();
		mainToolBar->addAction(selectRect);
		mainToolBar->addAction(selectCircle);
		mainToolBar->addAction(selectFree);

		// Build tool bar
		buildToolBar = new QToolBar(mainWindow);
		buildToolBar->setObjectName(QStringLiteral("buildToolBar"));
		buildToolBar->setWindowTitle("Build Toolbar");

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
		postToolBar = new QToolBar(mainWindow);
		postToolBar->setObjectName(QStringLiteral("postToolBar"));
		postToolBar->setWindowTitle("Post Toolbar");
		mainWindow->addToolBar(Qt::TopToolBarArea, postToolBar);

		QAction* actionRefresh = addAction("Reload", "actionRefresh", "refresh");
		QAction* actionFirst = addAction("first", "actionFirst", "back");
		QAction* actionPrev = addAction("previous", "actionPrev", "prev");
		actionPlay = addAction("Play", "actionPlay", "play"); actionPlay->setShortcut(Qt::Key_Space);
		actionPlay->setCheckable(true);
		QAction* actionNext = addAction("next", "actionNext", "next");
		QAction* actionLast = addAction("last", "actionLast", "forward");
		QAction* actionTime = addAction("Time settings", "actionTimeSettings", "clock");

		selectData = new CDataFieldSelector;
		selectData->setWhatsThis("<font color=\"black\">Use this to select the current data variable that will be used to display the color map on the mesh.");
		selectData->setMinimumWidth(300);
//		selectData->setFixedHeight(23);
		selectData->setObjectName("selectData");
		
		actionRefresh->setWhatsThis("<font color=\"black\">Click this to reload the plot file.");
		actionFirst->setWhatsThis("<font color=\"black\">Click this to go to the first time step in the model.");
		actionPrev->setWhatsThis("<font color=\"black\">Click this to go to the previous time step in the model.");
		actionPlay->setWhatsThis("<font color=\"black\">Click this to toggle the animation on or off");
		actionNext->setWhatsThis("<font color=\"black\">Click this to go to the next time step");
		actionLast->setWhatsThis("<font color=\"black\">Click this to go to the last time step in the model.");
		actionTime->setWhatsThis("<font color=\"black\">Click this to open the Time Info dialog box.");

		actionColorMap = addAction("Toggle colormap", "actionColorMap", "colormap");
		actionColorMap->setCheckable(true);
		actionColorMap->setWhatsThis("<font color=\"black\">Click this to turn on the color map on the model.");

		postToolBar->addAction(actionRefresh);
		postToolBar->addSeparator();
		postToolBar->addAction(actionFirst);
		postToolBar->addAction(actionPrev);
		postToolBar->addAction(actionPlay);
		postToolBar->addAction(actionNext);
		postToolBar->addAction(actionLast);
		postToolBar->addWidget(pspin = new QSpinBox);
		pspin->setObjectName("selectTime");
		pspin->setMinimumWidth(80);
		pspin->setSuffix("/100");
		postToolBar->addAction(actionTime);
		postToolBar->addWidget(selectData);
		postToolBar->addAction(actionColorMap);
		postToolBar->addSeparator();
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
	}

	void SetSelectionMode(int nselect)
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

	// build the dockable windows
	// Note that this must be called after the menu is created.
	void buildDockWidgets(::CMainWindow* wnd)
	{
		wnd->setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);

		QDockWidget* dock1 = new QDockWidget("Files", m_wnd); dock1->setObjectName("dockFiles");
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
		dock4->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
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
		QDockWidget* dock7 = new QDockWidget("Model DB", m_wnd); dock7->setObjectName("dockDatabase");
		databasePanel = new ::CDatabasePanel(wnd, dock7);
		dock7->setWidget(databasePanel);
		menuWindows->addAction(dock7->toggleViewAction());
		m_wnd->tabifyDockWidget(dock1, dock7);
#endif
		// make sure the file viewer is the visible tab
		dock1->raise();
	}

	void showCurveEditor()
	{
		if (curveWnd == 0) curveWnd = new CCurveEditor(m_wnd);

		curveWnd->Update();

		curveWnd->show();
		curveWnd->raise();
		curveWnd->activateWindow();
	}

	void showMeshInspector()
	{
		if (meshWnd == 0) meshWnd = new ::CMeshInspector(m_wnd);

		meshWnd->Update();

		meshWnd->show();
		meshWnd->raise();
		meshWnd->activateWindow();
	}

	void updateMeshInspector()
	{
		if (meshWnd && meshWnd->isVisible())
		{
			meshWnd->Update();
		}
	}

	void setRecentFiles(QStringList& recentFiles)
	{
		setRecentFileList(m_recentFiles, recentFiles, menuRecentFiles, recentFilesActionGroup);
	}

	void setRecentFEFiles(QStringList& recentFiles)
	{
		setRecentFileList(m_recentFEFiles, recentFiles, menuRecentFEFiles, recentFEFilesActionGroup);
	}

	void setRecentGeomFiles(QStringList& recentFiles)
	{
		setRecentFileList(m_recentGeomFiles, recentFiles, menuRecentGeomFiles, recentGeomFilesActionGroup);
	}

	void addToRecentFiles(const QString& file)
	{
		addToRecentFilesList(m_recentFiles, file, menuRecentFiles, recentFilesActionGroup);
	}

	void addToRecentFEFiles(const QString& file)
	{
		addToRecentFilesList(m_recentFEFiles, file, menuRecentFEFiles, recentFEFilesActionGroup);
	}

	void addToRecentGeomFiles(const QString& file)
	{
		addToRecentFilesList(m_recentGeomFiles, file, menuRecentGeomFiles, recentGeomFilesActionGroup);
	}

	void showFileViewer()
	{
		fileViewer->parentWidget()->raise();
	}

	void showModelViewer()
	{
		modelViewer->parentWidget()->raise();
	}

	void showBuildPanel()
	{
		buildPanel->parentWidget()->raise();
	}

	void showPostPanel()
	{
		postPanel->parentWidget()->raise();
	}

	void showDatabasePanel()
	{
		databasePanel->parentWidget()->raise();
	}

	void stopAnimation()
	{
		m_isAnimating = false;
		actionPlay->setChecked(false);
	}

private:
	void setRecentFileList(QStringList& dstList, const QStringList& fileList, QMenu* menu, QActionGroup* actionGroup)
	{
		dstList = fileList;

		int N = dstList.count();
		if (N > MAX_RECENT_FILES) N = MAX_RECENT_FILES;

		for (int i = 0; i < N; ++i)
		{
			QString file = dstList.at(i);

			QAction* pa = menu->addAction(file);

			actionGroup->addAction(pa);
		}
	}

	void addToRecentFilesList(QStringList& dstList, const QString& file, QMenu* menu, QActionGroup* actionGroup)
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
};
