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
	m_ps = ps;
	SetName("Model");

	m_lastMesh = nullptr;

	m_stol = 60.0;

	m_bnorm = false;
	m_scaleNormals = 1.0;
	m_bghost = false;
	m_nDivs = 0; // this means "auto"
	m_brenderInteriorNodes = true;

	m_doZSorting = true;

	m_renderInnerSurface = true;

	m_bshowMesh = true;

	m_line_col = GLColor(0, 0, 0);
	m_node_col = GLColor(0, 0, 255);
	m_sel_col = GLColor(255, 0, 0);

	m_nrender = RENDER_MODE_SOLID;

	m_nconv = CONV_FR_XZ;

	m_selectType = SELECT_FE_ELEMS;
	m_selectStyle = SELECT_RECT;
	m_selection = nullptr;

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
		if ((*pd)->Type() == DATA_VEC3)
		{
			std::string sname = (*pd)->GetName();
			if ((sname == "displacement") || (sname == "Displacement")) ndisp = i;
		}
	}

	if (ndisp != -1)
	{
		ps->SetDisplacementField(BUILD_FIELD( DATA_CLASS::NODE_DATA, ndisp, 0));
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
	SetSelection(nullptr);
	ClearInternalSurfaces();
	m_ps = ps;
	if (ps) BuildInternalSurfaces();
}

//-----------------------------------------------------------------------------
void CGLModel::ShowShell2Solid(bool b) { m_render.m_bShell2Solid = b; }
bool CGLModel::ShowShell2Solid() const { return m_render.m_bShell2Solid; }

//-----------------------------------------------------------------------------
void CGLModel::ShowBeam2Solid(bool b) { m_render.m_bBeam2Solid = b; }
bool CGLModel::ShowBeam2Solid() const { return m_render.m_bBeam2Solid; }

//-----------------------------------------------------------------------------
void CGLModel::SolidBeamRadius(float f) { m_render.m_bSolidBeamRadius = f; }
float CGLModel::SolidBeamRadius() const { return m_render.m_bSolidBeamRadius; }

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
		if ((*pd)->Type() == DATA_VEC3) ++nv;
		if ((*pd)->GetName() == szvectorField) ndisp = i;
	}

	if (nv == 0) return false;

	if (m_pdis) delete m_pdis;
	m_pdis = new CGLDisplacementMap(this);
	if (ndisp != -1)
	{
		ps->SetDisplacementField(BUILD_FIELD(DATA_CLASS::NODE_DATA, ndisp, 0));
	}
	else ps->SetDisplacementField(-1);

	ResetAllStates();

	return true;
}

//-----------------------------------------------------------------------------
bool CGLModel::HasDisplacementMap()
{
	if (m_pdis == 0) return false;
	return (GetFSModel()->GetDisplacementField() >= 0);
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
void CGLModel::RemoveDisplacementMap()
{
	FEPostModel* ps = GetFSModel();
	ps->SetDisplacementField(-1);
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
	RenderInteriorNodes(rc.m_settings.m_bext == false);

	// get the FE model
	FEPostModel* fem = GetFSModel();

	m_bshowMesh = rc.m_settings.m_bmesh;

	int mode = GetSelectionType();

	// render the faces
	if (mode == SELECT_FE_FACES)
	{
		RenderFaces(m_ps, rc);
	}
	else if (mode == SELECT_FE_ELEMS)
	{
		RenderElems(m_ps, rc);
	}
	else
	{
		// for nodes, edges, draw the faces as well
		RenderSurface(m_ps, rc);
	}

	// render mesh lines
	if (m_bshowMesh && (GetSelectionType() != SELECT_FE_EDGES))
	{
		RenderMeshLines(rc);
	}

	// render outline
	if (rc.m_settings.m_bfeat)
	{
		GLColor c = m_line_col;
		glColor3ub(c.r, c.g, c.b);

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
	if (mode == SELECT_FE_EDGES)
	{
		rc.m_cam->LineDrawMode(true);
		RenderEdges(fem, rc);
		rc.m_cam->LineDrawMode(false);
	}

	// render the nodes
	if (mode == SELECT_FE_NODES)
	{
		rc.m_cam->LineDrawMode(true);
		RenderNodes(fem, rc);
		rc.m_cam->LineDrawMode(false);
	}

	// Render discrete elements
	float lineWidth;
	glGetFloatv(GL_LINE_WIDTH, &lineWidth);
	glLineWidth(rc.m_settings.m_line_size);
	RenderDiscrete(rc);
	glLineWidth(lineWidth);

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
	RenderObjects(rc);
}

//-----------------------------------------------------------------------------
void CGLModel::RenderMeshLines(CGLContext& rc)
{
	FEPostModel* fem = GetFSModel();
	if (fem == nullptr) return;
	for (int m = 0; m < fem->Materials(); ++m)
	{
		// get the material
		Material* pmat = fem->GetMaterial(m);

		// make sure the material is visible
		if (pmat->bvisible && pmat->bmesh)
		{
			// store attributes
			glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT);

			glDisable(GL_LIGHTING);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			if (pmat->bclip == false) CGLPlaneCutPlot::DisableClipPlanes();

			rc.m_cam->LineDrawMode(true);

			// set the material properties
			GLColor c = pmat->meshcol;
			glColor4ub(c.r, c.g, c.b, 128);
			RenderMeshLines(fem, m);

			rc.m_cam->LineDrawMode(false);

			CGLPlaneCutPlot::EnableClipPlanes();

			// restore attributes
			glPopAttrib();
		}
	}
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
	if (ShowBeam2Solid())
	{
		RenderDiscreteAsSolid(rc);
	}
	else
	{
		RenderDiscreteAsLines(rc);
	}
}

//-----------------------------------------------------------------------------
void CGLModel::RenderDiscreteAsLines(CGLContext& rc)
{
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	Post::FEPostMesh& mesh = *GetActiveMesh();
	int curMat = -1;
	bool bvisible = true;

	// render un-selected, active elements
	if (m_pcol->IsActive())
	{
		m_pcol->GetColorMap()->GetTexture().MakeCurrent();

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

				if (bvisible) RenderDiscreteElement(edge);
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

			if (bvisible) RenderDiscreteElement(edge);
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
			RenderDiscreteElement(edge);
		}
	}
	glEnd();

	glPopAttrib();
}

