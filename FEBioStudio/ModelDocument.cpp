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

#include "stdafx.h"
#include "ModelDocument.h"
#include "DocTemplate.h"
#include "version.h"
#include <MeshIO/PRVObjectFormat.h>
#include <FEMLib/FEUserMaterial.h>
#include <PostLib/VolumeRenderer.h>
#include "Commands.h"
#include "MainWindow.h"
#include "ImageSliceView.h"
#include "ModelFileWriter.h"
#include <QMessageBox>
#include <GeomLib/GModel.h>
#include <PostGL/GLPlot.h>
#include <FEBio/FEBioImport.h>
#include <FEBioLink/FEBioInit.h>
#include <FEBioLink/FEBioClass.h>
#include "GLModelScene.h"
#include "units.h"
#include "GLHighlighter.h"
#include <QJsonDocument>
#include "DocManager.h"

class CModelContext
{
public:
	CModelContext(CModelDocument* doc) : m_doc(doc) 
	{
		m_objs = 0;
		m_parts = 0;
		m_faces = 0;
		m_edges = 0;
		m_nodes = 0;
		m_steps = 0;
		m_mats = 1;	// needs to be initialized to 1

		m_activeItem = nullptr;
	}

	void Push()
	{
		assert(m_doc == CDocument::GetActiveDocument());
		m_objs = GObject::GetCounter();
		m_parts = GPart::GetCounter();
		m_faces = GFace::GetCounter();
		m_edges = GEdge::GetCounter();
		m_nodes = GNode::GetCounter();
		m_steps = FSStep::GetCounter();
		m_mats = GMaterial::GetCounter();
	}

	void Pull()
	{
		GObject::SetCounter(m_objs);
		GPart::SetCounter(m_parts);
		GFace::SetCounter(m_faces);
		GEdge::SetCounter(m_edges);
		GNode::SetCounter(m_nodes);
		FSStep::SetCounter(m_steps);
		GMaterial::SetCounter(m_mats);
	}

	void SetActiveItem(FSObject* po) { m_activeItem = po; }
	FSObject* GetActiveItem() { return m_activeItem; }

private:
	CModelDocument*	m_doc;

	// counters
	int	m_objs;
	int	m_parts;
	int	m_faces;
	int m_edges;
	int m_nodes;
	int m_steps;
	int m_mats;

	// active item in model tree
	FSObject* m_activeItem;
};

CModelDocument::~CModelDocument()
{
	delete m_context;
	m_context = nullptr;

	delete m_scene;
	m_scene = nullptr;
}

CModelDocument::CModelDocument(CMainWindow* wnd) : CGLDocument(wnd)
{
	SetIcon(":/icons/FEBioStudio.png");

	m_context = new CModelContext(this);

	m_scene = new CGLModelScene(this);
	m_scene->SetEnvironmentMap(wnd->GetEnvironmentMap());

	SetFileWriter(new CModelFileWriter(this));

	QObject::connect(this, SIGNAL(selectionChanged()), wnd, SLOT(on_selectionChanged()));
}

void CModelDocument::Clear()
{
	CGLDocument::Clear();

	// reset the project
	m_Project.Reset();

	// clear all the jobs
	m_JobList.Clear();

	// reset selection
	if (m_psel) delete m_psel;
	m_psel = 0;
}

//-----------------------------------------------------------------------------
void CModelDocument::Activate()
{
	CGLDocument::Activate();

	m_context->Pull();

	// reset active module
	FEBio::SetActiveProject(&m_Project);
}

void CModelDocument::Deactivate()
{
	m_context->Push();
	FEBio::SetActiveProject(nullptr);
}

//-----------------------------------------------------------------------------
void CModelDocument::SetActiveItem(FSObject* po)
{
	m_context->SetActiveItem(po);
}

//-----------------------------------------------------------------------------
FSObject* CModelDocument::GetActiveItem()
{
	return m_context->GetActiveItem();
}

//-----------------------------------------------------------------------------
//! Get the project
FSProject& CModelDocument::GetProject()
{ 
	return m_Project; 
}

//-----------------------------------------------------------------------------
FSModel* CModelDocument::GetFSModel()
{ 
	return &m_Project.GetFSModel(); 
}

