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
#include "GLColorMap.h"
#include <PostGL/GLModel.h>
#include <PostLib/constants.h>
#include <FSCore/ColorMapManager.h>
using namespace Post;

//-----------------------------------------------------------------------------
// CGLColorMap
//-----------------------------------------------------------------------------

int CGLColorMap::m_defaultRngType = Range_Type::RANGE_DYNAMIC;

CGLColorMap::CGLColorMap(CGLModel *po) : CGLDataMap(po)
{
	AddIntParam (-1, "data_field", "Data field")->SetEnumNames("@data_scalar");
	AddBoolParam(true, "gradient_smoothing");
	AddIntParam (ColorMapManager::JET, "gradient")->SetEnumNames("@color_map");
	AddBoolParam(true, "nodal_smoothing");
	AddIntParam (10, "range_divisions")->SetIntRange(1, 100);
	AddBoolParam(true, "show_legend");
	AddIntParam (m_defaultRngType, "max_range_type")->SetEnumNames("dynamic\0static\0user\0");
	AddDoubleParam(0, "user_max");
	AddIntParam (m_defaultRngType, "min_range_type")->SetEnumNames("dynamic\0static\0user\0");
	AddDoubleParam(0, "user_min");
	AddBoolParam(false, "show_minmax_markers", "Show min/max markers");
	AddColorParam(GLColor(200, 200, 200), "inactive_color");

	m_range.min = m_range.max = 0;
	m_range.mintype = m_range.maxtype = m_defaultRngType;
	m_rmin = m_rmax = vec3d(0, 0, 0);

	m_nfield = -1;
	m_breset = true;
	m_bDispNodeVals = true;

	SetName("Color Map");

	UpdateData(false);

	// we start the colormap as inactive
	Activate(false);
}

//-----------------------------------------------------------------------------
CGLColorMap::~CGLColorMap()
{
}

//-----------------------------------------------------------------------------
bool CGLColorMap::UpdateData(bool bsave)
{
	if (bsave)
	{
		m_nfield = GetIntValue(DATA_FIELD);
		m_bDispNodeVals = GetBoolValue(NODAL_VALS);
		m_range.maxtype = GetIntValue(MAX_RANGE_TYPE);
		m_range.mintype = GetIntValue(MIN_RANGE_TYPE);
		if (m_range.maxtype == RANGE_USER) m_range.max = GetFloatValue(USER_MAX);
		if (m_range.mintype == RANGE_USER) m_range.min = GetFloatValue(USER_MIN);

		Update();
	}
	else
	{
		SetIntValue(DATA_FIELD, m_nfield);
		SetBoolValue(NODAL_VALS, m_bDispNodeVals);
		SetIntValue(MAX_RANGE_TYPE, m_range.maxtype);
		SetIntValue(MIN_RANGE_TYPE, m_range.mintype);
		SetFloatValue(USER_MAX, m_range.max);
		SetFloatValue(USER_MIN, m_range.min);
	}

	return false;
}

//-----------------------------------------------------------------------------
bool CGLColorMap::ShowMinMaxMarkers() const
{
	return GetBoolValue(SHOW_MINMAX_MARKERS);
}

//-----------------------------------------------------------------------------
void CGLColorMap::SetEvalField(int n)
{
	if (n != m_nfield)
	{
		m_nfield = n;
		m_breset = true;
		UpdateData(false);
	}
}

//-----------------------------------------------------------------------------
GLColor CGLColorMap::GetInactiveColor()
{
	return GetColorValue(INACTIVE_COLOR);
}

//-----------------------------------------------------------------------------
void CGLColorMap::Update()
{
	Update(GetModel()->CurrentTimeIndex(), 0.f, false);
}

//-----------------------------------------------------------------------------
void CGLColorMap::Update(int ntime, float dt, bool breset)
{
	CGLModel* po = GetModel();
	FEPostModel* pfem = po->GetFSModel();

	int N = pfem->GetStates();
	if (N == 0) return;

	int n0 = ntime;
	int n1 = (ntime + 1 >= N ? ntime : ntime + 1);
	if (dt == 0.f) n1 = n0;

	UpdateState(n0, breset);
	if (n0 != n1) UpdateState(n1, breset);

	UpdateRange(n0, n1, dt, breset);

	UpdateRenderMesh(n0, n1, dt);
}

