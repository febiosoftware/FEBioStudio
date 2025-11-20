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
#include "GLVolumeFlowPlot.h"
#include "GLModel.h"
#include <GLLib/GLContext.h>
#include <GLLib/GLCamera.h>
#include <FSCore/ClassDescriptor.h>
#include <FSCore/util.h>
#include <GLLib/GLRenderEngine.h>

using namespace Post;

extern int LUT[256][15];
extern int ET_HEX[12][2];

REGISTER_CLASS(GLVolumeFlowPlot, CLASS_PLOT, "volume-flow", 0);

GLVolumeFlowPlot::GLVolumeFlowPlot()
{
	SetTypeString("volume-flow");

	static int n = 1;
	char szname[128] = { 0 };
	sprintf(szname, "VolumeFlow%d", n++);
	SetName(szname);

	AddIntParam(0, "data_field")->SetEnumNames("@data_scalar");
	AddIntParam(0, "color_map")->SetEnumNames("@color_map");
	AddBoolParam(true, "smooth_color_map");
	AddIntParam(10, "range_divisions")->SetIntRange(2, 50);
	AddDoubleParam(0.2, "opacity_scale")->SetFloatRange(0.0, 1.0);
	AddDoubleParam(1.0, "opacity_strength")->SetFloatRange(0.001, 0.999);
	AddIntParam(1, "mesh_subdivisions")->SetIntRange(1, MAX_MESH_DIVS);
	AddBoolParam(true, "show_legend");
	AddIntParam(0, "max_range_type")->SetEnumNames("dynamic\0static\0user\0");
	AddDoubleParam(0, "user_max");
	AddIntParam(0, "min_range_type")->SetEnumNames("dynamic\0static\0user\0");
	AddDoubleParam(0, "user_min");

	m_alpha = 0.2f;
	m_gain = 0.5f;
	m_nfield = 0;
	m_bsmooth = true;
	m_nDivs = 10;
	m_meshDivs = 1;
	m_range.min = m_range.max = 0;
	m_range.mintype = m_range.maxtype = RANGE_DYNAMIC;
	m_range.valid = true;

	UpdateData(false);
}

bool GLVolumeFlowPlot::UpdateData(bool bsave)
{
	if (bsave)
	{
		bool update = false;
		if (m_nfield != GetIntValue(DATA_FIELD)) { m_nfield = GetIntValue(DATA_FIELD); update = true; }
		m_Col.SetColorMap(GetIntValue(COLOR_MAP));
		m_bsmooth = GetBoolValue(SMOOTH_COLOR_MAP);
		m_nDivs = GetIntValue(RANGE_DIVISIONS);
		if (m_alpha != GetFloatValue(OPACITY_SCALE)) { m_alpha = GetFloatValue(OPACITY_SCALE); update = true; };
		if (m_gain != GetFloatValue(OPACITY_STRENGTH)) { m_gain = GetFloatValue(OPACITY_STRENGTH); update = true; };
		if (m_meshDivs != GetIntValue(MESH_DIVISIONS)) { m_meshDivs = GetIntValue(MESH_DIVISIONS); update = true; };
		if (m_range.maxtype != GetIntValue(MAX_RANGE_TYPE)) { m_range.maxtype = GetIntValue(MAX_RANGE_TYPE); update = true; }
		if (m_range.mintype != GetIntValue(MIN_RANGE_TYPE)) { m_range.mintype = GetIntValue(MIN_RANGE_TYPE); update = true; }
		if (m_range.maxtype == RANGE_USER) m_range.max = GetFloatValue(USER_MAX);
		if (m_range.mintype == RANGE_USER) m_range.min = GetFloatValue(USER_MIN);

		m_Col.SetSmooth(m_bsmooth);
		m_Col.SetDivisions(m_nDivs);

		if (update) Update(GetModel()->CurrentTimeIndex(), 0.f, true);
	}
	else
	{
		SetIntValue(DATA_FIELD, m_nfield);
		SetIntValue(COLOR_MAP, m_Col.GetColorMap());
		SetBoolValue(SMOOTH_COLOR_MAP, m_bsmooth);
		SetIntValue(RANGE_DIVISIONS, m_nDivs);
		SetFloatValue(OPACITY_SCALE, m_alpha);
		SetFloatValue(OPACITY_STRENGTH, m_gain);
		SetIntValue(MESH_DIVISIONS, m_meshDivs);
		SetIntValue(MAX_RANGE_TYPE, m_range.maxtype);
		SetIntValue(MIN_RANGE_TYPE, m_range.mintype);
		SetFloatValue(USER_MAX, m_range.max);
		SetFloatValue(USER_MIN, m_range.min);
	}

	return false;
}

