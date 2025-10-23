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

#include "GLWidget.h"
#include "GLWidgetManager.h"
#include <assert.h>
#include "convert.h"
#include <sstream>
#include "GLPainter.h"
using std::stringstream;

//-----------------------------------------------------------------------------

GLWidget* GLWidget::m_pfocus = 0;

GLColor GLWidget::m_base = GLColor(0,0,0);

QFont GLWidget::m_defaultFont("Helvetica", 13);

std::map<std::string, std::string>	GLWidget::m_stringTable;

GLWidget::GLWidget(int x, int y, int w, int h, const char* szlabel)
{
	m_minw = 30;
	m_minh = 20;
	m_boverridefgc = false;

	m_x = x;
	m_y = y;
	m_w = w;
	m_h = h;
	m_szlabel = (char*) szlabel;

	m_fgc = m_base;
	m_bgFillColor[0] = GLColor(0,0,0,0);
	m_bgFillColor[1] = GLColor(0,0,0,0);
	m_bgFillMode = FILL_NONE;
	m_bgLineMode = LINE_NONE;
	m_bgLineSize = 1;
	m_bgLineColor = GLColor(0, 0, 0, 0);

	m_font = m_defaultFont;

	m_nsnap = 0;

	m_balloc = false;

	m_bshow = true;
}

GLWidget::~GLWidget(void)
{
	if (m_balloc) delete [] m_szlabel;
	m_szlabel = 0;
	if (m_parent) m_parent->RemoveWidget(this);
}

void GLWidget::set_label(const char* szlabel)
{ 
	if (m_balloc) delete[] m_szlabel;
	m_szlabel = 0;
	m_balloc = false;

	m_szlabel = (char*)szlabel; 
}

void GLWidget::copy_label(const char* szlabel)
{
	if (m_balloc) delete [] m_szlabel;
	m_szlabel = 0;
	m_balloc = false;

	int n = (int)strlen(szlabel);
	if (n)
	{
		m_szlabel = new char[n+1];
		strcpy(m_szlabel, szlabel);
		m_balloc = true;
	}
}

bool GLWidget::is_inside(int x, int y)
{
	if ((x>=m_x) && (x <=m_x+m_w) && 
		(y>=m_y) && (y <=m_y+m_h)) return true;

	return false;
}

std::string GLWidget::processLabel() const
{
	if ((m_szlabel == 0) || (m_szlabel[0] == 0)) return "";

	char s[256] = { 0 };
	stringstream ss;
	char* c = m_szlabel;
	while (*c)
	{
		if (*c == '$')
		{
			char* c2 = c;
			++c;
			if (*c && (*c == '('))
			{
				while (*c && (*c != ')')) ++c;
				if (*c && (*c == ')'))
				{
					++c;
					int l = c - c2;
					strncpy(s, c2, l);
					s[l] = 0;

					ss << getStringTableValue(s);
				}
			}
		}
		else if (*c == '\\')
		{
			++c;
			if (*c == 'n') ss << "\n";
			
			if (*c) c++;
		}
		else ss << *c++;
	}
	return ss.str();
}

void GLWidget::clearStringTable()
{
	m_stringTable.clear();
}

void GLWidget::addToStringTable(const std::string& key, const std::string& value)
{
	m_stringTable[key] = value;
}

void GLWidget::addToStringTable(const std::string& key, double value)
{
	char s[256] = { 0 };
	sprintf(s, "%.4g", value);
	m_stringTable[key] = s;
}

std::string GLWidget::getStringTableValue(const std::string& key)
{
	return m_stringTable[key];
}

void GLWidget::draw_bg(int x0, int y0, int x1, int y1, GLPainter* painter)
{
	QColor c1 = toQColorAlpha(m_bgFillColor[0]);
	QColor c2 = toQColorAlpha(m_bgFillColor[1]);

	QRect rt(x0, y0, x1 - x0, y1 - y0);

	switch (m_bgFillMode)
	{
	case FILL_NONE: break;
	case FILL_COLOR1:
		painter->fillRect(rt, c1);
		break;
	case FILL_COLOR2:
		painter->fillRect(rt, c2);
		break;
	case FILL_HORIZONTAL:
	{
		QLinearGradient grad(rt.topLeft(), rt.topRight());
		grad.setColorAt(0, c1);
		grad.setColorAt(1, c2);
		painter->fillRect(rt, grad);
	}
	break;
	case FILL_VERTICAL:
	{
		QLinearGradient grad(rt.topLeft(), rt.bottomLeft());
		grad.setColorAt(0, c1);
		grad.setColorAt(1, c2);
		painter->fillRect(rt, grad);
	}
	break;
	}

	switch (m_bgLineMode)
	{
	case LINE_NONE: break;
	case LINE_SOLID:
	{
		painter->setPen(QPen(toQColor(m_bgLineColor), m_bgLineSize));
		painter->drawRect(rt);
	}
	break;
	}
}

void GLWidget::call_event_handlers(int nevent)
{
	for (auto f : m_eventHandlers)
	{
		f(this, nevent);
	}
}

void GLWidget::draw(GLPainter* painter)
{
	if (painter) snap_to_bounds(*painter);
}

void GLWidget::snap_to_bounds(GLPainter& painter)
{
	int W = painter.deviceWidth();
	int H = painter.deviceHeight();

	if      (m_nsnap & GLW_ALIGN_LEFT   ) m_x = 0;
	else if (m_nsnap & GLW_ALIGN_RIGHT  ) m_x = W - m_w - 1;
	else if (m_nsnap & GLW_ALIGN_HCENTER) m_x = W / 2 - m_w / 2;

	if      (m_nsnap & GLW_ALIGN_TOP    ) m_y = 0;
	else if (m_nsnap & GLW_ALIGN_BOTTOM ) m_y = H - m_h - 1;
	else if (m_nsnap & GLW_ALIGN_VCENTER) m_y = H / 2 - m_h / 2;
}