//-----------------------------------------------------------------------------
GModel* CModelDocument::GetGModel()
{
	return &m_Project.GetFSModel().GetModel();
}

//-----------------------------------------------------------------------------
// return the active object
GObject* CModelDocument::GetActiveObject()
{
	GObject* po = nullptr;
	GObjectSelection sel(GetGModel());
	if (sel.Count() == 1) po = sel.Object(0);
	return po;
}

//-----------------------------------------------------------------------------
BOX CModelDocument::GetModelBox() 
{ 
	BOX box = m_Project.GetFSModel().GetModel().GetBoundingBox(); 

	// add any image models
	for (int i = 0; i < ImageModels(); ++i)
	{
		box += GetImageModel(i)->GetBoundingBox();
	}

	return box;
}

int CModelDocument::GetMeshMode()
{
	return (m_wnd ? m_wnd->GetMeshMode() : MESH_MODE_VOLUME);
}

void CModelDocument::Update()
{
	GetGModel()->UpdateBoundingBox();
	CGLDocument::Update();
}

std::string CModelDocument::GetRenderString()
{
	FSModel* ps = GetFSModel();
	GModel& model = ps->GetModel();
	int activeLayer = model.GetActiveMeshLayer();
	string s = string("  Mesh Layer > ") + model.GetMeshLayerName(activeLayer);
	return s;
}

void CModelDocument::AddObject(GObject* po)
{
	DoCommand(new CCmdAddAndSelectObject(GetGModel(), po), po->GetNameAndType());
	Update();
}

void CModelDocument::DeleteObjects(std::vector<FSObject*> objList)
{
	// get all the parts out since we don't want to process those one by one
	std::vector<GPart*> partList;
	for (int i = 0; i < objList.size(); ++i)
	{
		GPart* pg = dynamic_cast<GPart*>(objList[i]);
		if (pg)
		{
			partList.push_back(pg);
			objList[i] = nullptr;
		}
	}

	if (!partList.empty())
	{
		GetGModel()->DeleteParts(partList);
	}

	for (int i = 0; i < (int)objList.size(); ++i)
	{
		if (objList[i]) DeleteObject(objList[i]);
	}
}

