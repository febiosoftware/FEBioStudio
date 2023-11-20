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
#include "TimelineWidget.h"
#include <QMouseEvent>
#include <QPainter>
#include <math.h>

CTimelineWidget::CTimelineWidget(QWidget* parent) : QWidget(parent)
{
	m_tmin = m_tmax = 0.0;
	m_nselect = -1;
	m_first = m_last = 0;
	m_ftime = 0.0;
	m_ndrag = -1;
	m_drawRangeRect = false;

	setMinimumHeight(50);
}

void CTimelineWidget::clearData()
{
	m_data.clear();
	update();
}

void CTimelineWidget::setSelection(int i)
{
	m_nselect = -1;
	if (m_data.empty() == false)
	{
		if ((i >= m_first) && (i <= m_last)) m_nselect = i;
	}
	update();
}

void CTimelineWidget::setCurrentTime(float ftime)
{
	if (ftime == m_ftime) return;

	bool binc = (ftime > m_ftime);

	m_ftime = ftime;

	if ((m_ftime < m_tmin) || (m_ftime > m_tmax))
	{
		double dt = (m_tmax - m_tmin)*0.5;

		if (binc)
		{
			m_tmin = m_ftime - dt * 0.25;
			m_tmax = m_ftime + dt * 1.75;
		}
		else
		{
			m_tmin = m_ftime - dt * 1.75;
			m_tmax = m_ftime + dt * 0.25;
		}

		if (m_tmin < m_dataMin) { m_tmin = m_dataMin; m_tmax = m_tmin + dt*2; }
		if (m_tmax > m_dataMax) { m_tmax = m_dataMax; m_tmin = m_tmax - dt*2; }

		m_tinc = getscale(m_tmin, m_tmax);
	}

	update();
}

void CTimelineWidget::setRange(int nmin, int nmax)
{
	if (m_data.empty() == false)
	{
		if (nmin > nmax)
		{
			int tmp = nmin;
			nmin = nmax; nmax = tmp;
		}

		if (nmin < 0) nmin = 0;
		if (nmax >= (int)m_data.size()) nmax = (int)m_data.size() - 1;

		m_first = nmin;
		m_last = nmax;
		update();
	}
}

void CTimelineWidget::addPoint(double x, double y, int flag)
{
	DataPoint pt;
	pt.ptf = QPointF(x, y);
	pt.pt = QPoint(0, 0);
	pt.flag = flag;
	m_data.push_back(pt);
}

void CTimelineWidget::setTimePoints(const std::vector< std::pair<double, int> >& data)
{
	// copy data
	m_data.clear();
	double l = 0.0;
	int maxl = 0.0;
	for (int i = 0; i < data.size(); ++i)
	{
		double x = data[i].first;
		double y = 0.0;
		if ((i > 0) && (data[i].first <= data[i - 1].first))
		{
			y = ++l;
		}
		else l = 0.0;
		if (l > maxl) maxl = l;
		addPoint(x, y, data[i].second);
	}
	if (maxl > 0.0)
	{
		for (int i = 0; i < data.size(); ++i)
		{
			QPointF& pt = m_data[i].ptf;
			double l = pt.y() / maxl;
			pt.setY(l);
		}
	}

	for (int i = 0; i < data.size(); ++i)

	// clear selection
	m_nselect = -1;
	m_ndrag = -1;

	// update range
	if (m_data.empty() == false)
	{
		m_dataMin = m_dataMax = m_data[0].ptf.x();
		for (int i = 1; i < (int)m_data.size(); ++i)
		{
			if (m_data[i].ptf.x() < m_dataMin) m_dataMin = m_data[i].ptf.x();
			if (m_data[i].ptf.x() > m_dataMax) m_dataMax = m_data[i].ptf.x();
		}

		if (m_dataMin == m_dataMax) m_dataMax++;

		m_first = 0;
		m_last = (int)m_data.size() - 1;
	}
	else
	{
		m_dataMin = m_dataMax = 0.0;
	}

	UpdateScale();

	// redraw
	update();
}

