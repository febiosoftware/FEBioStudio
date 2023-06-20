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
#include <MeshLib/FENodeNodeList.h>
#include <MeshLib/triangulate.h>
#include <MeshTools/FESelection.h>
#include <GLLib/glx.h>
#include <sstream>
using namespace Post;

class GLMusclePath::PathData
{
public:
	struct Data
	{
		double	pathLength = 0.0;
		vec3d	r0;	// start point
		vec3d	r1;	// end point
		vec3d	rd;	// departure point
		vec3d	tng;	// tangent at departure point (towards start point)

		void Zero()
		{
			pathLength = 0.0;
			r0 = vec3d(0, 0, 0);
			r1 = vec3d(0, 0, 0);
			rd = vec3d(0, 0, 0);
			tng = vec3d(0, 0, 0);
		}
	};

	struct Point
	{
		vec3d	r;
		int		tag;
		int		mat;	// material tag of part this point is on
	};

public:
	PathData() {}
	~PathData() {}

	void push_back(const vec3d& r, int tag = 0, int mat = -1)
	{
		Point p = { r, tag, mat };
		m_points.push_back(p);
	}

	std::vector<vec3d> GetPoints() const
	{
		std::vector<vec3d> p(m_points.size());
		for (size_t i = 0; i < m_points.size(); ++i) p[i] = m_points[i].r;
		return p;
	}

	void SetPoints(const vector<vec3d>& points)
	{
		m_points.clear();
		for (size_t i = 0; i < points.size(); ++i) push_back(points[i]);
	}

	size_t Points() const { return m_points.size(); }

	Point& EndPoint() { return m_points[Points() - 1]; }

public:
	std::vector<Point>	m_points;		// points defining the path

	Data m_data;

private:
	PathData(const PathData& path) {}
};

REGISTER_CLASS(GLMusclePath, CLASS_PLOT, "muscle-path", 0);

