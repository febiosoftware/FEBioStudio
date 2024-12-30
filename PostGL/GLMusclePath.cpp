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
#include <MeshTools/FSTriMesh.h>
#include <MeshTools/FEGeodesic.h>
#include <GLLib/glx.h>
#include <FSCore/ClassDescriptor.h>
#include "../FEBioStudio/GLRenderEngine.h"
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

		// projection onto guided surface
		int	nproj;
		vec2d q;
	};

public:
	PathData() {}
	~PathData() {}

	void push_back(const vec3d& r, int tag = 0, int mat = -1)
	{
		Point p = { r, tag, -1, vec2d(0,0)};
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
	Point& operator[] (size_t i) { return m_points[i]; }

public:
	std::vector<Point>	m_points;		// points defining the path

	Data m_data;
};

REGISTER_CLASS(GLMusclePath, CLASS_PLOT, "muscle-path", 0);

static int ncount = 1;
GLMusclePath::GLMusclePath()
{
	SetTypeString("muscle-path");

	m_node0 = 0;
	m_node1 = 0;
	m_ndiv = 20;
	m_maxIter = 100;
	m_tol = 1e-6;
	m_searchRadius = 0.0;
	m_snaptol = 1.5;

	m_selectedPoint = -1;
	m_selectionRadius = 0.2;
	m_pathGuide = 0;

	m_part[0] = m_part[1] = -1;

	AddIntParam(m_node0, "point0", "start point");
	AddIntParam(m_node1, "point1", "end point");
	AddIntParam(m_ndiv, "divisions", "Subdivisions");
	AddIntParam(m_maxIter, "max_iters", "Max smoothness iters.")->SetIntRange(0, 200);
	AddDoubleParam(m_tol, "tol", "Smoothness tol.");
	AddDoubleParam(m_snaptol, "snap_tol", "Snap tolerance");
	AddDoubleParam(m_searchRadius, "search_radius", "Search radius");
	AddChoiceParam(m_pathGuide, "path_guide", "Path guide")->SetEnumNames("(none)\0");
	AddDoubleParam(5.0, "size", "Path radius");
	AddColorParam(GLColor(255, 0, 0), "color");
	AddColorParam(GLColor(164, 0, 164), "color0", "start point color");
	AddColorParam(GLColor(164, 164, 0), "color1", "end point color");
	AddChoiceParam(0, "render_mode", "Render mode")->SetEnumNames("detailed\0less detailed\0path-only\0");

	std::stringstream ss;
	ss << "MusclePath" << ncount++;
	SetName(ss.str());

	m_closestFace = -1;

	m_initPath = nullptr;
}

GLMusclePath::~GLMusclePath()
{
	ClearPaths();
	ClearInitPath();

}

void GLMusclePath::SetModel(CGLModel* pm)
{
	CGLPlot::SetModel(pm);
	
	// get the list of materials to use for the guide
	Post::FEPostModel* fem = pm->GetFSModel();
	char* szbuf = new char[1024]; szbuf[0] = 0;
	char* sz = szbuf;
	sprintf(sz, "(none)"); sz += 6; *sz++ = 0;
	for (int i=0; i<fem->Materials(); ++i)
	{ 
		Post::Material* gm = fem->GetMaterial(i);
		string name = gm->GetName();
		const char* sn = name.c_str();
		int n = strlen(sn);
		sprintf(sz, "%s", sn); sz += n; *sz++ = 0;
	}
	*sz = 0;

	Param& p = GetParam(PATH_GUIDE);
	p.CopyEnumNames(szbuf);

	delete[] szbuf;
}

bool GLMusclePath::OverrideInitPath() const
{
	return (m_initPath && m_initPath->Points());
}

std::vector<vec3d> GLMusclePath::GetInitPath() const
{
	std::vector<vec3d> p;
	if (m_initPath) p = m_initPath->GetPoints();
	return p;
}

void GLMusclePath::SetInitPath(const std::vector<vec3d>& path)
{
	UpdateData(true);
	Reset();
	m_initPath = new PathData();
	m_initPath->SetPoints(path);
	Update();
}