void CModelDocument::DeleteObject(FSObject* po)
{
	FSModel& fem = *GetFSModel();

	if (po == GetActiveItem()) SetActiveItem(nullptr);

	if (dynamic_cast<FSStep*>(po))
	{
		if (dynamic_cast<FSInitialStep*>(po))
		{
			QMessageBox::warning(m_wnd, "Delete step", "Cannot delete the initial step.");
			return;
		}
		else
		{
			DoCommand(new CCmdDeleteFSObject(po));
		}
	}
	else if (dynamic_cast<FSMaterial*>(po))
	{
		FSMaterial* pm = dynamic_cast<FSMaterial*>(po);
		FSMaterial* parent = const_cast<FSMaterial*>(pm->GetParentMaterial());
		FSProperty* pp = parent->FindProperty(pm);
		if (pp)// && (pp->maxSize() == 0))
		{
			pp->RemoveComponent(pm);
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
	else if (dynamic_cast<GObject*>(po))
	{
		GObject* obj = dynamic_cast<GObject*>(po);

		// check to see if there are any required nodes
		if (obj->CanDelete() == false)
		{
			QMessageBox::warning(m_wnd, "FEBio Studio", "This object cannot be deleted since other model components depend on it.");
			return;
		}

		DoCommand(new CCmdDeleteGObject(GetGModel(), obj), obj->GetName());
	}
    else if (dynamic_cast<CImageModel*>(po))
    {
        auto img = dynamic_cast<CImageModel*>(po);
        RemoveImageModel(img);
        delete img;

        // We don't handle image models on the command stack so that 
        // image deletion actually clears up ram
        ClearCommandStack();
    }
    else if (dynamic_cast<CImageAnalysis*>(po))
    {
        DoCommand(new CCmdDeleteImageAnalysis(dynamic_cast<CImageAnalysis*>(po)));
    }
	else if (po->GetParent())
	{
		if (dynamic_cast<CFEBioJob*>(po))
		{
			CFEBioJob* job = dynamic_cast<CFEBioJob*>(po);
/*			if (job->GetPostDoc())
			{
				m_wnd->CloseView(job->GetPostDoc());
			}
*/
		}

		if (dynamic_cast<FSLoadController*>(po))
		{
			FSLoadController* plc = dynamic_cast<FSLoadController*>(po);
			if (plc->GetReferenceCount() > 0)
				QMessageBox::warning(m_wnd, "FEBio Studio", "This load controller cannot be deleted since other model components are using it.");
			else
				DoCommand(new CCmdDeleteFSModelComponent(dynamic_cast<FSModelComponent*>(po)), po->GetName());
		}
		else if (dynamic_cast<FSModelComponent*>(po))
			DoCommand(new CCmdDeleteFSModelComponent(dynamic_cast<FSModelComponent*>(po)), po->GetName());
		else
			DoCommand(new CCmdDeleteFSObject(po));
	}
	else if (dynamic_cast<FEMeshData*>(po))
	{
		FEMeshData* pd = dynamic_cast<FEMeshData*>(po);
		DoCommand(new CCmdRemoveMeshData(pd));
	}
	else
	{
		assert(false);
	}

	SetModifiedFlag(true);
}

//-----------------------------------------------------------------------------
int CModelDocument::FEBioJobs() const
{
	return (int)m_JobList.Size();
}

//-----------------------------------------------------------------------------
void CModelDocument::AddFEbioJob(CFEBioJob* job)
{
	m_JobList.Add(job);
	SetModifiedFlag();
}

CFEBioJob* CModelDocument::GetFEBioJob(int i)
{
	return m_JobList[i];
}

CFEBioJob* CModelDocument::FindFEBioJob(const std::string& s)
{
	for (int i = 0; i < FEBioJobs(); ++i)
	{
		CFEBioJob* job = m_JobList[i];
		if (job->GetName() == s) return job;
	}

	return nullptr;
}

void CModelDocument::DeleteAllJobs()
{
	m_JobList.Clear();
}

//-----------------------------------------------------------------------------
void CModelDocument::Save(OArchive& ar)
{
	std::string sfile = GetDocFileName();
	AppendChangeLog(QString("saved model to %1").arg(QString::fromStdString(sfile)));

	// save version info
	unsigned int version = SAVE_VERSION;
	ar.WriteChunk(CID_VERSION, version);

	// save model info
	ar.BeginChunk(CID_MODELINFO);
	{
		ar.WriteChunk(CID_MODELINFO_COMMENT, m_info);
		ar.WriteChunk(CID_MODELINFO_UNITS  , m_units);

		const ChangeLog& log = GetChangeLog();
		QString txt = log.toJson();
		string s = txt.toStdString();
		if (s.empty() == false) ar.WriteChunk(CID_MODELINFO_CHANGELOG, s);
	}
	ar.EndChunk();

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
void CModelDocument::Load(IArchive& ar)
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
			if (version < MIN_FSM_VERSION)
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
				else if (nid == CID_MODELINFO_UNITS)
				{
					nret = ar.read(m_units);
				}
				else if (nid == CID_MODELINFO_CHANGELOG)
				{
					string s;
					ar.read(s);
					ChangeLog newLog;
					newLog.fromJson(QString::fromStdString(s));
					SetChangeLog(newLog);
				}

				ar.CloseChunk();
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

bool CModelDocument::Initialize()
{
	// When called after reading an FEBio, the project's units may be different
	// than the model's (which is initialized to the default units). 
	if (m_Project.GetUnits() != 0)
	{
		SetUnitSystem(m_Project.GetUnits());

		if (GetActiveDocument() == this)
		{
			Units::SetUnitSystem(GetUnitSystem());
		}
	}

	return CGLDocument::Initialize();
}

//-----------------------------------------------------------------------------
bool CModelDocument::LoadTemplate(int n)
{
	int N = TemplateManager::Templates();
	if ((n<0) || (n >= N)) return false;
	DocTemplate& doc = TemplateManager::GetTemplate(n);
	return doc.Load(this);
}

std::vector<MODEL_ERROR> CModelDocument::CheckModel()
{
	vector<MODEL_ERROR> errorList;
	checkModel(GetProject(), errorList);
	return errorList;
}

bool CModelDocument::ExportMaterials(const std::string& fileName, const vector<GMaterial*>& matList)
{
	if (matList.size() == 0) return false;

	OArchive ar;
	//									P V M 
	if (ar.Create(fileName.c_str(), 0x0050564D) == false) return false;

	// save version info
	unsigned int version = PVM_VERSION_NUMBER;
	ar.WriteChunk(PVM_VERSION, version);

	// write materials
	int mats = (int)matList.size();
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

bool CModelDocument::ImportMaterials(const std::string& fileName)
{
	IArchive ar;
	if (ar.Open(fileName.c_str(), 0x0050564D) == false) return false;

	FSModel& fem = *GetFSModel();
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
					FSMaterial* pm = 0;
					if (ntype == FE_USER_MATERIAL) pm = new FSUserMaterial(FE_USER_MATERIAL, &fem);
					else pm = FEMaterialFactory::Create(&fem, ntype);

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

bool CModelDocument::ImportFEBioMaterials(const std::string& fileName)
{
	if (fileName.empty()) return false;
	FEBioFileImport feb(GetProject());
	return feb.ImportMaterials(fileName.c_str());
}

//-----------------------------------------------------------------------------
void CModelDocument::SetUnitSystem(int unitSystem)
{
	CGLDocument::SetUnitSystem(unitSystem);

	FSModel* fem = GetFSModel();
	if (fem)
	{
		// -- update global constants
		// universal gas constant
		const double R = 8.314462618153; // value in SI units
		Param* pR = fem->GetParam("R");
		if (pR) pR->SetFloatValue(Units::Convert(R, UNIT_GAS_CONSTANT, Units::SI, unitSystem));

		// faraday's constant
		const double Fc = 9.648533212331e4;  // value in SI units
		Param* pFc = fem->GetParam("Fc");
		if (pFc) pFc->SetFloatValue(Units::Convert(Fc, UNIT_FARADAY_CONSTANT, Units::SI, unitSystem));

        // reference temperature
        Param* pT = fem->GetParam("T");
        if (pT) pT->SetFloatValue(Units::Convert(pT->GetFloatValue(), UNIT_TEMPERATURE, Units::SI, unitSystem));

        // reference pressure
        Param* pP = fem->GetParam("P");
        if (pP) pP->SetFloatValue(Units::Convert(pP->GetFloatValue(), UNIT_PRESSURE, Units::SI, unitSystem));

	}
}

void CModelDocument::AddImageModel(CImageModel* imgModel)
{
	CGLDocument::AddImageModel(imgModel);

	// Add default image renderers
	CMainWindow* wnd = GetMainWindow();
	if (wnd)
	{
		CImageSliceView* sliceView = wnd->GetImageSliceView();
		CImageSliceViewRender* sr = new CImageSliceViewRender(sliceView);
		sr->SetImageModel(imgModel);
		imgModel->AddImageRenderer(sr);
	}

	Post::CVolumeRenderer* vr = new Post::CVolumeRenderer(imgModel);
	vr->Create();
	imgModel->AddImageRenderer(vr);
}

//-----------------------------------------------------------------------------
// SELECTION
//-----------------------------------------------------------------------------
void CModelDocument::UpdateSelection(bool report)
{
	// delete old selection
	if (m_psel) delete m_psel;
	m_psel = 0;

	FSModel* ps = GetFSModel();

	// figure out if there is a mesh selected
	GObject* po = GetActiveObject();
	FSMesh* pm = (po ? po->GetFEMesh() : 0);
	FSMeshBase* pmb = (po ? po->GetEditableMesh() : 0);
	FSLineMesh* plm = (po ? po->GetEditableLineMesh() : 0);

	// get the mesh mode
	int meshMode = m_wnd->GetMeshMode();
	GModel* gm = &ps->GetModel();

	if (m_vs.nitem == ITEM_MESH)
	{
		switch (m_vs.nselect)
		{
		case SELECT_OBJECT: m_psel = new GObjectSelection(gm); break;
		case SELECT_PART: m_psel = new GPartSelection(gm); break;
		case SELECT_FACE: m_psel = new GFaceSelection(gm); break;
		case SELECT_EDGE: m_psel = new GEdgeSelection(gm); break;
		case SELECT_NODE: m_psel = new GNodeSelection(gm); break;
		case SELECT_DISCRETE: m_psel = new GDiscreteSelection(gm); break;
		}
	}
	else
	{
		if (meshMode == MESH_MODE_VOLUME)
		{
			switch (m_vs.nitem)
			{
			case ITEM_ELEM: if (pm) m_psel = new FEElementSelection(pm); break;
			case ITEM_FACE: if (pm) m_psel = new FEFaceSelection(pm); break;
			case ITEM_EDGE: if (pm) m_psel = new FEEdgeSelection(pm); break;
			case ITEM_NODE: if (pm) m_psel = new FENodeSelection(pm); break;
			}
		}
		else
		{
			switch (m_vs.nitem)
			{
			case ITEM_ELEM: if (pm) m_psel = new FEElementSelection(pm); break;
			case ITEM_FACE: if (pmb) m_psel = new FEFaceSelection(pmb); break;
			case ITEM_EDGE: if (plm) m_psel = new FEEdgeSelection(plm); break;
			case ITEM_NODE: if (plm) m_psel = new FENodeSelection(plm); break;
			}
		}
	}

	CGLModelScene* scene = dynamic_cast<CGLModelScene*>(GetScene());
	if (scene) scene->UpdateSelectionMesh(m_psel);

	// update the window's toolbar to make sure it reflects the correct
	// selection tool
	if (report)
	{
		emit selectionChanged();
	}
}

//-----------------------------------------------------------------------------
void CModelDocument::HideCurrentSelection()
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
				DoCommand(new CCmdHideSelection(this));
			}
		}
	}
	else
	{
		// Hide sub-object (i.e. mesh) level items
		FESelection* ps = GetCurrentSelection();
		if (ps && ps->Size())
		{
			DoCommand(new CCmdHideSelection(this));
		}
	}
}

void CModelDocument::HideUnselected()
{
	int itemMode = GetItemMode();
	if (itemMode == ITEM_MESH)
	{
		int selMode = GetSelectionMode();
		if (selMode == SELECT_OBJECT)
		{
			GModel* mdl = GetGModel();
			vector<GObject*> po;
			for (int i = 0; i<mdl->Objects(); ++i)
				if (mdl->Object(i)->IsSelected() == false) po.push_back(mdl->Object(i));

			DoCommand(new CCmdHideObject(po, true));
		}
		else if (selMode == SELECT_PART)
		{
			GModel* mdl = GetGModel();
			std::list<GPart*> partList;
			for (int i = 0; i<mdl->Objects(); ++i)
			{
				GObject* po = mdl->Object(i);
				for (int j = 0; j<po->Parts(); ++j)
				{
					GPart* part = po->Part(j);
					if (part->IsSelected() == false) partList.push_back(part);
				}
			}

			DoCommand(new CCmdHideParts(mdl, partList));
		}
	}
	else
	{
		GObject* po = GetActiveObject();
		if (po == 0) return;

		if (itemMode == ITEM_ELEM)
		{
			FSMesh* pm = po->GetFEMesh();
			if (pm == 0) return;

			vector<int> elemList;
			for (int i = 0; i<pm->Elements(); ++i)
			{
				FSElement& el = pm->Element(i);
				if (el.IsSelected() == false) elemList.push_back(i);
			}
			DoCommand(new CCmdHideElements(po, elemList));
		}
		else if (itemMode == ITEM_FACE)
		{
			FSSurfaceMesh* pm = dynamic_cast<FSSurfaceMesh*>(po->GetEditableMesh());
			if (pm == 0) return;

			vector<int> faceList;
			for (int i = 0; i<pm->Faces(); ++i)
			{
				FSFace& face = pm->Face(i);
				if (face.IsSelected() == false) faceList.push_back(i);
			}
			DoCommand(new CCmdHideFaces(pm, faceList));
		}
	}
}

void CModelDocument::SelectItems(FSObject* po, const std::vector<int>& l, int n)
{
	GModel* mdl = GetGModel();
	FSModel* ps = GetFSModel();

	// create the selection command
	FEItemListBuilder* pl = 0;

	IHasItemLists* phs = dynamic_cast<IHasItemLists*>(po);
	if (phs) pl = phs->GetItemList(n);

	GGroup* pg = dynamic_cast<GGroup*>(po);
	if (pg) pl = pg;

	FSGroup* pf = dynamic_cast<FSGroup*>(po);
	if (pf) pl = pf;

	CCommand* pcmd = 0;
	if (pl)
	{
		switch (pl->Type())
		{
		case GO_NODE: SetSelectionMode(SELECT_NODE); pcmd = new CCmdSelectNode(mdl, l, false); break;
		case GO_EDGE: SetSelectionMode(SELECT_EDGE); pcmd = new CCmdSelectEdge(mdl, l, false); break;
		case GO_FACE: SetSelectionMode(SELECT_FACE); pcmd = new CCmdSelectSurface(mdl, l, false); break;
		case GO_PART: SetSelectionMode(SELECT_PART); pcmd = new CCmdSelectPart(mdl, l, false); break;
		default:
			if (dynamic_cast<FSGroup*>(pl))
			{
				SetSelectionMode(SELECT_OBJECT);
				FSGroup* pg = dynamic_cast<FSGroup*>(pl);
				FSMesh* pm = dynamic_cast<FSMesh*>(pg->GetMesh());
				assert(pm);
				switch (pg->Type())
				{
				case FE_NODESET: SetItemMode(ITEM_NODE); pcmd = new CCmdSelectFENodes(pm, l, false); break;
				case FE_EDGESET: SetItemMode(ITEM_EDGE); pcmd = new CCmdSelectFEEdges(pm, l, false); break;
				case FE_SURFACE: SetItemMode(ITEM_FACE); pcmd = new CCmdSelectFaces(pm, l, false); break;
				case FE_ELEMSET: SetItemMode(ITEM_ELEM); pcmd = new CCmdSelectElements(pm, l, false); break;
				case FE_PARTSET: 
				{
					FSPartSet* pg = dynamic_cast<FSPartSet*>(pl);
					if (pg)
					{
						vector<int> elemList = pg->BuildElementIndexList(l);
						SetItemMode(ITEM_ELEM); pcmd = new CCmdSelectElements(pm, elemList, false);
					}
				}
				break;
				default:
					assert(false);
				}

				// make sure the parent object is selected
				GObject* po = pm->GetGObject();
				assert(po);
				if (po && !po->IsSelected())
				{
					CCmdGroup* pgc = new CCmdGroup("Select");
					pgc->AddCommand(new CCmdSelectObject(mdl, po, false));
					pgc->AddCommand(pcmd);
					pcmd = pgc;
				}
			}
		}
	}
	else if (dynamic_cast<GMaterial*>(po))
	{
		SetSelectionMode(SELECT_PART);
		pcmd = new CCmdSelectPart(mdl, l, false);
	}
	else if (dynamic_cast<GDiscreteElementSet*>(po))
	{
		SetSelectionMode(SELECT_DISCRETE);
		GDiscreteElementSet* ds = dynamic_cast<GDiscreteElementSet*>(po);
		pcmd = new CCmdSelectDiscreteElements(ds, l, false);
	}

	// execute command
	if (pcmd)
	{
		DoCommand(pcmd);
		m_wnd->UpdateToolbar();
		m_wnd->Update();
	}
}

//-----------------------------------------------------------------------------
bool CModelDocument::ImportGeometry(FSFileImport* preader, const char *szfile)
{
	ClearCommandStack();

	// try to load the file
	if (preader->Load(szfile) == false) return false;

	// set the modified flag
	m_bModified = true;

	return true;
}

// helper function for applying a modifier
bool CModelDocument::ApplyFEModifier(FEModifier& modifier, GObject* po, FESelection* sel, bool clearSel)
{
	// get the mesh
	FSMesh* pm = po->GetFEMesh();
	if (pm == 0) return false;

	// apply modifier and create new mesh
	FSMesh* newMesh = 0;
	try {
		if (sel && (sel->Type() != SELECT_OBJECTS))
			newMesh = modifier.Apply(po, sel);
		else
			newMesh = modifier.Apply(pm);
	}
	catch (...)
	{
		modifier.SetError("Exception detected.");
		return false;
	}

	// make sure the modifier succeeded
	if ((newMesh == nullptr) && !modifier.AllowNullMesh()) return false;

	// clear the selection
	if (clearSel && newMesh) newMesh->ClearFaceSelection();

	// swap the meshes
	string ss = modifier.GetName();
	return DoCommand(new CCmdChangeFEMesh(po, newMesh), ss.c_str(), false);
}

bool CModelDocument::ApplyFESurfaceModifier(FESurfaceModifier& modifier, GSurfaceMeshObject* po, FSGroup* sel)
{
	// get the surface mesh
	FSSurfaceMesh* mesh = po->GetSurfaceMesh();
	if (mesh == 0) return false;

	// create a new mesh
	FSSurfaceMesh* newMesh = 0;
	try {
		newMesh = modifier.Apply(mesh, sel);
	}
	catch (...)
	{
		modifier.SetError("Exception detected.");
		return false;
	}

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
	return DoCommand(cmd, po->GetName(), false);
}

template <class T> std::vector<T*> itemlist_cast(std::vector<GLHighlighter::Item>& items)
{
	std::vector<T*> castedItems;
	for (GLHighlighter::Item& it : items)
	{
		T* tp = dynamic_cast<T*>(it.item);
		if (tp) castedItems.push_back(tp);
	}
	return castedItems;
}

bool CModelDocument::SelectHighlightedItems()
{
	int selectMode = GetSelectionMode();

	std::vector<GLHighlighter::Item> items = GLHighlighter::GetItems();
	if (items.empty()) return false;

	if (selectMode == SelectionMode::SELECT_PART)
	{
		std::vector<GPart*> partList = itemlist_cast<GPart>(items);
		if (!partList.empty())
		{
			vector<int> partIDs;
			for (GPart* p : partList) partIDs.push_back(p->GetID());
			DoCommand(new CCmdSelectPart(GetGModel(), partIDs, false));
		}
		else return false;
	}
	else if (selectMode == SelectionMode::SELECT_FACE)
	{
		std::vector<GFace*> faceList = itemlist_cast<GFace>(items);
		if (!faceList.empty())
		{
			vector<int> faceIDs;
			for (GFace* f : faceList) faceIDs.push_back(f->GetID());
			DoCommand(new CCmdSelectSurface(GetGModel(), faceIDs, false));
		}
		else return false;
	}
	else if (selectMode == SelectionMode::SELECT_EDGE)
	{
		std::vector<GEdge*> edgeList = itemlist_cast<GEdge>(items);
		if (!edgeList.empty())
		{
			vector<int> edgeIDs;
			for (GEdge* e : edgeList) edgeIDs.push_back(e->GetID());
			DoCommand(new CCmdSelectEdge(GetGModel(), edgeIDs, false));
		}
		else return false;
	}
	else if (selectMode == SelectionMode::SELECT_NODE)
	{
		std::vector<GNode*> nodeList = itemlist_cast<GNode>(items);
		if (!nodeList.empty())
		{
			vector<int> nodeIDs;
			for (GNode* n : nodeList) nodeIDs.push_back(n->GetID());
			DoCommand(new CCmdSelectNode(GetGModel(), nodeIDs, false));
		}
		else return false;
	}
	else return false;

	return true;
}

void CModelDocument::ToggleActiveParts()
{
	if (GetSelectionMode() != SELECT_PART) return;

	GObject* po = GetActiveObject();
	if (po == nullptr) return;

	vector<GPart*> selectedParts;
	for (int i = 0; i < po->Parts(); ++i)
	{
		GPart* pg = po->Part(i);
		if (pg && pg->IsSelected()) selectedParts.push_back(pg);
	}
	if (selectedParts.empty()) return;

	DoCommand(new CCmdToggleActiveParts(selectedParts));
}

CModelDocument* CreateNewModelDocument(CMainWindow* wnd, int moduleID, std::string name, int units)
{
	CModelDocument* doc = new CModelDocument(wnd);
	doc->GetProject().SetModule(moduleID);

	CDocManager* dm = wnd->GetDocManager();

	if (name.size() == 0) name = dm->GenerateNewDocName();
	doc->SetDocTitle(name);

	if (units == -1) units = wnd->GetDefaultUnitSystem();
	doc->SetUnitSystem(units);
	return doc;
}

CModelDocument* CreateDocumentFromTemplate(CMainWindow* wnd, int templateID, std::string name, int units)
{
	CModelDocument* doc = new CModelDocument(wnd);
	if (doc->LoadTemplate(templateID) == false)
	{
		delete doc;
		return nullptr;
	}
	doc->SetDocTitle(name);
	doc->SetUnitSystem(units);
	return doc;
}
