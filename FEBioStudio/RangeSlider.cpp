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
#include "RangeSlider.h"
#include <QPainter>
#include <QMouseEvent>

CRangeSlider::CRangeSlider(QWidget* parent) : QWidget(parent)
{
	m_min = 0.0;
	m_max = 1.0;

	m_val1 = 0.3;
	m_val2 = 0.7;

	m_sel = -1;

	m_selColor = QColor::fromRgb(0, 128, 255);
}

const int handleWidth  = 11;
const int handleHeight = 17;
const int grooveHeight = 5;

QRect CRangeSlider::grooveRect()
{
	QRect rt = rect();
	return QRect(rt.left() + handleWidth/2-1, rt.center().y() - grooveHeight / 2, rt.width() - handleWidth+2, grooveHeight);
}

QRect CRangeSlider::selectionRect()
{
	QRect rt = grooveRect();
	int x1 = rt.left() + rt.width() * ((m_val1 - m_min) / (m_max - m_min));
	int x2 = rt.left() + rt.width() * ((m_val2 - m_min) / (m_max - m_min));
	return QRect(x1, rt.top(), (x2 - x1 + 1), grooveHeight);
}

QRect CRangeSlider::leftHandleRect()
{
	QRect rt = grooveRect();
	int x1 = rt.left() + rt.width() * ((m_val1 - m_min) / (m_max - m_min));
	return QRect(x1 - handleWidth / 2, rt.center().y() - handleHeight / 2, handleWidth, handleHeight);
}

QRect CRangeSlider::rightHandleRect()
{
	QRect rt = grooveRect();
	int x2 = rt.left() + rt.width() * ((m_val2 - m_min) / (m_max - m_min));
	return QRect(x2 - handleWidth / 2, rt.center().y() - handleHeight / 2, handleWidth, handleHeight);
}

void CRangeSlider::paintEvent(QPaintEvent* event)
{
	if (m_max == m_min) return;

	QPainter painter(this);

	// draw the background groove
	QRect groove = grooveRect();
	painter.setPen(QPen(QColor::fromRgb(64,64,64)));
	painter.drawRoundedRect(groove, grooveHeight/2, grooveHeight/2);

	// draw the selection
	QRect selRect = selectionRect();
	painter.setBrush(QBrush(m_selColor));
	painter.drawRect(selRect);

	// draw left handle
	const QColor c0 = palette().shadow().color();
	const QColor c1 = palette().light().color();

	QBrush handleBrush = palette().color(QPalette::Button);
	QRect handle1 = leftHandleRect();
	painter.fillRect(handle1, handleBrush);
	painter.setPen(c0);
	painter.drawLine(handle1.right(), handle1.top(), handle1.right(), handle1.bottom());
	painter.drawLine(handle1.left(), handle1.bottom(), handle1.right(), handle1.bottom());
	painter.setPen(c1);
	painter.drawLine(handle1.left(), handle1.top(), handle1.right(), handle1.top());
	painter.drawLine(handle1.left(), handle1.top(), handle1.left(), handle1.bottom());

	// draw right handle
	QRect handle2 = rightHandleRect();
	painter.fillRect(handle2, handleBrush);
	painter.setPen(c0);
	painter.drawLine(handle2.right(), handle2.top(), handle2.right(), handle2.bottom());
	painter.drawLine(handle2.left(), handle2.bottom(), handle2.right(), handle2.bottom());
	painter.setPen(c1);
	painter.drawLine(handle2.left(), handle2.top(), handle2.right(), handle2.top());
	painter.drawLine(handle2.left(), handle2.top(), handle2.left(), handle2.bottom());
}

