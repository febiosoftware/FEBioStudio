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

#include "GLMesh.h"
#include <stack>
#include <algorithm>
#include <assert.h>

using std::stack;

GLMesh::GLMesh(void)
{
	m_hasNeighborList = false;
}

GLMesh::~GLMesh(void)
{
}

void GLMesh::Create(int nodes, int faces, int edges)
{
	if (nodes > 0) m_Node.resize(nodes);
	if (faces > 0) m_Face.resize(faces);
	if (edges > 0) m_Edge.resize(edges);
	m_FIL.clear();
	m_EIL.clear();
	setModified(true);
	m_hasNeighborList = false;
}

void GLMesh::Clear()
{
	m_Node.clear();
	m_Edge.clear();
	m_Face.clear();
	m_FIL.clear();
	m_EIL.clear();
	setModified(true);
	m_hasNeighborList = false;
}

int GLMesh::AddNode(const vec3f& r, int gid)
{
	NODE v;
	v.r = r;
	v.pid = gid;
	v.nid = -1;
	m_Node.push_back(v);
	setModified(true);
	m_hasNeighborList = false;
	return ((int)m_Node.size() - 1);
}

int GLMesh::AddNode(const vec3f& r, int nodeID, int gid)
{
	NODE v;
	v.r = r;
	v.pid = gid;
	v.nid = nodeID;
	m_Node.push_back(v);
	setModified(true);
	m_hasNeighborList = false;
	return ((int)m_Node.size() - 1);
}

int	GLMesh::AddNode(const vec3f& r, GLColor c)
{
	NODE v;
	v.r = r;
	v.c = c;
	v.pid = 0;
	v.nid = 0;
	m_Node.push_back(v);
	setModified(true);
	m_hasNeighborList = false;
	return ((int)m_Node.size() - 1);
}

void GLMesh::AddEdge(int* n, int nodes, int gid)
{
	EDGE e;
	if (nodes == 2)
	{
		e.n[0] = n[0];
		e.n[1] = n[1];
		e.pid = gid;
		m_Edge.push_back(e);
	}
	else if (nodes == 3)
	{
		e.n[0] = n[0];
		e.n[1] = n[2];
		e.pid = gid;
		m_Edge.push_back(e);

		e.n[0] = n[2];
		e.n[1] = n[1];
		e.pid = gid;
		m_Edge.push_back(e);
	}
	else if (nodes == 4)
	{
		e.n[0] = n[0];
		e.n[1] = n[2];
		e.pid = gid;
		m_Edge.push_back(e);

		e.n[0] = n[2];
		e.n[1] = n[3];
		e.pid = gid;
		m_Edge.push_back(e);

		e.n[0] = n[3];
		e.n[1] = n[1];
		e.pid = gid;
		m_Edge.push_back(e);
	}
	else assert(false);
	setModified(true);
	m_hasNeighborList = false;
}

void GLMesh::AddEdge(vec3f* r, int nodes, int gid)
{
	EDGE e;
	if (nodes == 2)
	{
		e.n[0] = AddNode(r[0]);
		e.n[1] = AddNode(r[1]);
		e.vr[0] = r[0];
		e.vr[1] = r[1];
		e.pid = gid;
		m_Edge.push_back(e);
	}
	else if (nodes == 3)
	{
		e.n[0] = AddNode(r[0]);
		e.n[1] = AddNode(r[2]);
		e.vr[0] = r[0];
		e.vr[1] = r[2];
		e.pid = gid;
		m_Edge.push_back(e);

		e.n[0] = AddNode(r[2]);
		e.n[1] = AddNode(r[1]);
		e.vr[0] = r[2];
		e.vr[1] = r[1];
		e.pid = gid;
		m_Edge.push_back(e);
	}
	else if (nodes == 4)
	{
		e.n[0] = AddNode(r[0]);
		e.n[1] = AddNode(r[2]);
		e.vr[0] = r[0];
		e.vr[1] = r[2];
		e.pid = gid;
		m_Edge.push_back(e);

		e.n[0] = AddNode(r[2]);
		e.n[1] = AddNode(r[3]);
		e.vr[0] = r[2];
		e.vr[1] = r[3];
		e.pid = gid;
		m_Edge.push_back(e);

		e.n[0] = AddNode(r[3]);
		e.n[1] = AddNode(r[1]);
		e.vr[0] = r[3];
		e.vr[1] = r[1];
		e.pid = gid;
		m_Edge.push_back(e);
	}
	else assert(false);
	setModified(true);
	m_hasNeighborList = false;
}

