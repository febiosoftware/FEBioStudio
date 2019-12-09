#pragma once
#include <QMainWindow>
#include <QCloseEvent>
#include <MathLib/math3d.h>
#include <QtCore/QProcess>
#include "GLView.h"

class FSObject;
class CDocument;
class CFileThread;
class FileReader;
class GMaterial;
class CCreatePanel;
class CBuildPanel;
class QMenu;
class CGraphWindow;
class CPostDoc;
class CFEBioJob;
class CSSHHandler;

namespace Ui {
	class CMainWindow;
}

namespace Post {
	class CGLModel;
}

class CMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit CMainWindow(bool reset = false, QWidget* parent = 0);
	~CMainWindow();

public:
	//! reset window data
	void Reset();

	//! Get the main document
	CDocument* GetDocument() { return m_doc; }

	//! Open a document
	void OpenDocument(const QString& fileName);

	//! Open an FE model
	void OpenFEModel(const QString& fileName);

	//! import a list of files
	void ImportFiles(const QStringList& fileNames);

	// open the object in the curve editor
	void OpenInCurveEditor(FSObject* po);

	// open the object in the model viewer
	void ShowInModelViewer(FSObject* po);

	// add to the log 
	void AddLogEntry(const QString& txt);

	// add to the output window
	void AddOutputEntry(const QString& txt);

	// clear the log
	void ClearLog();

	// clear the output window
	void ClearOutput();

	// get the build panel
	CBuildPanel* GetBuildPanel();

	// get the create panel
	CCreatePanel* GetCreatePanel();

	// sets the current folder
	void SetCurrentFolder(const QString& folder);

	// see if user wants to save doc
	bool maybeSave();

	// write some useful info to the log regarding the selection
	void ReportSelection();

	// get the current theme
	int currentTheme() const;

	// set the current theme
	void setCurrentTheme(int n);

	//! Open a plot file
	void OpenPlotFile(const QString& fileName);
    
    //! Process drag event
    void dragEnterEvent(QDragEnterEvent *e);
    
    //! Process drop event
    void dropEvent(QDropEvent *e);

	// --- WINDOW UPDATE ---

	//! Update the window title.
	void UpdateTitle();

	//! Update the window content
	void Update(QWidget* psend = 0, bool breset = false);

	//! Update the GL control bar
	void UpdateGLControlBar();

	//! Update the post tool bar
	void UpdatePostToolbar();

	//! set the post doc that will be rendered in the GL view
	void SetActivePostDoc(CPostDoc* postDoc);

	//! Update the post panel
	void UpdatePostPanel();

	//! Update the model editor
	void UpdateModel(FSObject* po = 0, bool bupdate = true);

	//! Update the entire UI
	void UpdateUI();

	//! Update the toolbar
	void UpdateToolbar();

	//! set the selection mode
	void SetSelectionMode(int nselect);

	// ----------------------

	//! redraw the GLView
	void RedrawGL();

	//! Zoom in on a box
	void ZoomTo(const BOX& box);

	// set a message on the status bar
	void SetStatusMessage(const QString& message);

	// clear the status message
	void ClearStatusMessage();

	// build the context menu for the GLView
	void BuildContextMenu(QMenu& menu);

	// Update the physics menu based on active modules
	void UpdatePhysicsUi();

	// Generate a map
	void GenerateMap(FSObject* po);

public:
	void ClearRecentProjectsList();

public:
	void AddBC          (int nstep);
	void AddBoundaryLoad(int nstep);
	void AddBodyLoad    (int nstep);
	void AddContact     (int nstep);
	void AddConstraint  (int nstep);
    void AddConnector   (int nstep);


	//! helper function for the create menu handlers
	void OnCreateObject(const char* sz);

	//! helper function for the sketch menu handlers
	void OnSketchObject(int n);

	//! helper function for the modify menu handlers
	void OnAddModifier(const char* sz);

public:
	// remove a graph from the list
	void RemoveGraph(CGraphWindow* graph);

	// Add a graph to the list of managed graph windows
	void AddGraph(CGraphWindow* graph);

public:
	QIcon GetResourceIcon(const QString& iconName);

private:
	void writeSettings();
	void readSettings();
	void readThemeSetting();
    
public:
	FileReader* CreateFileReader(const QString& fileName);
	void ReadFile(const QString& fileName, FileReader* fileReader, bool clearDoc);

	void OpenFile(const QString& fileName);

