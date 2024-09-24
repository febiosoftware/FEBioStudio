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
#include "GLIsoSurfacePlot.h"
#include "GLWLib/GLWidgetManager.h"
#include "PostLib/constants.h"
#include <GLLib/GLContext.h>
#include <GLLib/GLCamera.h>
#include "GLModel.h"
#include <FSCore/ClassDescriptor.h>
using namespace Post;

extern int LUT[256][15];
extern int ET_HEX[12][2];

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

REGISTER_CLASS(CGLIsoSurfacePlot, CLASS_PLOT, "iso-surface", 0);

CGLIsoSurfacePlot::CGLIsoSurfacePlot()
{
	SetTypeString("iso-surface");

	static int n = 1;
	char szname[128] = { 0 };
	sprintf(szname, "Isosurface.%02d", n++);
	SetName(szname);

	AddIntParam(0, "data_field")->SetEnumNames("@data_scalar");
	AddIntParam(0, "color_map")->SetEnumNames("@color_map");
	AddDoubleParam(1.0, "transparency")->SetFloatRange(0.0, 1.0, 0.01);
	AddBoolParam(true, "allow_clipping");
	AddBoolParam(true, "slice_hidden");
	AddIntParam(1, "slices");
	AddBoolParam(true, "show_Legend");
	AddBoolParam(true, "smooth");
	AddIntParam(0, "range_Type")->SetEnumNames("dynamic\0static\0user\0");
	AddDoubleParam(1.0, "user_range_max");
	AddDoubleParam(0.0, "user_range_min");

	m_nslices = 5;
	m_bsmooth = true;
	m_bcut_hidden = false;
	m_nfield = -1;
	m_transparency = 1.0;

	m_rangeType = RNG_DYNAMIC;
	m_rngMin = 0.;
	m_rngMax = 1.;
	m_userMin = 0.;
	m_userMax = 1.;

	m_Col.SetDivisions(m_nslices);
	m_Col.SetSmooth(false);

	GLLegendBar* bar = new GLLegendBar(&m_Col, 0, 0, 600, 100, GLLegendBar::ORIENT_HORIZONTAL);
	bar->align(GLW_ALIGN_BOTTOM | GLW_ALIGN_HCENTER);
	bar->SetType(GLLegendBar::DISCRETE);
	bar->copy_label(szname);
	bar->ShowTitle(true);
	SetLegendBar(bar);

	UpdateData(false);
}

int CGLIsoSurfacePlot::GetSlices() 
{ 
	return m_nslices; 
}

void CGLIsoSurfacePlot::SetSlices(int nslices)
{ 
	m_nslices = nslices; 
	m_Col.SetDivisions(nslices); 
}

bool CGLIsoSurfacePlot::UpdateData(bool bsave)
{
	GLLegendBar* bar = GetLegendBar();

	if (bsave)
	{
		bool update = false;
		if (m_nfield    != GetIntValue  (DATA_FIELD)) { m_nfield    = GetIntValue  (DATA_FIELD); update = true; }
		if (m_nslices   != GetIntValue  (SLICES    )) { m_nslices   = GetIntValue  (SLICES    ); update = true; }
		if (m_bsmooth   != GetBoolValue (SMOOTH    )) { m_bsmooth   = GetBoolValue (SMOOTH    ); update = true; }
		if (m_rangeType != GetIntValue  (RANGE_TYPE)) { m_rangeType = GetIntValue  (RANGE_TYPE); update = true; }
		if (m_userMax   != GetFloatValue(USER_MAX  )) { m_userMax   = GetFloatValue(USER_MAX  ); update = true; }
		if (m_userMin   != GetFloatValue(USER_MIN  )) { m_userMin   = GetFloatValue(USER_MIN  ); update = true; }
		if (m_Col.GetColorMap() != GetIntValue(COLOR_MAP)) { m_Col.SetColorMap(GetIntValue(COLOR_MAP)); update = true; }

		AllowClipping(GetBoolValue(CLIP));
		m_bcut_hidden = GetBoolValue(HIDDEN);
		if (GetBoolValue(LEGEND)) bar->show(); else bar->hide();

		m_Col.SetDivisions(m_nslices);

		if (update) Update();

		if (m_transparency != GetFloatValue(TRANSPARENCY))
		{
			// set the mesh transparency
			m_transparency = GetFloatValue(TRANSPARENCY);
			m_glmesh.SetTransparency((ubyte)(255.0 * m_transparency));
		}
	}
	else
	{
		SetIntValue(DATA_FIELD, m_nfield);
		SetFloatValue(TRANSPARENCY, m_transparency);
		SetIntValue(COLOR_MAP, m_Col.GetColorMap());
		SetBoolValue(CLIP, AllowClipping());
		SetBoolValue(HIDDEN, m_bcut_hidden);
		SetIntValue(SLICES, m_nslices);
		SetBoolValue(LEGEND, bar->visible());
		SetBoolValue(SMOOTH, m_bsmooth);
		SetIntValue(RANGE_TYPE, m_rangeType);
		SetFloatValue(USER_MAX, m_userMax);
		SetFloatValue(USER_MIN, m_userMin);
	}

	return false;
}