void GLMesh::AddEdge(vec3f r[2], GLColor c)
{
	EDGE e;
	e.n[0] = AddNode(r[0]);
	e.n[1] = AddNode(r[1]);
	e.vr[0] = r[0];
	e.vr[1] = r[1];
	e.c[0] = e.c[1] = c;
	e.pid = 0;
	m_Edge.push_back(e);
	setModified(true);
	m_hasNeighborList = false;
}

void GLMesh::AddEdge(vec3f r[2], GLColor c[2])
{
	EDGE e;
	e.n[0] = AddNode(r[0]);
	e.n[1] = AddNode(r[1]);
	e.vr[0] = r[0];
	e.vr[1] = r[1];
	e.c[0] = c[0];
	e.c[1] = c[1];
	e.pid = 0;
	m_Edge.push_back(e);
	setModified(true);
	m_hasNeighborList = false;
}

void GLMesh::AddEdge(const vec3f& a, const vec3f& b)
{
	EDGE e;
	e.n[0] = AddNode(a);
	e.n[1] = AddNode(b);
	e.vr[0] = a;
	e.vr[1] = b;
	e.pid = 0;
	m_Edge.push_back(e);
	setModified(true);
	m_hasNeighborList = false;
}

int GLMesh::AddFace(const GLMesh::FACE& face)
{
	if (m_FIL.empty()) NewPartition();
	auto& fil = m_FIL.back();
	fil.nf++;
	m_Face.push_back(face);
	setModified(true);
	m_hasNeighborList = false;
	return ((int)m_Face.size() - 1);
}

void GLMesh::NewPartition(int tag)
{
	PARTITION p;
	p.n0 = m_Face.size();
	p.nf = 0;
	p.tag = tag;
	m_FIL.push_back(p);
}

int GLMesh::AddFace(int n0, int n1, int n2, int groupID, int smoothID, bool bext, int faceId, int elemId, int mat)
{
	FACE f;
	f.n[0] = n0;
	f.n[1] = n1;
	f.n[2] = n2;
	f.c[0] = GLColor(0, 0, 0);
	f.c[1] = GLColor(0, 0, 0);
	f.c[2] = GLColor(0, 0, 0);
	f.pid = groupID;
	f.sid = smoothID;
	f.bext = bext;
	f.fid = faceId;
	f.eid = elemId;
	f.mid = mat;
	setModified(true);
	return AddFace(f);
}

void GLMesh::AddFace(const int* n, int nodes, int groupID, int smoothID, bool bext, int faceId, int elemId, int mat)
{
	switch (nodes)
	{
	case 3: // TRI3
		{
			AddFace(n[0], n[1], n[2], groupID, smoothID, bext, faceId, elemId, mat);
		}
		break;
	case 4: // QUAD4
		{
			AddFace(n[2], n[3], n[0], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[0], n[1], n[2], groupID, smoothID, bext, faceId, elemId, mat);
		}
		break;
	case 6: // TRI6
		{
			AddFace(n[0], n[3], n[5], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[1], n[4], n[3], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[2], n[5], n[4], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[3], n[4], n[5], groupID, smoothID, bext, faceId, elemId, mat);
		}
		break;
	case 7: // TRI7
		{
			AddFace(n[0], n[3], n[6], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[1], n[6], n[3], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[1], n[4], n[6], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[2], n[6], n[4], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[2], n[5], n[6], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[0], n[6], n[5], groupID, smoothID, bext, faceId, elemId, mat);
		}
		break;
	case 8: // QUAD8
		{
			AddFace(n[0], n[4], n[7], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[4], n[1], n[5], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[5], n[2], n[6], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[6], n[3], n[7], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[5], n[6], n[7], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[4], n[5], n[7], groupID, smoothID, bext, faceId, elemId, mat);
		}
		break;
	case 9: // QUAD9
		{
			AddFace(n[0], n[4], n[7], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[4], n[1], n[5], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[5], n[2], n[6], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[6], n[3], n[7], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[4], n[8], n[7], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[4], n[5], n[8], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[8], n[5], n[6], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[8], n[6], n[7], groupID, smoothID, bext, faceId, elemId, mat);
		}
		break;
	case 10: // TRI10
		{
			AddFace(n[0], n[3], n[7], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[1], n[5], n[4], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[2], n[8], n[6], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[9], n[7], n[3], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[9], n[3], n[4], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[9], n[4], n[5], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[9], n[5], n[6], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[9], n[6], n[8], groupID, smoothID, bext, faceId, elemId, mat);
			AddFace(n[9], n[8], n[7], groupID, smoothID, bext, faceId, elemId, mat);
		}
		break;
	default:
		assert(false);
	}
}

