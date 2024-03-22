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
#include "GLPlaneCutPlot.h"
#include <GLLib/GLContext.h>
#include <GLLib/GLCamera.h>
#include "GLModel.h"
#include <MeshLib/hex.h>
#include <MeshTools/FESelection.h>
using namespace Post;

extern int LUT[256][15];
extern int LUT2D[16][4];
extern int ET_HEX[12][2];
extern int ET2D[4][2];

const int HEX_NT[8] = {0, 1, 2, 3, 4, 5, 6, 7};
const int PEN_NT[8] = {0, 1, 2, 2, 3, 4, 5, 5};
const int TET_NT[8] = {0, 1, 2, 2, 3, 3, 3, 3};
const int PYR_NT[8] = {0, 1, 2, 3, 4, 4, 4, 4};
const int QUAD_NT[4] = { 0, 1, 2, 3 };
const int TRI_NT[4]  = { 0, 1, 2, 2 };

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

vector<int> CGLPlaneCutPlot::m_clip;
vector<CGLPlaneCutPlot*> CGLPlaneCutPlot::m_pcp;

REGISTER_CLASS(CGLPlaneCutPlot, CLASS_PLOT, "planecut", 0);

CGLPlaneCutPlot::CGLPlaneCutPlot()
{
	SetTypeString("planecut");

	SetRenderOrder(1);

	static int n = 1;
	char szname[128] = { 0 };
	sprintf(szname, "Planecut.%02d", n++);
	SetName(szname);

	AddBoolParam(true, "show_plane");
	AddBoolParam(true, "cut_hidden");
	AddBoolParam(true, "show_mesh" );
	AddColorParam(GLColor(0, 0, 0), "mesh_color");
	AddDoubleParam(0, "transparency")->SetFloatRange(0.0, 1.0);
	AddDoubleParam(0, "x-normal")->SetFloatRange(-1.0, 1.0);
	AddDoubleParam(0, "y-normal")->SetFloatRange(-1.0, 1.0);
	AddDoubleParam(0, "z-normal")->SetFloatRange(-1.0, 1.0);
	AddDoubleParam(0, "offset")->SetFloatRange(-1.0, 1.0, 0.01);

	m_normal = vec3d(1, 0, 0);
	m_offset = 0.0;
	m_scl = 1.0;

	m_rot = 0.f;
	m_transparency = 0.25;
	m_bcut_hidden = false;
	m_bshowplane = true;
	m_bshow_mesh = false;

	m_meshColor = GLColor(0, 0, 0);

	m_nclip = GetFreePlane();
	if (m_nclip >= 0) m_pcp[m_nclip] = this;

	m_bupdateSlice = false;

	UpdateData(false);
}

CGLPlaneCutPlot::~CGLPlaneCutPlot()
{
	ReleasePlane();
}

bool CGLPlaneCutPlot::UpdateData(bool bsave)
{
	if (bsave)
	{
		m_bshowplane  = GetBoolValue(SHOW_PLANE);
		m_bcut_hidden = GetBoolValue(CUT_HIDDEN);
		m_bshow_mesh  = GetBoolValue(SHOW_MESH);
		m_meshColor = GetColorValue(MESH_COLOR);
		m_transparency = GetFloatValue(TRANSPARENCY);
		m_normal.x = GetFloatValue(NORMAL_X);
		m_normal.y = GetFloatValue(NORMAL_Y);
		m_normal.z = GetFloatValue(NORMAL_Z);
		m_offset = GetFloatValue(OFFSET);

		m_bupdateSlice = true;
	}
	else
	{
		SetBoolValue(SHOW_PLANE, m_bshowplane);
		SetBoolValue(CUT_HIDDEN, m_bcut_hidden);
		SetBoolValue(SHOW_MESH, m_bshow_mesh);
		SetColorValue(MESH_COLOR, m_meshColor);
		SetFloatValue(TRANSPARENCY, m_transparency);
		SetFloatValue(NORMAL_X, m_normal.x);
		SetFloatValue(NORMAL_Y, m_normal.y);
		SetFloatValue(NORMAL_Z, m_normal.z);
		SetFloatValue(OFFSET  , m_offset);
	}

	return false;
}

void CGLPlaneCutPlot::DisableClipPlanes()
{
	for (int i=0; i<(int) m_clip.size(); ++i)
	{
		if (m_clip[i] != 0) glDisable(GL_CLIP_PLANE0 + i);
	}
}

void CGLPlaneCutPlot::ClearClipPlanes()
{
	for (int i = 0; i<(int)m_clip.size(); ++i)
	{
		if (m_clip[i] != 0) glDisable(GL_CLIP_PLANE0 + i);
		m_clip[i] = 0;
		m_pcp[i] = nullptr;
	}
}

