#include "stdafx.h"
#include "PostDocument.h"
#include "ModelDocument.h"
#include "PostObject.h"
#include <FSCore/FSDir.h>
#include <PostLib/FEPostModel.h>
#include <PostLib/Palette.h>
#include <PostGL/GLModel.h>

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

//-----------------------------------------------------------------------------
ModelData::ModelData()
{
	m_isValid = false;
}

//-----------------------------------------------------------------------------
bool ModelData::IsValid() const
{
	return m_isValid;
}

//-----------------------------------------------------------------------------
void ModelData::ReadData(Post::CGLModel* po)
{
	if (po == 0)
	{
		m_isValid = false;
		return;
	}

	// set model props
	m_mdl.m_ntime = po->CurrentTimeIndex();
	m_mdl.m_bnorm = po->m_bnorm;
	m_mdl.m_bghost = po->m_bghost;
	m_mdl.m_bShell2Hex = po->ShowShell2Solid();
	m_mdl.m_nshellref = po->ShellReferenceSurface();
	m_mdl.m_nDivs = po->m_nDivs;
	m_mdl.m_nrender = po->m_nrender;
	m_mdl.m_smooth = po->GetSmoothingAngle();

	// set colormap props
	Post::CGLColorMap* pglmap = po->GetColorMap();
	if (pglmap)
	{
		m_cmap.m_bactive = pglmap->IsActive();
		m_cmap.m_nRangeType = pglmap->GetRangeType();
		m_cmap.m_bDispNodeVals = pglmap->DisplayNodalValues();
		m_cmap.m_nField = pglmap->GetEvalField();
		pglmap->GetRange(m_cmap.m_user);

		Post::CColorTexture* pcm = pglmap->GetColorMap();
		m_cmap.m_ntype = pcm->GetColorMap();
		m_cmap.m_ndivs = pcm->GetDivisions();
		m_cmap.m_bsmooth = pcm->GetSmooth();
		//		pcm->GetRange(m_cmap.m_min, m_cmap.m_max);
	}

	// displacement map
	Post::FEPostModel* ps = po->GetFEModel();
	m_dmap.m_nfield = ps->GetDisplacementField();

	// materials 
	int N = ps->Materials();
	m_mat.resize(N);
	for (int i = 0; i<N; ++i) m_mat[i] = *ps->GetMaterial(i);

	// store the data field strings
	m_data.clear();
	Post::FEDataManager* pDM = ps->GetDataManager();
	Post::FEDataFieldPtr pdf = pDM->FirstDataField();
	for (int i = 0; i<pDM->DataFields(); ++i, ++pdf) m_data.push_back(string((*pdf)->GetName()));

	m_isValid = true;
}

void ModelData::WriteData(Post::CGLModel* po)
{
	if ((m_isValid == false) || (po == nullptr)) return;

	// set model data
	po->m_bnorm = m_mdl.m_bnorm;
	po->m_bghost = m_mdl.m_bghost;
	po->ShowShell2Solid(m_mdl.m_bShell2Hex);
	po->ShellReferenceSurface(m_mdl.m_nshellref);
	po->m_nDivs = m_mdl.m_nDivs;
	po->m_nrender = m_mdl.m_nrender;
	po->SetSmoothingAngle(m_mdl.m_smooth);

	// set color map data
	Post::CGLColorMap* pglmap = po->GetColorMap();
	if (pglmap)
	{
		pglmap->SetRangeType(m_cmap.m_nRangeType);
		pglmap->SetRange(m_cmap.m_user);
		pglmap->DisplayNodalValues(m_cmap.m_bDispNodeVals);
		pglmap->SetEvalField(m_cmap.m_nField);
		pglmap->Activate(m_cmap.m_bactive);

		Post::CColorTexture* pcm = pglmap->GetColorMap();
		pcm->SetColorMap(m_cmap.m_ntype);
		pcm->SetDivisions(m_cmap.m_ndivs);
		pcm->SetSmooth(m_cmap.m_bsmooth);
		//		pcm->SetRange(m_cmap.m_min, m_cmap.m_max);
	}

	// displacement map
	Post::FEPostModel* ps = po->GetFEModel();
	ps->SetDisplacementField(m_dmap.m_nfield);

	// materials
	if (!m_mat.empty())
	{
		int N0 = (int)m_mat.size();
		int N1 = ps->Materials();
		int N = (N0<N1 ? N0 : N1);
		for (int i = 0; i<N; ++i) *ps->GetMaterial(i) = m_mat[i];

		// update the mesh state
		Post::FEPostMesh* pmesh = po->GetActiveMesh();
		for (int i = 0; i<N; ++i)
		{
			Post::FEMaterial* pm = ps->GetMaterial(i);
			if (pm->bvisible == false) po->HideMaterial(i);
		}
	}

	// reload data fields
	int ndata = (int)m_data.size();
	Post::FEDataManager* pDM = ps->GetDataManager();
	Post::FEDataFieldPtr pdf;
	for (int i = 0; i<ndata; ++i)
	{
		string& si = m_data[i];

		// see if the model already defines this field
		bool bfound = false;
		pdf = pDM->FirstDataField();
		for (int i = 0; i<pDM->DataFields(); ++i, ++pdf)
		{
			if (si.compare((*pdf)->GetName()) == 0) { bfound = true; break; }
		}

		// If not, try to add it
		if (bfound == false) Post::AddStandardDataField(*po, si);
	}

	int ntime = m_mdl.m_ntime;
	if (ntime >= ps->GetStates() - 1) ntime = ps->GetStates() - 1;
	po->SetCurrentTimeIndex(ntime);
	po->Update(false);
}


