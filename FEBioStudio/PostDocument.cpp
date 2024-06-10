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
#include <GeomLib/GModel.h>
#include <GLWLib/GLWidgetManager.h>
//---------------------------------------
// NOTE: We need to include these FEBio files to make sure we get the correct
//       implementations for the type_to_string and string_to_type functions.
#include <FEBio/FEBioFormat.h>
#include <FEBio/FEBioExport.h>
//#include <XML/XMLWriter.h>
//#include <XML/XMLReader.h>
//---------------------------------------
#include <XPLTLib/xpltFileReader.h>
#include <FSCore/ClassDescriptor.h>
#include "PostSessionFile.h"
#include "units.h"
#include "GLPostScene.h"
#include "MainWindow.h"

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
	m_mdl.m_bShell2Solid = po->ShowShell2Solid();
	m_mdl.m_bBeam2Solid = po->ShowBeam2Solid();
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
	po->ShowShell2Solid(m_mdl.m_bShell2Solid);
	po->ShowBeam2Solid(m_mdl.m_bBeam2Solid);
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

	// give the plots a chance to reload data if necessary
	for (int i = 0; i < po->Plots(); ++i)
	{
		Post::CGLPlot* plt = po->Plot(i);
		plt->Reload();
	}

	po->Update(false);
}


CPostDocument::CPostDocument(CMainWindow* wnd, CModelDocument* doc) : CGLDocument(wnd), m_doc(doc)
{
	SetIcon(":/icons/PostView.png");

	m_fem = new Post::FEPostModel;
	m_postObj = nullptr;
	m_glm = nullptr;

	m_binit = false;

	m_scene = new CGLPostScene(this);
	m_scene->SetEnvironmentMap(wnd->GetEnvironmentMap());

	SetItemMode(ITEM_ELEM);

	// we do want to show the title and subtitle for post docs.
	m_showTitle = true;
	m_showSubtitle = true;

	QObject::connect(this, SIGNAL(selectionChanged()), wnd, SLOT(on_selectionChanged()));
}

CPostDocument::~CPostDocument()
{
	m_bModified = false;
	Clear();

	for (int i = 0; i < m_graphs.size(); ++i) delete m_graphs[i];
	m_graphs.clear();

	delete m_glm; m_glm = nullptr;
	delete m_fem; m_fem = nullptr;
	SetCurrentSelection(nullptr);

	delete m_scene;
	m_scene = nullptr;
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

	// clear any selection
	SetCurrentSelection(nullptr);

	// assign default material attributes
	const Post::CPalette& pal = Post::CPaletteManager::CurrentPalette();
	ApplyPalette(pal);

	// make sure the correct GLWidgetManager's edit layer is active
	CGLWidgetManager::GetInstance()->SetEditLayer(m_widgetLayer);

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
		CGView& view = *GetView();
		view.Reset();
		CGLCamera& cam = view.GetCamera();
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

	xpltFileReader* reader = dynamic_cast<xpltFileReader*>(GetFileReader());
	if (reader)
	{
		const char* szunits = reader->GetUnits();
		if (szunits)
		{
			int n = Units::FindUnitSytemFromName(szunits);
			SetUnitSystem(n);
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

void CPostDocument::Activate()
{
	Post::FEPostModel::SetInstance(m_fem);
	CGLDocument::Activate();
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
	else if (dynamic_cast<CImageModel*>(po))
	{
		CImageModel* img = dynamic_cast<CImageModel*>(po);
		m_img.Remove(img);
		delete img;
	}
	else if (dynamic_cast<Post::CGLImageRenderer*>(po))
	{
		Post::CGLImageRenderer* ir = dynamic_cast<Post::CGLImageRenderer*>(po);
		CImageModel* img = ir->GetImageModel();
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
		return GetFSModel()->GetDataManager()->getDataString(nfield, Post::TENSOR_SCALAR);
	}
	else return "";
}

std::string CPostDocument::GetFieldUnits()
{
	if (IsValid())
	{
		int nfield = GetGLModel()->GetColorMap()->GetEvalField();
		const char* szunits = GetFSModel()->GetDataManager()->getDataUnits(nfield);
		if (szunits)
		{
			QString s = QString("(%1)").arg(Units::GetUnitString(szunits));
			return s.toStdString();
		}
		else return "";
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
	if (!IsValid()) return BOX(-1, -1, -1, 1, 1, 1);

	BOX box;
	FESelection* currentSelection = GetCurrentSelection();
	if (currentSelection && currentSelection->Size())
	{
		box = currentSelection->GetBoundingBox();
	}

	if ((box.Width() < 1e-5) || (box.Height() < 1e-4) || (box.Depth() < 1e-4))
	{
		float R = box.Radius();
		box.InflateTo(R, R, R);
	}

	return box;
}

std::string CPostDocument::GetFileName()
{
	return m_fileName;
}


bool CPostDocument::IsValid()
{
	return ((m_glm != nullptr) && (m_glm->GetFSModel() != nullptr) && (m_postObj != nullptr));
}

void CPostDocument::SetModifiedFlag(bool bset)
{
	// ignore this, since post docs can't be saved anyways
}

void CPostDocument::SetInitFlag(bool b)
{
	m_binit = b;
}

void CPostDocument::UpdateSelection(bool report)
{
	Post::CGLModel* mdl = GetGLModel();

	// delete old selection
	if (mdl) mdl->SetSelection(nullptr); 
	if (m_psel) delete m_psel;
	m_psel = nullptr;

	// figure out if there is a mesh selected
	GObject* po = GetActiveObject();
	FSMesh* pm = (po ? po->GetFEMesh() : 0);
	FSMeshBase* pmb = (po ? po->GetEditableMesh() : 0);
	FSLineMesh* plm = (po ? po->GetEditableLineMesh() : 0);

	switch (m_vs.nitem)
	{
	case ITEM_ELEM: if (pm) m_psel = new FEElementSelection(pm); break;
	case ITEM_FACE: if (pm) m_psel = new FEFaceSelection(pm); break;
	case ITEM_EDGE: if (pm) m_psel = new FEEdgeSelection(pm); break;
	case ITEM_NODE: if (pm) m_psel = new FENodeSelection(pm); break;
	default:
		return;
	}
	if (m_psel) m_psel->SetMovable(false);

	if (mdl) mdl->SetSelection(m_psel);

	if (report)
	{
		emit selectionChanged();
	}
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
	PostSessionFileWriter writer(this);
	return writer.Write(fileName.c_str());
}

//-------------------------------------------------------------------------------------------
void CPostDocument::SetGLModel(Post::CGLModel* glm)
{
	if (m_glm) delete m_glm;
	m_glm = glm;
}
