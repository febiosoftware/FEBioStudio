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
#include "GLModel.h"
#include <PostLib/FEDataManager.h>
#include <PostLib/constants.h>

typedef unsigned char byte;

using namespace Post;

//-----------------------------------------------------------------------------
extern int ET_HEX[12][2];
extern int ET_HEX20[12][3];
extern int ET_TET[6][2];
extern int ET_PENTA[9][2];
extern int ET_PENTA15[9][3];
extern int ET_TET10[6][3];
extern int ET_PYRA5[8][2];
extern int ET_PYRA13[8][3];

//-----------------------------------------------------------------------------
// constructor
CGLModel::CGLModel(FEPostModel* ps)
{
	m_postMdl = ps;
	SetName("Model");

	m_lastMesh = nullptr;

	m_stol = 60.0;

	m_bnorm = false;
	m_scaleNormals = 1.0;
	m_bghost = false;
	m_nDivs = 0; // this means "auto"
	m_brenderInteriorNodes = true;

	m_renderInnerSurface = true;

	m_solidBeamRadius = 1.f;
	m_bShell2Solid = false;
	m_bBeam2Solid = false;

	m_nshellref = 0;

	m_bshowMesh = true;

	m_line_col = GLColor(0, 0, 0);
	m_node_col = GLColor(0, 0, 255);
	m_sel_col = GLColor(255, 0, 0);

	m_nrender = RENDER_MODE_SOLID;

	m_nconv = CONV_FR_XZ;

	m_selectType = SELECT_FE_ELEMS;
	m_selectStyle = SELECT_RECT;
	m_selection = nullptr;

	m_ghost_color = GLColor(96, 96, 96);

	if (ps == nullptr) return;

	SetCurrentTimeIndex(0);

	// see if the mesh has any vector fields
	// which can be used for displacement maps
	FEDataManager* pdm = ps->GetDataManager();
	FEDataFieldPtr pd = pdm->FirstDataField();
	int ndisp = -1;
	for (int i=0; i<pdm->DataFields(); ++i, ++pd)
	{
		if ((*pd)->Type() == DATA_VEC3)
		{
			std::string sname = (*pd)->GetName();
			if ((sname == "displacement") || (sname == "Displacement")) ndisp = i;
		}
	}

	if (ndisp != -1)
	{
		ps->SetDisplacementField(BUILD_FIELD( DATA_CLASS::NODE_DATA, ndisp, 0));
		m_dispMap.reset(new CGLDisplacementMap(this));
	}

	// add a default color map
	m_colMap.reset(new CGLColorMap(this));

	if (ps)
	{
		m_postObj.reset(new CPostObject(this));

		// Set the FE mesh and update
		m_postObj->SetFEMesh(ps->GetFEMesh(0));
		m_postObj->Update(true);
	}
	else m_postObj = nullptr;

	UpdateEdge();
	if (m_postObj) m_postObj->BuildInternalSurfaces();
	Update(false);
}

//-----------------------------------------------------------------------------
//! destructor
CGLModel::~CGLModel(void)
{
}

void CGLModel::Clear()
{
	SetFEModel(nullptr);
	m_postObj.reset();
}

bool CGLModel::IsValid() const
{
	return (m_postMdl && m_postObj);
}

void CGLModel::SetFEModel(FEPostModel* ps)
{
	SetSelection(nullptr);
	m_postMdl = ps;
	m_postObj.reset(); 
	if (ps)
	{
		m_postObj.reset(new CPostObject(this));
		m_postObj->SetFEMesh(ps->GetFEMesh(0));
		m_postObj->Update(true);
		m_postObj->BuildInternalSurfaces();
	}
}

CPostObject* CGLModel::GetPostObject()
{
	return m_postObj.get();
}

void CGLModel::ShowShell2Solid(bool b) { m_bShell2Solid = b; }
bool CGLModel::ShowShell2Solid() const { return m_bShell2Solid; }

//-----------------------------------------------------------------------------
void CGLModel::ShowBeam2Solid(bool b) { m_bBeam2Solid = b; }
bool CGLModel::ShowBeam2Solid() const { return m_bBeam2Solid; }

//-----------------------------------------------------------------------------
void CGLModel::SolidBeamRadius(float f) { m_solidBeamRadius = f; }
float CGLModel::SolidBeamRadius() const { return m_solidBeamRadius; }

