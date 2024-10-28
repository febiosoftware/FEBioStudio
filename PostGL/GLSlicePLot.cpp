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
#include "GLSlicePLot.h"
#include "GLWLib/GLWidgetManager.h"
#include "PostLib/constants.h"
#include "GLModel.h"
#include <GLLib/GLContext.h>
#include <GLLib/glx.h>
#include <FSCore/ClassDescriptor.h>
using namespace Post;

extern int LUT[256][15];
extern int ET_HEX[12][2];

REGISTER_CLASS(CGLSlicePlot, CLASS_PLOT, "slices", 0);

CGLSlicePlot::CGLSlicePlot()
{
	SetTypeString("slices");

	static int n = 1;
	char szname[128] = {0};
	sprintf(szname, "Slice.%02d", n++);
	SetName(szname);

	m_norm = vec3f(1,0,0);

	AddIntParam(0, "data_field")->SetEnumNames("@data_scalar");
	AddIntParam(0, "color_map")->SetEnumNames("@color_map");
	AddIntParam(10, "divs", "Range divisions");
	AddBoolParam(true, "smooth", "Gradient smoothing");
	AddDoubleParam(1.0, "transparency")->SetFloatRange(0,1);
	AddBoolParam(true, "allow_clipping");
	AddBoolParam(true, "show_legend"   );
	AddIntParam(0, "slices");
	AddDoubleParam(0, "slice_offset")->SetFloatRange(0.0, 1.0);
	AddVec2dParam(vec2d(0,1), "slice_range");
	AddVectorDoubleParam(m_user_slices, "user_slices")->SetFixedSize(false);
	AddIntParam(0, "range")->SetEnumNames("dynamic\0user\0");
	AddDoubleParam(0, "range_max");
	AddDoubleParam(0, "range_min");
	AddDoubleParam(0, "x-normal" );
	AddDoubleParam(0, "y-normal" );
	AddDoubleParam(0, "z-normal" );
	AddBoolParam(true, "show_box");

	m_nslices = 10;
	m_nfield = -1;
	m_offset = 0.5f;
	m_slice_range = vec2d(0, 1);

	m_lastTime = 0;
	m_lastDt = 0.f;

	m_Col.SetDivisions(10);
	m_Col.SetSmooth(false);

	m_nrange = 0;
	m_fmin = 0.f;
	m_fmax = 0.f;

	GLLegendBar* bar = new GLLegendBar(&m_Col, 0, 0, 120, 500);
	bar->align(GLW_ALIGN_LEFT| GLW_ALIGN_VCENTER);
	bar->copy_label(szname);
	SetLegendBar(bar);

	UpdateData(false);
}

int CGLSlicePlot::GetSlices() { return m_nslices; }
void CGLSlicePlot::SetSlices(int nslices) { m_nslices = nslices; m_Col.SetDivisions(nslices); }

