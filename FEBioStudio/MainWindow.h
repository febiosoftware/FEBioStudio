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

#pragma once
#include <QMainWindow>
#include <QCloseEvent>
#include <QtCore/QProcess>
#include <FSCore/box.h>
#include <FEMLib/GMaterial.h>
#include <FSCore/math3d.h>

class FSObject;
class CDocument;
class CGLDocument;
class CModelDocument;
class CPostDocument;
class CFileThread;
class CPostFileThread;
class FileReader;
class GMaterial;
class CCreatePanel;
class CBuildPanel;
class CRepositoryPanel;
class QMenu;
class CGraphWindow;
class CPostDocument;
class CFEBioJob;
class CSSHHandler;
class xpltFileReader;
class CDocManager;
class QueuedFile;
class FEBioStudioProject;
class CGLView;
class CImageModel;
class CImageSliceView;
class C2DImageTimeView;
class GObject;
class FSPairedInterface;

enum class ImageFileType;

namespace Ui {
	class CMainWindow;
}

namespace Post {
	class CGLModel;
	class CGLObject;
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

	//! close the current open project
	void CloseProject();

	//! Get the active document
	CDocument* GetDocument();
	CGLDocument* GetGLDocument();
	CModelDocument* GetModelDocument();
	CPostDocument* GetPostDocument();

	// get the document manager
	CDocManager* GetDocManager();

	// Open a project folder
	bool OpenProject(const QString& projectPath);

	//! add a document 
	void AddDocument(CDocument* doc);

	//! import a list of files
	void ImportFiles(const QStringList& fileNames);

	void ImportProjectArchive(const QString& fileName);

	// open the object in the curve editor
	void OpenInCurveEditor(FSObject* po);

	// open the object in the model viewer
	void ShowInModelViewer(FSObject* po);

	// show the log panel
	void ShowLogPanel();

	// clear the log
	void ClearLog();

	// clear the output window
	void ClearOutput();

	// get the build panel
	CBuildPanel* GetBuildPanel();

	// get the create panel
	CCreatePanel* GetCreatePanel();

	// get the database panel
	CRepositoryPanel* GetDatabasePanel();

	// sets the current folder
	void SetCurrentFolder(const QString& folder);

	// see if user wants to save doc
	bool maybeSave();
	bool maybeSave(CDocument* doc);

	// write some useful info to the log regarding the selection
	void ReportSelection();

	// get the current theme
	int currentTheme() const;

	// check for dark theme
	bool usingDarkTheme() const;

	// set the current theme
	void setCurrentTheme(int n);

	// clear command stack on save
	bool clearCommandStackOnSave() const;

	// set clear command stack on save
	void setClearCommandStackOnSave(bool b);

	//! get the mesh mode
	int GetMeshMode();

	// set the current time value
	void SetCurrentTimeValue(float ftime);

	// get the current project
	FEBioStudioProject* GetProject();

	// autoSave Interval
	void setAutoSaveInterval(int interval);
	int autoSaveInterval();

	// autoUpdate Check
    QString GetServerMessage();
	bool updaterPresent();
	bool updateAvailable();

	// set/get default unit system for new models
	void SetDefaultUnitSystem(int n);
	int GetDefaultUnitSystem() const;

	// febio config file
	bool GetLoadConfigFlag();
	QString GetConfigFileName();

	void SetLoadConfigFlag(bool b);
	void SetConfigFileName(QString s);

	// --- WINDOW UPDATE ---

	//! Update the window title.
	void UpdateTitle();

	//! update the tab's text
	void UpdateTab(CDocument* doc);

	//! Update the window content
	void Update(QWidget* psend = 0, bool breset = false);

	// update the mesh inspector (if it is visible)
	void UpdateMeshInspector(bool breset);

	//! Update the GL control bar
	void UpdateGLControlBar();

	//! Update the post tool bar
	void UpdatePostToolbar();

	//! Update UI configuration
	void UpdateUIConfig();

	//! set the post doc that will be rendered in the GL view
	void SetActiveDocument(CDocument* doc);

	//! Update the post panel
	void UpdatePostPanel(bool braise = false, Post::CGLObject* po = nullptr);

	//! Update the model editor
	void UpdateModel(FSObject* po = 0, bool bupdate = true);

	//! Update the entire UI
	void UpdateUI();

	//! Update the toolbar
	void UpdateToolbar();

	//! set the selection mode
	void SetSelectionMode(int nselect);

	//! set item selection mode
	void SetItemSelectionMode(int nselect, int nitem);

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

	// clear the recent files list
	void ClearRecentFilesList();

	// remove a graph from the list
	void RemoveGraph(CGraphWindow* graph);

