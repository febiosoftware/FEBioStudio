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
#include "GLShader.h"
#include "GLTexture1D.h"
#include <GLLib/glx.h>

void GLStandardShader::Activate()
{
	glDisable(GL_COLOR_MATERIAL);

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
}

void GLStandardShader::Render(const GMesh::FACE& f)
{
	glNormal3fv(&f.vn[0].x); glVertex3fv(&f.vr[0].x);
	glNormal3fv(&f.vn[1].x); glVertex3fv(&f.vr[1].x);
	glNormal3fv(&f.vn[2].x); glVertex3fv(&f.vr[2].x);
}

void GLTexture1DShader::SetTexture(GLTexture1D* tex)
{
	m_tex = tex;
}

void GLTexture1DShader::Activate()
{
	glEnable(GL_COLOR_MATERIAL);
	glColor3ub(255, 255, 255);
	glEnable(GL_TEXTURE_1D);
	m_tex->MakeCurrent();
}

void GLTexture1DShader::Deactivate()
{
	glDisable(GL_TEXTURE_1D);
}

void GLTexture1DShader::Render(const GMesh::FACE& f)
{
	glNormal3fv(&f.vn[0].x); glTexCoord1f(f.t[0]); glVertex3fv(&f.vr[0].x);
	glNormal3fv(&f.vn[1].x); glTexCoord1f(f.t[1]); glVertex3fv(&f.vr[1].x);
	glNormal3fv(&f.vn[2].x); glTexCoord1f(f.t[2]); glVertex3fv(&f.vr[2].x);
}

GLStandardModelShader::GLStandardModelShader()
{
	m_col = GLColor(200, 200, 200);
}

GLStandardModelShader::GLStandardModelShader(const GLColor& c) : m_col(c)
{

}

void GLStandardModelShader::Activate()
{
	GLfloat col[] = { 0.8f, 0.6f, 0.6f, 1.f };
	GLfloat rev[] = { 0.8f, 0.6f, 0.6f, 1.f };
	GLfloat spc[] = { 0.0f, 0.0f, 0.0f, 1.f };
	GLfloat emi[] = { 0.0f, 0.0f, 0.0f, 1.f };

	m_col.toFloat(col);

	glDisable(GL_COLOR_MATERIAL);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col);
	glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, rev);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spc);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emi);
	glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 0);
}

void GLStandardModelShader::Deactivate()
{
}

void GLStandardModelShader::Render(const GMesh::FACE& f)
{
	glNormal3fv(&f.vn[0].x); glVertex3fv(&f.vr[0].x);
	glNormal3fv(&f.vn[1].x); glVertex3fv(&f.vr[1].x);
	glNormal3fv(&f.vn[2].x); glVertex3fv(&f.vr[2].x);
}

void GLFaceColorShader::Activate()
{
	GLfloat zero[] = { 0.0f, 0.0f, 0.0f, 1.f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zero);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, zero);
	glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 0);

	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
}

void GLFaceColorShader::Render(const GMesh::FACE& f)
{
	glNormal3fv(&f.vn[0].x); glColor4ub(f.c[0].r, f.c[0].g, f.c[0].b, f.c[0].a); glVertex3fv(&f.vr[0].x);
	glNormal3fv(&f.vn[1].x); glColor4ub(f.c[1].r, f.c[1].g, f.c[1].b, f.c[1].a); glVertex3fv(&f.vr[1].x);
	glNormal3fv(&f.vn[2].x); glColor4ub(f.c[2].r, f.c[2].g, f.c[2].b, f.c[2].a); glVertex3fv(&f.vr[2].x);
}