bool CGLSlicePlot::UpdateData(bool bsave)
{
	if (bsave)
	{
		m_nfield = GetIntValue(DATA_FIELD);
		m_Col.SetColorMap(GetIntValue(COLOR_MAP));
		AllowClipping(GetBoolValue(CLIP));

		int divs = GetIntValue(RANGE_DIVS);
		bool smooth = GetBoolValue(GRAD_SMOOTH);
		m_Col.SetDivisions(divs);
		m_Col.SetSmooth(smooth);
		if (GetLegendBar())
		{
			bool b = GetBoolValue(SHOW_LEGEND);
			if (b) GetLegendBar()->show(); else GetLegendBar()->hide();
		}
		m_nslices = GetIntValue(SLICES);
		m_offset = GetFloatValue(SLICE_OFFSET);
		m_slice_range = GetVec2dValue(SLICE_RANGE);
		m_user_slices = GetParamVectorDouble(USER_SLICES);

		double& x = m_slice_range.x();
		double& y = m_slice_range.y();
		if (x < 0) x = 0;
		if (y > 1) y = 1;
		if (x > y) { double tmp = x; x = y; y = tmp; }

		m_nrange = GetIntValue(RANGE);
		m_fmax = GetFloatValue(RANGE_MAX);
		m_fmin = GetFloatValue(RANGE_MIN);
		m_norm.x = GetFloatValue(NORMAL_X);
		m_norm.y = GetFloatValue(NORMAL_Y);
		m_norm.z = GetFloatValue(NORMAL_Z);

		Update();
	}
	else
	{
		SetIntValue(DATA_FIELD, m_nfield);
		SetIntValue(COLOR_MAP, m_Col.GetColorMap());
		SetIntValue(RANGE_DIVS, m_Col.GetDivisions());
		SetBoolValue(GRAD_SMOOTH, m_Col.GetSmooth());
		SetBoolValue(CLIP, AllowClipping());
		SetIntValue(SLICES, m_nslices);
		SetFloatValue(SLICE_OFFSET, m_offset);
		SetVec2dValue(SLICE_RANGE, m_slice_range);
		SetParamVectorDouble(USER_SLICES, m_user_slices);
		SetIntValue(RANGE, m_nrange);
		SetFloatValue(RANGE_MAX, m_fmax);
		SetFloatValue(RANGE_MIN, m_fmin);
		SetFloatValue(NORMAL_X, m_norm.x);
		SetFloatValue(NORMAL_Y, m_norm.y);
		SetFloatValue(NORMAL_Z, m_norm.z);
		if (GetLegendBar())
		{
			SetBoolValue(SHOW_LEGEND, GetLegendBar()->visible());
		}
	}

	return false;
}

void CGLSlicePlot::SetSliceOffset(float f) 
{ 
	m_offset = f; 
	if (m_offset < 0.f) m_offset = 0.f;
	if (m_offset > 1.f) m_offset = 1.f;
}

void CGLSlicePlot::Render(CGLContext& rc)
{
	if (m_nfield == -1) return;
	if (m_box.IsValid() == false) return;

	bool showBox = GetBoolValue(SHOW_BOX);
	if (showBox)
	{
		glColor3ub(200, 200, 200);
		glx::renderBox(m_box, false, 1.0);
	}

	GLTexture1D& tex = m_Col.GetTexture();

	uint8_t a = uint8_t(255.0*GetFloatValue(TRANSPARENCY));

	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_TEXTURE_1D);
	glDisable(GL_LIGHTING);
	tex.MakeCurrent();

	glColor4ub(255, 255, 255, a);

	if (a < 255) m_mesh.ZSortFaces(*rc.m_cam);

	m_mesh.Render();

	glDisable(GL_TEXTURE_1D);
	glPopAttrib();
}

///////////////////////////////////////////////////////////////////////////////

int CGLSlicePlot::UpdateSlice(float ref, std::vector<std::pair<int, float> >& activeElements)
{
	const int HEX_NT[8] = {0, 1, 2, 3, 4, 5, 6, 7};
	const int PEN_NT[8] = {0, 1, 2, 2, 3, 4, 5, 5};
	const int TET_NT[8] = {0, 1, 2, 2, 3, 3, 3, 3};
    const int PYR_NT[8] = {0, 1, 2, 3, 4, 4, 4, 4};
	const int* nt;

	// get the mesh
	CGLModel* mdl = GetModel();
	FEPostModel* ps = mdl->GetFSModel();
	FEPostMesh* pm = mdl->GetActiveMesh();

	vec3d norm = to_vec3d(m_norm);
	norm.Normalize();

	// loop over all elements
	int faceCount = 0;
	for (int i=0; i<pm->Elements(); ++i)
	{
		// render only if the element is visible and
		// its material is enabled
		FEElement_& el = pm->ElementRef(i);
		Material* pmat = ps->GetMaterial(el.m_MatID);
		if (pmat->benable && el.IsVisible() && el.IsSolid())
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
				continue;
			}

			// get the nodal values
			float ex[8];	// element nodal distances
			for (int k=0; k<8; ++k)
			{
				FSNode& node = pm->Node(el.m_node[nt[k]]);
				ex[k] = node.r*norm;
			}

			// calculate the case of the element
			int ncase = 0;
			for (int k=0; k<8; ++k) 
				if (ex[k] <= ref) ncase |= (1 << k);

			if ((ncase > 0) && (ncase < 255))
			{
				activeElements.push_back(pair<int, double>(i, ref));

				int* pf = LUT[ncase];
				for (int l = 0; l < 5; ++l)
				{
					if (*pf == -1) break;
					faceCount++;
					pf += 3;
				}
			}
		}
	}	
	return faceCount;
}

