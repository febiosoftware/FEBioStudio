#include "stdafx.h"
#include "Document.h"
#include <GLLib/glx.h>
#include <GeomLib/GMeshObject.h>
#include <FSCore/enum.h>
#include "version.h"
#include "MainWindow.h"
#include <GeomLib/GPrimitive.h>
#include <GeomLib/GCurveMeshObject.h>
#include <GeomLib/GSurfaceMeshObject.h>
#include <MeshTools/FEFileExport.h>
#include <FEMLib/FEUserMaterial.h>
#include <FEMLib/FEMultiMaterial.h>
#include <MeshIO/PRVObjectFormat.h>
#include <QMessageBox>
#include <FEMLib/FEStepComponent.h>
#include <XML/XMLWriter.h>
#include <PostLib/Palette.h>
#include <PostGL/GLPlot.h>
#include <PostGL/GLDisplacementMap.h>
#include <PostGL/GLColorMap.h>
#include <PostGL/GLModel.h>
#include <PostLib/GLImageRenderer.h>
#include <PostLib/ImageModel.h>
#include <FSCore/FSDir.h>
#include <QtCore/QDir>
#include "PostDoc.h"
#include "Commands.h"
#include <sstream>

extern const int COLORS = 16;
GLColor col[COLORS] = {
	GLColor(240, 164, 96),
	GLColor(240, 240, 0),
	GLColor(240, 0, 240),
	GLColor(0, 240, 240),
	GLColor(240, 180, 0),
	GLColor(240, 0, 180),
	GLColor(180, 240, 0),
	GLColor(0, 240, 180),
	GLColor(180, 0, 240),
	GLColor(0, 180, 240),
	GLColor(0, 180, 0),
	GLColor(0, 0, 180),
	GLColor(180, 180, 0),
	GLColor(0, 180, 180),
	GLColor(180, 0, 180),
	GLColor(120, 0, 240)
};

void VIEW_SETTINGS::Defaults()
{
	m_bgrid = true;
	m_bmesh = true;
	m_bfeat = true;
	m_bnorm = false;
	m_nrender = RENDER_SOLID;
	m_scaleNormals = 1.0;
	m_nconv = 0;

	m_bjoint = true;
	m_bwall = true;
	m_brigid = true;
	m_bfiber = false;
	m_bcontour = false;
	m_blma = false;
	m_fiber_scale = 1.0;
	m_showHiddenFibers = false;
	m_showDiscrete = true;

	m_bcull = false;
	m_bconn = true;
	m_bmax = true;
	m_bpart = true;
	m_bhide = false;
	m_bext = false;
	m_bsoft = false;
	m_fconn = 30.f;
	m_bcullSel = true;
	m_bselpath = false;

	m_apply = 0;
	m_clearUndoOnSave = true;

	m_pos3d = vec3d(0, 0, 0);

	m_bTags = true;
	m_ntagInfo = 0;

	m_col1 = GLColor(164,164,255);
	m_col2 = GLColor(96,96,164);		

	m_mcol = GLColor(0, 0, 128);
	m_fgcol = GLColor(0, 0, 0);
	m_node_size = 7.f;
	m_line_size = 1.0f;
	m_bline_smooth = true;
	m_bpoint_smooth = true;

	m_snapToGrid = true;
	m_snapToNode = false;

	m_bLighting = true;
	m_bShadows = false;
	m_shadow_intensity = 0.5f;
	m_ambient = 0.09f;
	m_diffuse = 0.8f;

	m_transparencyMode = 0; // = off
	m_objectColor = 0; // = default (by material)
}

//=============================================================================
CDocObserver::CDocObserver(CDocument* doc) : m_doc(doc)
{
	if (m_doc) m_doc->AddObserver(this);
}

CDocObserver::~CDocObserver()
{
	if (m_doc) m_doc->RemoveObserver(this);
}

void CDocObserver::DocumentDelete()
{
	m_doc = nullptr;
	DocumentUpdate(true);
}

//-----------------------------------------------------------------------------
// Construction/Destruction
//-----------------------------------------------------------------------------
CDocument::CDocument(CMainWindow* wnd) : m_wnd(wnd)
{
	m_pCmd = new CCommandManager(this);
	m_psel = nullptr;
	m_activeJob = nullptr;

	// update the Post palette to match PreView's
	Post::CPaletteManager& PM = Post::CPaletteManager::GetInstance();
	
	Post::CPalette pal("preview");
	for (int i = 0; i < COLORS; ++i)
	{
		GLColor c = col[i];
		GLColor glc(c.r, c.g, c.b);
		pal.AddColor(glc);
	}

	PM.AddPalette(pal);
	PM.SetCurrentIndex(PM.Palettes() - 1);

	NewDocument();
}

//-----------------------------------------------------------------------------
CDocument::~CDocument()
{
	// remove all observers
	for (int i = 0; i < m_Observers.size(); ++i)
		m_Observers[i]->DocumentDelete();

	m_Observers.clear();

	delete m_pCmd;
}

//-----------------------------------------------------------------------------
void CDocument::NewDocument()
{
	// reset the filename
	m_filePath.clear();

	// reset the project
	m_Project.Reset();

	// reset the counters
	GModel::Reset();
	GMaterial::ResetRefs();

	// set document as not modified
	m_bModified = false;
	m_bValid = true;

	// Clear the command history
	m_pCmd->Clear();

	// Set default modes
	m_vs.nselect = SELECT_OBJECT;
	m_vs.ntrans = TRANSFORM_NONE;
	m_vs.nitem = ITEM_MESH;
	m_vs.nstyle = REGION_SELECT_BOX;

	// default display properties
	m_view.Defaults();

	// set some theme dependant settings
	int ntheme = GetMainWindow()->currentTheme();
	if (ntheme == 0)
	{
		m_view.m_col1 = GLColor(255, 255, 255);
		m_view.m_col2 = GLColor(128, 128, 255);
		m_view.m_nbgstyle = BG_HORIZONTAL;
	}
	else
	{
		m_view.m_col1 = GLColor(83, 83, 83);
		m_view.m_col2 = GLColor(128, 128, 128);
		m_view.m_nbgstyle = BG_HORIZONTAL;
	}

	// reset selection
	if (m_psel) delete m_psel;
	m_psel = 0;

	// clear job list
	m_JobList.Clear();
	m_activeJob = nullptr;
}

//-----------------------------------------------------------------------------
void CDocument::Clear()
{
	// reset the filename
	m_filePath.clear();

	// reset the project
	m_Project.Reset();

	// reset the counters
	GModel::Reset();
	GMaterial::ResetRefs();

	// set document as not modified
	m_bModified = false;
	m_bValid = true;

	// Clear the command history
	m_pCmd->Clear();

	// clear all the jobs
	m_JobList.Clear();

	// reset selection
	if (m_psel) delete m_psel;
	m_psel = 0;
}

//-----------------------------------------------------------------------------
bool CDocument::IsModified() { return m_bModified; }

