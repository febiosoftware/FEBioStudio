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
#pragma once
#include <FSCore/math3d.h>
#include <FSCore/color.h>
#include "OGLBase.h"

class GLMesh;
class GLCamera;

#ifndef ubyte
#define ubyte unsigned char
#endif

// Mesh class used for GL rendering using vertex arrays
// This base class has a protected constructor, so cannot be used directly.
// Instead, use one of the derived classes below. 
class OGLMesh : public OGLBase
{
private:
	enum VertexBuffer
	{
		VERTEX_DATA,
		NORMAL_DATA,
		TEXTURE_DATA,
		COLOR_DATA,
		INDEX_DATA
	};

public:
	enum Flags {
		FLAG_NORMAL  = 1,
		FLAG_TEXTURE = 2,
		FLAG_COLOR   = 4,
		FLAG_ALL = 15
	};

	enum RenderMode
	{
		ImmediateMode,
		VertexArrayMode,
		VBOMode
	};

	struct Vertex
	{
		vec3f	r;
		vec3f	n;
		vec3f	t;
		GLColor	c;
	};

public:
	// clear all mesh data
	void Clear();

	// set the render mode
	void SetRenderMode(RenderMode mode);

	// call this to start building the mesh
	void BeginMesh();

	// add a vertex to the mesh
	void AddVertex(double* r, double* n, double* t);
	void AddVertex(const vec3f& r);
	void AddVertex(const vec3d& r);
	void AddVertex(const vec3d& r, const vec3d& n, const GLColor& c);
	void AddVertex(const vec3f& r, const vec3f& n, const GLColor& c);
	void AddVertex(const vec3f& r, const vec3f& n, const GLColor& c, float tex);
	void AddVertex(const vec3f& r, const vec3f& n, const GLColor& c, const vec3f& tex);
	void AddVertex(const vec3d& r, const vec3d& n);
	void AddVertex(const vec3f& r, const vec3f& n);
	void AddVertex(const vec3f& r, float tex);
	void AddVertex(const vec3d& r, float tex);
	void AddVertex(const vec3f& r, const GLColor& c);
	void AddVertex(const vec3d& r, const GLColor& c);
	void AddVertex(const vec3d& r, float tex, const GLColor& c);
	void AddVertex(const Vertex& v);

	// this when done building the mesh
	void EndMesh();

	// render the mesh
	void Render(unsigned int flags = 0);

	void Render(int nstart, int ncount, unsigned int nflags = 0);

	// set the transparency of the mesh
	void SetTransparency(ubyte a);

	// is the mesh valid
	bool IsValid() const { return m_bvalid; }

	// return the number of vertices in the mesh
	size_t Vertices() const { return m_vertexCount; }

	// return vertex data
	Vertex GetVertex(size_t i) const;

public:
	void incRef();
	void decRef();
	void resetRef();
	int refs() const;

	virtual ~OGLMesh();

protected:
	OGLMesh(unsigned int mode);

	void AllocVertexBuffers(size_t maxVertices, unsigned flags);

private:
	void RenderImmediate(unsigned int flags);
	void RenderVertexArrays(unsigned int flags);
	void RenderVBO(unsigned int flags);

	void InitVBO();
	void ClearVBO();

protected:
	float* m_vr = nullptr;	// vertex coordinates
	float* m_vn = nullptr;	// vertex normals
	float* m_vt = nullptr;	// vertex texture coordinates
	ubyte* m_vc = nullptr; // vertex color (4 x unsigned byte)

	unsigned int* m_ind = nullptr; // vertex indices (used for z-sorting)
	bool m_useIndices;
	
	size_t m_vertexCount = 0;	// number of vertices
	size_t m_maxVertexCount = 0;	// max number of vertices
	unsigned int m_flags;
	bool	m_bvalid;	// is the mesh valid and ready for rendering?

	unsigned int	m_vbo[5];

	unsigned int m_mode;	// primitive type to render (set by derived classes)
	RenderMode	m_renderMode;
	bool	m_initVBO;