CPostDocument::CPostDocument(CMainWindow* wnd, CModelDocument* doc) : CDocument(wnd), m_doc(doc)
{
	m_fem = new Post::FEPostModel;
}

CPostDocument::~CPostDocument()
{
	Clear();
	delete m_fem;
}

void CPostDocument::Clear()
{
	m_pCmd->Clear();
	SetModifiedFlag(false);

	m_MD.ReadData(m_glm);

	delete m_glm; m_glm = nullptr;
	delete m_postObj; m_postObj = nullptr;
}

bool CPostDocument::Initialize()
{
	assert(m_fem);
	m_fem->UpdateBoundingBox();

	// assign default material attributes
	const Post::CPalette& pal = Post::CPaletteManager::CurrentPalette();
	ApplyPalette(pal);

	if (m_glm) delete m_glm; m_glm = new Post::CGLModel(m_fem);
	if (m_postObj) delete m_postObj; m_postObj = new CPostObject(m_glm);

	m_timeSettings.Defaults();
	m_timeSettings.m_start = 0;
	m_timeSettings.m_end = GetStates() - 1;

	if (m_MD.IsValid())
	{
		m_MD.WriteData(m_glm);
		m_postObj->Update();
	}
	else
	{
		// get the boundingbox
		BOX box = GetBoundingBox();

		// reset the camera
		m_view.Reset();
		CGLCamera& cam = m_view.GetCamera();
		cam.Reset();
		cam.SetTargetDistance(box.Radius() * 3);
		cam.SetTarget(box.Center());
		cam.Update(true);

		// set the current time
		// this will also update the scene
		SetActiveState(0);
	}

	UpdateFEModel(true);

	return true;
}


GObject* CPostDocument::GetActiveObject()
{
	if (IsValid()) return GetPostObject();
	return nullptr;
}

int CPostDocument::GetStates()
{
	assert(m_fem);
	return m_fem->GetStates();
}

Post::FEPostModel* CPostDocument::GetFEModel()
{
	return m_fem;
}

Post::CGLModel* CPostDocument::GetGLModel()
{
	return m_glm;
}

CGView* CPostDocument::GetView()
{
	return &m_view;
}

void CPostDocument::SetActiveState(int n)
{
	assert(m_glm);
	m_glm->SetCurrentTimeIndex(n);
	m_glm->Update(false);
	m_postObj->UpdateMesh();
}

int CPostDocument::GetActiveState()
{
	return m_glm->CurrentTimeIndex();
}

TIMESETTINGS& CPostDocument::GetTimeSettings()
{
	return m_timeSettings;
}

int CPostDocument::GetEvalField()
{
	if (m_glm == nullptr) return -1;
	Post::CGLColorMap* pc = m_glm->GetColorMap();
	if (pc == 0) return -1;

	return pc->GetEvalField();
}

void CPostDocument::ActivateColormap(bool bchecked)
{
	Post::CGLModel* po = m_glm;
	po->GetColorMap()->Activate(bchecked);
	UpdateFEModel();
}

