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
using namespace Post;

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
