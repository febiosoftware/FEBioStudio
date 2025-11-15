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
#include "GLRenderEngine.h"
#include "GLCamera.h"
#include "GLMesh.h"

using namespace gl;

class GLRenderEngine::Imp
{
public:
	Imp() 
	{
		vertList.reserve(1024);
	}
	~Imp() 
	{
		if (mesh) delete mesh;
	}

	// modelview matrix stack
	gl::Matrix4 modelView;
	std::stack<gl::Matrix4> mvStack;

	// for building immediate mode meshes
	bool buildingShape = false;
	GLMesh* mesh = nullptr;
	bool isMVIdentity = true;
	std::vector<GLMesh::NODE> vertList;
	GLColor currentColor;
	gl::Vec3 currentTexCoord;
	gl::Vec3 currentNormal;
	PrimitiveType currentPrimitive = NULL_PRIMITIVE;

public:
	void buildPoints();
	void buildLines();
	void buildLineStrip();
	void buildLineLoop();
	void buildTriangles();
	void buildTriangleFan();
	void buildQuads();
	void buildQuadStrip();
};

GLRenderEngine::GLRenderEngine() : m(*(new Imp)) {}
GLRenderEngine::~GLRenderEngine() { delete &m; }

void GLRenderEngine::ResetStats()
{
	m_stats.clear();
}

GLRenderStats GLRenderEngine::GetRenderStats() const
{
	return m_stats;
}

void GLRenderEngine::start() 
{ 
	ResetStats(); 
	while (!m.mvStack.empty()) m.mvStack.pop();
	m.modelView.makeIdentity();
}

void GLRenderEngine::pushTransform()
{
	m.mvStack.push(m.modelView);
}

void GLRenderEngine::popTransform()
{
	assert(!m.mvStack.empty());
	m.modelView = m.mvStack.top();
	m.mvStack.pop();
}

void GLRenderEngine::resetTransform()
{
	m.modelView.makeIdentity();
}

gl::Matrix4 GLRenderEngine::currentTransform() const
{
	return m.modelView;
}

void GLRenderEngine::translate(const vec3d& r)
{
	Matrix4 T = Matrix4::translate(Vec3(r));
	m.modelView *= T;
}

void GLRenderEngine::rotate(const quatd& rot)
{
	Matrix4 R = Matrix4::rotate(rot);
	m.modelView *= R;
}

void GLRenderEngine::rotate(double deg, double x, double y, double z)
{
	quatd q(deg * DEG2RAD, vec3d(x, y, z));
	rotate(q);
}

void GLRenderEngine::scale(double x, double y, double z)
{
	Matrix4 S = Matrix4::scale(x, y, z);
	m.modelView *= S;
}

void GLRenderEngine::transform(const vec3d& r0, const vec3d& r1)
{
	translate(r0);
	rotate(r1 - r0);
}

void GLRenderEngine::transform(const vec3d& pos, const quatd& rot)
{
	translate(pos);
	rotate(rot);
}

void GLRenderEngine::multTransform(const double* a)
{
	Matrix4 M(
		a[0], a[1], a[2], a[3],
		a[4], a[5], a[6], a[7],
		a[8], a[9], a[10], a[11],
		a[12], a[13], a[14], a[15]
	);

	m.modelView *= M;
}

void GLRenderEngine::rotate(const vec3d& r, vec3d ref)
{
	vec3d n = r; n.Normalize();
	vec3d z = ref; z.Normalize();
	if ((z * n) == -1)
	{
		vec3d e(1, 0, 0);
		if (fabs(e * n) > 0.9) e = vec3d(0, 1, 0);
		vec3d t = (n ^ e);
		rotate(180, t.x, t.y, t.z);
	}
	else if (z * n != 1)
	{
		quatd q(z, n);
		rotate(q);
	}
}

void GLRenderEngine::transform(const Transform& T)
{
	vec3d r = T.GetPosition();
	vec3d s = T.GetScale();
	quatd q = T.GetRotation();

	// translate mesh
	translate(r);

	// orient mesh
	rotate(q);

	// scale the mesh
	scale(s.x, s.y, s.z);
}

void GLRenderEngine::renderPoint(const vec3d& r)
{
	begin(POINTS);
	{
		vertex(r);
	}
	end();
}

void GLRenderEngine::renderLine(const vec3d& a, const vec3d& b)
{
	begin(LINES);
	{
		vertex(a);
		vertex(b);
	}
	end();
}

void GLRenderEngine::renderPath(const std::vector<vec3d>& points)
{
	if (points.size() < 2) return;
	begin(LINESTRIP);
	{
		for (const vec3d& p : points) vertex(p);
	}
	end();
}

