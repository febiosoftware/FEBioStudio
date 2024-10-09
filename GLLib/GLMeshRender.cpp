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
#include <MeshLib/FECoreMesh.h>
#include <MeshLib/FEElement.h>
#include <MeshLib/MeshMetrics.h>
#include <MeshLib/quad8.h>
#include <MeshLib/GMesh.h>
#include "glx.h"
#include "GLMesh.h"
#include "GLContext.h"
#include "GLCamera.h"
#include "GLShader.h"
#include <FECore/FETransform.h>

// drawing routines for faces
// Note: Call these functions from within glBegin(GL_TRIANGLES)\glEnd() section
void RenderQUAD4(FSMeshBase* pm, const FSFace& f);
void RenderQUAD8(FSMeshBase* pm, const FSFace& f);
void RenderQUAD9(FSMeshBase* pm, const FSFace& f);
void RenderTRI3 (FSMeshBase* pm, const FSFace& f);
void RenderTRI6 (FSMeshBase* pm, const FSFace& f);
void RenderTRI7 (FSMeshBase* pm, const FSFace& f);
void RenderTRI10(FSMeshBase* pm, const FSFace& f);

void RenderSmoothQUAD4(FSMeshBase* pm, const FSFace& face, int ndivs);
void RenderSmoothQUAD8(FSMeshBase* pm, const FSFace& face, int ndivs);
void RenderSmoothQUAD9(FSMeshBase* pm, const FSFace& face, int ndivs);
void RenderSmoothTRI3 (FSMeshBase* pm, const FSFace& face, int ndivs);
void RenderSmoothTRI6 (FSMeshBase* pm, const FSFace& face, int ndivs);
void RenderSmoothTRI7 (FSMeshBase* pm, const FSFace& face, int ndivs);
void RenderSmoothTRI10(FSMeshBase* pm, const FSFace& face, int ndivs);

void RenderFace1Outline(FSCoreMesh* pm, FSFace& face);
void RenderFace2Outline(FSCoreMesh* pm, FSFace& face, int ndivs);
void RenderFace3Outline(FSCoreMesh* pm, FSFace& face, int ndivs);

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

void GLMeshRender::RenderLineLoop(const vec3d& r0, const vec3d& r1, const vec3d& r2, const vec3d& r3)
{
	glBegin(GL_LINE_LOOP);
	{
		glx::vertex3d(r0);
		glx::vertex3d(r1);
		glx::vertex3d(r2);
		glx::vertex3d(r3);
	}
	glEnd();
}

