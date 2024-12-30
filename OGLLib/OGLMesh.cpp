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
#include "OGLMesh.h"
#include <GL/glew.h>
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
#include <GLLib/GLCamera.h>

OGLMesh::OGLMesh(unsigned int mode)
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
	m_useIndices = false;
	m_flags = 0;
	m_vbo[0] = m_vbo[1] = m_vbo[2] = m_vbo[3] = m_vbo[4] = 0;
	m_renderMode = VertexArrayMode;
	m_initVBO = false;
	m_refCount = 0;
	m_count = 0;
	m_start = 0;
}

OGLMesh::~OGLMesh()
{
	Clear();
}

void OGLMesh::incRef()
{
	m_refCount++;
}

void OGLMesh::decRef()
{
	m_refCount--;
	assert(m_refCount >= 0);
}

void OGLMesh::resetRef()
{
	m_refCount = 0;
}

int OGLMesh::refs() const
{
	return m_refCount;
}

void OGLMesh::SetRenderMode(OGLMesh::RenderMode mode)
{
	assert(m_bvalid == false);
	if (m_bvalid == false) m_renderMode = mode;
}

// clear all mesh data
void OGLMesh::Clear()
{
	m_bvalid = false;
	m_vertexCount = 0;
	m_maxVertexCount = 0;
	m_useIndices = false;
	delete[] m_vr; m_vr = nullptr;
	delete[] m_vn; m_vn = nullptr;
	delete[] m_vt; m_vt = nullptr;
	delete[] m_vc; m_vc = nullptr;
	delete[] m_ind; m_ind = nullptr;

	if (m_initVBO)
	{
		ClearVBO();
	}
}

void OGLMesh::AllocVertexBuffers(size_t maxVertices, unsigned flags)
{
	m_bvalid = false;
	m_flags = flags;
	m_initVBO = false;
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

	if (m_ind) { delete[] m_ind; m_ind = nullptr; }

	m_maxVertexCount = maxVertices;
}

void OGLMesh::BeginMesh()
{
	m_vertexCount = 0;
	m_bvalid = false;
}

void OGLMesh::EndMesh()
{
	m_bvalid = ((m_vr != nullptr) && (m_vertexCount != 0));
}

void OGLMesh::SetTransparency(ubyte a)
{
	if (m_vc == nullptr) return;
	for (int i = 0; i < m_vertexCount; ++i) m_vc[4 * i + 3] = a;
}

void OGLMesh::Render(unsigned int flags)
{
	if (!m_bvalid) return;

	m_start = 0;
	m_count = m_vertexCount;

	switch (m_renderMode)
	{
	case ImmediateMode  : RenderImmediate(flags); break;
	case VertexArrayMode: RenderVertexArrays(flags); break;
	case VBOMode        : RenderVBO(flags); break;
	}
}

void OGLMesh::Render(int nstart, int ncount, unsigned int flags)
{
	if (!m_bvalid) return;

	m_start = nstart;
	m_count = ncount;

	switch (m_renderMode)
	{
	case ImmediateMode  : RenderImmediate(flags); break;
	case VertexArrayMode: RenderVertexArrays(flags); break;
	case VBOMode        : RenderVBO(flags); break;
	}
}

void OGLMesh::RenderImmediate(unsigned int flags)
{
	glBegin(m_mode);
	{
		int i0 = m_start;
		int i1 = m_start + m_count;
		for (int i = i0; i < i1; ++i)
		{
			if (m_vn && (flags & FLAG_NORMAL )) glNormal3fv(m_vn + 3 * i);
			if (m_vt && (flags & FLAG_TEXTURE)) glTexCoord3fv(m_vt + 3 * i);
			if (m_vc && (flags & FLAG_COLOR  )) glColor4ubv(m_vc + 4 * i);
			glVertex3fv(m_vr + 3 * i);
		}
	}
	glEnd();
}

void OGLMesh::RenderVertexArrays(unsigned int flags)
{
	glEnableClientState(GL_VERTEX_ARRAY);
	if (m_vn && (flags & FLAG_NORMAL )) glEnableClientState(GL_NORMAL_ARRAY);
	if (m_vt && (flags & FLAG_TEXTURE)) glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	if (m_vc && (flags & FLAG_COLOR  )) glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, m_vr);
	if (m_vn && (flags & FLAG_NORMAL )) glNormalPointer(GL_FLOAT, 0, m_vn);
	if (m_vt && (flags & FLAG_TEXTURE)) glTexCoordPointer(3, GL_FLOAT, 0, m_vt);
	if (m_vc && (flags & FLAG_COLOR  )) glColorPointer(4, GL_UNSIGNED_BYTE, 0, m_vc);

	if (m_ind)
		glDrawElements(m_mode, m_vertexCount, GL_UNSIGNED_INT, m_ind);
	else
		glDrawArrays(m_mode, m_start, m_count);

	glDisableClientState(GL_VERTEX_ARRAY);
	if (m_vn && (flags & FLAG_NORMAL )) glDisableClientState(GL_NORMAL_ARRAY);
	if (m_vt && (flags & FLAG_TEXTURE)) glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	if (m_vc && (flags & FLAG_COLOR  )) glDisableClientState(GL_COLOR_ARRAY);
}