void GLRenderEngine::renderRect(double x0, double y0, double x1, double y1)
{
	begin(QUADS);
	{
		vertex(vec3d(x0, y0, 0));
		vertex(vec3d(x1, y0, 0));
		vertex(vec3d(x1, y1, 0));
		vertex(vec3d(x0, y1, 0));
	}
	end();
}

void GLRenderEngine::renderLine(const vec3d& a, const vec3d& b, const vec3d& c)
{
	begin(LINESTRIP);
	{
		vertex(a);
		vertex(b);
		vertex(c);
	}
	end();
}

void GLRenderEngine::renderGMeshOutline(const GLCamera& cam, const GLMesh& gmsh, const Transform& T)
{
	// get some settings
	quatd q = cam.GetOrientation();
	vec3d p = cam.GlobalPosition();

	// this array will collect all points to render
	std::vector<vec3f> points; points.reserve(1024);

	int NF = gmsh.Faces();
	if (NF > 0)
	{
		// loop over all faces
		for (int i = 0; i < NF; ++i)
		{
			const GLMesh::FACE& f = gmsh.Face(i);
			for (int j = 0; j < 3; ++j)
			{
				bool bdraw = false;

				if (f.nbr[j] > i)
				{
					const GLMesh::FACE& f2 = gmsh.Face(f.nbr[j]);
					vec3d n1 = T.LocalToGlobalNormal(to_vec3d(f.fn));
					vec3d n2 = T.LocalToGlobalNormal(to_vec3d(f2.fn));

					if (cam.IsOrtho())
					{
						q.RotateVector(n1);
						q.RotateVector(n2);
						if (n1.z * n2.z <= 0) bdraw = true;
					}
					else
					{
						int a = j;
						int b = (j + 1) % 3;
						vec3d ra = T.LocalToGlobal(to_vec3d(gmsh.Node(f.n[a]).r));
						vec3d rb = T.LocalToGlobal(to_vec3d(gmsh.Node(f.n[b]).r));
						vec3d c = (ra + rb) * 0.5;
						vec3d pc = p - c;
						double d1 = pc * n1;
						double d2 = pc * n2;
						if (d1 * d2 <= 0) bdraw = true;
					}
				}

				if (bdraw)
				{
					int a = f.n[j];
					int b = f.n[(j + 1) % 3];
					if (a > b) { a ^= b; b ^= a; a ^= b; }

					points.push_back(gmsh.Node(a).r);
					points.push_back(gmsh.Node(b).r);
				}
			}
		}
	}
	if (points.empty()) return;

	int nodes = points.size();
	int edges = points.size() / 2;

	GLMesh mesh;
	mesh.Create(nodes, 0, edges);
	for (int i = 0; i < edges; ++i)
	{
		mesh.AddEdge(points[2 * i], points[2 * i + 1]);
	}
	mesh.Update();

	renderGMeshEdges(mesh, false);
}

// TODO: Do I really need to pass the transform? Can't I just use the modelview matrix?
void GLRenderEngine::renderGMeshOutline(const GLCamera& cam, const GLMesh& gmsh, const Transform& T, int surfID)
{
	// get some settings
	quatd q = cam.GetOrientation();
	vec3d p = cam.GlobalPosition();

	// this array will collect all points to render
	vector<vec3f> points; points.reserve(1024);

	const GLMesh::PARTITION& part = gmsh.Partition(surfID);
	int NF = part.nf;
	if (NF > 0)
	{
		// loop over all faces
		for (int i = 0; i < NF; ++i)
		{
			const GLMesh::FACE& f = gmsh.Face(i + part.n0);
			for (int j = 0; j < 3; ++j)
			{
				bool bdraw = false;

				if (f.nbr[j] < 0)
				{
					bdraw = true;
				}
				else if (f.nbr[j] > i)
				{
					const GLMesh::FACE& f2 = gmsh.Face(f.nbr[j]);

					if (f.pid != f2.pid)
					{
						bdraw = true;
					}
					else
					{
						vec3d n1 = T.LocalToGlobalNormal(to_vec3d(f.fn));
						vec3d n2 = T.LocalToGlobalNormal(to_vec3d(f2.fn));

						if (cam.IsOrtho())
						{
							q.RotateVector(n1);
							q.RotateVector(n2);
							if (n1.z * n2.z <= 0) bdraw = true;
						}
						else
						{
							int a = j;
							int b = (j + 1) % 3;
							vec3d ra = T.LocalToGlobal(to_vec3d(gmsh.Node(f.n[a]).r));
							vec3d rb = T.LocalToGlobal(to_vec3d(gmsh.Node(f.n[b]).r));
							vec3d c = (ra + rb) * 0.5;
							vec3d pc = p - c;
							double d1 = pc * n1;
							double d2 = pc * n2;
							if (d1 * d2 <= 0) bdraw = true;
						}
					}
				}

				if (bdraw)
				{
					int a = f.n[j];
					int b = f.n[(j + 1) % 3];
					if (a > b) { a ^= b; b ^= a; a ^= b; }

					points.push_back(gmsh.Node(a).r);
					points.push_back(gmsh.Node(b).r);
				}
			}
		}
	}
	if (points.empty()) return;

	int nodes = points.size();
	int edges = points.size() / 2;

	GLMesh mesh;
	mesh.Create(nodes, 0, edges);
	for (int i = 0; i < edges; ++i)
	{
		mesh.AddEdge(points[2 * i], points[2 * i + 1]);
	}
	mesh.Update();

	renderGMeshEdges(mesh, false);
}

