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
#include <MeshTools/GModel.h>
#include <FSCore/FSDir.h>
#include <QtCore/QDir>
#include "Commands.h"
#include "FEBioJob.h"
#include <sstream>

// defined in MeshTools\GMaterial.cpp
extern GLColor col[];

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
	m_bconn = false;
	m_bmax = true;
	m_bpart = true;
	m_bhide = false;
	m_bext = false;
	m_bsoft = false;
	m_fconn = 30.f;
	m_bcullSel = true;
	m_bselpath = false;

	m_apply = 0;

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
	m_fileWriter = nullptr;
	m_fileReader = nullptr;

	static int n = 1;

	m_pCmd = new CCommandManager(this);

	// update the Post palette to match PreView's
	Post::CPaletteManager& PM = Post::CPaletteManager::GetInstance();
	
	Post::CPalette pal("preview");
	for (int i = 0; i < GMaterial::MAX_COLORS; ++i)
	{
		GLColor c = col[i];
		GLColor glc(c.r, c.g, c.b);
		pal.AddColor(glc);
	}

	PM.AddPalette(pal);
	PM.SetCurrentIndex(PM.Palettes() - 1);

	// reset the filename
	m_filePath.clear();

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
void CDocument::Clear()
{
	// reset the filename
	m_filePath.clear();

	// set document as not modified
	m_bModified = false;
	m_bValid = true;

	// Clear the command history
	m_pCmd->Clear();
}

//-----------------------------------------------------------------------------
bool CDocument::Initialize()
{
	return true;
}

//-----------------------------------------------------------------------------
bool CDocument::IsModified() 
{ 
	return m_bModified;
}

//-----------------------------------------------------------------------------
void CDocument::SetModifiedFlag(bool bset)
{
	if (bset != m_bModified)
	{
		m_bModified = bset;
		if (m_wnd) m_wnd->UpdateTab(this);
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
	CMainWindow* wnd = GetMainWindow();
	wnd->AddLogEntry(QString("Executing command: %1\n").arg(pcmd->GetName()));
}

//-----------------------------------------------------------------------------
void CDocument::AddCommand(CCommand* pcmd, const std::string& s)
{
	m_pCmd->AddCommand(pcmd);
	SetModifiedFlag();
	UpdateSelection();
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
	CMainWindow* wnd = GetMainWindow();
	wnd->AddLogEntry(QString("Executing command: %1\n").arg(pcmd->GetName()));
	bool ret = m_pCmd->DoCommand(pcmd);
	SetModifiedFlag();
	UpdateSelection();
	return ret;
}

//-----------------------------------------------------------------------------
bool CDocument::DoCommand(CCommand* pcmd, const std::string& s)
{
	CMainWindow* wnd = GetMainWindow();
	if (s.empty() == false)
	{
		wnd->AddLogEntry(QString("Executing command: %1 (%2)\n").arg(pcmd->GetName()).arg(QString::fromStdString(s)));
	}
	else wnd->AddLogEntry(QString("Executing command: %1\n").arg(pcmd->GetName()));

	bool ret = m_pCmd->DoCommand(pcmd);
	SetModifiedFlag();
	UpdateSelection();
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

void CDocument::UpdateSelection(bool breport)
{

}

//-----------------------------------------------------------------------------
void CDocument::SetViewState(VIEW_STATE vs)
{
	m_vs = vs;
	UpdateSelection(false);
//	if (m_wnd) m_wnd->UpdateUI();
}

//-----------------------------------------------------------------------------
GObject* CDocument::GetActiveObject()
{
	return nullptr;
}

//-----------------------------------------------------------------------------
QString CDocument::ToAbsolutePath(const QString& relativePath)
{
	return ToAbsolutePath(relativePath.toStdString());
}

//-----------------------------------------------------------------------------
QString CDocument::ToAbsolutePath(const std::string& relativePath)
{
	QString s = QString::fromStdString(FSDir::expandMacros(relativePath));
	QDir modelDir(QString::fromStdString(GetDocFolder()));
	return QDir::toNativeSeparators(QDir::cleanPath(modelDir.absoluteFilePath(s)));
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
// set the document's title
void CDocument::SetDocTitle(const std::string& t)
{
	m_title = t;
}

//-----------------------------------------------------------------------------
std::string CDocument::GetDocTitle()
{
	return m_title;
}

//-----------------------------------------------------------------------------
void CDocument::SetDocFilePath(const std::string& filePath)
{ 
	m_filePath = FSDir::filePath(filePath);
	m_title = GetDocFileName();
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

// set/get the file reader
void CDocument::SetFileReader(FileReader* fileReader)
{
	if (fileReader != m_fileReader)
	{
		if (m_fileReader) delete m_fileReader;
		m_fileReader = fileReader;
	}
}

FileReader* CDocument::GetFileReader()
{
	return m_fileReader;
}

// set/get the file writer
void CDocument::SetFileWriter(FileWriter* fileWriter)
{
	if (fileWriter != m_fileWriter)
	{
		if (m_fileWriter) delete m_fileWriter;
		m_fileWriter = fileWriter;
	}
}

FileWriter* CDocument::GetFileWriter()
{
	return m_fileWriter;
}

//-----------------------------------------------------------------------------
bool CDocument::SaveDocument(const std::string& fileName)
{
	if (m_fileWriter)
	{
		bool b = m_fileWriter->Write(fileName.c_str());
		if (b) SetDocFilePath(fileName);
		return b;
	}
	else return false;
}

//-----------------------------------------------------------------------------
bool CDocument::SaveDocument()
{
	return m_fileWriter->Write(m_filePath.c_str());
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
// import image data
Post::CImageModel* CDocument::ImportImage(const std::string& fileName, int nx, int ny, int nz, BOX box)
{
	static int n = 1;

	// we pass the relative path to the image model
	string relFile = FSDir::makeRelative(fileName, "$(ProjectDir)");

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

int CDocument::ImageModels() const
{
	return (int)m_img.Size();
}

void CDocument::AddImageModel(Post::CImageModel* img)
{
	assert(img);
	m_img.Add(img);
}

Post::CImageModel* CDocument::GetImageModel(int i)
{
	return m_img[i];
}

void CDocument::DeleteAllImageModels()
{
	m_img.Clear();
}
