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
#include "GLLegendBar.h"
#include "convert.h"
#include <FSCore/ColorMapManager.h>
#include <GLLib/ColorTexture.h>
#include <QFontMetrics>

GLLegendBar::GLLegendBar(int x, int y, int w, int h, int orientation) : GLWidget(x, y, w, h, 0)
{
	m_pMap = new CColorTexture;
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

void GLLegendBar::draw(GLPainter* painter)
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

void GLLegendBar::draw_bg(int x0, int y0, int x1, int y1, GLPainter* painter)
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

void GLLegendBar::draw_gradient_vert(GLPainter* painter)
{
	int nsteps = m_pMap->GetDivisions();
	if (nsteps < 1) nsteps = 1;

	int x0 = x() + w() - 50;
	int y0 = y() + 30;
	int x1 = x0 + 25;
	int y1 = y0 + h() - 40;

	if (m_labelPos == 1)
	{
		x0 = x() + 25;
		x1 = x0 + 25;
	}

	int ipow;
	double p = 1;
	double a = fmax(fabs(m_fmin), fabs(m_fmax));
	if (a > 0)
	{
		double g = log10(a);
		ipow = (int)floor(g);
	}
	else ipow = 0;

	// draw colored bar
	int W = x1 - x0;
	int H = y1 - y0;
	if ((W > 1) && (H > 1))
	{
		QImage img(QSize(W, H), QImage::Format_RGB888);
		GLTexture1D& tex = m_pMap->GetTexture();

		for (int j = 0; j < H; ++j)
		{
			float w = 1.f - (float)j / (float)(H - 1.f);
			QColor c = toQColor(tex.sample(w));
			for (int i = 0; i < W; ++i)
			{
				img.setPixelColor(i, j, c);
			}
		}

		painter->drawImage(x0, y0, img);
	}

	// lines
	if (m_lineWidth > 0.f)
	{
		QPen pen(toQColor(m_fgc), m_lineWidth);
		painter->setPen(pen);
		for (int i = 0; i <= nsteps; i++)
		{
			int yt = y0 + i * (y1 - y0) / nsteps;
			painter->drawLine(x0, yt, x1, yt);
		}
	}

	// labels
	if (m_blabels)
	{
		painter->setPen(QColor(m_fgc.r, m_fgc.g, m_fgc.b));
		painter->setFont(m_font);
		QFontMetrics fm(m_font);

		if ((abs(ipow) > 2))
		{
			char pstr[256];
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

		for (int i = 0; i <= nsteps; i++)
		{
			int yt = y0 + i * (y1 - y0) / nsteps;

			double f;
			if      (i ==      0) f = m_fmax;
			else if (i == nsteps) f = m_fmin;
			else f = m_fmax + i * (m_fmin - m_fmax) / nsteps;

			char str[256];
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

void GLLegendBar::draw_gradient_horz(GLPainter* painter)
{
	int nsteps = m_pMap->GetDivisions();
	if (nsteps < 1) nsteps = 1;

	int x0 = x() + 10;
	int y0 = y() + h() - 50;
	int x1 = x() + w() - 30;
	int y1 = y0 + 25;

	if (m_labelPos == 1)
	{
		y0 = y() + 25;
		y1 = y0 + 25;
	}

	double a = fmax(fabs(m_fmin), fabs(m_fmax));
	int ipow;
	if (a > 0)
	{
		double g = log10(a);
		ipow = (int)floor(g);
	}
	else ipow = 0;
	double p = pow(10.0, ipow);

	int W = x1 - x0;
	int H = y1 - y0;
	if ((W > 1) && (H > 1))
	{
		QImage img(QSize(W, H), QImage::Format_RGB888);
		GLTexture1D& tex = m_pMap->GetTexture();

		for (int i = 0; i < W; ++i)
		{
			float w = (float)i / (float)(W - 1.f);
			QColor c = toQColor(tex.sample(w));
			for (int j = 0; j < H; ++j)
			{
				img.setPixelColor(i, j, c);
			}
		}

		painter->drawImage(x0, y0, img);
	}

	// lines
	if (m_lineWidth > 0.f)
	{
		QPen pen(toQColor(m_fgc), m_lineWidth);
		painter->setPen(pen);
		for (int i = 0; i <= nsteps; i++)
		{
			int xt = x0 + i * (x1 - x0) / nsteps;
			painter->drawLine(xt, y0, xt, y1);
		}
	}

	// labels
	if (m_blabels)
	{
		painter->setPen(QColor(m_fgc.r, m_fgc.g, m_fgc.b));
		painter->setFont(m_font);
		QFontMetrics fm(m_font);

		if ((abs(ipow) > 2))
		{
			char pstr[256];
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
		}

		char szfmt[16] = { 0 };
		sprintf(szfmt, "%%.%dg", m_nprec);

		double f = 0;
		char str[256];
		for (int i = 0; i <= nsteps; i++)
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

void GLLegendBar::draw_discrete_vert(GLPainter* painter)
{
	// TODO: implement this
	assert(false);
}

void GLLegendBar::draw_discrete_horz(GLPainter* painter)
{
	int nsteps = m_pMap->GetDivisions();
	if (nsteps < 1) nsteps = 1;

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

	double a = fmax(fabs(m_fmin), fabs(m_fmax));
	int ipow;
	if (a > 0)
	{
		double g = log10(a);
		ipow = (int)floor(g);
	}
	else ipow = 0;
	double p = 1;

	float lineWidth = m_lineWidth;
	if (lineWidth <= 0.f) lineWidth = 1.f;

	QPen pen(toQColor(m_fgc), m_lineWidth);
	painter->setPen(pen);

	const CColorMap& map = m_pMap->ColorMap();

	float denom = (nsteps <= 1 ? 1.f : nsteps - 1.f);

	// render the lines
	for (int i = 1; i < nsteps + 1; i++)
	{
		if (m_nrot == ORIENT_VERTICAL)
		{
			double yt = y0 + i * (y1 - y0) / (nsteps + 1);
			double f = 1.f - (i - 1) / denom;

			GLColor c = map.map((float)f);
			pen.setColor(toQColor(c));
			painter->setPen(pen);
			painter->drawLine(x0 + 1, yt, x1 - 1, yt);
		}
		else
		{
			int xt = x0 + i * (x1 - x0) / (nsteps + 1);
			double f = (i - 1) / denom;

			GLColor c = map.map((float)f);
			pen.setColor(toQColor(c));
			painter->setPen(pen);

			painter->drawLine(xt, y0 + 1, xt, y1 - 1);
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
		for (int i = 1; i < nsteps + 1; i++)
		{
			if (m_nrot == ORIENT_VERTICAL)
			{
				double yt = y0 + i * (y1 - y0) / (nsteps + 1);
				double f = m_fmax + (i - 1) * (m_fmin - m_fmax) / denom;

				sprintf(str, szfmt, (fabs(f / p) < 1e-5 ? 0 : f / p));

				painter->drawText(x0 - 55, yt - 8, 50, 20, Qt::AlignRight, str);
			}
			else
			{
				int xt = x0 + i * (x1 - x0) / (nsteps + 1);
				double f = m_fmin + (i - 1) * (m_fmax - m_fmin) / denom;

				sprintf(str, szfmt, (fabs(f / p) < 1e-5 ? 0 : f / p));

				int fw = fm.horizontalAdvance(str);
				painter->drawText(xt - fw / 2, y1 + fh + 5, str);
			}
		}
	}
}
