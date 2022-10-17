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
#include "GLPlaneCutPlot.h"
#include "PostLib/FEDataManager.h"
#include <GLLib/GLContext.h>
#include "PostLib/constants.h"
#include <GLLib/GLCamera.h>
#include <MeshLib/FENodeEdgeList.h>
#include <GLWLib/GLWidgetManager.h>
#include <GLLib/GLMeshRender.h>
#include <GLLib/glx.h>
#include <stack>
//using namespace std;
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
	m_ps = ps;
	SetName("Model");

	m_lastMesh = nullptr;

	static int layer = 1;
	m_layer = layer++;

	m_stol = 60.0;

	CGLWidgetManager::GetInstance()->SetActiveLayer(m_layer);

	m_bnorm = false;
	m_scaleNormals = 1.0;
	m_bghost = false;
	m_nDivs = 0; // this means "auto"
	m_brenderInteriorNodes = true;

	m_doZSorting = true;

	m_brenderPlotObjects = true;

	m_bshowMesh = true;

	m_line_col = GLColor(0, 0, 0);
	m_node_col = GLColor(0, 0, 255);
	m_sel_col = GLColor(255, 0, 0);

	m_nrender = RENDER_MODE_SOLID;

	m_nconv = CONV_FR_XZ;

	m_selectMode = SELECT_ELEMS;
	m_selectStyle = SELECT_RECT;

	m_pcol = nullptr;
	m_pdis = nullptr;

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
		if ((*pd)->Type() == DATA_VEC3F)
		{
			std::string sname = (*pd)->GetName();
			if ((sname == "displacement") || (sname == "Displacement")) ndisp = i;
		}
	}

	if (ndisp != -1)
	{
		ps->SetDisplacementField(BUILD_FIELD(1, ndisp, 0));
		m_pdis = new CGLDisplacementMap(this);
	}

	// add a default color map
	m_pcol = new CGLColorMap(this);

	UpdateEdge();
	BuildInternalSurfaces();
	Update(false);
}

//-----------------------------------------------------------------------------
//! destructor
CGLModel::~CGLModel(void)
{
	delete m_pdis;
	delete m_pcol;
	ClearInternalSurfaces();
}

//-----------------------------------------------------------------------------
void CGLModel::SetFEModel(FEPostModel* ps)
{
	ClearSelectionLists();
	ClearInternalSurfaces();
	m_ps = ps;
	if (ps) BuildInternalSurfaces();
}

//-----------------------------------------------------------------------------
void CGLModel::ShowShell2Solid(bool b) { m_render.m_bShell2Solid = b; }
bool CGLModel::ShowShell2Solid() const { return m_render.m_bShell2Solid; }

//-----------------------------------------------------------------------------
int CGLModel::ShellReferenceSurface() const { return m_render.m_nshellref; }
void CGLModel::ShellReferenceSurface(int n) { m_render.m_nshellref = n; }

//-----------------------------------------------------------------------------
Post::FEPostMesh* CGLModel::GetActiveMesh()
{
	FEPostModel* pfem = GetFSModel();
	if (pfem)
	{
		if (pfem->GetStates() > 0) return m_ps->CurrentState()->GetFEMesh();
		return pfem->GetFEMesh(0);
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
Post::FEState* CGLModel::GetActiveState()
{
	FEPostModel* pfem = GetFSModel();
	if (pfem && (pfem->GetStates() > 0)) return m_ps->CurrentState();
	return nullptr;
}

//-----------------------------------------------------------------------------
void CGLModel::ResetAllStates()
{
	FEPostModel* fem = GetFSModel();
	if ((fem == 0) || (fem->GetStates() == 0)) return;

	int N = fem->GetStates();
	for (int i=0; i<N; ++i)
	{
		FEState* ps = fem->GetState(i);
		ps->m_nField = -1;
	}
}

//-----------------------------------------------------------------------------
float CGLModel::CurrentTime() const { return (m_ps ? m_ps->CurrentTime() : 0.f); }

//-----------------------------------------------------------------------------
int CGLModel::CurrentTimeIndex() const { return (m_ps ? m_ps->CurrentTimeIndex() : -1); }

//-----------------------------------------------------------------------------
void CGLModel::SetCurrentTimeIndex(int ntime)
{
	if (m_ps && m_ps->GetStates()) m_ps->SetCurrentTimeIndex(ntime);
}

//-----------------------------------------------------------------------------
void CGLModel::SetTimeValue(float ftime)
{
	if (m_ps && m_ps->GetStates()) m_ps->SetTimeValue(ftime);
}

//-----------------------------------------------------------------------------
// Update the model data
bool CGLModel::Update(bool breset)
{
	if (m_ps == nullptr) return true;

	FEPostModel& fem = *m_ps;
	if (fem.GetStates() == 0) return true;

	// get the time inc value
	int ntime = fem.CurrentTimeIndex();
	float dt = fem.CurrentTime() - fem.GetTimeValue(ntime);

	// update the state of the mesh
	GetFSModel()->UpdateMeshState(ntime);

	// Calling this will rebuild the internal surfaces
	// This should only be done when the mesh has changed
	Post::FEPostMesh* currentMesh = fem.CurrentState()->GetFEMesh();
	if (breset || (currentMesh != m_lastMesh))
	{
		UpdateInternalSurfaces(false);
		m_lastMesh = currentMesh;
	}

	// update displacement map
	if (m_pdis && m_pdis->IsActive()) m_pdis->Update(ntime, dt, breset);

	// update the colormap
	if (m_pcol && m_pcol->IsActive()) m_pcol->Update(ntime, dt, breset);

	// NOTE: commenting this out since this would cause the FieldDataSelector's menu
	//       to be rebuild each time a user selected a new field
//	GetFSModel()->UpdateDependants();

	// update the plot list
	for (int i = 0; i < (int)m_pPlot.Size(); ++i)
	{
		CGLPlot* pi = m_pPlot[i];
		if (pi->IsActive()) pi->Update(ntime, dt, breset);
	}

	return true;
}

//-----------------------------------------------------------------------------
void CGLModel::UpdateDisplacements(int nstate, bool breset)
{
	if (m_pdis && m_pdis->IsActive()) m_pdis->Update(nstate, 0.f, breset);
}

//-----------------------------------------------------------------------------
void CGLModel::SetMaterialParams(Material* pm)
{
	GLfloat fv[4] = {0,0,0,1};
	const float f = 1.f / 255.f;

	GLubyte a = (GLubyte) (255.f*pm->transparency);

	glColor4ub(pm->diffuse.r, pm->diffuse.g, pm->diffuse.b, a);

/*	fv[0] = (float) pm->ambient.r*f;
	fv[1] = (float) pm->ambient.g*f;
	fv[2] = (float) pm->ambient.b*f;
	fv[3] = (float) a*f;
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, fv);
*/

	fv[0] = (float) pm->specular.r*f;
	fv[1] = (float) pm->specular.g*f;
	fv[2] = (float) pm->specular.b*f;
	fv[3] = 1.f;
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, fv);

	fv[0] = (float) pm->emission.r*f;
	fv[1] = (float) pm->emission.g*f;
	fv[2] = (float) pm->emission.b*f;
	fv[3] = 1.f;
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, fv);

	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, pm->shininess*64.f);
}

//-----------------------------------------------------------------------------
void CGLModel::SetSmoothingAngle(double w)
{ 
	m_stol = w;

	FEPostModel* ps = GetFSModel();
	if (ps == 0) return;

	FSMeshBase* pm = ps->GetFEMesh(0);
	pm->AutoSmooth(m_stol);
}

//-----------------------------------------------------------------------------
bool CGLModel::AddDisplacementMap(const char* szvectorField)
{
	if (szvectorField == nullptr) szvectorField = "displacement";

	FEPostModel* ps = GetFSModel();

	// see if the mesh has any vector fields
	// which can be used for displacement maps
	FEDataManager* pdm = ps->GetDataManager();
	FEDataFieldPtr pd = pdm->FirstDataField();
	int nv = 0;
	int ndisp = -1;
	for (int i=0; i<pdm->DataFields(); ++i, ++pd)
	{
		if ((*pd)->Type() == DATA_VEC3F) ++nv;
		if ((*pd)->GetName() == szvectorField) ndisp = i;
	}

	if (nv == 0) return false;

	if (m_pdis) delete m_pdis;
	m_pdis = new CGLDisplacementMap(this);
	if (ndisp != -1)
	{
		ps->SetDisplacementField(BUILD_FIELD(1, ndisp, 0));
	}
	else ps->SetDisplacementField(0);

	ResetAllStates();

	return true;
}

//-----------------------------------------------------------------------------
bool CGLModel::HasDisplacementMap()
{
	if (m_pdis == 0) return false;
	return (GetFSModel()->GetDisplacementField() != 0);
}

//-----------------------------------------------------------------------------
void CGLModel::ResetMesh()
{
	FEPostModel& fem = *GetFSModel();
	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);

	Post::FERefState& ref = *fem.GetState(0)->m_ref;

	int NN = mesh.Nodes();
	for (int i = 0; i<NN; ++i)
	{
		FSNode& node = mesh.Node(i);
		node.r = to_vec3d(ref.m_Node[i].m_rt);
	}

	// reevaluate normals
	mesh.UpdateNormals();
}

//-----------------------------------------------------------------------------
//! Toggle element visibility
void CGLModel::ToggleVisibleElements()
{
	FEPostModel& fem = *GetFSModel();
	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);

	for (int i = 0; i < mesh.Elements(); ++i)
	{
		FSElement& el = mesh.Element(i);
		if (el.IsVisible()) el.Hide(); else {
			el.Show();
			el.Unhide();
		}
	}

	// nodes will be hidden if all elements they attach to are hidden
	int NN = mesh.Nodes();
	for (int i = 0; i<NN; ++i) mesh.Node(i).m_ntag = 0;
	for (int i = 0; i<mesh.Elements(); ++i)
	{
		FEElement_& el = mesh.ElementRef(i);
		if (el.IsHidden() == false)
		{
			int ne = el.Nodes();
			for (int j = 0; j<ne; ++j) mesh.Node(el.m_node[j]).m_ntag = 1;
		}
	}

	for (int i = 0; i < NN; ++i)
	{
		FSNode& node = mesh.Node(i);
		if (node.m_ntag == 0) mesh.Node(i).Hide();
		else
		{
			node.Unhide();
			node.Show();
		}
	}

	// hide faces
	int NF = mesh.Faces();
	for (int i = 0; i<NF; ++i)
	{
		FSFace& f = mesh.Face(i);
		if (f.IsExternal())
		{
			if (mesh.ElementRef(f.m_elem[0].eid).IsHidden()) f.Hide();
			else { f.Show(); f.Unhide(); }
		}
		else
		{
			if (mesh.ElementRef(f.m_elem[0].eid).IsHidden() && 
				mesh.ElementRef(f.m_elem[1].eid).IsHidden()) f.Hide();
			else
			{
				f.Show();
				f.Unhide();
			}
		}
	}

	// hide edges
	int NL = mesh.Edges();
	for (int i = 0; i<NL; ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		FSNode& node0 = mesh.Node(edge.n[0]);
		FSNode& node1 = mesh.Node(edge.n[1]);
		if (node0.IsHidden() || node1.IsHidden()) edge.Hide();
		else
		{
			edge.Show();
			edge.Unhide();
		}
	}
	UpdateSelectionLists();
}

//-----------------------------------------------------------------------------
void CGLModel::RemoveDisplacementMap()
{
	FEPostModel* ps = GetFSModel();
	ps->SetDisplacementField(0);
	delete m_pdis;
	m_pdis = 0;

	// reset the mesh
	ResetMesh();

	// just to be safe, let's reset all states to force them to reevaluate
	ResetAllStates();
}

