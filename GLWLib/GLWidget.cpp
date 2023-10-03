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

#ifdef WIN32
#include <Windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "GLWidget.h"
#include <assert.h>
#include "convert.h"
#include <sstream>
using namespace Post;
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

	m_layer = 0;
}

GLWidget::~GLWidget(void)
{
	if (m_balloc) delete [] m_szlabel;
	m_szlabel = 0;
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

//-----------------------------------------------------------------------------

GLBox::GLBox(int x, int y, int w, int h, const char *szlabel) : GLWidget(x, y, w, h, szlabel)
{
	m_bshadow = false;
	m_shc = GLColor(200,200,200);
	m_margin = 5;
	m_align = LeftJustified;
}

void GLBox::fit_to_size()
{
	if (m_szlabel && (m_szlabel[0] != 0))
	{
		string s = processLabel();
		QFontMetrics fm(m_font);
		QSize size = fm.size(Qt::TextExpandTabs, QString::fromStdString(s));
		resize(x(), y(), size.width() + 2 * m_margin, size.height() + 2 * m_margin);
	}
}

void GLBox::draw_bg(int x0, int y0, int x1, int y1, QPainter* painter)
{
	QColor c1 = toQColor(m_bgFillColor[0]);
	QColor c2 = toQColor(m_bgFillColor[1]);

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

void GLBox::draw(QPainter* painter)
{
	int x0 = m_x;
	int y0 = m_y;
	int x1 = m_x + m_w;
	int y1 = m_y + m_h;

	draw_bg(x0, y0, x1, y1, painter);

	x0 += m_margin;
	y0 += m_margin;
	int w = m_w - 2*m_margin;
	int h = m_h - 2*m_margin;

	if (m_szlabel)
	{
		// set the align flag
		int flags = Qt::AlignVCenter;
		switch (m_align)
		{
		case LeftJustified : flags |= Qt::AlignLeft; break;
		case Centered      : flags |= Qt::AlignHCenter; break;
		case RightJustified: flags |= Qt::AlignRight; break;
		}

		string label = processLabel();
		if (m_bshadow)
		{
			int dx = m_font.pointSize()/10+1;
			painter->setPen(QColor(m_shc.r, m_shc.g, m_shc.b));
			painter->setFont(m_font);
			painter->drawText(x0+dx, y0+dx, w, h, flags, QString::fromStdString(label));
		}
		QPen pen = painter->pen();
		pen.setColor(QColor(m_fgc.r, m_fgc.g, m_fgc.b));
		painter->setFont(m_font);
		painter->setPen(pen);
		painter->drawText(x0, y0, w, h, flags, QString::fromStdString(label));
	}
}

//-----------------------------------------------------------------------------

GLLegendBar::GLLegendBar(CColorTexture* pm, int x, int y, int w, int h, int orientation) : GLWidget(x, y, w, h, 0)
{
	m_pMap = pm;
	m_ntype = GRADIENT;
	m_nrot = orientation;
	m_btitle = false;
	m_blabels = true;
	m_nprec = 4;
	m_labelPos = 0;

	m_fmin = 0.0f;
	m_fmax = 1.0f;

	m_lineWidth = 1.f;
}

void GLLegendBar::draw(QPainter* painter)
{
	int x0 = m_x;
	int y0 = m_y;
	int x1 = m_x + m_w;
	int y1 = m_y + m_h;
	draw_bg(x0, y0, x1, y1, painter);

	switch (m_ntype)
	{
	case GRADIENT: 
		if (m_nrot == ORIENT_VERTICAL) draw_gradient_vert(painter);
		else draw_gradient_horz(painter);
		break;
	case DISCRETE:
		if (m_nrot == ORIENT_VERTICAL) draw_discrete_vert(painter); 
		else draw_discrete_horz(painter);
		break;
	default:
		assert(false);
	}
}

void GLLegendBar::SetOrientation(int n)
{
	if (m_nrot == n) return;
	m_nrot = n;

	// swap width and height
	int tmp = m_w;
	m_w = m_h;
	m_h = tmp;
}

void GLLegendBar::SetRange(float fmin, float fmax)
{
	m_fmin = fmin;
	m_fmax = fmax;
}

void GLLegendBar::GetRange(float& fmin, float& fmax)
{
	fmin = m_fmin;
	fmax = m_fmax;
}

int GLLegendBar::GetDivisions()
{
	return m_pMap->GetDivisions();
}

void GLLegendBar::SetDivisions(int n)
{
	m_pMap->SetDivisions(n);
}

bool GLLegendBar::SmoothTexture() const
{
	return m_pMap->GetSmooth();
}

void GLLegendBar::SetSmoothTexture(bool b)
{
	m_pMap->SetSmooth(b);
}

void GLLegendBar::draw_bg(int x0, int y0, int x1, int y1, QPainter* painter)
{
	QColor c1 = toQColor(m_bgFillColor[0]);
	QColor c2 = toQColor(m_bgFillColor[1]);

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

void GLLegendBar::draw_gradient_vert(QPainter* painter)
{
	painter->beginNativePainting();
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	// draw the legend
	int nsteps = m_pMap->GetDivisions();
	if (nsteps < 1) nsteps = 1;

	glDisable(GL_CULL_FACE);

	glEnable(GL_TEXTURE_1D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	m_pMap->GetTexture().MakeCurrent();

	GLint dfnc;
	glGetIntegerv(GL_DEPTH_FUNC, &dfnc);
	glDepthFunc(GL_ALWAYS);

	glDisable(GL_BLEND);

	// provide some room for the title
	int titleHeight = 0;
	if (m_btitle && m_szlabel)
	{
		QFontMetrics fm(m_font);
		titleHeight = fm.height() + 2;
	}

	int x0 = x() + w() - 50;
	int y0 = y() + 30;
	int x1 = x0 + 25;
	int y1 = y0 + h() - 40;

	if (m_labelPos == 1)
	{
		x0 = x() + 25;
		x1 = x0 + 25;
	}

	glColor3ub(255, 255, 255);
	glBegin(GL_QUADS);
	{
		glTexCoord1d(0); glVertex2i(x0, y1); 
		glTexCoord1d(0); glVertex2i(x1, y1); 
		glTexCoord1d(1); glVertex2i(x1, y0); 
		glTexCoord1d(1); glVertex2i(x0, y0); 
	}
	glEnd();

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisable(GL_TEXTURE_1D);

	glDisable(GL_LIGHTING);

	int i, yt, ipow;
	double f, p=1;
	char str[256], pstr[256];
	
	double a = MAX(fabs(m_fmin),fabs(m_fmax));
	if (a > 0)
	{
		double g = log10(a);
		ipow = (int) floor(g);
	}
	else ipow = 0;

	if (m_lineWidth > 0.f)
	{
		glColor3ub(m_fgc.r, m_fgc.g, m_fgc.b);
		glLineWidth(m_lineWidth);
		glBegin(GL_LINES);
		{
			for (i = 0; i <= nsteps; i++)
			{
				yt = y0 + i * (y1 - y0) / nsteps;
				f = m_fmax + i * (m_fmin - m_fmax) / nsteps;

				glVertex2i(x0 + 1, yt);
				glVertex2i(x1 - 1, yt);
			}
		}
		glEnd();
	}

	glDepthFunc(dfnc);

	glPopAttrib();
	painter->endNativePainting();

	if (m_blabels)
	{
		painter->setPen(QColor(m_fgc.r,m_fgc.g,m_fgc.b));
		painter->setFont(m_font);
		QFontMetrics fm(m_font);
	
		if((abs(ipow)>2))
		{
			sprintf(pstr, "x10");
			painter->drawText(x0, y0-5, QString(pstr));

			// change font size and draw superscript
			sprintf(pstr, "%d", ipow);
			QFontMetrics fm = painter->fontMetrics();
			int l = fm.horizontalAdvance(QString("x10"));
			QFont f = m_font;
			f.setPointSize(m_font.pointSize() - 2);
			painter->setFont(f);
			painter->drawText(x0+l, y0-14, QString(pstr));
			
			// reset font size
			painter->setFont(m_font);
			p = pow(10.0, ipow);
		}

		char szfmt[16]={0};
		sprintf(szfmt, "%%.%dg", m_nprec);

		for (i=0; i<=nsteps; i++)
		{
			yt = y0 + i*(y1 - y0)/nsteps;
			f = m_fmax + i*(m_fmin - m_fmax)/nsteps;

			sprintf(str, szfmt, (fabs(f/p) < 1e-5 ? 0 : f/p));
			QString s(str);

			if (m_labelPos == 0)
			{
				int w = fm.horizontalAdvance(s);
				int h = fm.ascent()/2;
		
				painter->drawText(x0-w-5, yt + h, s);
			}
			else
			{
				int h = fm.ascent() / 2;
				painter->drawText(x1 + 5, yt + h, s);
			}
		}
	}

	// render the title
	if (m_btitle && m_szlabel)
	{
		QFontMetrics fm(m_font);
		int w = fm.horizontalAdvance(m_szlabel);
		int x = (x0 + x1) / 2 - w/2;
		int y = y0 - fm.height() - 2;

		painter->setPen(QColor(m_fgc.r, m_fgc.g, m_fgc.b));
		painter->setFont(m_font);
		painter->drawText(x, y, m_szlabel);
	}
}

void GLLegendBar::draw_gradient_horz(QPainter* painter)
{
	painter->beginNativePainting();
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	// draw the legend
	int nsteps = m_pMap->GetDivisions();
	if (nsteps < 1) nsteps = 1;

	glDisable(GL_CULL_FACE);

	glEnable(GL_TEXTURE_1D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	m_pMap->GetTexture().MakeCurrent();

	GLint dfnc;
	glGetIntegerv(GL_DEPTH_FUNC, &dfnc);
	glDepthFunc(GL_ALWAYS);

	glDisable(GL_BLEND);

	int x0 = x() + 10;
	int y0 = y() + h() - 50;
	int x1 = x() + w() - 30;
	int y1 = y0 + 25;

	if (m_labelPos == 1)
	{
		y0 = y() + 25;
		y1 = y0 + 25;
	}

	glColor3ub(255, 255, 255);
	glBegin(GL_QUADS);
	{
		glTexCoord1d(0); glVertex2i(x0, y0);
		glTexCoord1d(0); glVertex2i(x0, y1);
		glTexCoord1d(1); glVertex2i(x1, y1);
		glTexCoord1d(1); glVertex2i(x1, y0);
	}
	glEnd();

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisable(GL_TEXTURE_1D);

	glDisable(GL_LIGHTING);

	int i, ipow;
	double p = 1;
	char str[256], pstr[256];

	double a = MAX(fabs(m_fmin), fabs(m_fmax));
	if (a > 0)
	{
		double g = log10(a);
		ipow = (int)floor(g);
	}
	else ipow = 0;

	if (m_lineWidth > 0.f)
	{
		glColor3ub(m_fgc.r, m_fgc.g, m_fgc.b);
		glLineWidth(m_lineWidth);
		glBegin(GL_LINES);
		{
			for (i = 0; i <= nsteps; i++)
			{
				int xt = x0 + i * (x1 - x0) / nsteps;
				double f = m_fmax + i * (m_fmin - m_fmax) / nsteps;

				glVertex2i(xt, y0 + 1);
				glVertex2i(xt, y1 - 1);
			}
		}
		glEnd();
	}

	glDepthFunc(dfnc);

	glPopAttrib();
	painter->endNativePainting();

	if (m_blabels)
	{
		painter->setPen(QColor(m_fgc.r, m_fgc.g, m_fgc.b));
		painter->setFont(m_font);
		QFontMetrics fm(m_font);

		if ((abs(ipow)>2))
		{
			sprintf(pstr, "x10");
			int x = x1 + 5;
			int y = y1 - 5;
			painter->drawText(x, y, QString(pstr));

			// change font size and draw superscript
			sprintf(pstr, "%d", ipow);
			QFontMetrics fm = painter->fontMetrics();
			int l = fm.horizontalAdvance(QString("x10"));
			QFont f = m_font;
			f.setPointSize(m_font.pointSize() - 2);
			painter->setFont(f);
			painter->drawText(x + l, y - 14, QString(pstr));

			// reset font size
			painter->setFont(m_font);
			p = pow(10.0, ipow);
		}

		char szfmt[16] = { 0 };
		sprintf(szfmt, "%%.%dg", m_nprec);

		for (i = 0; i <= nsteps; i++)
		{
			int xt = x0 + i*(x1 - x0) / nsteps;
			double f = m_fmin + i*(m_fmax - m_fmin) / nsteps;

			sprintf(str, szfmt, (fabs(f / p) < 1e-5 ? 0 : f / p));
			QString s(str);

			if (m_labelPos == 0)
			{
				int w = fm.horizontalAdvance(s);
				int h = fm.ascent() / 2;

				painter->drawText(xt - w/2, y0 - h - 5, s);
			}
			else
			{
				int w = fm.horizontalAdvance(s);
				int h = fm.ascent() / 2;
				painter->drawText(xt - w / 2, y1 + h + 10, s);
			}
		}
	}

	// render the title
	if (m_btitle && m_szlabel)
	{
		painter->setPen(QColor(m_fgc.r, m_fgc.g, m_fgc.b));
		painter->setFont(m_font);

		if (m_nrot == ORIENT_HORIZONTAL)
		{
			painter->drawText(x(), y(), w(), h(), Qt::AlignCenter | Qt::AlignBottom, m_szlabel);
		}
	}
}

void GLLegendBar::draw_discrete_vert(QPainter* painter)
{
	// TODO: implement this
	assert(false);
}

void GLLegendBar::draw_discrete_horz(QPainter* painter)
{
	painter->beginNativePainting();
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	// draw the legend
	int nsteps = m_pMap->GetDivisions();
	if (nsteps < 1) nsteps = 1;

	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);

	GLint dfnc;
	glGetIntegerv(GL_DEPTH_FUNC, &dfnc);
	glDepthFunc(GL_ALWAYS);

	int x0, y0, x1, y1;
	if (m_nrot == ORIENT_VERTICAL)
	{
		x0 = x() + w() - 50;
		y0 = y() + 30;
		x1 = x0 + 25;
		y1 = y0 - 40;
	}
	else
	{
		x0 = x()+30;
		y0 = y() + h() - 90;
		x1 = x()+w()-50;
		y1 = y0 + 25;
	}

	glColor3ub(m_fgc.r,m_fgc.g,m_fgc.b);

	int i, yt, ipow;
	double f, p=1;
	
	double a = MAX(fabs(m_fmin),fabs(m_fmax));
	if (a > 0)
	{
		double g = log10(a);
		ipow = (int) floor(g);
	}
	else ipow = 0;

//	gl_font(m_font, m_font_size);
	
/*	if((abs(ipow)>2) && m_blabels)
	{
		sprintf(pstr, "x10");
		gl_draw(pstr, x1+10, y1, 28, 20, (Fl_Align)(FL_ALIGN_LEFT | FL_ALIGN_INSIDE));
		// change font size and draw superscript
		sprintf(pstr, "%d", ipow);
		int l = (int) fl_width("x10");
		gl_font(m_font, m_font_size-2);
		gl_draw(pstr, x1+10+l, y1, 28, 20, (Fl_Align) (FL_ALIGN_LEFT | FL_ALIGN_INSIDE));
		// reset font size
		gl_font(m_font, m_font_size);
		p = pow(10.0, ipow);
	}

*/

	if (m_blabels)
	{
//		char szfmt[16]={0};
//		sprintf(szfmt, "%%.%dg", m_nprec);

		CColorMap& map = ColorMapManager::GetColorMap(m_pMap->GetColorMap());

		float denom = (nsteps <= 1 ? 1.f : nsteps - 1.f);

		float lineWidth = m_lineWidth;
		if (lineWidth <= 0.f) lineWidth = 1.f;

		// render the lines and text
		for (i=1; i<nsteps+1; i++)
		{
			if (m_nrot == ORIENT_VERTICAL)
			{
				yt = y0 + i*(y1 - y0)/(nsteps+1);
				f = 1.f - (i - 1) / denom;

				glColor3ub(m_fgc.r,m_fgc.g,m_fgc.b);

				GLColor c = map.map((float) f);
				glColor3ub(c.r, c.g, c.b);

				glLineWidth(lineWidth);
				glBegin(GL_LINES);
				{
					glVertex2i(x0+1, yt);
					glVertex2i(x1-1, yt);
				}
				glEnd();
			}
			else
			{
				int xt = x0 + i*(x1 - x0)/(nsteps+1);
				f = (i - 1) / denom;

				glColor3ub(m_fgc.r,m_fgc.g,m_fgc.b);

				GLColor c = map.map((float)f);
				glColor3ub(c.r, c.g, c.b);

				glLineWidth(lineWidth);
				glBegin(GL_LINES);
				{
					glVertex2i(xt, y0+1);
					glVertex2i(xt, y1-1);
				}	
				glEnd();
			}
		}
	}

	glDepthFunc(dfnc);

	glPopAttrib();
	painter->endNativePainting();

	// render the title
	if (m_btitle && m_szlabel)
	{
		painter->setPen(QColor(m_fgc.r, m_fgc.g, m_fgc.b));
		painter->setFont(m_font);

		if (m_nrot == ORIENT_HORIZONTAL)
		{
			painter->drawText(x(), y(), w(), h(), Qt::AlignCenter | Qt::AlignBottom, m_szlabel);
		}
	}

	if (m_blabels)
	{
		painter->setPen(QColor(m_fgc.r, m_fgc.g, m_fgc.b));
		painter->setFont(m_font);
		QFontMetrics fm(m_font);
		int fh = fm.height();

		char szfmt[16]={0}, str[128] = {0};
		sprintf(szfmt, "%%.%dg", m_nprec);

		float denom = (nsteps <= 1 ? 1.f : nsteps - 1.f); 

		// render the lines and text
		for (i = 1; i<nsteps + 1; i++)
		{
			if (m_nrot == ORIENT_VERTICAL)
			{
				yt = y0 + i*(y1 - y0) / (nsteps + 1);
				f = m_fmax + (i-1)*(m_fmin - m_fmax) / denom;

				sprintf(str, szfmt, (fabs(f/p) < 1e-5 ? 0 : f/p));

				painter->drawText(x0 - 55, yt - 8, 50, 20, Qt::AlignRight, str);
			}
			else
			{
				int xt = x0 + i*(x1 - x0) / (nsteps + 1);
				f = m_fmin + (i-1)*(m_fmax - m_fmin) / denom;

				sprintf(str, szfmt, (fabs(f/p) < 1e-5 ? 0 : f/p));

				int fw = fm.horizontalAdvance(str);
				painter->drawText(xt - fw/2, y1 + fh + 5, str);
			}
		}
	}
}

//-----------------------------------------------------------------------------

GLTriad::GLTriad(int x, int y, int w, int h) : GLWidget(x, y, w, h)
{
	m_rot = quatd(0.f, vec3d(1.f,0.f,0.f));
	m_bcoord_labels = true;
}

void GLTriad::draw(QPainter* painter)
{
	painter->beginNativePainting();
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	GLfloat ones[] = {1.f, 1.f, 1.f, 1.f};
	GLfloat ambient[] = {0.0f,0.0f,0.0f,1.f};
	GLfloat specular[] = {0.5f,0.5f,0.5f,1};
	GLfloat emission[] = {0,0,0,1};
	GLfloat	light[] = {0, 0, -1, 0};

	int view[4];
	glGetIntegerv(GL_VIEWPORT, view);
    
	double DPR = painter->device()->devicePixelRatio();

	int x0 = (int)(DPR*x());
	int y0 = view[3]-(int)(DPR*(y() + h()));
	int x1 = x0 + (int)(DPR*w());
	int y1 = view[3]-(int)(DPR*y());
	if (x1 < x0) { x0 ^= x1; x1 ^= x0; x0 ^= x1; }
	if (y1 < y0) { y0 ^= y1; y1 ^= y0; y0 ^= y1; }

	glViewport(x0, y0, x1-x0, y1-y0);

	float ar = 1.f;
	if (h() != 0) ar = fabs((float) w() / (float) h());

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	float d = 1.2f;
	if (ar >= 1.f)	gluOrtho2D(-d*ar, d*ar, -d, d); else gluOrtho2D(-d, d, -d/ar, d/ar);

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
	float a = 180*q.GetAngle()/PI;

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
	glTranslatef(0,0,.8f);
	gluCylinder(pcyl, r1, 0, 0.2, 10, 1);
	glPopMatrix();

	glPushMatrix();
	glRotatef(-90, 1, 0, 0);
	glColor3ub(0, 255, 0);
	gluCylinder(pcyl, r0, r0, .9, 5, 1);
	glTranslatef(0,0,.8f);
	gluCylinder(pcyl, r1, 0, 0.2, 10, 1);
	glPopMatrix();

	glPushMatrix();
	glColor3ub(0, 0, 255);
	gluCylinder(pcyl, r0, r0, .9, 5, 1);
	glTranslatef(0,0,.8f);
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
		y0 = (view[3] - y0)/DPR;
		y1 = (view[3] - y1)/DPR;

		ex.x = x0 + (x1 - x0)*(ex.x + 1)*0.5; ex.y = y0 + (y1 - y0)*(ex.y + 1)*0.5;
		ey.x = x0 + (x1 - x0)*(ey.x + 1)*0.5; ey.y = y0 + (y1 - y0)*(ey.y + 1)*0.5;
		ez.x = x0 + (x1 - x0)*(ez.x + 1)*0.5; ez.y = y0 + (y1 - y0)*(ez.y + 1)*0.5;

		painter->setFont(m_font);
		painter->setPen(toQColor(m_fgc));
		painter->drawText(ex.x, ex.y, "X");
		painter->drawText(ey.x, ey.y, "Y");
		painter->drawText(ez.x, ez.y, "Z");
	}
}

//-----------------------------------------------------------------------------

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
	painter->beginNativePainting();

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	switch (m_state)
	{
	case FREE      : glColor3ub(255, 255, 0); break;
	case FIXED_SIZE: glColor3ub(255, 128, 0); break;
	case LOCKED    : glColor3ub(255, 0, 0); break;
	}

	glLineWidth(2.f);
	int x0 = x()-1;
	int y0 = y()-1;
	int x1 = x()+w()+1;
	int y1 = y()+h()+1;
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
	int x1 = m_x+m_w;
	int y1 = m_y+m_h;

	// see if it's on the border
	int a = 2;
	if ((x>=x0-a) && (x<=x0+a) && (y >= y0) && (y <= y1)) return true;
	if ((x>=x1-a) && (x<=x1+a) && (y >= y0) && (y <= y1)) return true;
	if ((y>=y0-a) && (y<=y0+a) && (x >= x0) && (x <= x1)) return true;
	if ((y>=y1-a) && (y<=y1+a) && (x >= x0) && (x <= x1)) return true;

	// see if it's in the resize triangle
	double r = (m_x + m_w - x) / 20.0;
	double s = (m_y + m_h - y) / 20.0;
	if ((r >= 0) && (s >= 0) && (r + s <= 1.0))
	{
		return true;
	}

	return false;
}
