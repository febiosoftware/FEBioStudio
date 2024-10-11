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
	m_bSolidBeamRadius = 1.f;
	m_nshellref = 0;
	m_ndivs = 1;
	m_pointSize = 7.f;
	m_useShaders = false;
	m_defaultShader = nullptr;
}

void GLMeshRender::ResetStats()
{
	m_stats.clear();
}

GLRenderStats GLMeshRender::GetRenderStats()
{
	return m_stats;
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

void GLMeshRender::RenderGMesh(const GMesh& mesh)
{
	if (!m_useShaders || m_shaders.empty()) return;

	int lastShader = -1;
	GLFacetShader* shader = m_defaultShader;
	if (shader) shader->Activate();
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
				if (shader) shader->Deactivate();
				if (f.mid != -1)
				{
					shader = m_shaders[f.mid];
					if (shader == nullptr) shader = m_defaultShader;
				}
				else
					shader = m_defaultShader;
				if (shader) shader->Activate();
			}
			if (shader)
			{
				glBegin(GL_TRIANGLES);
				{
					for (int j = 0; j < nf; ++j)
					{
						shader->Render(mesh.Face(n0 + j));
					}
				}
				glEnd();
				m_stats.triangles += nf;
			}
		}
	}
	if (shader) shader->Deactivate();
}

void GLMeshRender::RenderGMesh(const GMesh& mesh, GLFacetShader& shader)
{
	shader.Activate();
	int NF = mesh.Faces();
	glBegin(GL_TRIANGLES);
	{
		for (int i = 0; i < NF; ++i)
		{
			shader.Render(mesh.Face(i));
		}
	}
	glEnd();
	shader.Deactivate();
	m_stats.triangles += NF;
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
		glBegin(GL_TRIANGLES);
		{
			for (int i = 0; i < NF; ++i)
			{
				shader->Render(mesh.Face(i + n0));
			}
		}
		glEnd();
		if (shader != m_defaultShader) shader->Deactivate();
		m_stats.triangles += NF;
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
		glBegin(GL_TRIANGLES);
		{
			for (int i = 0; i < NF; ++i)
			{
				shader.Render(mesh.Face(i + n0));
			}
		}
		glEnd();
		shader.Deactivate();
		m_stats.triangles += NF;
	}
}

void GLMeshRender::RenderEdges(const GMesh& mesh, int nid)
{
	int N = mesh.Edges();
	if (N == 0) return;
	if ((nid < 0) || (nid >= mesh.EILs())) return;
	glBegin(GL_LINES);
	{
		const pair<int, int>& eil = mesh.EIL(nid);
		for (int i = 0; i < eil.second; ++i)
		{
			const GMesh::EDGE& e = mesh.Edge(i + eil.first);
			assert(e.pid == nid);
			if ((e.n[0] != -1) && (e.n[1] != -1))
			{
				const vec3f& r0 = mesh.Node(e.n[0]).r;
				const vec3f& r1 = mesh.Node(e.n[1]).r;
				glVertex3d(r0.x, r0.y, r0.z);
				glVertex3d(r1.x, r1.y, r1.z);
			}
		}
	}
	glEnd();
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
				bdraw = true;
			}
			else if (outline)
			{
				int j1 = (j + 1) % 3;
				if (f.n[j] < f.n[j1])
				{
					GMesh::FACE& f2 = pm->Face(f.nbr[j]);
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

			if (bdraw)
			{
				points.push_back(f.vr[j]);
				points.push_back(f.vr[(j+1)%3]);
			}
		}
	}
	if (points.empty()) return;

	// build the line mesh
	GLLineMesh lineMesh;
	lineMesh.Create((int)points.size() / 2);
	lineMesh.BeginMesh();
	for (auto& p : points) lineMesh.AddVertex(p);
	lineMesh.EndMesh();

	// render the active edges
	lineMesh.Render();
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
	GLLineMesh lineMesh;
	lineMesh.Create((int)points.size() / 2);
	lineMesh.BeginMesh();
	for (auto& p : points) lineMesh.AddVertex(p);
	lineMesh.EndMesh();

	// render the active edges
	lineMesh.Render();
}

