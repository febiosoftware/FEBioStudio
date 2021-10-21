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
#include "GLProbe.h"
#include "GLModel.h"
#include <MeshLib/MeshTools.h>
#include <PostLib/constants.h>
#include <MeshTools/FETetGenMesher.h>
#include <MeshLib/FENodeNodeList.h>
#include <GLLib/glx.h>
#include <sstream>
using namespace Post;

GLProbe::GLProbe(CGLModel* fem) : CGLPlot(fem)
{
	static int n = 1;
	char sz[256] = { 0 };
	sprintf(sz, "Probe%d", n++);
	SetName(sz);

	m_initPos = vec3d(0, 0, 0);
	m_size = 1.0;
	m_col = GLColor::White();
	m_bfollow = true;

	AddVecParam(m_initPos, "Initial position");
	AddDoubleParam(m_size, "Size scale factor");
	AddColorParam(m_col, "Color");
	AddBoolParam(m_bfollow, "Follow");

	m_lastTime = 0;
	m_lastdt = 1.0;
	m_R = 1.0;
	m_elem = -1;
}

bool GLProbe::UpdateData(bool bsave)
{
	if (bsave)
	{
		vec3d r = m_initPos;

		m_initPos = GetVecValue(INIT_POS);
		m_size = GetFloatValue(SIZE);
		m_col = GetColorValue(COLOR);
		m_bfollow = GetBoolValue(FOLLOW);

		if (!(r == m_initPos)) Update();
	}
	else
	{
		SetVecValue(INIT_POS, m_initPos);
		SetFloatValue(SIZE, m_size);
		SetColorValue(COLOR, m_col);
		SetBoolValue(FOLLOW, m_bfollow);
	}

	return false;
}

void GLProbe::Render(CGLContext& rc)
{
	double R = m_R * m_size;
	GLUquadricObj* pobj = gluNewQuadric();
	glColor3ub(m_col.r, m_col.g, m_col.b);
	glPushMatrix();
	{
		glTranslated(m_pos.x, m_pos.y, m_pos.z);
		gluSphere(pobj, R, 32, 32);
	}
	glPopMatrix();

	gluDeleteQuadric(pobj);
}

void GLProbe::Update()
{
	Update(m_lastTime, m_lastdt, true);
}

bool ProjectToElement(FEElement& el, const vec3f& p, vec3f* x0, vec3f* xt, vec3f& q)
{
	int ne = el.Nodes();
	BOX box;
	for (int i = 0; i < ne; ++i) box += x0[i];
	if (box.IsInside(p) == false) return false;

	double r[3] = { 0,0,0 };
	project_inside_element(el, p, r, x0);
	if (IsInsideElement(el, r, 0.001))
	{
		q = el.eval(xt, r[0], r[1], r[2]);
		return true;
	}
	return false;
}

void GLProbe::Update(int ntime, float dt, bool breset)
{
	if ((breset == false) && (ntime == m_lastTime) && (dt == m_lastdt)) return;

	m_lastTime = ntime;
	m_lastdt = dt;

	m_pos = m_initPos;
	m_elem = -1;

	CGLModel* mdl = GetModel();
	if (mdl == nullptr) return;

	FEPostMesh* mesh = mdl->GetActiveMesh();
	if (mesh == nullptr) return;

	// update the size of the probe
	BOX box = mdl->GetFEModel()->GetBoundingBox();
	m_R = 0.05*box.GetMaxExtent();

	// see if we need to revaluate the FEFindElement object
	// We evaluate it when the plot needs to be reset, or when the model has a displacement map
	bool bdisp = mdl->HasDisplacementMap();
	if (bdisp == false) return;

	vec3f p0 = to_vec3f(m_initPos);
	m_elem = ProjectToMesh(ntime, p0, m_pos);
}