public slots:
	void on_actionNew_triggered();
	void on_actionOpen_triggered();
	void on_actionSave_triggered();
	void on_actionSaveAs_triggered();
	void on_actionInfo_triggered();
	void on_actionImportFEModel_triggered();
	void on_actionExportFEModel_triggered();
	void on_actionImportGeometry_triggered();
	void on_actionExportGeometry_triggered();
#ifdef HAS_QUAZIP
	void on_actionImportProject_triggered();
	void on_actionExportProject_triggered();
#endif
	void on_actionImportImage_triggered();
	void on_actionConvertFeb_triggered();
	void on_actionConvertGeo_triggered();
	void on_actionExit_triggered();
	void on_recentFiles_triggered(QAction* action);
	void on_recentFEFiles_triggered(QAction* action);
	void on_recentGeomFiles_triggered(QAction* action);

	void on_actionUndo_triggered();
	void on_actionRedo_triggered();
	void on_actionInvertSelection_triggered();
	void on_actionClearSelection_triggered();
	void on_actionDeleteSelection_triggered();
	void on_actionHideSelection_triggered();
	void on_actionHideUnselected_triggered();
	void on_actionUnhideAll_triggered();
	void on_actionToggleVisible_triggered();
	void on_actionNameSelection_triggered();
	void on_actionTransform_triggered();
	void on_actionCollapseTransform_triggered();
	void on_actionClone_triggered();
	void on_actionCloneGrid_triggered();
	void on_actionCloneRevolve_triggered();
	void on_actionMerge_triggered();
	void on_actionPurge_triggered();
	void on_actionDetach_triggered();
	void on_actionExtract_triggered();
	void on_actionEditProject_triggered();

	void on_actionAddBC_triggered();
	void on_actionAddNodalLoad_triggered();
	void on_actionAddSurfLoad_triggered();
	void on_actionAddBodyLoad_triggered();
	void on_actionAddIC_triggered();
	void on_actionAddContact_triggered();
	void on_actionAddRigidConstraint_triggered();
	void on_actionAddRigidConnector_triggered();
	void on_actionAddMaterial_triggered();
	void on_actionAddStep_triggered();
	void on_actionAddReaction_triggered();
	void on_actionSoluteTable_triggered();
	void on_actionSBMTable_triggered();

	void on_actionCurveEditor_triggered();
	void on_actionMeshInspector_triggered();
	void on_actionElasticityConvertor_triggered();
	void on_actionFEBioRun_triggered();
	void on_actionFEBioStop_triggered();
	void on_actionFEBioOptimize_triggered();
	void on_actionOptions_triggered();

	// Post menu actions
	void on_actionPlaneCut_triggered();
	void on_actionMirrorPlane_triggered();
	void on_actionVectorPlot_triggered();
	void on_actionTensorPlot_triggered();
	void on_actionIsosurfacePlot_triggered();
	void on_actionSlicePlot_triggered();
	void on_actionDisplacementMap_triggered();
	void on_actionStreamLinePlot_triggered();
	void on_actionParticleFlowPlot_triggered();
	void on_actionVolumeFlowPlot_triggered();
	void on_actionImageSlicer_triggered();
	void on_actionVolumeRender_triggered();
	void on_actionMarchingCubes_triggered();
	void on_actionGraph_triggered();
	void on_actionSummary_triggered();
	void on_actionStats_triggered();
	void on_actionIntegrate_triggered();

	// Record menu actions
	void on_actionRecordNew_triggered();
	void on_actionRecordStart_triggered();
	void on_actionRecordPause_triggered();
	void on_actionRecordStop_triggered();

	void on_actionUndoViewChange_triggered();
	void on_actionRedoViewChange_triggered();
	void on_actionZoomSelect_triggered();
	void on_actionZoomExtents_triggered();
	void on_actionViewCapture_toggled(bool bchecked);
	void on_actionOrtho_toggled(bool b);
	void on_actionShowGrid_toggled(bool b);
	void on_actionShowMeshLines_toggled(bool b);
	void on_actionShowEdgeLines_toggled(bool b);
	void on_actionBackfaceCulling_toggled(bool b);
	void on_actionViewSmooth_toggled(bool bchecked);
	void on_actionShowNormals_toggled(bool b);
	void on_actionShowFibers_toggled(bool b);
	void on_actionShowDiscrete_toggled(bool b);
	void on_actionFront_triggered();
	void on_actionBack_triggered();
	void on_actionLeft_triggered();
	void on_actionRight_triggered();
	void on_actionTop_triggered();
	void on_actionBottom_triggered();
	void on_actionWireframe_toggled(bool b);
	void on_actionSnap3D_triggered();
	void on_actionTrack_toggled(bool b);

	void on_actionOnlineHelp_triggered();
	void on_actionFEBioURL_triggered();
	void on_actionFEBioForum_triggered();
	void on_actionFEBioResources_triggered();
	void on_actionFEBioPubs_triggered();
	void on_actionAbout_triggered();

	void on_actionSelect_toggled(bool b);
	void on_actionTranslate_toggled(bool b);
	void on_actionRotate_toggled(bool b);
	void on_actionScale_toggled(bool b);
	void on_selectCoord_currentIndexChanged(int n);

	void on_actionSelectObjects_toggled(bool b);
	void on_actionSelectParts_toggled(bool b);
	void on_actionSelectSurfaces_toggled(bool b);
	void on_actionSelectCurves_toggled(bool b);
	void on_actionSelectNodes_toggled(bool b);
	void on_actionSelectDiscrete_toggled(bool b);

	void on_selectRect_toggled(bool b);
	void on_selectCircle_toggled(bool b);
	void on_selectFree_toggled(bool b);

	// Post toolbar
	void on_selectData_currentValueChanged(int i);
	void on_actionPlay_toggled(bool bchecked);
	void on_actionFirst_triggered();
	void on_actionPrev_triggered();
	void on_actionNext_triggered();
	void on_actionLast_triggered();
	void on_actionTimeSettings_triggered();
	void on_actionColorMap_toggled(bool bchecked);
	void on_selectTime_valueChanged(int n);

	// Font toolbar
	void on_fontStyle_currentFontChanged(const QFont& font);
	void on_fontSize_valueChanged(int i);
	void on_fontBold_toggled(bool checked);
	void on_fontItalic_toggled(bool bchecked);
	void on_actionProperties_triggered();

	void on_tab_currentChanged(int n);
	void on_tab_tabCloseRequested(int n);

	void on_modelViewer_currentObjectChanged(FSObject* po);

	void CloseView(int n);
	void CloseView(CPostDoc* postDoc);

	void SetCurrentState(int n);

	void closeEvent(QCloseEvent* ev);
	void keyPressEvent(QKeyEvent* ev);

	void finishedReadingFile(bool success, const QString& errorString);
	void checkFileProgress();

	void StopAnimation();

	void onTimer();

	void on_glview_pointPicked(const vec3d& r);
	void on_glview_selectionChanged();

	void onRunFinished(int exitCode, QProcess::ExitStatus es);
	void onReadyRead();
	void onErrorOccurred(QProcess::ProcessError err);

	void onExportMaterials(const vector<GMaterial*>& matList);
	void onExportAllMaterials();
	void onImportMaterials();

	void DeleteAllMaterials();
	void DeleteAllBC();
	void DeleteAllLoads();
	void DeleteAllIC();
	void DeleteAllContact();
	void DeleteAllConstraints();
	void DeleteAllConnectors();
	void DeleteAllSteps();

	CGLView* GetGLView();

	void changeViewMode(View_Mode vm);

	Post::CGLModel* GetCurrentModel();

	// update the font toolbar
	// (e.g. when a GL widget gets selected)
	void UpdateFontToolbar();

	void RunFEBioJob(CFEBioJob* job);

	void NextSSHFunction(CSSHHandler*);
	void ShowSSHProgress(bool show);
	void UpdateSSHProgress(int);

	bool DoModelCheck();

public:
	QStringList GetRecentFileList();

private:
	void ReadNextFileInQueue();
	bool HandleSSHMessage(CSSHHandler*);

public:
	int Views();
	void SetActiveView(int n);
	void AddView(const std::string& viewName, CPostDoc* doc = nullptr, bool makeActive = true);
	CPostDoc* GetActiveDocument();
	int FindView(CPostDoc* postDoc);

private:
	Ui::CMainWindow*	ui;
	CDocument*			m_doc;
	CFileThread*		m_fileThread;
	QStringList			m_fileQueue;
};

class CResource
{
public:
	static QIcon Icon(const QString& iconName);

public:
	static void Init(CMainWindow* wnd);

private:
	static CMainWindow*	m_wnd;
};