void GLMusclePath::ClearInitPath()
{
	if (m_initPath) delete m_initPath;
	m_initPath = nullptr;
}

void GLMusclePath::ClearPaths()
{
	for (int i = 0; i < m_path.size(); ++i)
	{
		delete m_path[i];
	}
	m_path.clear();
}

void GLMusclePath::Render(GLRenderEngine& re, CGLContext& rc)
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

		GLColor gray((uint8_t)r, (uint8_t)g, (uint8_t)b);

		// draw the muscle path
		glColor3ub(gray.r, gray.g, gray.b);
		glx::drawSmoothPath(re, points, R);

		// draw the end points
		re.setColor(col0);
		glx::drawSphere(re, r0, 1.5 * R);

		re.setColor(col1);
		glx::drawSphere(re, r1, 1.5 * R);

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
		glx::drawSmoothPath(re, points, R);

		if ((renderMode == 0) || (renderMode == 1))
		{
			// draw the end points
			re.setColor(col0);
			glx::drawSphere(re, r0, 1.5 * R);

			re.setColor(col1);
			glx::drawSphere(re, r1, 1.5 * R);

			for (int i = 0; i < N; ++i)
			{
				auto& pt = path->m_points[i];
				int ntag = pt.tag;
				if ((ntag == 2) || (renderMode == 0))
				{
					float sphereRadius = 1.5f * R;
					switch (ntag)
					{
					case 0: glColor3ub(0, 128, 0); break;
					case 1: glColor3ub(0, 255, 0); break;
					case 2: glColor3ub(255, 255, 0); break;
					default:
						glColor3ub(0, 0, 0);
					}
					vec3d r0 = pt.r;

					if (i == m_selectedPoint)
					{
						sphereRadius = 2.0 * R;
						glColor3ub(255, 255, 255);
					}

					glx::drawSphere(re, r0, sphereRadius);
				}

				// draw the tangent vector
				if ((ntag == 2) && (renderMode < 2))
				{
					double L = 0.1*path->m_data.pathLength;
					if (L == 0) L = 1;
					vec3d r = pt.r;
					vec3d t = path->m_data.tng;

					re.pushTransform();

					glTranslatef(r.x, r.y, r.z);
					quatd q;
					if (t * vec3d(0, 0, 1) == -1.0) q = quatd(PI, vec3d(1, 0, 0));
					else q = quatd(vec3d(0, 0, 1), t);
					float w = q.GetAngle();
					if (fabs(w) > 1e-6)
					{
						vec3d p = q.GetVector();
						if (p.Length() > 1e-6) glRotated(w * 180 / PI, p.x, p.y, p.z);
						else glRotated(w * 180 / PI, 1, 0, 0);
					}

					double D = 1.25 * R;

					glx::drawCylinder(re, D, L, 20);
					re.translate(vec3d(0, 0, L * 0.9));
					glx::drawCone(re, 2 * D, 0.5 * L, 20);

					re.popTransform();
				}
			}
		}
	}
}