void CGLPlaneCutPlot::EnableClipPlanes()
{
	for (int i=0; i<(int) m_clip.size(); ++i)
	{
		if (m_clip[i] != 0)
		{
			glEnable(GL_CLIP_PLANE0 + i);

			CGLPlaneCutPlot* pc = m_pcp[i];

			if (pc)
			{
				// get the plane equations
				GLdouble a[4];
				pc->GetNormalizedEquations(a);

				// set the clip plane coefficients
				glClipPlane(GL_CLIP_PLANE0 + i, a);
			}
		}
	}
}

void CGLPlaneCutPlot::InitClipPlanes()
{
	// allocate the clip array
	if (m_clip.size() == 0)
	{
		int N = 0;
		glGetIntegerv(GL_MAX_CLIP_PLANES, &N);
		assert(N);
		m_clip.assign(N, 0);
		m_pcp.assign(N, 0);
	}
}

void CGLPlaneCutPlot::Update(int ntime, float dt, bool breset)
{
	m_bupdateSlice = true;
}

///////////////////////////////////////////////////////////////////////////////

void CGLPlaneCutPlot::GetNormalizedEquations(double a[4])
{
	vec3d n(m_normal);
	m_T.GetRotation().RotateVector(n);
	n.Normalize();

	double a3 = m_T.GetPosition()*n - m_scl*m_offset;

	a[0] = n.x;
	a[1] = n.y;
	a[2] = n.z;
	a[3] = a3;
}

//-----------------------------------------------------------------------------
// Return the plane normal
vec3d CGLPlaneCutPlot::GetPlaneNormal() const
{
	return m_normal;
}

void CGLPlaneCutPlot::SetPlaneNormal(const vec3d& n)
{
	m_normal = n;
	m_normal.Normalize();
	SetFloatValue(NORMAL_X, m_normal.x);
	SetFloatValue(NORMAL_Y, m_normal.y);
	SetFloatValue(NORMAL_Z, m_normal.z);
}

float CGLPlaneCutPlot::GetPlaneOffset()
{
	return (float) m_offset;
}

void CGLPlaneCutPlot::SetPlaneOffset(float a)
{
	m_offset = a;
	SetFloatValue(OFFSET, a);
}

float CGLPlaneCutPlot::GetOffsetScale() const
{
	return m_scl;
}

vec3d CGLPlaneCutPlot::GetPlanePosition() const
{
	vec3d p = m_T.GetPosition();
	vec3d N = GetPlaneNormal(); N.Normalize();
	p += N * (m_scl * m_offset);
	return p;
}

///////////////////////////////////////////////////////////////////////////////

void CGLPlaneCutPlot::Render(CGLContext& rc)
{
	// make sure we have a clip plane ID assigned
	if (m_nclip == -1) return;

	// see if we are tracking or not
	vec3d r = m_T.GetPosition();
	if (rc.m_btrack)
	{
		m_T.SetPosition(-rc.m_track_pos);
		m_T.SetRotation(rc.m_track_rot);
	}
	else
	{
		BOX box = GetModel()->GetFSModel()->GetBoundingBox();
		m_T.SetPosition(-box.Center());
		m_T.SetRotation(quatd(0.0, vec3d(1, 0, 0)));
	}

	if (m_bupdateSlice || ((r == m_T.GetPosition()) == false))
	{
		UpdatePlaneCut();
		m_bupdateSlice = false;
	}

	// get the plane equations
	GLdouble a[4];
	GetNormalizedEquations(a);

	// set the clip plane coefficients
	glClipPlane(GL_CLIP_PLANE0 + m_nclip, a);

	// make sure the current clip plane is not active
	glDisable(GL_CLIP_PLANE0 + m_nclip);

	// render the slice
	RenderSlice();

	// render the mesh
	if (m_bshow_mesh)
	{
		rc.m_cam->LineDrawMode(true);
		RenderMesh();
		RenderOutline();
		rc.m_cam->LineDrawMode(false);
	}

	if (rc.m_settings.m_bfeat)
	{
		RenderOutline();
	}

	// render the plane
	if (m_bshowplane)
	{
		glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT);
		glDisable(GL_LIGHTING);
		DisableClipPlanes();
//		rc.m_cam->LineDrawMode(true);
		RenderPlane();
//		rc.m_cam->LineDrawMode(false);
		EnableClipPlanes();
		glPopAttrib();
	}

	// enable the clip plane
	glEnable(GL_CLIP_PLANE0 + m_nclip);
}