double CTimelineWidget::getscale(double fmin, double fmax)
{
	// update scale
	double df = fmax - fmin;
	double g = log10(df);
	double d = floor(g) - 1.0;
	double finc = pow(10, d);

	int W = rect().width();
	int nd = W / (int)(df / finc);
	if (nd < 50)
	{
		double finc0 = finc;
		finc = 2 * finc0;
		nd = W / (int)(df / finc);
		if (nd < 50)
		{
			finc = 5 * finc0;
			nd = W / (int)(df / finc);
			if (nd < 50) finc = 10 * finc0;
		}
	}
	return finc;
}

void CTimelineWidget::UpdateScale()
{
	// update scale
	m_tinc = getscale(m_dataMin, m_dataMax);

	m_tmin = floor(m_dataMin / m_tinc)*m_tinc;
	m_tmax = ceil(m_dataMax / m_tinc)*m_tinc;
}

void CTimelineWidget::panTimeRange(double dt)
{
	double newMin = m_tmin - dt;
	double newMax = m_tmax - dt;

	if (newMin < m_dataMin)
	{
		newMin = m_dataMin;
		newMax = newMin + (m_tmax - m_tmin);
	}
	if (newMax > m_dataMax)
	{
		newMax = m_dataMax;
		newMin = newMax - (m_tmax - m_tmin);
	}

	m_tmin = newMin;
	m_tmax = newMax;
}

void CTimelineWidget::resizeEvent(QResizeEvent* ev)
{
	UpdateScale();
}

void CTimelineWidget::mousePressEvent(QMouseEvent* ev)
{
	QPoint pt = ev->pos();
	m_prevPt = pt;

	if (ev->modifiers() && Qt::ControlModifier)
	{
		ev->accept();
		return;
	}

	if (m_data.empty() == false)
	{
		// first see if one the range selectors was clicked
		m_ndrag = -1;
		if (m_leftBox.contains(pt))
		{
			m_ndrag = 0;
			ev->accept();
			return;
		}

		if (m_rightBox.contains(pt))
		{
			m_ndrag = 1;
			ev->accept();
			return;
		}

		if (m_drawRangeRect && m_slideRect.contains(pt))
		{
			m_ndrag = 2;
			ev->accept();
			return;
		}

		if (m_dataRect.contains(pt))
		{

			int yc = (m_dataRect.top() + m_dataRect.bottom()) / 2;
			int Hy = 2 * (m_dataRect.bottom() - yc) / 3;

			// find the time point that is closest to the click
			int xp = ev->x();
			int yp = ev->y();
			int isel = -1;
			int dmin = 0;
			for (int i = m_first; i <= m_last; ++i)
			{
				int xi = m_data[i].pt.x();
				int yi = m_data[i].pt.y();

				int d = abs(xp - xi) + abs(yp - yi);
				if ((isel == -1) || (d < dmin))
				{
					isel = i;
					dmin = d;
				}
			}

			if (isel != -1)
			{
				emit pointClicked(isel);
				ev->accept();
			}
		}
	}
}