int GLProbe::ProjectToMesh(int nstate, const vec3f& r0, vec3d& rt)
{
	CGLModel* mdl = GetModel();
	if (mdl == nullptr) return -1;

	Post::FEState* state = mdl->GetFEModel()->GetState(nstate);
	Post::FERefState* ps = state->m_ref;
	Post::FEPostMesh& mesh = *state->GetFEMesh();

	int nelem = -1;
	vec3f x0[FEElement::MAX_NODES];
	vec3f xt[FEElement::MAX_NODES];
	int NE = mesh.Elements();
	for (int i = 0; i < NE; ++i)
	{
		FEElement& el = mesh.Element(i);
		int ne = el.Nodes();
		for (int j = 0; j < el.Nodes(); ++j)
		{
			x0[j] = ps->m_Node[el.m_node[j]].m_rt;
			xt[j] = to_vec3f(mesh.Node(el.m_node[j]).r);
		}

		if (m_bfollow)
		{
			vec3f q;
			if (ProjectToElement(el, r0, x0, xt, q))
			{
				rt = q;
				nelem = i;
				break;
			}
		}
		else
		{
			vec3f q;
			if (ProjectToElement(el, r0, x0, x0, q))
			{
				rt = q;
				nelem = i;
				break;
			}
		}
	}

	return nelem;
}

GLColor GLProbe::GetColor() const
{
	return m_col;
}

void GLProbe::SetColor(const GLColor& c)
{
	m_col = c;
}

double GLProbe::DataValue(int nfield, int nstep)
{
	FEPostModel& fem = *GetModel()->GetFEModel();
	float val = 0.f;
	vec3f p0 = to_vec3f(m_initPos);
	int nelem = ProjectToMesh(nstep, p0, m_pos);
	if (nelem >= 0)
	{
		float data[FEElement::MAX_NODES];
		fem.EvaluateElement(nelem, nstep, nfield, data, val);
	}
	return val;
}

//==============================================================================

static int n = 1;
GLMusclePath::GLMusclePath(CGLModel* fem) : CGLPlot(fem)
{
	AddIntParam(0, "start point");
	AddIntParam(0, "end point");
	AddVecParam(vec3d(0, 0, 0), "Center of rotation");
	AddDoubleParam(5.0, "size");
	AddColorParam(GLColor(255, 0, 0), "color");

	std::stringstream ss;
	ss << "MusclePath" << n++;
	SetName(ss.str());

	m_hull = nullptr;

	// we need the active face selection 
	if (fem)
	{
		FEPostMesh& mesh = *fem->GetActiveMesh();
		mesh.TagAllNodes(0);
		for (int i = 0; i < mesh.Faces(); ++i)
		{
			FEFace& f = mesh.Face(i);
			if (f.IsSelected())
			{
				for (int j = 0; j < f.Nodes(); ++j) mesh.Node(f.n[j]).m_ntag = 1;
			}
		}

		// get all the tagged nodes
		for (int i = 0; i < mesh.Nodes(); ++i)
		{
			if (mesh.Node(i).m_ntag == 1) m_selNodes.push_back(i);
		}
	}
}

void GLMusclePath::Render(CGLContext& rc)
{
	if (m_path.empty()) return;

	CGLModel* glm = GetModel();

	double R = GetFloatValue(SIZE);
	GLColor c = GetColorValue(COLOR);

	int N = (int)m_path.size();
	vec3d r0 = m_path[0];
	vec3d r1 = m_path[N-1];
	
	// draw the muscle path
	glColor3ub(c.r, c.g, c.b);
	glx::drawSphere(r0, 1.5*R);
	glx::drawSphere(r1, 1.5*R);
	glx::drawSmoothPath(m_path, R);

	// draw the rotation center
	vec3d o = GetVecValue(ROTATION_CENTER);
	glColor3ub(255, 255, 0);
	glx::drawSphere(o, R);

	if (m_hull)
	{
		GLMeshRender gl;
		glColor3ub(255, 64, 164);
		gl.RenderMeshLines(m_hull);
	}
}

void GLMusclePath::Update()
{

}