//-----------------------------------------------------------------------------
void CGLModel::RenderDiscreteElement(GLEdge::EDGE& edge)
{
	Post::FEPostMesh& mesh = *GetActiveMesh();
	FEElement_* pe = mesh.ElementPtr(edge.elem);
	if (pe == nullptr) return;

	if (pe->Type() == FE_BEAM2)
	{
		float t0 = edge.tex[0];
		float t1 = edge.tex[1];
		vec3d r0 = mesh.Node(edge.n0).r;
		vec3d r1 = mesh.Node(edge.n1).r;
		glTexCoord1d(t0); glVertex3d(r0.x, r0.y, r0.z);
		glTexCoord1d(t1); glVertex3d(r1.x, r1.y, r1.z);
	}
	else if (pe->Type() == FE_BEAM3)
	{
		vec3d r[3];
		r[0] = mesh.Node(pe->m_node[0]).r;
		r[1] = mesh.Node(pe->m_node[1]).r;
		r[2] = mesh.Node(pe->m_node[2]).r;
		float t[3];
		t[0] = edge.tex[0];
		t[1] = edge.tex[1];
		t[2] = 0.5f * (t[0] + t[1]);
		vec3d rp = r[0];
		float tp = t[0];
		const int NDIV = 12;
		for (int n = 1; n <= NDIV; ++n)
		{
			float w = -1.f + (n / (float)NDIV) * 2.f;
			float H[3] = { 0.5f * w * (w - 1.f), 0.5f * w * (w + 1.f), 1.f - w * w };
			vec3d rn = r[0] * H[0] + r[1] * H[1] + r[2] * H[2];
			float tn = t[0] * H[0] + t[1] * H[1] + t[2] * H[2];

			glTexCoord1d(tp); glVertex3d(rp.x, rp.y, rp.z);
			glTexCoord1d(tn); glVertex3d(rn.x, rn.y, rn.z);

			rp = rn;
			tp = tn;
		}
	}
}

