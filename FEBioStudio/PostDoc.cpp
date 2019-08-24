#include "stdafx.h"
#include "PostDoc.h"
#include <XPLTLib/xpltFileReader.h>
#include <PostLib/FEModel.h>
#include <GLLib/GLContext.h>
#include <GLLib/GLCamera.h>
#include <PostLib/Palette.h>
#include <PostGL/GLModel.h>
#include "GLView.h"
#include "Document.h"

void TIMESETTINGS::Defaults()
{
	m_mode = MODE_FORWARD;
	m_fps = 10.0;
	m_start = 1;
	m_end = 0;	//	has to be set after loading a model
	m_bloop = true;
	m_bfix = false;
	m_inc = 1;
	m_dt = 0.01;
}

class CPostDoc::Imp 
{
public:
	Imp() : fem(nullptr), glm(nullptr), m_postObj(nullptr) { m_timeSettings.Defaults(); }
	~Imp() { delete fem; delete glm; delete m_postObj; }

	void clear()
	{
		delete glm; glm = nullptr;
		delete fem; fem = nullptr;
		delete m_postObj; m_postObj = nullptr;
	}

public:
	Post::CGLModel*	glm;
	Post::FEModel*	fem;

	CPostObject*	m_postObj;

	TIMESETTINGS m_timeSettings;
};

CPostDoc::CPostDoc() : imp(new CPostDoc::Imp)
{
}

CPostDoc::~CPostDoc()
{
	delete imp;
}

int CPostDoc::GetStates()
{
	assert(imp->fem);
	return imp->fem->GetStates();
}

Post::FEModel* CPostDoc::GetFEModel()
{
	return imp->fem;
}

Post::CGLModel* CPostDoc::GetGLModel()
{
	return imp->glm;
}

void CPostDoc::SetActiveState(int n)
{
	assert(imp->glm);
	imp->glm->setCurrentTimeIndex(n);
	imp->glm->Update(false);
	imp->m_postObj->UpdateMesh();
}

int CPostDoc::GetActiveState()
{
	return imp->glm->currentTimeIndex();
}

TIMESETTINGS& CPostDoc::GetTimeSettings()
{
	return imp->m_timeSettings;
}

int CPostDoc::GetEvalField()
{
	if (imp->glm == nullptr) return -1;
	Post::CGLColorMap* pc = imp->glm->GetColorMap();
	if (pc == 0) return -1;

	return pc->GetEvalField();
}

std::string CPostDoc::GetTitle()
{
	if (imp->fem) return imp->fem->GetTitle();
	else return "";
}

void CPostDoc::ActivateColormap(bool bchecked)
{
	Post::CGLModel* po = imp->glm;
	po->GetColorMap()->Activate(bchecked);
	UpdateFEModel();
}

std::string CPostDoc::GetFieldString()
{
	if (IsValid())
	{
		int nfield = GetGLModel()->GetColorMap()->GetEvalField();
		return GetFEModel()->GetDataManager()->getDataString(nfield, Post::DATA_SCALAR);
	}
	else return "";
}

float CPostDoc::GetTimeValue()
{
	if (imp->glm) return imp->glm->currentTime();
	else return 0.f;
}

void CPostDoc::UpdateAllStates()
{
	if (IsValid() == false) return;
	int N = imp->fem->GetStates();
	int ntime = GetActiveState();
	for (int i = 0; i<N; ++i) SetActiveState(i);
	SetActiveState(ntime);
}

void CPostDoc::UpdateFEModel(bool breset)
{
	if (!IsValid()) return;

	// update the model
	if (imp->glm) imp->glm->Update(breset);
}

void CPostDoc::SetDataField(int n)
{
	imp->glm->GetColorMap()->SetEvalField(n);
	imp->glm->Update(false);
}