//-----------------------------------------------------------------------------
void CGLModel::Render(CGLContext& rc)
{
	if (GetFSModel() == nullptr) return;

	// activate all clipping planes
	CGLPlaneCutPlot::EnableClipPlanes();

	// first we render all the plots
	RenderPlots(rc, 0);

	// activate all clipping planes
	CGLPlaneCutPlot::EnableClipPlanes();

	// set the render interior nodes flag
	RenderInteriorNodes(rc.m_bext == false);

	// get the FE model
	FEPostModel* fem = GetFSModel();

	m_bshowMesh = rc.m_showMesh;

	// Render discrete elements
	float lineWidth;
	glGetFloatv(GL_LINE_WIDTH, &lineWidth);
	glLineWidth(rc.m_springThick);
	RenderDiscrete(rc);
	glLineWidth(lineWidth);

	int mode = GetSelectionMode();

	// render the faces
	if (mode == SELECT_FACES)
	{
		RenderFaces(m_ps, rc);
	}
	else if (mode == SELECT_ELEMS)
	{
		RenderElems(m_ps, rc);
	}
	else
	{
		// for nodes, edges, draw the faces as well
		RenderSurface(m_ps, rc);
	}

	// render outline
	if (rc.m_showOutline)
	{
		rc.m_cam->LineDrawMode(true);
		RenderOutline(rc);
		rc.m_cam->LineDrawMode(false);
	}

	// render the selected elements and faces
	RenderSelection(rc);

	// render the normals
	if (m_bnorm) RenderNormals(rc);

	// render the ghost
	if (m_bghost) RenderGhost(rc);

	// render the edges
	if (mode == SELECT_EDGES)
	{
		rc.m_cam->LineDrawMode(true);
		RenderEdges(fem, rc);
		rc.m_cam->LineDrawMode(false);
	}

	// render the nodes
	if (mode == SELECT_NODES)
	{
		rc.m_cam->LineDrawMode(true);
		RenderNodes(fem, rc);
		rc.m_cam->LineDrawMode(false);
	}

	// first render all the plots that need to be rendered after the model
	// (i.e. planecuts)
	RenderPlots(rc, 1);

	// render min/max markers
	Post::CGLColorMap* pcm = GetColorMap();
	if (pcm && pcm->ShowMinMaxMarkers())
	{
		RenderMinMaxMarkers(rc);
	}

	// render decorations
	RenderDecorations();

	// render all the objects
	if (m_brenderPlotObjects) RenderObjects(rc);
}

//-----------------------------------------------------------------------------
void CGLModel::RenderPlots(CGLContext& rc, int renderOrder)
{
	GPlotList& PL = m_pPlot;
	// clear all clipping planes
	CGLPlaneCutPlot::ClearClipPlanes();
	for (int i = 0; i < (int)PL.Size(); ++i)
	{
		CGLPlaneCutPlot* p = dynamic_cast<CGLPlaneCutPlot*>(m_pPlot[i]);
		if (p && p->IsActive()) p->Activate(true);
	}
	
	for (int i = 0; i<(int)PL.Size(); ++i)
	{
		CGLPlot* pl = m_pPlot[i];

		if (pl->AllowClipping()) CGLPlaneCutPlot::EnableClipPlanes();
		else CGLPlaneCutPlot::DisableClipPlanes();

		if (pl->IsActive() && (pl->GetRenderOrder() == renderOrder)) pl->Render(rc);
	}
	CGLPlaneCutPlot::DisableClipPlanes();
}

//-----------------------------------------------------------------------------
void CGLModel::RenderDiscrete(CGLContext& rc)
{
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	Post::FEPostMesh& mesh = *GetActiveMesh();
	int curMat = -1;
	bool bvisible = true;

	// render un-selected, active elements
	if (m_pcol->IsActive())
	{
		glEnable(GL_TEXTURE_1D);

		glColor3ub(255, 255, 255);
		glBegin(GL_LINES);
		for (int i = 0; i < m_edge.Edges(); ++i)
		{
			GLEdge::EDGE& edge = m_edge.Edge(i);
			FEElement_* pe = mesh.ElementPtr(edge.elem);
			if (pe && !pe->IsSelected() && pe->IsVisible())
			{
				int mat = edge.mat;
				if (mat != curMat)
				{
					Material* pmat = m_ps->GetMaterial(mat);
					curMat = mat;
					bvisible = pmat->bvisible;
					if (!pmat->benable) bvisible = false;
				}

				if (bvisible)
				{
					vec3d r0 = mesh.Node(edge.n0).r;
					vec3d r1 = mesh.Node(edge.n1).r;

					float t0 = edge.tex[0];
					float t1 = edge.tex[1];

					glTexCoord1d(t0); glVertex3d(r0.x, r0.y, r0.z);
					glTexCoord1d(t1); glVertex3d(r1.x, r1.y, r1.z);
				}
			}
		}
		glEnd();
	}

	// turn-off texturing for the rest
	glDisable(GL_TEXTURE_1D);

	// loop over un-selected, inactive elements
	curMat = -1;
	glBegin(GL_LINES);
	for (int i=0; i<m_edge.Edges(); ++i)
	{
		GLEdge::EDGE& edge = m_edge.Edge(i);
		FEElement_* pe = mesh.ElementPtr(edge.elem);
		if (pe && !pe->IsSelected() && pe->IsVisible())
		{
			int mat = edge.mat;
			if (mat != curMat)
			{
				Material* pmat = m_ps->GetMaterial(mat);
				GLColor c = pmat->diffuse;
				glColor3ub(c.r, c.g, c.b);
				curMat = mat;
				bvisible = pmat->bvisible;
				if (m_pcol->IsActive() && pmat->benable) bvisible = false;
			}

			if (bvisible)
			{
				vec3d r0 = mesh.Node(edge.n0).r;
				vec3d r1 = mesh.Node(edge.n1).r;
				glVertex3d(r0.x, r0.y, r0.z);
				glVertex3d(r1.x, r1.y, r1.z);
			}
		}
	}
	glEnd();

	// loop over selected elements
	glColor3ub(255, 0, 0);
	glBegin(GL_LINES);
	for (int i = 0; i < m_edge.Edges(); ++i)
	{
		GLEdge::EDGE& edge = m_edge.Edge(i);
		FEElement_* pe = mesh.ElementPtr(edge.elem);
		if (pe && pe->IsSelected() && pe->IsVisible())
		{
			vec3d r0 = mesh.Node(edge.n0).r;
			vec3d r1 = mesh.Node(edge.n1).r;

			glVertex3d(r0.x, r0.y, r0.z);
			glVertex3d(r1.x, r1.y, r1.z);
		}
	}
	glEnd();

	glPopAttrib();
}

//-----------------------------------------------------------------------------
void CGLModel::RenderFaces(FEPostModel* ps, CGLContext& rc)
{
	// get the mesh
	Post::FEPostMesh* pm = GetActiveMesh();

	// we render the mesh by looping over the materials
	// first we render the opaque meshes
	for (int m=0; m<ps->Materials(); ++m)
	{
		// get the material
		Material* pmat = ps->GetMaterial(m);

		// make sure the material is visible
		if (pmat->bvisible && (pmat->transparency>.99f)) 
		{
			RenderSolidPart(ps, rc, m);
		}
	}

	// next, we render the transparent meshes
	for (int m=0; m<ps->Materials(); ++m)
	{
		// get the material
		Material* pmat = ps->GetMaterial(m);

		// make sure the material is visible
		if (pmat->bvisible && (pmat->transparency<=.99f) && (pmat->transparency>0.001f)) 
		{
			RenderSolidPart(ps, rc, m);
		}
	}
}

//-----------------------------------------------------------------------------
void CGLModel::RenderElems(FEPostModel* ps, CGLContext& rc)
{
	// get the mesh
	Post::FEPostMesh* pm = GetActiveMesh();

	// we render the mesh by looping over the materials
	// first we render the opaque meshes
	for (int m = 0; m<ps->Materials(); ++m)
	{
		// get the material
		Material* pmat = ps->GetMaterial(m);

		// make sure the material is visible
		if (pmat->bvisible && (pmat->transparency>.99f))
		{
			RenderSolidPart(ps, rc, m);
		}
	}

	// next, we render the transparent meshes
	for (int m = 0; m<ps->Materials(); ++m)
	{
		// get the material
		Material* pmat = ps->GetMaterial(m);

		// make sure the material is visible
		if (pmat->bvisible && (pmat->transparency <= .99f) && (pmat->transparency>0.001f))
		{
			RenderSolidPart(ps, rc, m);
		}
	}
}

//-----------------------------------------------------------------------------
void CGLModel::RenderSurface(FEPostModel* ps, CGLContext& rc)
{
	// get the mesh
	Post::FEPostMesh* pm = GetActiveMesh();

	// we render the mesh by looping over the materials
	// first we render the opaque meshes
	for (int m = 0; m<ps->Materials(); ++m)
	{
		// get the material
		Material* pmat = ps->GetMaterial(m);

		// make sure the material is visible
		if (pmat->bvisible && (pmat->transparency>.99f))
		{
			RenderSolidPart(ps, rc, m);
		}
	}

	// next, we render the transparent meshes
	for (int m = 0; m<ps->Materials(); ++m)
	{
		// get the material
		Material* pmat = ps->GetMaterial(m);

		// make sure the material is visible
		if (pmat->bvisible && (pmat->transparency <= .99f) && (pmat->transparency>0.001f))
		{
			RenderSolidPart(ps, rc, m);
		}
	}
}

//-----------------------------------------------------------------------------

