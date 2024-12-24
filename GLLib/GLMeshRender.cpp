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

#include "GLMeshRender.h"
#include <MeshLib/GMesh.h>
#include "glx.h"
#include "GLMesh.h"
#include "GLCamera.h"
#include "GLShader.h"
#include <FECore/FETransform.h>

GLMeshRender::GLMeshRender()
{
	m_bShell2Solid = false;
	m_bBeam2Solid = false;
	m_nshellref = 0;
	m_ndivs = 1;
	m_pointSize = 7.f;
	m_useShaders = false;
	m_defaultShader = nullptr;

	m_pointShader = nullptr;
	m_lineShader = nullptr;
	m_facetShader = nullptr;
}

GLMeshRender::~GLMeshRender()
{
	delete m_pointShader;
	delete m_lineShader;
	delete m_facetShader;
}

void GLMeshRender::SetUseShaders(bool b)
{
	m_useShaders = b;
}

void GLMeshRender::ClearShaders()
{
	for (auto s : m_shaders) delete s;
	m_shaders.clear();
	m_defaultShader = nullptr;
}

void GLMeshRender::SetDefaultShader(GLFacetShader* shader)
{
	m_defaultShader = shader;
}

void GLMeshRender::AddShader(GLFacetShader* shader)
{
	m_shaders.push_back(shader);
}

void GLMeshRender::SetPointShader(GLPointShader* pointShader)
{
	assert((m_pointShader == nullptr) || (!m_pointShader->IsActive()));
	delete m_pointShader;
	m_pointShader = pointShader;
}

void GLMeshRender::SetLineShader(GLLineShader* lineShader)
{
	assert((m_lineShader == nullptr) || (!m_lineShader->IsActive()));
	delete m_lineShader;
	m_lineShader = lineShader;
}

void GLMeshRender::SetFacetShader(GLFacetShader* facetShader)
{
	assert((m_facetShader == nullptr) || (!m_facetShader->IsActive()));
	delete m_facetShader;
	m_facetShader = facetShader;
}

//-----------------------------------------------------------------------------
void GLMeshRender::PushState()
{
	glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT);
}

//-----------------------------------------------------------------------------
void GLMeshRender::PopState()
{
	glPopAttrib();
}

void GLMeshRender::RenderLine(const vec3d& a, const vec3d& b)
{
	glBegin(GL_LINES);
	{
		glVertex3d(a.x, a.y, a.z);
		glVertex3d(b.x, b.y, b.z);
	}
	glEnd();
}

void GLMeshRender::RenderGMesh(const GMesh& mesh)
{
	if (!m_useShaders || m_shaders.empty()) return;

	int lastShader = -1;
	GLFacetShader* shader = m_defaultShader;
	for (int i = 0; i < mesh.Partitions(); ++i)
	{
		const GMesh::PARTITION& p = mesh.Partition(i);
		int n0 = p.n0;
		int nf = p.nf;
		if (nf > 0)
		{
			const GMesh::FACE& f = mesh.Face(n0);
			if (f.mid != lastShader)
			{
				lastShader = f.mid;
				if (f.mid != -1)
				{
					shader = m_shaders[f.mid];
					if (shader == nullptr) shader = m_defaultShader;
				}
				else
					shader = m_defaultShader;
			}
			if (shader)
			{
				shader->Activate();
				for (int j = 0; j < nf; ++j)
				{
					shader->Render(mesh.Face(n0 + j));
				}
				shader->Deactivate();
			}
		}
	}
}

void GLMeshRender::RenderGMesh(const GMesh& mesh, GLFacetShader& shader)
{
	int NF = mesh.Faces();
	if (NF == 0) return;
	shader.Activate();
	for (int i = 0; i < NF; ++i)
	{
		shader.Render(mesh.Face(i));
	}
	shader.Deactivate();
}

void GLMeshRender::RenderGMesh(const GMesh& mesh, int surfID)
{
	if ((surfID < 0) || (surfID >= (int)mesh.Partitions())) return;
	const GMesh::PARTITION& p = mesh.Partition(surfID);
	if (p.nf > 0)
	{
		int NF = p.nf;
		int n0 = p.n0;
		// It's assumed that the default shader has already been activated!
		GLFacetShader* shader = m_defaultShader;
		if (m_useShaders)
		{
			int shaderID = mesh.Face(n0).mid;
			if ((shaderID >= 0) && (shaderID < m_shaders.size()))
			{
				shader = m_shaders[shaderID];
				shader->Activate();
			}
		}
		if (shader == nullptr) return;
		for (int i = 0; i < NF; ++i)
		{
			shader->Render(mesh.Face(i + n0));
		}
		if (shader != m_defaultShader) shader->Deactivate();
	}
}