void GLVolumeFlowPlot::Update(int ntime, float dt, bool breset)
{
	UpdateNodalData(ntime, breset);
}

void GLVolumeFlowPlot::CreateSlices(std::vector<Slice>& slice, const vec3d& normal)
{
	// get the model
	CGLModel& mdl = *GetModel();

	FEPostModel* fem = mdl.GetFSModel();
	FEState* state = fem->CurrentState();

	// get the current mesh
	FSMeshBase& mesh = *mdl.GetActiveMesh();

	// get the largest dimension
	BOX box = m_box;
	vec3d r0 = box.r0();
	vec3d r1 = box.r1();
	double R = (r1 - r0).norm();

	// coordinates of box
	vec3d c[8];
	c[0] = vec3d(r0.x, r0.y, r0.z);
	c[1] = vec3d(r1.x, r0.y, r0.z);
	c[2] = vec3d(r1.x, r1.y, r0.z);
	c[3] = vec3d(r0.x, r1.y, r0.z);
	c[4] = vec3d(r0.x, r0.y, r1.z);
	c[5] = vec3d(r1.x, r0.y, r1.z);
	c[6] = vec3d(r1.x, r1.y, r1.z);
	c[7] = vec3d(r0.x, r1.y, r1.z);

	// find the min, max projection distance
	double tmin = 1e99, tmax = -1e99;
	for (int i = 0; i < 8; ++i)
	{
		double ti = normal * c[i];
		if (ti < tmin) tmin = ti;
		if (ti > tmax) tmax = ti;
	}
	double rng = tmax - tmin;

	// this is the resolution for the largest dimension
	const int MAX_RES = 128;
	const int MIN_RES = 8;

	// set the resolutions based on the box dimensions
	int nslices = (int)(rng / R * MAX_RES); if (nslices < MIN_RES) nslices = MIN_RES;

	int ndivs = m_meshDivs;
	if (ndivs < 1) ndivs = 1;
	if (ndivs > MAX_MESH_DIVS) ndivs = MAX_MESH_DIVS;
	nslices *= ndivs;

	// allocate the slices
	slice.resize(nslices);

	// build the slices
#pragma omp parallel for default(shared)
	for (int i = 0; i < nslices; ++i)
	{
		float x = tmin + ((float)i)*(tmax - tmin) / (nslices - 1.f);
		CreateSlice(slice[i], normal, x);
	}
}