//-----------------------------------------------------------------------------
void CGLPlaneCutPlot::UpdateTriMesh()
{
	CGLModel* mdl = GetModel();
	FEPostModel* ps = mdl->GetFSModel();
	FSMeshBase* pm = mdl->GetActiveMesh();

	float rng[2];
	CGLColorMap& colorMap = *mdl->GetColorMap();
	colorMap.GetRange(rng);
	if (rng[1] == rng[0]) ++rng[1];

	vector<pair<vec3d, double> > activePoints; activePoints.reserve(1024);
	vector<pair<vec3d, GLColor> > inactivePoints; inactivePoints.reserve(1024);

	// loop over all enabled materials
	for (int n = 0; n < ps->Materials(); ++n)
	{
		Material* pmat = ps->GetMaterial(n);
		if ((pmat->bvisible || m_bcut_hidden) && pmat->bclip)
		{
			// repeat over all active faces
			int NF = m_slice.Faces();
			for (int i = 0; i < NF; ++i)
			{
				GLSlice::FACE& face = m_slice.Face(i);
				if ((face.mat == n) && (face.bactive))
				{
					vec3d* r = face.r;
					float* tex = face.tex;

					float t1 = (tex[0] - rng[0]) / (rng[1] - rng[0]);
					float t2 = (tex[1] - rng[0]) / (rng[1] - rng[0]);
					float t3 = (tex[2] - rng[0]) / (rng[1] - rng[0]);

					activePoints.push_back(pair<vec3d, double>(r[0], t1));
					activePoints.push_back(pair<vec3d, double>(r[1], t2));
					activePoints.push_back(pair<vec3d, double>(r[2], t3));
				}
			}

			// render inactive faces
			for (int i = 0; i < NF; ++i)
			{
				GLSlice::FACE& face = m_slice.Face(i);
				if ((face.mat == n) && (!face.bactive))
				{
					// render the face
					vec3d* r = face.r;
					float* tex = face.tex;
					inactivePoints.push_back(pair < vec3d, GLColor>(r[0], pmat->diffuse));
					inactivePoints.push_back(pair < vec3d, GLColor>(r[1], pmat->diffuse));
					inactivePoints.push_back(pair < vec3d, GLColor>(r[2], pmat->diffuse));
				}
			}
		}
	}

	if (activePoints.empty())
	{
		m_activeMesh.Clear();
	}
	else
	{
		m_activeMesh.Create(activePoints.size()/3, GLMesh::FLAG_TEXTURE);
		m_activeMesh.BeginMesh();
		{
			for (auto& v : activePoints) m_activeMesh.AddVertex(v.first, v.second);
		}
		m_activeMesh.EndMesh();
	}

	if (inactivePoints.empty())
	{
		m_inactiveMesh.Clear();
	}
	else
	{
		m_inactiveMesh.Create(inactivePoints.size()/3, GLMesh::FLAG_COLOR);
		m_inactiveMesh.BeginMesh();
		{
			for (auto& v : inactivePoints) m_inactiveMesh.AddVertex(v.first, v.second);
		}
		m_inactiveMesh.EndMesh();
	}
}

//-----------------------------------------------------------------------------
// Render the plane cut slice 
void CGLPlaneCutPlot::RenderSlice()
{
	CGLModel* mdl = GetModel();
	CGLColorMap* pcol = mdl->GetColorMap();

	// get the plane equations
	GLdouble a[4];
	GetNormalizedEquations(a);

	// set the plane normal
	vec3d norm(a[0], a[1], a[2]); norm.Normalize();
	glNormal3d(norm.x, norm.y, norm.z);

	// render active (i.e. textured) mesh
	GLTexture1D& tex = pcol->GetColorMap()->GetTexture();
	glDisable(GL_CULL_FACE);
	glEnable(GL_COLOR_MATERIAL);

	glEnable(GL_TEXTURE_1D);
	tex.MakeCurrent();
	glColor3ub(255, 255, 255);

	m_activeMesh.Render();

	// now render the inactive mesh
	glDisable(GL_TEXTURE_1D);

	m_inactiveMesh.Render();
}

