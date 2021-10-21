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

#include "stdafx.h"
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
#include "GLTexture1D.h"

GLTexture1D::GLTexture1D()
{
	m_texID = 0;
	m_n = 1024; 
	for (int i=0; i<3*m_n; i++) m_pb[i] = 0;
	m_bupdate = true;
}

GLTexture1D::GLTexture1D(const GLTexture1D& tex)
{
	m_n = tex.m_n;
	for (int i = 0; i<3 * m_n; i++) m_pb[i] = tex.m_pb[i];
	m_bupdate = true;
}

void GLTexture1D::SetTexture(unsigned char* pb)
{ 
	for (int i=0; i<3*m_n; i++) m_pb[i] = pb[i]; 
	m_bupdate = true;
}

void GLTexture1D::Update()
{
	m_bupdate = true;
}

void GLTexture1D::MakeCurrent()
{ 
	if (m_texID == 0)
	{
		glGenTextures(1, &m_texID);
		glBindTexture(GL_TEXTURE_1D, m_texID);

		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //(m_bsmooth ? GL_LINEAR : GL_NEAREST));
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //(m_bsmooth ? GL_LINEAR : GL_NEAREST));

		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	}
	else glBindTexture(GL_TEXTURE_1D, m_texID);

	if (m_bupdate)
	{
		glTexImage1D(GL_TEXTURE_1D, 0, 3, m_n, 0, GL_RGB, GL_UNSIGNED_BYTE, m_pb); 
		m_bupdate = false;
	}
};

int GLTexture1D::Size()
{
	return m_n; 
}

unsigned char* GLTexture1D::GetBytes() 
{ 
	return m_pb; 
}