bool CPostDoc::LoadPlotfile(const std::string& fileName)
{
	const char* szfile = fileName.c_str();

	// extract the file title
	const char* sztitle = 0;
	const char* ch2 = strrchr(szfile, '/');
	if (ch2 == 0)
	{
		ch2 = strrchr(szfile, '\\');
		if (ch2 == 0) ch2 = szfile; else ++ch2;
	}
	else ++ch2;
	sztitle = ch2;

	// clear the post doc
	imp->clear();

	// create new FE model
	imp->fem = new Post::FEModel;

	xpltFileReader xplt;

	if (xplt.Load(*imp->fem, szfile) == false)
	{
		delete imp->fem;
		imp->fem = nullptr;
		return false;
	}

	// set the file name as title
	imp->fem->SetTitle(sztitle);

	// assign material attributes
	const Post::CPalette& pal = Post::CPaletteManager::CurrentPalette();
	ApplyPalette(pal);

	// create new GLmodel
	imp->glm = new Post::CGLModel(imp->fem);

	imp->m_postObj = new CPostObject(imp->glm);

	imp->m_timeSettings.m_start = 0;
	imp->m_timeSettings.m_end = GetStates() - 1;

	return true;
}

bool CPostDoc::IsValid()
{
	return (imp->glm != nullptr);
}

void CPostDoc::ApplyPalette(const Post::CPalette& pal)
{
	int NCOL = pal.Colors();
	int nmat = imp->fem->Materials();
	for (int i = 0; i<nmat; i++)
	{
		GLColor c = pal.Color(i % NCOL);

		Post::FEMaterial& m = *imp->fem->GetMaterial(i);
		m.diffuse = c;
		m.ambient = c;
		m.specular = GLColor(128, 128, 128);
		m.emission = GLColor(0, 0, 0);
		m.shininess = 0.5f;
		m.transparency = 1.f;
	}
}