void CTimelineWidget::mouseMoveEvent(QMouseEvent* ev)
{
	if (m_ndrag != -1)
	{
		if ((m_ndrag == 0) || (m_ndrag == 1))
		{
			int W = m_dataRect.width();

			int xp = ev->x();
			int yp = ev->y();

			int yc = (m_dataRect.top() + m_dataRect.bottom()) / 2;
			int Hy = 2 * (m_dataRect.bottom() - yc) / 3;

			int dmin = W * 10, imin = -1;
			for (int i = 0; i < (int)m_data.size(); ++i)
			{
				int xi = m_data[i].pt.x();
				int yi = m_data[i].pt.y();

				int d = abs(xp - xi) + abs(yp - yi);
				if (d < dmin)
				{
					dmin = d;
					imin = i;
				}
			}

			if ((m_ndrag == 0) && (imin != m_first))
			{
				m_first = imin;
				if (m_first > m_last)
				{
					m_first = m_last;
					m_last = imin;
					m_ndrag = 1;
				}

				emit rangeChanged(m_first, m_last);

				update();
				ev->accept();
				return;
			}

			if ((m_ndrag == 1) && (imin != m_last))
			{
				m_last = imin;
				if (m_last < m_first)
				{
					m_last = m_first;
					m_first = imin;
					m_ndrag = 0;
				}

				emit rangeChanged(m_first, m_last);

				update();
				ev->accept();
				return;
			}
		}
		else if (m_ndrag == 2)
		{
			QPoint pt = ev->pos();
			int dx = pt.x() - m_prevPt.x();

			double dataRange = m_dataMax - m_dataMin;
			if (dataRange <= 0) return;

			double W = (double)m_rangeRect.width();
			if (W == 0) return;

			double dt = dx * dataRange / W;

			panTimeRange(-dt);
			update();

			m_prevPt = pt;
			ev->accept();
			return;
		}
	}
	else if (ev->modifiers() && Qt::ControlModifier)
	{
		QPoint pt = ev->pos();

		int dx = pt.x() - m_prevPt.x();

		int w = width();
		if (w == 0) w = 1.0;
		double dt = dx * (m_tmax - m_tmin) / w;

		panTimeRange(dt);

		update();
		m_prevPt = pt;

		ev->accept();
		return;
	}
	else
	{
		QRect rt = rect();
		int x0 = rt.left();
		int x1 = rt.right();
		int W = x1 - x0;

		// find the time point that is closest to the click
		int xp = ev->x();
		int yp = ev->y();

		int yc = (m_dataRect.top() + m_dataRect.bottom()) / 2;
		int Hy = 2 * (m_dataRect.bottom() - yc) / 3;

		int isel = -1;
		int dmin = 0;
		for (int i = m_first; i <= m_last; ++i)
		{
			int xi = m_data[i].pt.x();
			int yi = m_data[i].pt.y();

			int d = abs(xp - xi) + abs(yp - yi);
			if ((isel == -1) || (d < dmin))
			{
				isel = i;
				dmin = d;
			}
		}

		if (isel != -1)
		{
			emit pointClicked(isel);
			ev->accept();
		}
	}
}

void CTimelineWidget::mouseReleaseEvent(QMouseEvent* ev)
{
	if (m_ndrag != -1)
	{
		m_ndrag = -1;
		update();
	}
}

void CTimelineWidget::wheelEvent(QWheelEvent* ev)
{
	bool scrollUp = (ev->pixelDelta().y() < 0) || (ev->angleDelta().y() < 0);
	bool scrollDown = (ev->pixelDelta().y() > 0) || (ev->angleDelta().y() > 0);

	if (ev->modifiers() && Qt::ControlModifier)
	{
		double tw = (m_tmax - m_tmin) * 0.5;
		double tc = (m_tmax + m_tmin) * 0.5;

		if (scrollUp)
		{
			m_tmin = tc - tw / 0.95;
			m_tmax = tc + tw / 0.95;
		}
		else if (scrollDown)
		{
			m_tmin = tc - tw * 0.95;
			m_tmax = tc + tw * 0.95;
		}

		if (m_tmin < m_dataMin) m_tmin = m_dataMin;
		if (m_tmax > m_dataMax) m_tmax = m_dataMax;

		m_tinc = getscale(m_tmin, m_tmax);

		update();
	}
	else
	{
		QPoint pt = ev->position().toPoint();
		if (m_dataRect.contains(pt))
		{
			if (scrollDown && (m_nselect < m_last))
			{
				emit pointClicked(m_nselect + 1);
				ev->accept();
			}
			else if (scrollUp && (m_nselect > m_first))
			{
				emit pointClicked(m_nselect - 1);
				ev->accept();
			}
		}
	}
}

void drawBox(QPainter& painter, const QRect& rt, const QColor& c)
{
	painter.fillRect(rt, c);
	painter.setPen(Qt::black);
	int x0 = rt.left();
	int x1 = rt.right();
	int y0 = rt.top();
	int y1 = rt.bottom();
	painter.drawLine(x0, y1, x1, y1);
	painter.drawLine(x1, y0, x1, y1);

	painter.setPen(Qt::white);
	painter.drawLine(x0, y0, x1, y0);
	painter.drawLine(x0, y0, x0, y1 - 1);
}

