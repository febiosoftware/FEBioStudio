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

GLVAMesh::GLVAMesh()
{
	m_vr = nullptr;
	m_vn = nullptr;
	m_vt = nullptr;
	m_vertexCount = 0;
	m_bvalid = false;
}

GLVAMesh::~GLVAMesh()
{
	m_vertexCount = 0;
	delete[] m_vr;
	delete[] m_vn;
	delete[] m_vt;
	m_bvalid = false;
}

void GLVAMesh::SetData(double* vr, double* vn, double* vt, unsigned int vertexCount)
{
	m_vertexCount = 0;
	delete[] m_vr;
	delete[] m_vn;
	delete[] m_vt;
	m_vr = vr;
	m_vn = vn;
	m_vt = vt;
	m_vertexCount = vertexCount;
	m_bvalid = ((vr != nullptr) && (vertexCount != 0));
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

void GLVAMesh::Render()
{
	if (!m_bvalid) return;

	glEnableClientState(GL_VERTEX_ARRAY);
	if (m_vn) glEnableClientState(GL_NORMAL_ARRAY);
	if (m_vt) glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_DOUBLE, 0, m_vr);
	if (m_vn) glNormalPointer(GL_DOUBLE, 0, m_vn);
	if (m_vt) glTexCoordPointer(3, GL_DOUBLE, 0, m_vt);

	glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);

	glDisableClientState(GL_VERTEX_ARRAY);
	if (m_vn) glDisableClientState(GL_NORMAL_ARRAY);
	if (m_vt) glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}