void CPostDoc::Render(CGLView* view)
{
	CGLCamera cam = view->GetCamera();

	// convert PreView camera to PostView camera
	CGLCamera glcam;
	glcam.SetTarget(to_vec3f(cam.GetPosition()));
	glcam.SetLocalTarget(to_vec3f(cam.Target()));
	glcam.SetOrientation(cam.GetOrientation());
	glcam.Update(true);

	VIEW_SETTINGS& vs = view->GetDocument()->GetViewSettings();

	CGLContext rc;
	rc.m_cam = &glcam;
	rc.m_showOutline = vs.m_bfeat;
	rc.m_showMesh = vs.m_bmesh;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glcam.Transform();

	// match the selection mode
	int selectionMode = Post::SELECT_ELEMS;
	switch (view->GetDocument()->GetItemMode())
	{
	case ITEM_MESH:
	case ITEM_ELEM: selectionMode = Post::SELECT_ELEMS; break;
	case ITEM_FACE: selectionMode = Post::SELECT_FACES; break;
	case ITEM_EDGE: selectionMode = Post::SELECT_EDGES; break;
	case ITEM_NODE: selectionMode = Post::SELECT_NODES; break;
	}
	imp->glm->SetSelectionMode(selectionMode);

	imp->glm->Render(rc);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

CPostObject* CPostDoc::GetPostObject()
{
	return imp->m_postObj;
}

CPostObject::CPostObject(Post::CGLModel* glm) : GMeshObject((FEMesh*)nullptr)
{
	// store the model
	m_glm = glm;

	// build the FE mesh
	BuildMesh();

	// Set the FE mesh and update
	SetFEMesh(GetFEMesh());
	Update(true);
}

FEMeshBase* CPostObject::GetEditableMesh()
{
	return GetFEMesh();
}

FELineMesh* CPostObject::GetEditableLineMesh()
{
	return GetFEMesh();
}

BOX CPostObject::GetBoundingBox()
{
	FEMesh* mesh = GetFEMesh();
	if (mesh) return mesh->GetBoundingBox();
	else return BOX();
}

// build the FEMesh
FEMesh* CPostObject::BuildMesh()
{
	DeleteFEMesh();

	Post::FEMeshBase* postMesh = m_glm->GetFEModel()->GetFEMesh(0);

	int NN = postMesh->Nodes();
	int NE = postMesh->Elements();
	int NF = postMesh->Faces();
	int NC = postMesh->Edges();

	FEMesh* mesh = new FEMesh;
	mesh->SetGObject(this);

	mesh->Create(NN, NE, NF, NC);

	for (int i = 0; i < NN; ++i)
	{
		FENode& nd = mesh->Node(i);
		Post::FENode& ns = postMesh->Node(i);

		nd.r.x = ns.m_rt.x;
		nd.r.y = ns.m_rt.y;
		nd.r.z = ns.m_rt.z;
	}

	for (int i = 0; i < NF; ++i)
	{
		FEFace& fd = mesh->Face(i);
		FEFace& fs = postMesh->Face(i);

		fd.m_type = fs.m_type;

		fd.m_gid = fs.m_sid;
		fd.m_sid = fs.m_sid;

		for (int j = 0; j < fs.Nodes(); ++j)
		{
			fd.n[j] = fs.n[j];
		}
	}

	for (int i = 0; i < NC; ++i)
	{
		FEEdge& ed = mesh->Edge(i);
		FEEdge& es = postMesh->Edge(i);

		ed.m_gid = 0;
		ed.m_type = es.Type();

		for (int j = 0; j < es.Nodes(); ++j)
		{
			ed.n[j] = es.n[j];
		}
	}

	for (int i = 0; i < NE; ++i)
	{
		FEElement& ed = mesh->Element(i);
		FEElement_& es = postMesh->Element(i);

		ed.m_gid = es.m_MatID;
		ed.SetType(es.Type());

		for (int j = 0; j < es.Nodes(); ++j) ed.m_node[j] = es.m_node[j];
	}

	mesh->UpdateElementNeighbors();
	mesh->UpdateFaceElementTable();
	mesh->AutoPartitionSurface();
	mesh->Update();

	ReplaceFEMesh(mesh);

	return mesh;
}

// is called whenever the selection has changed
void CPostObject::UpdateSelection()
{
	// map selection of nodes and elements
	Post::FEMeshBase* postMesh = m_glm->GetFEModel()->GetFEMesh(0);

	FEMesh* mesh = GetFEMesh();
	for (int i = 0; i < mesh->Nodes(); ++i)
	{
		if (mesh->Node(i).IsSelected()) postMesh->Node(i).Select();
		else postMesh->Node(i).Unselect();
	}

	for (int i = 0; i < mesh->Edges(); ++i)
	{
		if (mesh->Edge(i).IsSelected()) postMesh->Edge(i).Select();
		else postMesh->Edge(i).Unselect();
	}

	for (int i = 0; i < mesh->Faces(); ++i)
	{
		if (mesh->Face(i).IsSelected()) postMesh->Face(i).Select();
		else postMesh->Face(i).Unselect();
	}

	for (int i = 0; i < mesh->Elements(); ++i)
	{
		if (mesh->Element(i).IsSelected()) postMesh->Element(i).Select();
		else postMesh->Element(i).Unselect();
	}

	m_glm->UpdateSelectionLists();
}

void CPostObject::UpdateMesh()
{
	Post::FEMeshBase* postMesh = m_glm->GetFEModel()->GetFEMesh(0);
	FEMesh* mesh = GetFEMesh();

	int NN = postMesh->Nodes();
	for (int i = 0; i < NN; ++i)
	{
		FENode& nd = mesh->Node(i);
		Post::FENode& ns = postMesh->Node(i);

		nd.r.x = ns.m_rt.x;
		nd.r.y = ns.m_rt.y;
		nd.r.z = ns.m_rt.z;
	}

	mesh->UpdateNormals();
	mesh->UpdateBox();

	BuildGMesh();
}
