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
#include <MeshLib/triangulate.h>
#include <GLLib/glx.h>
#include <sstream>
using namespace Post;

class GLMusclePath::PathData
{
public:
	enum { PathLength, MomentArm };

public:
	PathData() : m_hull(nullptr) {}
	~PathData()
	{
		if (m_hull) delete m_hull;
	}

public:
	std::vector<vec3d>	m_points;		// points defining the path
	::FEMesh*			m_hull;			// the convex hull

	vec3d	m_ro;		// position of origin

	double		m_data[2];	// 0 = length, 1 = moment arm

private:
	PathData(const PathData& path) {}
};

static int n = 1;
GLMusclePath::GLMusclePath(CGLModel* fem) : CGLPlot(fem)
{
	AddIntParam(0, "start point");
	AddIntParam(0, "end point");
	AddVecParam(vec3d(0, 0, 0), "Center of rotation");
	AddChoiceParam(0, "Shortest path method")->SetEnumNames("Straight line\0 3-point trace\0Shortest path 2D\0Shortest path\0");
	AddDoubleParam(5.0, "size");
	AddColorParam(GLColor(255, 0, 0), "color");
	AddBoolParam(true, "Draw convex hull");

	std::stringstream ss;
	ss << "MusclePath" << n++;
	SetName(ss.str());

	m_closestFace = -1;

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

GLMusclePath::~GLMusclePath()
{
	ClearPaths();
}

void GLMusclePath::ClearPaths()
{
	for (int i = 0; i < m_path.size(); ++i)
	{
		delete m_path[i];
	}
	m_path.clear();
}

void GLMusclePath::Render(CGLContext& rc)
{
	if (m_path.empty()) return;

	CGLModel* glm = GetModel();

	int nstate = glm->CurrentTimeIndex();
	if ((nstate < 0) || (nstate >= m_path.size())) return;

	PathData* path = m_path[nstate];
	if (path == nullptr) return;

	double R = GetFloatValue(SIZE);
	GLColor c = GetColorValue(COLOR);

	// draw the path
	int N = (int) path->m_points.size();
	if (N > 1)
	{
		vec3d r0 = path->m_points[0];
		vec3d r1 = path->m_points[N - 1];

		// draw the muscle path
		glColor3ub(c.r, c.g, c.b);
		glx::drawSmoothPath(path->m_points, R);

		// draw the end points
		glx::drawSphere(r0, 1.5 * R);

		glColor3ub(255, 0, 255);
		glx::drawSphere(r1, 1.5 * R);

		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		glBegin(GL_LINES);
		for (int i = 0; i < N - 1; ++i)
		{
			vec3d r0 = path->m_points[i];
			vec3d r1 = path->m_points[i + 1];
			glx::vertex3d(r0);
			glx::vertex3d(r1);
		}
		glEnd();
		glPopAttrib();
	}

	// draw the rotation center
	vec3d o = path->m_ro;
	glColor3ub(255, 255, 0);
	glx::drawSphere(o, R);

	// draw the convex hull for the path
	bool bdrawDebug = GetBoolValue(DRAW_DEBUG);
	if (path->m_hull && bdrawDebug)
	{
		GLMeshRender gl;
		glColor3ub(255, 64, 164);
		gl.RenderMeshLines(path->m_hull);
	}
}

void GLMusclePath::Update()
{
	Update(GetModel()->CurrentTimeIndex(), 0.f, false);
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
//		assert(nmin != -1);
		if (nmin == -1)
		{
			// hhmm, something went wrong
			return std::vector<vec3d>();
		}

		L0 = dist[nmin];
		dist[nmin] = -1.0;
		ncurrent = nmin;
	}

	// build the path
	// NOTE: This traverses the path in reverse!
	std::vector<vec3d> tmp;
	ncurrent = m1;
	tmp.push_back(mesh.Node(m1).pos());
	do
	{
		int parentNode = mesh.Node(ncurrent).m_ntag;

		vec3d rc = mesh.Node(parentNode).pos();
		tmp.push_back(rc);

		ncurrent = parentNode;

	} while (ncurrent != m0);

	// invert the temp path to get the final path
	std::vector<vec3d> path;
	int n = tmp.size();
	if (n > 0)
	{
		path.resize(n);
		for (int i = 0; i < n; ++i)
		{
			path[i] = tmp[n - i - 1];
		}
	}

	return path;
}

void GLMusclePath::Update(int ntime, float dt, bool breset)
{
	CGLModel* glm = GetModel();
	Post::FEPostModel& fem = *glm->GetFEModel();

	if (breset)
	{
		m_closestFace = -1;

		// clear current path data
		ClearPaths();

		// allocate new path data
		m_path.assign(fem.GetStates(), nullptr);
	}

	if ((ntime < 0) || (ntime >= m_path.size())) return;

	// If we already calculated the path for this time step, we're done
	if (m_path[ntime] != nullptr) return;

	UpdatePath(ntime);
}

