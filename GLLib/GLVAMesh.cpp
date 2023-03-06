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
#include "GLVAMesh.h"
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

GLVAMesh::GLVAMesh()
{
	m_vr = nullptr;
	m_vn = nullptr;
	m_vt = nullptr;
	m_vc = nullptr;
	m_ind = nullptr;
	m_vertexCount = 0;
	m_maxVertexCount = 0;
	m_bvalid = false;
}

GLVAMesh::~GLVAMesh()
{
	Clear();
}

// clear all mesh data
void GLVAMesh::Clear()
{
	m_bvalid = false;
	m_vertexCount = 0;
	m_maxVertexCount = 0;
	delete[] m_vr;
	delete[] m_vn;
	delete[] m_vt;
	delete[] m_vc;
	delete[] m_ind;
}

void GLVAMesh::AllocVertexBuffers(int maxVertices, unsigned flags)
{
	m_bvalid = false;
	m_vertexCount = 0;
	if (flags & FLAG_VERTEX)
	{
		if ((m_vr == nullptr) || (maxVertices > m_maxVertexCount)) { delete[] m_vr; m_vr = new double[3 * maxVertices]; }
	}
	else { delete[] m_vr; m_vr = nullptr; }

	if (flags & FLAG_NORMAL)
	{
		if ((m_vn == nullptr) || (maxVertices > m_maxVertexCount)) { delete[] m_vn; m_vn = new double[3 * maxVertices]; }
	}
	else { delete[] m_vn; m_vn = nullptr; }

	if (flags & FLAG_TEXTURE)
	{
		if ((m_vt == nullptr) || (maxVertices > m_maxVertexCount)) { delete[] m_vt; m_vt = new double[3 * maxVertices]; }
	}
	else { delete[] m_vt; m_vt = nullptr; }

	if (flags & FLAG_COLOR)
	{
		if ((m_vc == nullptr) || (maxVertices > m_maxVertexCount)) { delete[] m_vc; m_vc = new ubyte[4 * maxVertices]; }
	}
	else { delete[] m_vc; m_vc = nullptr; }
	m_maxVertexCount = maxVertices;
}

void GLVAMesh::BeginMesh()
{
	m_vertexCount = 0;
	m_bvalid = false;
}

void GLVAMesh::EndMesh()
{
	m_bvalid = ((m_vr != nullptr) && (m_vertexCount != 0));
}

void GLVAMesh::CreateFromGMesh(const GMesh& gmsh)
{
	int faces = gmsh.Faces();
	AllocVertexBuffers(3 * faces, FLAG_VERTEX | FLAG_NORMAL | FLAG_COLOR);

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

void GLVAMesh::CreateFromGMesh(const GMesh& gmsh, int surfID, unsigned int flags)
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

void GLVAMesh::SetTransparency(ubyte a)
{
	if (m_vc == nullptr) return;
	for (int i = 0; i < m_vertexCount; ++i) m_vc[4 * i + 3] = a;
}

void GLVAMesh::ZSortFaces(const CGLCamera& cam)
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
		r[0] = vec3d(m_vr[9 * i    ], m_vr[9 * i + 1], m_vr[9 * i + 2]);
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
		m_ind[3*i  ] = 3*n;
		m_ind[3*i+1] = 3*n+1;
		m_ind[3*i+2] = 3*n+2;
	}

	m_bvalid = true;
}

void GLVAMesh::SortBackwards()
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

void GLVAMesh::SortForwards()
{
	if (m_bvalid == false) return;
	m_bvalid = false;
	delete[] m_ind;
	m_ind = nullptr;
	m_bvalid = true;
}

void GLVAMesh::Render()
{
	if (!m_bvalid) return;

	glEnableClientState(GL_VERTEX_ARRAY);
	if (m_vn) glEnableClientState(GL_NORMAL_ARRAY);
	if (m_vt) glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	if (m_vc) glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_DOUBLE, 0, m_vr);
	if (m_vn) glNormalPointer(GL_DOUBLE, 0, m_vn);
	if (m_vt) glTexCoordPointer(3, GL_DOUBLE, 0, m_vt);
	if (m_vc) glColorPointer(4, GL_UNSIGNED_BYTE, 0, m_vc);

	if (m_ind)
		glDrawElements(GL_TRIANGLES, m_vertexCount, GL_UNSIGNED_INT, m_ind);
	else
		glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);

	glDisableClientState(GL_VERTEX_ARRAY);
	if (m_vn) glDisableClientState(GL_NORMAL_ARRAY);
	if (m_vt) glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	if (m_vc) glDisableClientState(GL_COLOR_ARRAY);
}
