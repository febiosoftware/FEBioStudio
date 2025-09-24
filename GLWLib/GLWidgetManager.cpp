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

#include "GLWidgetManager.h"
#include <QOpenGLWidget>
#include <assert.h>
#include <QPainter>

CGLWidgetManager::CGLWidgetManager()
{
	m_pview = nullptr;
}

CGLWidgetManager::~CGLWidgetManager()
{
	for (int i = 0; i < (int)m_Widget.size(); ++i)
	{
		GLWidget* pw = m_Widget[i];
		assert(pw->m_parent == this);
		pw->m_parent = nullptr;
		delete pw;
	}
	m_Widget.clear();
}

void CGLWidgetManager::AttachToView(QOpenGLWidget* pview)
{
	assert(pview);
	m_pview = pview;
}

// Make sure widget are within bounds. (Call when parent QOpenGLWidget changes size)
void CGLWidgetManager::CheckWidgetBounds()
{
	// make sure we have a view
	if (m_pview == nullptr) return;

	// get the view's dimensions
	int w = m_pview->width();
	int h = m_pview->height();

	// resize widgets
	for (int i = 0; i<Widgets(); ++i)
	{
		GLWidget* pw = m_Widget[i];

		int x0 = pw->x();
		if (x0 < 0) x0 = 0;

		int y0 = pw->y();
		if (y0 < 0) y0 = 0;

		int x1 = x0 + pw->w();
		if (x1 >= w) { x1 = w - 1; x0 = x1 - pw->w(); }
		if (x0 < 0) x0 = 0;

		int y1 = y0 + pw->h();
		if (y1 >= h) { y1 = h - 1; y0 = y1 - pw->h(); }
		if (y0 < 0) y0 = 0;

		pw->resize(x0, y0, x1 - x0, y1 - y0);
	}
}

void CGLWidgetManager::AddWidget(GLWidget* pw)
{
	assert(pw->m_parent == nullptr);
	pw->m_parent = this;
	m_Widget.push_back(pw);
}

void CGLWidgetManager::RemoveWidget(GLWidget* pw)
{
	std::vector<GLWidget*>::iterator it = m_Widget.begin();
	for (int i=0; i<(int) m_Widget.size(); ++i, ++it)
	{
		if (m_Widget[i] == pw) 
		{
			assert(pw->m_parent == this);
			pw->m_parent = nullptr;
			m_Widget.erase(it);
			break;
		}
	}
}

int CGLWidgetManager::handle(int x, int y, int nevent)
{
	static int xp, yp;
	static int hp, fsp;
	static bool bresize = false;
	static bool bdrag = false;

	// see if there is a widget that wishes to handle this event
	// first we see if the user is trying to select a widget
	if (nevent == GLWEvent::GLW_PUSH)
	{
		bool bsel = false;
		for (int i=0; i<(int) m_Widget.size(); ++i)
		{
			GLWidget* pw = m_Widget[i];
			if (pw->visible() && pw->is_inside(x,y))
			{
				m_Widget[i]->set_focus();
				bsel = true;
				break;
			}
		}
		if (!bsel) GLWidget::set_focus(0);
	}

	GLWidget* pw = GLWidget::get_focus();
	if (pw) 
	{
		// see if the widget wants to handle it
		if (pw->handle(x, y, nevent)) return 1;

		switch (nevent)
		{
		case GLWEvent::GLW_PUSH:
			{
				xp = x;
				yp = y;
				double r = (pw->m_x+pw->m_w-x)/20.0;
				double s = (pw->m_y+pw->m_h-y)/20.0;
				if (pw->resizable() && (r >= 0) && (s >= 0) && (r+s <= 1.0))
				{
					bresize = true;
					bdrag = false;
					hp = pw->m_h;
					fsp = pw->m_font.pointSize();
				}
				else {
					bdrag = true; bresize = false;
				}
			}
			return 1;
		case GLWEvent::GLW_DRAG:
			if (pw->has_focus())
			{
				int x0 = pw->x();
				int y0 = pw->y();
				int w0 = pw->w();
				int h0 = pw->h();
				pw->align(0);

				if (bresize && pw->resizable())
				{
					if (x0 + w0 + (x - xp) >= m_pview->width()) { pw->align(GLW_ALIGN_RIGHT); x = xp; }

					unsigned int n = pw->GetSnap();
					if (y0 + h0 + (y - yp) >= m_pview->height()) { pw->align(n | GLW_ALIGN_BOTTOM); y = yp; }

					pw->resize(x0, y0, w0 + (x - xp), h0 + (y - yp));

					int hn = pw->h();

					float ar = (float)hn / (float)hp;

					pw->m_font.setPointSize((int)(ar * fsp));
				}
				else if (bdrag)
				{
					pw->resize(x0 + (x - xp), y0 + (y - yp), w0, h0);

					if (pw->x() <= 0) { pw->align(GLW_ALIGN_LEFT); x = xp; }
					else if (pw->x() + pw->w() >= m_pview->width()) { pw->align(GLW_ALIGN_RIGHT); x = xp; }

					unsigned int n = pw->GetSnap();
					if (pw->y() <= 0) { pw->align(n | GLW_ALIGN_TOP); y = yp; }
					else if (pw->y() + pw->h() >= m_pview->height()) { pw->align(n | GLW_ALIGN_BOTTOM); y = yp; }
				}
				else return 1;

				xp = x;
				yp = y;
			}
			return 1;
		case GLWEvent::GLW_RELEASE:
			{
				bresize = false;
				bdrag   = false;
			}
			break;
		}
	}

	return 0;
}

void CGLWidgetManager::DrawWidgets(QPainter* painter)
{
	for (int i=0; i<(int) m_Widget.size(); ++i) 
	{
		GLWidget* pw = m_Widget[i];
		if (pw->visible())
		{
			DrawWidget(pw, painter);
		}
	}
}

void CGLWidgetManager::DrawWidget(GLWidget* pw, QPainter* painter)
{
	// now draw the widget
	pw->draw(painter);

	// if the widget has the focus, draw a box around it
	if (pw->has_focus())
	{
		int x0 = pw->m_x;
		int y0 = pw->m_y;
		int x1 = pw->m_x + pw->m_w;
		int y1 = (pw->m_y + pw->m_h);

		painter->setPen(QPen(QColor::fromRgb(0, 0, 128)));
		painter->fillRect(x0, y0, pw->m_w, pw->m_h, QBrush(QColor::fromRgb(200, 200, 200, 64)));

		if (pw->resizable())
		{
			painter->drawLine(x1 - 20, y1, x1 - 1, y1 - 19);
			painter->drawLine(x1 - 15, y1, x1 - 1, y1 - 14);
			painter->drawLine(x1 - 10, y1, x1 - 1, y1 - 9);
			painter->drawLine(x1 - 5, y1, x1 - 1, y1 - 4);
		}
		
		painter->drawRect(x0, y0, pw->m_w, pw->m_h);
	}
}