void GLMusclePath::UpdatePath(int ntime)
{
	CGLModel* glm = GetModel();
	Post::FEPostModel& fem = *glm->GetFEModel();
	FEPostMesh& mesh = *glm->GetActiveMesh();

	int n0 = GetIntValue(START_POINT) - 1;
	int n1 = GetIntValue(END_POINT) - 1;

	int NN = mesh.Nodes();
	if ((n0 < 0) || (n0 >= NN)) return;
	if ((n1 < 0) || (n1 >= NN)) return;

	vec3d r0 = fem.NodePosition(n0, ntime);
	vec3d r1 = fem.NodePosition(n1, ntime);

	PathData* path = new PathData;

	// update the location of the reference configuration first
	// since the 3-point path method needs this
	path->m_ro = UpdateOrigin(ntime);

	// see which method we're going to use
	int method = GetIntValue(METHOD);
	bool b = false;
	switch (method)
	{
	case 0: b = UpdateStraighLine   (path, ntime); break;
	case 1: b = Update3PointPath    (path, ntime); break;
	case 2: b = UpdateShortestPath2D(path, ntime); break;
	case 3: b = UpdateShortestPath  (path, ntime); break;
	}
	if (b == false) { delete path; path = nullptr; }

	// All is well, so assign the new path
	m_path[ntime] = path;

	// also update the path data
	UpdatePathData(ntime);
}

bool GLMusclePath::UpdateStraighLine(GLMusclePath::PathData* path, int ntime)
{
	CGLModel* glm = GetModel();
	Post::FEPostModel& fem = *glm->GetFEModel();

	int n0 = GetIntValue(START_POINT) - 1;
	int n1 = GetIntValue(END_POINT) - 1;

	vec3d r0 = fem.NodePosition(n0, ntime);
	vec3d r1 = fem.NodePosition(n1, ntime);

	path->m_points.clear();
	path->m_points.push_back(r0);
	path->m_points.push_back(r1);

	return true;
}

bool GLMusclePath::Update3PointPath(GLMusclePath::PathData* path, int ntime)
{
	CGLModel* glm = GetModel();
	Post::FEPostModel& fem = *glm->GetFEModel();

	int n0 = GetIntValue(START_POINT) - 1;
	int n1 = GetIntValue(END_POINT) - 1;

	// determine the three points on plane
	vec3d r0 = fem.NodePosition(n0, ntime);
	vec3d r1 = fem.NodePosition(n1, ntime);
	vec3d rc = path->m_ro;

	// determine plane normal
	vec3d N = (r1 - rc) ^ (r0 - rc);
	N.Normalize();

	// determine initial length
	double L0 = (r1 - r0).Length();

	// find all points of the mesh that intersect with this plane
	vector<vec3d> pt;
	pt.push_back(r0);
	vec3d ra = r0;
	const double dr = 0.1*L0;
	FEPostMesh& mesh = *glm->GetActiveMesh();
	do
	{
		// determine step direction
		vec3d t = r1 - ra; 
		t.Normalize();

		vec3d rb = ra + t * dr;

		// make sure we don't overstep
		double w = t * (r1 - ra);
		if (((rb - ra).Length() < 1e-12) || (w <= dr))
		{
			pt.push_back(r1);
			break;
		}

		vec3d n = rb - rc; n.Normalize();

		// find all intersections from origin to b
		vector<vec3d> q = FindAllIntersections(mesh, rc, n, false);
		if (q.empty() == false)
		{
			double Lmax = 0;
			int imax = -1;
			for (int i = 0; i < q.size(); ++i)
			{
				double Li = (q[i] - rc) * n;
				if ((imax == -1) || (Li > Lmax))
				{
					imax = i;
					Lmax = Li;
				}
			}
			assert(imax != -1);

			// add the farthest one
			rb = q[imax];
		}
		
		pt.push_back(rb);
		if (pt.size() > 20) break;
		ra = rb;
	} 
	while (true);

	// add the last point
	if ((ra - r1).Length() > 1e-7) pt.push_back(r1);

	path->m_points = pt;

	return true;
}