//-----------------------------------------------------------------------------
void CGLSlicePlot::SetEvalField(int n) 
{ 
	m_nfield = n; 
	Update(GetModel()->CurrentTimeIndex(), 0.0, false);
}
//-----------------------------------------------------------------------------
void CGLSlicePlot::Update()
{
	Update(m_lastTime, m_lastDt, false);
}

//-----------------------------------------------------------------------------
void CGLSlicePlot::UpdateBoundingBox()
{
	CGLModel* mdl = GetModel();
	FEPostModel* ps = mdl->GetFSModel();
	FEPostMesh* pm = mdl->GetActiveMesh();

	// only count enabled parts
	BOX box;
	for (int i = 0; i < pm->Elements(); ++i)
	{
		FEElement_& el = pm->ElementRef(i);
		Material* pmat = ps->GetMaterial(el.m_MatID);
		if (pmat->benable && el.IsVisible())
		{
			int ne = el.Nodes();
			for (int k = 0; k < ne; ++k)
			{
				FSNode& node = pm->Node(el.m_node[k]);
				box += node.pos();
			}
		}
	}

	m_box = box;
}

//-----------------------------------------------------------------------------
void CGLSlicePlot::Update(int ntime, float dt, bool breset)
{
	CGLModel* mdl = GetModel();

	UpdateBoundingBox();

	FEPostMesh* pm = mdl->GetActiveMesh();
	FEPostModel* pfem = mdl->GetFSModel();

	int NN = pm->Nodes();
	int NS = pfem->GetStates();

	m_lastTime = ntime;
	m_lastDt = dt;

	if (breset) { m_map.Clear(); m_rng.clear(); m_val.clear(); }

	if (m_map.States() != pfem->GetStates())
	{
		m_map.Create(NS, NN, 0.f, -1);
		m_rng.resize(NS);
		m_val.resize(NN);
	}
	if (m_nfield == -1) return;

	// see if we need to update this state
	if (breset ||(m_map.GetTag(ntime) != m_nfield))
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

	// copy current nodal values
	m_val = m_map.State(ntime);

	// set range for color map
	switch (m_nrange)
	{
	case 0:
		m_crng = m_rng[ntime];
		break;
	case 1:
		m_crng = vec2f((float)m_fmin, (float)m_fmax);
		break;
	}
	if (m_crng.x == m_crng.y) m_crng.y++;

	GLLegendBar* bar = GetLegendBar();
	bar->SetRange(m_crng.x, m_crng.y);

	// update the mesh
	UpdateMesh();
}