void CGLIsoSurfacePlot::Render(CGLContext& rc)
{
	if (m_nfield == 0) return;

	glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT);
	{
		glColor4ub(255,255,255,255);
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		glDisable(GL_TEXTURE_1D);
		GLfloat zero[4] = {0.f};
		GLfloat one[4] = {1.f, 1.f, 1.f, 1.f};
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, zero);
		glLightfv(GL_LIGHT0, GL_SPECULAR, zero);
	//	glLightfv(GL_LIGHT0, GL_AMBIENT, one);
	//	glLightfv(GL_LIGHT0, GL_DIFFUSE, one);

		// z-sorting if necessary
		if (m_transparency < 0.99) m_glmesh.ZSortFaces(*rc.m_cam);

		// render the mesh
		m_glmesh.Render();
	}
	glPopAttrib();
}

void CGLIsoSurfacePlot::UpdateMesh()
{
	if (m_nslices <= 0) return;

	float vmin = m_crng.x;
	float vmax = m_crng.y;
	float D = vmax - vmin;

	// build a GMesh
	GMesh mesh;
	for (int i = 0; i < m_nslices; ++i)
	{
		float ref = vmin + ((float)i + 0.5f)* D / (m_nslices);

		float w = (ref - vmin) / D;

		CColorMap& map = m_Col.ColorMap();
		GLColor col = map.map(w);
		UpdateSlice(mesh, ref, col);
	}

	// create a GLVAMesh
	m_glmesh.CreateFromGMesh(mesh);

	// set the mesh transparency
	m_glmesh.SetTransparency((ubyte)(255.0 * m_transparency));
}

///////////////////////////////////////////////////////////////////////////////