void GLMeshRender::RenderLines(vec3d* r, int n)
{
	glBegin(GL_LINES);
	{
		for (int i = 0; i < n; ++i) glx::vertex3d(r[i]);
	}
	glEnd();
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
void GLMeshRender::RenderOutline(CGLContext& rc, GMesh* pm, const Transform& T, bool outline)
{
	// get some settings
	CGLCamera& cam = *rc.m_cam;
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

void GLMeshRender::RenderSurfaceOutline(CGLContext& rc, GMesh* pm, const Transform& T, int surfID)
{
	// get some settings
	CGLCamera& cam = *rc.m_cam;
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

void GLMeshRender::RenderFEFaces(FSMeshBase* pm, const std::vector<int>& faceList)
{
	if (faceList.empty()) return;
	glBegin(GL_TRIANGLES);
	{
		for (int i : faceList)
		{
			FSFace& face = pm->Face(i);
			RenderFEFace(face, pm);
		}
	}
	glEnd();
}

void GLMeshRender::RenderFEFacesOutline(FSMeshBase* pm, const std::vector<int>& faceList)
{
	glBegin(GL_LINES);
	for (int i : faceList)
	{
		FSFace& face = pm->Face(i);
		int ne = face.Edges();
		for (int j = 0; j < ne; ++j)
		{
			FSFace* pf = (face.m_nbr[j] >= 0 ? pm->FacePtr(face.m_nbr[j]) : 0);
			if ((pf == 0) || !pf->IsSelected() || !pf->IsVisible())
			{
				int jp1 = (j + 1) % ne;
				const vec3d& r1 = pm->Node(face.n[j]).r;
				const vec3d& r2 = pm->Node(face.n[jp1]).r;

				glVertex3d(r1.x, r1.y, r1.z);
				glVertex3d(r2.x, r2.y, r2.z);
			}
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderFEFacesOutline(FSCoreMesh* pm, const std::vector<FSFace*>& faceList)
{
	glBegin(GL_LINES);
	for (FSFace* pf : faceList)
	{
		FSFace& face = *pf;
		RenderFaceOutline(face, pm);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderFEFacesOutline(FSMeshBase* pm, std::function<bool(const FSFace& face)> f)
{
	glBegin(GL_LINES);
	for (int i = 0; i < pm->Faces(); i++)
	{
		FSFace& face = pm->Face(i);
		int ne = face.Edges();
		if (f(face))
		{
			for (int j = 0; j < ne; ++j)
			{
				FSFace* pf = (face.m_nbr[j] >= 0 ? pm->FacePtr(face.m_nbr[j]) : 0);
				if ((pf == 0) || !pf->IsSelected() || !pf->IsVisible())
				{
					int jp1 = (j + 1) % ne;
					const vec3d& r1 = pm->Node(face.n[j]).r;
					const vec3d& r2 = pm->Node(face.n[jp1]).r;

					glVertex3d(r1.x, r1.y, r1.z);
					glVertex3d(r2.x, r2.y, r2.z);
				}
			}
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderFace(FSFace& face, FSCoreMesh* pm)
{
	if (m_bShell2Solid)
	{
		if (pm->ElementRef(face.m_elem[0].eid).IsShell())
		{
			RenderThickShell(face, pm);
			return;
		}
	}
	if (m_ndivs == 1)
	{
		// Render the facet
		switch (face.m_type)
		{
		case FE_FACE_QUAD4: ::RenderQUAD4(pm, face); break;
		case FE_FACE_QUAD8: ::RenderQUAD8(pm, face); break;
		case FE_FACE_QUAD9: ::RenderQUAD9(pm, face); break;
		case FE_FACE_TRI3 : ::RenderTRI3 (pm, face); break;
		case FE_FACE_TRI6 : ::RenderTRI6 (pm, face); break;
		case FE_FACE_TRI7 : ::RenderTRI7 (pm, face); break;
		case FE_FACE_TRI10: ::RenderTRI10(pm, face); break;
		default:
			assert(false);
		}
	}
	else
	{
		// Render the facet
		switch (face.m_type)
		{
		case FE_FACE_QUAD4: RenderSmoothQUAD4(pm, face, m_ndivs); break;
		case FE_FACE_QUAD8: RenderSmoothQUAD8(pm, face, m_ndivs); break;
		case FE_FACE_QUAD9: RenderSmoothQUAD9(pm, face, m_ndivs); break;
		case FE_FACE_TRI3 : RenderSmoothTRI3 (pm, face, m_ndivs); break;
		case FE_FACE_TRI6 : RenderSmoothTRI6 (pm, face, m_ndivs); break;
		case FE_FACE_TRI7 : RenderSmoothTRI7 (pm, face, m_ndivs); break;
		case FE_FACE_TRI10: RenderSmoothTRI10(pm, face, m_ndivs); break;
		default:
			assert(false);
		}
	}
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderFace(FSFace& face, FSCoreMesh* pm, GLColor c[4])
{
	if (m_bShell2Solid)
	{
		if (pm->ElementRef(face.m_elem[0].eid).IsShell())
		{
			RenderThickShell(face, pm);
			return;
		}
	}

	RenderFESurfaceMeshFace(face, pm, c);
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderFESurfaceMeshFace(FSFace& face, FSMeshBase* pm, GLColor c[4])
{
	vec3d& r1 = pm->Node(face.n[0]).r;
	vec3d& r2 = pm->Node(face.n[1]).r;
	vec3d& r3 = pm->Node(face.n[2]).r;
	vec3d r4 = (face.n[3] >= 0 ? pm->Node(face.n[3]).r : r3);

	vec3f& n1 = face.m_nn[0];
	vec3f& n2 = face.m_nn[1];
	vec3f& n3 = face.m_nn[2];
	vec3f& n4 = face.m_nn[3];

	vec3f& fn = face.m_fn;

	float t[FSFace::MAX_NODES];
	pm->FaceNodeTexCoords(face, t);

	switch (face.m_type)
	{
	case FE_FACE_QUAD4:
	case FE_FACE_QUAD8:
	case FE_FACE_QUAD9:
		if (m_ndivs <= 1)
		{
			glNormal3f(n1.x, n1.y, n1.z); glColor4ub(c[0].r, c[0].g, c[0].b, c[0].a); glTexCoord1f(t[0]); glVertex3f(r1.x, r1.y, r1.z);
			glNormal3f(n2.x, n2.y, n2.z); glColor4ub(c[1].r, c[1].g, c[1].b, c[1].a); glTexCoord1f(t[1]); glVertex3f(r2.x, r2.y, r2.z);
			glNormal3f(n3.x, n3.y, n3.z); glColor4ub(c[2].r, c[2].g, c[2].b, c[2].a); glTexCoord1f(t[2]); glVertex3f(r3.x, r3.y, r3.z);

			glNormal3f(n3.x, n3.y, n3.z); glColor4ub(c[2].r, c[2].g, c[2].b, c[2].a); glTexCoord1f(t[2]); glVertex3f(r3.x, r3.y, r3.z);
			glNormal3f(n4.x, n4.y, n4.z); glColor4ub(c[3].r, c[3].g, c[3].b, c[3].a); glTexCoord1f(t[3]); glVertex3f(r4.x, r4.y, r4.z);
			glNormal3f(n1.x, n1.y, n1.z); glColor4ub(c[0].r, c[0].g, c[0].b, c[0].a); glTexCoord1f(t[0]); glVertex3f(r1.x, r1.y, r1.z);
		}
		else
		{
			RenderSmoothQUAD4(pm, face, m_ndivs);
		}
		break;
	case FE_FACE_TRI3:
	case FE_FACE_TRI6:
	case FE_FACE_TRI7:
	case FE_FACE_TRI10:
		glNormal3f(n1.x, n1.y, n1.z); glColor4ub(c[0].r, c[0].g, c[0].b, c[0].a); glTexCoord1f(t[0]); glVertex3f(r1.x, r1.y, r1.z);
		glNormal3f(n2.x, n2.y, n2.z); glColor4ub(c[1].r, c[1].g, c[1].b, c[1].a); glTexCoord1f(t[1]); glVertex3f(r2.x, r2.y, r2.z);
		glNormal3f(n3.x, n3.y, n3.z); glColor4ub(c[2].r, c[2].g, c[2].b, c[2].a); glTexCoord1f(t[2]); glVertex3f(r3.x, r3.y, r3.z);
		break;
	default:
		assert(false);
	}
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderFaceOutline(FSFace& face, FSCoreMesh* pm)
{
	if (m_bShell2Solid)
	{
		if (pm->ElementRef(face.m_elem[0].eid).IsShell())
		{
			RenderThickShellOutline(face, pm);
			return;
		}
	}

	GLboolean btex;
	glGetBooleanv(GL_TEXTURE_1D, &btex);
	glDisable(GL_TEXTURE_1D);

	glBegin(GL_LINE_LOOP);
	{
		// render the edges of the face
		switch (face.m_type)
		{
		case FE_FACE_TRI3:
		case FE_FACE_QUAD4: RenderFace1Outline(pm, face); break;
		case FE_FACE_TRI6:
		case FE_FACE_TRI7:
		case FE_FACE_QUAD8:
		case FE_FACE_QUAD9: RenderFace2Outline(pm, face, m_ndivs); break;
		case FE_FACE_TRI10: RenderFace3Outline(pm, face, m_ndivs); break;
		default:
			assert(false);
		}
	}
	glEnd();

	if (btex) glEnable(GL_TEXTURE_1D);
}

//-----------------------------------------------------------------------------
void GLMeshRender::RenderFEFace(const FSFace& face, FSMeshBase* pm)
{
	if (m_bShell2Solid)
	{
		FSCoreMesh* pcm = dynamic_cast<FSCoreMesh*>(pm);
		if (pcm) {
			RenderThickShell(face, pcm);
			return;
		}
	}

	if (m_ndivs == 1)
	{
		switch (face.Type())
		{
		case FE_FACE_TRI3 : ::RenderTRI3 (pm, face); break;
		case FE_FACE_TRI6 : ::RenderTRI6 (pm, face); break;
		case FE_FACE_TRI7 : ::RenderTRI7 (pm, face); break;
		case FE_FACE_TRI10: ::RenderTRI10(pm, face); break;
		case FE_FACE_QUAD4: ::RenderQUAD4(pm, face); break;
		case FE_FACE_QUAD8: ::RenderQUAD8(pm, face); break;
		case FE_FACE_QUAD9: ::RenderQUAD9(pm, face); break;
		default:
			assert(false);
		}
	}
	else
	{
		// Render the facet
		switch (face.m_type)
		{
		case FE_FACE_QUAD4: RenderSmoothQUAD4(pm, face, m_ndivs); break;
		case FE_FACE_QUAD8: RenderSmoothQUAD8(pm, face, m_ndivs); break;
		case FE_FACE_QUAD9: RenderSmoothQUAD9(pm, face, m_ndivs); break;
		case FE_FACE_TRI3 : RenderSmoothTRI3 (pm, face, m_ndivs); break;
		case FE_FACE_TRI6 : RenderSmoothTRI6 (pm, face, m_ndivs); break;
		case FE_FACE_TRI7 : RenderSmoothTRI7 (pm, face, m_ndivs); break;
		case FE_FACE_TRI10: RenderSmoothTRI10(pm, face, m_ndivs); break;
		default:
			assert(false);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
void GLMeshRender::RenderThickShell(const FSFace &face, FSCoreMesh* pm)
{
	switch (face.m_type)
	{
	case FE_FACE_QUAD4:
	case FE_FACE_QUAD8:
	case FE_FACE_QUAD9:
		RenderThickQuad(face, pm);
		break;
	case FE_FACE_TRI3:
	case FE_FACE_TRI6:
	case FE_FACE_TRI7:
	case FE_FACE_TRI10:
		RenderThickTri(face, pm);
		break;
	}
}

void GLMeshRender::RenderThickQuad(const FSFace &face, FSCoreMesh* pm)
{
	FEElement_& el = pm->ElementRef(face.m_elem[0].eid);
	double* h = el.m_h;

	vec3d r1 = pm->Node(face.n[0]).r;
	vec3d r2 = pm->Node(face.n[1]).r;
	vec3d r3 = pm->Node(face.n[2]).r;
	vec3d r4 = pm->Node(face.n[3]).r;

	vec3d n1 = to_vec3d(face.m_nn[0]);
	vec3d n2 = to_vec3d(face.m_nn[1]);
	vec3d n3 = to_vec3d(face.m_nn[2]);
	vec3d n4 = to_vec3d(face.m_nn[3]);

	vec3d r1a, r2a, r3a, r4a;
	vec3d r1b, r2b, r3b, r4b;

	switch (m_nshellref)
	{
	case 0:	// midsurface
		r1a = r1 - n1*(0.5f*h[0]); r1b = r1 + n1*(0.5f*h[0]);
		r2a = r2 - n2*(0.5f*h[1]); r2b = r2 + n2*(0.5f*h[1]);
		r3a = r3 - n3*(0.5f*h[2]); r3b = r3 + n3*(0.5f*h[2]);
		r4a = r4 - n4*(0.5f*h[3]); r4b = r4 + n4*(0.5f*h[3]);
		break;
	case 1: // top surface
		r1a = r1; r1b = r1 + n1*h[0];
		r2a = r2; r2b = r2 + n2*h[1];
		r3a = r3; r3b = r3 + n3*h[2];
		r4a = r4; r4b = r4 + n4*h[3];
		break;
	case 2: // bottom surface
		r1a = r1 - n1*h[0]; r1b = r1;
		r2a = r2 - n2*h[1]; r2b = r2;
		r3a = r3 - n3*h[2]; r3b = r3;
		r4a = r4 - n4*h[3]; r4b = r4;
		break;
	}

	vec3d m1 = (r2 - r1) ^ n1; m1.Normalize();
	vec3d m2 = (r3 - r2) ^ n1; m2.Normalize();
	vec3d m3 = (r4 - r3) ^ n1; m3.Normalize();
	vec3d m4 = (r1 - r4) ^ n1; m4.Normalize();

	vec3f fn = face.m_fn;

	float t1 = face.m_tex[0];
	float t2 = face.m_tex[1];
	float t3 = face.m_tex[2];
	float t4 = face.m_tex[3];

	// top face
	glx::vertex3d(r1b, n1, t1); glx::vertex3d(r2b, n2, t2); glx::vertex3d(r3b, n3, t3);
	glx::vertex3d(r3b, n3, t3); glx::vertex3d(r4b, n4, t4); glx::vertex3d(r1b, n1, t1);

	// bottom face
	glx::vertex3d(r4a, -n4, t4); glx::vertex3d(r3a, -n3, t3); glx::vertex3d(r2a, -n2, t2);
	glx::vertex3d(r2a, -n2, t2); glx::vertex3d(r1a, -n1, t1); glx::vertex3d(r4a, -n4, t4);

	// side faces
	if (face.m_nbr[0] == -1)
	{
		glx::vertex3d(r1a, m1, t1); glx::vertex3d(r2a, m1, t2); glx::vertex3d(r2b, m1, t2);
		glx::vertex3d(r2b, m1, t2); glx::vertex3d(r1b, m1, t1); glx::vertex3d(r1a, m1, t1);
	}

	if (face.m_nbr[1] == -1)
	{
		glx::vertex3d(r2a, m2, t2); glx::vertex3d(r3a, m2, t3); glx::vertex3d(r3b, m2, t3);
		glx::vertex3d(r3b, m2, t3); glx::vertex3d(r2b, m2, t2); glx::vertex3d(r2a, m2, t2);
	}

	if (face.m_nbr[2] == -1)
	{
		glx::vertex3d(r3a, m3, t3); glx::vertex3d(r4a, m3, t4); glx::vertex3d(r4b, m3, t4);
		glx::vertex3d(r4b, m3, t4); glx::vertex3d(r3b, m3, t3); glx::vertex3d(r3a, m3, t3);
	}

	if (face.m_nbr[3] == -1)
	{
		glx::vertex3d(r4a, m4, t4); glx::vertex3d(r1a, m4, t1); glx::vertex3d(r1b, m4, t1);
		glx::vertex3d(r1b, m4, t1); glx::vertex3d(r4b, m4, t4); glx::vertex3d(r4a, m4, t4);
	}
}

void GLMeshRender::RenderThickTri(const FSFace &face, FSCoreMesh* pm)
{
	FEElement_& el = pm->ElementRef(face.m_elem[0].eid);
	double* h = el.m_h;

	vec3d r1 = pm->Node(face.n[0]).r;
	vec3d r2 = pm->Node(face.n[1]).r;
	vec3d r3 = pm->Node(face.n[2]).r;

	//h[0] = (h[0] + h[1] + h[2])/3;
	//h[1] = h[0];
	//h[2] = h[0];
	vec3d n1 = to_vec3d(face.m_nn[0]);
	vec3d n2 = to_vec3d(face.m_nn[1]);
	vec3d n3 = to_vec3d(face.m_nn[2]);

	vec3d r1a, r2a, r3a;
	vec3d r1b, r2b, r3b;

	switch (m_nshellref)
	{
	case 0:	// midsurface
		r1a = r1 - n1*(0.5f*h[0]); r1b = r1 + n1*(0.5f*h[0]);
		r2a = r2 - n2*(0.5f*h[1]); r2b = r2 + n2*(0.5f*h[1]);
		r3a = r3 - n3*(0.5f*h[2]); r3b = r3 + n3*(0.5f*h[2]);
		break;
	case 1: // top surface
		r1a = r1; r1b = r1 + n1*h[0];
		r2a = r2; r2b = r2 + n2*h[1];
		r3a = r3; r3b = r3 + n3*h[2];
		break;
	case 2: // bottom surface
		r1a = r1 - n1*h[0]; r1b = r1;
		r2a = r2 - n2*h[1]; r2b = r2;
		r3a = r3 - n3*h[2]; r3b = r3;
		break;
	}

	vec3d m1 = (r2 - r1) ^ n1; m1.Normalize();
	vec3d m2 = (r3 - r2) ^ n1; m2.Normalize();
	vec3d m3 = (r1 - r3) ^ n1; m3.Normalize();

	vec3f fn = face.m_fn;

	float t1 = face.m_tex[0];
	float t2 = face.m_tex[1];
	float t3 = face.m_tex[2];

	// top face
	glx::vertex3d(r1b, n1, t1);
	glx::vertex3d(r2b, n2, t2);
	glx::vertex3d(r3b, n3, t3);

	// bottom face
	glx::vertex3d(r3a, -n3, t3);
	glx::vertex3d(r2a, -n2, t2);
	glx::vertex3d(r1a, -n1, t1);

	// side faces
	if (face.m_nbr[0] == -1)
	{
		glx::vertex3d(r1a, m1, t1); glx::vertex3d(r2a, m1, t2); glx::vertex3d(r2b, m1, t2);
		glx::vertex3d(r2b, m1, t2); glx::vertex3d(r1b, m1, t1); glx::vertex3d(r1a, m1, t1);
	}

	if (face.m_nbr[1] == -1)
	{
		glx::vertex3d(r2a, m2, t2); glx::vertex3d(r3a, m2, t3); glx::vertex3d(r3b, m2, t3);
		glx::vertex3d(r3b, m2, t3); glx::vertex3d(r2b, m2, t2); glx::vertex3d(r2a, m2, t2);
	}

	if (face.m_nbr[2] == -1)
	{
		glx::vertex3d(r3a, m3, t3); glx::vertex3d(r1a, m3, t1); glx::vertex3d(r1b, m3, t1);
		glx::vertex3d(r1b, m3, t1); glx::vertex3d(r3b, m3, t3); glx::vertex3d(r3a, m3, t3);
	}
}

void GLMeshRender::RenderThickShellOutline(FSFace &face, FSCoreMesh* pm)
{
	FEElement_& el = pm->ElementRef(face.m_elem[0].eid);
	double* h = el.m_h;

	vec3d r1 = pm->Node(face.n[0]).r;
	vec3d r2 = pm->Node(face.n[1]).r;
	vec3d r3 = pm->Node(face.n[2]).r;
	vec3d r4 = pm->Node(face.n[3]).r;

	vec3d n1 = to_vec3d(face.m_nn[0]);
	vec3d n2 = to_vec3d(face.m_nn[1]);
	vec3d n3 = to_vec3d(face.m_nn[2]);
	vec3d n4 = to_vec3d(face.m_nn[3]);

	vec3d r1a, r2a, r3a, r4a;
	vec3d r1b, r2b, r3b, r4b;

	switch (m_nshellref)
	{
	case 0:	// midsurface
		r1a = r1 - n1*(0.5f*h[0]); r1b = r1 + n1*(0.5f*h[0]);
		r2a = r2 - n2*(0.5f*h[1]); r2b = r2 + n2*(0.5f*h[1]);
		r3a = r3 - n3*(0.5f*h[2]); r3b = r3 + n3*(0.5f*h[2]);
		r4a = r4 - n4*(0.5f*h[3]); r4b = r4 + n4*(0.5f*h[3]);
		break;
	case 1: // top surface
		r1a = r1; r1b = r1 + n1*h[0];
		r2a = r2; r2b = r2 + n2*h[1];
		r3a = r3; r3b = r3 + n3*h[2];
		r4a = r4; r4b = r4 + n4*h[3];
		break;
	case 2: // bottom surface
		r1a = r1 - n1*h[0]; r1b = r1;
		r2a = r2 - n2*h[1]; r2b = r2;
		r3a = r3 - n3*h[2]; r3b = r3;
		r4a = r4 - n4*h[3]; r4b = r4;
		break;
	}

	GLboolean btex;
	glGetBooleanv(GL_TEXTURE_1D, &btex);
	glDisable(GL_TEXTURE_1D);

	switch (face.m_type)
	{
	case FE_FACE_QUAD4:
	case FE_FACE_QUAD8:
	case FE_FACE_QUAD9:
		glBegin(GL_LINES);
		{
			glVertex3f(r1a.x, r1a.y, r1a.z); glVertex3f(r2a.x, r2a.y, r2a.z);
			glVertex3f(r2a.x, r2a.y, r2a.z); glVertex3f(r3a.x, r3a.y, r3a.z);
			glVertex3f(r3a.x, r3a.y, r3a.z); glVertex3f(r4a.x, r4a.y, r4a.z);
			glVertex3f(r4a.x, r4a.y, r4a.z); glVertex3f(r1a.x, r1a.y, r1a.z);

			glVertex3f(r1b.x, r1b.y, r1b.z); glVertex3f(r2b.x, r2b.y, r2b.z);
			glVertex3f(r2b.x, r2b.y, r2b.z); glVertex3f(r3b.x, r3b.y, r3b.z);
			glVertex3f(r3b.x, r3b.y, r3b.z); glVertex3f(r4b.x, r4b.y, r4b.z);
			glVertex3f(r4b.x, r4b.y, r4b.z); glVertex3f(r1b.x, r1b.y, r1b.z);

			glVertex3f(r1a.x, r1a.y, r1a.z); glVertex3f(r1b.x, r1b.y, r1b.z);
			glVertex3f(r2a.x, r2a.y, r2a.z); glVertex3f(r2b.x, r2b.y, r2b.z);
			glVertex3f(r3a.x, r3a.y, r3a.z); glVertex3f(r3b.x, r3b.y, r3b.z);
			glVertex3f(r4a.x, r4a.y, r4a.z); glVertex3f(r4b.x, r4b.y, r4b.z);
		}
		glEnd();
		break;
	case FE_FACE_TRI3:
	case FE_FACE_TRI6:
	case FE_FACE_TRI7:
	case FE_FACE_TRI10:
		glBegin(GL_LINES);
		{
			glVertex3f(r1a.x, r1a.y, r1a.z); glVertex3f(r2a.x, r2a.y, r2a.z);
			glVertex3f(r2a.x, r2a.y, r2a.z); glVertex3f(r3a.x, r3a.y, r3a.z);
			glVertex3f(r3a.x, r3a.y, r3a.z); glVertex3f(r1a.x, r1a.y, r1a.z);

			glVertex3f(r1b.x, r1b.y, r1b.z); glVertex3f(r2b.x, r2b.y, r2b.z);
			glVertex3f(r2b.x, r2b.y, r2b.z); glVertex3f(r3b.x, r3b.y, r3b.z);
			glVertex3f(r3b.x, r3b.y, r3b.z); glVertex3f(r1b.x, r1b.y, r1b.z);

			glVertex3f(r1a.x, r1a.y, r1a.z); glVertex3f(r1b.x, r1b.y, r1b.z);
			glVertex3f(r2a.x, r2a.y, r2a.z); glVertex3f(r2b.x, r2b.y, r2b.z);
			glVertex3f(r3a.x, r3a.y, r3a.z); glVertex3f(r3b.x, r3b.y, r3b.z);
		}
		glEnd();
		break;
	default:
		assert(false);
	}

	if (btex) glEnable(GL_TEXTURE_1D);
}

//-----------------------------------------------------------------------------
void RenderQUAD4(FSMeshBase* pm, const FSFace& f)
{
	assert(f.m_type == FE_FACE_QUAD4);

	// get the nodal data
	vec3d r[4]; pm->FaceNodePosition(f, r);

	vec3f n[4];
	pm->FaceNodeNormals(f, n);

	float t[4];
	pm->FaceNodeTexCoords(f, t);

	// render the quads
	glx::quad4(r, n, t);
}

//-----------------------------------------------------------------------------
// Render a 8-noded quad
void RenderQUAD8(FSMeshBase* pm, const FSFace& f)
{
	assert(f.m_type == FE_FACE_QUAD8);

	// get the nodal data
	vec3d r[8]; pm->FaceNodePosition(f, r);

	vec3f n[8];
	pm->FaceNodeNormals(f, n);

	float t[8];
	pm->FaceNodeTexCoords(f, t);

	glx::quad8(r, n, t);
}

//-----------------------------------------------------------------------------
// Render a 9-noded quad
void RenderQUAD9(FSMeshBase* pm, const FSFace& f)
{
	assert(f.m_type == FE_FACE_QUAD9);

	// get the nodal data
	vec3d r[9]; pm->FaceNodePosition(f, r);

	vec3f n[9];
	pm->FaceNodeNormals(f, n);

	float t[9];
	pm->FaceNodeTexCoords(f, t);

	glx::quad9(r, n, t);
}

//-----------------------------------------------------------------------------
// Render a 3-noded tri
void RenderTRI3(FSMeshBase* pm, const FSFace& f)
{
	assert(f.m_type == FE_FACE_TRI3);
	vec3d r[3]; pm->FaceNodePosition(f, r);
	glx::tri3(r, f.m_nn, f.m_tex);
}

//-----------------------------------------------------------------------------
// Render a 6-noded tri
void RenderTRI6(FSMeshBase* pm, const FSFace& f)
{
	assert(f.m_type == FE_FACE_TRI6);

	// get the nodal data
	vec3d r[6]; pm->FaceNodePosition(f, r);

	vec3f n[6];
	pm->FaceNodeNormals(f, n);

	float t[6];
	pm->FaceNodeTexCoords(f, t);

	glx::tri6(r, n, t);
}

//-----------------------------------------------------------------------------
// Render a 7-noded tri
void RenderTRI7(FSMeshBase* pm, const FSFace& f)
{
	assert(f.m_type == FE_FACE_TRI7);

	// get the nodal data
	vec3d r[7]; pm->FaceNodePosition(f, r);

	vec3f n[7];
	pm->FaceNodeNormals(f, n);

	float t[7];
	pm->FaceNodeTexCoords(f, t);

	glx::tri7(r, n, t);
}

//-----------------------------------------------------------------------------
// Render a 10-noded tri
void RenderTRI10(FSMeshBase* pm, const FSFace& f)
{
	assert(f.m_type == FE_FACE_TRI10);

	// get the nodal data
	vec3d r[10]; pm->FaceNodePosition(f, r);

	vec3f n[10];
	pm->FaceNodeNormals(f, n);

	float t[10];
	pm->FaceNodeTexCoords(f, t);

	glx::tri10(r, n, t);
}

//-----------------------------------------------------------------------------
// Render a sub-divided 4-noded quadrilateral
void RenderSmoothQUAD4(FSMeshBase* pm, const FSFace& face, int ndivs)
{
	vec3d r[4] = {
		pm->Node(face.n[0]).r,
		pm->Node(face.n[1]).r,
		pm->Node(face.n[2]).r,
		pm->Node(face.n[3]).r
	};

	vec3f n[4] = { face.m_nn[0], face.m_nn[1], face.m_nn[2], face.m_nn[3] };

	float t[4];
	pm->FaceNodeTexCoords(face, t);

	glx::smoothQUAD4(r, n, t, ndivs);
}

//-----------------------------------------------------------------------------
// Render a sub-divided 8-noded quadrilateral
void RenderSmoothQUAD8(FSMeshBase* pm, const FSFace& face, int ndivs)
{
	assert(face.m_type == FE_FACE_QUAD8);

	vec3d r[8];
	r[0] = pm->Node(face.n[0]).r;
	r[1] = pm->Node(face.n[1]).r;
	r[2] = pm->Node(face.n[2]).r;
	r[3] = pm->Node(face.n[3]).r;
	r[4] = pm->Node(face.n[4]).r;
	r[5] = pm->Node(face.n[5]).r;
	r[6] = pm->Node(face.n[6]).r;
	r[7] = pm->Node(face.n[7]).r;

	vec3f n[8];
	n[0] = face.m_nn[0];
	n[1] = face.m_nn[1];
	n[2] = face.m_nn[2];
	n[3] = face.m_nn[3];
	n[4] = face.m_nn[4];
	n[5] = face.m_nn[5];
	n[6] = face.m_nn[6];
	n[7] = face.m_nn[7];

	float t[8];
	pm->FaceNodeTexCoords(face, t);

	glx::smoothQUAD8(r, n, t, ndivs);
}


//-----------------------------------------------------------------------------
// Render a sub-divided 9-noded quadrilateral
void RenderSmoothQUAD9(FSMeshBase* pm, const FSFace& face, int ndivs)
{
	assert(face.m_type == FE_FACE_QUAD9);

	vec3d r[9];
	r[0] = pm->Node(face.n[0]).r;
	r[1] = pm->Node(face.n[1]).r;
	r[2] = pm->Node(face.n[2]).r;
	r[3] = pm->Node(face.n[3]).r;
	r[4] = pm->Node(face.n[4]).r;
	r[5] = pm->Node(face.n[5]).r;
	r[6] = pm->Node(face.n[6]).r;
	r[7] = pm->Node(face.n[7]).r;
	r[8] = pm->Node(face.n[8]).r;

	vec3f n[9];
	n[0] = face.m_nn[0];
	n[1] = face.m_nn[1];
	n[2] = face.m_nn[2];
	n[3] = face.m_nn[3];
	n[4] = face.m_nn[4];
	n[5] = face.m_nn[5];
	n[6] = face.m_nn[6];
	n[7] = face.m_nn[7];
	n[8] = face.m_nn[8];

	float t[9];
	pm->FaceNodeTexCoords(face, t);

	glx::smoothQUAD9(r, n, t, ndivs);
}

//-----------------------------------------------------------------------------
// Render a sub-divided 6-noded triangle
void RenderSmoothTRI3(FSMeshBase* pm, const FSFace& face, int ndivs)
{
	RenderTRI3(pm, face);
}

//-----------------------------------------------------------------------------
// Render a sub-divided 6-noded triangle
void RenderSmoothTRI6(FSMeshBase* pm, const FSFace& face, int ndivs)
{
	assert(face.m_type == FE_FACE_TRI6);

	vec3d r[6];
	r[0] = pm->Node(face.n[0]).r;
	r[1] = pm->Node(face.n[1]).r;
	r[2] = pm->Node(face.n[2]).r;
	r[3] = pm->Node(face.n[3]).r;
	r[4] = pm->Node(face.n[4]).r;
	r[5] = pm->Node(face.n[5]).r;

	vec3f n[6];
	n[0] = face.m_nn[0];
	n[1] = face.m_nn[1];
	n[2] = face.m_nn[2];
	n[3] = face.m_nn[3];
	n[4] = face.m_nn[4];
	n[5] = face.m_nn[5];

	float t[6];
	pm->FaceNodeTexCoords(face, t);

	glx::smoothTRI6(r, n, t, ndivs);
}

//-----------------------------------------------------------------------------
// Render a sub-divided 7-noded triangle
void RenderSmoothTRI7(FSMeshBase* pm, const FSFace& face, int ndivs)
{
	assert(face.m_type == FE_FACE_TRI7);

	vec3d r[7];
	r[0] = pm->Node(face.n[0]).r;
	r[1] = pm->Node(face.n[1]).r;
	r[2] = pm->Node(face.n[2]).r;
	r[3] = pm->Node(face.n[3]).r;
	r[4] = pm->Node(face.n[4]).r;
	r[5] = pm->Node(face.n[5]).r;
	r[6] = pm->Node(face.n[6]).r;

	vec3f n[7];
	n[0] = face.m_nn[0];
	n[1] = face.m_nn[1];
	n[2] = face.m_nn[2];
	n[3] = face.m_nn[3];
	n[4] = face.m_nn[4];
	n[5] = face.m_nn[5];
	n[6] = face.m_nn[6];

	float t[7];
	pm->FaceNodeTexCoords(face, t);

	glx::smoothTRI7(r, n, t, ndivs);
}

//-----------------------------------------------------------------------------
// Render a sub-divided 10-noded triangle
void RenderSmoothTRI10(FSMeshBase* pm, const FSFace& face, int ndivs)
{
	assert(face.m_type == FE_FACE_TRI10);

	vec3d r[10];
	r[0] = pm->Node(face.n[0]).r;
	r[1] = pm->Node(face.n[1]).r;
	r[2] = pm->Node(face.n[2]).r;
	r[3] = pm->Node(face.n[3]).r;
	r[4] = pm->Node(face.n[4]).r;
	r[5] = pm->Node(face.n[5]).r;
	r[6] = pm->Node(face.n[6]).r;
	r[7] = pm->Node(face.n[7]).r;
	r[8] = pm->Node(face.n[8]).r;
	r[9] = pm->Node(face.n[9]).r;

	vec3f n[10];
	n[0] = face.m_nn[0];
	n[1] = face.m_nn[1];
	n[2] = face.m_nn[2];
	n[3] = face.m_nn[3];
	n[4] = face.m_nn[4];
	n[5] = face.m_nn[5];
	n[6] = face.m_nn[6];
	n[7] = face.m_nn[7];
	n[8] = face.m_nn[8];
	n[9] = face.m_nn[9];

	float t[10];
	pm->FaceNodeTexCoords(face, t);

	glx::smoothTRI10(r, n, t, ndivs);
}

void RenderFace1Outline(FSCoreMesh* pm, FSFace& face)
{
	int N = face.Nodes();
	for (int i = 0; i < N; ++i)
	{
		vec3d r = pm->Node(face.n[i]).r; glVertex3f(r.x, r.y, r.z);
	}
}

void RenderFace2Outline(FSCoreMesh* pm, FSFace& face, int ndivs)
{
	vec3f a[3];
	int NE = face.Edges();
	for (int i = 0; i<NE; ++i)
	{
		FSEdge e = face.GetEdge(i);
		a[0] = to_vec3f(pm->Node(e.n[0]).r);
		a[1] = to_vec3f(pm->Node(e.n[1]).r);
		a[2] = to_vec3f(pm->Node(e.n[2]).r);
		const int M = (ndivs < 2 ? 2 : ndivs);
		for (int n = 0; n<M; ++n)
		{
			double t = n / (double)M;
			vec3f p = e.eval(a, t);
			glVertex3d(p.x, p.y, p.z);
		}
	}
}

void RenderFace3Outline(FSCoreMesh* pm, FSFace& face, int ndivs)
{
	vec3f a[4];
	int NE = face.Edges();
	for (int i = 0; i<NE; ++i)
	{
		FSEdge e = face.GetEdge(i);
		a[0] = to_vec3f(pm->Node(e.n[0]).r);
		a[1] = to_vec3f(pm->Node(e.n[1]).r);
		a[2] = to_vec3f(pm->Node(e.n[2]).r);
		a[3] = to_vec3f(pm->Node(e.n[3]).r);
		const int M = (ndivs < 2 ? 3 : ndivs);
		for (int n = 0; n<M; ++n)
		{
			double t = n / (double)M;
			vec3f p = e.eval(a, t);
			glVertex3f(p.x, p.y, p.z);
		}
	}
}

void GLMeshRender::RenderNormals(FSMeshBase* pm, float scale, int ntag)
{
	// store the attributes
	glPushAttrib(GL_ENABLE_BIT);

	// disable lighting
	glDisable(GL_LIGHTING);

	glBegin(GL_LINES);
	{
		// render the normals
		for (int i = 0; i < pm->Faces(); ++i)
		{
			FSFace& face = pm->Face(i);
			if (face.m_ntag == ntag)
			{
				vec3d r1(0, 0, 0);
				vec3d fn = to_vec3d(face.m_fn);

				int n = face.Nodes();
				for (int j = 0; j < n; ++j) r1 += pm->Node(face.n[j]).r;
				r1 /= (double)n;

				GLfloat r = (GLfloat)fabs(fn.x);
				GLfloat g = (GLfloat)fabs(fn.y);
				GLfloat b = (GLfloat)fabs(fn.z);

				vec3d r2 = r1 + fn * scale;

				glx::line(r1, r2, GLColor::White(), GLColor::FromRGBf(r, g, b));
			}
		}
	}
	glEnd();

	// restore attributes
	glPopAttrib();
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