void GLMesh::AddFace(vec3f* r, int gid, int smoothId, bool bext)
{
	int n[3];
	n[0] = AddNode(r[0]);
	n[1] = AddNode(r[1]);
	n[2] = AddNode(r[2]);

	AddFace(n, 3, gid, smoothId, bext);
}

void GLMesh::AddFace(vec3f r[3], GLColor c)
{
	int n0 = AddNode(r[0]);
	int n1 = AddNode(r[1]);
	int n2 = AddNode(r[2]);

	FACE face;
	face.n[0] = n0;
	face.n[1] = n1;
	face.n[2] = n2;
	face.vr[0] = r[0];
	face.vr[1] = r[1];
	face.vr[2] = r[2];
	face.c[0] = face.c[1] = face.c[2] = c;
	AddFace(face);
}

void GLMesh::AddFace(vec3f r[3], vec3f n[3], GLColor c)
{
	int n0 = AddNode(r[0]);
	int n1 = AddNode(r[1]);
	int n2 = AddNode(r[2]);

	FACE face;
	face.n[0] = n0;
	face.n[1] = n1;
	face.n[2] = n2;
	face.vn[0] = n[0];
	face.vn[1] = n[1];
	face.vn[2] = n[2];
	face.vr[0] = r[0];
	face.vr[1] = r[1];
	face.vr[2] = r[2];
	face.c[0] = face.c[1] = face.c[2] = c;
	AddFace(face);
}

void GLMesh::AddFace(vec3f r[3], vec3f n[3], float tex, GLColor c)
{
	int n0 = AddNode(r[0]);
	int n1 = AddNode(r[1]);
	int n2 = AddNode(r[2]);

	FACE face;
	face.n[0] = n0;
	face.n[1] = n1;
	face.n[2] = n2;
	face.vn[0] = n[0];
	face.vn[1] = n[1];
	face.vn[2] = n[2];
	face.vr[0] = r[0];
	face.vr[1] = r[1];
	face.vr[2] = r[2];
	face.t[0] = face.t[1] = face.t[2] = vec3f(tex, 0, 0);
	face.c[0] = face.c[1] = face.c[2] = c;
	AddFace(face);
}

void GLMesh::AddFace(vec3f r[3], vec3f n[3], float tex[3], GLColor c)
{
	int n0 = AddNode(r[0]);
	int n1 = AddNode(r[1]);
	int n2 = AddNode(r[2]);

	FACE face;
	face.n[0] = n0;
	face.n[1] = n1;
	face.n[2] = n2;
	face.vn[0] = n[0];
	face.vn[1] = n[1];
	face.vn[2] = n[2];
	face.vr[0] = r[0];
	face.vr[1] = r[1];
	face.vr[2] = r[2];
	face.t[0] = vec3f(tex[0], 0, 0);
	face.t[1] = vec3f(tex[1], 0, 0);
	face.t[2] = vec3f(tex[2], 0, 0);
	face.c[0] = face.c[1] = face.c[2] = c;
	AddFace(face);
}

void GLMesh::AddFace(vec3f r[3], float t[3], GLColor c[3])
{
	int n0 = AddNode(r[0]);
	int n1 = AddNode(r[1]);
	int n2 = AddNode(r[2]);

	FACE face;
	face.n[0] = n0;
	face.n[1] = n1;
	face.n[2] = n2;
	face.vr[0] = r[0];
	face.vr[1] = r[1];
	face.vr[2] = r[2];
	face.t[0] = vec3f(t[0], 0, 0);
	face.t[1] = vec3f(t[1], 0, 0);
	face.t[2] = vec3f(t[2], 0, 0);
	face.c[0] = c[0];
	face.c[1] = c[1];
	face.c[2] = c[2];
	AddFace(face);
}