//-----------------------------------------------------------------------------
void CGLPlaneCutPlot::UpdateLineMesh()
{
	CGLModel* mdl = GetModel();
	FEPostModel* ps = mdl->GetFSModel();
	FEPostMesh* pm = mdl->GetActiveMesh();

	EDGE edge[15];
	int en[8];
	int ne;

	float ev[8];
	vec3d ex[8];

	vec3d r[3];

	// get the plane equation
	double a[4];
	GetNormalizedEquations(a);

	// set the plane normal
	vec3d norm((float)a[0], (float)a[1], (float)a[2]);

	double ref = -a[3];

	Post::FEState& state = *ps->CurrentState();

	int matId = -1;
	Material* pmat = nullptr;

	// repeat over all elements
	vector<vec3d> points; points.reserve(1024);
	for (int i=0; i<pm->Elements(); ++i)
	{
		// render only when visible
		FEElement_& el = pm->ElementRef(i);
		if (el.m_MatID != matId)
		{
			pmat = ps->GetMaterial(el.m_MatID);
			matId = el.m_MatID;
		}

		if (pmat && (el.m_ntag > 0) && el.IsSolid() && (pmat->bmesh) && (pmat->bvisible || m_bcut_hidden) && (pmat->bclip))
		{
			const int* nt = nullptr;
			switch (el.Type())
			{
			case FE_HEX8   : nt = HEX_NT; break;
			case FE_HEX20  : nt = HEX_NT; break;
			case FE_HEX27  : nt = HEX_NT; break;
			case FE_PENTA6 : nt = PEN_NT; break;
            case FE_PENTA15: nt = PEN_NT; break;
            case FE_TET4   : nt = TET_NT; break;
            case FE_TET5   : nt = TET_NT; break;
			case FE_TET10  : nt = TET_NT; break;
			case FE_TET15  : nt = TET_NT; break;
			case FE_TET20  : nt = TET_NT; break;
            case FE_PYRA5  : nt = PYR_NT; break;
            case FE_PYRA13 : nt = PYR_NT; break;
			default:
				assert(false);
				continue;
			}

			// calculate the case of the element
			int ncase = el.m_ntag;

			// get the nodal values
			for (int k=0; k<8; ++k)
			{
				int nk = el.m_node[nt[k]];
				FSNode& node = pm->Node(nk);
				en[k] = nk;
				ev[k] = state.m_NODE[nk].m_val;
				ex[k] = node.r;
			}

			// loop over faces
			int* pf = LUT[ncase];
			ne = 0;
			for (int l=0; l<5; l++)
			{
				if (*pf == -1) break;

				// calculate nodal positions
				for (int k=0; k<3; k++)
				{
					int n1 = ET_HEX[pf[k]][0];
					int n2 = ET_HEX[pf[k]][1];

					double w1 = norm*ex[n1];
					double w2 = norm*ex[n2];
	
					double w = 0.0;
					if (w2 != w1)
						w = (ref - w1)/(w2 - w1);

					r[k] = ex[n1]*(1-w) + ex[n2]*w;
				}

				// add all edges to the list
				for (int k=0; k<3; ++k)
				{
					int n1 = pf[k];
					int n2 = pf[(k+1)%3];

					bool badd = true;
					for (int m=0; m<ne; ++m)
					{
						int m1 = edge[m].m_n[0];
						int m2 = edge[m].m_n[1];
						if (((n1 == m1) && (n2 == m2)) ||
							((n1 == m2) && (n2 == m1)))
						{
							badd = false;
							edge[m].m_ntag++;
							break;
						}
					}

					if (badd)
					{
						edge[ne].m_n[0] = n1;
						edge[ne].m_n[1] = n2;
						edge[ne].m_r[0] = r[k];
						edge[ne].m_r[1] = r[(k+1)%3];
						edge[ne].m_ntag = 0;
						++ne;
					}
				}
	
				pf+=3;
			}

			// add the lines
			for (int k=0; k<ne; ++k)
				if (edge[k].m_ntag == 0)
				{
					vec3d& r0 = edge[k].m_r[0];
					vec3d& r1 = edge[k].m_r[1];
					points.push_back(r0);
					points.push_back(r1);
				}
		}
	}

	if (points.empty())
	{
		m_lineMesh.Clear();
		return;
	}
	else
	{
		m_lineMesh.Create(points.size() / 2);
		m_lineMesh.BeginMesh();
		{
			for (auto& v : points) m_lineMesh.AddVertex(v);
		}
		m_lineMesh.EndMesh();
	}
}

//-----------------------------------------------------------------------------
// Render the mesh of the plane cut
void CGLPlaneCutPlot::RenderMesh()
{
	GLColor c = m_meshColor;
	glColor3ub(c.r, c.g, c.b);	

	// store attributes
	glPushAttrib(GL_ENABLE_BIT);

	// set attributes
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_1D);

	m_lineMesh.Render();

	// restore attributes
	glPopAttrib();
}

//-----------------------------------------------------------------------------
void CGLPlaneCutPlot::UpdateOutlineMesh()
{
	m_outlineMesh.Create(m_slice.Edges());
	m_outlineMesh.BeginMesh();
	for (int i = 0; i < m_slice.Edges(); ++i)
	{
		GLSlice::EDGE& edge = m_slice.Edge(i);
		m_outlineMesh.AddLine(edge.r[0], edge.r[1]);
	}
	m_outlineMesh.EndMesh();
}

//-----------------------------------------------------------------------------
// Render the outline of the mesh of the plane cut
void CGLPlaneCutPlot::RenderOutline()
{
	// store attributes
	glPushAttrib(GL_ENABLE_BIT);

	// set attributes
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_1D);
	glEnable(GL_COLOR_MATERIAL);

	// because plots are drawn before the mesh
	// we get visual artifacts from the background seeping through.
	// therefor we turn blending of
	glDisable(GL_BLEND);

	glColor3ub(0,0,0);
	m_outlineMesh.Render();

	// restore attributes
	glPopAttrib();
}

//-----------------------------------------------------------------------------
void CGLPlaneCutPlot::UpdatePlaneCut()
{
	UpdateSlice();
	UpdateTriMesh();
	UpdateLineMesh();
	UpdateOutlineMesh();
}