bool GLMusclePath::UpdateShortestPath2D(GLMusclePath::PathData* path, int ntime)
{
	CGLModel* glm = GetModel();
	Post::FEPostModel& fem = *glm->GetFEModel();

	int n0 = GetIntValue(START_POINT) - 1;
	int n1 = GetIntValue(END_POINT) - 1;

	// determine the three points on plane
	vec3d r0 = fem.NodePosition(n0, ntime);
	vec3d r1 = fem.NodePosition(n1, ntime);
	vec3d rc = path->m_ro;

	// determine plane normal
	vec3d N = (r1 - rc) ^ (r0 - rc);
	N.Normalize();

	quatd Q(N, vec3d(0, 0, 1));

	// find all edge-intersections with this plane
	FEPostMesh& mesh = *glm->GetActiveMesh();
	vector<vec3d> pt;
	for (int i = 0; i < mesh.Edges(); ++i)
	{
		FEEdge& edge = mesh.Edge(i);
		vec3d ra = fem.NodePosition(edge.n[0], ntime);
		vec3d rb = fem.NodePosition(edge.n[1], ntime);

		double D = N * (rb - ra);
		if (D != 0.0)
		{
			double lam = N * (rc - ra) / D;
			if ((lam >= 0.0) && (lam <= 1.0))
			{
				vec3d q = ra + (rb - ra) * lam;

				// make sure the point q projects onto the r0-r1 line
				vec3d t = r1 - r0;
				double mu = (t * (q - r0)) / (t * t);

				if ((mu >= 0) && (mu <= 1.0))
				{
					vec3d q2 = Q * (q - rc);
					pt.push_back(q2);
				}
			}
		}
	}

	// calculate convex hull
	std::vector<vec3d> ch = convex_hull2d(pt);

	// find the original nodes on this hull
	int m0 = -1;
	int m1 = -1;
	double L0min, L1min;
	vec3d ra = Q * (r0 - rc);
	vec3d rb = Q * (r1 - rc);
	int n = ch.size();
	for (int i = 0; i < n; ++i)
	{
		vec3d ri = ch[i];
		double L0 = (ra - ri).SqrLength();
		double L1 = (rb - ri).SqrLength();

		if ((m0 == -1) || (L0 < L0min))
		{
			m0 = i;
			L0min = L0;
		}

		if ((m1 == -1) || (L1 < L1min))
		{
			m1 = i;
			L1min = L1;
		}
	}

	// there should be two paths. Find the shortest
	double L1 = 0.0;
	for (int i = m0; i != m1;)
	{
		int ip1 = (i + 1) % n;
		double dL = (ch[i] - ch[ip1]).Length();
		L1 += dL;
		++i;
		if (i >= n) i = 0;
	}

	double L2 = 0.0;
	for (int i = m1; i != m0;)
	{
		int ip1 = (i + 1) % n;
		double dL = (ch[i] - ch[ip1]).Length();
		L2 += dL;
		++i;
		if (i >= n) i = 0;
	}

	pt.clear();
	if (L1 < L2)
	{
		for (int i = m0; i != m1;)
		{
			pt.push_back(ch[i]);
			++i;
			if (i >= n) i = 0;
		}
		pt.push_back(ch[m1]);
	}
	else
	{
		for (int i = m1; i != m0;)
		{
			pt.push_back(ch[i]);
			++i;
			if (i >= n) i = 0;
		}
		pt.push_back(ch[m0]);
	}

	// transform back to 3D
	quatd Qi = Q.Inverse();
	for (int i = 0; i < pt.size(); ++i)
	{
		vec3d r = pt[i];
		pt[i] = rc + Qi * r;
	}

	path->m_points = pt;

	return true;
}


bool GLMusclePath::UpdateShortestPath(GLMusclePath::PathData* path, int ntime)
{
	CGLModel* glm = GetModel();
	Post::FEPostModel& fem = *glm->GetFEModel();

	int n0 = GetIntValue(START_POINT) - 1;
	int n1 = GetIntValue(END_POINT) - 1;

	vec3d r0 = fem.NodePosition(n0, ntime);
	vec3d r1 = fem.NodePosition(n1, ntime);

	if (m_selNodes.size() < 2)
	{
		path->m_points.push_back(r0);
		path->m_points.push_back(r1);
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

		path->m_hull = mesher.Create(v);
		if (path->m_hull == nullptr) { return false; }

		// make sure our initial points are still outside
		FEMesh& mesh = *path->m_hull;
		mesh.TagAllNodes(0);
		for (int i = 0; i < mesh.Faces(); ++i)
		{
			FEFace& face = mesh.Face(i);
			for (int j = 0; j < face.Nodes(); ++j) mesh.Node(face.n[j]).m_ntag = 1;
		}
		if (mesh.Node(0).m_ntag != 1) { assert(false); return false; }
		if (mesh.Node(1).m_ntag != 1) { assert(false); return false; }

		// extract the outide surface
		FEMesh* surf = mesh.ExtractFaces(false);

		// calculate the shortest path
		// NOTE: This assumes that the first and last nodes remain at indices 0 and 1 of the surface mesh!
		path->m_points = FindShortestPath(*surf, 0, 1);
	}

	return true;
}