//-----------------------------------------------------------------------------
void CDocument::SetModifiedFlag(bool bset)
{
	if (bset != m_bModified)
	{
		m_bModified = bset;
		if (m_wnd) m_wnd->UpdateTitle();
	}
}

//-----------------------------------------------------------------------------
bool CDocument::IsValid() { return m_bValid; }

//-----------------------------------------------------------------------------
// COMMAND FUNCTIONS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
bool CDocument::CanUndo() { return m_pCmd->CanUndo(); }

//-----------------------------------------------------------------------------
bool CDocument::CanRedo() { return m_pCmd->CanRedo(); }

//-----------------------------------------------------------------------------
void CDocument::AddCommand(CCommand* pcmd)
{
	m_pCmd->AddCommand(pcmd);
	SetModifiedFlag();
	UpdateSelection();
	GetGModel()->UpdateBoundingBox();
	CMainWindow* wnd = GetMainWindow();
	wnd->AddLogEntry(QString("Executing command: %1\n").arg(pcmd->GetName()));
}

//-----------------------------------------------------------------------------
void CDocument::AddCommand(CCommand* pcmd, const std::string& s)
{
	m_pCmd->AddCommand(pcmd);
	SetModifiedFlag();
	UpdateSelection();
	GetGModel()->UpdateBoundingBox();
	CMainWindow* wnd = GetMainWindow();
	if (s.empty() == false)
	{
		wnd->AddLogEntry(QString("Executing command: %1 (%2)\n").arg(pcmd->GetName()).arg(QString::fromStdString(s)));
	}
	else wnd->AddLogEntry(QString("Executing command: %1\n").arg(pcmd->GetName()));
}

//-----------------------------------------------------------------------------
const char* CDocument::GetUndoCmdName() { return m_pCmd->GetUndoCmdName(); }

//-----------------------------------------------------------------------------
const char* CDocument::GetRedoCmdName() { return m_pCmd->GetRedoCmdName(); }

//-----------------------------------------------------------------------------
bool CDocument::DoCommand(CCommand* pcmd)
{
	bool ret = m_pCmd->DoCommand(pcmd);
	SetModifiedFlag();
	UpdateSelection();
	GetGModel()->UpdateBoundingBox();

	CMainWindow* wnd = GetMainWindow();
	wnd->AddLogEntry(QString("Executing command: %1\n").arg(pcmd->GetName()));
	return ret;
}

//-----------------------------------------------------------------------------
bool CDocument::DoCommand(CCommand* pcmd, const std::string& s)
{
	bool ret = m_pCmd->DoCommand(pcmd);
	SetModifiedFlag();
	UpdateSelection();
	GetGModel()->UpdateBoundingBox();

	CMainWindow* wnd = GetMainWindow();
	if (s.empty() == false)
	{
		wnd->AddLogEntry(QString("Executing command: %1 (%2)\n").arg(pcmd->GetName()).arg(QString::fromStdString(s)));
	}
	else wnd->AddLogEntry(QString("Executing command: %1\n").arg(pcmd->GetName()));
	return ret;
}


//-----------------------------------------------------------------------------
const std::string& CDocument::GetCommandErrorString() const
{
	return m_pCmd->GetErrorString();
}

//-----------------------------------------------------------------------------
void CDocument::UndoCommand()
{
	string cmdName = m_pCmd->GetUndoCmdName();
	m_pCmd->UndoCommand();
	SetModifiedFlag();
	UpdateSelection();
	CMainWindow* wnd = GetMainWindow();
	wnd->AddLogEntry(QString("Undo last command (%1)\n").arg(QString::fromStdString(cmdName)));
}

//-----------------------------------------------------------------------------
void CDocument::RedoCommand()
{
	string cmdName = m_pCmd->GetRedoCmdName();
	m_pCmd->RedoCommand();
	SetModifiedFlag();
	UpdateSelection();
	CMainWindow* wnd = GetMainWindow();
	wnd->AddLogEntry(QString("Redo command (%1)\n").arg(QString::fromStdString(cmdName)));
}

//-----------------------------------------------------------------------------
void CDocument::ClearCommandStack()
{
	m_pCmd->Clear();
}

//-----------------------------------------------------------------------------
void CDocument::AddObserver(CDocObserver* observer)
{
	// no duplicates allowed
	for (int i = 0; i<m_Observers.size(); ++i)
	{
		if (m_Observers[i] == observer)
		{
			assert(false);
			return;
		}
	}

	m_Observers.push_back(observer);
}

//-----------------------------------------------------------------------------
void CDocument::RemoveObserver(CDocObserver* observer)
{
	for (int i = 0; i<m_Observers.size(); ++i)
	{
		if (m_Observers[i] == observer)
		{
			m_Observers.erase(m_Observers.begin() + i);
			return;
		}
	}
	assert(false);
}

//-----------------------------------------------------------------------------
void CDocument::UpdateObservers(bool bnew)
{
	if (m_Observers.empty()) return;

	for (int i = 0; i<m_Observers.size(); ++i)
	{
		CDocObserver* observer = m_Observers[i];
		if (observer) observer->DocumentUpdate(bnew);
	}
}

//-----------------------------------------------------------------------------
// VIEW STATE
//-----------------------------------------------------------------------------
void CDocument::SetViewState(VIEW_STATE vs)
{
	m_vs = vs;
	UpdateSelection();
//	if (m_wnd) m_wnd->UpdateUI();
}

//-----------------------------------------------------------------------------
// SELECTION
//-----------------------------------------------------------------------------
void CDocument::UpdateSelection()
{
	// delete old selection
	if (m_psel) delete m_psel;
	m_psel = 0;

	FEModel* ps = GetFEModel();

	// figure out if there is a mesh selected
	GObject* po = GetActiveObject();
	FEMesh* pm = (po ? po->GetFEMesh() : 0);
	FEMeshBase* pmb = (po ? po->GetEditableMesh() : 0);
	FELineMesh* plm = (po ? po->GetEditableLineMesh() : 0);

	// get the mesh mode
	int meshMode = m_wnd->GetMeshMode();

	if (m_vs.nitem == ITEM_MESH)
	{
		switch (m_vs.nselect)
		{
		case SELECT_OBJECT  : m_psel = new GObjectSelection(ps); break;
		case SELECT_PART    : m_psel = new GPartSelection(ps); break;
		case SELECT_FACE    : m_psel = new GFaceSelection(ps); break;
		case SELECT_EDGE    : m_psel = new GEdgeSelection(ps); break;
		case SELECT_NODE    : m_psel = new GNodeSelection(ps); break;
		case SELECT_DISCRETE: m_psel = new GDiscreteSelection(ps); break;
		}
	}
	else
	{
		if (meshMode == MESH_MODE_VOLUME)
		{
			switch (m_vs.nitem)
			{
			case ITEM_ELEM: if (pm) m_psel = new FEElementSelection(ps, pm); break;
			case ITEM_FACE: if (pm) m_psel = new FEFaceSelection(ps, pm); break;
			case ITEM_EDGE: if (pm) m_psel = new FEEdgeSelection(ps, pm); break;
			case ITEM_NODE: if (pm) m_psel = new FENodeSelection(ps, pm); break;
			}
		}
		else
		{
			switch (m_vs.nitem)
			{
			case ITEM_ELEM: if (pm) m_psel = new FEElementSelection(ps, pm); break;
			case ITEM_FACE: if (pmb) m_psel = new FEFaceSelection(ps, pmb); break;
			case ITEM_EDGE: if (plm) m_psel = new FEEdgeSelection(ps, plm); break;
			case ITEM_NODE: if (plm) m_psel = new FENodeSelection(ps, plm); break;
			}
		}
	}

	// update the window's toolbar to make sure it reflects the correct
	// selection tool
	m_wnd->ReportSelection();
}