void CPostDocument::DeleteObject(Post::CGLObject* po)
{
	Post::CGLPlot* pp = dynamic_cast<Post::CGLPlot*>(po);
	if (pp)
	{
		delete pp;
	}
	else if (dynamic_cast<GLCameraTransform*>(po))
	{
		GLCameraTransform* pt = dynamic_cast<GLCameraTransform*>(po);
		CGView* pview = GetView();
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

std::string CPostDocument::GetFieldString()
{
	if (IsValid())
	{
		int nfield = GetGLModel()->GetColorMap()->GetEvalField();
		return GetFEModel()->GetDataManager()->getDataString(nfield, Post::DATA_SCALAR);
	}
	else return "";
}

float CPostDocument::GetTimeValue()
{
	if (m_glm) return m_glm->CurrentTime();
	else return 0.f;
}

float CPostDocument::GetTimeValue(int n)
{
	if (m_glm) return m_glm->GetFEModel()->GetTimeValue(n);
	else return 0.f;
}

void CPostDocument::SetCurrentTimeValue(float ftime)
{
	if (IsValid())
	{
		m_glm->SetTimeValue(ftime);
		UpdateFEModel();
	}
}

void CPostDocument::UpdateAllStates()
{
	if (IsValid() == false) return;
	int N = m_fem->GetStates();
	int ntime = GetActiveState();
	for (int i = 0; i<N; ++i) SetActiveState(i);
	SetActiveState(ntime);
}

void CPostDocument::UpdateFEModel(bool breset)
{
	if (!IsValid()) return;

	// update the model
	if (m_glm) m_glm->Update(breset);
}

void CPostDocument::SetDataField(int n)
{
	m_glm->GetColorMap()->SetEvalField(n);
	m_glm->Update(false);
}

BOX CPostDocument::GetBoundingBox()
{
	BOX b;
	if (m_fem) b = m_fem->GetBoundingBox();
	return b;
}

BOX CPostDocument::GetSelectionBox()
{
	BOX box;

	if (IsValid() == false)
	{
		box = BOX(-1, -1, -1, 1, 1, 1);
		return box;
	}

	Post::CGLModel* mdl = GetGLModel();
	if (mdl == nullptr)
	{
		box = BOX(-1, -1, -1, 1, 1, 1);
	}
	else
	{
		Post::FEPostMesh& mesh = *mdl->GetActiveMesh();
		const vector<FEElement_*> selElems = mdl->GetElementSelection();
		for (int i = 0; i < (int)selElems.size(); ++i)
		{
			FEElement_& el = *selElems[i];
			int nel = el.Nodes();
			for (int j = 0; j < nel; ++j) box += mesh.Node(el.m_node[j]).r;
		}

		const vector<FEFace*> selFaces = GetGLModel()->GetFaceSelection();
		for (int i = 0; i < (int)selFaces.size(); ++i)
		{
			FEFace& face = *selFaces[i];
			int nel = face.Nodes();
			for (int j = 0; j < nel; ++j) box += mesh.Node(face.n[j]).r;
		}

		const vector<FEEdge*> selEdges = GetGLModel()->GetEdgeSelection();
		for (int i = 0; i < (int)selEdges.size(); ++i)
		{
			FEEdge& edge = *selEdges[i];
			int nel = edge.Nodes();
			for (int j = 0; j < nel; ++j) box += mesh.Node(edge.n[j]).r;
		}

		const vector<FENode*> selNodes = GetGLModel()->GetNodeSelection();
		for (int i = 0; i < (int)selNodes.size(); ++i)
		{
			FENode& node = *selNodes[i];
			box += node.r;
		}
	}

	//	if (box.IsValid())
	{
		if ((box.Width() < 1e-5) || (box.Height() < 1e-4) || (box.Depth() < 1e-4))
		{
			float R = box.Radius();
			box.InflateTo(R, R, R);
		}
	}

	return box;
}

std::string CPostDocument::GetFileName()
{
	return m_fileName;
}


bool CPostDocument::IsValid()
{
	return ((m_glm != nullptr) && (m_glm->GetFEModel() != nullptr) && (m_postObj != nullptr));
}

void CPostDocument::ApplyPalette(const Post::CPalette& pal)
{
	int NCOL = pal.Colors();
	int nmat = m_fem->Materials();
	for (int i = 0; i<nmat; i++)
	{
		GLColor c = pal.Color(i % NCOL);

		Post::FEMaterial& m = *m_fem->GetMaterial(i);
		m.diffuse = c;
		m.ambient = c;
		m.specular = GLColor(128, 128, 128);
		m.emission = GLColor(0, 0, 0);
		m.shininess = 0.5f;
		m.transparency = 1.f;
	}
}

CPostObject* CPostDocument::GetPostObject()
{
	return m_postObj;
}