void GLRenderEngine::setColor(GLColor c)
{
	m.currentColor = c;
}

void GLRenderEngine::vertex(const vec3d& r)
{
	GLMesh::NODE p;
	gl::Vec4 q = m.modelView * gl::Vec4(r);
	gl::Vec4 N = m.modelView * gl::Vec4(m.currentNormal, 0);
	p.r = vec3f(q[0], q[1], q[2]);
	p.n = vec3f(N[0], N[1], N[2]); p.n.Normalize();
	p.c = m.currentColor;
	p.t = to_vec3f(m.currentTexCoord);
	m.vertList.push_back(p);
}

void GLRenderEngine::normal(const vec3d& r)
{
	m.currentNormal = Vec3(r);
}

void GLRenderEngine::texCoord1d(double t)
{
	m.currentTexCoord = vec3f(t, 0, 0);
}

void GLRenderEngine::texCoord2d(double r, double s)
{
	m.currentTexCoord = Vec3(r, s, 0);
}

void GLRenderEngine::beginShape()
{
	if (m.mesh) delete m.mesh;
	m.mesh = new GLMesh();
	m.vertList.clear();
	m.currentColor = GLColor(255, 255, 255);
	m.currentNormal = Vec3(0, 0, 1);
	m.buildingShape = true;
	m.currentPrimitive = GLRenderEngine::NULL_PRIMITIVE;

	pushTransform();
	resetTransform();
}

void GLRenderEngine::begin(PrimitiveType prim)
{
	if (!m.buildingShape) {
		beginShape(); 
		m.buildingShape = false; // this was set to true in beginShape
	}
	m.currentPrimitive = prim;
}

void GLRenderEngine::endShape()
{
	if (m.mesh) m.mesh->UpdateBoundingBox();
	popTransform();
	m.buildingShape = false;

	renderImmediateModeMesh();
}

GLMesh* GLRenderEngine::takeImmediateModeMesh()
{
	assert(m.mesh);
	GLMesh* pm = m.mesh;
	m.mesh = nullptr;
	return pm;
}

void GLRenderEngine::renderImmediateModeMesh()
{
	GLMesh* pm = takeImmediateModeMesh();
	if (pm)
	{
		if (pm->Faces() > 0)
		{
			renderGMesh(*pm, false);
		}

		if (pm->Edges() > 0)
		{
			renderGMeshEdges(*pm, false);
		}

		if ((pm->Nodes() > 0) && (pm->Edges() == 0) && (pm->Faces() == 0))
		{
			renderGMeshNodes(*pm, false);
		}

		delete pm;
	}
}

void GLRenderEngine::end()
{
	switch (m.currentPrimitive)
	{
	case PrimitiveType::POINTS     : m.buildPoints(); break;
	case PrimitiveType::LINES      : m.buildLines(); break;
	case PrimitiveType::LINESTRIP  : m.buildLineStrip(); break;
	case PrimitiveType::LINELOOP   : m.buildLineLoop(); break;
	case PrimitiveType::TRIANGLES  : m.buildTriangles(); break;
	case PrimitiveType::TRIANGLEFAN: m.buildTriangleFan(); break;
	case PrimitiveType::QUADS      : m.buildQuads(); break;
	case PrimitiveType::QUADSTRIP  : m.buildQuadStrip(); break;
	default:
		assert(false);
	}

	m.vertList.clear();
	m.currentPrimitive = GLRenderEngine::NULL_PRIMITIVE;

	if (!m.buildingShape) endShape();
}