//-----------------------------------------------------------------------------
void GLVolumeFlowPlot::UpdateBoundingBox()
{
	CGLModel* mdl = GetModel();
	FEPostModel* ps = mdl->GetFSModel();
	FSMesh* pm = mdl->GetActiveMesh();

	// only count enabled parts
	BOX box;
	for (int i = 0; i < pm->Elements(); ++i)
	{
		FSElement_& el = pm->ElementRef(i);
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

void GLVolumeFlowPlot::UpdateNodalData(int ntime, bool breset)
{
	CGLModel* mdl = GetModel();

	FSMesh* pm = mdl->GetActiveMesh();
	FEPostModel* pfem = mdl->GetFSModel();

	int NN = pm->Nodes();
	int NS = pfem->GetStates();

	// update the box
	UpdateBoundingBox();

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

		// only count enabled parts
		pm->TagAllNodes(0);
		for (int i = 0; i < pm->Elements(); ++i)
		{
			FSElement_& el = pm->ElementRef(i);
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
		for (int i = 0; i < NN; ++i)
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

	// update range
	float fmin = m_rng[ntime].x;
	float fmax = m_rng[ntime].y;

	switch (m_range.mintype)
	{
	case 1:	if (fmin > m_range.min) fmin = m_range.min; break;
	case 2: fmin = m_range.min; break;
	}

	switch (m_range.maxtype)
	{
	case 1:	if (fmax < m_range.max) fmax = m_range.max; break;
	case 2: fmax = m_range.max; break;
	}

	m_range.min = fmin;
	m_range.max = fmax;
	if (fmax == fmin) m_range.max += 1;
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
	FEPostModel* ps = mdl->GetFSModel();
	FSMesh* pm = mdl->GetActiveMesh();

	vec2f rng;
	rng.x = m_range.min;
	rng.y = m_range.max;

	if (rng.x == rng.y) rng.y++;

	int matId = -1;
	Material* pmat = nullptr;

	// loop over all elements
	for (int iel = 0; iel <pm->Elements(); ++iel)
	{
		// render only if the element is visible and
		// its material is enabled
		FSElement_& el = pm->ElementRef(iel);
		if (el.m_MatID != matId)
		{
			pmat = ps->GetMaterial(el.m_MatID);
			matId = el.m_MatID;
		}

		if ((pmat && pmat->benable) && el.IsVisible() && el.IsSolid())
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
			case FE_TET10: nt = TET_NT; break;
			case FE_TET15: nt = TET_NT; break;
			case FE_TET20: nt = TET_NT; break;
			case FE_PYRA5: nt = PYR_NT; break;
			case FE_PYRA13: nt = PYR_NT; break;
			default:
				assert(false);
				return;
			}

			// get the case
			int ncase = 0;
			for (int k = 0; k<8; ++k)
			{
				vec3d& r = pm->Node(el.m_node[nt[k]]).r;
				double v = r*norm;
				if (v <= ref) ncase |= (1 << k);
			}

			// loop over faces
			if ((ncase > 0) && (ncase < 255))
			{
				// get the nodal data
				for (int k = 0; k < 8; ++k)
				{
					FSNode& node = pm->Node(el.m_node[nt[k]]);

					float f = m_val[el.m_node[nt[k]]];
					f = (f - rng.x) / (rng.y - rng.x);

					ev[k] = f;
					er[k] = node.r;
					ex[k] = node.r * norm;
				}

				int* pf = LUT[ncase];
				for (int l = 0; l < 5; l++)
				{
					if (*pf == -1) break;

					// calculate nodal positions
					vec3d r[3];
					double v[3];
					for (int k = 0; k < 3; k++)
					{
						int n1 = ET_HEX[pf[k]][0];
						int n2 = ET_HEX[pf[k]][1];

						double w = 0.5;
						if (ex[n2] != ex[n1])
							w = (ref - ex[n1]) / (ex[n2] - ex[n1]);

						r[k] = er[n1] * (1 - w) + er[n2] * w;
						v[k] = ev[n1] * (1 - w) + ev[n2] * w;
					}

					Slice::Face face;
					face.v[0] = v[0]; face.r[0] = to_vec3f(r[0]);
					face.v[1] = v[1]; face.r[1] = to_vec3f(r[1]);
					face.v[2] = v[2]; face.r[2] = to_vec3f(r[2]);

					slice.add(face);

					pf += 3;
				}
			}
		}
	}
}

void GLVolumeFlowPlot::Render(GLRenderEngine& re, GLContext& rc)
{
	re.setMaterial(GLMaterial::CONSTANT, GLColor::White(), GLMaterial::VERTEX_COLOR, false);

	// get view direction
	vec3d view(0,0,1);
	quatd q = rc.m_cam->GetOrientation();
	q.Inverse().RotateVector(view);

	// update the geometry
	std::vector<GLVolumeFlowPlot::Slice> slice;
	CreateSlices(slice, view);
	UpdateMesh(slice, m_mesh);

	// render the geometry
	re.renderGMesh(m_mesh, false);
}

void GLVolumeFlowPlot::UpdateMesh(std::vector<GLVolumeFlowPlot::Slice>& slice, GLMesh& mesh)
{
	// count the faces
	int totalFaces = 0;
	int n = (int)slice.size();
	for (int i = 0; i < n; i++)
	{
		Slice& s = slice[i];
		totalFaces += (int)s.m_Face.size();
	}

	// allocate the mesh
	mesh.Clear();

	CColorMap& col = m_Col.ColorMap();

	// build the mesh
	for (int i = 0; i < n; i++)
	{
		Slice& s = slice[i];

		int NF = (int)s.m_Face.size();
		GLColor c[3];
		for (int j = 0; j < NF; ++j)
		{
			Slice::Face& face = s.m_Face[j];

			double v = face.v[0];
			double a = (v > 0 ? (v < 1 ? v : 1) : 0);
			a = m_alpha * gain(m_gain, a);

			c[0] = col.map(v);
			c[0].a = (uint8_t)(255 * a);

			v = face.v[1];
			a = (v > 0 ? (v < 1 ? v : 1) : 0);
			a = m_alpha * gain(m_gain, a);

			c[1] = col.map(v);
			c[1].a = (uint8_t)(255 * a);

			v = face.v[2];
			a = (v > 0 ? (v < 1 ? v : 1) : 0);
			a = m_alpha * gain(m_gain, a);
			
			c[2] = col.map(v);
			c[2].a = (uint8_t)(255 * a);

			mesh.AddFace(face.r, c);
		}
	}
	mesh.Update();
}

LegendData GLVolumeFlowPlot::GetLegendData() const
{
	LegendData l;
	bool showLegend = GetBoolValue(SHOW_LEGEND);
	if (m_range.valid && showLegend)
	{
		l.discrete = false;
		l.ndivs = m_nDivs;
		l.vmin = m_range.min;
		l.vmax = m_range.max;
		l.smooth = true;
		l.colormap = GetIntValue(COLOR_MAP);
		l.title = GetName();
	}

	return l;
}
