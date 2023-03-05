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

// Mesh used for GL rendering using vertex arrays
class GLVAMesh
{
public:
	GLVAMesh();
	~GLVAMesh();

	// Set the data buffers
	void SetData(double* vr, double* vn = nullptr, double* vt = nullptr, unsigned int vertexCount = 0);

	// call this to start building the mesh
	void BeginMesh();

	// add a vertex to the mesh
	void AddVertex(double* r, double* n, double* t);

	// this when done building the mesh
	void EndMesh();

	// render the mesh
	void Render();

private:
	double* m_vr = nullptr;	// vertex coordinates
	double* m_vn = nullptr;	// vertex normals
	double* m_vt = nullptr;	// vertex texture coordinates
	unsigned int m_vertexCount = 0;	// number of vertices
	bool	m_bvalid;	// is the mesh valid and ready for rendering?
};

inline void GLVAMesh::AddVertex(double* r, double* n, double* t)
{
	size_t i = m_vertexCount++;
	if (r && m_vr) { m_vr[3 * i] = r[0]; m_vr[3 * i + 1] = r[1]; m_vr[3 * i + 2] = r[2]; }
	if (n && m_vn) { m_vn[3 * i] = n[0]; m_vn[3 * i + 1] = n[1]; m_vn[3 * i + 2] = n[2]; }
	if (r && m_vt) { m_vt[3 * i] = t[0]; m_vt[3 * i + 1] = t[1]; m_vt[3 * i + 2] = t[2]; }
}