void GLMesh::AddFace(vec3f r[3], GLColor c[3])
{
	int n0 = AddNode(r[0]);
	int n1 = AddNode(r[1]);
	int n2 = AddNode(r[2]);

	FACE face;
	face.n[0] = n0;
	face.n[1] = n1;
	face.n[2] = n2;
	face.vr[0] = r[0];
	face.vr[1] = r[1];
	face.vr[2] = r[2];
	face.c[0] = c[0];
	face.c[1] = c[1];
	face.c[2] = c[2];
	AddFace(face);
}

void GLMesh::AddFace(vec3f r[3], float t[3])
{
	int n0 = AddNode(r[0]);
	int n1 = AddNode(r[1]);
	int n2 = AddNode(r[2]);

	FACE face;
	face.n[0] = n0;
	face.n[1] = n1;
	face.n[2] = n2;
	face.vr[0] = r[0];
	face.vr[1] = r[1];
	face.vr[2] = r[2];
	face.t[0] = vec3f(t[0], 0, 0);
	face.t[1] = vec3f(t[1], 0, 0);
	face.t[2] = vec3f(t[2], 0, 0);
	AddFace(face);
}

void GLMesh::AddFace(vec3f r[3], vec3f t[3])
{
	int n0 = AddNode(r[0]);
	int n1 = AddNode(r[1]);
	int n2 = AddNode(r[2]);

	FACE face;
	face.n[0] = n0;
	face.n[1] = n1;
	face.n[2] = n2;
	face.vr[0] = r[0];
	face.vr[1] = r[1];
	face.vr[2] = r[2];
	face.t[0] = t[0];
	face.t[1] = t[1];
	face.t[2] = t[2];
	AddFace(face);
}
int GLMesh::SetFaceTex(int f0, float* t, int n)
{
	GLMesh::FACE* pf = &m_Face[f0];
	switch (n)
	{
	case 3:
		pf->t[0].x = t[0]; pf->t[1].x = t[1]; pf->t[2].x = t[2];
		return f0 + 1;
	break;
	case 4:
		pf->t[0].x = t[2]; pf->t[1].x = t[3]; pf->t[2].x = t[0]; pf++;
		pf->t[0].x = t[0]; pf->t[1].x = t[1]; pf->t[2].x = t[2]; pf++;
		return f0 + 2;
		break;
	case 6:
	{
		pf->t[0].x = t[0]; pf->t[1].x = t[3]; pf->t[2].x = t[5]; pf++;
		pf->t[0].x = t[1]; pf->t[1].x = t[4]; pf->t[2].x = t[3]; pf++;
		pf->t[0].x = t[2]; pf->t[1].x = t[5]; pf->t[2].x = t[4]; pf++;
		pf->t[0].x = t[3]; pf->t[1].x = t[4]; pf->t[2].x = t[5]; pf++;
		return f0 + 4;
	}
	break;
	case 7: // TRI7
	{
		pf->t[0].x = t[0]; pf->t[1].x = t[3]; pf->t[2].x = t[6]; pf++;
		pf->t[0].x = t[1]; pf->t[1].x = t[6]; pf->t[2].x = t[3]; pf++;
		pf->t[0].x = t[1]; pf->t[1].x = t[4]; pf->t[2].x = t[6]; pf++;
		pf->t[0].x = t[2]; pf->t[1].x = t[6]; pf->t[2].x = t[4]; pf++;
		pf->t[0].x = t[2]; pf->t[1].x = t[5]; pf->t[2].x = t[6]; pf++;
		pf->t[0].x = t[0]; pf->t[1].x = t[6]; pf->t[2].x = t[5]; pf++;
		return f0 + 6;
	}
	break;
	case 8: // QUAD8
	{
		pf->t[0].x = t[0]; pf->t[1].x = t[4]; pf->t[2].x = t[7]; pf++;
		pf->t[0].x = t[4]; pf->t[1].x = t[1]; pf->t[2].x = t[5]; pf++;
		pf->t[0].x = t[5]; pf->t[1].x = t[2]; pf->t[2].x = t[6]; pf++;
		pf->t[0].x = t[6]; pf->t[1].x = t[3]; pf->t[2].x = t[7]; pf++;
		pf->t[0].x = t[5]; pf->t[1].x = t[6]; pf->t[2].x = t[7]; pf++;
		pf->t[0].x = t[4]; pf->t[1].x = t[5]; pf->t[2].x = t[7]; pf++;
		return f0 + 6;
	}
	break;
	case 9: // QUAD9
	{
		pf->t[0].x = t[0]; pf->t[1].x = t[4]; pf->t[2].x = t[7]; pf++;
		pf->t[0].x = t[4]; pf->t[1].x = t[1]; pf->t[2].x = t[5]; pf++;
		pf->t[0].x = t[5]; pf->t[1].x = t[2]; pf->t[2].x = t[6]; pf++;
		pf->t[0].x = t[6]; pf->t[1].x = t[3]; pf->t[2].x = t[7]; pf++;
		pf->t[0].x = t[4]; pf->t[1].x = t[8]; pf->t[2].x = t[7]; pf++;
		pf->t[0].x = t[4]; pf->t[1].x = t[5]; pf->t[2].x = t[8]; pf++;
		pf->t[0].x = t[8]; pf->t[1].x = t[5]; pf->t[2].x = t[6]; pf++;
		pf->t[0].x = t[8]; pf->t[1].x = t[6]; pf->t[2].x = t[7]; pf++;
		return f0 + 8;
	}
	break;
	case 10: // TRI10
	{
		pf->t[0].x = t[0]; pf->t[1].x = t[3]; pf->t[2].x = t[7]; pf++;
		pf->t[0].x = t[1]; pf->t[1].x = t[5]; pf->t[2].x = t[4]; pf++;
		pf->t[0].x = t[2]; pf->t[1].x = t[8]; pf->t[2].x = t[6]; pf++;
		pf->t[0].x = t[9]; pf->t[1].x = t[7]; pf->t[2].x = t[3]; pf++;
		pf->t[0].x = t[9]; pf->t[1].x = t[3]; pf->t[2].x = t[4]; pf++;
		pf->t[0].x = t[9]; pf->t[1].x = t[4]; pf->t[2].x = t[5]; pf++;
		pf->t[0].x = t[9]; pf->t[1].x = t[5]; pf->t[2].x = t[6]; pf++;
		pf->t[0].x = t[9]; pf->t[1].x = t[6]; pf->t[2].x = t[8]; pf++;
		pf->t[0].x = t[9]; pf->t[1].x = t[8]; pf->t[2].x = t[7]; pf++;
		return f0 + 9;
	}
	break;
	default:
		assert(false);
	}
	setModified(true);
	return f0;
}