//-----------------------------------------------------------------------------
int CGLModel::ShellReferenceSurface() const { return m_nshellref; }
void CGLModel::ShellReferenceSurface(int n) { m_nshellref = n; }

//-----------------------------------------------------------------------------
FSMesh* CGLModel::GetActiveMesh()
{
	if (m_postMdl)
	{
		if (m_postMdl->GetStates() > 0) return m_postMdl->CurrentState()->GetFEMesh();
		return m_postMdl->GetFEMesh(0);
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
Post::FEState* CGLModel::GetActiveState()
{
	if (m_postMdl && (m_postMdl->GetStates() > 0)) return m_postMdl->CurrentState();
	return nullptr;
}

//-----------------------------------------------------------------------------
float CGLModel::CurrentTime() const { return (m_postMdl ? m_postMdl->CurrentTime() : 0.f); }

//-----------------------------------------------------------------------------
int CGLModel::CurrentTimeIndex() const { return (m_postMdl ? m_postMdl->CurrentTimeIndex() : -1); }

//-----------------------------------------------------------------------------
void CGLModel::SetCurrentTimeIndex(int ntime)
{
	if (m_postMdl && m_postMdl->GetStates()) m_postMdl->SetCurrentTimeIndex(ntime);
}

//-----------------------------------------------------------------------------
void CGLModel::SetTimeValue(float ftime)
{
	if (m_postMdl && m_postMdl->GetStates()) m_postMdl->SetTimeValue(ftime);
}

//-----------------------------------------------------------------------------
// Update the model data
bool CGLModel::Update(bool breset)
{
	if (m_postMdl == nullptr) return true;

	FEPostModel& fem = *m_postMdl;
	if (fem.GetStates() == 0) return true;

	// get the time inc value
	int ntime = fem.CurrentTimeIndex();
	float dt = fem.CurrentTime() - fem.GetTimeValue(ntime);

	// update the state of the mesh
	fem.UpdateMeshState(ntime);

	// Calling this will rebuild the internal surfaces
	// This should only be done when the mesh has changed
	FSMesh* currentMesh = fem.CurrentState()->GetFEMesh();
	if (breset || (currentMesh != m_lastMesh))
	{
		UpdateInternalSurfaces(false);
		m_lastMesh = currentMesh;
	}

	// update displacement map
	if (m_dispMap && m_dispMap->IsActive()) m_dispMap->Update(ntime, dt, breset);

	// update the colormap
	if (m_colMap && m_colMap->IsActive()) m_colMap->Update(ntime, dt, breset);

	// NOTE: commenting this out since this would cause the FieldDataSelector's menu
	//       to be rebuild each time a user selected a new field
//	GetFSModel()->UpdateDependants();

	// update the plot list
	for (int i = 0; i < (int)m_pPlot.Size(); ++i)
	{
		CGLPlot* pi = m_pPlot[i];
		if (pi->IsActive()) pi->Update(ntime, dt, breset);
	}

	if (m_postObj)
	{
		Post::FEState* state = GetActiveState();
		if (state)
		{
			FSMesh* postMesh = state->GetFEMesh();
			if (m_postObj->GetFEMesh() != postMesh)
			{
				m_postObj->SetFEMesh(postMesh);
				m_postObj->Update(true);
				UpdateInternalSurfaces(false);
			}
			if (postMesh) postMesh->UpdateBoundingBox();

			m_postObj->UpdateMesh();
			UpdateSelectionMesh();
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
void CGLModel::UpdateDisplacements(int nstate, bool breset)
{
	if (m_dispMap && m_dispMap->IsActive()) m_dispMap->Update(nstate, 0.f, breset);
}

//-----------------------------------------------------------------------------
void CGLModel::SetSmoothingAngle(double w)
{ 
	m_stol = w;
}

//-----------------------------------------------------------------------------
bool CGLModel::AddDisplacementMap(const char* szvectorField)
{
	if (szvectorField == nullptr) szvectorField = "displacement";
	if (m_postMdl == nullptr) return false;

	// see if the mesh has any vector fields
	// which can be used for displacement maps
	FEDataManager* pdm = m_postMdl->GetDataManager();
	FEDataFieldPtr pd = pdm->FirstDataField();
	int nv = 0;
	int ndisp = -1;
	for (int i=0; i<pdm->DataFields(); ++i, ++pd)
	{
		if ((*pd)->Type() == DATA_VEC3) ++nv;
		if ((*pd)->GetName() == szvectorField) ndisp = i;
	}

	if (nv == 0) return false;

	m_dispMap.reset(new CGLDisplacementMap(this));
	if (ndisp != -1)
	{
		m_postMdl->SetDisplacementField(BUILD_FIELD(DATA_CLASS::NODE_DATA, ndisp, 0));
	}
	else m_postMdl->SetDisplacementField(-1);

	m_postMdl->ResetAllStates();

	return true;
}

//-----------------------------------------------------------------------------
bool CGLModel::HasDisplacementMap()
{
	if ((m_postMdl == nullptr) || (m_dispMap == nullptr)) return false;
	return (m_postMdl->GetDisplacementField() >= 0);
}

//-----------------------------------------------------------------------------
void CGLModel::ResetMesh()
{
	if (m_postMdl == nullptr) return;
	FEPostModel& fem = *m_postMdl;
	FSMesh& mesh = *fem.GetFEMesh(0);

	Post::FERefState& ref = *fem.GetState(0)->m_ref;

	int NN = mesh.Nodes();
	for (int i = 0; i<NN; ++i)
	{
		FSNode& node = mesh.Node(i);
		node.r = to_vec3d(ref.m_Node[i].m_rt);
	}
}

//-----------------------------------------------------------------------------
void CGLModel::RemoveDisplacementMap()
{
	if (m_postMdl == nullptr) return;
	m_postMdl->SetDisplacementField(-1);

	m_dispMap.reset();

	// reset the mesh
	ResetMesh();

	// just to be safe, let's reset all states to force them to reevaluate
	m_postMdl->ResetAllStates();
}

//-----------------------------------------------------------------------------
bool CGLModel::RenderInnerSurfaces()
{
	return m_renderInnerSurface;
}

//-----------------------------------------------------------------------------
void CGLModel::RenderInnerSurfaces(bool b)
{
	m_renderInnerSurface = b;
}

void CGLModel::SetSubDivisions(int ndivs)
{ 
	if (ndivs != m_nDivs)
	{
		m_nDivs = ndivs;
		Update(true);
	}
}

int CGLModel::GetSubDivisions()
{
	FSMesh* mesh = GetActiveMesh();
	if (mesh == nullptr) return 1;

	if (m_nDivs < 1)
	{
		int NE = mesh->Elements();
		if (NE == 0) return 1;

		const int max_elem = 10000;
		int ndivs = max_elem / NE;
		if (ndivs > 10) ndivs = 10;
		if (ndivs <  1) ndivs = 1;

#ifndef NDEBUG
		if (ndivs > 2) ndivs = 2;
#endif

		return ndivs;
	}
	else return m_nDivs;
}

//-----------------------------------------------------------------------------
void CGLModel::SetSelection(FESelection* sel)
{
	m_selection = sel;
	UpdateSelectionMesh();
}

void CGLModel::UpdateSelectionMesh()
{
	m_selectionMesh.reset(new GLMesh());
	BuildSelectionMesh(m_selection, *m_selectionMesh);
}

//-----------------------------------------------------------------------------
//! unhide all items
void CGLModel::UnhideAll()
{
	FSMesh& mesh = *GetActiveMesh();
	for (int i = 0; i<mesh.Elements(); ++i) mesh.ElementRef(i).Unhide();
	for (int i = 0; i<mesh.Faces(); ++i) mesh.Face(i).Unhide();
	for (int i = 0; i<mesh.Edges(); ++i) mesh.Edge(i).Unhide();
	for (int i = 0; i<mesh.Nodes(); ++i) mesh.Node(i).Unhide();
	UpdateInternalSurfaces();
}

//-----------------------------------------------------------------------------
// Hide elements with a particular material ID
void CGLModel::HideMaterial(int nmat)
{
	FSMesh& mesh = *GetActiveMesh();

	// Hide the elements with the material ID
	for (int i = 0; i < mesh.Elements(); ++i)
	{
		FSElement_& el = mesh.ElementRef(i);
		if (el.m_MatID == nmat)
		{
			el.Show(false);
		}
	}

	// hide faces
	// Faces are hidden if the adjacent element is hidden
	int NF = mesh.Faces();
	for (int i=0; i<NF; ++i)
	{
		FSFace& f = mesh.Face(i);
		if (f.IsExternal())
		{
			if (mesh.ElementRef(f.m_elem[0].eid).IsInvisible()) f.Show(false);
		}
		else
		{
			FSElement_& e0 = mesh.ElementRef(f.m_elem[0].eid);
			FSElement_& e1 = mesh.ElementRef(f.m_elem[1].eid);

			if (e0.IsInvisible() && e1.IsInvisible()) f.Show(false);
		}
	}

	// hide nodes: nodes will be hidden if all elements they attach to are hidden
	int NN = mesh.Nodes();
	for (int i=0; i<NN; ++i) mesh.Node(i).m_ntag = 0;
	for (int i=0; i<mesh.Elements(); ++i)
	{
		FSElement_& el = mesh.ElementRef(i);
		if (el.IsInvisible() == false)
		{
			int ne = el.Nodes();
			for (int j=0; j<ne; ++j) mesh.Node(el.m_node[j]).m_ntag = 1;
		}
	}
	for (int i=0; i<NN; ++i) if (mesh.Node(i).m_ntag == 0) mesh.Node(i).Show(false);

	// hide edges
	// edges are hidden if both nodes are hidden
	int NL = mesh.Edges();
	for (int i=0; i<NL; ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		if ((mesh.Node(edge.n[0]).IsInvisible()) &&
			(mesh.Node(edge.n[1]).IsInvisible())) edge.Show(false);
	}
}

//-----------------------------------------------------------------------------
// Show elements with a certain material ID
void CGLModel::ShowMaterial(int nmat)
{
	FSMesh& mesh = *GetActiveMesh();

	// unhide the elements with mat ID nmat
	int NE = mesh.Elements();
	for (int i=0; i<NE; ++i) 
	{
		FSElement_& e = mesh.ElementRef(i);
		if (e.m_MatID == nmat) mesh.ElementRef(i).Show(true);
	}

	// show faces
	int NF = mesh.Faces();
	for (int i=0; i<NF; ++i)
	{
		FSFace& f = mesh.Face(i);

		if (f.IsExternal())
		{
			if (mesh.ElementRef(f.m_elem[0].eid).IsInvisible() == false) f.Show(true);
		}
		else
		{
			FSElement_& e0 = mesh.ElementRef(f.m_elem[0].eid);
			FSElement_& e1 = mesh.ElementRef(f.m_elem[1].eid);
			if (!e0.IsInvisible() || !e1.IsInvisible()) f.Show(true);
		}
	}

	// show nodes
	int NN = mesh.Nodes();
	for (int i=0; i<NN; ++i) mesh.Node(i).m_ntag = 0;
	for (int i=0; i<mesh.Elements(); ++i)
	{
		FSElement_& el = mesh.ElementRef(i);
		if (el.IsInvisible() == false)
		{
			int ne = el.Nodes();
			for (int j=0; j<ne; ++j) mesh.Node(el.m_node[j]).m_ntag = 1;
		}
	}
	for (int i=0; i<NN; ++i) if (mesh.Node(i).m_ntag == 1) mesh.Node(i).Show(true);

	// show edges
	int NL = mesh.Edges();
	for (int i=0; i<NL; ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		if ((mesh.Node(edge.n[0]).IsInvisible() == false) &&
			(mesh.Node(edge.n[1]).IsInvisible()) == false) edge.Show(true);
	}
}

// Show elements with a certain material ID
void CGLModel::UpdateMeshVisibility()
{
	if (m_postMdl == nullptr) return;

	FSMesh& mesh = *GetActiveMesh();
	Post::FEPostModel& fem = *m_postMdl;

	int NE = mesh.Elements();
	for (int i = 0; i < NE; ++i)
	{
		FSElement_& e = mesh.ElementRef(i);
		if (e.m_MatID >= 0)
		{
			Post::Material* mat = fem.GetMaterial(e.m_MatID);
			e.Show(mat->bvisible);
		}
	}

	// show faces
	int NF = mesh.Faces();
	for (int i = 0; i < NF; ++i)
	{
		FSFace& f = mesh.Face(i);
		bool bshow = false;
		if (f.IsExternal())
		{
			if (mesh.ElementRef(f.m_elem[0].eid).IsInvisible() == false) bshow = true;
		}
		else
		{
			FSElement_& e0 = mesh.ElementRef(f.m_elem[0].eid);
			FSElement_& e1 = mesh.ElementRef(f.m_elem[1].eid);
			if (!e0.IsInvisible() || !e1.IsInvisible()) bshow = true;
		}
		f.Show(bshow);
	}

	// show nodes
	int NN = mesh.Nodes();
	for (int i = 0; i < NN; ++i) mesh.Node(i).m_ntag = 0;
	for (int i = 0; i < mesh.Elements(); ++i)
	{
		FSElement_& el = mesh.ElementRef(i);
		if (el.IsInvisible() == false)
		{
			int ne = el.Nodes();
			for (int j = 0; j < ne; ++j) mesh.Node(el.m_node[j]).m_ntag = 1;
		}
	}
	for (int i = 0; i < NN; ++i)
	{
		FSNode& node = mesh.Node(i);
		node.Show(node.m_ntag == 1);
	}

	// show edges
	int NL = mesh.Edges();
	for (int i = 0; i < NL; ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		if ((mesh.Node(edge.n[0]).IsInvisible() == false) &&
			(mesh.Node(edge.n[1]).IsInvisible()) == false) edge.Show(true);
		else edge.Show(false);
	}

	if (m_postObj) m_postObj->BuildInternalSurfaces();
}

//-----------------------------------------------------------------------------
void CGLModel::SelectElemsInRange(float fmin, float fmax, bool bsel)
{
	FSMesh* pm = GetActiveMesh();
	int N = pm->Elements();
	FEState* state = GetActiveState();
	if (state == nullptr) return;
	for (int i = 0; i<N; ++i)
	{
		FSElement_& el = pm->ElementRef(i);
		if (el.IsEnabled() && el.IsVisible() && ((bsel == false) || (el.IsSelected())))
		{
			float v = state->m_ELEM[i].m_val;
			if ((v >= fmin) && (v <= fmax)) el.Select();
			else el.Unselect();
		}
	}
}

//-----------------------------------------------------------------------------
void CGLModel::SelectNodesInRange(float fmin, float fmax, bool bsel)
{
	FSMesh* pm = GetActiveMesh();
	int N = pm->Nodes();
	FEState* state = GetActiveState();
	if (state == nullptr) return;
	for (int i = 0; i<N; ++i)
	{
		FSNode& node = pm->Node(i);
		if (node.IsEnabled() && node.IsVisible() && ((bsel == false) || (node.IsSelected())))
		{
			float v = state->m_NODE[i].m_val;
			if ((v >= fmin) && (v <= fmax)) node.Select();
			else node.Unselect();
		}
	}
}

//-----------------------------------------------------------------------------
void CGLModel::SelectEdgesInRange(float fmin, float fmax, bool bsel)
{
	FSMesh* pm = GetActiveMesh();
	int N = pm->Edges();
	FEState* state = GetActiveState();
	if (state == nullptr) return;
	for (int i = 0; i<N; ++i)
	{
		FSEdge& edge = pm->Edge(i);
		if (edge.IsEnabled() && edge.IsVisible() && ((bsel == false) || (edge.IsSelected())))
		{
			float v = state->m_EDGE[i].m_val;
			if ((v >= fmin) && (v <= fmax)) edge.Select();
			else edge.Unselect();
		}
	}
}

//-----------------------------------------------------------------------------
void CGLModel::SelectFacesInRange(float fmin, float fmax, bool bsel)
{
	FSMesh* pm = GetActiveMesh();
	FEState* state = GetActiveState();
	if (state == nullptr) return;
	int N = pm->Faces();
	for (int i = 0; i<N; ++i)
	{
		FSFace& f = pm->Face(i);
		if (f.IsEnabled() && f.IsVisible() && ((bsel == false) || (f.IsSelected())))
		{
			float v = state->m_FACE[i].m_val;
			if ((v >= fmin) && (v <= fmax)) f.Select();
			else f.Unselect();
		}
	}
}

//-----------------------------------------------------------------------------
// Hide selected elements
void CGLModel::HideSelectedElements()
{
	FSMesh& mesh = *GetActiveMesh();

	// hide selected elements
	int NE = mesh.Elements();
	for (int i=0; i<NE; i++)
	{
		FSElement_& e = mesh.ElementRef(i);
		if (e.IsSelected()) e.Hide();
	}

	// hide nodes: nodes will be hidden if all elements they attach to are hidden
	int NN = mesh.Nodes();
	for (int i=0; i<NN; ++i) mesh.Node(i).m_ntag = 0;
	for (int i=0; i<mesh.Elements(); ++i)
	{
		FSElement_& el = mesh.ElementRef(i);
		if (el.IsHidden() == false)
		{
			int ne = el.Nodes();
			for (int j=0; j<ne; ++j) mesh.Node(el.m_node[j]).m_ntag = 1;
		}
	}
	for (int i=0; i<NN; ++i) if (mesh.Node(i).m_ntag == 0) mesh.Node(i).Hide();

	// hide faces
	int NF = mesh.Faces();
	for (int i=0; i<NF; ++i)
	{
		FSFace& f = mesh.Face(i);
		if (mesh.ElementRef(f.m_elem[0].eid).IsHidden()) f.Hide();
	}

	// hide edges
	int NL = mesh.Edges();
	for (int i=0; i<NL; ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		FSNode& node0 = mesh.Node(edge.n[0]);
		FSNode& node1 = mesh.Node(edge.n[1]);
		if (node0.IsHidden() || node1.IsHidden()) edge.Hide();
	}

	if (m_postObj) m_postObj->BuildFERenderMesh();
}

//-----------------------------------------------------------------------------
// Hide selected elements
void CGLModel::HideUnselectedElements()
{
	FSMesh& mesh = *GetActiveMesh();

	// hide unselected elements
	int NE = mesh.Elements();
	for (int i=0; i<NE; i++)
	{
		FSElement_& e = mesh.ElementRef(i);
		if (!e.IsSelected()) e.Hide();
	}

	// hide nodes: nodes will be hidden if all elements they attach to are hidden
	int NN = mesh.Nodes();
	for (int i=0; i<NN; ++i) mesh.Node(i).m_ntag = 0;
	for (int i=0; i<mesh.Elements(); ++i)
	{
		FSElement_& el = mesh.ElementRef(i);
		if (el.IsHidden() == false)
		{
			int ne = el.Nodes();
			for (int j=0; j<ne; ++j) mesh.Node(el.m_node[j]).m_ntag = 1;
		}
	}
	for (int i=0; i<NN; ++i) if (mesh.Node(i).m_ntag == 0) mesh.Node(i).Hide();

	// hide faces
	int NF = mesh.Faces();
	for (int i=0; i<NF; ++i)
	{
		FSFace& f = mesh.Face(i);
		if (mesh.ElementRef(f.m_elem[0].eid).IsHidden()) f.Hide();
	}

	// hide edges
	int NL = mesh.Edges();
	for (int i=0; i<NL; ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		FSNode& node0 = mesh.Node(edge.n[0]);
		FSNode& node1 = mesh.Node(edge.n[1]);
		if (node0.IsHidden() || node1.IsHidden()) edge.Hide();
	}
}

//-----------------------------------------------------------------------------
// Hide selected faces
void CGLModel::HideSelectedFaces()
{
	FSMesh& mesh = *GetActiveMesh();
	// hide the faces and the elements that they are attached to
	int NF = mesh.Faces();
	for (int i=0; i<NF; ++i) 
	{
		FSFace& f = mesh.Face(i);
		if (f.IsSelected())
		{
			f.Hide();
			mesh.ElementRef(f.m_elem[0].eid).Hide();
		}
	}

	// hide faces that were hidden by hiding the elements
	for (int i=0; i<NF; ++i)
	{
		FSFace& f = mesh.Face(i);
		if (mesh.ElementRef(f.m_elem[0].eid).IsVisible() == false) f.Hide();
	}

	// hide nodes: nodes will be hidden if all elements they attach to are hidden
	int NN = mesh.Nodes();
	for (int i=0; i<NN; ++i) mesh.Node(i).m_ntag = 0;
	for (int i=0; i<mesh.Elements(); ++i)
	{
		FSElement_& el = mesh.ElementRef(i);
		if (el.IsHidden() == false)
		{
			int ne = el.Nodes();
			for (int j=0; j<ne; ++j) mesh.Node(el.m_node[j]).m_ntag = 1;
		}
	}
	for (int i=0; i<NN; ++i) if (mesh.Node(i).m_ntag == 0) mesh.Node(i).Hide();

	// hide edges
	int NL = mesh.Edges();
	for (int i=0; i<NL; ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		FSNode& node0 = mesh.Node(edge.n[0]);
		FSNode& node1 = mesh.Node(edge.n[1]);
		if (node0.IsHidden() || node1.IsHidden()) edge.Hide();
	}
}

//-----------------------------------------------------------------------------
// hide selected edges
void CGLModel::HideSelectedEdges()
{
	FSMesh& mesh = *GetActiveMesh();

	int NN = mesh.Nodes();
	for (int i=0; i<NN; ++i) mesh.Node(i).m_ntag = -1;

	// hide edges
	int NL = mesh.Edges();
	for (int i=0; i<NL; ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		if (edge.IsSelected()) 
		{
			edge.Hide();
			mesh.Node(edge.n[0]).m_ntag = 1;
			mesh.Node(edge.n[1]).m_ntag = 1;
		}
	}

	// hide surfaces
	FSEdge edge;
	int NF = mesh.Faces();
	for (int i=0; i<NF; ++i)
	{
		FSFace& face = mesh.Face(i);
		int ne = face.Edges();
		for (int j=0; j<ne; ++j)
		{
			edge = face.GetEdge(j);
			if ((mesh.Node(edge.n[0]).m_ntag == 1)&&
				(mesh.Node(edge.n[1]).m_ntag == 1)) 
			{
				// hide the face
				face.Hide();

				// hide the adjacent element
				mesh.ElementRef(face.m_elem[0].eid).Hide();
				break;
			}
		}
	}

	// hide faces that were hidden by hiding the elements
	for (int i=0; i<NF; ++i)
	{
		FSFace& f = mesh.Face(i);
		if (mesh.ElementRef(f.m_elem[0].eid).IsVisible() == false) f.Hide();
	}

	// hide nodes that were hidden by hiding elements
	for (int i=0; i<NN; ++i) mesh.Node(i).m_ntag = 0;
	int NE = mesh.Elements();
	for (int i=0; i<NE; ++i)
	{
		FSElement_& el = mesh.ElementRef(i);
		if (el.IsHidden() == false)
		{
			int ne = el.Nodes();
			for (int j=0; j<ne; ++j) mesh.Node(el.m_node[j]).m_ntag = 1;
		}
	}
	for (int i=0; i<NN; ++i)
		if (mesh.Node(i).m_ntag == 0) mesh.Node(i).Hide();

	// hide edges
	for (int i=0; i<NL; ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		FSNode& node0 = mesh.Node(edge.n[0]);
		FSNode& node1 = mesh.Node(edge.n[1]);
		if (node0.IsHidden() || node1.IsHidden()) edge.Hide();
	}
}

//-----------------------------------------------------------------------------
// hide selected nodes
void CGLModel::HideSelectedNodes()
{
	FSMesh& mesh = *GetActiveMesh();

	// hide nodes and all elements they attach to
	int NN = mesh.Nodes();
	for (int i=0; i<NN; ++i)
	{
		FSNode& n = mesh.Node(i);
		n.m_ntag = 0;
		if (n.IsSelected()) { n.Hide(); n.m_ntag = 1; }
	}

	int NE = mesh.Elements();
	for (int i=0; i<NE; ++i)
	{
		FSElement_& el = mesh.ElementRef(i);
		int ne = el.Nodes();
		for (int j=0; j<ne; ++j) 
		{
			FSNode& node = mesh.Node(el.m_node[j]);
			if (node.IsHidden() && (node.m_ntag == 1)) el.Hide();
		}
	}

	// hide nodes that were hidden by hiding elements
	for (int i=0; i<NE; ++i)
	{
		FSElement_& el = mesh.ElementRef(i);
		if (el.IsHidden())
		{
			int ne = el.Nodes();
			for (int j=0; j<ne; ++j) mesh.Node(el.m_node[j]).Hide();
		}
	}

	// hide faces that were hidden by hiding the elements
	int NF = mesh.Faces();
	for (int i=0; i<NF; ++i)
	{
		FSFace& f = mesh.Face(i);
		if (mesh.ElementRef(f.m_elem[0].eid).IsVisible() == false) f.Hide();
	}

	// hide edges
	int NL = mesh.Edges();
	for (int i=0; i<NL; ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		FSNode& node0 = mesh.Node(edge.n[0]);
		FSNode& node1 = mesh.Node(edge.n[1]);
		if (node0.IsHidden() || node1.IsHidden()) edge.Hide();
	}
}

//-----------------------------------------------------------------------------
void CGLModel::UpdateEdge()
{
	m_edge.Clear();
	FSMesh* mesh = GetActiveMesh();
	if (mesh == nullptr) return;

	for (int i=0; i<mesh->Elements(); ++i)
	{
		FSElement_& el = mesh->ElementRef(i);
		if (el.IsBeam())
		{
			GLEdge::EDGE edge;
			edge.n0 = el.m_node[0];
			edge.n1 = el.m_node[1];
			edge.mat = el.m_MatID;
			edge.tex[0] = edge.tex[1] = 0.f;
			edge.elem = i;
			m_edge.AddEdge(edge);
		}
	}
}

//-----------------------------------------------------------------------------
void CGLModel::UpdateInternalSurfaces(bool eval)
{
	// Build the internal surfaces
	if (m_postObj) m_postObj->BuildInternalSurfaces();

	// reevaluate model
	if (eval) Update(false);
}

//-----------------------------------------------------------------------------
void CGLModel::GetSelectionList(vector<int>& L, int mode)
{
	L.clear();
	FSMesh* m = GetActiveMesh();
	if (m == nullptr) return;
	switch (mode)
	{
	case SELECT_FE_NODES:
	{
		for (int i = 0; i<m->Nodes(); ++i) if (m->Node(i).IsSelected()) L.push_back(i);
	}
	break;
	case SELECT_FE_EDGES:
	{
		for (int i = 0; i<m->Edges(); ++i) if (m->Edge(i).IsSelected()) L.push_back(i);
	}
	break;
	case SELECT_FE_FACES:
	{
		for (int i = 0; i<m->Faces(); ++i) if (m->Face(i).IsSelected()) L.push_back(i);
	}
	break;
	case SELECT_FE_ELEMS:
	{
		for (int i = 0; i<m->Elements(); ++i) if (m->ElementRef(i).IsSelected()) L.push_back(i);
	}
	break;
	}
}

void CGLModel::AddPlot(CGLPlot* pplot, bool update)
{
	pplot->SetModel(this);
	m_pPlot.Add(pplot);
	if (update) pplot->Update(CurrentTimeIndex(), 0.f, true);
}

void CGLModel::RemovePlot(Post::CGLPlot* pplot)
{
	m_pPlot.Remove(pplot);
}

void CGLModel::ClearPlots()
{
	m_pPlot.Clear();
}

void CGLModel::MovePlotUp(Post::CGLPlot* plot)
{
	for (size_t i = 1; i < m_pPlot.Size(); ++i)
	{
		if (m_pPlot[i] == plot)
		{
			CGLPlot* prv = m_pPlot[i - 1];
			m_pPlot.Set(i - 1, plot);
			m_pPlot.Set(i, prv);
			return;
		}
	}
}

void CGLModel::MovePlotDown(Post::CGLPlot* plot)
{
	for (size_t i = 0; i < m_pPlot.Size() - 1; ++i)
	{
		if (m_pPlot[i] == plot)
		{
			CGLPlot* nxt = m_pPlot[i + 1];
			m_pPlot.Set(i, nxt);
			m_pPlot.Set(i + 1, plot);
			return;
		}
	}
}

void CGLModel::UpdateColorMaps()
{
	int N = (int)m_pPlot.Size();
	for (int i = 0; i<N; ++i)
	{
		CGLPlot* p = m_pPlot[i];
		p->UpdateTexture();
	}
}

int CGLModel::DiscreteEdges()
{
	return (int)m_edge.Edges();
}

GLEdge::EDGE& CGLModel::DiscreteEdge(int i)
{
	return m_edge.Edge(i);
}

//=================================================================
GLPlotIterator::GLPlotIterator(CGLModel* mdl)
{
	m_n = 0;
	if (mdl && mdl->Plots())
	{
		for (int i = 0; i < mdl->Plots(); ++i)
		{
			Post::CGLPlot* plot = mdl->Plot(i);
			Post::GLPlotGroup* pg = dynamic_cast<Post::GLPlotGroup*>(plot);
			if (pg)
			{
				for (int j = 0; j<pg->Plots(); ++j)
				{
					m_plt.push_back(pg->GetPlot(j));
				}
			}
			else m_plt.push_back(plot);
		}
	}
}

void GLPlotIterator::operator ++ ()
{
	if ((m_n >= 0) && (m_n <= m_plt.size())) m_n++;
}

GLPlotIterator::operator CGLPlot* ()
{
	if ((m_n >= 0) && (m_n < m_plt.size())) return m_plt[m_n];
	else return nullptr;
}