//-----------------------------------------------------------------------------
FESelection* CDocument::GetCurrentSelection() { return m_psel; }

//-----------------------------------------------------------------------------
FEModel* CDocument::GetFEModel() { return &m_Project.GetFEModel(); }

//-----------------------------------------------------------------------------
GModel* CDocument::GetGModel()
{
	return &m_Project.GetFEModel().GetModel();
}

//-----------------------------------------------------------------------------
BOX CDocument::GetModelBox() { return m_Project.GetFEModel().GetModel().GetBoundingBox(); }

//-----------------------------------------------------------------------------
// return the active object
GObject* CDocument::GetActiveObject()
{
	GObject* po = nullptr;

	//	if (m_vs.nselect == SELECT_OBJECT)
	{
		FEModel* ps = GetFEModel();
		GObjectSelection sel(ps);
		if (sel.Count() == 1) po = sel.Object(0);
	}

	return po;
}

//-----------------------------------------------------------------------------
std::string CDocument::GetTypeString(FSObject* po)
{
	if (po == 0) return "(null)";

	if      (dynamic_cast<GPrimitive*         >(po)) return "Primitive";
	else if (dynamic_cast<GMeshObject*		  >(po)) return "Editable mesh";
	else if (dynamic_cast<GCurveMeshObject*   >(po)) return "Editable curve";
	else if (dynamic_cast<GSurfaceMeshObject* >(po)) return "Editable surface";
	else if (dynamic_cast<GMaterial*          >(po))
	{
		GMaterial* gmat = dynamic_cast<GMaterial*>(po);
		FEMaterial* mat = gmat->GetMaterialProperties();
		if (mat)
		{
			FEMaterialFactory& MF = *FEMaterialFactory::GetInstance();
			std::stringstream ss;
			const char* sztype = MF.TypeStr(mat);
			if (sztype == 0) sztype = "";
			ss << "Material" << " [" << sztype << "]";
			return ss.str();
		}
	}
	else if (dynamic_cast<FEStepComponent*>(po))
	{
		FEStepComponent* pc = dynamic_cast<FEStepComponent*>(po);
		return pc->GetTypeString();
	}
	else if (dynamic_cast<FEStep*>(po))
	{
		FEStep* step = dynamic_cast<FEStep*>(po);
		return step->GetTypeString();
	}
	else if (dynamic_cast<GDiscreteSpringSet*>(po)) return "Discrete element set";
	else if (dynamic_cast<GDiscreteElement*>(po)) return "discrete element";
	else if (dynamic_cast<FEPart*>(po)) return "element selection";
	else if (dynamic_cast<FESurface*>(po)) return "face selection";
	else if (dynamic_cast<FEEdgeSet*>(po)) return "edge selection";
	else if (dynamic_cast<FENodeSet*>(po)) return "node selection";
	else if (dynamic_cast<GPart*>(po)) return "Part";
	else if (dynamic_cast<GFace*>(po)) return "Surface";
	else if (dynamic_cast<GEdge*>(po)) return "Curve";
	else if (dynamic_cast<GNode*>(po)) return "Node";
	else if (dynamic_cast<GGroup*>(po)) return "Named selection";
	else if (dynamic_cast<FEGroup*>(po)) return "Named selection";
	else if (dynamic_cast<GObject*>(po)) return "Object";
	else if (dynamic_cast<FEDataMap*>(po)) return "Data map";
	else if (dynamic_cast<CFEBioJob*>(po)) return "Job";
	else if (dynamic_cast<Post::CImageModel*>(po)) return "3D Image volume";
	else if (dynamic_cast<Post::CGLPlot*>(po)) return "Plot";
	else if (dynamic_cast<Post::CGLDisplacementMap*>(po)) return "Displacement map";
	else if (dynamic_cast<Post::CGLColorMap*>(po)) return "Color map";
	else if (dynamic_cast<Post::CGLModel*>(po)) return "post model";
	else if (dynamic_cast<GModel*>(po)) return "model";
	else if (dynamic_cast<Post::CGLImageRenderer*>(po)) return "volume image renderer";
	else if (dynamic_cast<Post::CImageSource*>(po)) return "3D Image source";
	else if (dynamic_cast<FEMaterial*>(po))
	{
		FEMaterial* mat = dynamic_cast<FEMaterial*>(po);
		if (mat)
		{
			FEMaterialFactory& MF = *FEMaterialFactory::GetInstance();
			std::stringstream ss;
			const char* sztype = MF.TypeStr(mat);
			if (sztype == 0) sztype = "";
			ss << "Material" << " [" << sztype << "]";
			return ss.str();
		}
	}
	else if (dynamic_cast<FEMeshData*>(po)) return "Mesh data";
	else
	{
		assert(false);
	}

	return "(unknown)";
}

//-----------------------------------------------------------------------------
std::string CDocument::GetDocFilePath()
{ 
	return m_filePath; 
}

//-----------------------------------------------------------------------------
void CDocument::SetDocFilePath(const std::string& filePath)
{ 
	m_filePath = FSDir::filePath(filePath);
	string projectDir = GetDocFolder();
	FSDir::setMacro("ProjectDir", projectDir);
}

//-----------------------------------------------------------------------------
// get just the file name
std::string CDocument::GetDocFileName()
{
	if (m_filePath.empty()) return m_filePath;
	return FSDir::fileName(m_filePath);
}

//-----------------------------------------------------------------------------
std::string CDocument::GetDocFolder()
{
	if (m_filePath.empty()) return m_filePath;
	return FSDir::fileDir(m_filePath);
}

//-----------------------------------------------------------------------------
// get the base of the file name
std::string CDocument::GetDocFileBase()
{
	return FSDir::fileBase(m_filePath);
}

//-----------------------------------------------------------------------------