void drawLeftTriangle(QPainter& painter, const QColor& c, const QPointF& p, int size = 10)
{
	QPointF pt[3];
	pt[0] = p;
	pt[1].setX(p.x()); pt[1].setY(p.y() + size);
	pt[2].setX(p.x() + size); pt[2].setY(p.y());
	painter.setPen(QPen(c));
	painter.setBrush(c);
	painter.drawConvexPolygon(pt, 3);

	painter.setPen(Qt::white);
	painter.drawLine(pt[0], pt[2]);
	painter.setPen(Qt::black);
	painter.drawLine(pt[0], pt[1]);
	painter.drawLine(pt[1], pt[2]);
}

void drawRightTriangle(QPainter& painter, const QColor& c, const QPointF& p, int size = 10)
{
	QPointF pt[3];
	pt[0] = p;
	pt[1].setX(p.x()); pt[1].setY(p.y() + size);
	pt[2].setX(p.x() - size); pt[2].setY(p.y());
	painter.setPen(QPen(c));
	painter.setBrush(c);
	painter.drawConvexPolygon(pt, 3);

	painter.setPen(Qt::white);
	painter.drawLine(pt[0], pt[2]);
	painter.setPen(Qt::black);
	painter.drawLine(pt[0], pt[1]);
	painter.drawLine(pt[1], pt[2]);
}

void CTimelineWidget::paintEvent(QPaintEvent* ev)
{
	QPainter painter(this);

	// get the widget's rectangle
	QRect rt = rect();

	// if there is no data, just fill the rect and return
	if (m_data.empty())
	{
		painter.fillRect(rt, Qt::darkGray);
		return;
	}

	// the font height determines the height of the time bar
	QFontMetrics FM = painter.fontMetrics();
	int fontH = FM.height();

	// update rectangles
	m_timeRect = rt; m_timeRect.setTop(rt.bottom() - fontH - 8);
	m_dataRect = rt; m_dataRect.setBottom(m_timeRect.top() - 1);

	// see if we should draw the range bar
	const int RANGE_HEIGHT = 16;
	m_drawRangeRect = false;
	if ((m_tmin != m_dataMin) || (m_tmax != m_dataMax))
	{
		m_drawRangeRect = true;
		m_rangeRect = m_dataRect;
		m_dataRect.setTop( m_dataRect.top() + RANGE_HEIGHT);
		m_rangeRect.setBottom(m_dataRect.top() - 1);
	}

	// the left, right values are the min,max labels on the time bar
	m_tleft = floor(m_tmin / m_tinc) * m_tinc;
	m_tright = ceil(m_tmax / m_tinc) * m_tinc;

	// draw the rangebar (if needed)
	if (m_drawRangeRect) drawRangeBar(painter);

	// draw the data bar
	drawDataBar(painter);

	// draw the time bar
	drawTimeBar(painter);
}

void CTimelineWidget::drawTimeBar(QPainter& painter)
{
	int x0 = m_timeRect.left();
	int x1 = m_timeRect.right();
	int y0 = m_timeRect.top();
	int y1 = m_timeRect.bottom();
	int W = m_timeRect.width();

	QFontMetrics FM = painter.fontMetrics();
	int fontH = FM.height();

	painter.setRenderHint(QPainter::Antialiasing, false);
	painter.fillRect(m_timeRect, Qt::lightGray);
	painter.setPen(Qt::white);
	painter.drawLine(x0, y0, x1, y0);
	painter.setPen(Qt::darkGray);
	painter.drawLine(x0, y1, x1, y1);
	painter.setPen(Qt::black);

	double t = m_tleft;
	while (t <= m_tright)
	{
		int x = x0 + (int)((t - m_tmin) / (m_tmax - m_tmin) * W);
		painter.drawLine(x, y0, x, y0 + 5);
		t += m_tinc;
	}

	painter.setRenderHint(QPainter::Antialiasing, true);
	t = m_tleft;
	while (t <= m_tright)
	{
		int x = x0 + (int)((t - m_tmin) / (m_tmax - m_tmin) * W);

		QString txt = QString::number(t);
		int w = FM.horizontalAdvance(txt);
		painter.drawText(x - w / 2, y0 + 5 + fontH, txt);

		t += m_tinc;
	}
}