void GLMeshRender::RenderEdges(const GMesh& m)
{
	int NE = m.Edges();
	if (NE == 0) return;
	glBegin(GL_LINES);
	{
		for (int i = 0; i < NE; i++)
		{
			const GMesh::EDGE& e = m.Edge(i);
			glx::line(e.vr[0], e.vr[1]);
		}
	}
	glEnd();
}

void GLMeshRender::RenderEdges(const GMesh& m, std::function<bool(const GMesh::EDGE& e)> f)
{
	int NE = m.Edges();
	if (NE == 0) return;
	glBegin(GL_LINES);
	{
		for (int i = 0; i < NE; i++)
		{
			const GMesh::EDGE& e = m.Edge(i);
			if (f(e))
			{
				glx::line(e.vr[0], e.vr[1]);
			}
		}
	}
	glEnd();
}

void GLMeshRender::RenderEdges(const GMesh& m, GLLineShader& shader)
{
	int NE = m.Edges();
	if (NE == 0) return;
	shader.Activate();
	glBegin(GL_LINES);
	{
		for (int i = 0; i < NE; i++)
		{
			shader.Render(m.Edge(i));
		}
	}
	glEnd();
	shader.Deactivate();
}

void GLMeshRender::RenderNormals(const GMesh& mesh, GLLineShader& shader)
{
	shader.Activate();
	glBegin(GL_LINES);
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
	glEnd();
	shader.Deactivate();
}

void GLMeshRender::RenderPoints(GMesh& mesh)
{
	if (mesh.Nodes() == 0) return;
	GLfloat old_size;
	glGetFloatv(GL_POINT_SIZE, &old_size);
	glPointSize(m_pointSize);
	glBegin(GL_POINTS);
	{
		int NN = mesh.Nodes();
		for (int i = 0; i < NN; ++i)
		{
			vec3f& r = mesh.Node(i).r;
			glVertex3f(r.x, r.y, r.z);
		}
	}
	glEnd();
	glPointSize(old_size);
}

void GLMeshRender::RenderPoints(GMesh& mesh, GLPointShader& shader)
{
	if (mesh.Nodes() == 0) return;
	GLfloat old_size;
	glGetFloatv(GL_POINT_SIZE, &old_size);
	glPointSize(m_pointSize);
	shader.Activate();
	glBegin(GL_POINTS);
	{
		int NN = mesh.Nodes();
		for (int i = 0; i < NN; ++i)
		{
			shader.Render(mesh.Node(i));
		}
	}
	glEnd();
	shader.Deactivate();
	glPointSize(old_size);
}

void GLMeshRender::RenderPoints(GMesh& mesh, std::vector<int>& nodeList)
{
	GLfloat old_size;
	glGetFloatv(GL_POINT_SIZE, &old_size);
	glPointSize(m_pointSize);
	glBegin(GL_POINTS);
	{
		for (int i : nodeList)
		{
			vec3f& r = mesh.Node(i).r;
			glVertex3f(r.x, r.y, r.z);
		}
	}
	glEnd();
	glPointSize(old_size);
}

void GLMeshRender::RenderPoints(GMesh& mesh, std::function<bool(const GMesh::NODE& node)> fnc)
{
	if (mesh.Nodes() == 0) return;
	GLfloat old_size;
	glGetFloatv(GL_POINT_SIZE, &old_size);
	glPointSize(m_pointSize);
	glBegin(GL_POINTS);
	{
		int NN = mesh.Nodes();
		for (int i = 0; i < NN; ++i)
		{
			GMesh::NODE& node = mesh.Node(i);
			if (fnc(node))
			{
				vec3f& r = mesh.Node(i).r;
				glVertex3f(r.x, r.y, r.z);
			}
		}
	}
	glEnd();
	glPointSize(old_size);
}