static int n = 1;
GLMusclePath::GLMusclePath()
{
	SetTypeString("muscle-path");

	m_node0 = 0;
	m_node1 = 0;
	m_method = 0;
	m_ndiv = 20;
	m_maxIter = 30;
	m_tol = 1e-6;
	m_persist = false;
	m_searchRadius = 0.0;
	m_normalTol = -0.1;

	m_selectedPoint = -1;

	m_part[0] = m_part[1] = -1;

	AddIntParam(m_node0, "point0", "start point");
	AddIntParam(m_node1, "point1", "end point");
	AddChoiceParam(m_method, "method", "Shortest path method")->SetEnumNames("Straight line\0Wrapping path\0");
	AddBoolParam(m_persist, "persist");
	AddIntParam(m_ndiv, "divisions", "Subdivisions");
	AddIntParam(m_maxIter, "max_iters", "Max smoothness iters.")->SetIntRange(1, 100);
	AddDoubleParam(m_tol, "tol", "Smoothness tol.");
	AddDoubleParam(m_searchRadius, "search_radius", "Search radius");
	AddDoubleParam(0.1, "selection_radius", "Selection radius");
	AddDoubleParam(m_normalTol, "normal_tol", "Normal tolerance");
	AddDoubleParam(5.0, "size", "Path radius");
	AddColorParam(GLColor(255, 0, 0), "color");
	AddColorParam(GLColor(164, 0, 164), "color0", "start point color");
	AddColorParam(GLColor(164, 164, 0), "color1", "end point color");
	AddChoiceParam(0, "render_mode", "Render mode")->SetEnumNames("detailed\0path-only\0");

	std::stringstream ss;
	ss << "MusclePath" << n++;
	SetName(ss.str());

	m_closestFace = -1;
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

	double R = GetFloatValue(PATH_RADIUS);
	GLColor c = GetColorValue(COLOR);
	GLColor col0 = GetColorValue(COLOR0);
	GLColor col1 = GetColorValue(COLOR1);

	PathData* path = m_path[nstate];
	if (path == nullptr)
	{
		int n0 = GetIntValue(START_POINT) - 1;
		int n1 = GetIntValue(END_POINT) - 1;

		FEPostMesh& mesh = *glm->GetActiveMesh();
		int NN = mesh.Nodes();
		if ((n0 < 0) || (n0 >= NN)) return;
		if ((n1 < 0) || (n1 >= NN)) return;

		vec3d r0 = mesh.Node(n0).pos();
		vec3d r1 = mesh.Node(n1).pos();

		std::vector<vec3d> points = { r0, r1 };

		float r = (float)c.r;
		float g = (float)c.g;
		float b = (float)c.b;
		float a = (r + g + b)/3.f;

		float w = 0.9f;
		r = r * (1.f - w) + w * a;
		g = g * (1.f - w) + w * a;
		b = b * (1.f - w) + w * a;

		GLColor gray((Byte)r, (Byte)g, (Byte)b);

		// draw the muscle path
		glColor3ub(gray.r, gray.g, gray.b);
		glx::drawSmoothPath(points, R);

		// draw the end points
		glx::glcolor(col0);
		glx::drawSphere(r0, 1.5 * R);

		glx::glcolor(col1);
		glx::drawSphere(r1, 1.5 * R);

		return;
	}

	vector<vec3d> points = path->GetPoints();

	int renderMode = GetIntValue(RENDER_MODE);

	// draw the path
	int N = (int) path->m_points.size();
	if (N > 1)
	{
		vec3d r0 = path->m_points[0].r;
		vec3d r1 = path->m_points[N - 1].r;

		// draw the muscle path
		glColor3ub(c.r, c.g, c.b);
		glx::drawSmoothPath(points, R);

		// draw the end points
		glx::glcolor(col0);
		glx::drawSphere(r0, 1.5 * R);

		glx::glcolor(col1);
		glx::drawSphere(r1, 1.5 * R);

		if (renderMode == 0)
		{
			for (int i = 0; i < N; ++i)
			{
				int ntag = path->m_points[i].tag;
				float sphereRadius = 1.5f * R;
				switch (ntag)
				{
				case 0: glColor3ub(0, 128, 0); break;
				case 1: glColor3ub(0, 255, 0); break;
				case 2: glColor3ub(255, 255, 0); break;
				default:
					glColor3ub(0, 0, 0);
				}
				vec3d r0 = path->m_points[i].r;

				if (i == m_selectedPoint)
				{
					sphereRadius = 2.0 * R;
					glColor3ub(255, 255, 255);
				}

				glx::drawSphere(r0, sphereRadius);
			}
		}
	}
}

bool GLMusclePath::Intersects(Ray& ray, Intersection& q)
{
	int ntime = GetModel()->CurrentTimeIndex();
	if (ntime != 0) return false;

	// see if any of the spheres are close to the ray
	PathData& p = *m_path[ntime];

	double R = GetFloatValue(PATH_RADIUS);

	for (int i = 0; i < p.Points(); ++i)
	{
		PathData::Point& pt = p.m_points[i];

		double l = ((pt.r - ray.origin) * ray.direction) / ray.direction.norm2();
		if (l > 0)
		{
			vec3d r = ray.origin + ray.direction * l;
			double R2 = (pt.r - r).norm2();
			if (R2 < R * R)
			{
				q.point = r;
				q.m_index = i;
				return true;
			}
		}
	}

	return false;
}

class GLMusclePointSelection : public FESelection
{
public:
	GLMusclePointSelection(GLMusclePath::PathData* path, int pointIndex, double R, double softRadius) : FESelection(SELECT_OBJECTS), m_path(path), m_index(pointIndex), m_R(R), m_softRadius(softRadius)
	{
		Update();
		m_L = path->m_data.pathLength;
		if (m_softRadius <= 0.0) m_softRadius = 1.0;
	}

public:
	void Invert() {}
	void Translate(vec3d dr) 
	{
		vec3d p = m_path->m_points[m_index].r;
		for (int i = 1; i < m_path->Points() - 1; ++i)
		{
			vec3d& r = m_path->m_points[i].r;
			double l2 = (p - r).norm2() / (m_L*m_L);
			double s = exp(-l2 / (m_softRadius* m_softRadius));
			r += dr * s;
		}
		Update();
	}

