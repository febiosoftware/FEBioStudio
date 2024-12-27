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
#include <PostLib/FEPostModel.h>
#include "GLPlane.h"
using namespace Post;

CGLPlane::CGLPlane(FEPostModel* pm)
{
	m_pfem = pm;
	m_e[2] = vec3d(0,0,1);
}

CGLPlane::~CGLPlane(void)
{
}

void CGLPlane::Create(int n[3])
{
	FSMeshBase* pm = m_pfem->GetFEMesh(0);
	if (pm && (n[0] > 0) && (n[1] > 0) && (n[2] > 0))
	{
		FSNode& n1 = pm->Node(n[0]-1);
		FSNode& n2 = pm->Node(n[1]-1);
		FSNode& n3 = pm->Node(n[2]-1);

		vec3d r1 = n1.r;
		vec3d r2 = n2.r;
		vec3d r3 = n3.r;

		m_rc = (r1 + r2 + r3)/3.0;

		BOX box = m_pfem->GetBoundingBox();
		double R = box.Radius();

		m_e[0] = r1 - r2;
		m_e[1] = r3 - r2;
		m_e[2] = m_e[0] ^ m_e[1]; 
		m_e[1] = m_e[2] ^ m_e[0];

		m_e[0].Normalize();
		m_e[1].Normalize();
		m_e[2].Normalize();
	}
}

void CGLPlane::Render(GLRenderEngine& re, CGLContext& rc)
{
	FSMeshBase* pm = m_pfem->GetFEMesh(0);

	glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_CULL_FACE);
	GLfloat zero[4] = {0.f};
	GLfloat one[4] = {1.f, 1.f, 1.f, 1.f};
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, zero);
	glLightfv(GL_LIGHT0, GL_SPECULAR, zero);

	double R = m_pfem->GetBoundingBox().Radius();

	glPushMatrix();
	glTranslatef(m_rc.x, m_rc.y, m_rc.z);

	quatd q(vec3d(0,0,1), m_e[2]);
	double w = q.GetAngle();
	if (w != 0)
	{
		vec3d r = q.GetVector();
		glRotated((w*RAD2DEG), r.x, r.y, r.z);
	}

	glColor4ub(255, 0, 0, 128);
	glNormal3d(0,0,1);
	glBegin(GL_QUADS);
	{
		glVertex2d(-R, -R); 
		glVertex2d( R, -R); 
		glVertex2d( R,  R); 
		glVertex2d(-R,  R); 
	}
	glEnd();

	glPopMatrix();

	glPopAttrib();
}
