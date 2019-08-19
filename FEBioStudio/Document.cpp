// Document.cpp: implementation of the CDocument class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Document.h"
#include "glx.h"
#include <GeomLib/GMeshObject.h>
#include <FSCore/enum.h>
#include "version.h"
#include "MainWindow.h"
#include <GeomLib/GPrimitive.h>
#include <GeomLib/GCurveMeshObject.h>
#include <GeomLib/GSurfaceMeshObject.h>
#include <MeshTools/FEFileExport.h>
#include <MeshTools/FEUserMaterial.h>
#include <MeshTools/FEMultiMaterial.h>
#include <MeshIO/PRVObjectFormat.h>
#include <QMessageBox>
#include <FEMLib/FEStepComponent.h>
#include <ImageLib/ImageStack.h>
#include "GImageObject.h"
#include <XML/XMLWriter.h>
#include <PostLib/Palette.h>
#include <PostGL/GLPlot.h>
#include <PostGL/GLDisplacementMap.h>
#include <PostGL/GLColorMap.h>
#include <PostGL/GLModel.h>
#include "PostDoc.h"
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
	SetModifiedFlag(false);
	m_bValid = true;

	// Clear the command history
	m_pCmd->Clear();

	// Set default modes
	m_vs.nselect = SELECT_OBJECT;
	m_vs.ntrans = TRANSFORM_NONE;
	m_vs.nitem = ITEM_MESH;
	m_vs.nstyle = REGION_SELECT_BOX;

	// default display properties
	m_view.m_bgrid = true;
	m_view.m_bmesh = true;
	m_view.m_bfeat = true;
	m_view.m_bnorm = false;
	m_view.m_nrender = RENDER_SOLID;
	m_view.m_scaleNormals = 1.0;
    m_view.m_nconv = 0;

	m_view.m_bjoint = true;
	m_view.m_bwall = true;
	m_view.m_brigid = true;
	m_view.m_bfiber = false;
	m_view.m_bcontour = false;
	m_view.m_blma = false;
	m_view.m_fiber_scale = 1.0;
	m_view.m_showHiddenFibers = false;

	m_view.m_bcull = false;
	m_view.m_bconn = true;
	m_view.m_bmax = true;
	m_view.m_bpart = true;
	m_view.m_bhide = false;
	m_view.m_bext = false;
	m_view.m_bsoft = false;
	m_view.m_fconn = 30.f;
	m_view.m_bcullSel = true;
	m_view.m_bselpath = false;

	m_view.m_apply = 0;
	m_view.m_clearUndoOnSave = true;

	m_view.m_pos3d = vec3d(0, 0, 0);

	m_view.m_bTags = true;
	m_view.m_ntagInfo = 0;

	//	m_view.m_col1 = GLColor(164,164,255);
	//	m_view.m_col2 = GLColor(96,96,164);

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

	//	m_view.m_mcol = GLColor(128, 128, 128);
	m_view.m_mcol = GLColor(0, 0, 128);
	m_view.m_fgcol = GLColor(0, 0, 0);
	m_view.m_node_size = 7.f;
	m_view.m_line_size = 1.0f;
	m_view.m_bline_smooth = true;
	m_view.m_bpoint_smooth = true;

	m_view.m_snapToGrid = true;
	m_view.m_snapToNode = false;

	// reset selection
	if (m_psel) delete m_psel;
	m_psel = 0;

	// clear job list
	for (int i = 0; i < m_JobList.size(); ++i) delete m_JobList[i];
	m_JobList.clear();
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
	SetModifiedFlag(false);
	m_bValid = true;

	// Clear the command history
	m_pCmd->Clear();

	// reset selection
	if (m_psel) delete m_psel;
	m_psel = 0;
}

//-----------------------------------------------------------------------------
bool CDocument::IsModified() { return m_bModified; }