bool CDocument::SaveDocument(const std::string& fileName)
{
	OArchive ar;
	if (!ar.Create(fileName.c_str(), 0x00505256))
	{
		return false;
	}

	try
	{
		Save(ar);
		SetDocFilePath(fileName);
	}
	catch (...)
	{
		return false;
	}

	// clear the modified flag
	SetModifiedFlag(false);

	// clear the command stack
	if (m_view.m_clearUndoOnSave)
	{
		ClearCommandStack();
	}

	return true;
}

//-----------------------------------------------------------------------------
// FUNCTION: getline
// Reads a single line of a text file while skipping comment lines
//
char* getline(char* szline, int n, FILE* fp)
{
	char* ch;

	do
	{
		ch = fgets(szline, n, fp);
	} while (ch && (*ch == '*'));

	if (ch == 0) return 0;

	if (ch)
	{
		ch = strrchr(szline, '\n');
		if (ch) *ch = 0;
	}

	return szline;
}

//-----------------------------------------------------------------------------

void CDocument::Save(OArchive& ar)
{
	// save version info
	unsigned int version = SAVE_VERSION;
	ar.WriteChunk(CID_VERSION, version);

	// save model info
	ar.BeginChunk(CID_MODELINFO);
	{
		ar.WriteChunk(CID_MODELINFO_COMMENT, m_info);
	}
	ar.EndChunk();

	// save view settings
	ar.BeginChunk(CID_VIEW_SETTINGS);
	{
		ar.WriteChunk(CID_VIEW_CULL, m_view.m_bcull);
		ar.WriteChunk(CID_VIEW_CONNECTED, m_view.m_bconn);
		ar.WriteChunk(CID_VIEW_ANGLE_CONSTRAINT, m_view.m_bmax);
		ar.WriteChunk(CID_VIEW_CONN_ANGLE, m_view.m_fconn);
		ar.WriteChunk(CID_VIEW_PARTBOUNDS, m_view.m_bpart);
		ar.WriteChunk(CID_VIEW_AUTOHIDE, m_view.m_bhide);
		ar.WriteChunk(CID_VIEW_EXTONLY, m_view.m_bext);
		ar.WriteChunk(CID_VIEW_SOFTSELECT, m_view.m_bsoft);
		ar.WriteChunk(CID_VIEW_GRID, m_view.m_bgrid);
		ar.WriteChunk(CID_VIEW_MESH, m_view.m_bmesh);
		ar.WriteChunk(CID_VIEW_FEATURES, m_view.m_bfeat);
		ar.WriteChunk(CID_VIEW_NORMALS, m_view.m_bnorm);
		ar.WriteChunk(CID_VIEW_JOINTS, m_view.m_bjoint);
		ar.WriteChunk(CID_VIEW_RIGID_WALL, m_view.m_bwall);
		ar.WriteChunk(CID_VIEW_RIGID, m_view.m_brigid);
		ar.WriteChunk(CID_VIEW_FIBERS, m_view.m_bfiber);
		ar.WriteChunk(CID_VIEW_FIBER_SCALE, m_view.m_fiber_scale);
		ar.WriteChunk(CID_VIEW_CONTOUR, m_view.m_bcontour);
		ar.WriteChunk(CID_VIEW_LMA, m_view.m_blma);
		ar.WriteChunk(CID_VIEW_BGCOL1, m_view.m_col1);
		ar.WriteChunk(CID_VIEW_BGCOL2, m_view.m_col2);
		ar.WriteChunk(CID_VIEW_FGCOL, m_view.m_fgcol);
		ar.WriteChunk(CID_VIEW_MESH_COLOR, m_view.m_mcol);
		ar.WriteChunk(CID_VIEW_BGSTYLE, m_view.m_nbgstyle);
		ar.WriteChunk(CID_VIEW_NODESIZE, m_view.m_node_size);
		ar.WriteChunk(CID_VIEW_LINEWIDTH, m_view.m_line_size);
		ar.WriteChunk(CID_VIEW_RENDER, m_view.m_nrender);
		ar.WriteChunk(CID_VIEW_LINE_SMOOTH, m_view.m_bline_smooth);
		ar.WriteChunk(CID_VIEW_POINT_SMOOTH, m_view.m_bpoint_smooth);
		ar.WriteChunk(CID_VIEW_EMULATE_APPLY, m_view.m_apply);
		ar.WriteChunk(CID_VIEW_CLEAR_UNDO, m_view.m_clearUndoOnSave);
		ar.WriteChunk(CID_VIEW_SHOW_HIDDEN_FIBERS, m_view.m_showHiddenFibers);
        ar.WriteChunk(CID_VIEW_MULTIVIEW_CONV, m_view.m_nconv);
	}
	ar.EndChunk(); // CID_VIEW_SETTINGS

	// save resources
	if (ImageModels() > 0)
	{
		ar.BeginChunk(CID_RESOURCE_SECTION);
		{
			SaveResources(ar);
		}
		ar.EndChunk();
	}

	// save the project
	ar.BeginChunk(CID_PROJECT);
	{
		m_Project.Save(ar);
	}
	ar.EndChunk();

	// save the job lists
	for (int i = 0; i < FEBioJobs(); ++i)
	{
		CFEBioJob* job = GetFEBioJob(i);
		ar.BeginChunk(CID_FEBIOJOB);
		{
			job->Save(ar);
		}
		ar.EndChunk();
	}	
}

