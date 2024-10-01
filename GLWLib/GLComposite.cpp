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
#include "GLComposite.h"

GLComposite::GLComposite(int x, int y, int w, int h) : GLWidget(x, y, w, h)
{
	m_resizable = false;
	m_bgFillMode = FILL_COLOR1;
	m_bgFillColor[0] = GLColor(255, 255, 255, 50);
}

void GLComposite::draw(QPainter* painter)
{
	int y = m_y;
	for (GLWidget* w : m_children)
	{
		w->resize(m_x, y, m_w, w->h());
		y += w->h();
	}
	m_h = y - m_y;

	draw_bg(m_x, m_y, m_x + m_w, m_y + m_h, painter);

	for (GLWidget* w : m_children)
	{
		w->draw(painter);
	}
}

int GLComposite::handle(int x, int y, int nevent)
{
	for (GLWidget* w : m_children)
	{
		if (w->handle(x, y, nevent)) return 1;
	}
	return 0;
}
