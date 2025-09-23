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
#include <qopengl.h>
#include "GLSafeFrame.h"
#include <QPainter>

GLSafeFrame::GLSafeFrame(int x, int y, int w, int h) : GLWidget(x, y, w, h)
{
	m_state = FREE;
}

void GLSafeFrame::resize(int x, int y, int W, int H)
{
	if (m_state == FREE) GLWidget::resize(x, y, W, H);
	else if (m_state == FIXED_SIZE) GLWidget::resize(x, y, w(), h());
}

void GLSafeFrame::draw(QPainter* painter)
{
	GLWidget::draw(painter);

	painter->beginNativePainting();

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	switch (m_state)
	{
	case FREE: glColor3ub(255, 255, 0); break;
	case FIXED_SIZE: glColor3ub(255, 128, 0); break;
	case LOCKED: glColor3ub(255, 0, 0); break;
	}

	glLineWidth(2.f);
	int x0 = x() - 1;
	int y0 = y() - 1;
	int x1 = x() + w() + 1;
	int y1 = y() + h() + 1;
	glBegin(GL_LINE_LOOP);
	{
		glVertex2i(x0, y0);
		glVertex2i(x1, y0);
		glVertex2i(x1, y1);
		glVertex2i(x0, y1);
	}
	glEnd();

	glPopAttrib();
	painter->endNativePainting();
}

bool GLSafeFrame::is_inside(int x, int y)
{
	int x0 = m_x;
	int y0 = m_y;
	int x1 = m_x + m_w;
	int y1 = m_y + m_h;

	// see if it's on the border
	int a = 2;
	if ((x >= x0 - a) && (x <= x0 + a) && (y >= y0) && (y <= y1)) return true;
	if ((x >= x1 - a) && (x <= x1 + a) && (y >= y0) && (y <= y1)) return true;
	if ((y >= y0 - a) && (y <= y0 + a) && (x >= x0) && (x <= x1)) return true;
	if ((y >= y1 - a) && (y <= y1 + a) && (x >= x0) && (x <= x1)) return true;

	// see if it's in the resize triangle
	double r = (m_x + m_w - x) / 20.0;
	double s = (m_y + m_h - y) / 20.0;
	if ((r >= 0) && (s >= 0) && (r + s <= 1.0))
	{
		return true;
	}

	return false;
}
