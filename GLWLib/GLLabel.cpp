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
#include "GLLabel.h"
#include <QFontMetrics>

GLLabel::GLLabel(int x, int y, int w, int h, const char* szlabel) : GLWidget(x, y, w, h, szlabel)
{
	m_bshadow = false;
	m_shc = GLColor(200, 200, 200);
	m_margin = 5;
	m_align = LeftJustified;
}

void GLLabel::fit_to_size()
{
	if (m_szlabel && (m_szlabel[0] != 0))
	{
		std::string s = processLabel();
		QFontMetrics fm(m_font);
		QSize size = fm.size(Qt::TextExpandTabs, QString::fromStdString(s));
		resize(x(), y(), size.width() + 2 * m_margin, size.height() + 2 * m_margin);
	}
}

void GLLabel::draw(GLPainter* painter)
{
	// see if we need to resize the label to fit the text
	std::string label;
	if (m_szlabel)
	{
		label = processLabel();
		if ((label.size() > 0) && !has_focus())
		{
			QFontMetricsF fm(m_font);
			QSizeF size = fm.size(Qt::TextExpandTabs, QString::fromStdString(label));
			if ((size.width() > w() - 2*m_margin) || (size.height() > h() - 2 * m_margin))
			{
				resize(x(), y(), size.width() + 2 * m_margin, size.height() + 2 * m_margin);
			}
		}
	}

	GLWidget::draw(painter);

	int x0 = m_x;
	int y0 = m_y;
	int x1 = m_x + m_w;
	int y1 = m_y + m_h;

	draw_bg(x0, y0, x1, y1, painter);

	x0 += m_margin;
	y0 += m_margin;
	int w = m_w - 2 * m_margin;
	int h = m_h - 2 * m_margin;

	if (label.size() > 0)
	{
		// set the align flag
		int flags = Qt::AlignVCenter;
		switch (m_align)
		{
		case LeftJustified: flags |= Qt::AlignLeft; break;
		case Centered: flags |= Qt::AlignHCenter; break;
		case RightJustified: flags |= Qt::AlignRight; break;
		}

		if (m_bshadow)
		{
			int dx = m_font.pointSize() / 10 + 1;
			painter->setPen(QColor(m_shc.r, m_shc.g, m_shc.b));
			painter->setFont(m_font);
			painter->drawText(x0 + dx, y0 + dx, w, h, flags, QString::fromStdString(label));
		}
		GLColor fc = get_fg_color();
		QPen pen = painter->pen();
		pen.setColor(QColor(fc.r, fc.g, fc.b));
		painter->setFont(m_font);
		painter->setPen(pen);
		painter->drawText(x0, y0, w, h, flags, QString::fromStdString(label));
	}
}
