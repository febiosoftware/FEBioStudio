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
#else
#include <GL/gl.h>
#endif

#include "GLLegendBar.h"
#include "convert.h"
#include <FSCore/ColorMapManager.h>
#include <QPainter>

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
	GLWidget::draw(painter);

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

void GLLegendBar::SetColorGradient(int n)
{
	m_pMap->SetColorMap(n);
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

void setCurrentTexture(GLTexture1D& tex)
{
	unsigned int texID = tex.GetID();
	if (texID == 0)
	{
		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_1D, texID);

		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //(m_bsmooth ? GL_LINEAR : GL_NEAREST));
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //(m_bsmooth ? GL_LINEAR : GL_NEAREST));

		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);

		tex.SetID(texID);
	}
	else glBindTexture(GL_TEXTURE_1D, texID);

	if (tex.DoUpdate())
	{
		glTexImage1D(GL_TEXTURE_1D, 0, 3, tex.Size(), 0, GL_RGB, GL_UNSIGNED_BYTE, tex.GetBytes());
		tex.Update(false);
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

	setCurrentTexture(m_pMap->GetTexture());

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
	double f, p = 1;
	char str[256], pstr[256];

	double a = fmax(fabs(m_fmin), fabs(m_fmax));
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
				yt = y0 + i * (y1 - y0) / nsteps;
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
		painter->setPen(QColor(m_fgc.r, m_fgc.g, m_fgc.b));
		painter->setFont(m_font);
		QFontMetrics fm(m_font);

		if ((abs(ipow) > 2))
		{
			sprintf(pstr, "x10");
			painter->drawText(x0, y0 - 5, QString(pstr));

			// change font size and draw superscript
			sprintf(pstr, "%d", ipow);
			QFontMetrics fm = painter->fontMetrics();
			int l = fm.horizontalAdvance(QString("x10"));
			QFont f = m_font;
			f.setPointSize(m_font.pointSize() - 2);
			painter->setFont(f);
			painter->drawText(x0 + l, y0 - 14, QString(pstr));

			// reset font size
			painter->setFont(m_font);
			p = pow(10.0, ipow);
		}

		char szfmt[16] = { 0 };
		sprintf(szfmt, "%%.%dg", m_nprec);

		for (i = 0; i <= nsteps; i++)
		{
			yt = y0 + i * (y1 - y0) / nsteps;
			if      (i ==      0) f = m_fmax;
			else if (i == nsteps) f = m_fmin;
			else f = m_fmax + i * (m_fmin - m_fmax) / nsteps;

			sprintf(str, szfmt, (fabs(f / p) < 1e-5 ? 0 : f / p));
			QString s(str);

			if (m_labelPos == 0)
			{
				int w = fm.horizontalAdvance(s);
				int h = fm.ascent() / 2;

				painter->drawText(x0 - w - 5, yt + h, s);
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
		int x = (x0 + x1) / 2 - w / 2;
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

	setCurrentTexture(m_pMap->GetTexture());

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

	double a = fmax(fabs(m_fmin), fabs(m_fmax));
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

		if ((abs(ipow) > 2))
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

		double f = 0;
		for (i = 0; i <= nsteps; i++)
		{
			int xt = x0 + i * (x1 - x0) / nsteps;
			if      (i ==      0) f = m_fmin;
			else if (i == nsteps) f = m_fmax;
			else f = m_fmin + i * (m_fmax - m_fmin) / nsteps;

			sprintf(str, szfmt, (fabs(f / p) < 1e-5 ? 0 : f / p));
			QString s(str);

			if (m_labelPos == 0)
			{
				int w = fm.horizontalAdvance(s);
				int h = fm.ascent() / 2;

				painter->drawText(xt - w / 2, y0 - h - 5, s);
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
		x0 = x() + 30;
		y0 = y() + h() - 90;
		x1 = x() + w() - 50;
		y1 = y0 + 25;
	}

	glColor3ub(m_fgc.r, m_fgc.g, m_fgc.b);

	int i, yt, ipow;
	double f, p = 1;

	double a = fmax(fabs(m_fmin), fabs(m_fmax));
	if (a > 0)
	{
		double g = log10(a);
		ipow = (int)floor(g);
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
		for (i = 1; i < nsteps + 1; i++)
		{
			if (m_nrot == ORIENT_VERTICAL)
			{
				yt = y0 + i * (y1 - y0) / (nsteps + 1);
				f = 1.f - (i - 1) / denom;

				glColor3ub(m_fgc.r, m_fgc.g, m_fgc.b);

				GLColor c = map.map((float)f);
				glColor3ub(c.r, c.g, c.b);

				glLineWidth(lineWidth);
				glBegin(GL_LINES);
				{
					glVertex2i(x0 + 1, yt);
					glVertex2i(x1 - 1, yt);
				}
				glEnd();
			}
			else
			{
				int xt = x0 + i * (x1 - x0) / (nsteps + 1);
				f = (i - 1) / denom;

				glColor3ub(m_fgc.r, m_fgc.g, m_fgc.b);

				GLColor c = map.map((float)f);
				glColor3ub(c.r, c.g, c.b);

				glLineWidth(lineWidth);
				glBegin(GL_LINES);
				{
					glVertex2i(xt, y0 + 1);
					glVertex2i(xt, y1 - 1);
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

		char szfmt[16] = { 0 }, str[128] = { 0 };
		sprintf(szfmt, "%%.%dg", m_nprec);

		float denom = (nsteps <= 1 ? 1.f : nsteps - 1.f);

		// render the lines and text
		for (i = 1; i < nsteps + 1; i++)
		{
			if (m_nrot == ORIENT_VERTICAL)
			{
				yt = y0 + i * (y1 - y0) / (nsteps + 1);
				f = m_fmax + (i - 1) * (m_fmin - m_fmax) / denom;

				sprintf(str, szfmt, (fabs(f / p) < 1e-5 ? 0 : f / p));

				painter->drawText(x0 - 55, yt - 8, 50, 20, Qt::AlignRight, str);
			}
			else
			{
				int xt = x0 + i * (x1 - x0) / (nsteps + 1);
				f = m_fmin + (i - 1) * (m_fmax - m_fmin) / denom;

				sprintf(str, szfmt, (fabs(f / p) < 1e-5 ? 0 : f / p));

				int fw = fm.horizontalAdvance(str);
				painter->drawText(xt - fw / 2, y1 + fh + 5, str);
			}
		}
	}
}
