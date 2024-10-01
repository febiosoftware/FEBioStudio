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
#include "GLCheckBox.h"
#include <QPainter>
#include "convert.h"

GLCheckBox::GLCheckBox(int x, int y, int w, int h, const char* szlabel) : GLWidget(x, y, w, h, szlabel)
{
	m_checked = false;
	m_checkRect[0] = m_checkRect[1] = m_checkRect[2] = m_checkRect[3] = 0;
}

void GLCheckBox::draw(QPainter* painter)
{
	int x0 = m_x;
	int y0 = m_y;
	int x1 = m_x + m_w;
	int y1 = m_y + m_h;

	draw_bg(x0, y0, x1, y1, painter);

	const int margin = 2;
	x0 += margin;
	y0 += margin;
	int w = m_w - 2 * margin;
	int h = m_h - 2 * margin;

	QPen oldpen = painter->pen();

	// draw the checkmark
	const int checkSize = 2 * m_h / 3;
	QPen pen = painter->pen();
	pen.setColor(QColor(m_fgc.r, m_fgc.g, m_fgc.b));
	painter->setPen(pen);
	int xl = x0 + 2;
	int yl = y0 + (h - checkSize) / 2;
	painter->drawRoundedRect(xl, yl, checkSize, checkSize, 2, 2);
	m_checkRect[0] = xl;
	m_checkRect[1] = yl;
	m_checkRect[2] = xl + checkSize;
	m_checkRect[3] = yl + checkSize;
	if (m_checked)
	{
		int w = checkSize - 8;
		xl += 4;
		yl += 4;
		pen.setWidth(3);
		painter->setPen(pen);
		painter->drawLine(xl, yl + w / 2, xl + w / 2, yl + w);

		pen.setWidth(2);
		painter->setPen(pen);
		painter->drawLine(xl + w / 2, yl + w, xl + w, yl);
	}

	x0 += checkSize + checkSize / 2;

	if (m_szlabel)
	{
		// set the align flag
		int flags = Qt::AlignVCenter | Qt::AlignLeft;
		std::string label = processLabel();
		painter->setFont(m_font);
		painter->drawText(x0, y0, w, h, flags, QString::fromStdString(label));
	}

	painter->setPen(oldpen);
}

int GLCheckBox::handle(int x, int y, int nevent)
{
	if ((x >= m_checkRect[0]) && (x <= m_checkRect[2]) &&
		(y >= m_checkRect[1]) && (y <= m_checkRect[3]))
	{
		if (nevent == GLWEvent::GLW_PUSH)
		{
			m_checked = !m_checked;
			call_event_handlers(nevent);
			return 1;
		}
	}

	return 0;
}