void CGLIsoSurfacePlot::UpdateSlice(GMesh& mesh, float ref, GLColor col)
{
	float ev[8];	// element nodal values
	vec3f ex[8];	// element nodal positions
	vec3f en[8];	// element nodal gradients

	const int HEX_NT[8] = {0, 1, 2, 3, 4, 5, 6, 7};
	const int PEN_NT[8] = {0, 1, 2, 2, 3, 4, 5, 5};
	const int TET_NT[8] = {0, 1, 2, 2, 3, 3, 3, 3};
    const int PYR_NT[8] = {0, 1, 2, 3, 4, 4, 4, 4};

	CGLModel* mdl = GetModel();
	FEPostModel* ps = mdl->GetFSModel();

	// get the mesh
	FEPostMesh* pm = mdl->GetActiveMesh();

	const int* nt = nullptr;

	// loop over all elements
	for (int i=0; i<pm->Elements(); ++i)
	{
		// render only if the element is visible and
		// its material is enabled
		FEElement_& el = pm->ElementRef(i);
		Material* pmat = ps->GetMaterial(el.m_MatID);
		if (pmat->benable && (el.IsVisible() || m_bcut_hidden) && el.IsSolid())
		{
			switch (el.Type())
			{
			case FE_HEX8   : nt = HEX_NT; break;
			case FE_HEX20  : nt = HEX_NT; break;
			case FE_HEX27  : nt = HEX_NT; break;
			case FE_PENTA6 : nt = PEN_NT; break;
			case FE_PENTA15: nt = PEN_NT; break;
			case FE_TET4   : nt = TET_NT; break;
			case FE_TET5   : nt = TET_NT; break;
			case FE_PYRA5  : nt = PYR_NT; break;
			case FE_PYRA13 : nt = PYR_NT; break;
			case FE_TET10  : nt = TET_NT; break;
			case FE_TET15  : nt = TET_NT; break;
			default:
				assert(false);
			}

			// get the nodal values
			for (int k=0; k<8; ++k)
			{
				FSNode& node = pm->Node(el.m_node[nt[k]]);

				ev[k] = m_val[el.m_node[nt[k]]];
				ex[k] = to_vec3f(node.r);
				en[k] = m_grd[el.m_node[nt[k]]];
			}

			// calculate the case of the element
			int ncase = 0;
			for (int k=0; k<8; ++k) 
				if (ev[k] <= ref) ncase |= (1 << k);

			// loop over faces
			int* pf = LUT[ncase];
			for (int l=0; l<5; l++)
			{
				if (*pf == -1) break;

				// calculate nodal positions
				vec3f r[3], vn[3];
				for (int k=0; k<3; k++)
				{
					int n1 = ET_HEX[pf[k]][0];
					int n2 = ET_HEX[pf[k]][1];

					float w = (ref - ev[n1]) / (ev[n2] - ev[n1]);

					r[k] = ex[n1]*(1-w) + ex[n2]*w;
				}

				// calculate normals
				if (m_bsmooth)
				{
					for (int k=0; k<3; k++)
					{
						int n1 = ET_HEX[pf[k]][0];
						int n2 = ET_HEX[pf[k]][1];

						float w = (ref - ev[n1]) / (ev[n2] - ev[n1]);

						vn[k] = en[n1]*(1-w) + en[n2]*w;
						vn[k].Normalize();
					}
				}
				else
				{
					for (int k=0; k<3; k++)
					{
						int kp1 = (k+1)%3;
						int km1 = (k+2)%3;
						vn[k] = (r[kp1] - r[k])^(r[km1] - r[k]);
						vn[k].Normalize();
					}
				}

				// Add the face
				mesh.AddFace(r, vn, col);
				pf+=3;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CGLIsoSurfacePlot::SetEvalField(int n) 
{ 
	m_nfield = n; 
	Update(GetModel()->CurrentTimeIndex(), 0.0, false);
}

//-----------------------------------------------------------------------------
void CGLIsoSurfacePlot::Update()
{
	Update(m_lastTime, m_lastdt, true);
}

//-----------------------------------------------------------------------------
void CGLIsoSurfacePlot::Update(int ntime, float dt, bool breset)
{
	m_lastTime = ntime;
	m_lastdt = dt;
	if (m_nfield == 0) return;

	CGLModel* mdl = GetModel();

	FEPostMesh* pm = mdl->GetActiveMesh();
	FEPostModel* pfem = mdl->GetFSModel();

	int NN = pm->Nodes();
	int NS = pfem->GetStates();

	if (breset) { m_map.Clear(); m_GMap.Clear(); m_rng.clear(); m_val.clear(); m_grd.clear(); }

	if (m_map.States() != pfem->GetStates())
	{
		m_map.Create(NS, NN, 0.f, -1);
		m_GMap.Create(NS, NN, vec3f(0,0,0), -1);
		m_rng.resize(NS);
		m_val.resize(NN);
		m_grd.resize(NN);
	}

	// see if we need to update this state
	if (m_map.GetTag(ntime) != m_nfield)
	{
		m_map.SetTag(ntime, m_nfield);
		vector<float>& val = m_map.State(ntime);

		// only count enabled parts
		pm->TagAllNodes(0);
		for (int i = 0; i < pm->Elements(); ++i)
		{
			FEElement_& el = pm->ElementRef(i);
			Material* pmat = pfem->GetMaterial(el.m_MatID);
			if (pmat->benable && el.IsVisible())
			{
				int ne = el.Nodes();
				for (int k = 0; k < ne; ++k)
				{
					FSNode& node = pm->Node(el.m_node[k]);
					node.m_ntag = 1;
				}
			}
		}

		// evaluate nodal values
		float fmin = 1e34, fmax = -1e34;
		for (int i=0; i<NN; ++i) 
		{
			FSNode& node = pm->Node(i);
			if (node.m_ntag == 1)
			{
				NODEDATA nd;
				pfem->EvaluateNode(i, ntime, m_nfield, nd);
				val[i] = nd.m_val;

				if (val[i] < fmin) fmin = val[i];
				if (val[i] > fmax) fmax = val[i];
			}
			else val[i] = 0.0f;
		}

		m_rng[ntime] = vec2f(fmin, fmax);
	}

	// see if we need to update the gradient
	if (m_bsmooth && (m_GMap.GetTag(ntime) != m_nfield))
	{
		vector<float>& val = m_map.State(ntime);
		m_GMap.SetFEMesh(pm);
		m_GMap.Gradient(ntime, val);
		m_GMap.SetTag(ntime, m_nfield);
	}

	// copy nodal values into current value buffer
	m_val = m_map.State(ntime);
	if (m_bsmooth) m_grd = m_GMap.State(ntime);

	// update colormap range
	vec2f r = m_rng[ntime];

	// update static range
	if (breset) { m_rngMin = r.x; m_rngMax = r.y; }
	else
	{
		if (r.x < m_rngMin) m_rngMin = r.x;
		if (r.y > m_rngMax) m_rngMax = r.y;
	}

	// set range for color map
	switch (m_rangeType)
	{
	case RNG_DYNAMIC:
		m_crng = m_rng[ntime];
		break;
	case RNG_STATIC:
		m_crng = vec2f((float) m_rngMin, (float) m_rngMax);
		break;
	case RNG_USER:
		m_crng = vec2f((float)m_userMin, (float)m_userMax);
		break;
	}
	if (m_crng.x == m_crng.y) m_crng.y++;

	float vmin = m_crng.x;
	float vmax = m_crng.y;
	if (m_nslices > 0)
	{
		double D = vmax - vmin;
		double d = 0.5 * D / m_nslices;
		vmin += d;
		vmax -= d;
	}
	GetLegendBar()->SetRange(vmin, vmax);

	// Update the mesh
	UpdateMesh();
}