// Update normals for all faces using smoothing groups
void GLMesh::UpdateNormals()
{
	int NF = Faces();
	if (NF == 0) return;

	// calculate face normals
	for (int i=0; i<NF; ++i) 
	{
		FACE& f = m_Face[i];

		// reset smoothing id
		f.tag = -1;

		// calculate the face normal
		vec3f& r0 = Node(f.n[0]).r;
		vec3f& r1 = Node(f.n[1]).r;
		vec3f& r2 = Node(f.n[2]).r;
		f.fn = (r1 - r0)^(r2 - r0);

		if (f.sid < 0)
		{
			f.vn[0] = f.fn;
			f.vn[1] = f.fn;
			f.vn[2] = f.fn;
		}
	}

	//calculate the node normals
	int NN = Nodes();
	vector<vec3f> norm(NN, vec3f(0,0,0));

	vector<FACE*> F(NF);
	int FC = 0;

	vector<FACE*> stack(NF);
	int FS = 0;
	int nsg = 0;
	for (int i=0; i < NF; ++i)
	{
		FACE* pf = &m_Face[i];
		if ((pf->tag == -1) && (pf->sid >= 0))
		{
			// assign node normals
			for (int j = 0; j < FC; ++j)
			{
				FACE* pf2 = F[j];
				norm[pf2->n[0]] = vec3f(0.f, 0.f, 0.f);
				norm[pf2->n[1]] = vec3f(0.f, 0.f, 0.f);
				norm[pf2->n[2]] = vec3f(0.f, 0.f, 0.f);
			}
			FC = 0;

			stack[FS++] = pf;
			while (FS > 0)
			{
				// pop a face
				pf = stack[--FS];

				// mark as processed
				pf->tag = nsg;
				F[FC++] = pf;

				// add face normal to node normal
				norm[pf->n[0]] += pf->fn;
				norm[pf->n[1]] += pf->fn;
				norm[pf->n[2]] += pf->fn;

				// process neighbors
				for (int j = 0; j<3; ++j)
				{
					// push unprocessed neighbor
					if (pf->nbr[j] >= 0)
					{
						FACE* pf2 = &m_Face[pf->nbr[j]];
						if ((pf2->tag == -1) && (pf->sid == pf2->sid))
						{
							pf2->tag = -2;
							stack[FS++] = pf2;
						}
					}
				}
			}

			// assign node normals
			for (int j = 0; j<FC; ++j)
			{
				FACE* pf2 = F[j];
				assert(pf2->tag == nsg);
				pf2->vn[0] = norm[pf2->n[0]];
				pf2->vn[1] = norm[pf2->n[1]];
				pf2->vn[2] = norm[pf2->n[2]];
			}

			++nsg;
		}
	}

	// normalize face normals
	for (int i=0; i<NF; ++i)
	{
		FACE& f = m_Face[i];
		f.fn.Normalize();
		f.vn[0].Normalize();
		f.vn[1].Normalize();
		f.vn[2].Normalize();
	}

	setModified(true);
}

