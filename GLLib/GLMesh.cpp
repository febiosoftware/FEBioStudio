/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2023 University of Utah, The Trustees of Columbia University in
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
#include "GLMesh.h"
#ifdef WIN32
#include <Windows.h>
#include <gl/GL.h>
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#endif
#ifdef LINUX
#include <GL/gl.h>
#endif
#include <MeshLib/GMesh.h>
#include <algorithm>
#include "GLCamera.h"

GLMesh::GLMesh(unsigned int mode)
{
	m_mode = mode;
	m_vr = nullptr;
	m_vn = nullptr;
	m_vt = nullptr;
	m_vc = nullptr;
	m_ind = nullptr;
	m_vertexCount = 0;
	m_maxVertexCount = 0;
	m_bvalid = false;
}

GLMesh::~GLMesh()
{
	Clear();
}

// clear all mesh data
void GLMesh::Clear()
{
	m_bvalid = false;
	m_vertexCount = 0;
	m_maxVertexCount = 0;
	delete[] m_vr; m_vr = nullptr;
	delete[] m_vn; m_vn = nullptr;
	delete[] m_vt; m_vt = nullptr;
	delete[] m_vc; m_vc = nullptr;
	delete[] m_ind; m_ind = nullptr;
}

void GLMesh::AllocVertexBuffers(size_t maxVertices, unsigned flags)
{
	m_bvalid = false;
	m_vertexCount = 0;
	if ((m_vr == nullptr) || (maxVertices > m_maxVertexCount)) { delete[] m_vr; m_vr = new float[3 * maxVertices]; }

	if (flags & FLAG_NORMAL)
	{
		if ((m_vn == nullptr) || (maxVertices > m_maxVertexCount)) { delete[] m_vn; m_vn = new float[3 * maxVertices]; }
	}
	else { delete[] m_vn; m_vn = nullptr; }

	if (flags & FLAG_TEXTURE)
	{
		if ((m_vt == nullptr) || (maxVertices > m_maxVertexCount)) { delete[] m_vt; m_vt = new float[3 * maxVertices]; }
	}
	else { delete[] m_vt; m_vt = nullptr; }

	if (flags & FLAG_COLOR)
	{
		if ((m_vc == nullptr) || (maxVertices > m_maxVertexCount)) { delete[] m_vc; m_vc = new ubyte[4 * maxVertices]; }
	}
	else { delete[] m_vc; m_vc = nullptr; }
	m_maxVertexCount = maxVertices;
}

void GLMesh::BeginMesh()
{
	m_vertexCount = 0;
	m_bvalid = false;
}

void GLMesh::EndMesh()
{
	m_bvalid = ((m_vr != nullptr) && (m_vertexCount != 0));
}

void GLMesh::CreateFromGMesh(const GMesh& gmsh)
{
	int faces = gmsh.Faces();
	AllocVertexBuffers(3 * faces, FLAG_NORMAL | FLAG_COLOR);

	BeginMesh();
	for (int i = 0; i < gmsh.Faces(); ++i)
	{
		auto& tri = gmsh.Face(i);
		for (int j = 0; j < 3; ++j)
		{
			auto& vj = gmsh.Node(tri.n[j]);
			AddVertex(vj.r, tri.nn[j], tri.c[j]);
		}
	}
	EndMesh();
}

void GLMesh::CreateFromGMesh(const GMesh& gmsh, int surfID, unsigned int flags)
{
	if ((surfID < 0) || (surfID >= gmsh.m_FIL.size())) { assert(false); return; }

	pair<int, int> fil = gmsh.m_FIL[surfID];
	int faces = fil.second;
	AllocVertexBuffers(3 * faces, flags);

	BeginMesh();
	for (int i = 0; i < faces; ++i)
	{
		const GMesh::FACE& f = gmsh.Face(i + fil.first);
		assert(f.pid == surfID);
		for (int j = 0; j < 3; ++j)
		{
			auto& vj = gmsh.Node(f.n[j]);
			AddVertex(vj.r, f.nn[j], f.c[j]);
		}
	}
	EndMesh();
}

void GLMesh::SetTransparency(ubyte a)
{
	if (m_vc == nullptr) return;
	for (int i = 0; i < m_vertexCount; ++i) m_vc[4 * i + 3] = a;
}