void CRangeSlider::mousePressEvent(QMouseEvent* ev)
{
	if (m_max == m_min) { ev->ignore(); return; }

	ev->accept();

	QPoint pt = ev->pos();
	m_p0 = pt;

	// see if a handle is selected 
	QRect rt1 = leftHandleRect();
	if (rt1.contains(pt))
	{
		m_sel = 0;
		return;
	}

	QRect rt2 = rightHandleRect();
	if (rt2.contains(pt))
	{
		m_sel = 1;
		return;
	}

	QRect rts = selectionRect();
	if (rts.contains(pt))
	{
		m_sel = 2;
		return;
	}
}

void CRangeSlider::mouseMoveEvent(QMouseEvent* ev)
{
	if (m_max == m_min) { ev->ignore(); return; }
	if (m_sel == -1) { ev->ignore(); return; }
	ev->accept();

	int x = ev->pos().x();

	QRect rt = grooveRect();
	if (m_sel == 0)
	{
		double f = (double)(x - rt.left()) / (double)rt.width();
		double newVal = m_min + f * (m_max - m_min);
		setLeftPosition(newVal);
	}
	else if (m_sel == 1)
	{
		double f = (double)(x - rt.left()) / (double)rt.width();
		double newVal = m_min + f * (m_max - m_min);
		setRightPosition(newVal);
	}
	else if (m_sel == 2)
	{
		int xp = m_p0.x();
		int dx = x - xp;
		double df = (double) dx / (double) rt.width();

		double v1 = m_val1 + df;
		double v2 = m_val2 + df;
		if ((v1 > m_min) && (v2 < m_max))
		{
			setPositions(v1, v2);
		}

		m_p0 = ev->pos();
	}
}

void CRangeSlider::mouseReleaseEvent(QMouseEvent* ev)
{
	if (m_max == m_min) { ev->ignore(); return; }
	if (m_sel == -1) { ev->ignore(); return; }
	ev->accept();
	m_sel = -1;
}

void CRangeSlider::setLeftPosition(double newVal)
{
	if (newVal == m_val1) return;

	if (newVal < m_min) newVal = m_min;
	if (newVal > m_max) newVal = m_max;

	m_val1 = newVal;
	if (newVal > m_val2) m_val2 = newVal;

	emit positionChanged();

	repaint();
}

void CRangeSlider::setRightPosition(double newVal)
{
	if (newVal == m_val2) return;

	if (newVal < m_min) newVal = m_min;
	if (newVal > m_max) newVal = m_max;

	m_val2 = newVal;
	if (newVal < m_val1) m_val1 = newVal;

	emit positionChanged();

	repaint();
}

void CRangeSlider::setPositions(double leftVal, double rightVal)
{
	if ((leftVal == m_val1) && (rightVal == m_val2)) return;

	if (leftVal < m_min) leftVal = m_min;
	if (leftVal > m_max) leftVal = m_max;

	if (rightVal < m_min) rightVal = m_min;
	if (rightVal > m_max) rightVal = m_max;

	if (rightVal < leftVal) rightVal = leftVal;

	m_val1 = leftVal;
	m_val2 = rightVal;

	emit positionChanged();

	repaint();
}

void CRangeSlider::setRange(double minVal, double maxVal)
{
	if (maxVal < minVal) maxVal = minVal;
	m_min = minVal;
	m_max = maxVal;

	if (m_val1 < minVal) m_val1 = minVal;
	if (m_val1 > maxVal) m_val1 = maxVal;

	if (m_val2 < minVal) m_val2 = minVal;
	if (m_val2 > maxVal) m_val2 = maxVal;

	emit positionChanged();

	repaint();
}

double CRangeSlider::leftPosition() const { return m_val1; }
double CRangeSlider::rightPosition() const { return m_val2; }

QSize CRangeSlider::sizeHint() const
{
	const int sliderLength = 84;
	return QSize(sliderLength, 3 * handleHeight / 2);
}

QSize CRangeSlider::minimumSizeHint() const
{
	QSize s = sizeHint();
	s.setWidth(3 * handleWidth / 2);
	return s;
}

void CRangeSlider::setSelectionColor(QColor c)
{
	m_selColor = c;
	repaint();
}
