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
#include "PostDocument.h"
#include "ModelDocument.h"
#include "PostObject.h"
#include <FSCore/FSDir.h>
#include <PostLib/FEPostModel.h>
#include <PostLib/Palette.h>
#include <PostLib/constants.h>
#include <PostGL/GLModel.h>
#include <MeshTools/GModel.h>
//---------------------------------------
// NOTE: We need to include these FEBio files to make sure we get the correct
//       implementations for the type_to_string and string_to_type functions.
#include <FEBio/FEBioFormat.h>
#include <FEBio/FEBioExport.h>
//#include <XML/XMLWriter.h>
//#include <XML/XMLReader.h>
//---------------------------------------
#include <XPLTLib/xpltFileReader.h>
#include "ClassDescriptor.h"
#include <PostLib/FELSDYNAimport.h>
#include <PostLib/FEKinemat.h>
#include <QtCore/QDir>

template <> std::string type_to_string<GLColor>(const GLColor& v)
{
	std::stringstream ss;
	ss << v.r << "," << v.g << "," << v.b;
	return ss.str();
}

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
		m_cmap.m_maxRangeType = pglmap->GetMaxRangeType();
		m_cmap.m_minRangeType = pglmap->GetMinRangeType();
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
	Post::FEPostModel* ps = po->GetFSModel();
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
		pglmap->SetMaxRangeType(m_cmap.m_maxRangeType);
		pglmap->SetMinRangeType(m_cmap.m_minRangeType);
		pglmap->SetRange(m_cmap.m_user);
		pglmap->DisplayNodalValues(m_cmap.m_bDispNodeVals);

		Post::CColorTexture* pcm = pglmap->GetColorMap();
		pcm->SetColorMap(m_cmap.m_ntype);
		pcm->SetDivisions(m_cmap.m_ndivs);
		pcm->SetSmooth(m_cmap.m_bsmooth);
		//		pcm->SetRange(m_cmap.m_min, m_cmap.m_max);
	}

	// displacement map
	Post::FEPostModel* ps = po->GetFSModel();
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
			Post::Material* pm = ps->GetMaterial(i);
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
		if (bfound == false) Post::AddStandardDataField(*ps, si);
	}

	// see if we can reactivate the same data field
	if (pglmap)
	{
		if (pDM->IsValid(m_cmap.m_nField))
		{
			pglmap->SetEvalField(m_cmap.m_nField);
			pglmap->Activate(m_cmap.m_bactive);
		}
		else
		{
			pglmap->SetEvalField(0);
			pglmap->Activate(false);
		}
	}

	po->UpdateMeshState();
	int ntime = m_mdl.m_ntime;
	if (ntime >= ps->GetStates() - 1) ntime = ps->GetStates() - 1;
	po->SetCurrentTimeIndex(ntime);
	po->Update(false);
}


CPostDocument::CPostDocument(CMainWindow* wnd, CModelDocument* doc) : CGLDocument(wnd), m_doc(doc)
{
	SetIcon(":/icons/PostView.png");

	m_fem = new Post::FEPostModel;
	m_postObj = nullptr;
	m_glm = nullptr;
	m_sel = nullptr;

	m_binit = false;

	SetItemMode(ITEM_ELEM);
}

CPostDocument::~CPostDocument()
{
	m_bModified = false;
	Clear();

	for (int i = 0; i < m_graphs.size(); ++i) delete m_graphs[i];
	m_graphs.clear();

	delete m_glm; m_glm = nullptr;
	delete m_fem;
	if (m_sel) delete m_sel;
}

void CPostDocument::Clear()
{
	m_pCmd->Clear();
	SetModifiedFlag(false);

	m_MD.ReadData(m_glm);

	if (m_glm) m_glm->SetFEModel(nullptr);

	delete m_postObj; m_postObj = nullptr;
}