	// Add a graph to the list of managed graph windows
	void AddGraph(CGraphWindow* graph);

	// camera was changed
	void OnCameraChanged();

	// set the current time of the current post doc
	void SetCurrentTime(int n);

    void UpdateUiView();

	// the selection was transformed (i.e. translated, rotated, or scaled)
	void OnSelectionTransformed();

private:
	void writeSettings();
	void readSettings();
	void readThemeSetting();
    
public:
	// Read a file
	void ReadFile(CDocument* doc, const QString& fileName, FileReader* fileReader, int flags);

	FileReader* CreateFileReader(const QString& fileName);

	void OpenFile(const QString& fileName, bool showLoadOptions = true, bool openExternal = true);
	void OpenPostFile(const QString& fileName, CModelDocument* doc, bool showLoadOptions = true);

	bool SaveDocument(const QString& fileName);

	void ExportGeometry();
	void ExportPostGeometry();

	void SaveImage(QImage& image);

	CDocument* FindDocument(const std::string& filePath);

	bool CreateNewProject(QString fileName);

	CModelDocument* CreateNewDocument();

private:
	void ReadFile(QueuedFile& qfile);

	void OpenDocument(const QString& fileName);
	void OpenFEModel(const QString& fileName);
	void OpenFEBioFile(const QString& fileName);

	void SavePostDoc();

	void ShowWelcomePage();
	void CloseWelcomePage();

	void ProcessITKImage(const QString& fileName, ImageFileType type);

	bool ImportImage(CImageModel* imgModel);

	QString CurrentWorkingDirectory();

public slots:
    // add to the log 
	void AddLogEntry(const QString& txt);

	// add to the output window
	void AddOutputEntry(const QString& txt);

	void on_actionNewModel_triggered();
	void on_actionNewProject_triggered();
	void on_actionOpenProject_triggered();
	void on_actionOpen_triggered();
    // void on_actionReadInfo_triggered();
	void on_actionSave_triggered();
	void on_actionSaveAs_triggered();
	void on_actionSaveAll_triggered();
	void on_actionCloseAll_triggered();
	void on_actionSnapShot_triggered();
	void on_actionSaveProject_triggered();
	void on_actionExportFEModel_triggered();
	void on_actionImportGeometry_triggered();
	void on_actionExportGeometry_triggered();
	void on_actionImportProject_triggered();
	void on_actionExportProject_triggered();
	void on_actionImportRawImage_triggered();
	void on_actionImportDICOMImage_triggered();
	void on_actionImportTiffImage_triggered();
	void on_actionImportOMETiffImage_triggered();
    void on_actionImportNrrdImage_triggered();
	void on_actionImportImageSequence_triggered();
    void on_actionImportImageOther_triggered();
	void on_actionConvertFeb_triggered();
    void on_actionConvertFeb2Fsm_triggered();
    void on_actionConvertFsm2Feb_triggered();
	void on_actionConvertGeo_triggered();
	void on_actionExit_triggered();
	void on_recentFiles_triggered(QAction* action);
	void on_recentProjects_triggered(QAction* action);
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
	void on_actionFind_triggered();
	void on_actionSelectRange_triggered();
	void on_actionToggleVisible_triggered();
	void on_actionNameSelection_triggered();
	void on_actionTransform_triggered();
	void on_actionCollapseTransform_triggered();
	void on_actionClone_triggered();
	void on_actionCopyObject_triggered();
	void on_actionPasteObject_triggered();
	void on_actionCloneGrid_triggered();
	void on_actionCloneRevolve_triggered();
	void on_actionMerge_triggered();
	void on_actionPurge_triggered();
	void on_actionDetach_triggered();
	void on_actionExtract_triggered();
	void on_actionEditProject_triggered();
	void on_actionFaceToElem_triggered();
	void on_actionSurfaceToFaces_triggered();
	void on_actionSelectOverlap_triggered();
	void on_actionGrowSelection_triggered();
	void on_actionShrinkSelection_triggered();
	void on_actionSelectIsolatedVertices_triggered();
	void on_actionSyncSelection_triggered();

	void on_actionFindTxt_triggered();
	void on_actionFindAgain_triggered();
	void on_actionToggleComment_triggered();
	void on_actionDuplicateLine_triggered();
	void on_actionDeleteLine_triggered();