void CGLColorMap::UpdateRange(int n0, int n1, float dt, bool breset)
{
	CGLModel* po = GetModel();
	FEPostModel* pfem = po->GetFSModel();
	FSMesh* pm = po->GetActiveMesh();

	// get the states
	FEState& s0 = *pfem->GetState(n0);
	FEState& s1 = *pfem->GetState(n1);

	float df = s1.m_time - s0.m_time;
	if (df == 0) df = 1.f;
	float w = dt / df;

	m_rmin = m_rmax = vec3d(0, 0, 0);

	// update the range
	float fmin = 1e29f, fmax = -1e29f;
	if (IS_ELEM_FIELD(m_nfield) && (m_bDispNodeVals == false))
	{
		int ndata = FIELD_CODE(m_nfield);
		if (s0.m_Data[ndata].GetFormat() == DATA_ITEM)
		{
			int NE = pm->Elements();
			for (int i = 0; i < NE; ++i)
			{
				FSElement_& el = pm->ElementRef(i);
				ELEMDATA& d0 = s0.m_ELEM[i];
				ELEMDATA& d1 = s1.m_ELEM[i];
				if ((d0.m_state & StatusFlags::ACTIVE) && (d1.m_state & StatusFlags::ACTIVE))
				{
					vec3d r = pm->ElementCenter(el);
					float f0 = d0.m_val;
					float f1 = d1.m_val;
					float f = f0 + (f1 - f0) * w;
					if (f > fmax) { fmax = f; m_rmax = r; }
					if (f < fmin) { fmin = f; m_rmin = r; }
				}
			}
		}
		else
		{
			ValArray& elemData0 = s0.m_ElemData;
			ValArray& elemData1 = s1.m_ElemData;
			int NE = pm->Elements();
			for (int i = 0; i < NE; ++i)
			{
				FSElement_& el = pm->ElementRef(i);
				ELEMDATA& d0 = s0.m_ELEM[i];
				ELEMDATA& d1 = s1.m_ELEM[i];
				if ((d0.m_state & StatusFlags::ACTIVE) && (d1.m_state & StatusFlags::ACTIVE))
				{
					for (int j = 0; j < el.Nodes(); ++j)
					{
						float f0 = elemData0.value(i, j);
						float f1 = elemData1.value(i, j);
						float f = f0 + (f1 - f0) * w;
						if (f > fmax) { fmax = f; m_rmax = pm->Node(el.m_node[j]).r; }
						if (f < fmin) { fmin = f; m_rmin = pm->Node(el.m_node[j]).r; }
					}
				}
			}
		}
	}
	else if (IS_FACE_FIELD(m_nfield) && (m_bDispNodeVals == false))
	{
		int ndata = FIELD_CODE(m_nfield);
		if (s0.m_Data[ndata].GetFormat() == DATA_ITEM)
		{
			int NF = pm->Faces();
			for (int i = 0; i < NF; ++i)
			{
				FSFace& face = pm->Face(i);
				FACEDATA& d0 = s0.m_FACE[i];
				FACEDATA& d1 = s1.m_FACE[i];
				vec3d r = pm->FaceCenter(face);
				float f0 = d0.m_val;
				float f1 = d1.m_val;
				float f = f0 + (f1 - f0) * w;
				if (f > fmax) { fmax = f; m_rmax = r; }
				if (f < fmin) { fmin = f; m_rmin = r; }
			}
		}
		else
		{
			int NF = pm->Faces();
			ValArray& faceData0 = s0.m_FaceData;
			ValArray& faceData1 = s1.m_FaceData;
			for (int i = 0; i < NF; ++i)
			{
				FSFace& face = pm->Face(i);
				FACEDATA& fd0 = s0.m_FACE[i];
				FACEDATA& fd1 = s1.m_FACE[i];
				if (face.IsEnabled() && (fd0.m_ntag > 0))
				{
					face.m_ntag = 1;
					int nf = face.Nodes();
					for (int j = 0; j < nf; ++j)
					{
						float f0 = faceData0.value(i, j);
						float f1 = (n0 == n1 ? f0 : faceData1.value(i, j));
						float f = f0 + (f1 - f0) * w;
						if (f > fmax) fmax = f;
						if (f < fmin) fmin = f;
					}
				}
			}
		}
	}
	else
	{
		// evaluate all nodes to find range
		m_nodeData.resize(pm->Nodes());
		for (int i = 0; i < pm->Nodes(); ++i)
		{
			FSNode& node = pm->Node(i);
			NODEDATA& d0 = s0.m_NODE[i];
			NODEDATA& d1 = s1.m_NODE[i];
			if ((node.IsEnabled()) && (d0.m_ntag > 0) && (d1.m_ntag > 0))
			{
				float f0 = d0.m_val;
				float f1 = d1.m_val;
				float f = f0 + (f1 - f0) * w;
				node.m_ntag = 1;
				if (f > fmax) { fmax = f; m_rmax = pm->Node(i).r; }
				if (f < fmin) { fmin = f; m_rmin = pm->Node(i).r; }

				m_nodeData[i] = f;
			}
			else
			{
				node.m_ntag = 0;
				m_nodeData[i] = 0.f;
			}
		}
	}

	if (m_breset || breset)
	{
		if (m_range.maxtype != RANGE_USER) m_range.max = fmax;
		if (m_range.mintype != RANGE_USER) m_range.min = fmin;
		m_breset = false;
	}
	else
	{
		switch (m_range.maxtype)
		{
		case RANGE_DYNAMIC:
			m_range.max = fmax;
			break;
		case RANGE_STATIC:
			if (fmax > m_range.max) m_range.max = fmax;
			break;
		}

		switch (m_range.mintype)
		{
		case RANGE_DYNAMIC:
			m_range.min = fmin;
			break;
		case RANGE_STATIC:
			if (fmin < m_range.min) m_range.min = fmin;
			break;
		}
	}

	// update elements
	for (int i = 0; i < pm->Elements(); ++i)
	{
		FSElement_& el = pm->ElementRef(i);
		ELEMDATA& d0 = s0.m_ELEM[i];
		ELEMDATA& d1 = s1.m_ELEM[i];
		if ((d0.m_state & StatusFlags::ACTIVE) && (d1.m_state & StatusFlags::ACTIVE))
		{
			el.Activate();
		}
		else el.Deactivate();
	}

	for (int i = 0; i < pm->Faces(); ++i)
	{
		FSFace& face = pm->Face(i);
		FACEDATA& fd = s0.m_FACE[i];
		if (face.IsEnabled())
		{
			if (fd.m_ntag > 0) face.Activate(); else face.Deactivate();
		}
	}
}