void CTimelineWidget::drawDataBar(QPainter& painter)
{
	// draw the grid lines
	int x0 = m_dataRect.left();
	int x1 = m_dataRect.right();
	int y0 = m_dataRect.top();
	int y1 = m_dataRect.bottom();
	int W = m_dataRect.width();

	int xf = x0 + (int)((m_data[m_first].ptf.x() - m_tmin) / (m_tmax - m_tmin) * W);
	int xl = x0 + (int)((m_data[m_last].ptf.x() - m_tmin) / (m_tmax - m_tmin) * W);

	// draw the time rects background
	QRect rt1 = m_dataRect; rt1.setRight(xf);
	QRect rt2 = m_dataRect; rt2.setLeft(rt1.right()); rt2.setRight(xl);
	QRect rt3 = m_dataRect; rt3.setLeft(rt2.right());

	if (rt1.width() > 0) painter.fillRect(rt1, QColor::fromRgb(64, 64, 64));
	if (rt2.width() > 0) painter.fillRect(rt2, Qt::darkGray);
	if (rt3.width() > 0) painter.fillRect(rt3, QColor::fromRgb(64, 64, 64));

	// draw the range selectors
	const int boxSize = 15;
	m_leftBox = QRect(xf, y0, boxSize, boxSize);
	m_rightBox = QRect(xl - boxSize, y0, boxSize, boxSize);

	//	drawBox(painter, m_leftBox, (m_ndrag == 0 ? Qt::white : Qt::lightGray));
	drawLeftTriangle(painter, (m_ndrag == 0 ? Qt::white : Qt::lightGray), QPointF(xf, y0), boxSize);
	//	drawBox(painter, m_rightBox, (m_ndrag == 1 ? Qt::white : Qt::lightGray));
	drawRightTriangle(painter, (m_ndrag == 1 ? Qt::white : Qt::lightGray), QPointF(xl, y0), boxSize);

	painter.setRenderHint(QPainter::Antialiasing, true);


	double t = m_tleft;
	while (t <= m_tright)
	{
		int x = x0 + (int)((t - m_tmin) / (m_tmax - m_tmin) * W);
		painter.setPen(QPen(Qt::lightGray, 1));
		painter.drawLine(x, y0, x, y1);
		t += m_tinc;
	}

	// draw the current time value
	int x = x0 + (int)((m_ftime - m_tmin) / (m_tmax - m_tmin) * W);
	painter.setPen(Qt::black);
	painter.drawLine(x, y0, x, y1);

	// update data point positions
	int yc = (m_dataRect.top() + m_dataRect.bottom()) / 2;
	int Hy = 4 * (m_dataRect.height()) / 5;
	double maxl = 0.0;
	for (int i = 0; i < (int)m_data.size(); ++i)
	{
		double l = m_data[i].ptf.y();
		if (l > maxl) maxl = l;
	}

	for (int i = 0; i < (int)m_data.size(); ++i)
	{
		double t = m_data[i].ptf.x();
		double l = m_data[i].ptf.y();
		int x = x0 + (int)((t - m_tmin) / (m_tmax - m_tmin) * W);
		int y = yc;
		if (maxl != 0)
		{
			y = yc - 0.5 * Hy + Hy * l;
		}
		m_data[i].pt = QPoint(x, y);
	}

	// draw the data
	painter.setPen(Qt::black);
	int xp, yp;
	const int R = 9;
	const int R2 = 14;
	for (int i = 0; i < m_data.size(); ++i)
	{
		int x = m_data[i].pt.x();
		int y = m_data[i].pt.y();
		if (i != 0)
		{
			painter.drawLine(xp, yp, x, y);
		}
		xp = x;
		yp = y;
	}

	painter.setBrush(Qt::gray);
	for (int i = 0; i < m_first; ++i)
	{
		int x = m_data[i].pt.x();
		int y = m_data[i].pt.y();
		painter.drawEllipse(x - R / 2, y - R / 2, R, R);
	}

	for (int i = m_first; i <= m_last; ++i)
	{
		int x = m_data[i].pt.x();
		int y = m_data[i].pt.y();
		if (i != m_nselect)
		{
			int nflag = m_data[i].flag;
			switch (nflag)
			{
			case 0: painter.setBrush(Qt::green); break;
			case 1: painter.setBrush(Qt::magenta); break;
			case 2: painter.setBrush(Qt::yellow); break;
			case 3: painter.setBrush(Qt::red); break;
			case 4: painter.setBrush(Qt::blue); break;
			case 5: painter.setBrush(Qt::cyan); break;
			default: painter.setBrush(Qt::lightGray); break;
			}
			painter.drawEllipse(x - R / 2, y - R / 2, R, R);
		}
	}
	painter.setBrush(Qt::gray);
	for (int i = m_last + 1; i < (int)m_data.size(); ++i)
	{
		int x = m_data[i].pt.x();
		int y = m_data[i].pt.y();
		painter.drawEllipse(x - R / 2, y - R / 2, R, R);
	}

	if (m_nselect != -1)
	{
		painter.setPen(QPen(Qt::white, 2));
		int nflag = m_data[m_nselect].flag;
		switch (nflag)
		{
		case 0: painter.setBrush(Qt::green); break;
		case 1: painter.setBrush(Qt::magenta); break;
		case 2: painter.setBrush(Qt::yellow); break;
		default: painter.setBrush(Qt::lightGray); break;
		}
		int x = m_data[m_nselect].pt.x();
		int y = m_data[m_nselect].pt.y();
		painter.drawEllipse(x - R2 / 2, y - R / 2, R2, R2);
	}
}