void GLMeshRender::RenderGMesh(const GMesh& mesh, int surfID, GLFacetShader& shader)
{
	if ((surfID < 0) || (surfID >= (int)mesh.Partitions())) return;
	const GMesh::PARTITION& p = mesh.Partition(surfID);
	if (p.nf > 0)
	{
		int NF = p.nf;
		int n0 = p.n0;
		shader.Activate();
		for (int i = 0; i < NF; ++i)
		{
			shader.Render(mesh.Face(i + n0));
		}
		shader.Deactivate();
	}
}

void GLMeshRender::RenderEdges(const GMesh& mesh, int nid)
{
	if (m_lineShader == nullptr) { assert(false); return; }
	int N = mesh.Edges();
	if (N == 0) return;
	if ((nid < 0) || (nid >= mesh.EILs())) return;
	m_lineShader->Activate();
	{
		const pair<int, int>& eil = mesh.EIL(nid);
		for (int i = 0; i < eil.second; ++i)
		{
			const GMesh::EDGE& e = mesh.Edge(i + eil.first);
			assert(e.pid == nid);
			m_lineShader->Render(e);
		}
	}
	m_lineShader->Deactivate();
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderOutline(CGLCamera& cam, GMesh* pm, const Transform& T, bool outline)
{
	// get some settings
	quatd q = cam.GetOrientation();
	vec3d p = cam.GlobalPosition();

	// this array will collect all points to render
	vector<vec3f> points; points.reserve(1024);

	// loop over all faces
	for (int i = 0; i < pm->Faces(); ++i)
	{
		GMesh::FACE& f = pm->Face(i);

		for (int j = 0; j < 3; ++j)
		{
			bool bdraw = false;

			if (f.nbr[j] < 0)
			{
				bdraw = false;
			}
			else
			{
				int j1 = (j + 1) % 3;
				if (f.n[j] < f.n[j1])
				{
					GMesh::FACE& f2 = pm->Face(f.nbr[j]);
					if ((f.pid != f2.pid) || (f.sid != f2.sid))
					{
						bdraw = false;
					}
					else if (outline)
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
							vec3d r1 = T.LocalToGlobal(to_vec3d(f.vr[j]));
							vec3d r2 = T.LocalToGlobal(to_vec3d(f.vr[j1]));
							vec3d c = (r1 + r2) * 0.5;
							vec3d pc = p - c;
							double d1 = pc * n1;
							double d2 = pc * n2;
							if (d1 * d2 <= 0) bdraw = true;
						}
					}
				}
			}

			if (bdraw)
			{
				points.push_back(f.vr[j]);
				points.push_back(f.vr[(j+1)%3]);
			}
		}
	}
	if (points.empty()) return;

	// build the line mesh
	GMesh lineMesh;
	int NN = (int)points.size();
	int NE = (int)points.size() / 2;
	lineMesh.Create(NN, 0, NE);
	for (int i=0; i<NE; ++i)
	{
		lineMesh.Node(2 * i    ).r = points[2 * i];
		lineMesh.Node(2 * i + 1).r = points[2 * i + 1];
		GMesh::EDGE& edge = lineMesh.Edge(i);
		edge.n[0] = 2 * i;
		edge.n[1] = 2 * i + 1;
		edge.vr[0] = points[2 * i];
		edge.vr[1] = points[2 * i + 1];
	}
//	lineMesh.Update();

	// render the active edges
	RenderEdges(lineMesh);
}

void GLMeshRender::RenderSurfaceOutline(CGLCamera& cam, GMesh* pm, const Transform& T, int surfID)
{
	// get some settings
	quatd q = cam.GetOrientation();
	vec3d p = cam.GlobalPosition();

	// this array will collect all points to render
	vector<vec3f> points; points.reserve(1024);

	const GMesh::PARTITION& part = pm->Partition(surfID);
	int NF = part.nf;
	if (NF > 0)
	{
		// loop over all faces
		for (int i = 0; i < NF; ++i)
		{
			const GMesh::FACE& f = pm->Face(i + part.n0);
			for (int j = 0; j < 3; ++j)
			{
				bool bdraw = false;

				if (f.nbr[j] < 0)
				{
					bdraw = true;
				}
				else
				{
					GMesh::FACE& f2 = pm->Face(f.nbr[j]);

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
							vec3d ra = T.LocalToGlobal(to_vec3d(pm->Node(f.n[a]).r));
							vec3d rb = T.LocalToGlobal(to_vec3d(pm->Node(f.n[b]).r));
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

					points.push_back(pm->Node(a).r);
					points.push_back(pm->Node(b).r);
				}
			}
		}
	}
	if (points.empty()) return;

	// build the line mesh
	GMesh lineMesh;
	int NN = (int)points.size();
	int NE = (int)points.size() / 2;
	lineMesh.Create(NN, 0, NE);
	for (int i = 0; i < NE; ++i)
	{
		lineMesh.Node(2 * i).r = points[2 * i];
		lineMesh.Node(2 * i + 1).r = points[2 * i + 1];
		lineMesh.Edge(i).n[0] = 2 * i;
		lineMesh.Edge(i).n[1] = 2 * i + 1;
	}
	lineMesh.Update();

	// render the active edges
	RenderEdges(lineMesh);
}