//-----------------------------------------------------------------------------
void CDocument::Load(IArchive& ar)
{
	CCallStack::ClearStack();
	TRACE("CDocument::Load");
	unsigned int version;

	m_bValid = false;

	IArchive::IOResult nret = IArchive::IO_OK;
	while (ar.OpenChunk() == IArchive::IO_OK)
	{
		int nid = ar.GetChunkID();

		if (nid == CID_VERSION)
		{
			nret = ar.read(version);
			if (nret != IArchive::IO_OK) throw ReadError("Error occurred when parsing CID_VERSION (CDocument::Load)");
			if (version < MIN_PRV_VERSION)
			{
				throw InvalidVersion();
			}
			else if (version < SAVE_VERSION)
			{
//				int nret = flx_choice("This file was created with an older version of PreView.\nThe file data may not be read in correctly.\n\nWould you like to continue?", "Yes", "No", 0);
//				if (nret != 0) throw InvalidVersion();
			}
			else if (version > SAVE_VERSION)
			{
				throw InvalidVersion();
			}

			// set the version number of the file
			ar.SetVersion(version);
		}
		else if (nid == CID_MODELINFO)
		{
			while (ar.OpenChunk() == IArchive::IO_OK)
			{
				int nid = ar.GetChunkID();
				if (nid == CID_MODELINFO_COMMENT)
				{
					nret = ar.read(m_info);
				}
				ar.CloseChunk();
			}
		}
		else if (nid == CID_VIEW_SETTINGS)
		{
			while (ar.OpenChunk() == IArchive::IO_OK)
			{
				int nid = ar.GetChunkID();
				switch (nid)
				{
				case CID_VIEW_CULL            : nret = ar.read(m_view.m_bcull); break;
				case CID_VIEW_CONNECTED       : nret = ar.read(m_view.m_bconn); break;
				case CID_VIEW_ANGLE_CONSTRAINT: nret = ar.read(m_view.m_bmax); break;
				case CID_VIEW_CONN_ANGLE      : nret = ar.read(m_view.m_fconn); break;
//				case CID_VIEW_PARTBOUNDS      : nret = ar.read(m_view.m_bpart); break;
				case CID_VIEW_AUTOHIDE        : nret = ar.read(m_view.m_bhide); break;
				case CID_VIEW_EXTONLY         : nret = ar.read(m_view.m_bext); break;
				case CID_VIEW_SOFTSELECT      : nret = ar.read(m_view.m_bsoft); break;
				case CID_VIEW_GRID            : nret = ar.read(m_view.m_bgrid); break;
				case CID_VIEW_MESH            : nret = ar.read(m_view.m_bmesh); break;
				case CID_VIEW_FEATURES        : nret = ar.read(m_view.m_bfeat); break;
				case CID_VIEW_NORMALS         : nret = ar.read(m_view.m_bnorm); break;
				case CID_VIEW_JOINTS          : nret = ar.read(m_view.m_bjoint); break;
				case CID_VIEW_RIGID_WALL      : nret = ar.read(m_view.m_bwall); break;
				case CID_VIEW_RIGID           : nret = ar.read(m_view.m_brigid); break;
				case CID_VIEW_FIBERS          : nret = ar.read(m_view.m_bfiber); break;
				case CID_VIEW_FIBER_SCALE     : nret = ar.read(m_view.m_fiber_scale); break;
				case CID_VIEW_CONTOUR         : nret = ar.read(m_view.m_bcontour); break;
				case CID_VIEW_LMA             : nret = ar.read(m_view.m_blma); break;
//				case CID_VIEW_BGCOL1          : nret = ar.read(m_view.m_col1); break;
//				case CID_VIEW_BGCOL2          : nret = ar.read(m_view.m_col2); break;
				case CID_VIEW_FGCOL           : nret = ar.read(m_view.m_fgcol); break;
				case CID_VIEW_MESH_COLOR      : nret = ar.read(m_view.m_mcol); break;
//				case CID_VIEW_BGSTYLE         : nret = ar.read(m_view.m_nbgstyle); break;
				case CID_VIEW_NODESIZE        : nret = ar.read(m_view.m_node_size); break;
				case CID_VIEW_LINEWIDTH       : nret = ar.read(m_view.m_line_size); break;
				case CID_VIEW_RENDER          : nret = ar.read(m_view.m_nrender); break;
				case CID_VIEW_LINE_SMOOTH     : nret = ar.read(m_view.m_bline_smooth); break;
				case CID_VIEW_POINT_SMOOTH    : nret = ar.read(m_view.m_bpoint_smooth); break;
				case CID_VIEW_EMULATE_APPLY   : nret = ar.read(m_view.m_apply); break;
				case CID_VIEW_CLEAR_UNDO	  : nret = ar.read(m_view.m_clearUndoOnSave); break;
				case CID_VIEW_SHOW_HIDDEN_FIBERS: nret = ar.read(m_view.m_showHiddenFibers); break;
                case CID_VIEW_MULTIVIEW_CONV  : nret = ar.read(m_view.m_nconv); break;
				}
				ar.CloseChunk();
				if (nret != IArchive::IO_OK) throw ReadError("Error occurred when parsing CID_VIEW_SETTINGS (CDocument::Load)");
			}
		}
		else if (nid == CID_RESOURCE_SECTION)
		{
			LoadResources(ar);
		}
		else if (nid == CID_PROJECT)
		{
			m_Project.Load(ar);
		}
		else if (nid == CID_FEBIOJOB)
		{
			CFEBioJob* job = new CFEBioJob(this);
			m_JobList.Add(job);
			job->Load(ar);
		}
		ar.CloseChunk();
	}

	m_bValid = true;
}

void CDocument::SaveResources(OArchive& ar)
{
	for (int i = 0; i < ImageModels(); ++i)
	{
		Post::CImageModel& img = *GetImageModel(i);
		ar.BeginChunk(CID_RESOURCE_IMAGEMODEL);
		{
			img.Save(ar);
		}
		ar.EndChunk();
	}
}

void CDocument::LoadResources(IArchive& ar)
{
	while (ar.OpenChunk() == IArchive::IO_OK)
	{
		int nid = ar.GetChunkID();

		switch (nid)
		{
		case CID_RESOURCE_IMAGEMODEL:
		{
			Post::CImageModel* img = new Post::CImageModel(nullptr);
			m_img.Add(img);
			img->Load(ar);
		}
		break;
		}

		ar.CloseChunk();
	}
}

