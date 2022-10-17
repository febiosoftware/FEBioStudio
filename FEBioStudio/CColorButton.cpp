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

#include "stdafx.h"
#include "CColorButton.h"

CColorButton::CColorButton(QWidget* parent) : QWidget(parent)
{
	m_col = QColor(Qt::black);
	setMinimumSize(sizeHint());
}

void CColorButton::paintEvent(QPaintEvent* ev)
{
	QPainter p(this);
	p.fillRect(rect(), m_col);
	p.setPen(Qt::black);
	QRect rt = rect();
	rt.adjust(0,0,-1,-1);
	p.drawRect(rt);
}

void CColorButton::setColor(const QColor& c)
{ 
	m_col = c;
	repaint();
}

void CColorButton::mouseReleaseEvent(QMouseEvent* ev)
{
	QColorDialog dlg(this);
	QColor col = dlg.getColor(m_col);
	if (col.isValid())
	{
		m_col = col;
		repaint();
		emit colorChanged(m_col);
	}
}