//-----------------------------------------------------------------------------
void CGLModel::RenderDiscreteAsSolid(CGLContext& rc)
{
	glPushAttrib(GL_ENABLE_BIT);
	Post::FEPostMesh& mesh = *GetActiveMesh();
	int curMat = -1;
	bool bvisible = true;

	// find the shortest edge (that's not zero)
	double L2min = 0.0;
	for (int i = 0; i < m_edge.Edges(); ++i)
	{
		GLEdge::EDGE& edge = m_edge.Edge(i);
		FEElement_* pe = mesh.ElementPtr(edge.elem);
		if (pe)
		{
			vec3d r0 = mesh.Node(edge.n0).r;
			vec3d r1 = mesh.Node(edge.n1).r;
			double L = (r1 - r0).norm2();

			if ((L2min == 0.0) || (L < L2min))
			{
				L2min = L;
			}
		}
	}
	if (L2min == 0.0) return;
	double Lmin = sqrt(L2min);

	double f = m_render.m_bSolidBeamRadius;
	double W = f;// Lmin * 0.25 * f;

	// render un-selected, active elements
	if (m_pcol->IsActive())
	{
		m_pcol->GetColorMap()->GetTexture().MakeCurrent();
		glEnable(GL_TEXTURE_1D);

		glColor3ub(255, 255, 255);
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

				if (bvisible) RenderDiscreteElementAsSolid(edge, W);
			}
		}
	}

	// turn-off texturing for the rest
	glDisable(GL_TEXTURE_1D);
	glEnable(GL_CULL_FACE);

	// loop over un-selected, inactive elements, non-transparent
	curMat = -1;
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
				bvisible = pmat->bvisible && (pmat->transparency > 0.999f);
				GLColor c = pmat->diffuse;
				byte a = (byte) (255.f * pmat->transparency);
				glColor4ub(c.r, c.g, c.b, a);
				curMat = mat;
				if (m_pcol->IsActive() && pmat->benable) bvisible = false;
			}

			if (bvisible) RenderDiscreteElementAsSolid(edge, W);
		}
	}

	// loop over un-selected, inactive elements, transparent
	curMat = -1;
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
				bvisible = pmat->bvisible && (pmat->transparency < 0.999f);
				GLColor c = pmat->diffuse;
				byte a = (byte)(255.f * pmat->transparency);
				glColor4ub(c.r, c.g, c.b, a);
				curMat = mat;
				if (m_pcol->IsActive() && pmat->benable) bvisible = false;
			}

			if (bvisible)
			{
				vec3d r0 = mesh.Node(edge.n0).r;
				vec3d r1 = mesh.Node(edge.n1).r;
				glx::drawCappedCylinder(r0, r1, W);
			}
		}
	}


	// loop over selected elements
	glColor3ub(255, 0, 0);
	for (int i = 0; i < m_edge.Edges(); ++i)
	{
		GLEdge::EDGE& edge = m_edge.Edge(i);
		FEElement_* pe = mesh.ElementPtr(edge.elem);
		if (pe && pe->IsSelected() && pe->IsVisible())
		{
			RenderDiscreteElementAsSolid(edge, W);
		}
	}

	glPopAttrib();
}