std::vector<vec3d> FindShortestPath(FEMesh& mesh, int m0, int m1)
{
	// helper class for finding neighbors
	FENodeNodeList NNL(&mesh);

	const double INF = 1e34;
	int N = mesh.Nodes();
	vector<double> dist(N, INF);

	mesh.TagAllNodes(-1);

	int ncurrent = m0;
	mesh.Node(m0).m_ntag = m0;
	dist[m0] = 0.0;
	double L0 = 0.0;
	while (ncurrent != m1)
	{
		// get the position of the current node
		FENode& node0 = mesh.Node(ncurrent);
		vec3d rc = node0.pos();

		// update neighbor distances
		int nval = NNL.Valence(ncurrent);
		int nmin = -1;
		double dmin = 0.0;
		for (int i = 0; i < nval; ++i)
		{
			int mi = NNL.Node(ncurrent, i);
			assert(mi != ncurrent);

			FENode& nodei = mesh.Node(mi);
			if (dist[mi] >= 0)
			{
				vec3d ri = mesh.Node(mi).pos();
				double dL = (ri - rc).Length();

				double L1 = L0 + dL;
				if (L1 < dist[mi])
				{
					dist[mi] = L1;
					nodei.m_ntag = ncurrent;
				}
				else L1 = dist[mi];

				if ((nmin == -1) || (L1 < dmin))
				{
					nmin = mi;
					dmin = L1;
				}
			}
		}

		// choose the next node
		assert(nmin != -1);
		L0 = dist[nmin];
		dist[nmin] = -1.0;
		ncurrent = nmin;
	}

	// build the path
	// NOTE: This traverses the path in reverse!
	std::vector<vec3d> path;
	ncurrent = m1;
	path.push_back(mesh.Node(m1).pos());
	do
	{
		int parentNode = mesh.Node(ncurrent).m_ntag;

		vec3d rc = mesh.Node(parentNode).pos();
		path.push_back(rc);

		ncurrent = parentNode;

	} while (ncurrent != m0);

	return path;
}

void GLMusclePath::Update(int ntime, float dt, bool breset)
{
	m_path.clear();
	delete m_hull; m_hull = nullptr;

	CGLModel* glm = GetModel();
	FEPostMesh& mesh = *glm->GetActiveMesh();
	Post::FEPostModel& fem = *glm->GetFEModel();

	int n0 = GetIntValue(START_POINT) - 1;
	int n1 = GetIntValue(END_POINT  ) - 1;

	int NN = mesh.Nodes();
	if ((n0 < 0) || (n0 >= NN)) return;
	if ((n1 < 0) || (n1 >= NN)) return;

	vec3d r0 = fem.NodePosition(n0, ntime);
	vec3d r1 = fem.NodePosition(n1, ntime);

	if (m_selNodes.size() < 2)
	{
		m_path.push_back(r0);
		m_path.push_back(r1);
	}
	else
	{
		// collect the points
		int n = m_selNodes.size();
		vector<vec3d> v; v.reserve(n + 2);
		v.push_back(r0);
		v.push_back(r1);
		for (int i = 0; i < n; ++i)
		{
			int ni = m_selNodes[i];
			vec3d ri = fem.NodePosition(ni, ntime);
			v.push_back(ri);
		}

		// calculate the convex hull
		FEConvexHullMesher mesher;

		m_hull = mesher.Create(v);
		if (m_hull == nullptr) return;

		// make sure our initial points are still outside
		FEMesh& mesh = *m_hull;
		mesh.TagAllNodes(0);
		for (int i = 0; i < mesh.Faces(); ++i)
		{
			FEFace& face = mesh.Face(i);
			for (int j = 0; j < face.Nodes(); ++j) mesh.Node(face.n[j]).m_ntag = 1;
		}
		if (mesh.Node(0).m_ntag != 1) { assert(false); return; }
		if (mesh.Node(1).m_ntag != 1) { assert(false); return; }

		// extract the outide surface
		FEMesh* surf = mesh.ExtractFaces(false);

		// calculate the shortest path
		// NOTE: This assumes that the first and last nodes remain at indices 0 and 1 of the surface mesh!
		m_path = FindShortestPath(*surf, 0, 1);
	}
}

bool GLMusclePath::UpdateData(bool bsave)
{
	return false;
}

double GLMusclePath::DataValue(int field, int step)
{
	int n0 = GetIntValue(START_POINT) - 1;
	int n1 = GetIntValue(END_POINT) - 1;

	CGLModel* glm = GetModel();

	Post::FEPostModel& fem = *glm->GetFEModel();

	vec3d r0 = fem.NodePosition(n0, step);
	vec3d r1 = fem.NodePosition(n1, step);

	vec3d c = GetVecValue(ROTATION_CENTER);

	switch (field)
	{
	case 1: return (r1 - r0).Length(); break;
	case 2: 
	{
		vec3d e = r1 - r0; e.Normalize();
		vec3d t = r1 - c;
		vec3d n = e ^ t;
		return n.Length();
		break;
	}
	break;
	}

	return 0.0;
}