bool CmpFace(GLMesh::FACE f1, GLMesh::FACE f2)
{
	return (f1.pid < f2.pid);
}

bool CmpEdge(GLMesh::EDGE e1, GLMesh::EDGE e2)
{
	return (e1.pid < e2.pid);
}

void GLMesh::AutoPartition()
{
	int NF = m_Face.size();
	m_FIL.clear();
	if (NF == 0) return;

	// sort the face list by pid
	stable_sort(m_Face.begin(), m_Face.end(), CmpFace);

	// find the largest PID value
	// since the faces are sorted, this is the last one
	int FID = m_Face[NF-1].pid + 1;

	// find the start index and length of each surface
	m_FIL.resize(FID);
	for (int i=0; i<FID; ++i) m_FIL[i].nf = 0;
	for (int i=0; i<NF; ++i)
	{
		FACE& f = m_Face[i];
		m_FIL[f.pid].nf += 1;
	}
	m_FIL[0].n0 = 0;
	for (int i=1; i<FID; ++i) m_FIL[i].n0 = m_FIL[i-1].n0 + m_FIL[i-1].nf;

	setModified(true);
}

void GLMesh::Update(bool updateNormals)
{
	if (m_FIL.empty()) AutoPartition();

	int NF = (int) m_Face.size();
	if (NF)
	{
		for (int i = 0; i < NF; ++i)
		{
			FACE& face = m_Face[i];
			face.vr[0] = m_Node[face.n[0]].r;
			face.vr[1] = m_Node[face.n[1]].r;
			face.vr[2] = m_Node[face.n[2]].r;
		}
	}

	int NE = (int)m_Edge.size();
	if (NE)
	{
		// sort the edge list by pid
		stable_sort(m_Edge.begin(), m_Edge.end(), CmpEdge);

		// find the largest PID value
		// since the edges are sorted, this is the last one
		int EID = m_Edge[NE-1].pid + 1;

		// find the start index and length of each edge
		m_EIL.resize(EID);
		if (EID > 0)
		{
			for (int i=0; i<EID; ++i) m_EIL[i].second = 0;
			for (int i=0; i<NE; ++i)
			{
				EDGE& e = m_Edge[i];
				if (e.pid >= 0) m_EIL[e.pid].second += 1;
			}
			m_EIL[0].first = 0;
			for (int i=1; i<EID; ++i) m_EIL[i].first = m_EIL[i-1].first + m_EIL[i-1].second;
		}

		for (int i = 0; i < NE; ++i)
		{
			EDGE& edge = m_Edge[i];
			edge.vr[0] = m_Node[edge.n[0]].r;
			edge.vr[1] = m_Node[edge.n[1]].r;
		}
	}

	UpdateBoundingBox();
	if (!m_hasNeighborList) FindNeighbors();
	if (updateNormals) UpdateNormals();
}

