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
#include "GLMusclePath.h"
#include "GLModel.h"
#include <MeshLib/MeshTools.h>
#include <PostLib/constants.h>
#include <MeshTools/FETetGenMesher.h>
#include <MeshLib/FENodeNodeList.h>
#include <GLLib/glx.h>
#include <sstream>
using namespace Post;

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
	vec3d r1 = m_path[N - 1];

	// draw the muscle path
	glColor3ub(c.r, c.g, c.b);
	glx::drawSphere(r0, 1.5 * R);
	glx::drawSphere(r1, 1.5 * R);
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
	int n1 = GetIntValue(END_POINT) - 1;

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