bool CPostDocument::Initialize()
{
	// the init flag is only used when reading a session to prevent Initialize from getting called twice
	if (m_binit)
	{
		m_binit = false;
		return true;
	}

	assert(m_fem);
	m_fem->UpdateBoundingBox();

	// assign default material attributes
	const Post::CPalette& pal = Post::CPaletteManager::CurrentPalette();
	ApplyPalette(pal);

	if (m_glm == nullptr) m_glm = new Post::CGLModel(m_fem); else m_glm->SetFEModel(m_fem);
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

		// map colors from modeldoc
		if (m_doc)
		{
			FSModel& docfem = *m_doc->GetFSModel();
			GModel* mdl = m_doc->GetGModel();

			int mats = docfem.Materials();
			int dmats = mdl->DiscreteObjects();
			if (mats + dmats == m_fem->Materials())
			{
				for (int i = 0; i < mats; ++i)
				{
					GLColor c = docfem.GetMaterial(i)->Diffuse();

					Post::Material* mat = m_fem->GetMaterial(i);
					mat->ambient = c;
					mat->diffuse = c;
				}

				for (int i = 0; i < dmats; ++i)
				{
					GLColor c = mdl->DiscreteObject(i)->GetColor();
					Post::Material* mat = m_fem->GetMaterial(i + mats);
					mat->ambient = c;
					mat->diffuse = c;
				}
			}
		}
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

Post::FEPostModel* CPostDocument::GetFSModel()
{
	return m_fem;
}

Post::CGLModel* CPostDocument::GetGLModel()
{
	return m_glm;
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
	else if (dynamic_cast<Post::CImageModel*>(po))
	{
		Post::CImageModel* img = dynamic_cast<Post::CImageModel*>(po);
		m_img.Remove(img);
		delete img;
	}
	else if (dynamic_cast<Post::CGLImageRenderer*>(po))
	{
		Post::CGLImageRenderer* ir = dynamic_cast<Post::CGLImageRenderer*>(po);
		Post::CImageModel* img = ir->GetImageModel();
		img->RemoveRenderer(ir);
	}
	else if (dynamic_cast<Post::CGLDisplacementMap*>(po))
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
		return GetFSModel()->GetDataManager()->getDataString(nfield, Post::DATA_SCALAR);
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
	if (m_glm) return m_glm->GetFSModel()->GetTimeValue(n);
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

		const vector<FSEdge*> selEdges = GetGLModel()->GetEdgeSelection();
		for (int i = 0; i < (int)selEdges.size(); ++i)
		{
			FSEdge& edge = *selEdges[i];
			int nel = edge.Nodes();
			for (int j = 0; j < nel; ++j) box += mesh.Node(edge.n[j]).r;
		}

		const vector<FSNode*> selNodes = GetGLModel()->GetNodeSelection();
		for (int i = 0; i < (int)selNodes.size(); ++i)
		{
			FSNode& node = *selNodes[i];
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

FESelection* CPostDocument::GetCurrentSelection()
{
	if (m_sel) { delete m_sel; m_sel = nullptr; }

	FSMesh* pm = GetGLModel()->GetActiveMesh();

	int selectionMode = m_vs.nitem;
	switch (selectionMode)
	{
	case ITEM_NODE: m_sel = new FENodeSelection(nullptr, pm); break;
	case ITEM_EDGE: m_sel = new FEEdgeSelection(nullptr, pm); break;
	case ITEM_FACE: m_sel = new FEFaceSelection(nullptr, pm); break;
	case ITEM_ELEM: m_sel = new FEElementSelection(nullptr, pm); break;
	}

	return m_sel;
}

std::string CPostDocument::GetFileName()
{
	return m_fileName;
}


bool CPostDocument::IsValid()
{
	return ((m_glm != nullptr) && (m_glm->GetFSModel() != nullptr) && (m_postObj != nullptr));
}

void CPostDocument::ApplyPalette(const Post::CPalette& pal)
{
	int NCOL = pal.Colors();
	int nmat = m_fem->Materials();
	for (int i = 0; i<nmat; i++)
	{
		GLColor c = pal.Color(i % NCOL);

		Post::Material& m = *m_fem->GetMaterial(i);
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

int CPostDocument::Graphs() const
{
	return (int)m_graphs.size();
}

void CPostDocument::AddGraph(const CGraphData& data)
{
	CGraphData* newData = new CGraphData(data);
	if (newData->GetName().empty()) newData->SetName(newData->m_title.toStdString());
	m_graphs.push_back(newData);
}

const CGraphData* CPostDocument::GetGraphData(int i)
{
	return m_graphs[i];
}

int CPostDocument::FindGraphData(const CGraphData* data)
{
	for (int i = 0; i < m_graphs.size(); ++i)
	{
		if (m_graphs[i] == data) return i;
	}
	return -1;
}

void CPostDocument::ReplaceGraphData(int n, const CGraphData& data)
{
	if ((n >= 0) && (n < m_graphs.size()))
	{
		*m_graphs[n] = data;
	}
}

void CPostDocument::DeleteGraph(const CGraphData* data)
{
	int n = FindGraphData(data);
	delete m_graphs[n];
	m_graphs.erase(m_graphs.begin() + n);
}

bool CPostDocument::MergeFEModel(Post::FEPostModel* fem)
{
	if (fem == nullptr) return false;

	// delete the model
	delete m_glm; m_glm = nullptr;

	// merge the fem models
	bool bret = m_fem->Merge(fem);

	// we assume that the merge did not break the model
	// so we can always reinitialize
	return Initialize() && bret;
}

//-------------------------------------------------------------------------------------------
//! save to session file
bool CPostDocument::SavePostSession(const std::string& fileName)
{
	if (fileName.empty()) return false;

	Post::FEPostModel* fem = GetFSModel();
	if (fem == nullptr) return false;

	XMLWriter xml;
	if (xml.open(fileName.c_str()) == false) return false;

	XMLElement root("febiostudio_post_session");
	root.add_attribute("version", "1.0");
	xml.add_branch(root);
	{
		// save plot file
		std::string plotFile = GetDocFilePath();
		XMLElement plt("open");
		plt.add_attribute("file", plotFile);
		xml.add_empty(plt);

		// save material settings
		for (int i = 0; i < fem->Materials(); ++i)
		{
			Post::Material* mat = fem->GetMaterial(i);
			XMLElement el("material");
			el.add_attribute("id", i + 1);
			el.add_attribute("name", mat->GetName());
			xml.add_branch(el);
			{
				xml.add_leaf("diffuse"     , mat->diffuse);
				xml.add_leaf("ambient"     , mat->ambient);
				xml.add_leaf("specular"    , mat->specular);
				xml.add_leaf("emission"    , mat->emission);
				xml.add_leaf("mesh_color"  , mat->meshcol);
				xml.add_leaf("shininess"   , mat->shininess);
				xml.add_leaf("transparency", mat->transparency);
			}
			xml.close_branch(); // material
		}

		// save selections
		GObject* po = GetActiveObject();
		if (po)
		{
			for (int i = 0; i < po->FENodeSets(); ++i)
			{
				FENodeSet* pg = po->GetFENodeSet(i);

				XMLElement el("mesh:nodeset");
				el.add_attribute("name", pg->GetName());
				xml.add_branch(el);
				{
					std::list<int> items = pg->CopyItems();
					std::list<int>::iterator it = items.begin();
					int N = items.size();
					int l[16];
					for (int n = 0; n < N; n += 16)
					{
						int m = (n + 16 <= N ? 16 : N - n);
						for (int k = 0; k < m; ++k) l[k] = 1 + (*it++);
						xml.add_leaf("nodes", l, m);
					}
				}
				xml.close_branch();
			}

			for (int i = 0; i < po->FEEdgeSets(); ++i)
			{
				FEEdgeSet* pg = po->GetFEEdgeSet(i);

				XMLElement el("mesh:edgeset");
				el.add_attribute("name", pg->GetName());
				xml.add_branch(el);
				{
					std::list<int> items = pg->CopyItems();
					std::list<int>::iterator it = items.begin();
					int N = items.size();
					int l[16];
					for (int n = 0; n < N; n += 16)
					{
						int m = (n + 16 <= N ? 16 : N - n);
						for (int k = 0; k < m; ++k) l[k] = 1 + (*it++);
						xml.add_leaf("edges", l, m);
					}
				}
				xml.close_branch();
			}

			for (int i = 0; i < po->FESurfaces(); ++i)
			{
				FESurface* pg = po->GetFESurface(i);

				XMLElement el("mesh:surface");
				el.add_attribute("name", pg->GetName());
				xml.add_branch(el);
				{
					std::list<int> items = pg->CopyItems();
					std::list<int>::iterator it = items.begin();
					int N = items.size();
					int l[16];
					for (int n = 0; n < N; n += 16)
					{
						int m = (n + 16 <= N ? 16 : N - n);
						for (int k = 0; k < m; ++k) l[k] = 1 + (*it++);
						xml.add_leaf("faces", l, m);
					}
				}
				xml.close_branch();
			}

			for (int i = 0; i < po->FEParts(); ++i)
			{
				FEPart* pg = po->GetFEPart(i);

				XMLElement el("mesh:part");
				el.add_attribute("name", pg->GetName());
				xml.add_branch(el);
				{
					std::list<int> items = pg->CopyItems();
					std::list<int>::iterator it = items.begin();
					int N = items.size();
					int l[16];
					for (int n = 0; n < N; n += 16)
					{
						int m = (n + 16 <= N ? 16 : N - n);
						for (int k = 0; k < m; ++k) l[k] = 1 + (*it++);
						xml.add_leaf("elems", l, m);
					}
				}
				xml.close_branch();
			}
		}

		// save post model components
		Post::CGLModel* glm = GetGLModel();
		if (glm)
		{
			for (int i = 0; i < glm->Plots(); ++i)
			{
				Post::CGLPlot* plot = glm->Plot(i);

				std::string typeStr = plot->GetTypeString();
					
				XMLElement el("plot");
				el.add_attribute("type", typeStr);
				xml.add_branch(el);
				{
					for (int i = 0; i < plot->Parameters(); ++i)
					{
						Param& pi = plot->GetParam(i);
						const char* sz = pi.GetShortName();

						el.name(sz);
						switch (pi.GetParamType())
						{
						case Param_BOOL  : { bool b = pi.GetBoolValue(); el.value(b); } break;
						case Param_INT   : { int n = pi.GetIntValue(); el.value(n); } break;
						case Param_CHOICE: { int n = pi.GetIntValue(); el.value(n); } break;
						case Param_FLOAT : { double v = pi.GetFloatValue(); el.value(v); } break;
						case Param_VEC3D : { vec3d v = pi.GetVec3dValue(); el.value(v); } break;
						case Param_COLOR : { GLColor c = pi.GetColorValue(); int v[3] = { c.r, c.g, c.b }; el.value(v, 3); } break;
						}

						xml.add_leaf(el);
					}
				}
				xml.close_branch();
			}
		}
	}
	xml.close_branch(); // root

	xml.close();

	return true;
}

//-------------------------------------------------------------------------------------------
//! open session file
bool CPostDocument::OpenPostSession(const std::string& fileName)
{
	if (fileName.empty()) return false;

	// we'll use this for converting to absolute file paths.
	QFileInfo fi(QString::fromStdString(fileName));
	QDir currentDir(fi.absolutePath());

	XMLReader xml;
	if (xml.Open(fileName.c_str()) == false) return false;

	XMLTag tag;
	if (xml.FindTag("febiostudio_post_session", tag) == false)
	{
		return false;
	}

	if (m_glm) {
		delete m_glm; m_glm = nullptr;
	}
	try {
		++tag;
		do
		{
			if (tag == "open")
			{
				const char* szfile = tag.AttributeValue("file");
				xpltFileReader xplt(GetFSModel());
				if (xplt.Load(szfile) == false)
				{
					return false;
				}

				// now create a GL model
				m_glm = new Post::CGLModel(m_fem);

				// save the plot file as the document's path
				SetDocFilePath(szfile);

				if (Initialize() == false) return false;
				m_binit = true;
			}
			else if (tag == "kinemat")
			{
				int n[3] = { 1,999,1 };
				std::string modelFile, kineFile;
				++tag;
				do
				{
					if (tag == "model_file") tag.value(modelFile);
					if (tag == "kine_file" ) tag.value(kineFile);
					if (tag == "range"     ) tag.value(n, 3);
					++tag;
				} 
				while (!tag.isend());

				// create absolute file names for model and kine files
				modelFile = currentDir.absoluteFilePath(QString::fromStdString(modelFile)).toStdString();
				kineFile  = currentDir.absoluteFilePath(QString::fromStdString(kineFile)).toStdString();

				// read the model
				Post::FELSDYNAimport reader(m_fem);
				reader.read_displacements(true);
				bool bret = reader.Load(modelFile.c_str());
				if (bret == false) return false;

				// apply kine
				FEKinemat kine;
				kine.SetRange(n[0], n[1], n[2]);
				if (kine.Apply(m_fem, kineFile.c_str()) == false) return false;

				// update post document
				Initialize();

				// update displacements on all states
				Post::CGLModel& mdl = *GetGLModel();
				if (mdl.GetDisplacementMap() == nullptr)
				{
					mdl.AddDisplacementMap("Displacement");
				}
				int nstates = mdl.GetFSModel()->GetStates();
				for (int i = 0; i < nstates; ++i) mdl.UpdateDisplacements(i, true);
			}
			else if (tag == "material")
			{
				const char* szid = tag.AttributeValue("id");
				int nid = atoi(szid) - 1;
				if ((nid >= 0) && (nid < m_fem->Materials()))
				{
					Post::Material* mat = m_fem->GetMaterial(nid);

					++tag;
					do
					{
						if (tag == "diffuse") tag.value(mat->diffuse);
						if (tag == "ambient") tag.value(mat->ambient);
						if (tag == "specular") tag.value(mat->specular);
						if (tag == "emission") tag.value(mat->emission);
						if (tag == "mesh_color") tag.value(mat->meshcol);
						if (tag == "shininess") tag.value(mat->shininess);
						if (tag == "transparency") tag.value(mat->transparency);
						++tag;
					} while (!tag.isend());
				}
			}
			else if (tag == "mesh:nodeset")
			{
				const char* szname = tag.AttributeValue("name");
				vector<int> nodeList;
				++tag;
				do {
					if (tag == "nodes")
					{
						int l[16];
						int m = tag.value(l, 16);
						for (int i = 0; i < m; ++i) nodeList.push_back(l[i] - 1);
					}
					++tag;
				} while (!tag.isend());

				GObject* po = GetActiveObject();
				if (po)
				{
					FENodeSet* pg = new FENodeSet(po, nodeList);
					pg->SetName(szname);
					po->AddFENodeSet(pg);
				}
			}
			else if (tag == "mesh:edgeset")
			{
				const char* szname = tag.AttributeValue("name");
				vector<int> edgeList;
				++tag;
				do {
					if (tag == "edges")
					{
						int l[16];
						int m = tag.value(l, 16);
						for (int i = 0; i < m; ++i) edgeList.push_back(l[i] - 1);
					}
					++tag;
				} while (!tag.isend());

				GObject* po = GetActiveObject();
				if (po)
				{
					FEEdgeSet* pg = new FEEdgeSet(po, edgeList);
					pg->SetName(szname);
					po->AddFEEdgeSet(pg);
				}
			}
			else if (tag == "mesh:surface")
			{
				const char* szname = tag.AttributeValue("name");
				vector<int> faceList;
				++tag;
				do {
					if (tag == "faces")
					{
						int l[16];
						int m = tag.value(l, 16);
						for (int i = 0; i < m; ++i) faceList.push_back(l[i] - 1);
					}
					++tag;
				} while (!tag.isend());

				GObject* po = GetActiveObject();
				if (po)
				{
					FESurface* pg = new FESurface(po, faceList);
					pg->SetName(szname);
					po->AddFESurface(pg);
				}
			}
			else if (tag == "mesh:part")
			{
				const char* szname = tag.AttributeValue("name");
				vector<int> elemList;
				++tag;
				do {
					if (tag == "elems")
					{
						int l[16];
						int m = tag.value(l, 16);
						for (int i = 0; i < m; ++i) elemList.push_back(l[i] - 1);
					}
					++tag;
				} while (!tag.isend());

				GObject* po = GetActiveObject();
				if (po)
				{
					FEPart* pg = new FEPart(po, elemList);
					pg->SetName(szname);
					po->AddFEPart(pg);
				}
			}
			else if (tag == "plot")
			{
				const char* sztype = tag.AttributeValue("type");
				Post::CGLPlot* plot = FSCore::CreateClass<Post::CGLPlot>(CLASS_PLOT, sztype);

				const char* szname = tag.AttributeValue("name", true);
				if (szname) plot->SetName(szname);

				m_glm->AddPlot(plot);

				++tag;
				do
				{
					Param* p = plot->GetParam(tag.Name());
					if (p)
					{
						switch (p->GetParamType())
						{
						case Param_BOOL: { bool b; tag.value(b); p->SetBoolValue(b); } break;
						case Param_INT: { int n; tag.value(n); p->SetIntValue(n); } break;
						case Param_CHOICE: { int n; tag.value(n); p->SetIntValue(n); } break;
						case Param_FLOAT: { double g; tag.value(g); p->SetFloatValue(g); } break;
						case Param_VEC3D: { vec3d v; tag.value(v); p->SetVec3dValue(v); } break;
						case Param_COLOR: { GLColor c; tag.value(c); p->SetColorValue(c); } break;
						}
					}
					++tag;
				} while (!tag.isend());

				plot->UpdateData(true);
			}
			//			else xml.SkipTag(tag);
			else return false;
			++tag;
		} while (!tag.isend());
	}
	catch (...)
	{
		// TODO: We need this for catching end-of-file. 
	}

	xml.Close();

	return true;
}