	void Rotate(quatd q, vec3d c) {}
	void Scale(double s, vec3d dr, vec3d c) {}

	quatd GetOrientation() { return m_rot; }
	
	FEItemListBuilder* CreateItemList() { return nullptr; }

protected:
	void Update()
	{
		vec3d r = m_path->m_points[m_index].r;
		vec3d a = m_path->m_points[m_index - 1].r;
		vec3d b = m_path->m_points[m_index + 1].r;
		vec3d t = b - a; t.Normalize();

		m_rot = quatd(vec3d(1, 0, 0), t);

		m_box = BOX(r, r); m_box.Inflate(m_R);
	}

	int Count() { return 1; }

private:
	GLMusclePath::PathData* m_path = nullptr;
	int		m_index = -1;
	quatd	m_rot;
	double	m_R;
	double	m_L;
	double	m_softRadius = 1;
};

FESelection* GLMusclePath::SelectComponent(int index)
{
	int ntime = GetModel()->CurrentTimeIndex();
	if (ntime != 0) return nullptr;

	// see if any of the spheres are close to the ray
	PathData& p = *m_path[ntime];
	if ((index == 0) || (index == (p.Points() - 1))) return nullptr;

	m_selectedPoint = index;
	return new GLMusclePointSelection(&p, index, GetFloatValue(PATH_RADIUS), GetFloatValue(SELECTION_RADIUS));
}

void GLMusclePath::ClearSelection()
{
	if (m_selectedPoint >= 0)
	{ 
		m_selectedPoint = -1;
		UpdateWrappingPath(m_path[0], 0, false);
	}
}

void GLMusclePath::Update()
{
	Update(GetModel()->CurrentTimeIndex(), 0.f, false);
}

// helper function for finding the material ID from a surface node
int GetMaterialFromNode(FSMesh& mesh, int node)
{
	int matId = -1;
	int NF = mesh.Faces();
	for (int i = 0; i < NF; ++i)
	{
		FSFace& face = mesh.Face(i);
		if (face.HasNode(node))
		{
			if (face.m_elem[0].eid >= 0)
			{
				FSElement& el = mesh.Element(face.m_elem[0].eid);
				matId = el.m_MatID;
				break;
			}
		}
	}
	return matId;
}

void GLMusclePath::Reset()
{
	CGLModel* glm = GetModel();
	Post::FEPostModel& fem = *glm->GetFSModel();

	m_closestFace = -1;

	// clear current path data
	ClearPaths();

	// allocate new path data
	m_path.assign(fem.GetStates(), nullptr);

	int n0 = GetIntValue(START_POINT) - 1;
	int n1 = GetIntValue(END_POINT) - 1;
	if ((n0 < 0) || (n1 < 0)) return;

	// find the materials of start and end point
	// (we use this to decide which point is the departure point)
	FSMesh& mesh = *fem.GetState(0)->GetFEMesh();
	m_part[0] = GetMaterialFromNode(mesh, n0);
	m_part[1] = GetMaterialFromNode(mesh, n1);
	assert((m_part[0] >= 0) && (m_part[1] >= 0));
}

void GLMusclePath::SwapEndPoints()
{
	int n0 = GetIntValue(START_POINT);
	int n1 = GetIntValue(END_POINT);
	SetIntValue(START_POINT, n1);
	SetIntValue(END_POINT, n0);
	Reset();
	Update();
}

void GLMusclePath::Update(int ntime, float dt, bool breset)
{
	CGLModel* glm = GetModel();
	Post::FEPostModel& fem = *glm->GetFSModel();

	// make sure we have valid start and end points
	int n0 = GetIntValue(START_POINT) - 1;
	int n1 = GetIntValue(END_POINT) - 1;
	if ((n0 < 0) || (n1 < 0)) return;

	if (breset) Reset();

	if ((ntime < 0) || (ntime >= m_path.size())) return;

	// If we already calculated the path for this time step, we're done
	if (m_path[ntime] != nullptr) return;

	UpdatePath(ntime);
}