//-----------------------------------------------------------------------------
void CGLModel::RenderDiscreteElementAsSolid(GLEdge::EDGE& edge, double W)
{
	Post::FEPostMesh& mesh = *GetActiveMesh();
	FEElement_* pe = mesh.ElementPtr(edge.elem);
	if (pe == nullptr) return;

	if (pe->Type() == FE_BEAM2)
	{
		vec3d r0 = mesh.Node(edge.n0).r;
		vec3d r1 = mesh.Node(edge.n1).r;
		float t0 = edge.tex[0];
		float t1 = edge.tex[1];

		int leftCap  = (pe->m_nbr[0] == -1 ? 1 : 0);
		int rightCap = (pe->m_nbr[1] == -1 ? 1 : 0);

		glx::drawCappedCylinder(r0, r1, W, t0, t1, 16, leftCap, rightCap);
	}
	else if (pe->Type() == FE_BEAM3)
	{
		vec3d r[3];
		r[0] = mesh.Node(pe->m_node[0]).r;
		r[1] = mesh.Node(pe->m_node[1]).r;
		r[2] = mesh.Node(pe->m_node[2]).r;
		const int NDIV = 12;
		vector<vec3d> p(NDIV + 1);
		for (int n = 0; n <= NDIV; ++n)
		{
			float w = -1.f + (n / (float)NDIV) * 2.f;
			float H[3] = { 0.5f * w * (w - 1.f), 0.5f * w * (w + 1.f), 1.f - w * w };
			p[n] = r[0] * H[0] + r[1] * H[1] + r[2] * H[2];
		}

		int leftCap  = (pe->m_nbr[0] == -1 ? 1 : 0);
		int rightCap = (pe->m_nbr[1] == -1 ? 1 : 0);

		glx::drawSmoothPath(p, W, edge.tex[0], edge.tex[1], leftCap, rightCap);
	}
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
	int mode = GetSelectionType();

	// get the mesh
	FEPostModel* ps = m_ps;
	Post::FEPostMesh* pm = GetActiveMesh();

	glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_POLYGON_BIT);

	// now render the selected faces
	glDisable(GL_TEXTURE_1D);
	GLColor c = m_sel_col;
	glDisable(GL_LIGHTING);

	int ndivs = GetSubDivisions();
	m_render.SetDivisions(ndivs);

	// render the selected faces
	if (mode == SELECT_FE_FACES)
	{
		glColor3ub(c.r, c.g, c.b);
		m_render.SetRenderMode(GLMeshRender::SelectionMode);
		m_render.RenderFEFaces(pm, [](const FSFace& face) {
			return face.IsSelected();
			});
	}

	// render the selected elements
	if (mode == SELECT_FE_ELEMS)
	{
		FEElementSelection* elemSelection = dynamic_cast<FEElementSelection*>(m_selection);
		if (elemSelection && (elemSelection->Size() > 0))
		{
			glColor3ub(255, 64, 0);
			m_render.SetRenderMode(GLMeshRender::SelectionMode);
			m_render.RenderFEFaces(pm, [=](const FSFace& face) {
				FEElement_& el = pm->ElementRef(face.m_elem[0].eid);
				return el.IsSelected();
				});
		}
	}

	// render the outline of the selected elements
	m_render.SetRenderMode(GLMeshRender::OutlineMode);
	glColor3f(1.f, 1.f, 0);

	// do the selected elements first
	if (mode == SELECT_FE_ELEMS)
	{
		FEElementSelection* elemSelection = dynamic_cast<FEElementSelection*>(m_selection);
		if (elemSelection && (elemSelection->Size() > 0))
		{
			// NOTE: This only renders outlines for solid elements
			const vector<int>& itemList = elemSelection->ItemList();
			m_render.RenderFEElementsOutline(*pm, itemList);
		}
	}

	// now do the selected faces
	if (mode == SELECT_FE_FACES)
	{
		FEFaceSelection* faceSelection = dynamic_cast<FEFaceSelection*>(m_selection);
		if (faceSelection && (faceSelection->Size() > 0))
		{
			const vector<int>& itemList = faceSelection->ItemList();
			m_render.RenderFEFacesOutline(pm, itemList);
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
	if (m >= pm->ElemSets()) return;

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

	// for better transparency we first draw all the backfacing polygons.
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_CULL_FACE);

	GLColor d = pmat->diffuse;
	double tm = pmat->transparency;

	int ndivs = GetSubDivisions();
	m_render.SetDivisions(ndivs);

	int mode = GetSelectionType();

	if (m_doZSorting)
	{
		int NF = dom.Faces();
		std::map< double, int> zmap;
		// first, build a list of faces
		for (int i = 0; i < NF; ++i)
		{
			FSFace& face = dom.Face(i);
			FEElement_& el = pm->ElementRef(face.m_elem[0].eid);

			if (((mode != SELECT_FE_ELEMS) || !el.IsSelected()) && face.IsVisible())
			{
				// get the face center
				vec3d r = pm->FaceCenter(face);

				// convert to eye coordinates
				vec3d q = rc.m_cam->WorldToCam(r);

				// add it to the z-list
				zmap[q.z] = i;
			}
		}

		// build the sorted face list
		const vector<int>& faceList = dom.FaceList();
		vector<int> sortedFaceList(zmap.size());
		auto it = zmap.begin();
		for (size_t i = 0; i < zmap.size(); ++i, ++it) sortedFaceList[i] = faceList[it->second];

		// render the list
		glDisable(GL_CULL_FACE);
		m_render.RenderFEFaces(pm, sortedFaceList, [&](const FSFace& face, GLColor* c) {
			GLubyte a[FSFace::MAX_NODES] = { 0 };
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
				for (int j = 0; j < face.Nodes(); ++j)
					c[j] = GLColor(255, 255, 255, a[j]);
			}
			else
			{
				for (int j = 0; j < face.Nodes(); ++j)
					c[j] = GLColor(d.r, d.g, d.b, a[j]);
			}

			return true;
		});
	}
	else
	{
		glCullFace(GL_FRONT);
		m_render.RenderFEFaces(pm, dom.FaceList(), [&](const FSFace& face, GLColor* c) {

			FEElement_& el = pm->ElementRef(face.m_elem[0].eid);

			if (((mode != SELECT_FE_ELEMS) || !el.IsSelected()) && face.IsVisible() && (m_renderInnerSurface || face.IsExterior()))
			{
				GLubyte a[FSFace::MAX_NODES];
				for (int j = 0; j < face.Nodes(); ++j)
				{
					vec3d r = to_vec3d(face.m_nn[j]);
					q.RotateVector(r);
					double z = 1 - fabs(r.z);
					a[j] = (GLubyte)(255 * (tm + 0.5 * (1 - tm) * (z * z)));
				}

				if (benable)
				{
					for (int j = 0; j < face.Nodes(); ++j)
						c[j] = GLColor(255, 255, 255, a[j]);
				}
				else
				{
					for (int j = 0; j < face.Nodes(); ++j)
						c[j] = GLColor(d.r, d.g, d.b, a[j]);
				}

				// okay, we got one, so let's render it
				return true;
			}
			return false;
		});

		// and then we draw the front-facing ones.
		glCullFace(GL_BACK);
		m_render.RenderFEFaces(pm, dom.FaceList(), [&](const FSFace& face, GLColor* c) {
			FEElement_& el = pm->ElementRef(face.m_elem[0].eid);

			if (((mode != SELECT_FE_ELEMS) || !el.IsSelected()) && face.IsVisible())
			{
				GLubyte a[FSFace::MAX_NODES];
				for (int j = 0; j < face.Nodes(); ++j)
				{
					vec3d r = to_vec3d(face.m_nn[j]);
					q.RotateVector(r);
					double z = 1 - fabs(r.z);
					a[j] = (GLubyte)(255 * (tm + 0.5 * (1 - tm) * (z * z)));
				}

				if (benable)
				{
					for (int j = 0; j < face.Nodes(); ++j)
						c[j] = GLColor(255, 255, 255, a[j]);
				}
				else
				{
					for (int j = 0; j < face.Nodes(); ++j)
						c[j] = GLColor(d.r, d.g, d.b, a[j]);
				}

				// okay, we got one, so let's render it
				return true;
			}
			return false;
		});
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
	m_render.RenderFEFaces(pm, surf.FaceList(), [](const FSFace& face) {
		return (face.IsActive());
		});

	// render inactive faces
	if (btex) glDisable(GL_TEXTURE_1D);
	m_render.RenderFEFaces(pm, surf.FaceList(), [](const FSFace& face) {
		return (face.IsActive() == false);
		});

	if (btex) glEnable(GL_TEXTURE_1D);
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

//-----------------------------------------------------------------------------
void CGLModel::RenderInnerSurfaceOutline(int m, int ndivs)
{
	m_render.SetDivisions(ndivs);
	Post::FEPostMesh* pm = GetActiveMesh();
	GLSurface& inSurf = *m_innerSurface[m];
	for (int i = 0; i<inSurf.Faces(); ++i)
	{
		FSFace& facet = inSurf.Face(i);
		m_render.RenderFaceOutline(facet, pm);
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
		std::map< double, int> zmap;
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
				zmap[q.z] = i;
			}
		}

		// build the sorted face list
		const vector<int>& faceList = dom.FaceList();
		vector<int> sortedFaceList(zmap.size());
		auto it = zmap.begin();
		for (size_t i = 0; i < zmap.size(); ++i, it++) sortedFaceList[i] = faceList[it->second];

		// render the list
		m_render.RenderFEFaces(pm, sortedFaceList);
	}
	else
	{
		m_render.RenderFEFaces(pm, dom.FaceList(), [](const FSFace& f) {
			return (f.m_ntag == 1);
			});
	}

	// render inactive faces
	if (activeOnly == false)
	{
		if (btex) glDisable(GL_TEXTURE_1D);

		if (m_pcol->IsActive() && benable) glColor4ub(m_col_inactive.r, m_col_inactive.g, m_col_inactive.b, m_col_inactive.a);

		if (zsort)
		{
			int NF = dom.Faces();
			std::map< double, int> zmap;
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
					zmap[q.z] = i;
				}
			}

			// build the sorted face list
			const vector<int>& faceList = dom.FaceList();
			vector<int> sortedFaceList(zmap.size());
			auto it = zmap.begin();
			for (size_t i = 0; i < zmap.size(); ++i, ++it) sortedFaceList[i] = faceList[it->second];

			// render the list
			m_render.RenderFEFaces(pm, sortedFaceList);
		}
		else
		{
			m_render.RenderFEFaces(pm, dom.FaceList(), [](const FSFace& f) {
				return (f.m_ntag == 2);
				});
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

		// only render the outline if it's not already shown
		if (rc.m_settings.m_bfeat == false)
		{
			SetMaterialParams(pmat);
			RenderOutline(rc, mat);
		}
	}
}

