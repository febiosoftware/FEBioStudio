/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
#include <MeshTools/FENodeData.h>
#include <MeshTools/FESurfaceData.h>
#include <MeshTools/FEElementData.h>
#include <FSCore/FSDir.h>
#include <QtCore/QDir>
#include <QFileInfo>
#include <QDateTime>
#include "Commands.h"
#include "FEBioJob.h"
#include "Logger.h"
#include <sstream>
#include <QTextStream>

using std::stringstream;

#ifdef HAS_TEEM
#include <ImageLib/compatibility.h>
#endif

// defined in MeshTools\GMaterial.cpp
extern GLColor col[];

void VIEW_SETTINGS::Defaults(int ntheme)
{
	m_bgrid = true;
	m_bmesh = true;
	m_bfeat = true;
	m_bnorm = false;
	m_nrender = RENDER_SOLID;
	m_scaleNormals = 1.0;
//	m_nconv = 0; // Don't reset this, since this is read from settings file. TODO: Put this option elsewhere. 

	m_bjoint = true;
	m_bwall = true;
	m_brigid = true;
	m_bfiber = false;
	m_bcontour = false;
	m_blma = false;
	m_fiber_scale = 1.0;
	m_showHiddenFibers = false;
	m_showDiscrete = true;
	m_showRigidLabels = false;

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

	if (ntheme == 0)
	{
		m_col1 = GLColor(255, 255, 255);
		m_col2 = GLColor(128, 128, 255);
		m_nbgstyle = BG_HORIZONTAL;
	}
	else
	{
		m_col1 = GLColor(83, 83, 83);
		m_col2 = GLColor(128, 128, 128);
		m_nbgstyle = BG_HORIZONTAL;
	}

	m_mcol = GLColor(0, 0, 128);
	m_fgcol = GLColor(0, 0, 0);
	m_node_size = 7.f;
	m_line_size = 1.0f;
	m_bline_smooth = true;
	m_bpoint_smooth = true;
	m_bzsorting = true;

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

//==============================================================================
// CDocument
//==============================================================================

CDocument* CDocument::m_activeDoc = nullptr;

CDocument::CDocument(CMainWindow* wnd) : m_wnd(wnd)
{
	// set document as not modified
	m_bModified = false;
	m_bValid = true;

	// reset the filename
	m_filePath.clear();
	m_autoSaveFilePath.clear();
}

CDocument* CDocument::GetActiveDocument()
{
	return m_activeDoc;
}

void CDocument::SetActiveDocument(CDocument* doc)
{
	if (doc == m_activeDoc) return;

	if (m_activeDoc) m_activeDoc->Deactivate();
	m_activeDoc = doc;
	if (doc) m_activeDoc->Activate();
}

//-----------------------------------------------------------------------------
CDocument::~CDocument()
{
	// make sure it's not the active doc
	if (GetActiveDocument() == this) SetActiveDocument(nullptr);

	// remove all observers
	for (int i = 0; i < m_Observers.size(); ++i)
		m_Observers[i]->DocumentDelete();
	m_Observers.clear();
}

//-----------------------------------------------------------------------------
void CDocument::Clear()
{
	// reset the filename
	m_filePath.clear();
	m_autoSaveFilePath.clear();

	// set document as not modified
	m_bModified = false;
	m_bValid = true;
}

//-----------------------------------------------------------------------------
bool CDocument::Initialize()
{
	return true;
}

//-----------------------------------------------------------------------------
// will be called when the document is activated
void CDocument::Activate()
{

}

// will be called when the document is deactivate
void CDocument::Deactivate()
{

}

//-----------------------------------------------------------------------------
CMainWindow* CDocument::GetMainWindow() 
{ 
	return m_wnd; 
}

//-----------------------------------------------------------------------------
bool CDocument::IsModified()
{
	return m_bModified;
}

//-----------------------------------------------------------------------------
void CDocument::SetModifiedFlag(bool bset)
{
	m_bModified = bset;
}

//-----------------------------------------------------------------------------
bool CDocument::IsValid() { return m_bValid; }

//-----------------------------------------------------------------------------
void CDocument::AddObserver(CDocObserver* observer)
{
	// no duplicates allowed
	for (int i = 0; i < m_Observers.size(); ++i)
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
	for (int i = 0; i < m_Observers.size(); ++i)
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

	for (int i = 0; i < m_Observers.size(); ++i)
	{
		CDocObserver* observer = m_Observers[i];
		if (observer) observer->DocumentUpdate(bnew);
	}
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

	SetAutoSaveFilePath();
}

//-----------------------------------------------------------------------------
std::string CDocument::GetAutoSaveFilePath()
{
	return m_autoSaveFilePath;
}

//-----------------------------------------------------------------------------
void CDocument::SetAutoSaveFilePath()
{
	//Construct the new autosave file path
	FSDir fsDir(m_filePath);
	QString newAutoSave = QString("%1/~%2_auto.%3").arg(fsDir.fileDir().c_str()).arg(fsDir.fileBase().c_str()).arg(fsDir.fileExt().c_str());

	// If an old autosave file exists, rename it
	QFile oldAutoSaveFile(m_autoSaveFilePath.c_str());
	if (oldAutoSaveFile.exists()) oldAutoSaveFile.rename(newAutoSave);

	// Set new autosave file path
	m_autoSaveFilePath = newAutoSave.toStdString();

}

void CDocument::SetUnsaved()
{
	m_filePath.clear();
	m_autoSaveFilePath.clear();

	SetModifiedFlag(true);
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
	SetDocFilePath(fileName);
	bool b = SaveDocument();
	if (b) SetModifiedFlag(false);
	return b;
}

//-----------------------------------------------------------------------------
bool CDocument::SaveDocument()
{
	return false;
}

//-----------------------------------------------------------------------------
bool CDocument::AutoSaveDocument()
{
	return true;
}

bool CDocument::loadPriorAutoSave()
{
	QFileInfo autoSaveInfo(m_autoSaveFilePath.c_str());

	if (autoSaveInfo.exists())
	{
		QFileInfo fileInfo(m_filePath.c_str());

		if (autoSaveInfo.lastModified() > fileInfo.lastModified()) return true;
	}

	return false;
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

std::string CDocument::GetIcon() const
{
	return m_iconName;
}

void CDocument::SetIcon(const std::string& iconName)
{
	m_iconName = iconName;
}

//==============================================================================
// CGLDocument
//==============================================================================

//-----------------------------------------------------------------------------
// Construction/Destruction
//-----------------------------------------------------------------------------
CGLDocument::CGLDocument(CMainWindow* wnd) : CDocument(wnd)
{
	m_pCmd = new CCommandManager(this);

	m_fileWriter = nullptr;
	m_fileReader = nullptr;

	// Clear the command history
	m_pCmd->Clear();

	// Set default modes
	m_vs.nselect = SELECT_OBJECT;
	m_vs.ntrans = TRANSFORM_NONE;
	m_vs.nitem = ITEM_MESH;
	m_vs.nstyle = REGION_SELECT_BOX;

	// set default unit system (0 == no unit system)
	m_units = 0;
}

//-----------------------------------------------------------------------------
CGLDocument::~CGLDocument()
{
	// remove autosave
	QFile autoSave(m_autoSaveFilePath.c_str());
	if(autoSave.exists()) autoSave.remove();

	delete m_pCmd;
}

//-----------------------------------------------------------------------------
void CGLDocument::Clear()
{
	CDocument::Clear();

	// Clear the command history
	m_pCmd->Clear();
}

//-----------------------------------------------------------------------------
void CGLDocument::SetUnitSystem(int unitSystem)
{
	m_units = unitSystem;
}

//-----------------------------------------------------------------------------
int CGLDocument::GetUnitSystem() const
{
	return m_units;
}

//-----------------------------------------------------------------------------
// COMMAND FUNCTIONS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
bool CGLDocument::CanUndo() { return m_pCmd->CanUndo(); }

//-----------------------------------------------------------------------------
bool CGLDocument::CanRedo() { return m_pCmd->CanRedo(); }

//-----------------------------------------------------------------------------
void CGLDocument::AddCommand(CCommand* pcmd)
{
	m_pCmd->AddCommand(pcmd);
	SetModifiedFlag();
	UpdateSelection();
	CMainWindow* wnd = GetMainWindow();
	wnd->AddLogEntry(QString("Executing command: %1\n").arg(pcmd->GetName()));
}

//-----------------------------------------------------------------------------
void CGLDocument::AddCommand(CCommand* pcmd, const std::string& s)
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
const char* CGLDocument::GetUndoCmdName() { return m_pCmd->GetUndoCmdName(); }

//-----------------------------------------------------------------------------
const char* CGLDocument::GetRedoCmdName() { return m_pCmd->GetRedoCmdName(); }

//-----------------------------------------------------------------------------
bool CGLDocument::DoCommand(CCommand* pcmd, bool b)
{
	CMainWindow* wnd = GetMainWindow();
	wnd->AddLogEntry(QString("Executing command: %1\n").arg(pcmd->GetName()));
	bool ret = m_pCmd->DoCommand(pcmd);
	SetModifiedFlag();
	if (b) UpdateSelection();
	return ret;
}

//-----------------------------------------------------------------------------
bool CGLDocument::DoCommand(CCommand* pcmd, const std::string& s, bool b)
{
	CMainWindow* wnd = GetMainWindow();
	if (s.empty() == false)
	{
		wnd->AddLogEntry(QString("Executing command: %1 (%2)\n").arg(pcmd->GetName()).arg(QString::fromStdString(s)));
	}
	else wnd->AddLogEntry(QString("Executing command: %1\n").arg(pcmd->GetName()));

	bool ret = m_pCmd->DoCommand(pcmd);
	SetModifiedFlag();
	UpdateSelection(b);
	return ret;
}

//-----------------------------------------------------------------------------
const std::string& CGLDocument::GetCommandErrorString() const
{
	return m_pCmd->GetErrorString();
}

//-----------------------------------------------------------------------------
void CGLDocument::UndoCommand()
{
	string cmdName = m_pCmd->GetUndoCmdName();
	m_pCmd->UndoCommand();
	SetModifiedFlag();
	UpdateSelection();
	CMainWindow* wnd = GetMainWindow();
	wnd->AddLogEntry(QString("Undo last command (%1)\n").arg(QString::fromStdString(cmdName)));
}

//-----------------------------------------------------------------------------
void CGLDocument::RedoCommand()
{
	string cmdName = m_pCmd->GetRedoCmdName();
	m_pCmd->RedoCommand();
	SetModifiedFlag();
	UpdateSelection();
	CMainWindow* wnd = GetMainWindow();
	wnd->AddLogEntry(QString("Redo command (%1)\n").arg(QString::fromStdString(cmdName)));
}

//-----------------------------------------------------------------------------
void CGLDocument::ClearCommandStack()
{
	m_pCmd->Clear();
}

void CGLDocument::UpdateSelection(bool breport)
{

}

//-----------------------------------------------------------------------------
void CGLDocument::SetViewState(VIEW_STATE vs)
{
	m_vs = vs;
	UpdateSelection(false);
//	if (m_wnd) m_wnd->UpdateUI();
}

//-----------------------------------------------------------------------------
GObject* CGLDocument::GetActiveObject()
{
	return nullptr;
}

//-----------------------------------------------------------------------------
CGView* CGLDocument::GetView()
{
	return &m_view;
}

//-----------------------------------------------------------------------------
std::string CGLDocument::GetTypeString(FSObject* po)
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
	else if (dynamic_cast<FEDataMapGenerator*>(po)) return "Data map";
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
	else if (dynamic_cast<FEMeshData*>(po))
	{
		FENodeData* nodeData = dynamic_cast<FENodeData*>(po);
		if (nodeData) return "Node data";

		FESurfaceData* surfData = dynamic_cast<FESurfaceData*>(po);
		if (surfData) return "Surface data";

		FEElementData* elemData = dynamic_cast<FEElementData*>(po);
		if (elemData) return "Element data";

		FEPartData* partData = dynamic_cast<FEPartData*>(po);
		if (partData) return "Element data";

		assert(false);
		return "Mesh data";
	}
	else
	{
		assert(false);
	}

	return "(unknown)";
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

void CGLDocument::SaveResources(OArchive& ar)
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

void CGLDocument::LoadResources(IArchive& ar)
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


#ifdef HAS_TEEM
// Load Tiff Data
Post::CImageModel* CGLDocument::ImportTiff(const std::string& fileName)
{
	static int n = 1;
	// we pass the relative path to the image model
	string relFile = FSDir::makeRelative(fileName, "$(ProjectDir)");

	Post::CImageModel* po = new Post::CImageModel(nullptr);

  // Need to convert relFile to wstring maybe? 
  std::wstring relativeFile = s2ws(relFile);
	if (po->LoadTiffData(relativeFile) == false)
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

Post::CImageModel* CGLDocument::ImportNrrd(const std::string& filename)
{
	static int n = 1;
	// we pass the relative path to the image model
	string relFile = FSDir::makeRelative(filename, "$(ProjectDir)");

	Post::CImageModel* po = new Post::CImageModel(nullptr);

  // Need to convert relFile to wstring maybe? 
  std::wstring relativeFile = s2ws(relFile);
	if (po->LoadNrrdData(relativeFile) == false)
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
#endif

#ifdef HAS_DICOM
Post::CImageModel* CGLDocument::ImportDicom(const std::string& filename)
{
	static int n = 1;
	// we pass the relative path to the image model
	string relFile = FSDir::makeRelative(filename, "$(ProjectDir)");

	Post::CImageModel* po = new Post::CImageModel(nullptr);

	if (po->LoadDicomData(relFile) == false)
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
#endif

//-----------------------------------------------------------------------------
// import image data
Post::CImageModel* CGLDocument::ImportImage(const std::string& fileName, int nx, int ny, int nz, BOX box)
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

int CGLDocument::ImageModels() const
{
	return (int)m_img.Size();
}

void CGLDocument::AddImageModel(Post::CImageModel* img)
{
	assert(img);
	m_img.Add(img);
}

Post::CImageModel* CGLDocument::GetImageModel(int i)
{
	return m_img[i];
}

void CGLDocument::DeleteAllImageModels()
{
	m_img.Clear();
}

// set/get the file reader
void CGLDocument::SetFileReader(FileReader* fileReader)
{
	if (fileReader != m_fileReader)
	{
		if (m_fileReader) delete m_fileReader;
		m_fileReader = fileReader;
	}
}

FileReader* CGLDocument::GetFileReader()
{
	return m_fileReader;
}

// set/get the file writer
void CGLDocument::SetFileWriter(FileWriter* fileWriter)
{
	if (fileWriter != m_fileWriter)
	{
		if (m_fileWriter) delete m_fileWriter;
		m_fileWriter = fileWriter;
	}
}

FileWriter* CGLDocument::GetFileWriter()
{
	return m_fileWriter;
}

//-----------------------------------------------------------------------------
bool CGLDocument::SaveDocument()
{
	if (m_fileWriter && (m_filePath.empty() == false))
	{
		bool success = m_fileWriter->Write(m_filePath.c_str());

		if (success)
		{
			SetModifiedFlag(false);
		}

		return success;
	}
	else
		return false;
}

bool CGLDocument::AutoSaveDocument()
{
	if (m_fileWriter && (m_autoSaveFilePath.empty() == false))
	{
		CLogger::AddLogEntry(QString("Autosaving file: %1 ...").arg(m_title.c_str()));

		bool success = m_fileWriter->Write(m_autoSaveFilePath.c_str());

		CLogger::AddLogEntry(success ? "SUCCESS\n" : "FAILED\n");

		return success;
	}
	else
		return false;
}