void GLMusclePath::UpdatePath(int ntime)
{
	CGLModel* glm = GetModel();
	Post::FEPostModel& fem = *glm->GetFSModel();
	FEPostMesh& mesh = *glm->GetActiveMesh();

	int n0 = GetIntValue(START_POINT) - 1;
	int n1 = GetIntValue(END_POINT) - 1;

	int NN = mesh.Nodes();
	if ((n0 < 0) || (n0 >= NN)) return;
	if ((n1 < 0) || (n1 >= NN)) return;

	vec3d r0 = to_vec3d(fem.NodePosition(n0, ntime));
	vec3d r1 = to_vec3d(fem.NodePosition(n1, ntime));

	PathData* path = new PathData;

	// see which method we're going to use
	int method = GetIntValue(METHOD);
	bool b = false;
	switch (method)
	{
	case 0: b = UpdateStraightPath(path, ntime); break;
	case 1: b = UpdateWrappingPath(path, ntime); break;
	}
	if (b == false) { delete path; path = nullptr; }

	// All is well, so assign the new path
	m_path[ntime] = path;

	// also update the path data
	UpdatePathData(ntime);
}

bool GLMusclePath::UpdateStraightPath(GLMusclePath::PathData* path, int ntime)
{
	CGLModel* glm = GetModel();
	Post::FEPostModel& fem = *glm->GetFSModel();

	int n0 = GetIntValue(START_POINT) - 1;
	int n1 = GetIntValue(END_POINT) - 1;

	vec3d r0 = to_vec3d(fem.NodePosition(n0, ntime));
	vec3d r1 = to_vec3d(fem.NodePosition(n1, ntime));

	path->m_points.clear();
	path->push_back(r0, 0, m_part[0]);
	path->push_back(r1, 0, m_part[1]);

	return true;
}

void GLMusclePath::UpdatePathData(int ntime)
{
	if ((ntime < 0) || (ntime >= m_path.size())) return;

	PathData* path = m_path[ntime];
	if (path == nullptr) return;

	// get the path's points
	if (path->m_points.empty())
	{
		path->m_data.Zero();
		return;
	}

	int n = path->Points();
	if (n >= 2)
	{
		// start and end point coordinates
		vec3d r0 = path->m_points[0].r;
		vec3d r1 = path->m_points[n - 1].r;
		path->m_data.r0 = r0;
		path->m_data.r1 = r1;

		// find the first point in contact with the end-point's object
		for (int i = 0; i < n; ++i)
		{
			PathData::Point& pt = path->m_points[i];
			if (pt.tag == 2)
			{
				path->m_data.rd = pt.r;

				if (i > 0)
				{
					PathData::Point& pp = path->m_points[i - 1];
					vec3d t = pp.r - pt.r; t.Normalize();
					path->m_data.tng = t;
				}
				break;
			}
		}
	}

	// calculate path length
	double L = 0.0;
	for (int i = 0; i < path->Points() - 1; ++i)
	{
		vec3d& r0 = path->m_points[i].r;
		vec3d& r1 = path->m_points[i + 1].r;
		L += (r1 - r0).Length();
	}
	path->m_data.pathLength = L;
}

