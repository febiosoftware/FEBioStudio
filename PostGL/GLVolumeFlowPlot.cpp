/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
#include "GLVolumeFlowPlot.h"
#include "GLModel.h"
#include <GLLib/GLContext.h>
using namespace Post;

extern int LUT[256][15];
extern int ET_HEX[12][2];

GLVolumeFlowPlot::GLVolumeFlowPlot(CGLModel* mdl) : CGLPlot(mdl)
{
	static int n = 1;
	char szname[128] = { 0 };
	sprintf(szname, "VolumeFlow%d", n++);
	SetName(szname);

	AddIntParam(0, "Data field")->SetEnumNames("@data_scalar");
	AddIntParam(0, "Color map")->SetEnumNames("@color_map");
	AddDoubleParam(0.2, "Opacity scale");

	m_alpha = 0.2f;
	m_nfield = 0;
	m_nrange = 0;
	m_fmin = 0.f;
	m_fmax = 0.f;

	UpdateData(false);
}

bool GLVolumeFlowPlot::UpdateData(bool bsave)
{
	if (bsave)
	{
		m_nfield = GetIntValue(0);
		m_Col.SetColorMap(GetIntValue(1));
		m_alpha = GetFloatValue(2);
		Update(GetModel()->CurrentTimeIndex(), 0.f, true);
	}
	else
	{
		SetIntValue(0, m_nfield);
		SetIntValue(1, m_Col.GetColorMap());
		SetFloatValue(2, m_alpha);
	}

	return false;
}

void GLVolumeFlowPlot::Update(int ntime, float dt, bool breset)
{
	UpdateNodalData(ntime, breset);

	// get the model
	CGLModel& mdl = *GetModel();

	FEPostModel* fem = mdl.GetFEModel();
	FEState* state = fem->CurrentState();

	// get the current mesh
	FEMeshBase& mesh = *mdl.GetActiveMesh();

	// get the largest dimension
	BOX box = m_box;
	double D = box.GetMaxExtent();

	// this is the resolution for the largest dimension
	const int MAX_RES = 128;
	const int MIN_RES = 8;

	// set the resolutions based on the box dimensions
	int nx = (int)(box.Width () / D * MAX_RES); if (nx < MIN_RES) nx = MIN_RES;
	int ny = (int)(box.Height() / D * MAX_RES); if (ny < MIN_RES) ny = MIN_RES;
	int nz = (int)(box.Depth () / D * MAX_RES); if (nz < MIN_RES) nz = MIN_RES;

	// allocate the slices
	m_slice_X.resize(nx);
	m_slice_Y.resize(ny);
	m_slice_Z.resize(nz);

	// build the 
#pragma omp parallel default(shared)
	{
#pragma omp for
		for (int i = 0; i < nx; ++i)
		{
			float x = box.x0 + ((float)i)*(box.x1 - box.x0) / (nx - 1.f);
			Slice& slice = m_slice_X[i];
			CreateSlice(slice, vec3d(1, 0, 0), x);
		}

#pragma omp for
		for (int i = 0; i < ny; ++i)
		{
			float y = box.y0 + ((float)i)*(box.y1 - box.y0) / (ny - 1.f);
			Slice& slice = m_slice_Y[i];
			CreateSlice(slice, vec3d(0, 1, 0), y);
		}

#pragma omp for
		for (int i = 0; i < nz; ++i)
		{
			float z = box.z0 + ((float)i)*(box.z1 - box.z0) / (nz - 1.f);
			Slice& slice = m_slice_Z[i];
			CreateSlice(slice, vec3d(0, 0, 1), z);
		}
	}
}

void GLVolumeFlowPlot::UpdateNodalData(int ntime, bool breset)
{
	CGLModel* mdl = GetModel();

	FEMeshBase* pm = mdl->GetActiveMesh();
	FEPostModel* pfem = mdl->GetFEModel();

	int NN = pm->Nodes();
	int NS = pfem->GetStates();

	// update the box
	BOX b(pm->Node(0).r, pm->Node(0).r);
	for (int i = 0; i < NN; ++i)
	{
		vec3d& ri = pm->Node(i).r;
		if (ri.x < b.x0) b.x0 = ri.x; if (ri.x > b.x1) b.x1 = ri.x;
		if (ri.y < b.y0) b.y0 = ri.y; if (ri.y > b.y1) b.y1 = ri.y;
		if (ri.z < b.z0) b.z0 = ri.z; if (ri.z > b.z1) b.z1 = ri.z;
	}
	m_box = b;

	if (breset) { m_map.Clear(); m_rng.clear(); m_val.clear(); }

	if (m_map.States() != pfem->GetStates())
	{
		m_map.Create(NS, NN, 0.f, -1);
		m_rng.resize(NS);
		m_val.resize(NN);
	}
	if (m_nfield == 0) return;

	// see if we need to update this state
	if (breset || (m_map.GetTag(ntime) != m_nfield))
	{
		m_map.SetTag(ntime, m_nfield);
		vector<float>& val = m_map.State(ntime);

		NODEDATA nd;
		for (int i = 0; i<NN; ++i)
		{
			pfem->EvaluateNode(i, ntime, m_nfield, nd);
			val[i] = nd.m_val;
		}

		// evaluate the range
		float fmin, fmax;
		if (m_nrange == 0)
		{
			fmin = fmax = val[0];
			for (int i = 0; i<NN; ++i)
			{
				if (val[i] < fmin) fmin = val[i];
				if (val[i] > fmax) fmax = val[i];
			}
			if (fmin == fmax) fmax++;

			m_fmin = fmin;
			m_fmax = fmax;
		}
		else
		{
			fmin = m_fmin;
			fmax = m_fmax;
			if (fmin == fmax) fmax++;
		}

		m_rng[ntime] = vec2f(fmin, fmax);
	}

	// copy current nodal values
	m_val = m_map.State(ntime);

	// update range
	m_crng = m_rng[ntime];
}