void CGLModel::RenderSelection(CGLContext &rc)
{
	int mode = GetSelectionMode();

	// get the mesh
	FEPostModel* ps = m_ps;
	Post::FEPostMesh* pm = GetActiveMesh();

	glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT);

	// now render the selected faces
	glDisable(GL_TEXTURE_1D);
	GLColor c = m_sel_col;
	glColor4ub(c.r,c.g,c.g,128);
	glDisable(GL_LIGHTING);

	int ndivs = GetSubDivisions();
	m_render.SetDivisions(ndivs);

	// render the selected faces
	if (mode == SELECT_FACES)
	{
		glBegin(GL_TRIANGLES);
		for (int i=0; i<pm->Faces(); ++i)
		{
			FSFace& face = pm->Face(i);
			if (face.IsSelected())
			{
				// okay, we got one, so let's render it
				m_render.RenderFace(face, pm);
			}
		}
		glEnd();
	}

	// render the selected elements
	if (mode == SELECT_ELEMS)
	{
		glBegin(GL_TRIANGLES);
		for (int i = 0; i<pm->Faces(); ++i)
		{
			FSFace& face = pm->Face(i);
			FEElement_& el = pm->ElementRef(face.m_elem[0].eid);
			if (el.IsSelected())
			{
				// okay, we got one, so let's render it
				m_render.RenderFace(face, pm);
			}
		}
		glEnd();
	}

	// render the outline of the selected elements
	glDisable(GL_DEPTH_TEST);
	glColor3ub(255,255,0);

	// do the selected elements first
	if (mode == SELECT_ELEMS)
	{
		const vector<FEElement_*> elemSelection = GetElementSelection();
		for (int i = 0; i<(int)elemSelection.size(); ++i)
		{
			FEElement_& el = *elemSelection[i]; assert(el.IsSelected());
			m_render.RenderElementOutline(el, pm, ndivs);
		}
	}

	// now do the selected faces
	if (mode == SELECT_FACES)
	{
		vec3f r[FSFace::MAX_NODES];
		const vector<FSFace*> faceSelection = GetFaceSelection();
		for (int i = 0; i<(int)faceSelection.size(); ++i)
		{
			FSFace& f = *faceSelection[i]; 
			if (f.IsSelected() == false) continue;

			int n = f.Nodes();
			for (int j=0; j<n; ++j) r[j] = to_vec3f(pm->Node(f.n[j]).r);
			switch (f.m_type)
			{
			case FE_FACE_TRI3:
				glBegin(GL_LINE_LOOP);
				{
					glVertex3f(r[0].x, r[0].y, r[0].z);
					glVertex3f(r[1].x, r[1].y, r[1].z);
					glVertex3f(r[2].x, r[2].y, r[2].z);
				}
				glEnd();
				break;
			case FE_FACE_QUAD4:
				glBegin(GL_LINE_LOOP);
				{
					glVertex3f(r[0].x, r[0].y, r[0].z);
					glVertex3f(r[1].x, r[1].y, r[1].z);
					glVertex3f(r[2].x, r[2].y, r[2].z);
					glVertex3f(r[3].x, r[3].y, r[3].z);
				}
				glEnd();
				break;
			case FE_FACE_TRI6:
				glBegin(GL_LINE_LOOP);
				{
					RenderFace2Outline(pm, f, ndivs);
				}
				glEnd();
				break;
			case FE_FACE_TRI7:
				glBegin(GL_LINE_LOOP);
				{
					glVertex3f(r[0].x, r[0].y, r[0].z);
					glVertex3f(r[3].x, r[3].y, r[3].z);
					glVertex3f(r[1].x, r[1].y, r[1].z);
					glVertex3f(r[4].x, r[4].y, r[4].z);
					glVertex3f(r[2].x, r[2].y, r[2].z);
					glVertex3f(r[5].x, r[5].y, r[5].z);
				}
				glEnd();
				break;
			case FE_FACE_TRI10:
				glBegin(GL_LINE_LOOP);
				{
					glVertex3f(r[0].x, r[0].y, r[0].z);
					glVertex3f(r[3].x, r[3].y, r[3].z);
					glVertex3f(r[4].x, r[4].y, r[4].z);
					glVertex3f(r[1].x, r[1].y, r[1].z);
					glVertex3f(r[5].x, r[5].y, r[5].z);
					glVertex3f(r[6].x, r[6].y, r[6].z);
					glVertex3f(r[2].x, r[2].y, r[2].z);
					glVertex3f(r[8].x, r[8].y, r[8].z);
					glVertex3f(r[7].x, r[7].y, r[7].z);
				}
				glEnd();
				break;
			case FE_FACE_QUAD8:
				glBegin(GL_LINE_LOOP);
				{
					glVertex3f(r[0].x, r[0].y, r[0].z);
					glVertex3f(r[4].x, r[4].y, r[4].z);
					glVertex3f(r[1].x, r[1].y, r[1].z);
					glVertex3f(r[5].x, r[5].y, r[5].z);
					glVertex3f(r[2].x, r[2].y, r[2].z);
					glVertex3f(r[6].x, r[6].y, r[6].z);
					glVertex3f(r[3].x, r[3].y, r[3].z);
					glVertex3f(r[7].x, r[7].y, r[7].z);
				}
				glEnd();
				break;
			case FE_FACE_QUAD9:
				glBegin(GL_LINE_LOOP);
				{
					glVertex3f(r[0].x, r[0].y, r[0].z);
					glVertex3f(r[4].x, r[4].y, r[4].z);
					glVertex3f(r[1].x, r[1].y, r[1].z);
					glVertex3f(r[5].x, r[5].y, r[5].z);
					glVertex3f(r[2].x, r[2].y, r[2].z);
					glVertex3f(r[6].x, r[6].y, r[6].z);
					glVertex3f(r[3].x, r[3].y, r[3].z);
					glVertex3f(r[7].x, r[7].y, r[7].z);
				}
				glEnd();
				break;
			default:
				assert(false);
			}
		}
	}

	glPopAttrib();
}

//-----------------------------------------------------------------------------

void CGLModel::RenderTransparentMaterial(CGLContext& rc, FEPostModel* ps, int m)
{
	Material* pmat = ps->GetMaterial(m);
	Post::FEPostMesh* pm = GetActiveMesh();

	int transMode = pmat->m_ntransmode;

	// get the camera's orientation
	quatd q = rc.m_cam->GetOrientation();

	// make sure a part with this material exists
	if (m >= pm->Parts()) return;

	glPushAttrib(GL_ENABLE_BIT);

	// set the material properties
	bool benable = false;
	if (pmat->benable && m_pcol->IsActive())
	{
		benable = true;
		glEnable(GL_TEXTURE_1D);
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		GLubyte a = (GLubyte) (255.f*pmat->transparency);
		glColor4ub(255,255,255,a);
		m_pcol->GetColorMap()->GetTexture().MakeCurrent();
	}
	else
	{
		glDisable(GL_TEXTURE_1D);
		SetMaterialParams(pmat);
	}

	// see if we allow the model to be clipped
	if (pmat->bclip == false) CGLPlaneCutPlot::DisableClipPlanes();

	// render the unselected faces
	MeshDomain& dom = pm->Domain(m);
	int NF = dom.Faces();

	// for better transparency we first draw all the backfacing polygons.
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_CULL_FACE);

	GLColor d = pmat->diffuse;
	GLColor c[4];
	double tm = pmat->transparency;

	int ndivs = GetSubDivisions();

	int mode = GetSelectionMode();

	if (m_doZSorting)
	{
		glDisable(GL_CULL_FACE);

		vector< pair<int, double> > zlist; zlist.reserve(NF);
		// first, build a list of faces
		for (int i = 0; i < NF; ++i)
		{
			FSFace& face = dom.Face(i);
			FEElement_& el = pm->ElementRef(face.m_elem[0].eid);

			if (((mode != SELECT_ELEMS) || !el.IsSelected()) && face.IsVisible())
			{
				// get the face center
				vec3d r = pm->FaceCenter(face);

				// convert to eye coordinates
				vec3d q = rc.m_cam->WorldToCam(r);

				// add it to the z-list
				zlist.push_back(pair<int, double>(i, q.z));
			}
		}

		// sort the zlist
		std::sort(zlist.begin(), zlist.end(), [](pair<int, double>& a, pair<int, double>& b) {
			return a.second < b.second;
		});

		// render the list
		for (int i = 0; i < zlist.size(); ++i)
		{
			FSFace& face = dom.Face(zlist[i].first);

			GLubyte a[4] = { 255, 255, 255, 255 };
			if (transMode == RENDER_TRANS_NORMAL_WEIGHTED)
			{
				for (int j = 0; j < face.Nodes(); ++j)
				{
					vec3d r = to_vec3d(face.m_nn[j]);
					q.RotateVector(r);
					double z = 1 - fabs(r.z);
					a[j] = (GLubyte)(255 * (tm + 0.5 * (1 - tm) * (z * z)));
				}
			}
			else if (benable && (transMode == RENDER_TRANS_VALUE_WEIGHTED))
			{
				for (int j = 0; j < face.Nodes(); ++j)
				{
					float texj = face.m_tex[j];
					if (texj < 0.f) texj = 0.f;
					if (texj > 1.f) texj = 1.f;

					vec3d r = to_vec3d(face.m_nn[j]);
					q.RotateVector(r);
					double z = 1 - fabs(r.z);
					float f = tm*(z * z);

					float w = texj + (1.f - texj) * f;
					a[j] = (GLubyte)(255.f * w);
				}
			}

			if (benable)
			{
				c[0] = GLColor(255, 255, 255, a[0]);
				c[1] = GLColor(255, 255, 255, a[1]);
				c[2] = GLColor(255, 255, 255, a[2]);
				c[3] = GLColor(255, 255, 255, a[3]);
			}
			else
			{
				c[0] = GLColor(d.r, d.g, d.b, a[0]);
				c[1] = GLColor(d.r, d.g, d.b, a[1]);
				c[2] = GLColor(d.r, d.g, d.b, a[2]);
				c[3] = GLColor(d.r, d.g, d.b, a[3]);
			}

			// okay, we got one, so let's render it
			m_render.RenderFace(face, pm, c, ndivs);
		}
	}
	else
	{
		glCullFace(GL_FRONT);
		for (int i = 0; i < NF; ++i)
		{
			FSFace& face = dom.Face(i);
			FEElement_& el = pm->ElementRef(face.m_elem[0].eid);

			if (((mode != SELECT_ELEMS) || !el.IsSelected()) && face.IsVisible())
			{
				GLubyte a[4];
				for (int j = 0; j < face.Nodes(); ++j)
				{
					vec3d r = to_vec3d(face.m_nn[j]);
					q.RotateVector(r);
					double z = 1 - fabs(r.z);
					a[j] = (GLubyte)(255 * (tm + 0.5*(1 - tm)*(z*z)));
				}

				if (benable)
				{
					c[0] = GLColor(255, 255, 255, a[0]);
					c[1] = GLColor(255, 255, 255, a[1]);
					c[2] = GLColor(255, 255, 255, a[2]);
					c[3] = GLColor(255, 255, 255, a[3]);
				}
				else
				{
					c[0] = GLColor(d.r, d.g, d.b, a[0]);
					c[1] = GLColor(d.r, d.g, d.b, a[1]);
					c[2] = GLColor(d.r, d.g, d.b, a[2]);
					c[3] = GLColor(d.r, d.g, d.b, a[3]);
				}

				// okay, we got one, so let's render it
				m_render.RenderFace(face, pm, c, ndivs);
			}
		}

		// and then we draw the front-facing ones.
		glCullFace(GL_BACK);
		for (int i = 0; i < NF; ++i)
		{
			FSFace& face = dom.Face(i);
			FEElement_& el = pm->ElementRef(face.m_elem[0].eid);

			if (((mode != SELECT_ELEMS) || !el.IsSelected()) && face.IsVisible())
			{
				GLubyte a[4];
				for (int j = 0; j < face.Nodes(); ++j)
				{
					vec3d r = to_vec3d(face.m_nn[j]);
					q.RotateVector(r);
					double z = 1 - fabs(r.z);
					a[j] = (GLubyte)(255 * (tm + 0.5*(1 - tm)*(z*z)));
				}

				if (benable)
				{
					c[0] = GLColor(255, 255, 255, a[0]);
					c[1] = GLColor(255, 255, 255, a[1]);
					c[2] = GLColor(255, 255, 255, a[2]);
					c[3] = GLColor(255, 255, 255, a[3]);
				}
				else
				{
					c[0] = GLColor(d.r, d.g, d.b, a[0]);
					c[1] = GLColor(d.r, d.g, d.b, a[1]);
					c[2] = GLColor(d.r, d.g, d.b, a[2]);
					c[3] = GLColor(d.r, d.g, d.b, a[3]);
				}

				// okay, we got one, so let's render it
				m_render.RenderFace(face, pm, c, ndivs);
			}
		}
	}

	glPopAttrib();

	// reset the polygon mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// render the internal faces
	RenderInnerSurface(m);

	if (pmat->benable && m_pcol->IsActive())
	{
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	}

	glPopAttrib();
}

//-----------------------------------------------------------------------------
void CGLModel::RenderInnerSurface(int m, bool btex)
{
//	m_render.SetDivisions(1);
	Post::FEPostMesh* pm = GetActiveMesh();
	GLSurface& surf = *m_innerSurface[m];

	// render active faces
	if (btex) glEnable(GL_TEXTURE_1D);
	glBegin(GL_TRIANGLES);
	for (int i = 0; i<surf.Faces(); ++i)
	{
		FSFace& face = surf.Face(i);
		if (face.IsActive())
		{
			m_render.RenderFace(face, pm);
		}
	}
	glEnd();

	// render inactive faces
	if (btex) glDisable(GL_TEXTURE_1D);
	glBegin(GL_TRIANGLES);
	for (int i = 0; i<surf.Faces(); ++i)
	{
		FSFace& face = surf.Face(i);
		if (face.IsActive() == false)
		{
			m_render.RenderFace(face, pm);
		}
	}
	glEnd();

	if (btex) glEnable(GL_TEXTURE_1D);
}

//-----------------------------------------------------------------------------
void CGLModel::RenderInnerSurfaceOutline(int m, int ndivs)
{
	m_render.SetDivisions(ndivs);
	Post::FEPostMesh* pm = GetActiveMesh();
	GLSurface& inSurf = *m_innerSurface[m];
	for (int i = 0; i<inSurf.Faces(); ++i)
	{
		FSFace& facet = inSurf.Face(i);
		m_render.RenderFaceOutline(facet, pm, ndivs);
	}
}