void CTimelineWidget::drawRangeBar(QPainter& painter)
{
	double dataRange = m_dataMax - m_dataMin;
	if (dataRange <= 0) dataRange = 1.0;

	int x0 = m_rangeRect.left() + (int)(m_rangeRect.width()*(m_tmin - m_dataMin) / dataRange);
	int x1 = m_rangeRect.left() + (int)(m_rangeRect.width()*(m_tmax - m_dataMin) / dataRange);
	int y0 = m_rangeRect.top();
	int y1 = m_rangeRect.bottom();
	m_slideRect = QRect(x0, y0, x1 - x0, m_rangeRect.height());

	// draw the background
	int xf = m_rangeRect.left() + (int)((m_data[m_first].ptf.x() - m_dataMin) / dataRange * m_rangeRect.width());
	int xl = m_rangeRect.left() + (int)((m_data[m_last ].ptf.x() - m_dataMin) / dataRange * m_rangeRect.width());
	QRect rt1 = m_rangeRect; rt1.setRight(xf);
	QRect rt2 = m_rangeRect; rt2.setLeft(rt1.right()); rt2.setRight(xl);
	QRect rt3 = m_rangeRect; rt3.setLeft(rt2.right());
	if (rt1.width() > 0) painter.fillRect(rt1, QColor::fromRgb(32, 32, 32));
	if (rt2.width() > 0) painter.fillRect(rt2, Qt::black);
	if (rt3.width() > 0) painter.fillRect(rt3, QColor::fromRgb(32, 32, 32));

	// draw the current time point
	int xc = m_rangeRect.left() + (int)(m_rangeRect.width() * (m_ftime - m_dataMin) / dataRange);
	painter.setPen(Qt::darkGray); painter.drawLine(xc, y0, xc, y1);

	// draw the slider bar
	painter.fillRect(x0, m_rangeRect.top(), x1 - x0, m_rangeRect.height(), Qt::gray);
	painter.setPen(Qt::black); painter.drawLine(x0, y1, x1, y1); painter.drawLine(x1, y0, x1, y1);
	painter.setPen(Qt::white); painter.drawLine(x0, y0, x1, y0); painter.drawLine(x0, y0, x0, y1);
}