	int m_start, m_count;

	int	m_refCount;
};

inline void OGLMesh::AddVertex(double* r, double* n, double* t)
{
	size_t i = m_vertexCount++;
	if (r && m_vr) { m_vr[3 * i] = (float)r[0]; m_vr[3 * i + 1] = (float)r[1]; m_vr[3 * i + 2] = (float)r[2]; }
	if (n && m_vn) { m_vn[3 * i] = (float)n[0]; m_vn[3 * i + 1] = (float)n[1]; m_vn[3 * i + 2] = (float)n[2]; }
	if (r && m_vt) { m_vt[3 * i] = (float)t[0]; m_vt[3 * i + 1] = (float)t[1]; m_vt[3 * i + 2] = (float)t[2]; }
}

inline void OGLMesh::AddVertex(const vec3f& r)
{
	size_t i = m_vertexCount++;
	if (m_vr) { m_vr[3 * i] = r.x; m_vr[3 * i + 1] = r.y; m_vr[3 * i + 2] = r.z; }
}

inline void OGLMesh::AddVertex(const vec3d& r)
{
	size_t i = m_vertexCount++;
	if (m_vr) { m_vr[3 * i] = (float)r.x; m_vr[3 * i + 1] = (float)r.y; m_vr[3 * i + 2] = (float)r.z; }
}

inline void OGLMesh::AddVertex(const vec3d& r, const vec3d& n, const GLColor& c)
{
	size_t i = m_vertexCount++;
	if (m_vr) { m_vr[3 * i] = (float)r.x; m_vr[3 * i + 1] = (float)r.y; m_vr[3 * i + 2] = (float)r.z; }
	if (m_vn) { m_vn[3 * i] = (float)n.x; m_vn[3 * i + 1] = (float)n.y; m_vn[3 * i + 2] = (float)n.z; }
	if (m_vc) { m_vc[4 * i] = c.r; m_vc[4 * i + 1] = c.g; m_vc[4 * i + 2] = c.b; m_vc[4 * i + 3] = c.a; }
}

inline void OGLMesh::AddVertex(const vec3f& r, const vec3f& n, const GLColor& c)
{
	size_t i = m_vertexCount++;
	if (m_vr) { m_vr[3 * i] = r.x; m_vr[3 * i + 1] = r.y; m_vr[3 * i + 2] = r.z; }
	if (m_vn) { m_vn[3 * i] = n.x; m_vn[3 * i + 1] = n.y; m_vn[3 * i + 2] = n.z; }
	if (m_vc) { m_vc[4 * i] = c.r; m_vc[4 * i + 1] = c.g; m_vc[4 * i + 2] = c.b; m_vc[4 * i + 3] = c.a; }
}

inline void OGLMesh::AddVertex(const vec3f& r, const vec3f& n, const GLColor& c, float tex)
{
	size_t i = m_vertexCount++;
	if (m_vr) { m_vr[3 * i] = r.x; m_vr[3 * i + 1] = r.y; m_vr[3 * i + 2] = r.z; }
	if (m_vn) { m_vn[3 * i] = n.x; m_vn[3 * i + 1] = n.y; m_vn[3 * i + 2] = n.z; }
	if (m_vc) { m_vc[4 * i] = c.r; m_vc[4 * i + 1] = c.g; m_vc[4 * i + 2] = c.b; m_vc[4 * i + 3] = c.a; }
	if (m_vt) { m_vt[3 * i] = tex; m_vt[3 * i + 1] =   0; m_vt[3 * i + 2] = 0; }
}