bool GLMusclePath::UpdateData(bool bsave)
{
	if (bsave)
	{
		bool reset = false;

		int node0     = GetIntValue(START_POINT     ); if (node0  != m_node0        ) { m_node0        =   node0; reset = true; }
		int node1     = GetIntValue(END_POINT       ); if (node1  != m_node1        ) { m_node1        =   node1; reset = true; }
		int method    = GetIntValue(METHOD          ); if (method  != m_method      ) { m_method       =  method; reset = true; }
		bool persist  = GetBoolValue(PERSIST_PATH   ); if (persist != m_persist     ) { m_persist      = persist; reset = true; }
		int ndiv      = GetIntValue(SUBDIVISIONS    ); if (ndiv    != m_ndiv        ) { m_ndiv         =    ndiv; reset = true; }
		int maxIter   = GetIntValue(MAX_SMOOTH_ITERS); if (maxIter != m_maxIter     ) { m_maxIter      = maxIter; reset = true; }
		double tol    = GetFloatValue(SMOOTH_TOL    ); if (tol     != m_tol         ) { m_tol          =     tol; reset = true; }
		double radius = GetFloatValue(SEARCH_RADIUS ); if (radius  != m_searchRadius) { m_searchRadius =  radius; reset = true; }
		double nrmtol = GetFloatValue(NORMAL_TOL    ); if (nrmtol  != m_normalTol   ) { m_normalTol    =  nrmtol; reset = true; }
		
		Update(GetModel()->CurrentTimeIndex(), 0.f, reset);
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
	case 1: val = path->m_data.pathLength; break;
	case 2: val = path->m_data.r0.x; break;
	case 3: val = path->m_data.r0.y; break;
	case 4: val = path->m_data.r0.z; break;
	case 5: val = path->m_data.r1.x; break;
	case 6: val = path->m_data.r1.y; break;
	case 7: val = path->m_data.r1.z; break;
	case 8: val = path->m_data.rd.x; break;
	case 9: val = path->m_data.rd.y; break;
	case 10: val = path->m_data.rd.z; break;
	case 11: val = path->m_data.tng.x; break;
	case 12: val = path->m_data.tng.y; break;
	case 13: val = path->m_data.tng.z; break;
	default:
		assert(false);
	}

	// return 
	return val;
}

class RINGPOINT
{
public:
	vec3d	p;	// point on ring
	vec3d	n;	// local surface normal
	int		nface;	// face index

public:
	RINGPOINT() : nface(-1) {}
	RINGPOINT(const vec3d r) : p(r), nface(-1) {}
};

class FaceMesh
{
public:
	struct FACE
	{
		vec3d	r[3];
		vec3d	fn;
		int		mat;
	};

	size_t Faces() const { return m_Face.size(); }

	FACE& Face(size_t n) { return m_Face[n]; }

	size_t FindFace(const vec3d r)
	{
		for (size_t i = 0; i < m_Face.size(); ++i)
		{
			FACE& f = m_Face[i];
			if ((f.r[0] - r).SqrLength() < 1e-12) return i;
			if ((f.r[1] - r).SqrLength() < 1e-12) return i;
			if ((f.r[2] - r).SqrLength() < 1e-12) return i;
		}
		return -1;
	}

public:
	vector<FACE>	m_Face;
};

