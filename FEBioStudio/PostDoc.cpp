#include "stdafx.h"
#include "PostDoc.h"
#include <XPLTLib/xpltFileReader.h>
#include <PostLib/FEModel.h>
#include <GLLib/GLContext.h>
#include <GLLib/GLCamera.h>
#include <PostLib/GView.h>
#include <PostLib/Palette.h>
#include <PostGL/GLModel.h>
#include <PostGL/GLPlot.h>
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
		Post::CGLModel* glmLocal = glm;
		glm = nullptr;
		delete glmLocal;
		delete fem; fem = nullptr;
		delete m_postObj; m_postObj = nullptr;
	}

public:
	Post::CGLModel*	glm;
	Post::FEModel*	fem;
	Post::CGView	m_view;
	std::string		m_fileName;

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

Post::CGView* CPostDoc::GetView()
{
	return &imp->m_view;
}

void CPostDoc::SetActiveState(int n)
{
	assert(imp->glm);
	imp->glm->SetCurrentTimeIndex(n);
	imp->glm->Update(false);
	imp->m_postObj->UpdateMesh();
}

int CPostDoc::GetActiveState()
{
	return imp->glm->CurrentTimeIndex();
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

void CPostDoc::DeleteObject(Post::CGLObject* po)
{
	Post::CGLPlot* pp = dynamic_cast<Post::CGLPlot*>(po);
	if (pp)
	{
		delete pp;
	}
	else if (dynamic_cast<GLCameraTransform*>(po))
	{
		GLCameraTransform* pt = dynamic_cast<GLCameraTransform*>(po);
		Post::CGView* pview = GetView();
		pview->DeleteKey(pt);
	}
/*	else if (dynamic_cast<Post::CImageModel*>(po))
	{
		Post::CImageModel* img = dynamic_cast<Post::CImageModel*>(po);
		for (int i = 0; i < (int)m_img.size(); ++i)
		{
			if (m_img[i] == img)
			{
				delete img;
				m_img.erase(m_img.begin() + i);
				break;
			}
		}
	}
	else if (dynamic_cast<CGLImageRenderer*>(po))
	{
		CGLImageRenderer* ir = dynamic_cast<CGLImageRenderer*>(po);
		CImageModel* img = ir->GetImageModel();
		img->RemoveRenderer(ir);
	}
*/	else if (dynamic_cast<Post::CGLDisplacementMap*>(po))
	{
		Post::CGLDisplacementMap* map = dynamic_cast<Post::CGLDisplacementMap*>(po);
		Post::CGLModel* m = GetGLModel();
		assert(map == m->GetDisplacementMap());
		m->RemoveDisplacementMap();
		UpdateFEModel(true);
	}
/*	else if (dynamic_cast<Post::CGLVisual*>(po))
	{
		list<Post::CGLVisual*>::iterator it = m_pObj.begin();
		for (int i = 0; i<(int)m_pObj.size(); ++i, ++it)
		{
			Post::CGLVisual* pv = (*it);
			if (pv == po)
			{
				delete pv;
				m_pObj.erase(it);
				break;
			}
		}
	}
*/
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
	if (imp->glm) return imp->glm->CurrentTime();
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

std::string CPostDoc::GetFileName()
{
	return imp->m_fileName;
}

bool CPostDoc::LoadPlotfile(const std::string& fileName, const XPLT_OPTIONS& ops)
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
	xplt.SetReadStateFlag(ops.m_op);
	xplt.SetReadStatesList(ops.m_states);

	if (xplt.Load(*imp->fem, szfile) == false)
	{
		delete imp->fem;
		imp->fem = nullptr;
		return false;
	}

	// set the file name as title
	imp->fem->SetTitle(sztitle);
	imp->m_fileName = sztitle;

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
	CGLCamera& cam = view->GetCamera();

	VIEW_SETTINGS& vs = view->GetDocument()->GetViewSettings();

	CGLContext& rc = view->m_rc;
	rc.m_cam = &cam;
	rc.m_showOutline = vs.m_bfeat;
	rc.m_showMesh = vs.m_bmesh;
	rc.m_q = cam.GetOrientation();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	view->PositionCamera();

	glDisable(GL_CULL_FACE);

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

	// Set the FE mesh and update
	SetFEMesh(glm->GetFEModel()->GetFEMesh(0));
	Update(true);
}

CPostObject::~CPostObject()
{
	// The mesh is owned by the CGLModel
	// so we have to set the mesh to zero here, otherwise the GObject baseclass will try to 
	// delete it as well
	SetFEMesh(nullptr);
}

BOX CPostObject::GetBoundingBox()
{
	FEMesh* mesh = GetFEMesh();
	if (mesh) return mesh->GetBoundingBox();
	else return BOX();
}

// is called whenever the selection has changed
void CPostObject::UpdateSelection()
{
	m_glm->UpdateSelectionLists();
}

void CPostObject::UpdateMesh()
{
	Post::FEPostMesh* postMesh = m_glm->GetActiveState()->GetFEMesh();
	postMesh->UpdateBox();

	if (GetFEMesh() != postMesh)
	{
		SetFEMesh(postMesh);
		BuildGMesh();
	}
	else
	{
		GLMesh* mesh = GetRenderMesh(); assert(mesh);

		for (int i = 0; i < mesh->Nodes(); ++i)
		{
			GMesh::NODE& nd = mesh->Node(i);
			FENode& ns = postMesh->Node(nd.nid);

			nd.r = ns.r;
		}
		mesh->Update();
	}
}
