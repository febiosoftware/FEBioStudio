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
using namespace Post;

extern int LUT[256][15];
extern int ET_HEX[12][2];

CGLSlicePlot::CGLSlicePlot(CGLModel* po) : CGLLegendPlot(po)
{
	static int n = 1;
	char szname[128] = {0};
	sprintf(szname, "Slice.%02d", n++);
	SetName(szname);

	m_norm = vec3f(1,0,0);

	AddIntParam(0, "Data field")->SetEnumNames("@data_scalar");
	AddIntParam(0, "Color map")->SetEnumNames("@color_map");
	AddBoolParam(true, "Allow clipping");
	AddBoolParam(true, "Show legend"   );
	AddIntParam(0, "Slices");
	AddDoubleParam(0, "Slice offset")->SetFloatRange(0.0, 1.0);
	AddIntParam(0, "Range")->SetEnumNames("dynamic\0user\0");
	AddDoubleParam(0, "Range max");
	AddDoubleParam(0, "Range min");
	AddDoubleParam(0, "X-normal" );
	AddDoubleParam(0, "Y-normal" );
	AddDoubleParam(0, "Z-normal" );

	m_nslices = 10;
	m_nfield = 0;
	m_offset = 0.5f;

	m_lastTime = 0;
	m_lastDt = 0.f;

	m_Col.SetDivisions(m_nslices);
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
		// TODO: show legend
		m_nslices = GetIntValue(SLICES);
		m_offset = GetFloatValue(SLICE_OFFSET);
		m_nrange = GetIntValue(RANGE);
		m_fmax = GetFloatValue(RANGE_MAX);
		m_fmin = GetFloatValue(RANGE_MIN);
		m_norm.x = GetFloatValue(NORMAL_X);
		m_norm.y = GetFloatValue(NORMAL_Y);
		m_norm.z = GetFloatValue(NORMAL_Z);
	}
	else
	{
		SetIntValue(DATA_FIELD, m_nfield);
		SetIntValue(COLOR_MAP, m_Col.GetColorMap());
		SetBoolValue(CLIP, AllowClipping());
		SetIntValue(SLICES, m_nslices);
		SetFloatValue(SLICE_OFFSET, m_offset);
		SetIntValue(RANGE, m_nrange);
		SetFloatValue(RANGE_MAX, m_fmax);
		SetFloatValue(RANGE_MIN, m_fmin);
		SetFloatValue(NORMAL_X, m_norm.x);
		SetFloatValue(NORMAL_Y, m_norm.y);
		SetFloatValue(NORMAL_Z, m_norm.z);
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
	if (m_nfield == 0) return;
	m_box = GetModel()->GetFEModel()->GetBoundingBox();

	GLTexture1D& tex = m_Col.GetTexture();

	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_TEXTURE_1D);
	glDisable(GL_LIGHTING);
	tex.MakeCurrent();
	double fmin, fmax;
	vec3d n = m_norm;
	n.Normalize();
	m_box.Range(n, fmin, fmax);
	float Df = fabs(fmax - fmin);
	if (Df != 0.f)
	{
		fmin += 1e-3*Df;
		fmax -= 1e-3*Df;
	}
	glColor3ub(255, 255, 255);
	if (m_nslices == 1)
	{
		float ref = fmin + m_offset*(fmax - fmin);
		RenderSlice(ref);
	}
	else
	{
		float df = m_offset / m_nslices;
		fmin += df;
		fmax -= df;
		for (int i = 0; i < m_nslices; ++i)
		{
			float f = (float)i / (float)(m_nslices - 1);
			float ref = fmin + f*(fmax - fmin);
			RenderSlice(ref);
		}
	}
	glDisable(GL_TEXTURE_1D);
	glPopAttrib();
}

///////////////////////////////////////////////////////////////////////////////

