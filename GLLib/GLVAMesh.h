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

class GMesh;
class CGLCamera;

#ifndef ubyte
#define ubyte unsigned char
#endif

// Mesh used for GL rendering using vertex arrays
class GLVAMesh
{
public:
	GLVAMesh();
	~GLVAMesh();

	// Set the data buffers directly 
	void SetVertexData(double* vr, double* vn = nullptr, double* vt = nullptr, ubyte* vc = nullptr);

	// call this to start building the mesh
	void BeginMesh();

	// add a vertex to the mesh
	void AddVertex(double* r, double* n, double* t);
	void AddVertex(const vec3d& r, const vec3d& n, const GLColor& c);
	void AddVertex(const vec3f& r, const vec3f& n);

	// this when done building the mesh
	void EndMesh();

	// create from a GMesh
	void CreateFromGMesh(const GMesh& gmsh);

	// render the mesh
	void Render();

	// set the transparency of the mesh
	void SetTransparency(ubyte a);

	// z-sort the faces
	void ZSortFaces(const CGLCamera& cam);

private:
	double* m_vr = nullptr;	// vertex coordinates
	double* m_vn = nullptr;	// vertex normals
	double* m_vt = nullptr;	// vertex texture coordinates
	ubyte* m_vc = nullptr; // vertex color (4 x unsigned byte)
	unsigned int* m_ind = nullptr; // vertex indices (used for z-sorting)
	
	unsigned int m_vertexCount = 0;	// number of vertices
	bool	m_bvalid;	// is the mesh valid and ready for rendering?
};

inline void GLVAMesh::AddVertex(double* r, double* n, double* t)
{
	size_t i = m_vertexCount++;
	if (r && m_vr) { m_vr[3 * i] = r[0]; m_vr[3 * i + 1] = r[1]; m_vr[3 * i + 2] = r[2]; }
	if (n && m_vn) { m_vn[3 * i] = n[0]; m_vn[3 * i + 1] = n[1]; m_vn[3 * i + 2] = n[2]; }
	if (r && m_vt) { m_vt[3 * i] = t[0]; m_vt[3 * i + 1] = t[1]; m_vt[3 * i + 2] = t[2]; }

	if (m_vc) { m_vc[4 * i] = 0; m_vc[4 * i + 1] = 0; m_vc[4 * i + 2] = 0; m_vc[4 * i + 3] = 255; }
}

inline void GLVAMesh::AddVertex(const vec3d& r, const vec3d& n, const GLColor& c)
{
	size_t i = m_vertexCount++;
	if (m_vr) { m_vr[3 * i] = r.x; m_vr[3 * i + 1] = r.y; m_vr[3 * i + 2] = r.z; }
	if (m_vn) { m_vn[3 * i] = n.x; m_vn[3 * i + 1] = n.y; m_vn[3 * i + 2] = n.z; }
	if (m_vc) { m_vc[4 * i] = c.r; m_vc[4 * i + 1] = c.g; m_vc[4 * i + 2] = c.b; m_vc[4 * i + 3] = c.a; }

	if (m_vt) { m_vt[3 * i] =   0; m_vt[3 * i + 1] =   0; m_vt[3 * i + 2] =   0; }
}

inline void GLVAMesh::AddVertex(const vec3f& r, const vec3f& n)
{
	size_t i = m_vertexCount++;
	if (m_vr) { m_vr[3 * i] = (double)r.x; m_vr[3 * i + 1] = (double)r.y; m_vr[3 * i + 2] = (double)r.z; }
	if (m_vn) { m_vn[3 * i] = (double)n.x; m_vn[3 * i + 1] = (double)n.y; m_vn[3 * i + 2] = (double)n.z; }
	
	if (m_vc) { m_vc[4 * i] = 0; m_vc[4 * i + 1] = 0; m_vc[4 * i + 2] = 0; m_vc[4 * i + 3] = 255; }
	if (m_vt) { m_vt[3 * i] = 0; m_vt[3 * i + 1] = 0; m_vt[3 * i + 2] = 0; }
}