/*
void GLMesh::Extrude(double d, vec3d t)
{
	int i;

	int N = Nodes();
	int E = Edges();
	int F = Faces();

	// find max node ID
	int NID = -1;
	for (i=0; i<N; ++i) if (m_Node[i].pid > NID) NID = m_Node[i].pid;

	int m = 0;
	for (i=0; i<N; ++i) m_Node[i].tag = -1;
	for (i=0; i<E; ++i)
	{
		EDGE& e = m_Edge[i];
		m_Node[e.n[0]].tag = 1;
		m_Node[e.n[1]].tag = 1;
	}
	for (i=0; i<N; ++i)
	{
		NODE& ni = m_Node[i];
		if ((ni.tag != -1)&&(ni.pid != -1)) ni.tag = m++;
	}

	// copy nodes
	m_Node.reserve(2*N);
	for (i=0; i<N; ++i)
	{
		NODE n = m_Node[i];
		n.r += t*d;
		if (n.pid != -1) n.pid += NID;
		m_Node.push_back(n);
	}

	// find max edge ID
	int EID = -1;
	for (i=0; i<E; ++i) if (m_Edge[i].pid > EID) EID = m_Edge[i].pid;

	// copy edges
	m_Edge.reserve(2*E+N);
	for (i=0; i<E; ++i)
	{
		EDGE e = m_Edge[i];
		e.n[0] += N;
		e.n[1] += N;
		e.pid += EID+1;
		m_Edge.push_back(e);
	}

	for (i=0; i<N; ++i)
	{
		NODE& n = m_Node[i];
		if ((n.pid != -1) && (n.tag != -1))
		{
			EDGE e = m_Edge[i];
			e.n[1] = e.n[0] + N;
			e.pid = 2*(EID+1) + n.tag;
			m_Edge.push_back(e);
		}
	}

	// find max face ID
	int FID = -1;
	for (i=0; i<F; ++i) if (m_Face[i].pid > FID) FID = m_Face[i].pid;

	// create new top-surfaces
	m_Face.reserve(2*F + 2*E);

	for (i=0; i<F; ++i)
	{
		FACE f = m_Face[i];
		f.pid += FID+1;
		f.sid += 1;
		f.n[0] += N;
		f.n[1] += N;
		f.n[2] += N;
		m_Face.push_back(f);
	}

	for (i=F; i<2*F; ++i) if (m_Face[i].pid > FID) FID = m_Face[i].pid;

	// create new side-faces
	for (i=0; i<E; ++i)
	{
		EDGE& e = m_Edge[i];

		FACE f;
		f.pid = e.pid + FID + 1;
		f.sid = f.pid;
		f.n[0] = e.n[0];
		f.n[1] = e.n[1];
		f.n[2] = e.n[1] + N;
		m_Face.push_back(f);

		f.sid = f.pid;
		f.n[0] = e.n[1] + N;
		f.n[1] = e.n[0] + N;
		f.n[2] = e.n[0];
		m_Face.push_back(f);
	}

	Update();
	UpdateNormals();
}
*/

void GLMesh::UpdateBoundingBox()
{
	m_box.x0 = m_box.y0 = m_box.z0 = 0.0;
	m_box.x1 = m_box.y1 = m_box.z1 = 0.0;

	if (Nodes() > 0)
	{
		m_box.x0 = m_box.x1 = m_Node[0].r.x;
		m_box.y0 = m_box.y1 = m_Node[0].r.y;
		m_box.z0 = m_box.z1 = m_Node[0].r.z;

		int N = (int) m_Node.size();
		for (int i=0; i<N; ++i) m_box += to_vec3d(m_Node[i].r);
	}
}