void CGLColorMap::UpdateRenderMesh(int n0, int n1, float dt)
{
	CGLModel* po = GetModel();
	FEPostModel* pfem = po->GetFSModel();
	FSMesh* pm = po->GetActiveMesh();

	// get the states
	FEState& s0 = *pfem->GetState(n0);
	FEState& s1 = *pfem->GetState(n1);

	float df = s1.m_time - s0.m_time;
	if (df == 0) df = 1.f;
	float w = dt / df;

	float min = m_range.min;
	float max = m_range.max;
	if (min == max) max++;
	float dti = 1.f / (max - min);

	CPostObject* obj = po->GetPostObject();
	GLMesh* gmsh = obj->GetFERenderMesh(); assert(gmsh);
	if (gmsh == nullptr) return;

	vector<double> buf(pm->Nodes());

	if (m_bDispNodeVals == false)
	{
		ValArray& faceData0 = s0.m_FaceData;
		ValArray& faceData1 = s1.m_FaceData;

		vector<float> buf(pm->Nodes());

		for (int i = 0; i < gmsh->Faces(); ++i)
		{
			GLMesh::FACE& glface = gmsh->Face(i);
			if (glface.pid < obj->Faces())
			{
				assert(glface.fid >= 0);
				FSFace& face = pm->Face(glface.fid);

				FACEDATA& fd0 = s0.m_FACE[glface.fid];
				FACEDATA& fd1 = s1.m_FACE[glface.fid];
				if (face.IsEnabled() && (fd0.m_ntag > 0))
				{
					face.m_ntag = 1;
					for (int j = 0; j < face.Nodes(); ++j)
					{
						float f0 = faceData0.value(glface.fid, j);
						float f1 = (n0 == n1 ? f0 : faceData1.value(glface.fid, j));
						float f = f0 + (f1 - f0) * w;
						buf[face.n[j]] = f;
					}

					for (int j = 0; j < 3; ++j)
					{
						int nj = glface.n[j];
						glface.t[j] = vec3f((buf[nj] - min) * dti, 0.f, 0.f);
					}
				}
				else
				{
					for (int j = 0; j < 3; ++j)
					{
						glface.t[j] = vec3f(0.f, 0.f, 0.f);
					}
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < gmsh->Faces(); ++i)
		{
			GLMesh::FACE& glface = gmsh->Face(i);
			if (glface.pid < obj->Faces())
			{
				assert(glface.fid >= 0);
				FSFace& face = pm->Face(glface.fid);
				if (face.IsEnabled())
				{
					for (int j = 0; j < 3; ++j)
					{
						int nj = glface.n[j];
						float f = m_nodeData[nj];
						f = (f - min) * dti;
						glface.t[j] = vec3f(f, 0.f, 0.f);
					}
				}
				else
				{
					for (int j = 0; j < 3; ++j)
					{
						glface.t[j] = vec3f(0.f, 0.f, 0.f);
					}
				}
			}
		}
	}

	int NS = obj->InternalSurfaces();
	for (int i = 0; i < NS; ++i)
	{
		GLSurface& surf = obj->InteralSurface(i);
		int NF = surf.Faces();
		for (int j = 0; j < NF; ++j)
		{
			FSFace& face = surf.Face(j);
			if (face.m_elem[0].eid == -1) face.Deactivate();
			else
			{
				if (IS_FACE_FIELD(m_nfield)) face.Deactivate();
				else face.Activate();
			}
		}
	}

	// update the internal surfaces of the model
	for (int i = 0; i < gmsh->Faces(); ++i)
	{
		GLMesh::FACE& glface = gmsh->Face(i);
		if (glface.pid >= obj->Faces())
		{
			Post::GLSurface& surf = obj->InteralSurface(glface.pid - obj->Faces());
			FSFace& face = surf.Face(glface.fid);

			int iel = face.m_elem[0].eid;

			ELEMDATA& d0 = s0.m_ELEM[iel];
			ELEMDATA& d1 = s1.m_ELEM[iel];

			if (((d0.m_state & StatusFlags::ACTIVE) == 0) || ((d1.m_state & StatusFlags::ACTIVE) == 0))
			{
				face.Deactivate();
			}
			else
			{
				int nf = face.Nodes();
				for (int k = 0; k < nf; ++k)
				{
					float v = m_nodeData[face.n[k]];
					float tex = (v - min) / (max - min);
					buf[face.n[k]] = tex;
				}

				for (int j = 0; j < 3; ++j)
				{
					int nj = glface.n[j];
					glface.t[j] = vec3f(buf[nj], 0.f, 0.f);
				}
			}
		}
	}

	// update discrete edges
	if (IS_ELEM_FIELD(m_nfield))
	{
		for (int i = 0; i < po->DiscreteEdges(); ++i)
		{
			Post::GLEdge::EDGE& de = po->DiscreteEdge(i);
			int ni = de.elem;
			if (ni >= 0)
			{
				ELEMDATA& d0 = s0.m_ELEM[ni];
				ELEMDATA& d1 = s1.m_ELEM[ni];
				if ((d0.m_state & StatusFlags::ACTIVE) && (d1.m_state & StatusFlags::ACTIVE))
				{
					float f0 = d0.m_val;
					float f1 = d1.m_val;
					float f = f0 + (f1 - f0) * w;
					de.tex[0] = de.tex[1] = (f - min) * dti;
				}
				else de.tex[0] = de.tex[1] = 0.f;
			}
		}
	}
	else
	{
		for (int i = 0; i < po->DiscreteEdges(); ++i)
		{
			Post::GLEdge::EDGE& de = po->DiscreteEdge(i);
			for (int j = 0; j < 2; ++j)
			{
				int nj = (j == 0 ? de.n0 : de.n1);
				FSNode& node = pm->Node(nj);
				NODEDATA& d0 = s0.m_NODE[nj];
				NODEDATA& d1 = s1.m_NODE[nj];
				if ((node.IsEnabled()) && (d0.m_ntag > 0) && (d1.m_ntag > 0))
				{
					float f0 = d0.m_val;
					float f1 = d1.m_val;
					float f = f0 + (f1 - f0) * w;
					de.tex[j] = (f - min) * dti;
				}
			}
		}
	}
}

void CGLColorMap::UpdateState(int ntime, bool breset)
{
	// get the model
	CGLModel* po = GetModel();
	FEPostModel* pfem = po->GetFSModel();

	// make sure the field variable is still valid
	if (pfem->IsValidFieldCode(m_nfield, ntime) == false)
	{
		// This may happen after an update if fields are deleted.
		// reset the field code
		m_nfield = -1;
		breset = true;
	}

	// evaluate the mesh
	pfem->Evaluate(m_nfield, ntime, breset);
}