bool ClosestPointOnRing(FaceMesh& mesh, const vec3d& rc, const vec3d& t, const vec3d& a, const vec3d& b, const vec3d& na, RINGPOINT& pt, double normalTolerance)
{
	int NF = mesh.Faces();
	int imin = -1;
	double Dmin = 0.0;
	for (int i = 0; i < NF; ++i)
	{
		// figure out the case for this face
		// (i.e. decide if the plane (rc, t) intersects this triangle
		FaceMesh::FACE& face = mesh.Face(i);
		int ne = 3; // edges!
		double s[FSFace::MAX_NODES] = { 0 };
		int ncase = 0;
		vec3d* re = face.r;
		for (int j = 0; j < ne; ++j)
		{
			s[j] = (re[j] - rc) * t;
			if (s[j] > 0) ncase |= (1 << j);
		}

		const int LUT[8][2] = { {-1,-1}, {0,2},{0,1},{1,2},{1,2},{0,1},{0,2},{-1,-1} };

		if ((ncase != 0) && (ncase != 7))
		{
			int e[2] = { LUT[ncase][0], LUT[ncase][1] };

			// find the edge intersections
			vec3d er[2];
			for (int j = 0; j < 2; ++j)
			{
				int n = e[j];
				int n0 = n;
				int n1 = (n + 1) % ne;
				vec3d ra = re[n0];
				vec3d rb = re[n1];
				double la = t * (ra - rc);
				double lb = t * (rb - rc);

				assert(la * lb < 0);

				double l = (t * (rc - ra)) / (t*(rb - ra));
				vec3d p = ra + (rb - ra) * l;

				// make sure the point lies on the plane
				double e = t * (p - rc);
				assert(fabs(e) < 1e-12);

				er[j] = p;
			}

			// find the point that minimizes (a-p-b)
			vec3d dr = er[1] - er[0];
			double D = dr.SqrLength();
			if (D != 0.0)
			{
				// project point c onto the line {er[0], er[1]}
				vec3d c = (a + b) * 0.5;
				double l = (c * dr - er[0] * dr) / D;

				vec3d p;
				if (l <= 0.0) p = er[0];
				else if (l >= 1.0) p = er[1];
				else p = er[0] + (er[1] - er[0]) * l;

				double D2 = (p - a).SqrLength() + (p - b).SqrLength();
				if ((imin == -1) || (D2 < Dmin))
				{
					// calculate face normal
					vec3d fn = face.fn;

					// make sure the normal is not on the wrong side
					double dot = fn * na;
					if (dot > normalTolerance)
					{
						imin = i;
						Dmin = D2;
						pt.p = p;

						// make sure the point lies on the plane
						double e = t * (pt.p - rc);
						assert(fabs(e) < 1e-12);

						// project normal onto plane
						fn -= t * (fn * t); fn.Normalize();

						assert(fabs(fn * t) < 1e-12);

						pt.n = fn;

						pt.nface = i;
					}
				}
			}
		}
	}

	return (imin != -1);
}

void ProcessPath(vector<RINGPOINT>& pt)
{
	for (size_t i = 1; i < pt.size() - 1; ++i)
	{
		RINGPOINT& rm = pt[i - 1];
		RINGPOINT& ri = pt[i    ];
		RINGPOINT& rp = pt[i + 1];

		vec3d e1 = ri.p - rm.p;
		vec3d e2 = rp.p - ri.p;

		vec3d t = (e2 - e1); t.Normalize();
		if (t * ri.n >= 0.0)
		{
			// remove this point
			pt.erase(pt.begin() + i);
			i--;
		}
	}
}

void StraightenPath(vector<RINGPOINT>& pt)
{
	const int N = pt.size();
	if (N <= 2) return;

	int n = 1;
	while (n < N - 1)
	{
		RINGPOINT& pn = pt[n];
		if (pn.nface == -1)
		{
			int m0 = n - 1; assert(m0 >= 0);
			int m1 = n + 1;
			while ((m1 < N - 1) && (pt[m1].nface == -1)) m1++;
			assert(m1 < N);

			vec3d a = pt[m0].p;
			vec3d b = pt[m1].p;
			for (; n < m1; n++)
			{
				double l = (double)(n - m0) / (double)(m1 - m0);
				vec3d r = a + (b - a) * l;
				pt[n].p = r;
			}
		}
		else n++;
	}
}