bool GLMusclePath::Intersects(Ray& ray, Intersection& q)
{
	int ntime = GetModel()->CurrentTimeIndex();
	if (ntime != 0) return false;

	// make sure we have paths
	if (m_path.empty()) return false;

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
	GLMusclePointSelection(GLMusclePath* path, int pointIndex, double R, double softRadius) : FESelection(SELECT_OBJECTS), m_path(path), m_index(pointIndex), m_R(R), m_softRadius(softRadius)
	{
		Update();
		m_L = path->GetPath(0)->m_data.pathLength;
		if (m_softRadius <= 0.0) m_softRadius = 1.0;
	}

public:
	void Invert() {}
	void Translate(vec3d dr) 
	{
		GLMusclePath::PathData* path = m_path->GetPath(0);
		vec3d p = path->m_points[m_index].r;
		for (int i = 1; i < path->Points() - 1; ++i)
		{
			vec3d& r = path->m_points[i].r;
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
		GLMusclePath::PathData* path = m_path->GetPath(0);
		vec3d r = path->m_points[m_index].r;
		vec3d a = path->m_points[m_index - 1].r;
		vec3d b = path->m_points[m_index + 1].r;
		vec3d t = b - a; t.Normalize();

		m_rot = quatd(vec3d(1, 0, 0), t);

		m_box = BOX(r, r); m_box.Inflate(m_R);
	}

	int Count() const { return 1; }

private:
	GLMusclePath* m_path = nullptr;
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
	if (m_path.empty()) return nullptr;

	// see if any of the spheres are close to the ray
	PathData& p = *m_path[ntime];
	if ((index == 0) || (index == (p.Points() - 1))) return nullptr;

	m_selectedPoint = index;
	return new GLMusclePointSelection(this, index, GetFloatValue(PATH_RADIUS), m_selectionRadius);
}

void GLMusclePath::ClearSelection()
{
	if (m_selectedPoint >= 0)
	{ 
		m_selectedPoint = -1;
		ClearInitPath();
		if (m_path.empty() == false)
		{
			m_initPath = new PathData(*m_path[0]);
			UpdatePath(m_path[0], 0, true);
		}
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

	// clear guide mesh
	m_guideMesh.Clear();

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

	// update the path
	bool b = UpdatePath(path, ntime);
	if (b == false) { delete path; path = nullptr; }

	// All is well, so assign the new path
	m_path[ntime] = path;

	// also update the path data
	UpdatePathData(ntime);
}

bool GLMusclePath::UpdatePath(PathData* path, int ntime, bool reset)
{
	if (m_pathGuide == 0) return UpdateWrappingPath(path, ntime, reset);
	else return UpdateGuidedPath(path, ntime, reset);
}

void GLMusclePath::UpdatePathData(int ntime)
{
	if ((ntime < 0) || (ntime >= m_path.size())) return;
	if (m_path.empty()) return;

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
				else
				{
					PathData::Point& pp = path->m_points[1];
					vec3d t = pt.r - pp.r; t.Normalize();
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
		int ndiv      = GetIntValue(SUBDIVISIONS    ); if (ndiv    != m_ndiv        ) { m_ndiv         =    ndiv; reset = true; }
		int maxIter   = GetIntValue(MAX_SMOOTH_ITERS); if (maxIter != m_maxIter     ) { m_maxIter      = maxIter; reset = false; }
		double tol    = GetFloatValue(SMOOTH_TOL    ); if (tol     != m_tol         ) { m_tol          =     tol; reset = false; }
		double radius = GetFloatValue(SEARCH_RADIUS ); if (radius  != m_searchRadius) { m_searchRadius =  radius; reset = false; }
		double snaptol= GetFloatValue(SNAP_TOL      ); if (snaptol != m_snaptol     ) { m_snaptol      =  snaptol; reset = true; }
		int guide     = GetIntValue(PATH_GUIDE      ); if (guide   != m_pathGuide   ) { m_pathGuide    =    guide; reset = true; }

		if (reset) ClearInitPath();
		
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

void BuildFaceMesh(FSTriMesh& faceMesh, Post::FEPostModel& fem, Post::FEPostMesh& mesh, int ntime, double R, vec3d r0, vec3d r1, int partID[2])
{
	vec3d t = r1 - r0; t.Normalize();

	double R2 = R * R;
	int NF = mesh.Faces();
	int faces = 0;
	vector<int> Ntag(mesh.Nodes(), 0);
	vector<int> Ftag(mesh.Faces(), 1);
	if (R > 0)
	{
		// first identify the nodes that are within the search radius
		for (int i = 0; i < mesh.Nodes(); ++i)
		{
			FSNode& node = mesh.Node(i);
			Ntag[i] = 0;
			vec3d ri = to_vec3d(fem.NodePosition(i, ntime));
			double L0 = (ri - r0).SqrLength();
			double L1 = (ri - r1).SqrLength();
			if ((L0 < R2) || (L1 < R2)) Ntag[i] = 1;
			else
			{
				vec3d p = r0 + t * ((ri - r0) * t);
				double L2 = (p - ri).SqrLength();
				if (L2 < R2) Ntag[i] = 1;
			}
		}

		// now tag faces that contain tagged nodes
		faces = 0;
		for (int i = 0; i < NF; ++i)
		{
			FSFace& face = mesh.Face(i);
			int nn = face.Edges();
			Ftag[i] = 0;
			// see if any of the face nodes are tagged
			for (int j = 0; j < nn; ++j)
			{
				if (Ntag[face.n[j]] != 0)
				{
					// only add the face if it belongs to either parts
					// (This was done to avoid that the guiding mesh gets added to this list)
					if (face.m_elem[0].eid >= 0)
					{
						FSElement& el = mesh.Element(face.m_elem[0].eid);
						int mat = el.m_MatID;
						if ((mat == partID[0]) || (mat == partID[1]))
						{
							Ftag[i] = 1;
							faces++;
						}
					}
					break;
				}
			}
		}
	}
	else
	{
		// now tag faces with the correct material IDs
		faces = 0;
		for (int i = 0; i < NF; ++i)
		{
			FSFace& face = mesh.Face(i);
			Ftag[i] = 0;

			// only add the face if it belongs to either parts
			// (This was done to avoid that the guiding mesh gets added to this list)
			if (face.m_elem[0].eid >= 0)
			{
				FSElement& el = mesh.Element(face.m_elem[0].eid);
				int mat = el.m_MatID;
				if ((mat == partID[0]) || (mat == partID[1]))
				{
					Ftag[i] = 1;
					faces++;
				}
			}
		}
	}

	faceMesh.Create(faces);
	int n = 0;
	for (int i = 0; i < NF; ++i)
	{
		FSFace& fs = mesh.Face(i);
		if (Ftag[i] == 1)
		{
			FSTriMesh::FACE& fd = faceMesh.Face(n++);
			int ne = 3; // edges!
			for (int j = 0; j < ne; ++j)
			{
				fd.r[j] = to_vec3d(fem.NodePosition(fs.n[j], ntime));
			}

			if (fs.m_elem[0].eid >= 0)
			{
				FSElement& el = mesh.Element(fs.m_elem[0].eid);
				fd.tag = el.m_MatID;
			}

			vec3d e1 = fd.r[1] - fd.r[0];
			vec3d e2 = fd.r[2] - fd.r[0];
			fd.fn = e1 ^ e2; fd.fn.Normalize();
		}
	}
	assert(n == faces);
}

bool GLMusclePath::UpdateWrappingPath(PathData* path, int ntime, bool reset)
{
	CGLModel* glm = GetModel();
	Post::FEPostModel& fem = *glm->GetFSModel();
	FEPostMesh& mesh = *glm->GetActiveMesh();

	// get the nodal positions of the two end points
	int n0 = GetIntValue(START_POINT) - 1;
	int n1 = GetIntValue(END_POINT) - 1;

	vec3d r0 = to_vec3d(fem.NodePosition(n0, ntime));
	vec3d r1 = to_vec3d(fem.NodePosition(n1, ntime));

	// The face mesh is the mesh of all possible faces that can be intersected. 
	// We don't use the entire mesh for optimization purposes. The search radius
	// determines which faces will be included. 
	// (if search radius == 0, all faces will be included)
	double R = GetFloatValue(SEARCH_RADIUS);
	FSTriMesh faceMesh;
	BuildFaceMesh(faceMesh, fem, mesh, ntime, R, r0, r1, m_part);

	// let's get to work!
	vector<vec3d> pt;
	if (reset)
	{
		if ((ntime == 0) && (m_initPath == nullptr))
		{
			// create initial (straight) path
			const int STEPS = GetIntValue(SUBDIVISIONS);
			pt.push_back(r0);
			for (int i = 1; i < STEPS; ++i)
			{
				double w = (double)i / (double)STEPS;
				vec3d ri = r0 + (r1 - r0) * w;
				pt.push_back(ri);
			}
			pt.push_back(r1);
		}
		else if ((ntime == 0) && m_initPath)
		{
			// we'll use this path as an initial guess
			pt = m_initPath->GetPoints();

			// we do update the first and last point
			pt[0] = r0;
			pt[pt.size() - 1] = r1;
		}
		else
		{
			// get the previous path
			PathData* prevPath = m_path[ntime - 1];
			if (prevPath == nullptr) return false;

			// we'll use this path as an initial guess
			pt = prevPath->GetPoints();

			// we do update the first and last point
			pt[0] = r0;
			pt[pt.size() - 1] = r1;
		}
	}
	else
	{
		// use the current path, but do update the first and last point
		pt = path->GetPoints();
		pt[0] = r0;
		pt[pt.size()-1] = r1;
	}

	// get the parameters
	int maxIters = GetIntValue(MAX_SMOOTH_ITERS);
	double snaptol = GetFloatValue(SNAP_TOL);
	double tol = GetFloatValue(SMOOTH_TOL);

	// we don't use the snap tolerance for the initial time
	if (ntime == 0) snaptol = 100;

	// process the path
	PathOnMesh geo = ProjectToGeodesic(faceMesh, pt, maxIters, tol, snaptol);
	assert(geo.Points() == pt.size());

	// copy the points
	for (int i = 0; i < geo.Points(); ++i) pt[i] = geo[i].r;
	path->SetPoints(pt);

	// tag departure point
	int mat = m_part[1];	// get material at end-point

	// the first point in contact with this material is the departure point
	int depart = -1;
	for (int i = 0; i < path->Points(); ++i)
	{
		auto& pi = geo[i];
		PathData::Point& pt = path->m_points[i];
		pt.tag = (pi.nface == -1 ? 0 : 1);
		if ((depart==-1) && (pi.nface >= 0))
		{
			if (faceMesh.Face(pi.nface).tag == mat) depart = i;
		}
	}
	if (depart != -1) path->m_points[depart].tag = 2;

	// all done
	return true;
}

bool GLMusclePath::UpdateGuidedPath(PathData* path, int ntime, bool reset)
{
	CGLModel* glm = GetModel();
	Post::FEPostModel& fem = *glm->GetFSModel();
	FEPostMesh& mesh = *glm->GetActiveMesh();

	int n0 = GetIntValue(START_POINT) - 1;
	int n1 = GetIntValue(END_POINT) - 1;

	vec3d r0 = to_vec3d(fem.NodePosition(n0, ntime));
	vec3d r1 = to_vec3d(fem.NodePosition(n1, ntime));

	// update the guide mesh
	UpdateGuideMesh(ntime);

	// let's tag the nodes and faces that are within the search radius
	// (if search radius == 0, all faces will be tagged)
	double R = GetFloatValue(SEARCH_RADIUS);

	// build the face mesh
	FSTriMesh faceMesh;
	BuildFaceMesh(faceMesh, fem, mesh, ntime, R, r0, r1, m_part);

	if (reset && (ntime == 0))
	{
		// generate initial straight line
		vector<vec3d> points;
		points.push_back(r0);
		const int STEPS = GetIntValue(SUBDIVISIONS);
		for (int i = 1; i < STEPS; ++i)
		{
			double w = (double)i / (double)STEPS;
			vec3d p = r0 + (r1 - r0) * w;
			points.push_back(p);
		}
		points.push_back(r1);

		// allocate initial path
		if (m_initPath == nullptr) m_initPath = new PathData;
		PathData& initPath = *m_initPath;
		initPath.SetPoints(points);

		// do the rest of the points
		for (int i = 1; i < STEPS; ++i)
		{
			auto& pt = initPath[i];
			Intersection is;
			vec3d p = projectToSurface(m_guideMesh, pt.r, -1, nullptr, &is);
			pt.nproj = is.m_faceIndex;
			pt.q = vec2d(is.r[0], is.r[1]);
		}
	}

	// make sure we have the initial path
	if (m_initPath == nullptr) return false;

	if (path->Points() != m_initPath->Points())
		path->SetPoints(m_initPath->GetPoints());

	// we do update the first and last point
	auto& startPoint = (*path)[0];
	startPoint.r = r0;
	path->EndPoint().r = r1;

	// project other points
	for (int i = 1; i < path->Points() - 1; ++i)
	{
		auto& p0 = (*m_initPath)[i];
		auto& p1 = (*path)[i];

		vec3d r[FSFace::MAX_NODES];
		if (p0.nproj >= 0)
		{
			FSFace& f = m_guideMesh.Face(p0.nproj);
			m_guideMesh.FaceNodePosition(f, r);
			vec3d q = f.eval(r, p0.q.x(), p0.q.y());
			p1.r = q;
		}
	}

	// project all points onto the faceMesh
	// we need to do this to find out which points 
	// are in contact with which part
	// the first point in contact with this material is the departure point
	double D = 1.5*GetFloatValue(PATH_RADIUS);
	int mat = m_part[1];
	int depart = -1;
	for (int i = 0; i < path->Points(); ++i)
	{
		PathData::Point& pt = path->m_points[i];
		int nface = faceMesh.FindFace(pt.r, D);
		pt.tag = (nface == -1 ? 0 : 1);
		if ((depart == -1) && (nface >= 0))
		{
			if (faceMesh.Face(nface).tag == mat) depart = i;
		}
	}
	if (depart != -1) path->m_points[depart].tag = 2;

	// all done
	return true;
}

void GLMusclePath::BuildGuideMesh()
{
	m_guideMesh.Clear();

	CGLModel* glm = GetModel();
	Post::FEPostModel& fem = *glm->GetFSModel();
	FEPostMesh& mesh = *glm->GetActiveMesh();

	int partID = m_pathGuide - 1;
	if (partID < 0) return;

	int faces = 0;
	mesh.TagAllNodes(-1);
	mesh.TagAllFaces(-1);
	Post::MeshDomain& dom = mesh.Domain(partID);
	for (int i = 0; i < dom.Faces(); ++i)
	{
		FSFace& f = dom.Face(i);
		for (int j = 0; j < f.Nodes(); ++j) mesh.Node(f.n[j]).m_ntag = 1;
		f.m_ntag = faces++;
	}

	int nodes = 0;
	for (int i = 0; i < mesh.Nodes(); ++i)
	{
		FSNode& nd = mesh.Node(i);
		if (nd.m_ntag > 0) nd.m_ntag = nodes++;
	}

	m_guideMesh.Create(nodes, 0, faces);
	nodes = 0;
	for (int i = 0; i < mesh.Nodes(); ++i)
	{
		FSNode& nd = mesh.Node(i);
		if (nd.m_ntag >= 0)
		{
			FSNode& gn = m_guideMesh.Node(nodes++);
			gn.m_ntag = i;
			gn.r = nd.r;
		}
	}
	faces = 0;
	for (int i = 0; i < mesh.Faces(); ++i)
	{
		FSFace& f = mesh.Face(i);
		if (f.m_ntag >= 0)
		{
			FSFace& fd = m_guideMesh.Face(faces++);
			fd = f;
			fd.m_elem[0].eid = -1;
			for (int j = 0; j < f.Nodes(); ++j) fd.n[j] = mesh.Node(f.n[j]).m_ntag;
		}
	}

	m_guideMesh.Update();
}

void GLMusclePath::UpdateGuideMesh(int ntime)
{
	if (m_guideMesh.Nodes() == 0) BuildGuideMesh();
	assert(m_guideMesh.Nodes() != 0);

	CGLModel* glm = GetModel();
	Post::FEPostModel& fem = *glm->GetFSModel();

	for (int i = 0; i < m_guideMesh.Nodes(); ++i)
	{
		FSNode& nd = m_guideMesh.Node(i);
		nd.r = to_vec3d(fem.NodePosition(nd.m_ntag, ntime));
	}
}