void OGLMesh::RenderVBO(unsigned int flags)
{
	if (m_initVBO == false) InitVBO();
	if (m_initVBO == false) return;

	bool useNormals  = (m_flags & FLAG_NORMAL ) && (flags & FLAG_NORMAL);
	bool useTexCoord = (m_flags & FLAG_TEXTURE) && (flags & FLAG_TEXTURE);
	bool useColors   = (m_flags & FLAG_COLOR  ) && (flags & FLAG_COLOR);

	glEnableClientState(GL_VERTEX_ARRAY);
	if (useNormals ) glEnableClientState(GL_NORMAL_ARRAY);
	if (useTexCoord) glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	if (useColors  ) glEnableClientState(GL_COLOR_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[VERTEX_DATA]);
	glVertexPointer(3, GL_FLOAT, 0, 0);

	if (useNormals) {
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo[NORMAL_DATA]);
		glNormalPointer(GL_FLOAT, 0, 0);
	}

	if (useTexCoord) {
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo[TEXTURE_DATA]);
		glTexCoordPointer(3, GL_FLOAT, 0, 0);
	}

	if (useColors) {
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo[COLOR_DATA]);
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, 0);
	}

	if (m_useIndices)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[INDEX_DATA]);
		glDrawElements(m_mode, m_vertexCount, GL_UNSIGNED_INT, 0);
	}
	else
		glDrawArrays(m_mode, m_start, m_count);


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	if (m_useIndices) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableClientState(GL_VERTEX_ARRAY);
	if (useNormals ) glDisableClientState(GL_NORMAL_ARRAY);
	if (useTexCoord) glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	if (useColors  ) glDisableClientState(GL_COLOR_ARRAY);
}

void OGLMesh::ClearVBO()
{
	// NOTE: This assumes the current GL context is active
	if (m_vbo[0] != 0)
	{
		glDeleteBuffers(5, m_vbo);
		m_vbo[0] = m_vbo[1] = m_vbo[2] = m_vbo[3] = m_vbo[4] = 0;
	}
}

void OGLMesh::InitVBO()
{
	if (m_initVBO || (m_bvalid == false)) return;

	// first, see if we need to cleanup some old buffers
	ClearVBO();

	// generate 4 buffer objects
	glGenBuffers(5, m_vbo);

	// copy data to buffers
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[VERTEX_DATA]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_vertexCount * 3, m_vr, GL_STATIC_DRAW);

	if (m_vn)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo[NORMAL_DATA]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_vertexCount * 3, m_vn, GL_STATIC_DRAW);
	}

	if (m_vt)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo[TEXTURE_DATA]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_vertexCount * 3, m_vt, GL_STATIC_DRAW);
	}

	if (m_vc)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo[COLOR_DATA]);
		glBufferData(GL_ARRAY_BUFFER, m_vertexCount * 4, m_vc, GL_STATIC_DRAW);
	}

	if (m_ind)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo[INDEX_DATA]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(unsigned int) * m_vertexCount, m_ind, GL_STATIC_DRAW);
	}

	// unbind buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	m_initVBO = true;

	// delete memory
	delete[] m_vr; m_vr = nullptr;
	if (m_vn) { delete[] m_vn; m_vn = nullptr; }
	if (m_vt) { delete[] m_vt; m_vt = nullptr; }
	if (m_vc) { delete[] m_vc; m_vc = nullptr; }
}

//===================================================================================
OGLTriMesh::OGLTriMesh() : OGLMesh(GL_TRIANGLES) {}

void OGLTriMesh::Create(size_t maxTriangles, unsigned int flags)
{
	AllocVertexBuffers(3 * maxTriangles, flags);
}

