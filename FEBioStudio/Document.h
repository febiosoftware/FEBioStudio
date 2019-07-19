#pragma once

#include <PreViewLib/FEProject.h>
#include <PreViewLib/LoadCurve.h>
#include <PreViewLib/FESelection.h>
#include <PreViewLib/Serializable.h>
#include <PreViewIO/FileReader.h>
#include <FEMLib/FEDataMap.h>
#include "CommandManager.h"
#include "FEBioOpt.h"
#include "FEBioJob.h"

//-----------------------------------------------------------------------------
// Transform Modes
#define TRANSFORM_NONE		1
#define TRANSFORM_MOVE		2
#define TRANSFORM_ROTATE	3
#define TRANSFORM_SCALE		4

//-----------------------------------------------------------------------------
// Selection Modes
#define SELECT_OBJECT	1
#define SELECT_PART		2
#define SELECT_FACE		3
#define SELECT_EDGE		4
#define SELECT_NODE		5
#define SELECT_DISCRETE	6

//-----------------------------------------------------------------------------
// Selection Styles
#define SELECT_BOX		1
#define SELECT_CIRCLE	2
#define SELECT_FREE		3

//-----------------------------------------------------------------------------
// Sub-object item Modes
#define	ITEM_MESH		1
#define ITEM_ELEM		2
#define	ITEM_FACE		3
#define ITEM_NODE		4
#define ITEM_EDGE		5

//-----------------------------------------------------------------------------
// back ground styles
#define BG_COLOR1		0
#define BG_COLOR2		1
#define BG_HORIZONTAL	2
#define BG_VERTICAL		3

//-----------------------------------------------------------------------------
// render mode
#define RENDER_SOLID		0
#define RENDER_WIREFRAME	1

//-----------------------------------------------------------------------------
//! view settings
struct VIEW_SETTINGS
{
	bool	m_bcull;	//!< cull backface flag
	bool	m_bconn;	//!< select connected
	bool	m_bmax;		//!< max angle constraint for select connected
	bool	m_bpart;	//!< respect partition boundaries flag
	bool	m_bhide;	//!< auto hide
	bool	m_bext;		//!< ignore interior nodes
	bool	m_bsoft;	//!< soft selection mode
	bool	m_bcullSel;	//!< ignore backfacing when selecting

	bool	m_bselpath;	//!< select by shortest path

	float	m_fconn;	//!< max connectivity angle (for selecting faces)

	bool	m_bgrid;	//!< show grid flag
	bool	m_bmesh;	//!< show edged faces
	bool	m_bfeat;	//!< show feature edges
	bool	m_bnorm;	//!< show face normals
	double	m_scaleNormals;	//!< scale factor for normals
	int		m_nrender;	//!< render mode
    int     m_nconv;    //!< multiview projection convention

	int		m_apply;	//!< emulate apply via middle mouse button

	bool	m_bjoint;	//!< show rigid joints
	bool	m_bwall;	//!< show rigid walls
	bool	m_brigid;	//!< show rigid kinematics
	bool	m_bfiber;	//!< show material fibers
	bool	m_bcontour;	//!< show contour plot
	bool	m_blma;		//!< show local material axes
	double	m_fiber_scale;	//!< scale factor for rendering fibers
	bool	m_showHiddenFibers;	//!< show fibers/axes on hidden parts

	GLCOLOR	m_col1, m_col2;		//!< background colors
	GLCOLOR	m_fgcol;			//!< foreground color
	GLCOLOR	m_mcol;				//!< mesh line color
	int		m_nbgstyle;			//!< back ground style
	float	m_node_size;		//!< size of nodes when displayed
	float	m_line_size;		//!< line size
	bool	m_bline_smooth;		//!< line smoothing flag
	bool	m_bpoint_smooth;	//!< point smoothing flag

	bool	m_snapToGrid;		//!< snap to grid
	bool	m_snapToNode;		//!< snap to nodes

	bool	m_clearUndoOnSave;	//!< clear undo stack on save

	vec3d	m_pos3d;	// The location of the 3D cursor
};

//-----------------------------------------------------------------------------
class CMainWindow;
class FEFileExport;

//-----------------------------------------------------------------------------
// Document class which stores all the data
//
class CDocument : public CSerializable
{
public:
	// --- constructor/destructor ---
	CDocument(CMainWindow* wnd);
	virtual ~CDocument();

	void NewDocument();
	bool LoadTemplate(int n);

	void Clear();

	// --- I/O-routines ---
	// Save the document to PRV file
	bool SaveDocument(const std::string& fileName);

	// import model (will clear current project)
	bool ImportModel(FEFileImport* preader, const char* szfile);

	// import geometry (geometry is added to current project)
	bool ImportGeometry(FEFileImport* preader, const char* szfile);

	// import image data
	bool ImportImage(const std::string& fileName, int nx, int ny, int nz, BOX box);

	// save/load project
	void Load(IArchive& ar);
	void Save(OArchive& ar);