//-----------------------------------------------------------------------------
void CDocument::SetModifiedFlag(bool bset)
{
	m_bModified = bset;
//	if (m_wnd) m_wnd->UpdateTitle();
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

	switch (m_vs.nitem)
	{
	case ITEM_MESH:
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
	break;
	case ITEM_ELEM: if (pm) m_psel = new FEElementSelection(ps, pm); break;
	case ITEM_FACE: if (pmb) m_psel = new FEFaceSelection(ps, pmb); break;
	case ITEM_EDGE: if (plm) m_psel = new FEEdgeSelection(ps, plm); break;
	case ITEM_NODE: if (plm) m_psel = new FENodeSelection(ps, plm); break;
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
			ss << "Material" << " (" << sztype << ")";
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
	else if (dynamic_cast<GLinearSpringSet*>(po)) return "Linear spring set";
	else if (dynamic_cast<GNonlinearSpringSet*>(po)) return "Nonlinear spring set";
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
	else if (dynamic_cast<GImageObject*>(po)) return "3D Image";
	else if (dynamic_cast<Post::CGLPlot*>(po)) return "Plot";
	else if (dynamic_cast<Post::CGLDisplacementMap*>(po)) return "Displacement map";
	else if (dynamic_cast<Post::CGLColorMap*>(po)) return "Color map";
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
	m_filePath = filePath; 
}

//-----------------------------------------------------------------------------
// get just the file name
std::string CDocument::GetDocFileName()
{
	if (m_filePath.empty()) return m_filePath;

	std::string fileName;
	size_t n = m_filePath.rfind('\\');
	if (n == string::npos)
	{
		n = m_filePath.rfind('/');
		if (n == string::npos) n = 0; else n++;
	}
	else n++;

	fileName = m_filePath.substr(n);
	
	return fileName;
}

//-----------------------------------------------------------------------------
std::string CDocument::GetDocFolder()
{
	if (m_filePath.empty()) return m_filePath;

	size_t n = m_filePath.rfind('\\');
	if (n == string::npos)
	{
		n = m_filePath.rfind('/');
		if (n == string::npos) n = 0; else n++;
	}
	else n++;

	std::string folder = m_filePath.substr(0, n);

	return folder;
}

//-----------------------------------------------------------------------------
// get the base of the file name
std::string CDocument::GetDocFileBase()
{
	std::string file = GetDocFileName();
	size_t n = file.rfind('.');
	if (n == string::npos) return file;

	std::string base = file.substr(0, n);

	return base;
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

	// save the project
	ar.BeginChunk(CID_PROJECT);
	{
		m_Project.Save(ar);
	}
	ar.EndChunk();
}

//-----------------------------------------------------------------------------


void CDocument::Load(IArchive& ar)
{
	CCallStack::ClearStack();
	TRACE("CDocument::Load");
	unsigned int version;

	m_bValid = false;

	IOResult nret = IO_OK;
	while (ar.OpenChunk() == IO_OK)
	{
		int nid = ar.GetChunkID();

		if (nid == CID_VERSION)
		{
			nret = ar.read(version);
			if (nret != IO_OK) throw ReadError("Error occurred when parsing CID_VERSION (CDocument::Load)");
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
			while (ar.OpenChunk() == IO_OK)
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
			while (ar.OpenChunk() == IO_OK)
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
				if (nret != IO_OK) throw ReadError("Error occurred when parsing CID_VIEW_SETTINGS (CDocument::Load)");
			}
		}
		else if (nid == CID_PROJECT)
		{
			m_Project.Load(ar);
		}
		ar.CloseChunk();
	}

	m_bValid = true;
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
	SetModifiedFlag();

	return true;
}

//-----------------------------------------------------------------------------
// import image data
bool CDocument::ImportImage(const std::string& fileName, int nx, int ny, int nz, BOX box)
{
	GImageObject* po = new GImageObject;
	if (po->LoadImageData(fileName, nx, ny, nz, box) == false)
	{
		delete po;
		return false;
	}

	// use the file name to make the object's name
	size_t c1 = fileName.rfind('\\');
	if (c1 == fileName.npos) c1 = fileName.rfind('/');
	if (c1 == fileName.npos) c1 = -1;
	c1++;

	size_t c2 = fileName.rfind('.');
	if (c2 == fileName.npos) c2 = fileName.size();
	else if (c2 < c1) c2 = fileName.size();
	
	string name = fileName.substr(c1, c2 - c1);
	po->SetName(name);

	// add it to the project
	AddImageObject(po);

	return true;
}