	// Physics menu
	void on_actionAddNodalBC_triggered();
	void on_actionAddSurfaceBC_triggered();
	void on_actionAddGeneralBC_triggered();
	void on_actionAddNodalLoad_triggered();
	void on_actionAddSurfLoad_triggered();
	void on_actionAddBodyLoad_triggered();
	void on_actionAddRigidLoad_triggered();
	void on_actionAddIC_triggered();
	void on_actionAddContact_triggered();
	void on_actionAddSurfaceNLC_triggered();
	void on_actionAddBodyNLC_triggered();
	void on_actionAddGenericNLC_triggered();
	void on_actionAddRigidBC_triggered();
	void on_actionAddRigidIC_triggered();
	void on_actionAddRigidConnector_triggered();
	void on_actionAddMaterial_triggered();
	void on_actionAddMeshAdaptor_triggered();
	void on_actionAddLoadController_triggered();
	void on_actionAddMeshDataMap_triggered();
	void on_actionAddMeshDataGenerator_triggered();
	void on_actionAddStep_triggered();
	void on_actionStepViewer_triggered();
	void on_actionAddReaction_triggered();
    void on_actionAddMembraneReaction_triggered();
	void on_actionSoluteTable_triggered();
	void on_actionSBMTable_triggered();

	void on_actionCurveEditor_triggered();
	void on_actionMeshInspector_triggered();
	void on_actionMeshDiagnostic_triggered();
	void on_actionElasticityConvertor_triggered();
	void on_actionMaterialTest_triggered();
	void on_actionUnitConverter_triggered();
	void on_actionKinemat_triggered();
	void on_actionPlotMix_triggered();
	void on_actionFEBioRun_triggered();
	void on_actionFEBioStop_triggered();
	void on_actionFEBioOptimize_triggered();
	void on_actionFEBioTangent_triggered();
	void on_actionFEBioInfo_triggered();
	void on_actionFEBioPlugins_triggered();
	void on_actionOptions_triggered();
	void on_actionLayerInfo_triggered();

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
	void on_actionImageWarp_triggered();
	void on_actionAddProbe_triggered();
	void on_actionAddCurveProbe_triggered();
	void on_actionAddRuler_triggered();
	void on_actionMusclePath_triggered();
	void on_actionPlotGroup_triggered();
	void on_actionGraph_triggered();
	void on_actionScatter_triggered();
	void on_actionSummary_triggered();
	void on_actionStats_triggered();
	void on_actionIntegrate_triggered();
	void on_actionIntegrateSurface_triggered();
	void on_actionImportPoints_triggered();
	void on_actionImportLines_triggered();

	// Record menu actions
	void on_actionRecordNew_triggered();
	void on_actionRecordStart_triggered();
	void on_actionRecordPause_triggered();
	void on_actionRecordStop_triggered();

	// View menu actions
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
	void on_actionShowFibers_triggered();
	void on_actionShowMatAxes_toggled(bool b);
	void on_actionShowDiscrete_toggled(bool b);
	void on_actionShowRigidBodies_toggled(bool b);
	void on_actionShowRigidJoints_toggled(bool b);
	void on_actionShowRigidLabels_toggled(bool b);
	void on_actionToggleLight_triggered();
	void on_actionFront_triggered();
	void on_actionBack_triggered();
	void on_actionLeft_triggered();
	void on_actionRight_triggered();
	void on_actionTop_triggered();
	void on_actionBottom_triggered();
    void on_actionIsometric_triggered();
	void on_actionRenderMode_toggled(bool b);
	void on_actionSnap3D_triggered();
	void on_actionTrack_toggled(bool b);
	void on_actionViewVPSave_triggered();
	void on_actionViewVPPrev_triggered();
	void on_actionViewVPNext_triggered();
	void on_actionSyncViews_triggered();
	void on_actionToggleConnected_triggered();
	void on_actionToggleFPS_triggered();

	void on_actionUpdate_triggered(bool dev = false);
	void on_actionFEBioURL_triggered();
	void on_actionFEBioUM_triggered();
	void on_actionFEBioTM_triggered();
	void on_actionFBSManual_triggered();
	void on_actionFEBioForum_triggered();
	void on_actionFEBioResources_triggered();
	void on_actionFEBioPubs_triggered();
	void on_actionAbout_triggered();
	void on_actionWelcome_triggered();
    void on_actionBugReport_triggered();

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
	void on_actionMeasureTool_triggered();
	void on_actionPlaneCutTool_triggered();

	void on_postSelectRect_toggled(bool b);
	void on_postSelectCircle_toggled(bool b);
	void on_postSelectFree_toggled(bool b);
	void on_postActionMeasureTool_triggered();

	// Post toolbar
	void on_selectData_currentValueChanged(int i);
	void on_actionPlay_toggled(bool bchecked);
	void on_actionRefresh_triggered();
	void on_actionFirst_triggered();
	void on_actionPrev_triggered();
	void on_actionNext_triggered();
	void on_actionLast_triggered();
	void on_actionTimeSettings_triggered();
	void on_actionColorMap_toggled(bool bchecked);
	void on_selectTime_valueChanged(int n);