//-----------------------------------------------------------------------------
void CGLModel::RenderSolidMaterial(CGLContext& rc, FEPostModel* ps, int m, bool activeOnly)
{
	// make sure a part with this material exists
	FEPostMesh* pm = GetActiveMesh();
	if (pm == nullptr) return;
	if ((m<0) || (m >= pm->Domains())) return;

	MeshDomain& dom = pm->Domain(m);
	int NF = dom.Faces();
	if (NF == 0) return;

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
	int mode = GetSelectionType();

	// determine which faces to draw 
	// tag == 0 : no-draw
	// tag == 1 : draw active
	// tag == 2 : draw inactive
	// TODO: It seems that this can be precomputed and stored somewhere in the domains
	int numActiveFaces = 0;
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
				if (((mode != SELECT_FE_ELEMS) || !el.IsSelected()) && ((mode != SELECT_FE_FACES) || !face.IsSelected()) && face.IsVisible())
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
				if (((mode != SELECT_FE_ELEMS) || !el0.IsSelected()) && ((mode != SELECT_FE_FACES) || !face.IsSelected()))
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
				if (((mode != SELECT_FE_ELEMS) || !el1.IsSelected()) && ((mode != SELECT_FE_FACES) || !face.IsSelected()))
				{
					if (face.IsActive())
					{
						face.m_ntag = 1;
						numActiveFaces++;
					}
					else face.m_ntag = 2;
				}
			}
			else if (el0.IsVisible() && el1.IsVisible() && m_renderInnerSurface)
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
		if (mode != SELECT_FE_FACES)
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

	// get the face list
	vector<int> faceList;
	if (nmat != -1)
	{
		assert((nmat >= 0) && (nmat < pm->Domains()));
		if (pm->Domain(nmat).Faces() == 0) return;
		faceList = pm->Domain(nmat).FaceList();
	}
	else
	{
		faceList.resize(pm->Faces());
		for (int i = 0; i < pm->Faces(); ++i) faceList[i] = i;
	}
	if (faceList.empty()) return;

	// get some settings
	CGLCamera& cam = *rc.m_cam;
	quatd q = cam.GetOrientation();
	vec3d p = cam.GlobalPosition();
	int ndivs = GetSubDivisions();

	// this array will collect all points to render
	vector<vec3d> points; points.reserve(1024);

	// loop over all faces
	for (int i = 0; i < faceList.size(); ++i)
	{
		FSFace& f = pm->Face(faceList[i]);

		// NOTE: we don't want to draw outline of eroded elements.
		//       What I should do is flag eroded elements and faces as hidden
		//       so we don't need to do this elaborate check.
		bool faceVisible = f.IsVisible();
		if (faceVisible)
		{
			int eid = f.m_elem[0].eid;
			if (eid >= 0) faceVisible = (pm->Element(eid).IsEroded() == false);
		}
		
		if (faceVisible)
		{
			int n = f.Edges();
			for (int j = 0; j < n; ++j)
			{
				bool bdraw = false;

				if (f.m_nbr[j] < 0)
				{
					bdraw = true;
				}
				else
				{
					FSFace& f2 = pm->Face(f.m_nbr[j]);
					if ((f.m_gid != f2.m_gid) ||
						(f.m_sid != f2.m_sid) ||
						(f2.IsVisible() == false))
					{
						bdraw = true;
					}
					else if (rc.m_settings.m_nrender == RENDER_WIREFRAME)
					{
						vec3d n1 = to_vec3d(f.m_fn);
						vec3d n2 = to_vec3d(f2.m_fn);

						if (cam.IsOrtho())
						{
							q.RotateVector(n1);
							q.RotateVector(n2);
							if (n1.z * n2.z <= 0) bdraw = true;
						}
						else
						{
							int a = j;
							int b = (j + 1) % n;
							vec3d c = (pm->Node(f.n[a]).r + pm->Node(f2.n[b]).r) * 0.5;
							vec3d pc = p - c;
							double d1 = pc * n1;
							double d2 = pc * n2;
							if (d1 * d2 <= 0) bdraw = true;
						}
					}
				}

				if (bdraw)
				{
					int n = f.Edges();
					int a = f.n[j];
					int b = f.n[(j + 1) % n];
					if (a > b) { a ^= b; b ^= a; a ^= b; }

					switch (f.m_type)
					{
					case FE_FACE_TRI3:
					case FE_FACE_QUAD4:
					{
						points.push_back(pm->Node(a).r);
						points.push_back(pm->Node(b).r);
					}
					break;
					case FE_FACE_QUAD8:
					case FE_FACE_QUAD9:
					{
						vec3d r1 = pm->Node(a).r;
						vec3d r2 = pm->Node(b).r;
						vec3d r3 = pm->Node(f.n[j + 4]).r;

						float r, H[3];
						vec3d p;
						int n = (ndivs <= 1 ? 2 : ndivs);
						for (int i = 0; i < n; ++i)
						{
							r = -1.f + 2.f * i / n;
							H[0] = 0.5f * r * (r - 1.f);
							H[1] = 0.5f * r * (r + 1.f);
							H[2] = 1.f - r * r;
							p = r1 * H[0] + r2 * H[1] + r3 * H[2];
							points.push_back(p);

							r = -1.f + 2.f * (i + 1) / n;
							H[0] = 0.5f * r * (r - 1.f);
							H[1] = 0.5f * r * (r + 1.f);
							H[2] = 1.f - r * r;
							p = r1 * H[0] + r2 * H[1] + r3 * H[2];
							points.push_back(p);
						}
					}
					break;
					case FE_FACE_TRI6:
					case FE_FACE_TRI7:
					{
						vec3d r1 = pm->Node(a).r;
						vec3d r2 = pm->Node(b).r;
						vec3d r3 = pm->Node(f.n[j + 3]).r;

						float r, H[3];
						vec3d p;
						int n = (ndivs <= 1 ? 2 : ndivs);
						for (int i = 0; i < n; ++i)
						{
							r = -1.f + 2.f * i / n;
							H[0] = 0.5f * r * (r - 1.f);
							H[1] = 0.5f * r * (r + 1.f);
							H[2] = 1.f - r * r;
							p = r1 * H[0] + r2 * H[1] + r3 * H[2];
							points.push_back(p);

							r = -1.f + 2.f * (i + 1) / n;
							H[0] = 0.5f * r * (r - 1.f);
							H[1] = 0.5f * r * (r + 1.f);
							H[2] = 1.f - r * r;
							p = r1 * H[0] + r2 * H[1] + r3 * H[2];
							points.push_back(p);
						}
					}
					break;
					case FE_FACE_TRI10:
					{
						FSEdge e = f.GetEdge(j);

						vec3d r[4];
						r[0] = pm->Node(e.n[0]).r;
						r[1] = pm->Node(e.n[1]).r;
						r[2] = pm->Node(e.n[2]).r;
						r[3] = pm->Node(e.n[3]).r;

						vec3d p = r[0];
						int n = (ndivs < 3 ? 3 : ndivs);
						for (int i = 1; i <= n; ++i)
						{
							float w = (float)i / n;
							vec3d q = e.eval(r, w);
							points.push_back(p);
							points.push_back(q);
							p = q;
						}
					}
					break;
					default:
						assert(false);
					}
				}
			}
		}
	}

	// build the line mesh
	GLLineMesh lineMesh;
	lineMesh.Create((int)points.size() / 2);
	lineMesh.BeginMesh();
	for (auto& p : points) lineMesh.AddVertex(p);
	lineMesh.EndMesh();

	// render the active edges
	glPushAttrib(GL_ENABLE_BIT);
	{
		glDisable(GL_LIGHTING);
		lineMesh.Render();
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
	m_render.SetDivisions(ndivs);

	// now loop over all faces and see which face belongs to this material
	if (nmat < pm->Domains())
	{
		MeshDomain& dom = pm->Domain(nmat);
		for (int i=0; i<dom.Faces(); ++i)
		{
			FSFace& face = dom.Face(i);
			if (face.IsVisible())
			{
				// don't render lines on eroded elements
				if ((face.m_elem[0].eid >= 0) && (pm->Element(face.m_elem[0].eid).IsEroded() == false))
				{
					// okay, we got one, so let's render it
					m_render.RenderFaceOutline(face, pm);
				}
			}
		}
	}

	int mode = GetSelectionType();
	if (mode != SELECT_FE_FACES)
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
	m_render.RenderFENodes(*pm, [](const FSNode& node) {
		return (node.IsExterior() && node.m_ntag && (node.IsSelected() == false));
		});

	if (m_brenderInteriorNodes)
	{
		m_render.RenderFENodes(*pm, [](const FSNode& node) {
			return ((node.IsExterior() == false) && node.m_ntag && (node.IsSelected() == false));
			});
	}

	// render selected tagged nodes
	if (GetSelectionType() == SELECT_FE_NODES)
	{
		glDisable(GL_DEPTH_TEST);

		// render exterior selected nodes first
		glColor3ub(255, 255, 0);

		m_render.RenderFENodes(*pm, [](const FSNode& node) {
			return (node.IsExterior() && node.m_ntag && node.IsSelected());
			});

		// render interior nodes
		if (m_brenderInteriorNodes)
		{
			glColor3ub(255, 0, 0);
			m_render.RenderFENodes(*pm, [](const FSNode& node) {
				return ((node.IsExterior() == false) && node.m_ntag && node.IsSelected());
				});
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

	Post::FEPostMesh& mesh = *GetActiveMesh();
	int NE = mesh.Edges();

	GLLineMesh lineMesh;
	lineMesh.Create(NE * 2);

	// render unselected edges
	glColor3ub(0, 0, 255);
	lineMesh.BeginMesh();
	for (int i = 0; i<NE; ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		if (edge.IsVisible() && (edge.IsSelected() == false))
		{
			switch (edge.Type())
			{
			case FE_EDGE2:
				lineMesh.AddVertex(mesh.Node(edge.n[0]).r);
				lineMesh.AddVertex(mesh.Node(edge.n[1]).r);
				break;
			case FE_EDGE3:
				lineMesh.AddVertex(mesh.Node(edge.n[0]).r);
				lineMesh.AddVertex(mesh.Node(edge.n[1]).r);
				lineMesh.AddVertex(mesh.Node(edge.n[1]).r);
				lineMesh.AddVertex(mesh.Node(edge.n[2]).r);
				break;
			}
		}
	}
	lineMesh.EndMesh();
	lineMesh.Render();

	// render selected edges
	if (GetSelectionType() == SELECT_FE_EDGES)
	{
		glDisable(GL_DEPTH_TEST);
		glColor3ub(255, 255, 0);

		lineMesh.BeginMesh();
		for (int i = 0; i<NE; ++i)
		{
			FSEdge& edge = mesh.Edge(i);
			if (edge.IsVisible() && edge.IsSelected())
			{
				switch (edge.Type())
				{
				case FE_EDGE2:
					lineMesh.AddVertex(mesh.Node(edge.n[0]).r);
					lineMesh.AddVertex(mesh.Node(edge.n[1]).r);
					break;
				case FE_EDGE3:
					lineMesh.AddVertex(mesh.Node(edge.n[0]).r);
					lineMesh.AddVertex(mesh.Node(edge.n[1]).r);
					lineMesh.AddVertex(mesh.Node(edge.n[1]).r);
					lineMesh.AddVertex(mesh.Node(edge.n[2]).r);
					break;
				}
			}
		}
		lineMesh.EndMesh();
		lineMesh.Render();
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

	bool renderRB = rc.m_settings.m_brigid;
	bool renderRJ = rc.m_settings.m_bjoint;

	for (int i = 0; i < fem->PointObjects(); ++i)
	{
		Post::FEPostModel::PointObject & ob = *fem->GetPointObject(i);
		if (ob.IsActive())
		{
			glPushMatrix();
			glx::translate(ob.m_pos);
			glx::rotate(ob.m_rot);

			glx::translate(ob.m_rt);

			double size = R*ob.Scale();

			GLColor c = ob.Color();
			glColor3ub(c.r, c.g, c.b);
			switch (ob.m_tag)
			{
			case 1: if (renderRB) glx::renderRigidBody(size); break;
			case 2: if (renderRJ) glx::renderJoint(size); break;
			case 3: if (renderRJ) glx::renderJoint(size); break;
			case 4: if (renderRJ) glx::renderPrismaticJoint(size); break;
			case 5: if (renderRJ) glx::renderRevoluteJoint(size); break;
			case 6: if (renderRJ) glx::renderCylindricalJoint(size); break;
			case 7: if (renderRJ) glx::renderPlanarJoint(size); break;
			default:
				if (renderRB) glx::renderAxis(size);
			}
			glPopMatrix();
		}
	}

	for (int i = 0; i < fem->LineObjects(); ++i)
	{
		Post::FEPostModel::LineObject & ob = *fem->GetLineObject(i);
		if (ob.IsActive() && renderRJ)
		{
			glPushMatrix();
			glx::translate(ob.m_pos);
			glx::rotate(ob.m_rot);

			vec3d a = ob.m_r1;
			vec3d b = ob.m_r2;
			double Lt = sqrt((a - b) * (a - b));

			double L0 = sqrt((ob.m_r01 - ob.m_r02) * (ob.m_r01 - ob.m_r02));
			if (L0 == 0) L0 = Lt;

			GLColor c = ob.Color();
			glColor3ub(c.r, c.g, c.b);
			switch (ob.m_tag)
			{
			case 1: glx::renderSpring(a, b, R, (R == 0 ? 25 : L0 / R)); break;
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
}

// Show elements with a certain material ID
void CGLModel::UpdateMeshVisibility()
{
	Post::FEPostMesh& mesh = *GetActiveMesh();
	Post::FEPostModel& fem = *GetFSModel();

	int NE = mesh.Elements();
	for (int i = 0; i < NE; ++i)
	{
		FEElement_& e = mesh.ElementRef(i);
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
			FEElement_& e0 = mesh.ElementRef(f.m_elem[0].eid);
			FEElement_& e1 = mesh.ElementRef(f.m_elem[1].eid);
			if (!e0.IsInvisible() || !e1.IsInvisible()) bshow = true;
		}
		f.Show(bshow);
	}

	// show nodes
	int NN = mesh.Nodes();
	for (int i = 0; i < NN; ++i) mesh.Node(i).m_ntag = 0;
	for (int i = 0; i < mesh.Elements(); ++i)
	{
		FEElement_& el = mesh.ElementRef(i);
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
		else if ((f.m_elem[1].eid >= 0) && (mesh.ElementRef(f.m_elem[1].eid).IsEnabled())) f.Enable();
	}
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
	case SELECT_FE_NODES:
	{
		for (int i = 0; i<m.Nodes(); ++i) if (m.Node(i).IsSelected()) L.push_back(i);
	}
	break;
	case SELECT_FE_EDGES:
	{
		for (int i = 0; i<m.Edges(); ++i) if (m.Edge(i).IsSelected()) L.push_back(i);
	}
	break;
	case SELECT_FE_FACES:
	{
		for (int i = 0; i<m.Faces(); ++i) if (m.Face(i).IsSelected()) L.push_back(i);
	}
	break;
	case SELECT_FE_ELEMS:
	{
		for (int i = 0; i<m.Elements(); ++i) if (m.ElementRef(i).IsSelected()) L.push_back(i);
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