void GLMesh::FindNeighbors()
{
	int i, j, k;

	int NN = Nodes();
	int NF = Faces();
	if ((NN == 0) || (NF == 0)) return;

	// A. Build node-face table

	vector<int> val; val.assign(NN, 0);
	for (i=0; i<NF; ++i)
	{
		FACE& f = m_Face[i];
		val[f.n[0]]++;
		val[f.n[1]]++;
		val[f.n[2]]++;
	}
	
	vector<int> istrt(NN);
	istrt[0] = 0;
	for (i=1; i<NN; ++i) istrt[i] = istrt[i-1] + val[i-1];

	int n = 0;
	for (i=0; i<NN; ++i) n += val[i];
	vector<int> iface(n);

	val.assign(NN, 0);
	for (i=0; i<NF; ++i)
	{
		FACE& f = m_Face[i];
		iface[istrt[f.n[0]] + val[f.n[0]]] = i; val[f.n[0]]++;
		iface[istrt[f.n[1]] + val[f.n[1]]] = i; val[f.n[1]]++;
		iface[istrt[f.n[2]] + val[f.n[2]]] = i; val[f.n[2]]++;
	}

	// B. Find all neighbors
	for (i=0; i<NF; ++i)
	{
		FACE& f = m_Face[i];
		for (j=0; j<3; ++j)
		{
			f.nbr[j] = -1;
			int n0 = f.n[j];
			int n1 = f.n[(j+1)%3];
			int nf = val[n0];
			int f0 = istrt[n0];
			for (k=0; k<nf; ++k)
			{
				int n2 = iface[f0+k];
				if (n2 != i)
				{
					FACE& f2 = m_Face[n2];
					if (f.bext == f2.bext)
					{
						if (((f2.n[0]==n0) || (f2.n[1]==n0) || (f2.n[2]==n0)) && 
							((f2.n[0]==n1) || (f2.n[1]==n1) || (f2.n[2]==n1))) 
						{
							f.nbr[j] = n2;
							break;
						}
					}
				}
			}
		}
	}
	m_hasNeighborList = true;
}


void GLMesh::Attach(GLMesh &m, bool bupdate)
{
	int N0 = Nodes();
	int E0 = Edges();
	int F0 = Faces();

	int N1 = m.Nodes();
	int E1 = m.Edges();
	int F1 = m.Faces();

	// add nodes
	for (int i=0; i<N1; ++i) m_Node.push_back(m.Node(i));

	// add edges
	for (int i=0; i<E1; ++i)
	{
		EDGE e = m.Edge(i);
		e.n[0] += N0;
		e.n[1] += N0;
		m_Edge.push_back(e);
	}

	// add faces
	for (int i=0; i<F1; ++i)
	{
		FACE f = m.Face(i);
		f.n[0] += N0;
		f.n[1] += N0;
		f.n[2] += N0;
		AddFace(f);
	}
	setModified(true);

	if (bupdate) Update();
}

void GLMesh::AutoSmooth(double angleDegrees)
{
	int NF = Faces();

	// smoothing threshold
	double eps = (double)cos(angleDegrees * DEG2RAD);

	// clear face group ID's
	for (FACE& face : m_Face)
	{
		face.sid = -1;
	}

	// calculate face normals
	for (FACE& face : m_Face)
	{
		// calculate the face normals
		vec3f& r0 = Node(face.n[0]).r;
		vec3f& r1 = Node(face.n[1]).r;
		vec3f& r2 = Node(face.n[2]).r;

		face.fn = (r1 - r0) ^ (r2 - r0);
		face.fn.Normalize();
	}

	// stack for tracking unprocessed faces
	vector<FACE*> stack(NF);
	int ns = 0;

	// process all faces
	int nsg = 0;
	for (FACE& face : m_Face)
	{
		if (face.sid == -1)
		{
			stack[ns++] = &face;
			while (ns > 0)
			{
				// pop a face
				FACE* pf = stack[--ns];

				// mark as processed
				pf->sid = nsg;

				// loop over neighbors
				for (int j = 0; j < 3; ++j)
				{
					if (pf->nbr[j] >= 0)
					{
						FACE& face2 = Face(pf->nbr[j]);

						// push unprocessed neighbour
						if (face2.sid == -1)
						{
							bool badd = false;
							if (pf->fn * face2.fn >= eps)
							{
								badd = true;
							}

							if (badd)
							{
								face2.sid = -2;
								stack[ns++] = &face2;
							}
						}
					}
				}
			}
			++nsg;
		}
	}

	// update the normals
	UpdateNormals();
}