	// signals from documents
	void on_doCommand(QString msg);
	void on_selectionChanged();

	// Font toolbar
	void on_fontStyle_currentFontChanged(const QFont& font);
	void on_fontSize_valueChanged(int i);
	void on_fontBold_toggled(bool checked);
	void on_fontItalic_toggled(bool bchecked);
	void on_actionProperties_triggered();

	void on_tab_currentChanged(int n);
	void on_tab_tabCloseRequested(int n);

	void on_htmlview_anchorClicked(const QUrl& link);
	void on_xmledit_textChanged();
    void on_xmlTree_modelEdited();

	void on_clearProject();
	void on_closeProject();
	void on_closeFile(const QString& file);
	void on_addToProject(const QString& file);

    // XML toolbar
    void on_actionEditXmlAsText_triggered(bool checked);
    void on_actionAddAttribute_triggered();
    void on_actionAddElement_triggered();
    void on_actionRemoveRow_triggered();

	// slots from Post panel
	void OnPostObjectStateChanged();
	void OnPostObjectPropsChanged(FSObject* po);

	void on_modelViewer_currentObjectChanged(FSObject* po);

	void onShowPartSelector();

	void checkJobProgress();

	void OnSelectMeshLayer(QAction* ac);
	void OnSelectObjectTransparencyMode(QAction* ac);
	void OnSelectObjectColorMode(QAction* ac);

	void CloseView(int n, bool forceClose = false);
	void CloseView(CDocument* doc);

	void SetCurrentState(int n);

	void closeEvent(QCloseEvent* ev);
	void keyPressEvent(QKeyEvent* ev);

	void on_finishedReadingFile(bool success, const QString& errorString);
	void finishedReadingFile(bool success, QueuedFile& qfile, const QString& errorString);
	void checkFileProgress();

	void StopAnimation();

	void onTimer();

	void on_glview_pointPicked(const vec3d& r);
	void on_glview_selectionChanged();

	void onExportMaterials(const std::vector<GMaterial*>& matList);
	void onExportAllMaterials();
	void onImportMaterials();
	void onImportMaterialsFromModel(CModelDocument* src);

	void DeleteAllMaterials();
	void DeleteAllBC();
	void DeleteAllLoads();
	void DeleteAllIC();
	void DeleteAllContact();
	void DeleteAllConstraints();
	void DeleteAllRigidLoads();
	void DeleteAllRigidBCs();
	void DeleteAllRigidICs();
	void DeleteAllRigidConnectors();
	void DeleteAllSteps();
	void DeleteAllJobs();
	void OnDeleteAllLoadControllers();
	void OnDeleteAllMeshData();

	void OnReplaceContactInterface(FSPairedInterface* pci);

	CGLView* GetGLView();
    CImageSliceView* GetImageSliceView();
    C2DImageTimeView* GetC2DImageTimeView();

	void UpdateGraphs(bool breset);

	Post::CGLModel* GetCurrentModel();

	// update the font toolbar
	// (e.g. when a GL widget gets selected)
	void UpdateFontToolbar();

	void RunFEBioJob(CFEBioJob* job);

	bool ExportFEBioFile(CModelDocument* doc, const std::string& fileName, int febioFileVersion);

	void NextSSHFunction(CSSHHandler*);
	void ShowProgress(bool show, QString message = "");
	void ShowIndeterminateProgress(bool show, QString message = "");
	void UpdateProgress(int);

	bool DoModelCheck(CModelDocument* doc);

	void toggleOrtho();

	void autosave();

	void autoUpdateCheck(bool update);

	void updateOutput(const QString& txt);

public:
	QStringList GetRecentFileList();
	QStringList GetRecentProjectsList();
	QStringList GetRecentPluginsList();

	void AddRecentPlugin(const QString& fileName);

	QString ProjectFolder();
	QString ProjectName();

private:
	void ReadNextFileInQueue();
	bool HandleSSHMessage(CSSHHandler*);

public:
	int Views();
	void SetActiveView(int n);
	void AddView(const std::string& viewName, CDocument* doc, bool makeActive = true);
	int FindView(CDocument* doc);
	GObject* GetActiveObject();

    QSize GetEditorSize();

	static CMainWindow* GetInstance();

private:
	Ui::CMainWindow*	ui;

	CDocManager*		m_DocManager;

	CFileThread*		m_fileThread;
	std::vector<QueuedFile>	m_fileQueue;

	static CMainWindow*		m_mainWnd;
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