inline void OGLMesh::AddVertex(const vec3f& r, const vec3f& n, const GLColor& c, const vec3f& t)
{
	size_t i = m_vertexCount++;
	if (m_vr) { m_vr[3 * i] = r.x; m_vr[3 * i + 1] = r.y; m_vr[3 * i + 2] = r.z; }
	if (m_vn) { m_vn[3 * i] = n.x; m_vn[3 * i + 1] = n.y; m_vn[3 * i + 2] = n.z; }
	if (m_vc) { m_vc[4 * i] = c.r; m_vc[4 * i + 1] = c.g; m_vc[4 * i + 2] = c.b; m_vc[4 * i + 3] = c.a; }
	if (m_vt) { m_vt[3 * i] = t.x; m_vt[3 * i + 1] = t.y; m_vt[3 * i + 2] = t.z; }
}

inline void OGLMesh::AddVertex(const vec3d& r, float tex, const GLColor& c)
{
	size_t i = m_vertexCount++;
	if (m_vr) { m_vr[3 * i] = (float)r.x; m_vr[3 * i + 1] = (float)r.y; m_vr[3 * i + 2] = (float)r.z; }
	if (m_vc) { m_vc[4 * i] = c.r; m_vc[4 * i + 1] = c.g; m_vc[4 * i + 2] = c.b; m_vc[4 * i + 3] = c.a; }
	if (m_vt) { m_vt[3 * i] = tex; m_vt[3 * i + 1] = 0; m_vt[3 * i + 2] = 0; }
}

inline void OGLMesh::AddVertex(const vec3f& r, const vec3f& n)
{
	size_t i = m_vertexCount++;
	if (m_vr) { m_vr[3 * i] = r.x; m_vr[3 * i + 1] = r.y; m_vr[3 * i + 2] = r.z; }
	if (m_vn) { m_vn[3 * i] = n.x; m_vn[3 * i + 1] = n.y; m_vn[3 * i + 2] = n.z; }
}

inline void OGLMesh::AddVertex(const vec3d& r, const vec3d& n)
{
	size_t i = m_vertexCount++;
	if (m_vr) { m_vr[3 * i] = (float)r.x; m_vr[3 * i + 1] = (float)r.y; m_vr[3 * i + 2] = (float)r.z; }
	if (m_vn) { m_vn[3 * i] = (float)n.x; m_vn[3 * i + 1] = (float)n.y; m_vn[3 * i + 2] = (float)n.z; }
}

inline void OGLMesh::AddVertex(const vec3f& r, float tex)
{
	size_t i = m_vertexCount++;
	if (m_vr) { m_vr[3 * i] = r.x; m_vr[3 * i + 1] = r.y; m_vr[3 * i + 2] = r.z; }
	if (m_vt) { m_vt[3 * i] = tex; m_vt[3 * i + 1] = 0; m_vt[3 * i + 2] = 0; }
}

inline void OGLMesh::AddVertex(const vec3d& r, float tex)
{
	size_t i = m_vertexCount++;
	if (m_vr) { m_vr[3 * i] = (float)r.x; m_vr[3 * i + 1] = (float)r.y; m_vr[3 * i + 2] = (float)r.z; }
	if (m_vt) { m_vt[3 * i] = tex; m_vt[3 * i + 1] = 0; m_vt[3 * i + 2] = 0; }
}

inline void OGLMesh::AddVertex(const vec3f& r, const GLColor& c)
{
	size_t i = m_vertexCount++;
	if (m_vr) { m_vr[3 * i] = r.x; m_vr[3 * i + 1] = r.y; m_vr[3 * i + 2] = r.z; }
	if (m_vc) { m_vc[4 * i] = c.r; m_vc[4 * i + 1] = c.g; m_vc[4 * i + 2] = c.b; m_vc[4 * i + 3] = c.a; }
}

inline void OGLMesh::AddVertex(const vec3d& r, const GLColor& c)
{
	size_t i = m_vertexCount++;
	if (m_vr) { m_vr[3 * i] = (float)r.x; m_vr[3 * i + 1] = (float)r.y; m_vr[3 * i + 2] = (float)r.z; }
	if (m_vc) { m_vc[4 * i] = c.r; m_vc[4 * i + 1] = c.g; m_vc[4 * i + 2] = c.b; m_vc[4 * i + 3] = c.a; }
}