void GLRenderEngine::Imp::buildPoints()
{
	assert(currentPrimitive == PrimitiveType::POINTS);
	assert(mesh);
	if (mesh == nullptr) return;

	for (int i = 0; i < vertList.size(); ++i)
	{
		mesh->AddNode(vertList[i]);
	}
}

void GLRenderEngine::Imp::buildLines()
{
	assert(currentPrimitive == PrimitiveType::LINES);
	assert(mesh);
	if (mesh == nullptr) return;

	const int N = vertList.size();
	vector<int> NL(N);

	for (int i = 0; i < N; ++i)
	{
		NL[i] = mesh->AddNode(vertList[i]);
	}

	if (N > 1)
	{
		for (int i = 0; i < N; i += 2)
		{
			mesh->AddEdge(NL[i], NL[i + 1]);
		}
	}
}

void GLRenderEngine::Imp::buildLineStrip()
{
	assert(currentPrimitive == PrimitiveType::LINESTRIP);
	assert(mesh);
	if (mesh == nullptr) return;

	const int N = vertList.size();
	vector<int> NL(N);

	for (int i = 0; i < N; ++i)
	{
		NL[i] = mesh->AddNode(vertList[i]);
	}

	if (N > 1)
	{
		for (int i = 0; i < N - 1; i++)
		{
			mesh->AddEdge(NL[i], NL[i + 1]);
		}
	}
}

void GLRenderEngine::Imp::buildLineLoop()
{
	assert(currentPrimitive == PrimitiveType::LINELOOP);
	assert(mesh);
	if (mesh == nullptr) return;

	const int N = vertList.size();
	vector<int> NL(N);

	for (int i = 0; i < N; ++i)
	{
		NL[i] = mesh->AddNode(vertList[i]);
	}

	if (N > 1)
	{
		for (int i = 0; i < N; i++)
		{
			int ip1 = (i + 1) % N;
			mesh->AddEdge(NL[i], NL[ip1]);
		}
	}
}

void GLRenderEngine::Imp::buildTriangles()
{
	assert(currentPrimitive == PrimitiveType::TRIANGLES);
	assert(mesh);
	if (mesh == nullptr) return;

	const int N = vertList.size();
	vector<int> NL(N);

	for (int i = 0; i < N; ++i)
	{
		NL[i] = mesh->AddNode(vertList[i]);
	}

	if (N > 2)
	{
		for (int i = 0; i < N; i += 3)
		{
			mesh->AddFace(NL[i], NL[i + 1], NL[i + 2]);
		}
	}
}

void GLRenderEngine::Imp::buildTriangleFan()
{
	assert(currentPrimitive == PrimitiveType::TRIANGLEFAN);
	assert(mesh);
	if (mesh == nullptr) return;

	const int N = vertList.size();
	vector<int> NL(N);

	for (int i = 0; i < N; ++i)
	{
		NL[i] = mesh->AddNode(vertList[i]);
	}

	if (N > 2)
	{
		for (int i = 1; i < N - 1; i++)
		{
			mesh->AddFace(NL[0], NL[i], NL[i + 1]);
		}
	}
}

void GLRenderEngine::Imp::buildQuads()
{
	assert(currentPrimitive == PrimitiveType::QUADS);
	assert(mesh);
	if (mesh == nullptr) return;

	const int N = vertList.size();
	vector<int> NL(N);

	for (int i = 0; i < N; ++i)
	{
		NL[i] = mesh->AddNode(vertList[i]);
	}

	if (N > 3)
	{
		for (int i = 0; i < N - 3; i += 4)
		{
			int n0 = NL[i];
			int n1 = NL[i + 1];
			int n2 = NL[i + 2];
			int n3 = NL[i + 3];
			mesh->AddFace(n0, n1, n2);
			mesh->AddFace(n2, n3, n0);
		}
	}
}

void GLRenderEngine::Imp::buildQuadStrip()
{
	assert(currentPrimitive == PrimitiveType::QUADSTRIP);
	assert(mesh);
	if (mesh == nullptr) return;

	const int N = vertList.size();
	vector<int> NL(N);

	for (int i = 0; i < N; ++i)
	{
		NL[i] = mesh->AddNode(vertList[i]);
	}

	if (N > 3)
	{
		int n0 = NL[0];
		int n1 = NL[1];
		for (int i = 0; i < N - 3; i += 2)
		{
			int n2 = NL[i + 2];
			int n3 = NL[i + 3];
			mesh->AddFace(n0, n1, n2);
			mesh->AddFace(n1, n3, n2);
			n0 = n2;
			n1 = n3;
		}
	}
}
