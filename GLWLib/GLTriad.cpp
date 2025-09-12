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
#include "GLTriad.h"
#ifdef WIN32
#include <Windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif
#include <QPainter>
#include "convert.h"

GLTriad::GLTriad(int x, int y, int w, int h) : GLWidget(x, y, w, h)
{
	m_rot = quatd(0.f, vec3d(1.f, 0.f, 0.f));
	m_bcoord_labels = true;
}

void GLTriad::draw(QPainter* painter)
{
	GLWidget::draw(painter);

	painter->beginNativePainting();
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	GLfloat ones[] = { 1.f, 1.f, 1.f, 1.f };
	GLfloat ambient[] = { 0.0f,0.0f,0.0f,1.f };
	GLfloat specular[] = { 0.5f,0.5f,0.5f,1 };
	GLfloat emission[] = { 0,0,0,1 };
	GLfloat	light[] = { 0, 0, -1, 0 };

	int view[4];
	glGetIntegerv(GL_VIEWPORT, view);

	double DPR = painter->device()->devicePixelRatio();

	int x0 = (int)(DPR * x());
	int y0 = view[3] - (int)(DPR * (y() + h()));
	int x1 = x0 + (int)(DPR * w());
	int y1 = view[3] - (int)(DPR * y());
	if (x1 < x0) { x0 ^= x1; x1 ^= x0; x0 ^= x1; }
	if (y1 < y0) { y0 ^= y1; y1 ^= y0; y0 ^= y1; }

	glViewport(x0, y0, x1 - x0, y1 - y0);

	float ar = 1.f;
	if (h() != 0) ar = fabs((float)w() / (float)h());

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	float d = 1.2f;
	if (ar >= 1.f)	gluOrtho2D(-d * ar, d * ar, -d, d); else gluOrtho2D(-d, d, -d / ar, d / ar);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glClear(GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glDisable(GL_CULL_FACE);
	glFrontFace(GL_CW);

	glLightfv(GL_LIGHT0, GL_POSITION, light);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ones);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, ones);

	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 32);

	quatd q = m_rot;
	vec3d r = q.GetVector();
	float a = 180 * q.GetAngle() / PI;

	if ((a > 0) && (r.Length() > 0))
		glRotatef(a, r.x, r.y, r.z);

	// create the cylinder object
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	GLUquadricObj* pcyl = gluNewQuadric();

	const GLdouble r0 = .05;
	const GLdouble r1 = .15;

	glPushMatrix();
	glRotatef(90, 0, 1, 0);
	glColor3ub(255, 0, 0);
	gluCylinder(pcyl, r0, r0, .9, 5, 1);
	glTranslatef(0, 0, .8f);
	gluCylinder(pcyl, r1, 0, 0.2, 10, 1);
	glPopMatrix();

	glPushMatrix();
	glRotatef(-90, 1, 0, 0);
	glColor3ub(0, 255, 0);
	gluCylinder(pcyl, r0, r0, .9, 5, 1);
	glTranslatef(0, 0, .8f);
	gluCylinder(pcyl, r1, 0, 0.2, 10, 1);
	glPopMatrix();

	glPushMatrix();
	glColor3ub(0, 0, 255);
	gluCylinder(pcyl, r0, r0, .9, 5, 1);
	glTranslatef(0, 0, .8f);
	gluCylinder(pcyl, r1, 0, 0.2, 10, 1);
	glPopMatrix();

	gluDeleteQuadric(pcyl);

	// restore project matrix
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	// restore modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	// restore attributes
	glPopAttrib();

	// restore viewport
	glViewport(view[0], view[1], view[2], view[3]);

	painter->endNativePainting();

	// restore identity matrix
	if (m_bcoord_labels)
	{
		float a = 0.8f;
		vec3d ex(a, 0.f, 0.f);
		vec3d ey(0.f, a, 0.f);
		vec3d ez(0.f, 0.f, a);
		q.RotateVector(ex);
		q.RotateVector(ey);
		q.RotateVector(ez);

		x0 /= DPR;
		x1 /= DPR;
		y0 = (view[3] - y0) / DPR;
		y1 = (view[3] - y1) / DPR;

		ex.x = x0 + (x1 - x0) * (ex.x + 1) * 0.5; ex.y = y0 + (y1 - y0) * (ex.y + 1) * 0.5;
		ey.x = x0 + (x1 - x0) * (ey.x + 1) * 0.5; ey.y = y0 + (y1 - y0) * (ey.y + 1) * 0.5;
		ez.x = x0 + (x1 - x0) * (ez.x + 1) * 0.5; ez.y = y0 + (y1 - y0) * (ez.y + 1) * 0.5;

		painter->setFont(m_font);
		painter->setPen(toQColor(m_fgc));
		painter->drawText(ex.x, ex.y, "X");
		painter->drawText(ey.x, ey.y, "Y");
		painter->drawText(ez.x, ez.y, "Z");
	}
}