//-----------------------------------------------------------------------------
void CGLPlaneCutPlot::UpdateSlice()
{
	// Get the bounding box. We need it for determining the scale
	CGLModel* mdl = GetModel();
	BOX box = GetModel()->GetFSModel()->GetBoundingBox();
	double R = box.Radius();
	m_scl = (R == 0.0 ? 1.0 : R);

	// get the plane equations
	GLdouble a[4];
	GetNormalizedEquations(a);

	// set the plane normal
	vec3d norm((float)a[0], (float)a[1], (float)a[2]);

	double ref = -a[3];

	FEPostModel* ps = mdl->GetFSModel();
	FEPostMesh* pm = mdl->GetActiveMesh();

	m_slice.Clear();

	Post::FEState& state = *ps->CurrentState();

	// loop over all domains
	for (int n = 0; n < pm->Domains(); ++n)
	{
		MeshDomain& dom = pm->Domain(n);
		int matId = dom.GetMatID();
		if ((matId >= 0) && (matId < ps->Materials()))
		{
			Material* pmat = ps->GetMaterial(matId);
			if ((pmat->bvisible || m_bcut_hidden) && pmat->bclip)
			{
				AddDomain(pm, n);
			}
		}
	}

	AddFaces(pm);
}

void CGLPlaneCutPlot::AddDomain(FEPostMesh* pm, int n)
{
	float ev[8];
	vec3d ex[8];
	int	nf[8];
	EDGE edge[15];
	int en[8];
	int	rf[3];

	MeshDomain& dom = pm->Domain(n);

	// get the plane equations
	GLdouble a[4];
	GetNormalizedEquations(a);

	// set the plane normal
	vec3d norm((float)a[0], (float)a[1], (float)a[2]);

	double ref = -a[3];

	CGLModel* mdl = GetModel();
	int ndivs = mdl->GetSubDivisions();

	FEPostModel* ps = mdl->GetFSModel();
	Post::FEState& state = *ps->CurrentState();

	// repeat over all elements
	for (int i = 0; i < dom.Elements(); ++i)
	{
		// render only when visible
		FEElement_& el = dom.Element(i);
		if ((el.IsVisible() || m_bcut_hidden) && el.IsSolid())
		{
			const int *nt = nullptr;
			switch (el.Type())
			{
			case FE_HEX8: nt = HEX_NT; break;
			case FE_HEX20: nt = HEX_NT; break;
			case FE_HEX27: nt = HEX_NT; break;
			case FE_PENTA6: nt = PEN_NT; break;
			case FE_PENTA15: nt = PEN_NT; break;
			case FE_TET4: nt = TET_NT; break;
			case FE_TET5: nt = TET_NT; break;
			case FE_TET10: nt = TET_NT; break;
			case FE_TET15: nt = TET_NT; break;
			case FE_TET20: nt = TET_NT; break;
			case FE_PYRA5: nt = PYR_NT; break;
            case FE_PYRA13: nt = PYR_NT; break;
			}

			// get the nodal values
			for (int k = 0; k < 8; ++k)
			{
				FSNode& node = pm->Node(el.m_node[nt[k]]);
				nf[k] = (node.IsExterior() ? 1 : 0);
				ex[k] = node.r;
				en[k] = el.m_node[nt[k]];
				ev[k] = state.m_NODE[el.m_node[nt[k]]].m_val;
			}

			// calculate the case of the element
			int ncase = 0;
			for (int k = 0; k < 8; ++k)
				if (norm*ex[k] >= ref) ncase |= (1 << k);

			el.m_ntag = ncase;

			if ((ndivs <= 1) || (el.Shape() != ELEM_HEX))
			{
				// loop over faces
				int* pf = LUT[ncase];
				int ne = 0;
				for (int l = 0; l < 5; l++)
				{
					if (*pf == -1) break;

					// calculate nodal positions
					vec3d r[3];
					float tex[3], w1, w2, w;
					for (int k = 0; k < 3; k++)
					{
						int n1 = ET_HEX[pf[k]][0];
						int n2 = ET_HEX[pf[k]][1];

						w1 = norm * ex[n1];
						w2 = norm * ex[n2];

						if (w2 != w1)
							w = (ref - w1) / (w2 - w1);
						else
							w = 0.f;

						float v = ev[n1] * (1 - w) + ev[n2] * w;

						r[k] = ex[n1] * (1 - w) + ex[n2] * w;
						tex[k] = v;
						rf[k] = ((nf[n1] == 1) && (nf[n2] == 1) ? 1 : 0);
					}

					GLSlice::FACE face;
					face.mat = n;
					face.r[0] = r[0];
					face.r[1] = r[1];
					face.r[2] = r[2];
					face.tex[0] = tex[0];
					face.tex[1] = tex[1];
					face.tex[2] = tex[2];
					face.bactive = el.IsActive();

					m_slice.AddFace(face);

					pf += 3;
				}
			}
			else
			{
				for (int ix = 0; ix < ndivs; ++ix)
				{
					double wr0 = -1.0 + 2.0*ix / ndivs;
					double wr1 = -1.0 + 2.0*(ix + 1) / ndivs;
					for (int iy = 0; iy < ndivs; ++iy)
					{
						double ws0 = -1.0 + 2.0*iy / ndivs;
						double ws1 = -1.0 + 2.0*(iy + 1) / ndivs;
						for (int iz = 0; iz < ndivs; ++iz)
						{
							double wt0 = -1.0 + 2.0*iz / ndivs;
							double wt1 = -1.0 + 2.0*(iz + 1) / ndivs;

							double H[8][8];
							HEX8::shape(H[0], wr0, ws0, wt0);
							HEX8::shape(H[1], wr1, ws0, wt0);
							HEX8::shape(H[2], wr1, ws1, wt0);
							HEX8::shape(H[3], wr0, ws1, wt0);
							HEX8::shape(H[4], wr0, ws0, wt1);
							HEX8::shape(H[5], wr1, ws0, wt1);
							HEX8::shape(H[6], wr1, ws1, wt1);
							HEX8::shape(H[7], wr0, ws1, wt1);

							vec3d x[8];
							float v[8];
							for (int kk = 0; kk < 8; ++kk)
							{
								double* h = H[kk];
								x[kk] = vec3d(0, 0, 0);
								v[kk] = 0.0;
								for (int jj = 0; jj < 8; ++jj)
								{
									x[kk] += ex[jj] * h[jj];
									v[kk] += ev[jj] * h[jj];
								}
							}																					

							// calculate the case of the element
							int ncase = 0;
							for (int k = 0; k < 8; ++k)
								if (norm*x[k] >= ref) ncase |= (1 << k);

							// loop over faces
							int* pf = LUT[ncase];
							int ne = 0;
							for (int l = 0; l < 5; l++)
							{
								if (*pf == -1) break;

								// calculate nodal positions
								vec3d r[3];
								float tex[3], w1, w2, w;
								for (int k = 0; k < 3; k++)
								{
									int n1 = ET_HEX[pf[k]][0];
									int n2 = ET_HEX[pf[k]][1];

									w1 = norm * x[n1];
									w2 = norm * x[n2];

									if (w2 != w1)
										w = (ref - w1) / (w2 - w1);
									else
										w = 0.f;

									float f = v[n1] * (1 - w) + v[n2] * w;

									r[k] = x[n1] * (1 - w) + x[n2] * w;
									tex[k] = f;
								}

								GLSlice::FACE face;
								face.mat = n;
								face.r[0] = r[0];
								face.r[1] = r[1];
								face.r[2] = r[2];
								face.tex[0] = tex[0];
								face.tex[1] = tex[1];
								face.tex[2] = tex[2];
								face.bactive = el.IsActive();

								m_slice.AddFace(face);

								pf += 3;
							}
						}
					}
				}
			}
		}
	}
}