//-----------------------------------------------------------------------------
void CGLModel::RenderSolidDomain(CGLContext& rc, MeshDomain& dom, bool btex, bool benable, bool zsort, bool activeOnly)
{
	FEPostMesh* pm = GetActiveMesh();
	int ndivs = GetSubDivisions();
	m_render.SetDivisions(ndivs);

	if (btex) glEnable(GL_TEXTURE_1D);

	// render active faces
	if (zsort)
	{
		int NF = dom.Faces();
		vector< pair<int, double> > zlist; zlist.reserve(NF);
		for (int i = 0; i < NF; ++i)
		{
			FSFace& face = dom.Face(i);
			if (face.m_ntag == 1)
			{
				// get the face center
				vec3d r = pm->FaceCenter(face);

				// convert to eye coordinates
				vec3d q = rc.m_cam->WorldToCam(r);

				// add it to the z-list
				zlist.push_back(pair<int, double>(i, q.z));
			}
		}

		// sort the zlist
		std::sort(zlist.begin(), zlist.end(), [](pair<int, double>& a, pair<int, double>& b) {
			return a.second < b.second;
		});

		// render the list
		glBegin(GL_TRIANGLES);
		for (int i = 0; i < zlist.size(); ++i)
		{
			FSFace& face = dom.Face(zlist[i].first);
			m_render.RenderFace(face, pm);
		}
		glEnd();
	}
	else
	{
		glBegin(GL_TRIANGLES);
		int NF = dom.Faces();
		for (int i = 0; i < NF; ++i)
		{
			FSFace& face = dom.Face(i);
			if (face.m_ntag == 1)
			{
				// okay, we got one, so let's render it
				m_render.RenderFace(face, pm);
			}
		}
		glEnd();
	}

	// render inactive faces
	if (activeOnly == false)
	{
		if (btex) glDisable(GL_TEXTURE_1D);

		if (m_pcol->IsActive() && benable) glColor4ub(m_col_inactive.r, m_col_inactive.g, m_col_inactive.b, m_col_inactive.a);

		if (zsort)
		{
			int NF = dom.Faces();
			vector< pair<int, double> > zlist; zlist.reserve(NF);
			for (int i = 0; i < NF; ++i)
			{
				FSFace& face = dom.Face(i);
				if (face.m_ntag == 2)
				{
					// get the face center
					vec3d r = pm->FaceCenter(face);

					// convert to eye coordinates
					vec3d q = rc.m_cam->WorldToCam(r);

					// add it to the z-list
					zlist.push_back(pair<int, double>(i, q.z));
				}
			}

			// sort the zlist
			std::sort(zlist.begin(), zlist.end(), [](pair<int, double>& a, pair<int, double>& b) {
				return a.second < b.second;
				});

			// render the list
			glBegin(GL_TRIANGLES);
			for (int i = 0; i < zlist.size(); ++i)
			{
				FSFace& face = dom.Face(zlist[i].first);
				m_render.RenderFace(face, pm);
			}
			glEnd();
		}
		else
		{
			glBegin(GL_TRIANGLES);
			int NF = dom.Faces();
			for (int i = 0; i < NF; ++i)
			{
				FSFace& face = dom.Face(i);
				if (face.m_ntag == 2)
				{
					// okay, we got one, so let's render it
					m_render.RenderFace(face, pm);
				}
			}
			glEnd();
		}
		if (btex) glEnable(GL_TEXTURE_1D);
	}
}

//-----------------------------------------------------------------------------
void CGLModel::RenderSolidPart(FEPostModel* ps, CGLContext& rc, int mat)
{
	// get the material
	Material* pmat = ps->GetMaterial(mat);

	// set the rendering mode
	int nmode = m_nrender;
	if (pmat->m_nrender != RENDER_MODE_DEFAULT) nmode = pmat->m_nrender - 1;

	if (nmode == RENDER_MODE_SOLID)
	{
		if (pmat->m_ntransmode == RENDER_TRANS_VALUE_WEIGHTED) RenderTransparentMaterial(rc, ps, mat);
		else if ((pmat->transparency >= 0.99f) || (pmat->m_ntransmode == RENDER_TRANS_CONSTANT)) RenderSolidMaterial(rc, ps, mat, false);
		else RenderTransparentMaterial(rc, ps, mat);
	}
	else
	{
		if (pmat->benable && m_pcol->IsActive())
			RenderSolidMaterial(rc, ps, mat, true);
		RenderOutline(rc, mat);
	}

	// Render the mesh lines
	if (m_bshowMesh && (GetSelectionMode() != SELECT_EDGES))
	{
		// store attributes
		glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT);

		glDisable(GL_LIGHTING);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		if (pmat->bclip == false) CGLPlaneCutPlot::DisableClipPlanes();

		// make sure the material is visible
		if (pmat->bvisible && pmat->bmesh)
		{
			rc.m_cam->LineDrawMode(true);

			// set the material properties
			GLColor c = pmat->meshcol;
			glColor3ub(c.r, c.g, c.b);
			RenderMeshLines(ps, mat);

			rc.m_cam->LineDrawMode(false);
		}
		CGLPlaneCutPlot::EnableClipPlanes();

		// restore attributes
		glPopAttrib();
	}
}

//-----------------------------------------------------------------------------
void CGLModel::RenderSolidMaterial(CGLContext& rc, FEPostModel* ps, int m, bool activeOnly)
{
	// make sure a part with this material exists
	FEPostMesh* pm = GetActiveMesh();
	if (m >= pm->Domains()) return;

	// get the material
	Material* pmat = ps->GetMaterial(m);

	// get the transparency value
	GLubyte alpha = (GLubyte)(255.f*pmat->transparency);

	// set the color for inactive materials
	m_col_inactive = m_pcol->GetInactiveColor();
	m_col_inactive.a = alpha;

	// set the material properties
	SetMaterialParams(pmat);
	bool btex = false;
	if (pmat->benable && m_pcol->IsActive())
	{
		btex = true;
		glColor4ub(255,255,255, alpha);
		m_pcol->GetColorMap()->GetTexture().MakeCurrent();
	}
	else if (m_pcol->IsActive())
	{
		glColor4ub(m_col_inactive.r, m_col_inactive.g, m_col_inactive.b, alpha);
	}

	// see if we allow the model to be clipped
	glPushAttrib(GL_ENABLE_BIT);
	if (pmat->bclip == false) CGLPlaneCutPlot::DisableClipPlanes();

	// get selection mode
	int mode = GetSelectionMode();

	// determine which faces to draw 
	// tag == 0 : no-draw
	// tag == 1 : draw active
	// tag == 2 : draw inactive
	// TODO: It seems that this can be precomputed and stored somewhere in the domains
	int numActiveFaces = 0;
	MeshDomain& dom = pm->Domain(m);
	int NF = dom.Faces();
	for (int i = 0; i<NF; ++i)
	{
		FSFace& face = dom.Face(i);

		// assume no-draw
		face.m_ntag = 0;

		if (face.IsExternal())
		{
			FEElement_& el = pm->ElementRef(face.m_elem[0].eid);

			// check render state
			if (el.IsVisible())
			{
				if (((mode != SELECT_ELEMS) || !el.IsSelected()) && ((mode != SELECT_FACES) || !face.IsSelected()) && face.IsVisible())
				{
					if (face.IsActive())
					{
						face.m_ntag = 1;
						numActiveFaces++;
					}
					else face.m_ntag = 2;
				}
			}
		}
		else
		{
			// find out which element belongs to this domain
			FEElement_& el0 = pm->ElementRef(face.m_elem[0].eid);
			FEElement_& el1 = pm->ElementRef(face.m_elem[1].eid);
			if ((el0.m_MatID == m) && (el0.IsVisible() && !el1.IsVisible()))
			{
				if (((mode != SELECT_ELEMS) || !el0.IsSelected()) && ((mode != SELECT_FACES) || !face.IsSelected()))
				{
					if (face.IsActive())
					{
						face.m_ntag = 1;
						numActiveFaces++;
					}
					else face.m_ntag = 2;
				}
			}
			else if ((el1.m_MatID == m) && (el1.IsVisible() && !el0.IsVisible()))
			{
				if (((mode != SELECT_ELEMS) || !el1.IsSelected()) && ((mode != SELECT_FACES) || !face.IsSelected()))
				{
					if (face.IsActive())
					{
						face.m_ntag = 1;
						numActiveFaces++;
					}
					else face.m_ntag = 2;
				}
			}
			else if (el0.IsVisible() && el1.IsVisible())
			{
				if (el0.m_MatID == m)
				{
					Material* pm2 = m_ps->GetMaterial(el1.m_MatID);
					float f2 = pm2->transparency;
					if (alpha > f2)
					{
						if (face.IsActive())
						{
							face.m_ntag = 1;
							numActiveFaces++;
						}
						else face.m_ntag = 2;
					}
				}
				else if (el1.m_MatID == m)
				{
					Material* pm2 = m_ps->GetMaterial(el0.m_MatID);
					float f2 = pm2->transparency;
					if (alpha > f2)
					{
						if (face.IsActive())
						{
							face.m_ntag = 1;
							numActiveFaces++;
						}
						else face.m_ntag = 2;
					}
				}
			}
		}
	}

	if ((activeOnly == false) || (numActiveFaces > 0))
	{

		// do the rendering
		if (pmat->transparency > .999f)
		{
			RenderSolidDomain(rc, dom, btex, pmat->benable, false, activeOnly);
		}
		else
		{
			if (m_doZSorting)
			{
				RenderSolidDomain(rc, dom, btex, pmat->benable, true, activeOnly);
			}
			else
			{
				// for better transparency we first draw all the backfacing polygons.
				glPushAttrib(GL_ENABLE_BIT);
				glEnable(GL_CULL_FACE);

				glCullFace(GL_FRONT);
				RenderSolidDomain(rc, dom, btex, pmat->benable, false, activeOnly);

				// and then we draw the front-facing ones.
				glCullFace(GL_BACK);
				if (btex) glColor4ub(255, 255, 255, alpha);
				RenderSolidDomain(rc, dom, btex, pmat->benable, false, activeOnly);

				glPopAttrib();
			}
		}

		// render the internal surfaces
		if (mode != SELECT_FACES)
		{
			if (btex) glColor3ub(255, 255, 255);
			RenderInnerSurface(m, btex);
		}

		if (pmat->benable && m_pcol->IsActive())
		{
			glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		}
	}

	glPopAttrib();
}

//-----------------------------------------------------------------------------
// This algorithm is identical to the RenderOutline, except that it uses the
// original coordinates instead of the current ones
void CGLModel::RenderGhost(CGLContext &rc)
{
	int i, j, n;
	int a, b;
	vec3d r1, r2;

	FEPostModel* ps = m_ps;
	FSMeshBase* pm = GetActiveMesh();
	Post::FERefState* ref = GetActiveState()->m_ref;

	glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	GLColor c = m_ghost_color;
	glColor3ub(c.r,c.g,c.b);

	quatd q = rc.m_cam->GetOrientation();

	double eps = cos(GetSmoothingAngleRadians());

	for (i=0; i<pm->Faces(); ++i)
	{
		FSFace& f = pm->Face(i);
		if (f.IsVisible())
		{
			n = f.Edges();
			for (j=0; j<n; ++j)
			{
				bool bdraw = false;

				if (f.m_nbr[j] < 0)
				{
					bdraw = true;
				}
				else
				{
					FSFace& f2 = pm->Face(f.m_nbr[j]);
					if (f.m_gid != f2.m_gid)
					{
						bdraw = true;
					}
					else if (f.m_fn*f2.m_fn <= eps)
					{
						bdraw = true;
					}
					else
					{
						vec3d n1 = to_vec3d(f.m_fn);
						vec3d n2 = to_vec3d(f2.m_fn);
						q.RotateVector(n1);
						q.RotateVector(n2);
						if (n1.z*n2.z <= 0) 
						{
							bdraw = true;
						}
					}
				}

				if (bdraw)
				{
					a = f.n[j];
					b = f.n[(j+1)%n];

					if (a > b) { a ^= b; b ^= a; a ^= b; }

					r1 = to_vec3d(ref->m_Node[a].m_rt);
					r2 = to_vec3d(ref->m_Node[b].m_rt);

					glBegin(GL_LINES);
					{
						glVertex3f(r1.x, r1.y, r1.z);
						glVertex3f(r2.x, r2.y, r2.z);
					}
					glEnd();
				}
			}
		}
	}

	glPopAttrib();
}