void CGLSlicePlot::RenderSlice(float ref)
{
	int i, k, l;
	int ncase, *pf;
	float w;

	float ev[8];	// element nodal values
	float ex[8];	// element nodal distances
	vec3f er[8];

	const int HEX_NT[8] = {0, 1, 2, 3, 4, 5, 6, 7};
	const int PEN_NT[8] = {0, 1, 2, 2, 3, 4, 5, 5};
	const int TET_NT[8] = {0, 1, 2, 2, 3, 3, 3, 3};
    const int PYR_NT[8] = {0, 1, 2, 3, 4, 4, 4, 4};
	const int* nt;

	// get the mesh
	CGLModel* mdl = GetModel();
	FEPostModel* ps = mdl->GetFEModel();
	FEPostMesh* pm = mdl->GetActiveMesh();

	vec3f norm = m_norm;
	norm.Normalize();

	vec2f rng = m_crng;
	if (rng.x == rng.y) rng.y++;
	float f;

	// loop over all elements
	for (i=0; i<pm->Elements(); ++i)
	{
		// render only if the element is visible and
		// its material is enabled
		FEElement_& el = pm->ElementRef(i);
		FEMaterial* pmat = ps->GetMaterial(el.m_MatID);
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
			default:
				assert(false);
				return;
			}

			// get the nodal values
			for (k=0; k<8; ++k)
			{
				FENode& node = pm->Node(el.m_node[nt[k]]);

				f = m_val[el.m_node[nt[k]]];
				f = (f - rng.x) / (rng.y - rng.x);

				ev[k] = f;
				er[k] = to_vec3f(node.r);
				ex[k] = node.r*norm;
			}

			// calculate the case of the element
			ncase = 0;
			for (k=0; k<8; ++k) 
				if (ex[k] <= ref) ncase |= (1 << k);

			// loop over faces
			pf = LUT[ncase];
			for (l=0; l<5; l++)
			{
				if (*pf == -1) break;

				// calculate nodal positions
				vec3f r[3];
				float tex[3];
				for (k=0; k<3; k++)
				{
					int n1 = ET_HEX[pf[k]][0];
					int n2 = ET_HEX[pf[k]][1];

					if (ex[n2] != ex[n1])
						w = (ref - ex[n1]) / (ex[n2] - ex[n1]);
					else w = 0.5;

					r[k] = er[n1]*(1-w) + er[n2]*w;
					tex[k] = ev[n1]*(1-w) + ev[n2]*w;
				}

				glNormal3f(0,0,-1);

				// render the face
				glBegin(GL_TRIANGLES);
				{
					glTexCoord1f(tex[0]); glVertex3f(r[0].x, r[0].y, r[0].z);
					glTexCoord1f(tex[1]); glVertex3f(r[1].x, r[1].y, r[1].z);
					glTexCoord1f(tex[2]); glVertex3f(r[2].x, r[2].y, r[2].z);
				}
				glEnd();

				pf+=3;
			}
		}
	}	
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
void CGLSlicePlot::Update(int ntime, float dt, bool breset)
{
	CGLModel* mdl = GetModel();

	FEMeshBase* pm = mdl->GetActiveMesh();
	FEPostModel* pfem = mdl->GetFEModel();

	int NN = pm->Nodes();
	int NS = pfem->GetStates();

	if (breset) { m_map.Clear(); m_rng.clear(); m_val.clear(); }

	if (m_map.States() != pfem->GetStates())
	{
		m_map.Create(NS, NN, 0.f, -1);
		m_rng.resize(NS);
		m_val.resize(NN);
	}
	if (m_nfield == 0) return;

	// see if we need to update this state
	if (breset ||(m_map.GetTag(ntime) != m_nfield))
	{
		m_map.SetTag(ntime, m_nfield);
		vector<float>& val = m_map.State(ntime);

		NODEDATA nd;
		for (int i=0; i<NN; ++i)
		{
			pfem->EvaluateNode(i, ntime, m_nfield, nd);
			val[i] = nd.m_val;
		}

		// evaluate the range
		float fmin, fmax;
		fmin = fmax = val[0];
		for (int i=0;i<NN; ++i)
		{
			if (val[i] < fmin) fmin = val[i];
			if (val[i] > fmax) fmax = val[i];
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
}