void GLVolumeFlowPlot::CreateSlice(Slice& slice, const vec3d& norm, float ref)
{
	slice.clear();
	slice.reserve(100);

	float ev[8];	// element nodal values
	float ex[8];	// element nodal distances
	vec3d er[8];

	const int HEX_NT[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	const int PEN_NT[8] = { 0, 1, 2, 2, 3, 4, 5, 5 };
	const int TET_NT[8] = { 0, 1, 2, 2, 3, 3, 3, 3 };
    const int PYR_NT[8] = {0, 1, 2, 3, 4, 4, 4, 4};
	const int* nt;

	// get the mesh
	CGLModel* mdl = GetModel();
	FEPostModel* ps = mdl->GetFEModel();
	FEPostMesh* pm = mdl->GetActiveMesh();

	vec2f rng = m_crng;
	if (rng.x == rng.y) rng.y++;

	// loop over all elements
	for (int i = 0; i<pm->Elements(); ++i)
	{
		// render only if the element is visible and
		// its material is enabled
		FEElement_& el = pm->ElementRef(i);
		FEMaterial* pmat = ps->GetMaterial(el.m_MatID);
		if (pmat->benable && el.IsVisible() && el.IsSolid())
		{
			switch (el.Type())
			{
			case FE_HEX8: nt = HEX_NT; break;
			case FE_HEX20: nt = HEX_NT; break;
			case FE_HEX27: nt = HEX_NT; break;
			case FE_PENTA6: nt = PEN_NT; break;
			case FE_PENTA15: nt = PEN_NT; break;
			case FE_TET4: nt = TET_NT; break;
			case FE_TET5: nt = TET_NT; break;
            case FE_PYRA5: nt = PYR_NT; break;
            case FE_PYRA13: nt = PYR_NT; break;
			default:
				assert(false);
				return;
			}

			// get the nodal values
			for (int k = 0; k<8; ++k)
			{
				FENode& node = pm->Node(el.m_node[nt[k]]);

				float f = m_val[el.m_node[nt[k]]];
				f = (f - rng.x) / (rng.y - rng.x);

				ev[k] = f;
				er[k] = node.r;
				ex[k] = node.r*norm;
			}

			// calculate the case of the element
			int ncase = 0;
			for (int k = 0; k<8; ++k)
				if (ex[k] <= ref) ncase |= (1 << k);

			// loop over faces
			int* pf = LUT[ncase];
			for (int l = 0; l<5; l++)
			{
				if (*pf == -1) break;

				// calculate nodal positions
				vec3d r[3];
				double v[3];
				for (int k = 0; k<3; k++)
				{
					int n1 = ET_HEX[pf[k]][0];
					int n2 = ET_HEX[pf[k]][1];

					double w = 0.5;
					if (ex[n2] != ex[n1])
						w = (ref - ex[n1]) / (ex[n2] - ex[n1]);
					
					r[k] = er[n1] * (1 - w) + er[n2] * w;
					v[k] = ev[n1] * (1 - w) + ev[n2] * w;
				}

				// render the face
				Slice::Face face;
				face.v[0] = v[0]; face.r[0] = r[0];
				face.v[1] = v[1]; face.r[1] = r[1];
				face.v[2] = v[2]; face.r[2] = r[2];

				slice.add(face);

				pf += 3;
			}
		}
	}
}

void GLVolumeFlowPlot::Render(CGLContext& rc)
{
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	
	vec3d r(0,0,1);
	quatd q = rc.m_q;
	q.Inverse().RotateVector(r);

	double x = fabs(r.x);
	double y = fabs(r.y);
	double z = fabs(r.z);

	if ((x > y) && (x > z)) { RenderSlices(m_slice_X, (r.x > 0 ? 1 : -1)); }
	if ((y > x) && (y > z)) { RenderSlices(m_slice_Y, (r.y > 0 ? 1 : -1)); }
	if ((z > y) && (z > x)) { RenderSlices(m_slice_Z, (r.z > 0 ? 1 : -1)); }

	glPopAttrib();
}

void GLVolumeFlowPlot::RenderSlices(std::vector<GLVolumeFlowPlot::Slice>& slice, int step)
{
	// determine the order in which we have to render the slices
	int n = (int)slice.size();
	int n0, n1;
	if (step == 1) { n0 = 0; n1 = n; }
	else { n0 = n - 1; n1 = -1; }

	// get the color map
	CColorMap& col = m_Col.ColorMap();


	// start rendering
	GLColor c;
	glBegin(GL_TRIANGLES);
	{
		for (int i = n0; i != n1; i += step)
		{
			Slice& s = slice[i];

			int NF = (int)s.m_Face.size();
			for (int j = 0; j < NF; ++j)
			{
				Slice::Face& face = s.m_Face[j];

				c = col.map(face.v[0]);
				glColor4ub(c.r, c.g, c.b, 255 *face.v[0] * m_alpha);
				glVertex3f(face.r[0].x, face.r[0].y, face.r[0].z);

				c = col.map(face.v[1]);
				glColor4ub(c.r, c.g, c.b, 255 *face.v[1] * m_alpha);
				glVertex3f(face.r[1].x, face.r[1].y, face.r[1].z);

				c = col.map(face.v[2]);
				glColor4ub(c.r, c.g, c.b, 255 *face.v[2] * m_alpha);
				glVertex3f(face.r[2].x, face.r[2].y, face.r[2].z);
			}
		}
	}
	glEnd();
}