//-----------------------------------------------------------------------------
// NOTE: This algorithm does not always give satisfactory results. 
// In the case of perspective projection, the normal product should 
// be less than some value depending on the location of the edge, 
// in stead of zero (which is the correct value for ortho projection).

void CGLModel::RenderOutline(CGLContext& rc, int nmat)
{
	FEPostModel* ps = m_ps;
	Post::FEPostMesh* pm = GetActiveMesh();

	glPushAttrib(GL_ENABLE_BIT);

	glDisable(GL_LIGHTING);

	GLColor c = m_line_col;
	glColor3ub(c.r,c.g,c.b);

	quatd q = rc.m_cam->GetOrientation();

	int ndivs = GetSubDivisions();

	for (int i=0; i<pm->Faces(); ++i)
	{
		FSFace& f = pm->Face(i);
		FEElement_& el = pm->ElementRef(f.m_elem[0].eid);
		if (f.IsVisible() && el.IsVisible() && ((nmat == -1) || (el.m_MatID == nmat)))
		{
			int n = f.Edges();
			for (int j=0; j<n; ++j)
			{
				bool bdraw = false;

				if (f.m_nbr[j] < 0)
				{
					bdraw = true;
				}
				else
				{
					FSFace& f2 = pm->Face(f.m_nbr[j]);
					if (f.m_gid != f2.m_gid)
					{
						bdraw = true;
					}
					else if (f.m_sid != f2.m_sid)
					{
						bdraw = true;
					}
/*					else
					{
						vec3f n1 = f.m_fn;
						vec3f n2 = f2.m_fn;
						q.RotateVector(n1);
						q.RotateVector(n2);
						if (n1.z*n2.z <= 0) 
						{
							bdraw = true;
						}
					}
*/				}

				if (bdraw) m_render.RenderFaceEdge(f, j, pm, ndivs);
			}
		}
	}

	glPopAttrib();
}


///////////////////////////////////////////////////////////////////////////////

void CGLModel::RenderNormals(CGLContext& rc)
{
	// get the mesh
	FEPostModel* ps = m_ps;
	Post::FEPostMesh* pm = GetActiveMesh();

	BOX box = ps->GetBoundingBox();

	float scale = 0.05f*box.Radius()*m_scaleNormals;

	// tag faces
	for (int i = 0; i < pm->Faces(); ++i)
	{
		FSFace& face = pm->Face(i);
		if (face.IsVisible()) face.m_ntag = 1; else face.m_ntag = 0;
	}

	// render the normals on the tagged faces
	GLMeshRender render;
	render.RenderNormals(pm, scale, 1);
}

//-----------------------------------------------------------------------------
// Render the mesh lines for a specific material
//
void CGLModel::RenderMeshLines(FEPostModel* ps, int nmat)
{
	// get the mesh
	Post::FEPostMesh* pm = GetActiveMesh();

	int ndivs = GetSubDivisions();

	// now loop over all faces and see which face belongs to this material
	if (nmat < pm->Domains())
	{
		MeshDomain& dom = pm->Domain(nmat);
		for (int i=0; i<dom.Faces(); ++i)
		{
			FSFace& face = dom.Face(i);
			FEElement_& el = pm->ElementRef(face.m_elem[0].eid);
			if (face.IsVisible() && el.IsVisible())
			{
				// okay, we got one, so let's render it
				m_render.RenderFaceOutline(face, pm, ndivs);
			}
		}
	}

	int mode = GetSelectionMode();
	if (mode != SELECT_FACES)
	{
		// draw elements
		RenderInnerSurfaceOutline(nmat, ndivs);
	}
}

//-----------------------------------------------------------------------------
// Render the mesh lines of the model.

///////////////////////////////////////////////////////////////////////////////