//-----------------------------------------------------------------------------
// load a plot file
bool CDocument::LoadPlotFile(const std::string& fileName)
{
	// create a dummy job
	CFEBioJob* job = new CFEBioJob;
	job->SetPlotFileName(fileName);
	if (job->OpenPlotFile() == false)
	{
		delete job;
		return false;
	}
	else
	{
		AddFEbioJob(job);
		return true;
	}
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
			vector<GPart*> partList;
			for (int i=0; i<mdl->Objects(); ++i)
			{
				GObject* po = mdl->Object(i);
				for (int j=0; j<po->Parts(); ++j)
				{
					GPart* part = po->Part(j);
					if (part->IsSelected() == false) partList.push_back(part);
				}
			}

			DoCommand(new CCmdHidePart(partList));
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

	// Notice the order of the if-statements. This is important due to order of the inheritance structure. 
	// TODO: redefine base classes so this is no longer an issue.
	if (dynamic_cast<FEInitialCondition*>(po))
	{
		FEInitialCondition* pi = dynamic_cast<FEInitialCondition*>(po);
		FEStep* pstep = fem.GetStep(pi->GetStep());
		pstep->RemoveIC(pi);
	}
	else if (dynamic_cast<FEPrescribedDOF*>(po))
	{
		FEPrescribedDOF* pb = dynamic_cast<FEPrescribedDOF*>(po);
		FEStep* pstep = fem.GetStep(pb->GetStep());
		pstep->RemoveBC(pb);
	}
	else if (dynamic_cast<FEFixedDOF*>(po))
	{
		FEFixedDOF* pb = dynamic_cast<FEFixedDOF*>(po);
		FEStep* pstep = fem.GetStep(pb->GetStep());
		pstep->RemoveBC(pb);
	}
	else if (dynamic_cast<FEPrescribedBC*>(po))
	{
		FEPrescribedBC* pb = dynamic_cast<FEPrescribedBC*>(po);
		FEStep* pstep = fem.GetStep(pb->GetStep());
		pstep->RemoveLoad(pb);
	}
	else if (dynamic_cast<FEInterface*>(po))
	{
		FEInterface* pi = dynamic_cast<FEInterface*>(po);
		FEStep* pstep = fem.GetStep(pi->GetStep());
		pstep->RemoveInterface(pi);
	}
	else if (dynamic_cast<GMaterial*>(po))
	{
		GMaterial* pm = dynamic_cast<GMaterial*>(po);
		if (fem.CanDeleteMaterial(pm) ||
			(QMessageBox::question(m_wnd, "Delete Material", "This material is being used. Area you sure you want to delete it?") == QMessageBox::Yes))
		{
			fem.DeleteMaterial(pm);
		}
	}
	else if (dynamic_cast<FERigidConstraint*>(po))
	{
		FERigidConstraint* rc = dynamic_cast<FERigidConstraint*>(po);
		FEStep* pstep = fem.FindStep(rc->GetStep()); assert(pstep);
		if (pstep) pstep->RemoveRC(rc);
	}
	else if (dynamic_cast<FEConnector*>(po))
	{
		FEConnector* rc = dynamic_cast<FEConnector*>(po);
		FEStep* pstep = fem.FindStep(rc->GetStep()); assert(pstep);
		if (pstep) pstep->RemoveConnector(rc);
	}
	else if (dynamic_cast<FEStep*>(po))
	{
		if (dynamic_cast<FEInitialStep*>(po))
		{
			QMessageBox::warning(m_wnd, "Delete step", "Cannot delete the initial step.");
		}
		else
		{
			FEStep* pstep = dynamic_cast<FEStep*>(po);
			fem.DeleteStep(pstep);
		}
	}
	else if (dynamic_cast<GObject*>(po))
	{
		GObject* obj = dynamic_cast<GObject*>(po);
		GModel& m = fem.GetModel();
		m.RemoveObject(obj);
	}
	else if (dynamic_cast<GDiscreteObject*>(po))
	{
		GDiscreteObject* pd = dynamic_cast<GDiscreteObject*>(po);
		GModel& m = fem.GetModel();
		m.RemoveDiscreteObject(pd);
	}
	else if (dynamic_cast<FEPart*>(po))
	{
		FEPart* pg = dynamic_cast<FEPart*>(po);
		GObject* obj = pg->GetGObject(); assert(obj);
		if (obj) obj->RemoveFEPart(pg);
	}
	else if (dynamic_cast<FESurface*>(po))
	{
		FESurface* pg = dynamic_cast<FESurface*>(po);
		GObject* obj = pg->GetGObject(); assert(obj);
		if (obj) obj->RemoveFESurface(pg);
	}
	else if (dynamic_cast<FEEdgeSet*>(po))
	{
		FEEdgeSet* pg = dynamic_cast<FEEdgeSet*>(po);
		GObject* obj = pg->GetGObject(); assert(obj);
		if (obj) obj->RemoveFEEdgeSet(pg);
	}
	else if (dynamic_cast<FENodeSet*>(po))
	{
		FENodeSet* pg = dynamic_cast<FENodeSet*>(po);
		GObject* obj = pg->GetGObject(); assert(obj);
		if (obj) obj->RemoveFENodeSet(pg);
	}
	else if (dynamic_cast<GPart*>(po))
	{
		GPart* pg = dynamic_cast<GPart*>(po);
		GetGModel()->DeletePart(pg);

		// This cannot be undone (yet) so clear the undo stack
		ClearCommandStack();
	}
	else if (dynamic_cast<GNodeList*>(po))
	{
		GNodeList* pg = dynamic_cast<GNodeList*>(po);
		GetGModel()->RemoveNodeList(pg);
	}
	else if (dynamic_cast<GEdgeList*>(po))
	{
		GEdgeList* pg = dynamic_cast<GEdgeList*>(po);
		GetGModel()->RemoveEdgeList(pg);
	}
	else if (dynamic_cast<GFaceList*>(po))
	{
		GFaceList* pg = dynamic_cast<GFaceList*>(po);
		GetGModel()->RemoveFaceList(pg);
	}
	else if (dynamic_cast<GPartList*>(po))
	{
		GPartList* pg = dynamic_cast<GPartList*>(po);
		GetGModel()->RemovePartList(pg);
	}
	else if (dynamic_cast<FEBodyLoad*>(po))
	{
		FEBodyLoad* pbl = dynamic_cast<FEBodyLoad*>(po);
		FEStep* pstep = fem.GetStep(pbl->GetStep());
		pstep->RemoveLoad(pbl);
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
		else QMessageBox::warning(m_wnd, "PreView2", "Cannot delete this material property.");
	}
	else if (dynamic_cast<FEDataMap*>(po))
	{
		FEDataMap* map = dynamic_cast<FEDataMap*>(po);
		fem.RemoveMap(map);
		FEComponent* pc = map->GetParent();
		pc->DeleteMap(map);
	}
	else if (dynamic_cast<CFEBioJob*>(po))
	{
		CFEBioJob* job = dynamic_cast<CFEBioJob*>(po);
		DeleteFEBioJob(job);
	}
	else if (dynamic_cast<GImageObject*>(po))
	{
		GImageObject* imgObj = dynamic_cast<GImageObject*>(po);
		DeleteImageObject(imgObj);
	}
	else if (dynamic_cast<Post::CGLPlot*>(po))
	{
		Post::CGLPlot* plot = dynamic_cast<Post::CGLPlot*>(po);
		plot->GetModel()->DeletePlot(plot);
	}
	else
	{
		assert(false);
	}
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

	// swap the meshes
	return DoCommand(new CCmdChangeFESurfaceMesh(po, newMesh));

	return true;
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
	IOResult nret = IO_OK;
	while (ar.OpenChunk() == IO_OK)
	{
		int nid = ar.GetChunkID();

		if (nid == PVM_VERSION)
		{
			unsigned int version = 0;
			nret = ar.read(version);
			if (nret != IO_OK) return false;

			if (version != PVM_VERSION_NUMBER) return false;

			ar.SetVersion(version);
		}
		else if (nid == PVM_MATERIAL)
		{
			GMaterial* gmat = 0;
			while (ar.OpenChunk() == IO_OK)
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
	return (int)m_JobList.size();
}

void CDocument::AddFEbioJob(CFEBioJob* job, bool makeActive)
{
	m_JobList.push_back(job);
	if (makeActive) SetActiveJob(job);
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
		if (job->GetFileName() == s) return job;
	}

	return nullptr;
}

void CDocument::DeleteFEBioJob(CFEBioJob* job)
{
	for (int i = 0; i < FEBioJobs(); ++i)
	{
		CFEBioJob* job_i = m_JobList[i];
		if (job == job_i)
		{
			GetMainWindow()->DeleteView(i + 1);
			m_JobList.erase(m_JobList.begin() + i);
			delete job;
		}
	}
}

int CDocument::ImageObjects() const
{
	return (int)m_img.size();
}

void CDocument::AddImageObject(GImageObject* img)
{
	assert(img);
	m_img.push_back(img);
}

GImageObject* CDocument::GetImageObject(int i)
{
	return m_img[i];
}

void CDocument::DeleteAllImageObjects()
{
	for (int i = 0; i < ImageObjects(); ++i)
	{
		delete GetImageObject(i);
	}
	m_img.clear();
}

void CDocument::DeleteImageObject(GImageObject* img)
{
	for (int i = 0; i < ImageObjects(); ++i)
	{
		GImageObject* img_i = GetImageObject(i);
		if (img_i == img)
		{
			m_img.erase(m_img.begin() + i);
			delete img_i;
			return;
		}
	}
	assert(false);
}
