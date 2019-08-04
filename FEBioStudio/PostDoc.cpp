#include "stdafx.h"
#include "PostDoc.h"
#include <XPLTLib/xpltFileReader.h>
#include <PostViewLib/FEModel.h>
#include <PostViewLib/GLContext.h>
#include <PostViewLib/GLCamera.h>
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

std::string CPostDoc::GetFieldString()
{
	if (IsValid())
	{
		int nfield = GetGLModel()->GetColorMap()->GetEvalField();
		return GetFEModel()->GetDataManager()->getDataString(nfield, Post::DATA_SCALAR);
	}
	else return "";
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
	imp->glm->GetColorMap()->Activate(true);
	imp->glm->Update(false);
}

bool CPostDoc::Load(const std::string& fileName)
{
	const char* szfile = fileName.c_str();

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

	// create new GLmodel
	imp->glm = new Post::CGLModel(imp->fem);

	imp->m_postObj = new CPostObject(imp->glm);

	imp->m_timeSettings.m_start = 0;
	imp->m_timeSettings.m_end = GetStates() - 1;

	return true;
}

vec3f to_vec3f(const vec3d& r)
{
	return vec3f((float)r.x, (float)r.y, (float)r.z);
}

quat4f to_quat4f(const quatd& q)
{
	return quat4f((float)q.x, (float)q.y, (float) q.z, (float) q.w);
}

bool CPostDoc::IsValid()
{
	return (imp->glm != nullptr);
}

void CPostDoc::Render(CGLView* view)
{
	CGLCamera cam = view->GetCamera();

	// convert PreView camera to PostView camera
	Post::CGLCamera glcam;
	glcam.SetTarget(to_vec3f(cam.Position()));
	glcam.SetLocalTarget(to_vec3f(cam.Target()));
	glcam.SetOrientation(to_quat4f(cam.GetOrientation()));
	glcam.UpdatePosition(true);

	VIEW_SETTINGS& vs = view->GetDocument()->GetViewSettings();

	Post::CGLContext rc;
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

CPostObject::CPostObject(Post::CGLModel* glm) : GObject(CPostObject::POST_OBJECT)
{
	m_glm = glm;

	GFace* face = new GFace(this);
	face->m_nPID[0] = 0;
	AddFace(face);
	AddPart();
	Update();

	BuildMesh();
}

FEMeshBase* CPostObject::GetEditableMesh()
{
	return GetFEMesh();
}

FELineMesh* CPostObject::GetEditableLineMesh()
{
	return GetFEMesh();
}

void CPostObject::BuildGMesh()
{
	Post::FEModel* fem = m_glm->GetFEModel();
	if (fem == nullptr) return;

	Post::FEMeshBase* mesh = fem->GetFEMesh(0);

	int facets = 0;
	int NF = mesh->Faces();
	int NN = mesh->Nodes();
	vector<int> tag(NN, -1);
	for (int i = 0; i < NF; ++i)
	{
		const Post::FEFace& face = mesh->Face(i);
		
		if (face.Nodes() == 3) facets++;
		else if (face.Nodes() == 4) facets += 2;
		else assert(false);

		for (int j = 0; j < face.Nodes(); ++j)
		{
			tag[face.node[j]] = 1;
		}
	}
	int surfaceNodes = 0;
	for (int i = 0; i < NN; ++i)
	{
		if (tag[i] > 0)
		{
			tag[i] = surfaceNodes++;
		}
	}

	if (m_pGMesh) delete m_pGMesh;
	m_pGMesh = new GLMesh;
	m_pGMesh->Create(surfaceNodes, facets);

	for (int i = 0; i < NN; ++i)
	{
		if (tag[i] >= 0)
		{
			Post::FENode& node = mesh->Node(i);
			vec3d& r = m_pGMesh->Node(tag[i]).r;
			r.x = node.m_rt.x;
			r.y = node.m_rt.y;
			r.z = node.m_rt.z;
		}
	}

	facets = 0;
	for (int i = 0; i < NF; ++i)
	{
		const Post::FEFace& face = mesh->Face(i);
		if (face.Nodes() == 3)
		{
			GMesh::FACE& gf = m_pGMesh->Face(facets++);
			gf.n[0] = tag[face.node[0]];
			gf.n[1] = tag[face.node[1]];
			gf.n[2] = tag[face.node[2]];
		}
		else if (face.Nodes() == 4)
		{
			GMesh::FACE& gf1 = m_pGMesh->Face(facets++);
			gf1.n[0] = tag[face.node[0]];
			gf1.n[1] = tag[face.node[1]];
			gf1.n[2] = tag[face.node[2]];

			GMesh::FACE& gf2 = m_pGMesh->Face(facets++);
			gf2.n[0] = tag[face.node[2]];
			gf2.n[1] = tag[face.node[3]];
			gf2.n[2] = tag[face.node[0]];
		}
	}

	m_pGMesh->Update();
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
		Post::FEFace& fs = postMesh->Face(i);

		switch (fs.m_ntype)
		{
		case Post::FACE_QUAD4: fd.m_type = FE_FACE_QUAD4; break;
		case Post::FACE_TRI3 : fd.m_type = FE_FACE_TRI3; break;
		default:
			assert(false);
		}

		fd.m_gid = fs.m_nsg;
		fd.m_sid = fs.m_nsg;

		for (int j = 0; j < fs.Nodes(); ++j)
		{
			fd.n[j] = fs.node[j];
		}
	}

	for (int i = 0; i < NC; ++i)
	{
		FEEdge& ed = mesh->Edge(i);
		Post::FEEdge& es = postMesh->Edge(i);

		ed.m_gid = 0;
		ed.m_type = FE_EDGE2;

		for (int j = 0; j < es.Nodes(); ++j)
		{
			ed.n[j] = es.node[j];
		}
	}

	for (int i = 0; i < NE; ++i)
	{
		FEElement& ed = mesh->Element(i);
		Post::FEElement& es = postMesh->Element(i);

		ed.m_gid = 0;

		switch(es.Type())
		{
		case Post::FE_HEX8: ed.SetType(FE_HEX8); break;
		};

		for (int j = 0; j < es.Nodes(); ++j) ed.m_node[j] = es.m_node[j];
	}

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