void CGLSlicePlot::UpdateMesh()
{
	// get the mesh
	CGLModel* mdl = GetModel();
	FEPostModel* ps = mdl->GetFSModel();
	FEPostMesh* pm = mdl->GetActiveMesh();

	vector<pair<int, float> > activeElements;
	activeElements.reserve(pm->Faces());
	int faces = CountFaces(activeElements);

	m_mesh.Create(faces, GLMesh::FLAG_TEXTURE);

	float ev[8];	// element nodal values
	float ex[8];	// element nodal distances
	vec3f er[8];

	const int HEX_NT[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	const int PEN_NT[8] = { 0, 1, 2, 2, 3, 4, 5, 5 };
	const int TET_NT[8] = { 0, 1, 2, 2, 3, 3, 3, 3 };
	const int PYR_NT[8] = { 0, 1, 2, 3, 4, 4, 4, 4 };

	vec3d norm = to_vec3d(m_norm);
	norm.Normalize();

	vec2f rng = m_crng;
	if (rng.x == rng.y) rng.y++;
	float f;

	// loop over all elements
	m_mesh.BeginMesh();
	for (int i = 0; i < activeElements.size(); ++i)
	{
		// render only if the element is visible and
		// its material is enabled
		FEElement_& el = pm->ElementRef(activeElements[i].first);

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
		case FE_PYRA5  : nt = PYR_NT; break;
		case FE_PYRA13 : nt = PYR_NT; break;
		case FE_TET10  : nt = TET_NT; break;
		case FE_TET15  : nt = TET_NT; break;
		default:
			assert(false);
			continue;
		}

		// get the reference value for this element
		double ref = activeElements[i].second;

		// get the nodal values
		for (int k = 0; k < 8; ++k)
		{
			FSNode& node = pm->Node(el.m_node[nt[k]]);

			f = m_val[el.m_node[nt[k]]];
			f = (f - rng.x) / (rng.y - rng.x);

			ev[k] = f;
			er[k] = to_vec3f(node.r);
			ex[k] = node.r * norm;
		}

		// calculate the case of the element
		int ncase = 0;
		for (int k = 0; k < 8; ++k)
			if (ex[k] <= ref) ncase |= (1 << k);

		// loop over faces
		int* pf = LUT[ncase];
		for (int l = 0; l < 5; l++)
		{
			if (*pf == -1) break;

			// calculate nodal positions
			vec3f r[3];
			float tex[3];
			for (int k = 0; k < 3; k++)
			{
				int n1 = ET_HEX[pf[k]][0];
				int n2 = ET_HEX[pf[k]][1];

				double w = 0.5;
				if (ex[n2] != ex[n1])
					w = (ref - ex[n1]) / (ex[n2] - ex[n1]);

				r[k] = er[n1] * (1 - w) + er[n2] * w;
				tex[k] = ev[n1] * (1 - w) + ev[n2] * w;
			}

			// add the vertices
			m_mesh.AddVertex(r[0], tex[0]);
			m_mesh.AddVertex(r[1], tex[1]);
			m_mesh.AddVertex(r[2], tex[2]);

			pf += 3;
		}
	}
	m_mesh.EndMesh();
}

int CGLSlicePlot::CountFaces(std::vector<std::pair<int, float> >& activeElements)
{
	vec3d n = to_vec3d(m_norm);
	n.Normalize();
	double fmin, fmax;
	m_box.Range(n, fmin, fmax);
	float Df = fabs(fmax - fmin);
	if (Df != 0.f)
	{
		fmin += 1e-3 * Df;
		fmax -= 1e-3 * Df;
	}

	float frange = fmax - fmin;
	fmax = fmin + frange * m_slice_range.y();
	fmin = fmin + frange * m_slice_range.x();

	int faceCount = 0;
	if (m_nslices == 1)
	{
		float ref = fmin + m_offset * (fmax - fmin);
		faceCount += UpdateSlice(ref, activeElements);
	}
	else if (m_nslices > 0)
	{
		float df = m_offset / m_nslices;
		fmin += df;
		fmax -= df;
		for (int i = 0; i < m_nslices; ++i)
		{
			float f = (float)i / (float)(m_nslices - 1);
			float ref = fmin + f * (fmax - fmin);
			faceCount += UpdateSlice(ref, activeElements);
		}
	}

	for (int i = 0; i < m_user_slices.size(); ++i)
	{
		double f = m_user_slices[i];
		if (f < 0) f = 0;
		if (f > 1) f = 1;
		float ref = fmin + f * (fmax - fmin);
		faceCount += UpdateSlice(ref, activeElements);
	}

	return faceCount;
}