bool SmoothenPath(FaceMesh& mesh, vector<RINGPOINT>& pt, int maxIters, double tol, double normalTolerance)
{
	// evaluate the initial length
	int NP = pt.size();
	double L0 = 0;
	for (int i = 0; i < NP - 1; ++i)
	{
		RINGPOINT& ri = pt[i];
		RINGPOINT& rp = pt[i + 1];
		L0 += (rp.p - ri.p).Length();
	}

	// see if we can shrink the path
	int niter = 0;
	bool done = false;
	double Lp = L0;
	do
	{
		for (int i = 1; i < pt.size() - 1; ++i)
		{
			// next point
			RINGPOINT& pi = pt[i];

			// approximate tangent
			vec3d a = pt[i - 1].p;
			vec3d b = pt[i + 1].p;
			vec3d t = b - a; t.Normalize();

			vec3d ri = (a + b) * 0.5;

			// find the closest point to ri on the ring, defined by the intersection
			// of the mesh with the plane (ri; t)
			if (ClosestPointOnRing(mesh, ri, t, a, b, pt[i-1].n, pi, normalTolerance))
			{
				if ((ri - pi.p) * pi.n > 0.0)
				{
					pi.p = ri;
					pi.nface = -1;
				}
			}
		}

		StraightenPath(pt);

		// calculate new length
		double L1 = 0;
		for (int i = 0; i < NP - 1; ++i)
		{
			RINGPOINT& ri = pt[i];
			RINGPOINT& rp = pt[i + 1];
			L1 += (rp.p - ri.p).Length();
		}

		done = (fabs((L1 - Lp) / L0) < tol);
		Lp = L1;
		niter++;
		if (niter > maxIters) break;
	} 
	while (!done);

	return done;
}