	// get the complete file path
	std::string GetDocFilePath();

	// set the document file path
	void SetDocFilePath(const std::string& fileName);

	// get just the file name
	std::string GetDocFileName();

	// get the file folder
	std::string GetDocFolder();

	// get the base of the file name
	std::string GetDocFileBase();

	// convert a file between formats
	bool Convert(const std::string& inFileName, const std::string& outFileName, FEFileImport* reader, FEFileExport* writer);

	// --- Document validation ---
	bool IsModified();
	void SetModifiedFlag(bool bset = true);
	bool IsValid();

	// --- Command history functions ---
	bool CanUndo();
	bool CanRedo();
	void AddCommand(CCommand* pcmd);
	void AddCommand(CCommand* pcmd, const std::string& s);
	bool DoCommand(CCommand* pcmd);
	bool DoCommand(CCommand* pcmd, const std::string& s);
	void UndoCommand();
	void RedoCommand();
	const char* GetUndoCmdName();
	const char* GetRedoCmdName();
	void ClearCommandStack();
	const std::string& GetCommandErrorString() const;

	// --- Data ---
	FESelection* GetCurrentSelection();
	void UpdateSelection();

	FEModel* GetFEModel();

	GModel* GetGModel();

	BOX GetModelBox();

	// return the active object
	GObject* GetActiveObject();

	//! Get the project
	FEProject& GetProject() { return m_Project; }

	// --- view settings ---
	VIEW_SETTINGS& GetViewSettings() { return m_view; }

	void Set3DCursor(const vec3d& r) { m_view.m_pos3d = r; }
	vec3d Get3DCursor() const { return m_view.m_pos3d; }

	// --- view state ---
	VIEW_STATE GetViewState() { return m_vs; }
	void SetViewState(VIEW_STATE vs);

	int GetTransformMode() { return m_vs.ntrans; }
	void SetTransformMode(int mode) { m_vs.ntrans = mode; UpdateSelection(); }

	int GetSelectionMode() { return m_vs.nselect; }
	void SetSelectionMode(int mode) { m_vs.nitem = ITEM_MESH; m_vs.nselect = mode; UpdateSelection(); }

	void SetSelectionStyle(int nstyle) { m_vs.nstyle = nstyle; }
	int GetSelectionStyle() { return m_vs.nstyle; }

	int GetItemMode() { return m_vs.nitem; }
	void SetItemMode(int mode) { m_vs.nitem = mode; UpdateSelection(); }

	static std::string GetTypeString(FEObject* po);

	CMainWindow* GetMainWindow() { return m_wnd; }

	void DeleteObject(FEObject* po);

	bool ExportMaterials(const std::string& fileName, const vector<GMaterial*>& matList);
	bool ImportMaterials(const std::string& fileName);

public:
	// helper function for applying a modifier
	bool ApplyFEModifier(FEModifier& modifier, GObject* po, FEGroup* sel = 0, bool clearSel = true);
	bool ApplyFESurfaceModifier(FESurfaceModifier& modifier, GSurfaceMeshObject* po, FEGroup* sel = 0);

public:
	void GrowNodeSelection(FEMeshBase* pm);
	void GrowFaceSelection(FEMeshBase* pm);
	void GrowEdgeSelection(FEMeshBase* pm);
	void GrowElementSelection(FEMesh* pm);
	void ShrinkNodeSelection(FEMeshBase* pm);
	void ShrinkFaceSelection(FEMeshBase* pm);
	void ShrinkEdgeSelection(FEMeshBase* pm);
	void ShrinkElementSelection(FEMesh* pm);

	void HideCurrentSelection();
	void HideUnselected();

public:
	FEDataMap* CreateDataMap(FEObject* po, std::string& mapName, std::string& paramName, Param_Type type);

public:
	void setModelInfo(const std::string& s) { m_info = s; }
	std::string getModelInfo() const { return m_info; }

public:
	bool GenerateFEBioOptimizationFile(const std::string& fileName, FEBioOpt& opt);

public:
	int FEBioJobs() const;
	void AddFEbioJob(CFEBioJob* job, bool makeActive = true);
	CFEBioJob* GetFEBioJob(int i);

	void SetActiveJob(CFEBioJob* job);
	CFEBioJob* GetActiveJob();
	
	CFEBioJob* FindFEBioJob(const std::string& s);

protected:
	// Modified flag
	bool	m_bModified;	// is document modified since last saved ?
	bool	m_bValid;		// is the current document in a valid state for rendering

	// the FE Project
	FEProject	m_Project;

	// Binary file path
	std::string		m_filePath;

	// current selection
	FESelection*	m_psel;

	// The command manager
	CCommandManager*	m_pCmd;		// the command manager

	// display properties
	VIEW_SETTINGS	m_view;

	VIEW_STATE	m_vs;	// the view state

	CMainWindow*	m_wnd;

	std::string		m_info;

	vector<CFEBioJob*>	m_JobList;
	CFEBioJob*			m_activeJob;
};