void GLMeshRender::RenderEdges(const GMesh& m)
{
	if (m_lineShader == nullptr) { assert(false); return; }
	int NE = m.Edges();
	if (NE == 0) return;
	m_lineShader->Activate();
	{
		for (int i = 0; i < NE; i++)
		{
			const GMesh::EDGE& e = m.Edge(i);
			m_lineShader->Render(e);
		}
	}
	m_lineShader->Deactivate();
}

void GLMeshRender::RenderEdges(const GMesh& m, std::function<bool(const GMesh::EDGE& e)> f)
{
	if (m_lineShader == nullptr) { assert(false); return; }
	int NE = m.Edges();
	if (NE == 0) return;
	m_lineShader->Activate();
	for (int i = 0; i < NE; i++)
	{
		const GMesh::EDGE& e = m.Edge(i);
		if (f(e))
		{
			m_lineShader->Render(e);
		}
	}
	m_lineShader->Deactivate();
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderGMeshLines(GMesh* pm)
{
	if (pm == 0) return;

	glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT | GL_LINE_BIT);
	glDisable(GL_LIGHTING);
//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// loop over all faces
	glBegin(GL_LINES);
	for (int i = 0; i < pm->Faces(); i++)
	{
		GMesh::FACE& face = pm->Face(i);
		const vec3d& r1 = to_vec3d(pm->Node(face.n[0]).r);
		const vec3d& r2 = to_vec3d(pm->Node(face.n[1]).r);
		const vec3d& r3 = to_vec3d(pm->Node(face.n[2]).r);
		glx::lineLoop(r1, r2, r3);
	} // for
	glEnd();

	glPopAttrib();
}

void GLMeshRender::RenderNormals(const GMesh& mesh, GLLineShader& shader)
{
	shader.Activate();
	{
		GMesh::EDGE edge;
		for (int i = 0; i < mesh.Faces(); ++i)
		{
			const GMesh::FACE& face = mesh.Face(i);

			vec3f r1(0, 0, 0);
			vec3f fn = face.fn;

			for (int j = 0; j < 3; ++j) r1 += mesh.Node(face.n[j]).r;
			r1 /= 3.f;

			edge.vr[0] = r1;
			edge.vr[1] = fn; // Note that we store the normal as the second coordinate!

			shader.Render(edge);
		}
	}
	shader.Deactivate();
}

void GLMeshRender::RenderPoints(GMesh& mesh)
{
	if (m_pointShader == nullptr) { assert(false); return; }
	if (mesh.Nodes() == 0) return;
	GLfloat old_size;
	glGetFloatv(GL_POINT_SIZE, &old_size);
	glPointSize(m_pointSize);
	m_pointShader->Activate();
	{
		int NN = mesh.Nodes();
		for (int i = 0; i < NN; ++i)
		{
			m_pointShader->Render(mesh.Node(i));
		}
	}
	m_pointShader->Deactivate();
	glPointSize(old_size);
}

void GLMeshRender::RenderPoints(GMesh& mesh, std::vector<int>& nodeList)
{
	if (m_pointShader == nullptr) { assert(false); return; }
	if (nodeList.empty()) return;
	GLfloat old_size;
	glGetFloatv(GL_POINT_SIZE, &old_size);
	glPointSize(m_pointSize);
	m_pointShader->Activate();
	{
		for (int i : nodeList)
		{
			m_pointShader->Render(mesh.Node(i));
		}
	}
	m_pointShader->Deactivate();
	glPointSize(old_size);
}

void GLMeshRender::RenderPoints(GMesh& mesh, std::function<bool(const GMesh::NODE& node)> fnc)
{
	if (m_pointShader == nullptr) { assert(false); return; }
	if (mesh.Nodes() == 0) return;
	GLfloat old_size;
	glGetFloatv(GL_POINT_SIZE, &old_size);
	glPointSize(m_pointSize);
	m_pointShader->Activate();
	{
		int NN = mesh.Nodes();
		for (int i = 0; i < NN; ++i)
		{
			GMesh::NODE& node = mesh.Node(i);
			if (fnc(node))
			{
				m_pointShader->Render(mesh.Node(i));
			}
		}
	}
	m_pointShader->Deactivate();
	glPointSize(old_size);
}