bool GLMusclePath::UpdateWrappingPath(PathData* path, int ntime, bool reset)
{
	CGLModel* glm = GetModel();
	Post::FEPostModel& fem = *glm->GetFSModel();
	FEPostMesh& mesh = *glm->GetActiveMesh();

	int n0 = GetIntValue(START_POINT) - 1;
	int n1 = GetIntValue(END_POINT) - 1;

	double normalTol = GetFloatValue(NORMAL_TOL);

	vec3d r0 = to_vec3d(fem.NodePosition(n0, ntime));
	vec3d r1 = to_vec3d(fem.NodePosition(n1, ntime));

	vec3d t = r1 - r0; t.Normalize();

	// let's tag the nodes and faces that are within the search radius
	// (if search radius == 0, all faces will be tagged)
	double R = GetFloatValue(SEARCH_RADIUS);
	double R2 = R * R;
	int NF = mesh.Faces();
	int faces = 0;
	if (R > 0)
	{
		// first identify the nodes that are within the search radius
		for (int i = 0; i < mesh.Nodes(); ++i)
		{
			FSNode& node = mesh.Node(i);
			node.m_ntag = 0;
			vec3d ri = to_vec3d(fem.NodePosition(i, ntime));
			double L0 = (ri - r0).SqrLength();
			double L1 = (ri - r1).SqrLength();
			if ((L0 < R2) || (L1 < R2)) node.m_ntag = 1;
			else
			{
				vec3d p = r0 + t * ((ri - r0) * t);
				double L2 = (p - ri).SqrLength();
				if (L2 < R2) node.m_ntag = 1;
			}
		}

		// now tag faces that contain tagged nodes
		faces = 0;
		for (int i = 0; i < NF; ++i)
		{
			FSFace& face = mesh.Face(i);
			int nn = face.Edges();
			face.m_ntag = 0;
			for (int j = 0; j < nn; ++j)
			{
				if (mesh.Node(face.n[j]).m_ntag != 0)
				{
					face.m_ntag = 1;
					faces++;
					break;
				}
			}
		}
	}
	else 
	{
		mesh.TagAllFaces(1); faces = NF;
	}

	// build the face mesh
	FaceMesh faceMesh;
	faceMesh.m_Face.resize(NF);
	int n = 0;
	for (int i = 0; i < NF; ++i)
	{
		FSFace& fs = mesh.Face(i);
		if (fs.m_ntag == 1)
		{
			FaceMesh::FACE& fd = faceMesh.Face(n++);
			int ne = 3; // edges!
			for (int j = 0; j < ne; ++j)
			{
				fd.r[j] = to_vec3d(fem.NodePosition(fs.n[j], ntime));
			}

			if (fs.m_elem[0].eid >= 0)
			{
				FSElement& el = mesh.Element(fs.m_elem[0].eid);
				fd.mat = el.m_MatID;
			}

			vec3d e1 = fd.r[1] - fd.r[0];
			vec3d e2 = fd.r[2] - fd.r[0];
			fd.fn = e1 ^ e2; fd.fn.Normalize();
		}
	}
	assert(n == faces);

	// the path
	vector<RINGPOINT> pt;

	if (reset)
	{
		bool persist = GetBoolValue(PERSIST_PATH);
		if ((ntime == 0) || (persist == false))
		{
			// create initial (straight) path
			RINGPOINT startPoint(r0);

			// we need to find the face (and normal) of the initial point
			int nface = faceMesh.FindFace(r0); assert(nface >= 0);
			if (nface >= 0)
			{
				startPoint.nface = nface;
				startPoint.n = faceMesh.Face(nface).fn;
			}
			pt.push_back(startPoint);

			// do the rest of the points
			const int STEPS = GetIntValue(SUBDIVISIONS);
			for (int i = 1; i < STEPS; ++i, ++n)
			{
				RINGPOINT rp;
				double w = (double)i / (double)STEPS;
				rp.p = r0 + (r1 - r0) * w;
				pt.push_back(rp);
			}
			RINGPOINT endPoint(r1);
			pt.push_back(endPoint);
		}
		else
		{
			// get the previous path
			PathData* prevPath = m_path[ntime - 1];
			if (prevPath == nullptr) return false;

			// we'll use this path as an initial guess
			vector<vec3d> prevPt = prevPath->GetPoints();
			for (int i = 0; i < prevPt.size(); ++i) pt.push_back(RINGPOINT(prevPt[i]));

			// we do update the first and last point
			pt[0].p = r0;
			pt[pt.size() - 1].p = r1;
		}
	}
	else
	{
		// get the current path
		PathData* prevPath = m_path[ntime];
		if (prevPath == nullptr) return false;

		// we'll use this path as an initial guess
		vector<vec3d> prevPt = prevPath->GetPoints();
		for (int i = 0; i < prevPt.size(); ++i) pt.push_back(RINGPOINT(prevPt[i]));

		// we do update the first and last point
		// we need to find the face (and normal) of the initial point
		int nface = faceMesh.FindFace(r0); assert(nface >= 0);
		pt[0].p = r0;
		if (nface >= 0)
		{
			pt[0].nface = nface;
			pt[0].n = faceMesh.Face(nface).fn;
		}
		pt[pt.size() - 1].p = r1;
	}

	// process the initial path
	for (int i = 1; i < pt.size()-1; ++i)
	{
		// next point
		RINGPOINT& pi = pt[i];

		// approximate tangent
		vec3d a = pt[i - 1].p;
		vec3d b = pt[i + 1].p;
		vec3d t = b - a; t.Normalize();

		vec3d ri = (a + b) * 0.5;

		if (ClosestPointOnRing(faceMesh, ri, t, a, b, pt[i-1].n, pi, normalTol))
		{
			if ((ri - pi.p) * pi.n > 0.0)
			{
				pi.p = ri;
				pi.nface = -1;
			}
		}
	}

	// smoothen the path
	int maxIters = GetIntValue(MAX_SMOOTH_ITERS);
	if (maxIters > 0)
	{
		double tol = GetFloatValue(SMOOTH_TOL);
		SmoothenPath(faceMesh, pt, maxIters, tol, normalTol);
	}

	// copy the points to the PathData
	path->m_points.clear();
	for (size_t i = 0; i < pt.size(); ++i)
	{
		int nface = pt[i].nface;
		int mat = -1;
		if (nface != -1)
		{
			mat = faceMesh.Face(nface).mat;
		}
		path->push_back(pt[i].p, (nface == -1 ? 0 : 1), mat);
	}

	// tag departure point
	int mat = m_part[1];	// get material at end-point

	// make sure the end point has its mat tag set
	path->EndPoint().mat = mat;

	// the first point in contact with this material is the departure point
	for (int i = 0; i < path->Points(); ++i)
	{
		PathData::Point& pt = path->m_points[i];
		if (pt.mat == mat)
		{
			pt.tag = 2;
			break;
		}
	}

	// all done
	return true;
}