void CGLPlaneCutPlot::AddFaces(FEPostMesh* pm)
{
	float ev[8];
	vec3d ex[8];
	EDGE edge[15];
	int en[8];
	CGLModel* mdl = GetModel();
	FEPostModel* ps = mdl->GetFSModel();
	Post::FEState& state = *ps->CurrentState();

	// get the plane equations
	GLdouble a[4];
	GetNormalizedEquations(a);

	// set the plane normal
	vec3d norm((float)a[0], (float)a[1], (float)a[2]);

	double ref = -a[3];
	// loop over faces to determine edges
	for (int i = 0; i < pm->Faces(); ++i)
	{
		FSFace& face = pm->Face(i);

		int elemId = face.m_elem[0].eid;
		FSElement& el = pm->Element(elemId);
		int pid = el.m_MatID;
		if (pid >= 0)
		{
			MeshDomain& dom = pm->Domain(pid);
			int matId = dom.GetMatID();
			if ((matId >= 0) && (matId < ps->Materials()))
			{
				Material* pmat = ps->GetMaterial(matId);
				if ((pmat->bvisible || m_bcut_hidden) && pmat->bclip)
				{
					const int *nt = nullptr;
					switch (face.Type())
					{
					case FE_FACE_TRI3 : nt = TRI_NT; break;
					case FE_FACE_TRI6 : nt = TRI_NT; break;
					case FE_FACE_TRI7 : nt = TRI_NT; break;
					case FE_FACE_TRI10: nt = TRI_NT; break;
					case FE_FACE_QUAD4: nt = QUAD_NT; break;
					case FE_FACE_QUAD8: nt = QUAD_NT; break;
					case FE_FACE_QUAD9: nt = QUAD_NT; break;
					}

					// get the nodal values
					for (int k = 0; k<4; ++k)
					{
						FSNode& node = pm->Node(face.n[nt[k]]);
						ex[k] = node.r;
						en[k] = el.m_node[nt[k]];
						ev[k] = state.m_NODE[el.m_node[nt[k]]].m_val;
					}

					// calculate the case of the face
					int ncase = 0;
					for (int k = 0; k<4; ++k)
						if (norm*ex[k] >= ref) ncase |= (1 << k);

					// loop over faces
					int* pf = LUT2D[ncase];
					int ne = 0;
					for (int l = 0; l < 2; l++)
					{
						if (*pf == -1) break;

						// calculate nodal positions
						vec3d r[2];
						float w1, w2, w;
						for (int k = 0; k<2; k++)
						{
							int n1 = ET2D[pf[k]][0];
							int n2 = ET2D[pf[k]][1];

							w1 = norm*ex[n1];
							w2 = norm*ex[n2];

							if (w2 != w1)
								w = (ref - w1) / (w2 - w1);
							else
								w = 0.f;

							float v = ev[n1] * (1 - w) + ev[n2] * w;

							r[k] = ex[n1] * (1 - w) + ex[n2] * w;
						}

						// add the edge
						GLSlice::EDGE e;
						e.r[0] = r[0];
						e.r[1] = r[1];
						m_slice.AddEdge(e);

						pf += 2;
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Calculate the integral over the plane cut
float CGLPlaneCutPlot::Integrate(FEState* ps)
{
	int k, l;

	CGLModel* mdl = GetModel();

	FEPostModel* pfem = mdl->GetFSModel();
	FEPostMesh* pm = mdl->GetActiveMesh();

	float ev[8];
	vec3d ex[8];

	vec3d r[4];
	float v[4];

	// Integral
	float sum = 0.f;

	// calculate plane normal
	double a[4];
	GetNormalizedEquations(a);
	vec3d norm = GetPlaneNormal();

	double ref = -a[3];

	// repeat over all elements
	for (int i=0; i<pm->Elements(); ++i)
	{
		// consider only solid elements that are visible
		FEElement_& el = pm->ElementRef(i);
		Material* pmat = pfem->GetMaterial(el.m_MatID);
		if (el.IsSolid() && el.IsVisible() && pmat->bvisible)
		{
			// we consider all elements degenerate hexes
			// so get the equivalent hex' node numbering
			const int* nt = nullptr;
			switch (el.Type())
			{
			case FE_HEX8   : nt = HEX_NT; break;
			case FE_HEX20  : nt = HEX_NT; break;
			case FE_HEX27  : nt = HEX_NT; break;
			case FE_PENTA6 : nt = PEN_NT; break;
            case FE_PENTA15: nt = PEN_NT; break;
            case FE_TET4   : nt = TET_NT; break;
            case FE_TET5   : nt = TET_NT; break;
			case FE_TET10  : nt = TET_NT; break;
			case FE_TET15  : nt = TET_NT; break;
			case FE_TET20  : nt = TET_NT; break;
            case FE_PYRA5  : nt = PYR_NT; break;
            case FE_PYRA13 : nt = PYR_NT; break;
			}

			// get the nodal values
			for (k=0; k<8; ++k)
			{
				int m = el.m_node[nt[k]];
				FSNode& node = pm->Node(m);
				ev[k] = ps->m_NODE[m].m_val;
				ex[k] = to_vec3d(ps->m_NODE[m].m_rt);
			}

			// calculate the case of the element
			int ncase = 0;
			for (k=0; k<8; ++k) 
			if (norm*ex[k] >= ref) ncase |= (1 << k);

			// loop over faces
			int* pf = LUT[ncase];
			for (l=0; l<5; l++)
			{
				if (*pf == -1) break;

				// calculate nodal positions
				for (k=0; k<3; k++)
				{
					int n1 = ET_HEX[pf[k]][0];
					int n2 = ET_HEX[pf[k]][1];

					float w1 = norm*ex[n1];
					float w2 = norm*ex[n2];

					float w = 0.f; 
					if (w2 != w1) w = (ref - w1)/(w2 - w1);

					r[k] = ex[n1]*(1-w) + ex[n2]*w;
					v[k] = ev[n1]*(1-w) + ev[n2]*w;
				}

				// the integration requires a quad
				r[3] = r[2];
				v[3] = v[2];

				// integrate
				sum += IntegrateQuad(r, v);

				// next face
				pf+=3;
			}
		}
	}

	return sum;
}

//-----------------------------------------------------------------------------
// Render the cutting plane
void CGLPlaneCutPlot::RenderPlane()
{
	vec3d norm0 = m_normal;
	norm0.Normalize();

	GLdouble a[4];
	GetNormalizedEquations(a);
	vec3d norm(a[0], a[1], a[2]);

	// calculate reference value
	vec3d p0 = m_T.GetPosition();

	glPushMatrix();

	glTranslatef(-p0.x, -p0.y, -p0.z);

	quatd q = quatd(vec3d(0,0,1), norm);
	float w = q.GetAngle();
	if (w != 0)
	{
		vec3d v = q.GetVector();
		glRotatef(w*180/PI, v.x, v.y, v.z);
	}
	else if (vec3d(0, 0, 1) * norm <= -0.99999)
	{
		glRotatef(180, 1, 0, 0);
	}

	glRotatef(m_rot, 0, 0, 1);

	glTranslatef(0.f, 0.f, (float)m_scl*m_offset);

	float R = m_scl;

	// store attributes
	glPushAttrib(GL_ENABLE_BIT);

	GLdouble r = fabs(norm0.x);
	GLdouble g = fabs(norm0.y);
	GLdouble b = fabs(norm0.z);

	glColor4d(r, g, b, m_transparency);
	glDepthMask(false);
	glNormal3f(0,0,1);
	glBegin(GL_QUADS);
	{
		glVertex3f(-R, -R, 0);
		glVertex3f( R, -R, 0);
		glVertex3f( R,  R, 0);
		glVertex3f(-R,  R, 0);
	}
	glEnd();
	glDepthMask(true);

	glColor3ub(255, 255, 0);
	glDisable(GL_LIGHTING);
	glBegin(GL_LINE_LOOP);
	{
		glVertex3f(-R, -R, 0);
		glVertex3f( R, -R, 0);
		glVertex3f( R,  R, 0);
		glVertex3f(-R,  R, 0);
	}
	glEnd();

	glPopMatrix();

	// restore attributes
	glPopAttrib();
}

void CGLPlaneCutPlot::Activate(bool bact)
{
	if (bact != IsActive()) CGLObject::Activate(bact);
		
	if (bact)
	{
		m_nclip = GetFreePlane();
		if (m_nclip >= 0) m_pcp[m_nclip] = this;
	}
	else
	{
		ReleasePlane();
	}
}

int CGLPlaneCutPlot::GetFreePlane()
{
	// NOTE: This assumes that InitClipPlanes() has already been called
	int n = -1;
	if (m_clip.size() > 0)
	{
		// find an available clipping plane
		for (int i=0; i<(int) m_clip.size(); ++i)
		{
			if (m_clip[i] == 0)
			{
				n = i;
				m_clip[i] = 1;
				break;
			}
		}
	}

	assert(n >= 0);

	return n;
}

void CGLPlaneCutPlot::ReleasePlane()
{
	if ((m_clip.size() > 0) && (m_nclip != -1)) 
	{
		m_clip[m_nclip] = 0;
		m_pcp[m_nclip] = 0;
	}
	m_nclip = -1;
}

int CGLPlaneCutPlot::ClipPlanes()
{
	return (int)m_pcp.size();
}

CGLPlaneCutPlot* CGLPlaneCutPlot::GetClipPlane(int i)
{
	return m_pcp[i];
}

bool CGLPlaneCutPlot::IsInsideClipRegion(const vec3d& r)
{
	int N = CGLPlaneCutPlot::ClipPlanes();
	for (int i = 0; i<N; ++i)
	{
		CGLPlaneCutPlot* pcp = CGLPlaneCutPlot::GetClipPlane(i);
		if (pcp && pcp->IsActive())
		{
			double a[4];
			pcp->GetNormalizedEquations(a);

			double q = a[0] * r.x + a[1] * r.y + a[2] * r.z - a[3];
			if (q < 0) return false;
		}
	}
	return true;
}

class PlaneCutPlotSelection : public FESelection
{
public:	
	PlaneCutPlotSelection(CGLPlaneCutPlot* pc) : FESelection(SELECT_OBJECTS), m_pc(pc) { Update(); }

public:
	void Invert() {}
	void Translate(vec3d dr)
	{
		quatd Q = GetOrientation();
		Q.Inverse().RotateVector(dr);
		double dL = dr.z;
		float a = m_pc->GetPlaneOffset();
		float s = m_pc->GetOffsetScale();
		a += dL / s;
		a = (a > 1.f ? 1.f : (a < -1.f ? -1.f : a));
		m_pc->SetPlaneOffset(a);
		Update();
	}

	void Rotate(quatd q, vec3d c) 
	{
		vec3d N = m_pc->GetPlaneNormal();
		quatd Q = GetOrientation();
		q = Q.Inverse() * q * Q;
		q.RotateVector(N);
		m_pc->SetPlaneNormal(N);
		Update();
	}
	void Scale(double s, vec3d dr, vec3d c) {}

	quatd GetOrientation() 
	{
		vec3d N = m_pc->GetPlaneNormal();
		return quatd(vec3d(0,0,1), N); 
	}

	FEItemListBuilder* CreateItemList() { return nullptr; }

	void Update() 
	{
		vec3d r = m_pc->GetPlanePosition();
		m_box = BOX(r, r);
		m_box.Inflate(m_pc->GetOffsetScale());

		m_pc->UpdatePlaneCut();
	}

	int Count() { return 1; }

private:
	CGLPlaneCutPlot* m_pc;
};

bool CGLPlaneCutPlot::Intersects(Ray& ray, Intersection& q)
{
	return true;
}

FESelection* CGLPlaneCutPlot::SelectComponent(int index)
{
	return new PlaneCutPlotSelection(this);
}

void CGLPlaneCutPlot::ClearSelection()
{

}