void CGLModel::RenderShadows(FEPostModel* ps, const vec3d& lp, float inf)
{
	Post::FEPostMesh* pm = GetActiveMesh();

	// find all silhouette edges
	vec3d fn, fn2;
	vec3d n(lp); n.Normalize();
	int m;
	bool bvalid;
	for (int i=0; i<pm->Faces(); i++)
	{
		FSFace& f = pm->Face(i);

		m = f.Edges();
		bvalid = true;
		if      (f.n[0] == f.n[1]) bvalid = false;
		else if (f.n[0] == f.n[2]) bvalid = false;
		else if (f.n[1] == f.n[2]) bvalid = false;

		FEElement_& el = pm->ElementRef(f.m_elem[0].eid);
		Material* pmat = ps->GetMaterial(el.m_MatID);

		// see it this face is visible
		if (!f.IsVisible() || !pmat->bvisible) bvalid = false;

		// make sure this material casts shadows
		if (pmat->bcast_shadows == false) bvalid = false;

		if (bvalid)
		{
			// only look at front facing faces
			fn = to_vec3d(f.m_fn);
			if (fn*n > 0)
			{
				for (int j=0; j<m; j++)
				{
					FSFace* pf2 = 0;
					if (f.m_nbr[j] >= 0) pf2 = &pm->Face(f.m_nbr[j]);

					if (pf2)
					{
						fn2 = to_vec3d(pf2->m_fn);
					}

					// we got one!
					if ((pf2 == 0) || (fn2*n < 0))
					{
						vec3d a, b, c, d;
						a = pm->Node(f.n[j]).r;
						b = pm->Node(f.n[(j+1)%m]).r;
	
						c = a - n*inf;
						d = b - n*inf;

						glBegin(GL_QUADS);
						{
							vec3d n = (c-a)^(d-a);
							n.Normalize();

							glNormal3d(n.x, n.y, n.z);
							glVertex3d(a.x, a.y, a.z);
							glVertex3d(c.x, c.y, c.z);
							glVertex3d(d.x, d.y, d.z);
							glVertex3d(b.x, b.y, b.z);
						}
						glEnd();
					}
				}
			}
			else 
			{
				vec3d r1 = pm->Node(f.n[0]).r;
				vec3d r2 = pm->Node(f.n[1]).r;
				vec3d r3 = pm->Node(f.n[2]).r;
				vec3d r4 = pm->Node(f.n[3]).r;
	
				glNormal3d(-fn.x, -fn.y, -fn.z);

				switch (f.m_type)
				{
				case FE_FACE_QUAD4:
				case FE_FACE_QUAD8:
				case FE_FACE_QUAD9:
					glBegin(GL_QUADS);
					{
						glVertex3f(r4.x, r4.y, r4.z);
						glVertex3f(r3.x, r3.y, r3.z);
						glVertex3f(r2.x, r2.y, r2.z);
						glVertex3f(r1.x, r1.y, r1.z);
					}
					glEnd();
					break;
				case FE_FACE_TRI3:
				case FE_FACE_TRI6:
				case FE_FACE_TRI7:
				case FE_FACE_TRI10:
					glBegin(GL_TRIANGLES);
					{
						glVertex3f(r3.x, r3.y, r3.z);
						glVertex3f(r2.x, r2.y, r2.z);
						glVertex3f(r1.x, r1.y, r1.z);
					}
					glEnd();
					break;
				default:
					assert(false);
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void CGLModel::RenderNodes(FEPostModel* ps, CGLContext& rc)
{
	Post::FEPostMesh* pm = GetActiveMesh();

	// store attributes
	glPushAttrib(GL_ENABLE_BIT);

	glDisable(GL_LIGHTING);

	// reset tags and check visibility
	for (int i=0; i<pm->Nodes(); ++i)
	{
		FSNode& n = pm->Node(i);
		n.m_ntag = (n.IsVisible() ? 1 : 0);
	}

	// see if backface-culling is enabled or not
	GLboolean bcull;
	glGetBooleanv(GL_CULL_FACE, &bcull);
	if (bcull)
	{
		quatd q = rc.m_cam->GetOrientation();
		vec3f f;
		int NF = pm->Faces();
		for (int i = 0; i<NF; ++i)
		{
			FSFace& face = pm->Face(i);
			int n = face.Nodes();
			for (int j = 0; j<n; ++j)
			{
				vec3d f = to_vec3d(face.m_nn[j]);
				q.RotateVector(f);
				if (f.z < 0) pm->Node(face.n[j]).m_ntag = 0;
			}
		}
	}

	// render all unselected tagged nodes
	glColor3ub(m_node_col.r, m_node_col.g, m_node_col.b);
	glBegin(GL_POINTS);
	for (int i = 0; i<pm->Nodes(); ++i)
	{
		FSNode& node = pm->Node(i);
		if (node.IsExterior() && node.m_ntag && (node.IsSelected() == false))
		{
			// get the nodal coordinate
			vec3d& r = node.r;

			// render the point
			glVertex3d(r.x, r.y, r.z);
		}
	}
	if (m_brenderInteriorNodes)
	{
		for (int i = 0; i<pm->Nodes(); ++i)
		{
			FSNode& node = pm->Node(i);
			if ((node.IsExterior() == false) && node.m_ntag && (node.IsSelected() == false))
			{
				// get the nodal coordinate
				vec3d& r = node.r;

				// render the point
				glVertex3d(r.x, r.y, r.z);
			}
		}
	}
	glEnd();

	// render selected tagged nodes
	if (GetSelectionMode() == SELECT_NODES)
	{
		glDisable(GL_DEPTH_TEST);

		// render exterior selected nodes first
		glColor3ub(255, 255, 0);
		glBegin(GL_POINTS);
		for (int i = 0; i<pm->Nodes(); ++i)
		{
			FSNode& node = pm->Node(i);
			if (node.IsExterior() && node.m_ntag && node.IsSelected())
			{
				// get the nodal coordinate
				vec3d r = node.r;

				// render the point
				glVertex3d(r.x, r.y, r.z);
			}
		}
		glEnd();

		// render interior nodes
		if (m_brenderInteriorNodes)
		{
			glColor3ub(255, 0, 0);
			glBegin(GL_POINTS);
			for (int i = 0; i<pm->Nodes(); ++i)
			{
				FSNode& node = pm->Node(i);
				if ((node.IsExterior() == false) && node.m_ntag && node.IsSelected())
				{
					// get the nodal coordinate
					vec3d r = node.r;

					// render the point
					glVertex3d(r.x, r.y, r.z);
				}
			}
			glEnd();
		}
	}

	// restore attributes
	glPopAttrib();
}

//-----------------------------------------------------------------------------
// Render the edges of the model.
void CGLModel::RenderEdges(FEPostModel* ps, CGLContext& rc)
{
	// store attributes
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);

	vec3d r[3];

	Post::FEPostMesh& mesh = *GetActiveMesh();
	int NE = mesh.Edges();

	// render unselected edges
	glColor3ub(0, 0, 255);
	glBegin(GL_LINES);
	{
		for (int i = 0; i<NE; ++i)
		{
			FSEdge& edge = mesh.Edge(i);
			if (edge.IsVisible() && (edge.IsSelected() == false))
			{
				switch (edge.Type())
				{
				case FE_EDGE2:
					r[0] = mesh.Node(edge.n[0]).r;
					r[1] = mesh.Node(edge.n[1]).r;
					glVertex3d(r[0].x, r[0].y, r[0].z);
					glVertex3d(r[1].x, r[1].y, r[1].z);
					break;
				case FE_EDGE3:
					r[0] = mesh.Node(edge.n[0]).r;
					r[1] = mesh.Node(edge.n[1]).r;
					r[2] = mesh.Node(edge.n[2]).r;
					glVertex3d(r[0].x, r[0].y, r[0].z);
					glVertex3d(r[1].x, r[1].y, r[1].z);
					glVertex3d(r[1].x, r[1].y, r[1].z);
					glVertex3d(r[2].x, r[2].y, r[2].z);
					break;
				}
			}
		}
	}
	glEnd();

	// render selected edges
	if (GetSelectionMode() == SELECT_EDGES)
	{
		glDisable(GL_DEPTH_TEST);
		glColor3ub(255, 255, 0);
		glBegin(GL_LINES);
		{
			for (int i = 0; i<NE; ++i)
			{
				FSEdge& edge = mesh.Edge(i);
				if (edge.IsVisible() && edge.IsSelected())
				{
					switch (edge.Type())
					{
					case FE_EDGE2:
						r[0] = mesh.Node(edge.n[0]).r;
						r[1] = mesh.Node(edge.n[1]).r;
						glVertex3d(r[0].x, r[0].y, r[0].z);
						glVertex3d(r[1].x, r[1].y, r[1].z);
						break;
					case FE_EDGE3:
						r[0] = mesh.Node(edge.n[0]).r;
						r[1] = mesh.Node(edge.n[1]).r;
						r[2] = mesh.Node(edge.n[2]).r;
						glVertex3d(r[0].x, r[0].y, r[0].z);
						glVertex3d(r[1].x, r[1].y, r[1].z);
						glVertex3d(r[1].x, r[1].y, r[1].z);
						glVertex3d(r[2].x, r[2].y, r[2].z);
						break;
					}
				}
			}
		}
		glEnd();
	}

	// restore attributes
	glPopAttrib();
}

// render all the objects
void CGLModel::RenderObjects(CGLContext& rc)
{
	Post::FEPostModel* fem = GetFSModel();
	if ((fem->PointObjects() == 0) && (fem->LineObjects() == 0)) return;

	double scale = 0.05*(double)rc.m_cam->GetTargetDistance();
	double R = 0.5*scale;

	glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	for (int i = 0; i < fem->PointObjects(); ++i)
	{
		Post::FEPostModel::PointObject & ob = *fem->GetPointObject(i);
		if (ob.IsActive())
		{
			glPushMatrix();
			glx::translate(ob.m_pos);
			glx::rotate(ob.m_rot);

			glx::translate(ob.m_rt);

			GLColor c = ob.Color();
			glColor3ub(c.r, c.g, c.b);
			switch (ob.m_tag)
			{
			case 1: glx::renderRigidBody(R); break;
			case 2: glx::renderJoint(R); break;
			case 3: glx::renderJoint(R); break;
			case 4: glx::renderPrismaticJoint(R); break;
			case 5: glx::renderRevoluteJoint(R); break;
			case 6: glx::renderCylindricalJoint(R); break;
			case 7: glx::renderPlanarJoint(R); break;
			default:
				glx::renderAxis(R);
			}
			glPopMatrix();
		}
	}

	for (int i = 0; i < fem->LineObjects(); ++i)
	{
		Post::FEPostModel::LineObject & ob = *fem->GetLineObject(i);
		if (ob.IsActive())
		{
			glPushMatrix();
			glx::translate(ob.m_pos);
			glx::rotate(ob.m_rot);

			vec3d a = ob.m_r1;
			vec3d b = ob.m_r2;

			GLColor c = ob.Color();
			glColor3ub(c.r, c.g, c.b);
			switch (ob.m_tag)
			{
			case 1: glx::renderSpring(a, b, R); break;
			case 2: glx::renderDamper(a, b, R); break;
			case 4: glx::renderContractileForce(a, b, R); break;
			default:
				glx::drawLine(a, b);
			}
			glPopMatrix();
		}
	}

	glPopAttrib();
}

void CGLModel::RenderMinMaxMarkers(CGLContext& rc)
{
	Post::CGLColorMap* pcm = GetColorMap();
	if ((pcm == nullptr) || (pcm->IsActive() == false)) return;

	vec3d rmin = pcm->GetMinPosition();
	vec3d rmax = pcm->GetMaxPosition();

	Post::CColorTexture* tex = pcm->GetColorMap();
	CColorMap& map = tex->ColorMap();

	GLColor c0 = map.GetColor(0);
	GLColor c1 = map.GetColor(map.Colors() - 1);

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	float pointSize;
	glGetFloatv(GL_POINT_SIZE, &pointSize);

	glPointSize(15.f);
	glBegin(GL_POINTS);
	glColor3ub(255, 255, 255);
	glVertex3d(rmin.x, rmin.y, rmin.z);
	glVertex3d(rmax.x, rmax.y, rmax.z);
	glEnd();

	glPointSize(10.f);
	glBegin(GL_POINTS);
	glColor3ub(c0.r, c0.g, c0.b); glVertex3d(rmin.x, rmin.y, rmin.z);
	glColor3ub(c1.r, c1.g, c1.b); glVertex3d(rmax.x, rmax.y, rmax.z);
	glEnd();

	glPointSize(pointSize);

	glPopAttrib();
}

void CGLModel::RenderDecorations()
{
	if (m_decor.empty() == false)
	{
		glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glColor3ub(255, 255, 0);

		std::list<GDecoration*>::iterator it;
		for (it=m_decor.begin(); it != m_decor.end(); ++it)
		{
			GDecoration* pd = *it;
			if (pd->isVisible()) pd->render();
		}
		glPopAttrib();
	}
}

void CGLModel::AddDecoration(GDecoration* pd)
{
	m_decor.push_back(pd);
}

void CGLModel::RemoveDecoration(GDecoration* pd)
{
	std::list<GDecoration*>::iterator it;
	for (it=m_decor.begin(); it != m_decor.end(); ++it)
	{
		if (*it==pd)
		{
			m_decor.erase(it);
			return;
		}
	}
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
	if (m_nDivs < 1)
	{
		Post::FEPostMesh& mesh = *GetActiveMesh();
		int NE = mesh.Elements();
		if (NE == 0) return 1;

		const int max_elem = 10000;
		int ndivs = max_elem / NE;
		if (ndivs > 10) ndivs = 10;
		if (ndivs <  1) ndivs = 1;

#ifdef _DEBUG
		if (ndivs > 2) ndivs = 2;
#endif

		return ndivs;
	}
	else return m_nDivs;
}

//-----------------------------------------------------------------------------
void CGLModel::ClearSelectionLists()
{
	m_nodeSelection.clear();
	m_edgeSelection.clear();
	m_faceSelection.clear();
	m_elemSelection.clear();
}

//-----------------------------------------------------------------------------
void CGLModel::UpdateSelectionLists(int mode)
{
	Post::FEPostMesh& m = *GetActiveMesh();
	if ((mode == -1) || (mode == SELECT_NODES))
	{
		m_nodeSelection.clear();
		for (int i=0; i<m.Nodes(); ++i) if (m.Node(i).IsSelected()) m_nodeSelection.push_back(&m.Node(i));
	}

	if ((mode == -1) || (mode == SELECT_EDGES))
	{
		m_edgeSelection.clear();
		for (int i=0; i<m.Edges(); ++i) if (m.Edge(i).IsSelected()) m_edgeSelection.push_back(&m.Edge(i));
	}
	
	if ((mode == -1) || (mode == SELECT_FACES))
	{
		m_faceSelection.clear();
		for (int i=0; i<m.Faces(); ++i) if (m.Face(i).IsSelected()) m_faceSelection.push_back(&m.Face(i));
	}

	if ((mode == -1) || (mode == SELECT_ELEMS))
	{
		m_elemSelection.clear();
		for (int i=0; i<m.Elements(); ++i) if (m.ElementRef(i).IsSelected()) m_elemSelection.push_back(&m.ElementRef(i));
		UpdateInternalSurfaces();
	}
}

//-----------------------------------------------------------------------------
void CGLModel::SelectNodes(vector<int>& items, bool bclear)
{
	Post::FEPostMesh& m = *GetActiveMesh();
	int N = m.Nodes();
	if (bclear) for (int i=0; i<N; ++i) m.Node(i).Unselect();
	for (int i=0; i<(int) items.size(); ++i)
	{
		int nid = items[i];
		if ((nid >= 0) && (nid < N)) m.Node(nid).Select();
	}
	UpdateSelectionLists(SELECT_NODES);
}

//-----------------------------------------------------------------------------
void CGLModel::SelectEdges(vector<int>& items, bool bclear)
{
	Post::FEPostMesh& m = *GetActiveMesh();
	int N = m.Edges();
	if (bclear) for (int i = 0; i<N; ++i) m.Edge(i).Unselect();
	for (int i=0; i<(int) items.size(); ++i)
	{
		int eid = items[i];
		if ((eid >= 0) && (eid < N)) m.Edge(eid).Select();
	}
	UpdateSelectionLists(SELECT_EDGES);
}

//-----------------------------------------------------------------------------
void CGLModel::SelectFaces(vector<int>& items, bool bclear)
{
	Post::FEPostMesh& m = *GetActiveMesh();
	int N = m.Faces();
	if (bclear) for (int i=0; i<N; ++i) m.Face(i).Unselect();
	for (int i=0; i<(int) items.size(); ++i) 
	{
		int fid = items[i];
		if ((fid >= 0) && (fid < N)) m.Face(fid).Select();
	}
	UpdateSelectionLists(SELECT_FACES);
}

//-----------------------------------------------------------------------------
void CGLModel::SelectElements(vector<int>& items, bool bclear)
{
	Post::FEPostMesh& m = *GetActiveMesh();
	int N = m.Elements();
	if (bclear) for (int i=0; i<N; ++i) m.ElementRef(i).Unselect();
	for (int i=0; i<(int) items.size(); ++i)
	{
		int eid = items[i];
		if ((eid >= 0) && (eid < N)) m.ElementRef(eid).Select();
	}
	UpdateSelectionLists(SELECT_ELEMS);
}

//-----------------------------------------------------------------------------
//! unhide all items
void CGLModel::UnhideAll()
{
	Post::FEPostMesh& mesh = *GetActiveMesh();
	for (int i = 0; i<mesh.Elements(); ++i) mesh.ElementRef(i).Unhide();
	for (int i = 0; i<mesh.Faces(); ++i) mesh.Face(i).Unhide();
	for (int i = 0; i<mesh.Edges(); ++i) mesh.Edge(i).Unhide();
	for (int i = 0; i<mesh.Nodes(); ++i) mesh.Node(i).Unhide();
	UpdateInternalSurfaces();
}

//-----------------------------------------------------------------------------
// Clear all selection
void CGLModel::ClearSelection()
{
	Post::FEPostMesh& mesh = *GetActiveMesh();
	for (int i=0; i<mesh.Elements(); i++) mesh.ElementRef(i).Unselect();
	for (int i=0; i<mesh.Faces   (); i++) mesh.Face(i).Unselect();
	for (int i=0; i<mesh.Edges   (); i++) mesh.Edge(i).Unselect();
	for (int i=0; i<mesh.Nodes   (); i++) mesh.Node(i).Unselect();
	UpdateSelectionLists();
}

//-----------------------------------------------------------------------------
// Hide elements with a particular material ID
void CGLModel::HideMaterial(int nmat)
{
	Post::FEPostMesh& mesh = *GetActiveMesh();

	// Hide the elements with the material ID
	for (int i = 0; i < mesh.Elements(); ++i)
	{
		FEElement_& el = mesh.ElementRef(i);
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
			FEElement_& e0 = mesh.ElementRef(f.m_elem[0].eid);
			FEElement_& e1 = mesh.ElementRef(f.m_elem[1].eid);

			if (e0.IsInvisible() && e1.IsInvisible()) f.Show(false);
		}
	}

	// hide nodes: nodes will be hidden if all elements they attach to are hidden
	int NN = mesh.Nodes();
	for (int i=0; i<NN; ++i) mesh.Node(i).m_ntag = 0;
	for (int i=0; i<mesh.Elements(); ++i)
	{
		FEElement_& el = mesh.ElementRef(i);
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

	// selected items are unselected when hidden so
	// we need to update the selection lists.
	// This also updates the internal surfaces
	UpdateSelectionLists();
}

//-----------------------------------------------------------------------------
// Show elements with a certain material ID
void CGLModel::ShowMaterial(int nmat)
{
	Post::FEPostMesh& mesh = *GetActiveMesh();

	// unhide the elements with mat ID nmat
	int NE = mesh.Elements();
	for (int i=0; i<NE; ++i) 
	{
		FEElement_& e = mesh.ElementRef(i);
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
			FEElement_& e0 = mesh.ElementRef(f.m_elem[0].eid);
			FEElement_& e1 = mesh.ElementRef(f.m_elem[1].eid);
			if (!e0.IsInvisible() || !e1.IsInvisible()) f.Show(true);
		}
	}

	// show nodes
	int NN = mesh.Nodes();
	for (int i=0; i<NN; ++i) mesh.Node(i).m_ntag = 0;
	for (int i=0; i<mesh.Elements(); ++i)
	{
		FEElement_& el = mesh.ElementRef(i);
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

	UpdateSelectionLists();
}

//-----------------------------------------------------------------------------
// Enable elements with a certain mat ID
void CGLModel::UpdateMeshState()
{
	Post::FEPostMesh& mesh = *GetActiveMesh();
	FEPostModel& fem = *GetFSModel();

	// update the elements
	for (int i=0; i<mesh.Elements(); ++i)
	{
		FEElement_& el = mesh.ElementRef(i);
		int nmat = el.m_MatID;
		if (fem.GetMaterial(nmat)->enabled()) el.Enable();
		else el.Disable();
	}

	// now we update the nodes
	for (int i=0; i<mesh.Nodes(); ++i) mesh.Node(i).Disable();
	for (int i=0; i<mesh.Elements(); ++i)
	{
		FEElement_& el = mesh.ElementRef(i);
		if (el.IsEnabled())
		{
			int n = el.Nodes();
			for (int j=0; j<n; ++j) mesh.Node(el.m_node[j]).Enable();
		}
	}

	// enable the faces
	for (int i=0; i<mesh.Faces(); ++i) 
	{
		FSFace& f = mesh.Face(i);
		f.Disable();
		if (mesh.ElementRef(f.m_elem[0].eid).IsEnabled()) f.Enable();
	}
}

//-----------------------------------------------------------------------------
// Select elements that are connected through the surface
void CGLModel::SelectConnectedSurfaceElements(FEElement_ &el)
{
	if (!el.IsVisible()) return;

	Post::FEPostMesh& mesh = *GetActiveMesh();
	// tag all faces
	for (int i=0; i<mesh.Faces(); ++i) mesh.Face(i).m_ntag = 0;

	// find the face that this element belongs to
	for (int i=0; i<mesh.Faces(); ++i)
	{
		FSFace& f = mesh.Face(i);
		if (f.m_elem[0].eid == el.m_lid)
		{
			// propagate through all neighbors
			stack<FSFace*> S;
			S.push(&f);
			while (!S.empty())
			{
				FSFace* pf = S.top(); S.pop();
				pf->m_ntag = 1;
				FEElement_& e2 = mesh.ElementRef(pf->m_elem[0].eid);
				if (e2.IsVisible())
				{
					e2.Select();
					for (int j=0; j<pf->Edges(); ++j)
					{
						FSFace* pf2 = (pf->m_nbr[j] >= 0? &mesh.Face(pf->m_nbr[j]) : 0);
						if (pf2 && (pf2->m_ntag == 0)) S.push(pf2);
					}
				}
			}
		}
	}
	UpdateSelectionLists();
}

//-----------------------------------------------------------------------------
// Select elements that are connected through the volume
void CGLModel::SelectConnectedVolumeElements(FEElement_ &el)
{
	if (!el.IsVisible()) return;

	Post::FEPostMesh& mesh = *GetActiveMesh();
	// tag all elements
	for (int i=0; i<mesh.Elements(); ++i) mesh.ElementRef(i).m_ntag = 0;

	// propagate through all neighbors
	stack<FEElement_*> S;
	S.push(&el);
	while (!S.empty())
	{
		FEElement_* pe = S.top(); S.pop();
		pe->m_ntag = 1;
		pe->Select();
		for (int j=0; j<pe->Faces(); ++j)
		{
			FEElement_* pe2 = mesh.ElementPtr(pe->m_nbr[j]);
			if (pe2 && pe2->IsVisible() && (pe2->m_ntag == 0)) S.push(pe2);
		}
	}

	UpdateSelectionLists(SELECT_ELEMS);
}

//-----------------------------------------------------------------------------
// Select faces that are connected
void CGLModel::SelectConnectedEdges(FSEdge& e)
{
	Post::FEPostMesh& mesh = *GetActiveMesh();

	// clear tags on all edge
	int NE = mesh.Edges();
	for (int i=0; i<NE; ++i) mesh.Edge(i).m_ntag = 0;

	// build the node-edge table
	FSNodeEdgeList NEL;
	NEL.Build(&mesh);

	if (NEL.IsEmpty()) return;

	// find a face that has both nodes connects to
	stack<FSEdge*> Stack;
	Stack.push(&e);
	while (Stack.empty() == false)
	{
		FSEdge* pe = Stack.top(); Stack.pop();
		pe->m_ntag = 1;

		// get the edge vector
		vec3d n = mesh.Node(pe->n[1]).r - mesh.Node(pe->n[0]).r; n.Normalize();

		// find the neighbor edges whose vector is closely aligned to the edge vector n
		for (int i=0; i<2; ++i)
		{
			int m = pe->n[i];
			int ne = NEL.Edges(m);
			const std::vector<int>& EL = NEL.EdgeIndexList(m);
			for (int j=0; j<ne; ++j)
			{
				FSEdge& ej = mesh.Edge(EL[j]);

				if (ej.IsVisible() && (&ej != pe) && (ej.m_ntag == 0))
				{
					vec3d nj = mesh.Node(ej.n[1]).r - mesh.Node(ej.n[0]).r; nj.Normalize();
					if (n*nj > 0.866) Stack.push(&ej);
				}
			}
		}
	}

	// select all the tagged edges
	for (int i=0; i<NE; ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		if (edge.IsVisible() && (edge.m_ntag == 1)) edge.Select();
	}

	UpdateSelectionLists(SELECT_EDGES);
}

//-----------------------------------------------------------------------------
// Select faces that are connected
void CGLModel::SelectConnectedFaces(FSFace &f, double angleTol)
{
	Post::FEPostMesh& mesh = *GetActiveMesh();

	// clear tags on all faces
	for (int i=0; i<mesh.Faces(); ++i) mesh.Face(i).m_ntag = 0;

	double tol = cos(DEG2RAD*angleTol);

	// propagate through all neighbors
	stack<FSFace*> S;
	f.m_ntag = 1;
	S.push(&f);
	while (!S.empty())
	{
		FSFace* pf = S.top(); S.pop();
		if (pf->IsVisible())
		{
			pf->Select();
			for (int j=0; j<pf->Edges(); ++j)
			{
				FSFace* pf2 = (pf->m_nbr[j] >= 0? &mesh.Face(pf->m_nbr[j]) : 0);
				if (pf2 && (pf2->m_ntag == 0) && (pf2->m_sid == pf->m_sid) && (pf2->m_gid == pf->m_gid) && (f.m_fn*pf2->m_fn > tol))
				{
					pf2->m_ntag = 1;
					S.push(pf2);
				}
			}
		}
	}

	UpdateSelectionLists(SELECT_FACES);
}

//-----------------------------------------------------------------------------
// Select nodes that are connected on a surface
void CGLModel::SelectConnectedSurfaceNodes(int n)
{
	Post::FEPostMesh& mesh = *GetActiveMesh();

	// clear tags on all faces
	int NF = mesh.Faces();
	for (int i=0; i<NF; ++i) mesh.Face(i).m_ntag = 0;

	// find a face that has this node connects to
	FSFace* pf = 0;
	for (int i=0; i<mesh.Faces(); ++i)
	{
		FSFace& f = mesh.Face(i);
		if (f.IsVisible() && f.HasNode(n))
		{
			pf = &f;
			break;
		}
	}
	if (pf == 0) return;

	// propagate through all neighbors
	stack<FSFace*> S;
	pf->m_ntag = 1;
	S.push(pf);
	while (!S.empty())
	{
		FSFace* pf = S.top(); S.pop();
		if (pf->IsVisible())
		{
			for (int j=0; j<pf->Edges(); ++j)
			{
				FSFace* pf2 = (pf->m_nbr[j] >= 0? &mesh.Face(pf->m_nbr[j]) : 0);
				if (pf2 && (pf2->m_ntag == 0) && (pf2->m_sid == pf->m_sid)) 
				{
					pf2->m_ntag = 1;
					S.push(pf2);
				}
			}
		}
	}

	// select all the nodes of tagged faces
	for (int i=0; i<NF; ++i)
	{
		FSFace& f = mesh.Face(i);
		if (f.m_ntag == 1)
		{
			int nf = f.Nodes();
			for (int j=0; j<nf; ++j) mesh.Node(f.n[j]).Select();
		}
	}

	UpdateSelectionLists(SELECT_NODES);
}

//-----------------------------------------------------------------------------
// Select nodes that are connected on a volume
void CGLModel::SelectConnectedVolumeNodes(int n)
{
	Post::FEPostMesh& mesh = *GetActiveMesh();

	// clear tags on all elements
	int NE = mesh.Elements();
	for (int i=0; i<NE; ++i) mesh.ElementRef(i).m_ntag = 0;

	// find a visible element that has this node connects to
	FEElement_* pe = 0;
	for (int i=0; i<mesh.Elements(); ++i)
	{
		FEElement_& e = mesh.ElementRef(i);
		if (e.IsVisible() && e.HasNode(n))
		{
			pe = &e;
			break;
		}
	}
	if (pe == 0) return;

	// propagate through all neighbors
	stack<FEElement_*> S;
	pe->m_ntag = 1;
	S.push(pe);
	while (!S.empty())
	{
		FEElement_* pe = S.top(); S.pop();
		if (pe->IsVisible())
		{
			for (int j=0; j<pe->Faces(); ++j)
			{
				FEElement_* pe2 = mesh.ElementPtr(pe->m_nbr[j]);
				if (pe2 && (pe2->m_ntag == 0)) 
				{
					pe2->m_ntag = 1;
					S.push(pe2);
				}
			}
		}
	}

	// select all the nodes of tagged elements
	for (int i=0; i<NE; ++i)
	{
		FEElement_& e = mesh.ElementRef(i);
		if (e.m_ntag == 1)
		{
			int nf = e.Nodes();
			for (int j=0; j<nf; ++j) mesh.Node(e.m_node[j]).Select();
		}
	}

	UpdateSelectionLists(SELECT_NODES);
}


//-----------------------------------------------------------------------------
void CGLModel::SelectElemsInRange(float fmin, float fmax, bool bsel)
{
	Post::FEPostMesh* pm = GetActiveMesh();
	int N = pm->Elements();
	FEState* ps = GetActiveState();
	for (int i = 0; i<N; ++i)
	{
		FEElement_& el = pm->ElementRef(i);
		if (el.IsEnabled() && el.IsVisible() && ((bsel == false) || (el.IsSelected())))
		{
			float v = ps->m_ELEM[i].m_val;
			if ((v >= fmin) && (v <= fmax)) el.Select();
			else el.Unselect();
		}
	}
	UpdateSelectionLists();
}

//-----------------------------------------------------------------------------
void CGLModel::SelectNodesInRange(float fmin, float fmax, bool bsel)
{
	Post::FEPostMesh* pm = GetActiveMesh();
	int N = pm->Nodes();
	FEState* ps = GetActiveState();
	for (int i = 0; i<N; ++i)
	{
		FSNode& node = pm->Node(i);
		if (node.IsEnabled() && node.IsVisible() && ((bsel == false) || (node.IsSelected())))
		{
			float v = ps->m_NODE[i].m_val;
			if ((v >= fmin) && (v <= fmax)) node.Select();
			else node.Unselect();
		}
	}
	UpdateSelectionLists();
}

//-----------------------------------------------------------------------------
void CGLModel::SelectEdgesInRange(float fmin, float fmax, bool bsel)
{
	Post::FEPostMesh* pm = GetActiveMesh();
	int N = pm->Edges();
	FEState* ps = GetActiveState();
	for (int i = 0; i<N; ++i)
	{
		FSEdge& edge = pm->Edge(i);
		if (edge.IsEnabled() && edge.IsVisible() && ((bsel == false) || (edge.IsSelected())))
		{
			float v = ps->m_EDGE[i].m_val;
			if ((v >= fmin) && (v <= fmax)) edge.Select();
			else edge.Unselect();
		}
	}
	UpdateSelectionLists();
}

//-----------------------------------------------------------------------------
void CGLModel::SelectFacesInRange(float fmin, float fmax, bool bsel)
{
	Post::FEPostMesh* pm = GetActiveMesh();
	FEState* ps = GetActiveState();
	int N = pm->Faces();
	for (int i = 0; i<N; ++i)
	{
		FSFace& f = pm->Face(i);
		if (f.IsEnabled() && f.IsVisible() && ((bsel == false) || (f.IsSelected())))
		{
			float v = ps->m_FACE[i].m_val;
			if ((v >= fmin) && (v <= fmax)) f.Select();
			else f.Unselect();
		}
	}
	UpdateSelectionLists();
}

//-----------------------------------------------------------------------------
// Hide selected elements
void CGLModel::HideSelectedElements()
{
	Post::FEPostMesh& mesh = *GetActiveMesh();

	// hide selected elements
	int NE = mesh.Elements();
	for (int i=0; i<NE; i++)
	{
		FEElement_& e = mesh.ElementRef(i);
		if (e.IsSelected()) e.Hide();
	}

	// hide nodes: nodes will be hidden if all elements they attach to are hidden
	int NN = mesh.Nodes();
	for (int i=0; i<NN; ++i) mesh.Node(i).m_ntag = 0;
	for (int i=0; i<mesh.Elements(); ++i)
	{
		FEElement_& el = mesh.ElementRef(i);
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

	UpdateSelectionLists();
}

//-----------------------------------------------------------------------------
// Hide selected elements
void CGLModel::HideUnselectedElements()
{
	Post::FEPostMesh& mesh = *GetActiveMesh();

	// hide unselected elements
	int NE = mesh.Elements();
	for (int i=0; i<NE; i++)
	{
		FEElement_& e = mesh.ElementRef(i);
		if (!e.IsSelected()) e.Hide();
	}

	// hide nodes: nodes will be hidden if all elements they attach to are hidden
	int NN = mesh.Nodes();
	for (int i=0; i<NN; ++i) mesh.Node(i).m_ntag = 0;
	for (int i=0; i<mesh.Elements(); ++i)
	{
		FEElement_& el = mesh.ElementRef(i);
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

	UpdateSelectionLists();
}

//-----------------------------------------------------------------------------
// Hide selected faces
void CGLModel::HideSelectedFaces()
{
	Post::FEPostMesh& mesh = *GetActiveMesh();
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
		FEElement_& el = mesh.ElementRef(i);
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

	UpdateSelectionLists();
}

//-----------------------------------------------------------------------------
// hide selected edges
void CGLModel::HideSelectedEdges()
{
	Post::FEPostMesh& mesh = *GetActiveMesh();

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
		FEElement_& el = mesh.ElementRef(i);
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

	UpdateSelectionLists();
}

//-----------------------------------------------------------------------------
// hide selected nodes
void CGLModel::HideSelectedNodes()
{
	Post::FEPostMesh& mesh = *GetActiveMesh();

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
		FEElement_& el = mesh.ElementRef(i);
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
		FEElement_& el = mesh.ElementRef(i);
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

	UpdateSelectionLists();
}

//-----------------------------------------------------------------------------
void CGLModel::SelectAllNodes()
{
	Post::FEPostMesh* mesh = GetActiveMesh();
	if (mesh == 0) return;

	for (int i = 0; i<mesh->Nodes(); i++)
	{
		FSNode& n = mesh->Node(i);
		if (n.IsVisible()) n.Select();
	}

	UpdateSelectionLists(SELECT_NODES);
}

//-----------------------------------------------------------------------------
void CGLModel::SelectAllEdges()
{
	Post::FEPostMesh* mesh = GetActiveMesh();
	if (mesh == 0) return;

	for (int i = 0; i<mesh->Edges(); i++)
	{
		FSEdge& e = mesh->Edge(i);
		if (e.IsVisible()) e.Select();
	}

	UpdateSelectionLists(SELECT_EDGES);
}

//-----------------------------------------------------------------------------
void CGLModel::SelectAllFaces()
{
	Post::FEPostMesh* mesh = GetActiveMesh();
	if (mesh == 0) return;

	for (int i = 0; i<mesh->Faces(); i++)
	{
		FSFace& f = mesh->Face(i);
		if (f.IsVisible()) f.Select();
	}

	UpdateSelectionLists(SELECT_FACES);
}

//-----------------------------------------------------------------------------
void CGLModel::SelectAllElements()
{
	Post::FEPostMesh* mesh = GetActiveMesh();
	if (mesh == 0) return;

	for (int i = 0; i<mesh->Elements(); i++)
	{
		FEElement_& e = mesh->ElementRef(i);
		if (e.IsVisible()) e.Select();
	}

	UpdateSelectionLists(SELECT_ELEMS);
}

//-----------------------------------------------------------------------------
void CGLModel::InvertSelectedNodes()
{
	Post::FEPostMesh* mesh = GetActiveMesh();
	if (mesh)
	{
		for (int i = 0; i<mesh->Nodes(); i++)
		{
			FSNode& n = mesh->Node(i);
			if (n.IsVisible())
				if (n.IsSelected()) n.Unselect(); else n.Select();
		}
	}
	UpdateSelectionLists(SELECT_NODES);
}

//-----------------------------------------------------------------------------
void CGLModel::InvertSelectedEdges()
{
	Post::FEPostMesh* mesh = GetActiveMesh();
	if (mesh)
	{
		for (int i = 0; i<mesh->Edges(); i++)
		{
			FSEdge& e = mesh->Edge(i);
			if (e.IsVisible())
				if (e.IsSelected()) e.Unselect(); else e.Select();
		}
	}
	UpdateSelectionLists(SELECT_EDGES);
}

//-----------------------------------------------------------------------------
void CGLModel::InvertSelectedFaces()
{
	Post::FEPostMesh* mesh = GetActiveMesh();
	if (mesh)
	{
		for (int i = 0; i<mesh->Faces(); i++)
		{
			FSFace& f = mesh->Face(i);
			if (f.IsVisible())
				if (f.IsSelected()) f.Unselect(); else f.Select();
		}
	}
	UpdateSelectionLists(SELECT_FACES);
}

//-----------------------------------------------------------------------------
void CGLModel::InvertSelectedElements()
{
	Post::FEPostMesh* mesh = GetActiveMesh();
	if (mesh)
	{
		for (int i = 0; i<mesh->Elements(); i++)
		{
			FEElement_& e = mesh->ElementRef(i);
			if (e.IsVisible())
				if (e.IsSelected()) e.Unselect(); else e.Select();
		}
	}
	UpdateSelectionLists(SELECT_ELEMS);
}

//-----------------------------------------------------------------------------
void CGLModel::UpdateEdge()
{
	m_edge.Clear();
	Post::FEPostMesh* mesh = GetActiveMesh();
	if (mesh == nullptr) return;

	for (int i=0; i<mesh->Elements(); ++i)
	{
		FEElement_& el = mesh->ElementRef(i);
		if ((el.Type() == FE_BEAM2)||(el.Type() == FE_BEAM3))
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
void CGLModel::ClearInternalSurfaces()
{
	for (int i=0; i<(int)m_innerSurface.size(); ++i) delete m_innerSurface[i];
	m_innerSurface.clear();
}

//-----------------------------------------------------------------------------
void CGLModel::BuildInternalSurfaces()
{
	ClearInternalSurfaces();

	int nmat = m_ps->Materials();
	for (int i = 0; i<nmat; ++i) m_innerSurface.push_back(new GLSurface);

	Post::FEPostMesh* pmesh = GetActiveMesh();
	if (pmesh == nullptr) return;
	Post::FEPostMesh& mesh = *pmesh;

	FSFace face;
	int ndom = mesh.Domains();
	for (int m = 0; m<ndom; ++m)
	{
		MeshDomain& dom = mesh.Domain(m);
		int NE = dom.Elements();

		float f1 = m_ps->GetMaterial(m)->transparency;

		for (int i = 0; i<NE; ++i)
		{
			FEElement_& el = dom.Element(i);
			if (el.IsVisible())
			{
				for (int j = 0; j<el.Faces(); ++j)
				{
					FEElement_* pen = mesh.ElementPtr(el.m_nbr[j]);
					if (pen && (pen->m_MatID == el.m_MatID))
					{
						bool badd = (pen->IsSelected() != el.IsSelected()) || !pen->IsVisible();

						/*						if ((badd == false) && (el.m_MatID != pen->m_MatID))
						{
						Material* pm2 = m_ps->GetMaterial(pen->m_MatID);
						float f2 = pm2->transparency;
						if ((f1 > f2) || (pm2->m_nrender == RENDER_MODE_WIRE)) badd = true;
						}
						*/
						if (badd)
						{
							el.GetFace(j, face);
							face.m_elem[0].eid = el.m_lid; // store the element ID. This is used for selection ???
							face.m_elem[1].eid = pen->m_lid;

							// calculate the face normals
							vec3f r0 = to_vec3f(mesh.Node(face.n[0]).r);
							vec3f r1 = to_vec3f(mesh.Node(face.n[1]).r);
							vec3f r2 = to_vec3f(mesh.Node(face.n[2]).r);

							face.m_fn = (r1 - r0) ^ (r2 - r0);
							for (int k = 0; k < face.Nodes(); ++k) face.m_nn[k] = face.m_fn;
							face.m_fn.Normalize();
							face.m_sid = 0;

							m_innerSurface[m]->add(face);
						}
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CGLModel::UpdateInternalSurfaces(bool eval)
{
	// Build the internal surfaces
	BuildInternalSurfaces();

	// reevaluate model
	if (eval) Update(false);
}

//-----------------------------------------------------------------------------
void CGLModel::GetSelectionList(vector<int>& L, int mode)
{
	L.clear();
	Post::FEPostMesh& m = *GetActiveMesh();
	switch (mode)
	{
	case SELECT_NODES:
	{
		for (int i = 0; i<m.Nodes(); ++i) if (m.Node(i).IsSelected()) L.push_back(i);
	}
	break;
	case SELECT_EDGES:
	{
		for (int i = 0; i<m.Edges(); ++i) if (m.Edge(i).IsSelected()) L.push_back(i);
	}
	break;
	case SELECT_FACES:
	{
		for (int i = 0; i<m.Faces(); ++i) if (m.Face(i).IsSelected()) L.push_back(i);
	}
	break;
	case SELECT_ELEMS:
	{
		for (int i = 0; i<m.Elements(); ++i) if (m.ElementRef(i).IsSelected()) L.push_back(i);
	}
	break;
	}
}

void CGLModel::ConvertSelection(int oldMode, int newMode)
{
	if (newMode == SELECT_NODES)
	{
		Post::FEPostMesh& mesh = *GetFSModel()->GetFEMesh(0);

		if (oldMode == SELECT_EDGES)
		{
			int NE = mesh.Edges();
			for (int i = 0; i<NE; ++i)
			{
				FSEdge& e = mesh.Edge(i);
				if (e.IsSelected())
				{
					e.Unselect();
					int ne = e.Nodes();
					for (int j = 0; j<ne; ++j)
						mesh.Node(e.n[j]).Select();
				}
			}
		}
		if (oldMode == SELECT_FACES)
		{
			int NF = mesh.Faces();
			for (int i = 0; i<NF; ++i)
			{
				FSFace& f = mesh.Face(i);
				if (f.IsSelected())
				{
					f.Unselect();
					int nf = f.Nodes();
					for (int j = 0; j<nf; ++j)
						mesh.Node(f.n[j]).Select();
				}
			}
		}
		if (oldMode == SELECT_ELEMS)
		{
			int NE = mesh.Elements();
			for (int i = 0; i<NE; ++i)
			{
				FEElement_& e = mesh.ElementRef(i);
				if (e.IsSelected())
				{
					e.Unselect();
					int ne = e.Nodes();
					for (int j = 0; j<ne; ++j)
						mesh.Node(e.m_node[j]).Select();
				}
			}
		}
	}

	UpdateSelectionLists();
}

void CGLModel::AddPlot(CGLPlot* pplot)
{
	pplot->SetModel(this);
	m_pPlot.Add(pplot);
	pplot->Update(CurrentTimeIndex(), 0.f, true);
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