inline void OGLMesh::AddVertex(const Vertex& v)
{
	size_t i = m_vertexCount++;
	if (m_vr) { m_vr[3 * i] = v.r.x; m_vr[3 * i + 1] = v.r.y; m_vr[3 * i + 2] = v.r.z; }
	if (m_vn) { m_vn[3 * i] = v.n.x; m_vn[3 * i + 1] = v.n.y; m_vn[3 * i + 2] = v.n.z; }
	if (m_vt) { m_vt[3 * i] = v.t.x; m_vt[3 * i + 1] = v.t.y; m_vt[3 * i + 2] = v.t.z; }
	if (m_vc) { m_vc[4 * i] = v.c.r; m_vc[4 * i + 1] = v.c.g; m_vc[4 * i + 2] = v.c.b; m_vc[4 * i + 3] = v.c.a; }
}

inline OGLMesh::Vertex OGLMesh::GetVertex(size_t i) const
{
	Vertex v;
	if (m_vr) { float* r = m_vr + (3 * i); v.r = vec3f(r[0], r[1], r[2]); }
	if (m_vn) { float* n = m_vn + (3 * i); v.n = vec3f(n[0], n[1], n[2]); }
	if (m_vt) { float* t = m_vt + (3 * i); v.t = vec3f(t[0], t[1], t[2]); }
	if (m_vc) { ubyte* c = m_vc + (4 * i); v.c = GLColor(c[0], c[1], c[2], c[3]); }
	return v;
}

// Triangle mesh
class OGLTriMesh : public OGLMesh
{
public:
	OGLTriMesh();

	void Create(size_t maxTriangles, unsigned int flags = 0);

	void AddTriangle(const vec3d& r0, const vec3d& r1, const vec3d& r2);

	// z-sort the faces
	void ZSortFaces(const GLCamera& cam);

	// sort backwards/forwards
	void SortBackwards();
	void SortForwards();

	// create from a GLMesh
	void CreateFromGMesh(const GLMesh& gmsh, unsigned int flags = FLAG_NORMAL | FLAG_COLOR);
	void CreateFromGMesh(const GLMesh& gmsh, int surfaceID, unsigned int flags);
};

inline void OGLTriMesh::AddTriangle(const vec3d& r0, const vec3d& r1, const vec3d& r2)
{
	AddVertex(r0);
	AddVertex(r1);
	AddVertex(r2);
}

// quad mesh
class OGLQuadMesh : public OGLMesh
{
public:
	OGLQuadMesh();

	void Create(int maxQuads, unsigned int flags = 0);
};

// line mesh
class OGLLineMesh : public OGLMesh
{
public:
	OGLLineMesh();
	OGLLineMesh(int maxLines, unsigned int flags = 0);

	void Create(int maxLines, unsigned int flags = 0);

	void AddLine(const vec3f& r0, const vec3f& r1);
	void AddLine(const vec3d& r0, const vec3d& r1);
	void AddLine(const vec3f& r0, const vec3f& r1, GLColor& c);

	void CreateFromGMesh(const GLMesh& mesh, unsigned int flags = 0);
};

inline void OGLLineMesh::AddLine(const vec3f& r0, const vec3f& r1)
{
	AddVertex(r0);
	AddVertex(r1);
}

inline void OGLLineMesh::AddLine(const vec3d& r0, const vec3d& r1)
{
	AddVertex(r0);
	AddVertex(r1);
}

inline void OGLLineMesh::AddLine(const vec3f& r0, const vec3f& r1, GLColor& c)
{
	AddVertex(r0, c);
	AddVertex(r1, c);
}

// point mesh
class OGLPointMesh : public OGLMesh
{
public:
	OGLPointMesh();
	OGLPointMesh(int maxVertices, unsigned int flags = 0);

	void Create(int maxVertices, unsigned int flags = 0);

	void CreateFromGMesh(const GLMesh& mesh);
	void CreateFromTaggedGMesh(const GLMesh& mesh, int tag);
};