void OGLTriMesh::ZSortFaces(const GLCamera& cam)
{
	// not sure how to do this for VBOs
	if (m_renderMode == VBOMode) return;

	if (m_bvalid == false) return;
	m_bvalid = false;
	delete[] m_ind;

	unsigned int faces = m_vertexCount / 3; assert((m_vertexCount % 3) == 0);

	// first, build a list of faces
	vec3d r[3];
	std::vector<pair<double, int> > zmap(faces);
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
		zmap[i] = pair<double, int>(q.z, i);
	}

	// sort it
	std::sort(zmap.begin(), zmap.end(), [](const pair<double, int>& a, const pair<double, int>& b) { return a.first < b.first; });

	// build the new index list
	m_ind = new unsigned int[3 * faces];
	std::vector< pair<double, int> >::iterator it = zmap.begin();
	for (int i = 0; i < faces; ++i, ++it)
	{
		int n = it->second;
		m_ind[3 * i] = 3 * n;
		m_ind[3 * i + 1] = 3 * n + 1;
		m_ind[3 * i + 2] = 3 * n + 2;
	}
	m_useIndices = true;
	m_bvalid = true;
}

void OGLTriMesh::SortBackwards()
{
	// not sure how to do this for VBOs
	if (m_renderMode == VBOMode) return;

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

	m_useIndices = true;
	m_bvalid = true;
}

void OGLTriMesh::SortForwards()
{
	// not sure how to do this for VBOs
	if (m_renderMode == VBOMode) return;

	if (m_bvalid == false) return;
	m_bvalid = false;
	delete[] m_ind;
	m_ind = nullptr;
	m_useIndices = false;
	m_bvalid = true;
}


void OGLTriMesh::CreateFromGMesh(const GMesh& gmsh, unsigned int flags)
{
	int faces = gmsh.Faces();
	AllocVertexBuffers(3 * faces, flags);

	BeginMesh();
	for (int i = 0; i < gmsh.Faces(); ++i)
	{
		auto& tri = gmsh.Face(i);
		for (int j = 0; j < 3; ++j)
		{
			auto& vj = gmsh.Node(tri.n[j]);
			AddVertex(vj.r, tri.vn[j], tri.c[j], tri.t[j]);
		}
	}
	EndMesh();
}

void OGLTriMesh::CreateFromGMesh(const GMesh& gmsh, int surfID, unsigned int flags)
{
	if ((surfID < 0) || (surfID >= gmsh.Partitions())) { assert(false); return; }

	const GMesh::PARTITION& part = gmsh.Partition(surfID);
	int faces = part.nf;
	AllocVertexBuffers(3 * faces, flags);

	BeginMesh();
	for (int i = 0; i < faces; ++i)
	{
		const GMesh::FACE& f = gmsh.Face(i + part.n0);
		assert(f.pid == surfID);
		for (int j = 0; j < 3; ++j)
		{
			auto& vj = gmsh.Node(f.n[j]);
			AddVertex(vj.r, f.vn[j], f.c[j]);
		}
	}
	EndMesh();
}

//===================================================================================
OGLQuadMesh::OGLQuadMesh() : OGLMesh(GL_QUADS) {}

void OGLQuadMesh::Create(int maxQuads, unsigned int flags)
{
	AllocVertexBuffers(4 * maxQuads, flags);
}

//===================================================================================
OGLLineMesh::OGLLineMesh() : OGLMesh(GL_LINES) {}

OGLLineMesh::OGLLineMesh(int maxLines, unsigned int flags) : OGLMesh(GL_LINES)
{
	AllocVertexBuffers(2*maxLines, flags);
}

void OGLLineMesh::Create(int maxLines, unsigned int flags)
{
	AllocVertexBuffers(2 * maxLines, flags);
}

void OGLLineMesh::CreateFromGMesh(const GMesh& gmsh, unsigned int flags)
{
	int edges = gmsh.Edges();
	AllocVertexBuffers(2 * edges, flags);

	BeginMesh();
	for (int i = 0; i < gmsh.Edges(); ++i)
	{
		auto& edge = gmsh.Edge(i);
		for (int j = 0; j < 2; ++j)
		{
			auto& vj = gmsh.Node(edge.n[j]);
			AddVertex(vj.r, edge.c[j]);
		}
	}
	EndMesh();
}

//===================================================================================
OGLPointMesh::OGLPointMesh() : OGLMesh(GL_POINTS) {}

OGLPointMesh::OGLPointMesh(int maxVertices, unsigned int flags) : OGLMesh(GL_POINTS)
{
	AllocVertexBuffers(maxVertices, flags);
}

void OGLPointMesh::Create(int maxVertices, unsigned int flags)
{
	AllocVertexBuffers(maxVertices, flags);
}

void OGLPointMesh::CreateFromGMesh(const GMesh& gmsh)
{
	int nodes = gmsh.Nodes();
	AllocVertexBuffers(nodes, 0);

	BeginMesh();
	for (int i = 0; i < gmsh.Nodes(); ++i)
	{
		const GMesh::NODE& node = gmsh.Node(i);
		AddVertex(node.r);
	}
	EndMesh();
}