void GLMesh::Render()
{
	if (!m_bvalid) return;

	glEnableClientState(GL_VERTEX_ARRAY);
	if (m_vn) glEnableClientState(GL_NORMAL_ARRAY);
	if (m_vt) glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	if (m_vc) glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, m_vr);
	if (m_vn) glNormalPointer(GL_FLOAT, 0, m_vn);
	if (m_vt) glTexCoordPointer(3, GL_FLOAT, 0, m_vt);
	if (m_vc) glColorPointer(4, GL_UNSIGNED_BYTE, 0, m_vc);

	if (m_ind)
		glDrawElements(m_mode, m_vertexCount, GL_UNSIGNED_INT, m_ind);
	else
		glDrawArrays(m_mode, 0, m_vertexCount);

	glDisableClientState(GL_VERTEX_ARRAY);
	if (m_vn) glDisableClientState(GL_NORMAL_ARRAY);
	if (m_vt) glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	if (m_vc) glDisableClientState(GL_COLOR_ARRAY);
}

//===================================================================================
GLTriMesh::GLTriMesh() : GLMesh(GL_TRIANGLES) {}

void GLTriMesh::Create(size_t maxTriangles, unsigned int flags)
{
	AllocVertexBuffers(3 * maxTriangles, flags);
}

void GLTriMesh::ZSortFaces(const CGLCamera& cam)
{
	if (m_bvalid == false) return;
	m_bvalid = false;
	delete[] m_ind;

	unsigned int faces = m_vertexCount / 3; assert((m_vertexCount % 3) == 0);
	vector< pair<int, double> > zlist; zlist.reserve(faces);

	// first, build a list of faces
	vec3d r[3];
	for (int i = 0; i < faces; ++i)
	{
		r[0] = vec3d(m_vr[9 * i], m_vr[9 * i + 1], m_vr[9 * i + 2]);
		r[1] = vec3d(m_vr[9 * i + 3], m_vr[9 * i + 4], m_vr[9 * i + 5]);
		r[2] = vec3d(m_vr[9 * i + 6], m_vr[9 * i + 7], m_vr[9 * i + 8]);

		// get the face center
		vec3d o = (r[0] + r[1] + r[2]) / 3.0;

		// convert to eye coordinates
		vec3d q = cam.WorldToCam(o);

		// add it to the z-list
		zlist.push_back(pair<int, double>(i, q.z));
	}

	// sort the zlist
	std::sort(zlist.begin(), zlist.end(), [](pair<int, double>& a, pair<int, double>& b) {
		return a.second < b.second;
		});

	// build the new index list
	m_ind = new unsigned int[3 * faces];
	for (int i = 0; i < faces; ++i)
	{
		int n = zlist[i].first;
		m_ind[3 * i] = 3 * n;
		m_ind[3 * i + 1] = 3 * n + 1;
		m_ind[3 * i + 2] = 3 * n + 2;
	}

	m_bvalid = true;
}

void GLTriMesh::SortBackwards()
{
	if (m_bvalid == false) return;
	m_bvalid = false;
	delete[] m_ind;

	unsigned int faces = m_vertexCount / 3; assert((m_vertexCount % 3) == 0);

	// build the new index list
	m_ind = new unsigned int[3 * faces];
	for (int i = 0; i < faces; ++i)
	{
		int n = faces - i - 1;
		m_ind[3 * i] = 3 * n;
		m_ind[3 * i + 1] = 3 * n + 1;
		m_ind[3 * i + 2] = 3 * n + 2;
	}

	m_bvalid = true;
}

void GLTriMesh::SortForwards()
{
	if (m_bvalid == false) return;
	m_bvalid = false;
	delete[] m_ind;
	m_ind = nullptr;
	m_bvalid = true;
}

//===================================================================================
GLQuadMesh::GLQuadMesh() : GLMesh(GL_QUADS) {}

void GLQuadMesh::Create(int maxQuads, unsigned int flags)
{
	AllocVertexBuffers(4 * maxQuads, flags);
}

//===================================================================================
GLLineMesh::GLLineMesh() : GLMesh(GL_LINES) {}

GLLineMesh::GLLineMesh(int maxLines, unsigned int flags) : GLMesh(GL_LINES) 
{
	AllocVertexBuffers(2*maxLines, flags);
}

void GLLineMesh::Create(int maxLines, unsigned int flags)
{
	AllocVertexBuffers(2 * maxLines, flags);
}

//===================================================================================
GLPointMesh::GLPointMesh() : GLMesh(GL_POINTS) {}

GLPointMesh::GLPointMesh(int maxVertices, unsigned int flags) : GLMesh(GL_POINTS)
{
	AllocVertexBuffers(maxVertices, flags);
}

void GLPointMesh::Create(int maxVertices, unsigned int flags)
{
	AllocVertexBuffers(maxVertices, flags);
}