void GLMusclePath::UpdatePathData(int ntime)
{
	if ((ntime < 0) || (ntime >= m_path.size())) return;

	PathData* path = m_path[ntime];
	if (path == nullptr) return;

	// calculate path length
	vector<vec3d>& pt = path->m_points;
	double L = 0.0;
	for (int i = 0; i < pt.size() - 1; ++i)
	{
		vec3d& r0 = pt[i];
		vec3d& r1 = pt[i + 1];
		L += (r1 - r0).Length();
	}
	path->m_data[PathData::PathLength] = L;

	// calculate moment arm
	if (pt.size() >= 2)
	{
		int n = pt.size();
		vec3d& r0 = pt[n - 2];
		vec3d& r1 = pt[n - 1];

		vec3d c = path->m_ro;

		vec3d e = r1 - r0; e.Normalize();
		vec3d t = r1 - c;
		vec3d m = e ^ t;

		path->m_data[PathData::MomentArm] = m.Length();
	}
	else path->m_data[PathData::MomentArm] = 0.0;
}

bool GLMusclePath::UpdateData(bool bsave)
{
	if (bsave)
	{
		// TODO: This will recalculate everything! 
		Update(GetModel()->CurrentTimeIndex(), 0.f, true);
	}
	return false;
}

double GLMusclePath::DataValue(int field, int step)
{
	// make sure the range is valid
	if ((step < 0) || (step >= m_path.size())) return 0.0;

	// see if we should update the data
	if (m_path[step] == nullptr) UpdatePath(step);

	// get the path
	PathData* path = m_path[step];
	if (path == nullptr) return 0.0;

	// get the data field
	double val = 0.0;
	switch (field)
	{
	case 1: val = path->m_data[PathData::PathLength]; break;
	case 2: val = path->m_data[PathData::MomentArm ]; break;
	}

	// return 
	return val;
}

vec3d GLMusclePath::UpdateOrigin(int ntime)
{
	CGLModel* glm = GetModel();
	Post::FEPostModel& fem = *glm->GetFEModel();
	FEPostMesh& mesh = *glm->GetActiveMesh();

	if (m_closestFace == -1)
	{
		vec3d r0 = GetVecValue(ROTATION_CENTER);
		mesh.TagAllFaces(0);
		int nmin = -1;
		vec3d rmin;
		double L2min;
		// find the closest node on the surface
		for (int i = 0; i < mesh.Faces(); ++i)
		{
			FEFace& f = mesh.Face(i);
			vec3d ri(0,0,0);
			for (int j = 0; j < f.Nodes(); ++j)
			{
				FENode& nj = mesh.Node(f.n[j]);
				vec3d rj = fem.NodePosition(f.n[j], 0);
				ri += rj;
			}
			ri /= f.Nodes();

			// get the distance
			double L2 = (ri - r0).SqrLength();

			if ((nmin == -1) || (L2 < L2min))
			{
				nmin = i;
				L2min = L2;
				rmin = ri;
			}
		}
		if (nmin == -1) return r0;

		m_closestFace = nmin;

		vec3d dr = r0 - rmin;

		FEFace& f = mesh.Face(nmin);
		vec3d a0 = fem.NodePosition(f.n[0], 0);
		vec3d a1 = fem.NodePosition(f.n[1], 0);
		vec3d a2 = fem.NodePosition(f.n[2], 0);

		vec3d e1 = a1 - a0; e1.Normalize();
		vec3d e2 = a2 - a0; e2.Normalize();
		vec3d e3 = e1 ^ e2; e3.Normalize();
		e2 = e3 ^ e1; e2.Normalize();

		mat3d QT(				\
			e1.x, e1.y, e1.z,	\
			e2.x, e2.y, e2.z,	\
			e3.x, e3.y, e3.z	\
		);

		m_qr = QT * dr;
	}

	// calculate current position of origin
	FEFace& f = mesh.Face(m_closestFace);
	vec3d ri(0, 0, 0);
	for (int j = 0; j < f.Nodes(); ++j)
	{
		FENode& nj = mesh.Node(f.n[j]);
		vec3d rj = fem.NodePosition(f.n[j], ntime);
		ri += rj;
	}
	ri /= f.Nodes();

	vec3d a0 = fem.NodePosition(f.n[0], ntime);
	vec3d a1 = fem.NodePosition(f.n[1], ntime);
	vec3d a2 = fem.NodePosition(f.n[2], ntime);

	vec3d e1 = a1 - a0; e1.Normalize();
	vec3d e2 = a2 - a0; e2.Normalize();
	vec3d e3 = e1 ^ e2; e3.Normalize();
	e2 = e3 ^ e1; e2.Normalize();

	mat3d Q(\
		e1.x, e2.x, e3.x, \
		e1.y, e2.y, e3.y, \
		e1.z, e2.z, e3.z	\
	);

	vec3d dr = Q * m_qr;

	return ri + dr;
}