//-----------------------------------------------------------------------------
bool CDocument::ImportModel(FEFileImport* preader, const char* szfile)
{
	ClearCommandStack();

	// clear the current project
	m_Project.Reset();

	// try to load the file
	if (preader->Load(m_Project, szfile) == false)
	{
		m_Project.Reset();
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
bool CDocument::ImportGeometry(FEFileImport* preader, const char *szfile)
{
	ClearCommandStack();

	// try to load the file
	if (preader->Load(m_Project, szfile) == false) return false;

	// set the modified flag
	m_bModified = true;

	return true;
}

//-----------------------------------------------------------------------------
// import image data
Post::CImageModel* CDocument::ImportImage(const std::string& fileName, int nx, int ny, int nz, BOX box)
{
	static int n = 1;

	// we pass the relative path to the image model
	string relFile = FSDir::toRelativePath(fileName);

	Post::CImageModel* po = new Post::CImageModel(nullptr);
	if (po->LoadImageData(relFile, nx, ny, nz, box) == false)
	{
		delete po;
		return nullptr;
	}

	stringstream ss;
	ss << "ImageModel" << n++;
	po->SetName(ss.str());

	// add it to the project
	AddImageModel(po);

	return po;
}

//-----------------------------------------------------------------------------
bool CDocument::Convert(const std::string& inFileName, const std::string& outFileName, FEFileImport* reader, FEFileExport* writer)
{
	// dummy project so we don't override this one
	FEProject prj;

	// try to read the project
	const char* szfile = inFileName.c_str();
	if (reader->Load(prj, szfile) == false) return false;

	// try to save the project
	const char* szout = outFileName.c_str();

	writer->ClearLog();
	if (writer->Export(prj, szout) == false) return false;

	return true;
}

//-----------------------------------------------------------------------------
void CDocument::GrowNodeSelection(FEMeshBase* pm)
{
	vector<vector<int> > NFT;
	pm->BuildNodeFaceTable(NFT);

	int i;
	int NN = pm->Nodes();
	int NF = pm->Faces();
	for (i = 0; i<NN; ++i) pm->Node(i).m_ntag = 0;
	for (i = 0; i<NF; ++i) pm->Face(i).m_ntag = 0;
	for (i = 0; i<NN; ++i)
	{
		if (pm->Node(i).IsSelected())
		{
			vector<int>& FT = NFT[i];
			int nf = (int)FT.size();
			for (int j = 0; j<nf; ++j) pm->Face(FT[j]).m_ntag = 1;
		}
	}

	for (i = 0; i<NF; ++i)
	{
		FEFace& f = pm->Face(i);
		if (f.m_ntag == 1)
		{
			for (int j = 0; j<f.Nodes(); ++j) pm->Node(f.n[j]).m_ntag = 1;
		}
	}

	// count the selection
	int n = 0;
	for (i = 0; i<NN; ++i) if (pm->Node(i).m_ntag) ++n;
	if (n > 0)
	{
		int* pn = new int[n];
		n = 0;
		for (i = 0; i<NN; ++i) if (pm->Node(i).m_ntag) pn[n++] = i;
		DoCommand(new CCmdSelectFENodes(pm, pn, n, false));
		delete[] pn;
	}
}

//-----------------------------------------------------------------------------
void CDocument::GrowFaceSelection(FEMeshBase* pm)
{
	int N = pm->Faces(), i;
	for (i = 0; i<N; ++i) pm->Face(i).m_ntag = 0;
	for (i = 0; i<N; ++i)
	{
		FEFace& f = pm->Face(i);
		if (f.IsSelected())
		{
			f.m_ntag = 1;
			int n = f.Nodes();
			if (n == 6) n = 3;
			if (n == 8) n = 4;
			for (int j = 0; j<n; ++j) if (f.m_nbr[j] >= 0) pm->Face(f.m_nbr[j]).m_ntag = 1;
		}
	}

	// count the selection
	int n = 0;
	for (i = 0; i<N; ++i) if (pm->Face(i).m_ntag) ++n;
	if (n > 0)
	{
		int* pf = new int[n];
		n = 0;
		for (i = 0; i<N; ++i) if (pm->Face(i).m_ntag) pf[n++] = i;
		DoCommand(new CCmdSelectFaces(pm, pf, n, false));
		delete[] pf;
	}
}

//-----------------------------------------------------------------------------
void CDocument::GrowElementSelection(FEMesh* pm)
{
	int N = pm->Elements(), i;
	for (i = 0; i<N; ++i) pm->Element(i).m_ntag = 0;
	for (i = 0; i<N; ++i)
	{
		FEElement& e = pm->Element(i);
		if (e.IsSelected())
		{
			e.m_ntag = 1;
			if (e.IsSolid()) for (int j = 0; j<e.Faces(); ++j) if (e.m_nbr[j] >= 0) pm->Element(e.m_nbr[j]).m_ntag = 1;
			if (e.IsShell()) for (int j = 0; j<e.Edges(); ++j) if (e.m_nbr[j] >= 0) pm->Element(e.m_nbr[j]).m_ntag = 1;
		}
	}

	// count the selection
	int n = 0;
	for (i = 0; i<N; ++i) if (pm->Element(i).m_ntag) ++n;
	if (n > 0)
	{
		int* pe = new int[n];
		n = 0;
		for (i = 0; i<N; ++i) if (pm->Element(i).m_ntag) pe[n++] = i;
		DoCommand(new CCmdSelectElements(pm, pe, n, false));
		delete[] pe;
	}
}

//-----------------------------------------------------------------------------
void CDocument::GrowEdgeSelection(FEMeshBase* pm)
{
	// TODO: implement this
}


//-----------------------------------------------------------------------------
void CDocument::ShrinkNodeSelection(FEMeshBase* pm)
{
	vector<vector<int> > NFT;
	pm->BuildNodeFaceTable(NFT);

	int i;
	int NN = pm->Nodes();
	int NF = pm->Faces();
	for (i = 0; i<NN; ++i) pm->Node(i).m_ntag = (pm->Node(i).IsSelected() ? 1 : 0);
	for (i = 0; i<NF; ++i) pm->Face(i).m_ntag = 0;
	for (i = 0; i<NN; ++i)
	{
		if (pm->Node(i).IsSelected() == false)
		{
			vector<int>& FT = NFT[i];
			int nf = (int)FT.size();
			for (int j = 0; j<nf; ++j) pm->Face(FT[j]).m_ntag = 1;
		}
	}

	for (i = 0; i<NF; ++i)
	{
		FEFace& f = pm->Face(i);
		if (f.m_ntag == 1)
		{
			for (int j = 0; j<f.Nodes(); ++j) pm->Node(f.n[j]).m_ntag = 0;
		}
	}

	// count the selection
	int n = 0;
	for (i = 0; i<NN; ++i) if (pm->Node(i).m_ntag) ++n;
	if (n > 0)
	{
		int* pn = new int[n];
		n = 0;
		for (i = 0; i<NN; ++i) if (pm->Node(i).m_ntag) pn[n++] = i;
		DoCommand(new CCmdSelectFENodes(pm, pn, n, false));
		delete[] pn;
	}
}

//-----------------------------------------------------------------------------
void CDocument::ShrinkFaceSelection(FEMeshBase* pm)
{
	int N = pm->Faces(), i;
	for (i = 0; i<N; ++i) pm->Face(i).m_ntag = 1;
	for (i = 0; i<N; ++i)
	{
		FEFace& f = pm->Face(i);
		if (f.IsSelected() == false)
		{
			f.m_ntag = 0;
			int n = f.Nodes();
			if (n == 6) n = 3;
			if (n == 8) n = 4;
			for (int j = 0; j<n; ++j) if (f.m_nbr[j] >= 0) pm->Face(f.m_nbr[j]).m_ntag = 0;
		}
	}

	// count the selection
	int n = 0;
	for (i = 0; i<N; ++i) if (pm->Face(i).m_ntag) ++n;
	if (n > 0)
	{
		int* pf = new int[n];
		n = 0;
		for (i = 0; i<N; ++i) if (pm->Face(i).m_ntag) pf[n++] = i;
		DoCommand(new CCmdSelectFaces(pm, pf, n, false));
		delete[] pf;
	}
}

//-----------------------------------------------------------------------------
void CDocument::ShrinkEdgeSelection(FEMeshBase* pm)
{
	// TODO: implement this
}

//-----------------------------------------------------------------------------
void CDocument::ShrinkElementSelection(FEMesh* pm)
{
	int N = pm->Elements(), i;
	for (i = 0; i<N; ++i) pm->Element(i).m_ntag = 1;
	for (i = 0; i<N; ++i)
	{
		FEElement& e = pm->Element(i);
		if (e.IsSelected() == false)
		{
			e.m_ntag = 0;
			if (e.IsSolid()) for (int j = 0; j<e.Faces(); ++j) if (e.m_nbr[j] >= 0) pm->Element(e.m_nbr[j]).m_ntag = 0;
			if (e.IsShell()) for (int j = 0; j<e.Edges(); ++j) if (e.m_nbr[j] >= 0) pm->Element(e.m_nbr[j]).m_ntag = 0;
		}
	}

	// count the selection
	int n = 0;
	for (i = 0; i<N; ++i) if (pm->Element(i).m_ntag) ++n;
	if (n > 0)
	{
		int* pe = new int[n];
		n = 0;
		for (i = 0; i<N; ++i) if (pm->Element(i).m_ntag) pe[n++] = i;
		DoCommand(new CCmdSelectElements(pm, pe, n, false));
		delete[] pe;
	}
}

void CDocument::HideCurrentSelection()
{
	if (GetItemMode() == ITEM_MESH)
	{
		// Hide object level items
		int nmode = GetSelectionMode();
		if ((nmode == SELECT_OBJECT) || (nmode == SELECT_PART))
		{
			FESelection* ps = GetCurrentSelection();
			if (ps && ps->Size())
			{
				DoCommand(new CCmdHideSelection());
			}
		}
	}
	else
	{
		// Hide sub-object (i.e. mesh) level items
		FESelection* ps = GetCurrentSelection();
		if (ps && ps->Size())
		{
			DoCommand(new CCmdHideSelection());
		}
	}
}

void CDocument::HideUnselected()
{
	int itemMode = GetItemMode();
	if (itemMode == ITEM_MESH)
	{
		int selMode = GetSelectionMode();
		if (selMode == SELECT_OBJECT)
		{
			GModel* mdl = GetGModel();
			vector<GObject*> po;
			for (int i=0; i<mdl->Objects(); ++i) 
				if (mdl->Object(i)->IsSelected() == false) po.push_back(mdl->Object(i));

			DoCommand(new CCmdHideObject(po, true));
		}
		else if (selMode == SELECT_PART)
		{
			GModel* mdl = GetGModel();
			list<GPart*> partList;
			for (int i=0; i<mdl->Objects(); ++i)
			{
				GObject* po = mdl->Object(i);
				for (int j=0; j<po->Parts(); ++j)
				{
					GPart* part = po->Part(j);
					if (part->IsSelected() == false) partList.push_back(part);
				}
			}

			DoCommand(new CCmdHideParts(partList));
		}
	}
	else 
	{
		GObject* po = GetActiveObject();
		if (po == 0) return;

		if (itemMode == ITEM_ELEM)
		{
			FEMesh* pm = po->GetFEMesh();
			if (pm == 0) return;

			vector<int> elemList;
			for (int i=0; i<pm->Elements(); ++i)
			{
				FEElement& el = pm->Element(i);
				if (el.IsSelected() == false) elemList.push_back(i);
			}
			DoCommand(new CCmdHideElements(pm, elemList));
		}
		else if (itemMode == ITEM_FACE)
		{
			FESurfaceMesh* pm = dynamic_cast<FESurfaceMesh*>(po->GetEditableMesh());
			if (pm == 0) return;

			vector<int> faceList;
			for (int i = 0; i<pm->Faces(); ++i)
			{
				FEFace& face = pm->Face(i);
				if (face.IsSelected() == false) faceList.push_back(i);
			}
			DoCommand(new CCmdHideFaces(pm, faceList));
		}
	}
}

void CDocument::DeleteObject(FSObject* po)
{
	FEModel& fem = *GetFEModel();

	if (dynamic_cast<FEStep*>(po))
	{
		if (dynamic_cast<FEInitialStep*>(po))
		{
			QMessageBox::warning(m_wnd, "Delete step", "Cannot delete the initial step.");
			return;
		}
		else
		{
			DoCommand(new CCmdDeleteFSObject(po));
		}
	}
	else if (dynamic_cast<FEMaterial*>(po))
	{
		FEMaterial* pm = dynamic_cast<FEMaterial*>(po);
		FEMaterial* parent = const_cast<FEMaterial*>(pm->GetParentMaterial());
		FEMaterialProperty* pp = parent->FindProperty(pm);
		if (pp)// && (pp->maxSize() == 0))
		{
			pp->RemoveMaterial(pm);
		}
		else
		{
			QMessageBox::warning(m_wnd, "FEBio Studio", "Cannot delete this material property.");
			return;
		}
	}
	else if (dynamic_cast<GPart*>(po))
	{
		GPart* pg = dynamic_cast<GPart*>(po);
		GetGModel()->DeletePart(pg);

		// This cannot be undone (yet) so clear the undo stack
		ClearCommandStack();
	}
	else if (dynamic_cast<Post::CGLPlot*>(po))
	{
		// We don't push plane cut plots on the undo stack because
		// the plane cuts will remain active until the object is actually deleted. 
		assert(po->GetParent());
		delete po;
	}
	else if (po->GetParent())
	{
		if (dynamic_cast<CFEBioJob*>(po))
		{
			CFEBioJob* job = dynamic_cast<CFEBioJob*>(po);
			if (job->GetPostDoc())
			{
				m_wnd->CloseView(job->GetPostDoc());
			}
		}
		DoCommand(new CCmdDeleteFSObject(po));
	}
	else
	{
		assert(false);
	}

	SetModifiedFlag(true);
}

// helper function for applying a modifier
bool CDocument::ApplyFEModifier(FEModifier& modifier, GObject* po, FEGroup* sel, bool clearSel)
{
	// get the mesh
	FEMesh* pm = po->GetFEMesh();
	if (pm == 0) return false;

	// apply modifier and create new mesh
	FEMesh* newMesh = 0;
	if (sel)
		newMesh = modifier.Apply(sel);
	else
		newMesh = modifier.Apply(pm);

	// make sure the modifier succeeded
	if (newMesh == 0) return false;

	// clear the selection
	if (clearSel) newMesh->ClearFaceSelection();

	// swap the meshes
	string ss = modifier.GetName();
	return DoCommand(new CCmdChangeFEMesh(po, newMesh), ss.c_str());
}

bool CDocument::ApplyFESurfaceModifier(FESurfaceModifier& modifier, GSurfaceMeshObject* po, FEGroup* sel)
{
	// get the surface mesh
	FESurfaceMesh* mesh = po->GetSurfaceMesh();
	if (mesh == 0) return false;

	// create a new mesh
	FESurfaceMesh* newMesh = modifier.Apply(mesh, sel);
	if (newMesh == 0) return false;

	// if the object has an FE mesh, we need to delete it
	CCommand* cmd = nullptr;
	if (po->GetFEMesh())
	{
		CCmdGroup* cmdg = new CCmdGroup("Apply surface modifier");
		cmdg->AddCommand(new CCmdChangeFEMesh(po, nullptr));
		cmdg->AddCommand(new CCmdChangeFESurfaceMesh(po, newMesh));
		cmd = cmdg;
	}
	else cmd = new CCmdChangeFESurfaceMesh(po, newMesh);
	
	// swap the meshes
	return DoCommand(cmd);
}

bool CDocument::ExportMaterials(const std::string& fileName, const vector<GMaterial*>& matList)
{
	if (matList.size() == 0) return false;

	OArchive ar;
	//									P V M 
	if (ar.Create(fileName.c_str(), 0x0050564D) == false) return false;

	// save version info
	unsigned int version = PVM_VERSION_NUMBER;
	ar.WriteChunk(PVM_VERSION, version);

	// write materials
	int mats = (int) matList.size();
	for (int i = 0; i<mats; ++i)
	{
		GMaterial* pmat = matList[i];
		int ntype = pmat->GetMaterialProperties()->Type();

		ar.BeginChunk(PVM_MATERIAL);
		{
			ar.WriteChunk(PVM_MATERIAL_TYPE, ntype);
			ar.BeginChunk(PVM_MATERIAL_DATA);
			{
				pmat->Save(ar);
			}
			ar.EndChunk();
		}
		ar.EndChunk();
	}
	ar.Close();

	return true;
}

bool CDocument::ImportMaterials(const std::string& fileName)
{
	IArchive ar;
	if (ar.Open(fileName.c_str(), 0x0050564D) == false) return false;

	FEModel& fem = *GetFEModel();
	IArchive::IOResult nret = IArchive::IO_OK;
	while (ar.OpenChunk() == IArchive::IO_OK)
	{
		int nid = ar.GetChunkID();

		if (nid == PVM_VERSION)
		{
			unsigned int version = 0;
			nret = ar.read(version);
			if (nret != IArchive::IO_OK) return false;

			if (version != PVM_VERSION_NUMBER) return false;

			ar.SetVersion(version);
		}
		else if (nid == PVM_MATERIAL)
		{
			GMaterial* gmat = 0;
			while (ar.OpenChunk() == IArchive::IO_OK)
			{
				int nid = ar.GetChunkID();

				if (nid == PVM_MATERIAL_TYPE)
				{
					int ntype = -1;
					ar.read(ntype);
					// allocate the material
					FEMaterial* pm = 0;
					if (ntype == FE_USER_MATERIAL) pm = new FEUserMaterial(FE_USER_MATERIAL);
					else pm = FEMaterialFactory::Create(ntype);

					if (pm == 0) return false;
					gmat = new GMaterial(pm);
				}
				else if (nid == PVM_MATERIAL_DATA)
				{
					if (gmat == 0) return false;
					gmat->Load(ar);
				}

				ar.CloseChunk();
			}

			if (gmat) fem.AddMaterial(gmat);
		}
		ar.CloseChunk();
	}

	return true;
}

void CDocument::AddObject(GObject* po)
{
	DoCommand(new CCmdAddAndSelectObject(po));
	GetMainWindow()->Update(0, true);
}

FEDataMap* CDocument::CreateDataMap(FSObject* po, std::string& mapName, std::string& paramName, Param_Type type)
{
	FEModel& fem = *GetFEModel();
	FEComponent* pc = dynamic_cast<FEComponent*>(po);
	if (pc == 0) return 0;
	FEDataMap* dataMap = pc->CreateMap(paramName, type);
	dataMap->SetName(mapName);

	fem.AddDataMap(dataMap);

	return dataMap;
}

bool CDocument::GenerateFEBioOptimizationFile(const std::string& fileName, FEBioOpt& opt)
{
	const char* szlog[] = { "LOG_DEFAULT", "LOG_NEVER", "LOG_FILE_ONLY", "LOG_SCREEN_ONLY", "LOG_FILE_AND_SCREEN" };
	const char* szprt[] = { "PRINT_ITERATIONS", "PRINT_VERBOSE" };

	XMLWriter xml;

	if (xml.open(fileName.c_str()) == false) return false;

	XMLElement root("febio_optimize");
	root.add_attribute("version", "2.0");
	xml.add_branch(root);

	// print options
	XMLElement ops("Options");
	switch (opt.method)
	{
	case 0: ops.add_attribute("type", "levmar"); break;
	case 1: ops.add_attribute("type", "constrained levmar"); break;
	}
	xml.add_branch(ops);
	{
		xml.add_leaf("obj_tol", opt.obj_tol);
		xml.add_leaf("f_diff_scale", opt.f_diff_scale);
		xml.add_leaf("log_level", szlog[opt.outLevel]);
		xml.add_leaf("print_level", szprt[opt.printLevel]);
	}
	xml.close_branch();

	// parameters
	XMLElement params("Parameters");
	xml.add_branch(params);
	for (int i = 0; i < (int)opt.m_params.size(); ++i)
	{
		FEBioOpt::Param& pi = opt.m_params[i];
		XMLElement par("param");
		par.add_attribute("name", pi.m_name);
		double v[3] = {pi.m_initVal, pi.m_minVal, pi.m_maxVal};
		par.value(v, 3);
		xml.add_leaf(par);
	}
	xml.close_branch();

	// objective
	XMLElement obj("Objective");
	obj.add_attribute("type", "data-fit");
	xml.add_branch(obj);
	{
		XMLElement fnc("fnc");
		fnc.add_attribute("type", "parameter");
		xml.add_branch(fnc);
		{
			XMLElement p("param");
			p.add_attribute("name", opt.m_objParam);
			xml.add_empty(p);
		}
		xml.close_branch();

		xml.add_branch("data");
		{
			for (int i = 0; i < (int)opt.m_data.size(); ++i)
			{
				FEBioOpt::Data& di = opt.m_data[i];
				double v[2] = { di.m_time, di.m_value };
				xml.add_leaf("pt", v, 2);
			}
		}
		xml.close_branch();
	}
	xml.close_branch();

	// closing tag
	xml.close_branch();
	
	xml.close();

	return true;
}

int CDocument::FEBioJobs() const
{
	return (int) m_JobList.Size();
}

void CDocument::AddFEbioJob(CFEBioJob* job)
{
	m_JobList.Add(job);
	SetModifiedFlag();
}

CFEBioJob* CDocument::GetFEBioJob(int i)
{
	return m_JobList[i];
}

void CDocument::SetActiveJob(CFEBioJob* job)
{
	m_activeJob = job;
}

CFEBioJob* CDocument::GetActiveJob()
{
	return m_activeJob;
}

CFEBioJob* CDocument::FindFEBioJob(const std::string& s)
{
	for (int i = 0; i < FEBioJobs(); ++i)
	{
		CFEBioJob* job = m_JobList[i];
		if (job->GetName() == s) return job;
	}

	return nullptr;
}

int CDocument::ImageModels() const
{
	return (int)m_img.Size();
}

void CDocument::AddImageModel(Post::CImageModel* img)
{
	assert(img);
	m_img.Add(img);
	SetModifiedFlag();
}

Post::CImageModel* CDocument::GetImageModel(int i)
{
	return m_img[i];
}

void CDocument::DeleteAllImageModels()
{
	m_img.Clear();
}

std::vector<MODEL_ERROR> CDocument::CheckModel()
{
	vector<MODEL_ERROR> errorList;
	checkModel(this, errorList);
	return errorList;
}
