/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2026 University of Utah, The Trustees of Columbia University in
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
#include "ChartPainter.h"
#include <QPen>
#include <QBrush>
#include <QPainterPath>

static double calculateScale(double fmin, double fmax)
{
	double dx = fmax - fmin;
	double p = floor(log10(dx));
	double f = pow(10.0, p - 1);
	double m = floor(dx / f);

	double dd = f;
	if (m > 75) dd = 10 * f;
	else if (m > 30) dd = 5 * f;
	else if (m > 15) dd = 2 * f;

	return dd;
}

void drawDiamond(QPainter& painter, const QRect& rt)
{
	QPoint c = rt.center();
	const QPointF points[4] = {
		QPointF(c.x(), rt.top()),
		QPointF(rt.left(), c.y()),
		QPointF(c.x(), rt.bottom()),
		QPointF(rt.right(), c.y())
	};

	painter.drawConvexPolygon(points, 4);
}

void drawTriangle(QPainter& painter, const QRect& rt)
{
	QPoint c = rt.center();
	const QPointF points[3] = {
		QPointF(c.x(), rt.top()),
		QPointF(rt.left(), rt.bottom()),
		QPointF(rt.right(), rt.bottom()),
	};

	painter.drawConvexPolygon(points, 3);
}

void drawCross(QPainter& painter, const QRect& rt)
{
	painter.drawLine(rt.topLeft(), rt.bottomRight());
	painter.drawLine(rt.topRight(), rt.bottomLeft());
}

void drawPlus(QPainter& painter, const QRect& rt)
{
	QPoint c = rt.center();
	painter.drawLine(c.x(), rt.top(), c.x(), rt.bottom());
	painter.drawLine(rt.left(), c.y(), rt.right(), c.y());
}

void drawMarker(QPainter& painter, const QPointF& pt, int nsize, int type)
{
	int n2 = nsize / 2;
	QRect rect(pt.x() - n2, pt.y() - n2, nsize, nsize);
	switch (type)
	{
	case 0: break;
	case 1: painter.drawRect(rect); break;
	case 2: painter.drawEllipse(rect); break;
	case 3: drawDiamond(painter, rect); break;
	case 4: drawTriangle(painter, rect); break;
	case 5: drawCross(painter, rect); break;
	case 6: drawPlus(painter, rect); break;
	}
}

struct ChartPainter::Imp 
{
	QPainter& painter;

	QRect paintRect; // the rectangle that is being painted
	QRect titleRect; // the rectangle that is used to draw the title
	QRect plotRect;  // the rectangle that is used to draw the plots
	QRectF viewRect;  // the rectangle that defines the view of the data

	double xscale = 1, yscale = 1;

	Imp(QPainter& painter) : painter(painter) 
	{
		viewRect = QRectF(0, 0, 1, 1);
	}

	// convert view coordinates to plot coordinates
	QPointF ViewToPlot(const QPointF& p)
	{
		double x = plotRect.left() + (p.x() - viewRect.left()) / viewRect.width() * plotRect.width();
		double y = plotRect.bottom() - (p.y() - viewRect.top()) / viewRect.height() * plotRect.height();
		return QPointF(x, y);
	}

	// convert plot coordinates to view coordinates
	QPointF PlotToView(const QPointF& p)
	{
		double x = viewRect.left() + (p.x() - plotRect.left()) / plotRect.width() * viewRect.width();
		double y = viewRect.top() + (plotRect.bottom() - p.y()) / plotRect.height() * viewRect.height();
		return QPointF(x, y);
	}

	// update the scales based on the view rectangle
	void UpdateScales()
	{
		xscale = calculateScale(viewRect.left(), viewRect.right ());
		yscale = calculateScale(viewRect.top (), viewRect.bottom());
	}
};

ChartPainter::ChartPainter(QPainter& painter) : m(*new ChartPainter::Imp(painter)) {}
ChartPainter::~ChartPainter() { delete& m; }

void ChartPainter::SetViewRect(const QRectF& viewRect)
{
	m.viewRect = viewRect;
	m.UpdateScales();
}

void ChartPainter::DrawChart(const CGraphData& data, const QRect& rt)
{
	m.paintRect = rt;
	m.plotRect = rt;

	QFont f("Times", data.m_titleFontSize, QFont::Bold);
	QFontMetrics fm(f);

	if (data.m_bdrawTitle)
	{
		// calculate title rect
		m.titleRect = m.paintRect;
		m.titleRect.setHeight(fm.height() + 10);

		// draw the title
		DrawTitle(data);

		// update plot rect to adjust for title
		m.plotRect.setTop(m.titleRect.bottom());
	}

	// adjust the screen rectangle where the data will be drawn
	if (data.m_bfullScreenMode == false)
	{
		m.plotRect.adjust(50, 0, -90, -2 * fm.height() - 2);

		double gy = 1;
		if (data.m_bscaleAxisLabels)
		{
			int nydiv = (int)log10(m.yscale);
			if (nydiv != 0)
			{
				gy = pow(10.0, nydiv);
			}
		}

		if ((data.m_yAxis.labelPosition == LOW) && (data.m_yAxis.labelAlignment == ALIGN_LABEL_LEFT))
		{
			QFont f("Arial", data.m_axesFontSize);
			QFontMetrics fm(f);

			int y1 = m.plotRect.bottom();
			int maxWidth = 0;
			char sz[256] = { 0 };
			double fy = m.yscale * floor(m.viewRect.top() / m.yscale);
			while (fy < m.viewRect.bottom())
			{
				int iy = m.ViewToPlot(QPointF(0.0, fy)).y();
				if (iy < y1)
				{
					double g = fy / gy;
					if (fabs(g) < 1e-7) g = 0;
					snprintf(sz, sizeof sz, "%lg", g);
					QString s(sz);

					int w = fm.horizontalAdvance(s);
					if (w > maxWidth) maxWidth = w;
				}
				fy += m.yscale;
			}
			maxWidth += 10;
			if (maxWidth > m.plotRect.left()) m.plotRect.setLeft(m.paintRect.left() + maxWidth);
		}

		if (data.m_bshowLegend && !data.m_data.empty())
		{
			int maxWidth = 0;
			int N = (int) data.m_data.size();
			QFont f("Arial", data.m_legendFontSize);
			QFontMetrics fm(f);

			for (int i = 0; i < N; ++i)
			{
				CPlotData& plot = *data.m_data[i];
				int w = fm.horizontalAdvance(plot.label());
				if (w > maxWidth) maxWidth = w;
			}

			maxWidth += 10 + 40; // 40 is the space taken up by the line (=25 + 15 padding)
			double W = m.paintRect.right() - m.plotRect.right();
			if (maxWidth > W) m.plotRect.setRight(m.paintRect.right() - maxWidth);
		}

		m.painter.setPen(QPen(data.m_boxColor));
		m.painter.setBrush(Qt::NoBrush);
		m.painter.drawRect(m.plotRect);
	}

	// draw the grid
	if (data.m_bdrawGrid) DrawGrid(data);

	// draw the grid axes
	DrawAxes(data);

	// draw the axes tick marks
	DrawAxesTicks(data);

	// draw the axes labels
	if (data.m_bdrawAxesLabels) DrawAxesLabels(data);

	// draw the legend
	if (data.m_bshowLegend) DrawLegend(data);

	// render the data
	m.painter.setClipRect(m.plotRect);
	DrawAllData(data);
}

void ChartPainter::DrawTitle(const CGraphData& data)
{
	QFont f("Times", data.m_titleFontSize, QFont::Bold);
	m.painter.setFont(f);
	QPen pen(Qt::black, 1);
	m.painter.setPen(pen);
	m.painter.drawText(m.titleRect, Qt::AlignCenter, data.m_title);
}

void ChartPainter::DrawGrid(const CGraphData& data)
{
	char sz[256] = { 0 };
	QFont f("Arial", 10);
	QFontMetrics fm(f);
	m.painter.setFont(f);

	int x0 = m.plotRect.left();
	int x1 = m.plotRect.right();
	int y0 = m.plotRect.top();
	int y1 = m.plotRect.bottom();

	double xscale = m.xscale;
	double yscale = m.yscale;

	m.painter.setPen(QPen(data.m_gridCol, 1));
	m.painter.setRenderHint(QPainter::Antialiasing, false);

	// draw the y-grid lines
	if (data.m_bdrawYLines)
	{
		double fy = yscale * floor(m.viewRect.top() / yscale);
		while (fy < m.viewRect.bottom())
		{
			int iy = m.ViewToPlot(QPointF(0.0, fy)).y();
			if (iy < y1)
			{
				QPainterPath path;
				path.moveTo(x0, iy);
				path.lineTo(x1 - 1, iy);
				m.painter.drawPath(path);
			}
			fy += yscale;
		}
	}

	// draw the x-grid lines
	if (data.m_bdrawXLines)
	{
		double fx = xscale * floor(m.viewRect.left() / xscale);
		while (fx < m.viewRect.right())
		{
			int ix = m.ViewToPlot(QPointF(fx, 0.0)).x();
			if (ix > x0)
			{
				QPainterPath path;
				path.moveTo(ix, y0);
				path.lineTo(ix, y1 - 1);
				m.painter.drawPath(path);
			}
			fx += xscale;
		}
	}

	m.painter.setRenderHint(QPainter::Antialiasing, true);
}

void ChartPainter::DrawAxes(const CGraphData& data)
{
	// get the center in screen coordinates
	QPointF c = m.ViewToPlot(QPointF(0.0, 0.0));

	// render the X-axis
	if (data.m_xAxis.visible)
	{
		if ((c.y() > m.plotRect.top()) && (c.y() < m.plotRect.bottom()))
		{
			m.painter.setPen(QPen(data.m_xAxisCol, 2));
			QPainterPath xaxis;
			xaxis.moveTo(m.plotRect.left(), c.y());
			xaxis.lineTo(m.plotRect.right(), c.y());
			m.painter.drawPath(xaxis);
		}
	}

	// render the Y-axis
	if (data.m_yAxis.visible)
	{
		if ((c.x() > m.plotRect.left()) && (c.x() < m.plotRect.right()))
		{
			m.painter.setPen(QPen(data.m_yAxisCol, 2));
			QPainterPath yaxis;
			yaxis.moveTo(c.x(), m.plotRect.top());
			yaxis.lineTo(c.x(), m.plotRect.bottom());
			m.painter.drawPath(yaxis);
		}
	}
}

void ChartPainter::DrawAxesTicks(const CGraphData& data)
{
	char sz[256] = { 0 };
	QFont f("Arial", data.m_axesFontSize);
	QFontMetrics fm(f);
	m.painter.setFont(f);

	int x0 = m.plotRect.left();
	int x1 = m.plotRect.right();
	int y0 = m.plotRect.top();
	int y1 = m.plotRect.bottom();

	double xscale = m.xscale;
	double yscale = m.yscale;

	m.painter.setPen(QPen(Qt::black, 1));

	// determine the y-scale
	double gy = 1;
	if (data.m_bscaleAxisLabels)
	{
		int nydiv = (int)log10(yscale);
		if (nydiv != 0)
		{
			gy = pow(10.0, nydiv);
			snprintf(sz, sizeof sz, "x 1e%03d", nydiv);
			m.painter.setPen(QPen(data.m_yAxisCol));
			m.painter.drawText(x0 - 30, y0 - fm.descent() - 5, QString(sz));
		}
	}
	else if (data.m_customYAxisLabel.isEmpty() == false)
	{
		m.painter.setPen(QPen(data.m_yAxisCol));
		m.painter.drawText(x0 - 30, y0 - fm.height() + fm.descent(), data.m_customYAxisLabel);
	}

	// determine the x-scale
	double gx = 1;
	if (data.m_bscaleAxisLabels)
	{
		int nxdiv = (int)log10(xscale);
		if (nxdiv != 0)
		{
			gx = pow(10.0, nxdiv);
			snprintf(sz, sizeof sz, "x 1e%03d", nxdiv);
			m.painter.setPen(QPen(data.m_xAxisCol));
			m.painter.drawText(x1 + 5, y1, QString(sz));
		}
	}
	else if (data.m_customXAxisLabel.isEmpty() == false)
	{
		m.painter.setPen(QPen(data.m_xAxisCol));
		m.painter.drawText(x1 + 5, y1, data.m_customXAxisLabel);
	}

	// draw the y-labels
	if (data.m_yAxis.labelPosition != NONE)
	{
		m.painter.setPen(QPen(data.m_yAxisTickCol));

		int xPos = 0;
		switch (data.m_yAxis.labelPosition)
		{
		case LOW : xPos = x0; break;
		case HIGH: xPos = x1; break;
		case NEXT_TO_AXIS:
			xPos = m.ViewToPlot(QPointF(0., 0.)).x();
			if (xPos < x0) xPos = x0; else if (xPos > x1) xPos = x1;
			break;
		}

		m.painter.setPen(QPen(data.m_yAxisCol));

		double fy = yscale * floor(m.viewRect.top() / yscale);
		while (fy < m.viewRect.bottom())
		{
			int iy = m.ViewToPlot(QPointF(0.0, fy)).y();
			if (iy < y1)
			{
				double g = fy / gy;
				if (fabs(g) < 1e-7) g = 0;
				snprintf(sz, sizeof sz, "%lg", g);
				QString s(sz);

				if (data.m_yAxis.labelAlignment == ALIGN_LABEL_LEFT)
				{
					int w = m.painter.fontMetrics().horizontalAdvance(s);
					m.painter.drawText(xPos - w - 5, iy + m.painter.fontMetrics().height() / 3, s);
				}
				else
					m.painter.drawText(xPos + 5, iy + m.painter.fontMetrics().height() / 3, s);
			}
			fy += yscale;
		}
	}

	// draw the x-labels
	if (data.m_xAxis.labelPosition != NONE)
	{
		m.painter.setPen(QPen(data.m_xAxisTickCol));

		int yPos = 0;
		switch (data.m_xAxis.labelPosition)
		{
		case LOW: yPos = y1; break;
		case HIGH: yPos = y0; break;
		case NEXT_TO_AXIS:
			yPos = m.ViewToPlot(QPointF(0., 0.)).y();
			if (yPos < y0) yPos = y0; else if (yPos > y1) yPos = y1;
			break;
		}

		m.painter.setPen(QPen(data.m_xAxisCol));

		double fx = xscale * floor(m.viewRect.left() / xscale);
		while (fx < m.viewRect.right())
		{
			int ix = m.ViewToPlot(QPointF(fx, 0.0)).x();
			if (ix > x0)
			{
				double g = fx / gx;
				if (fabs(g) < 1e-7) g = 0;
				snprintf(sz, sizeof sz, "%lg", g);
				QString s(sz);
				int w = m.painter.fontMetrics().horizontalAdvance(s);
				if (data.m_xAxis.labelAlignment == ALIGN_LABEL_BOTTOM)
					m.painter.drawText(ix - w / 2, yPos + m.painter.fontMetrics().height(), s);
				else
					m.painter.drawText(ix - w / 2, yPos - 5, s);
			}
			fx += xscale;
		}
	}
}

void ChartPainter::DrawAxesLabels(const CGraphData& data)
{
	QFont f("Arial", data.m_axesFontSize);
	f.setBold(true);
	m.painter.setFont(f);

	if (data.m_xAxis.labelVisible && (data.m_xAxis.label.isEmpty() == false))
	{
		m.painter.drawText(m.paintRect, Qt::AlignHCenter | Qt::AlignBottom, data.m_xAxis.label);
	}

	if (data.m_yAxis.labelVisible && (data.m_yAxis.label.isEmpty() == false))
	{
		QRect rt(-m.paintRect.height(), 0, m.paintRect.height(), m.paintRect.width());
		m.painter.save();
		m.painter.rotate(-90);
		m.painter.drawText(rt, Qt::AlignHCenter | Qt::AlignTop, data.m_yAxis.label);
		m.painter.restore();
	}
}

void ChartPainter::DrawLegend(const CGraphData& data)
{
	int N = (int)data.m_data.size();
	if (N == 0) return;

	QRect legendRect = m.plotRect;
	legendRect.setLeft(m.plotRect.right());
	legendRect.setRight(m.paintRect.right());

	QFont f("Arial", data.m_legendFontSize);
	QFontMetrics fm(f);
	m.painter.setFont(f);

	int fh = fm.height();
	int fa = fm.ascent();

	int X0 = legendRect.left() + 10;
	int LW = 25;
	int X1 = X0 + LW + 5;
	int YC = legendRect.center().y();
	int H = legendRect.height() - 10;
	int Y0 = YC - N / 2 * (fh + 2);
	int Y1 = Y0 + N * (fh + 2);

	// draw the lines
	for (int i = 0; i < N; ++i)
	{
		CPlotData& plot = *data.m_data[i];
		m.painter.setPen(QPen(plot.lineColor(), 2));
		int Y = Y0 + i * (Y1 - Y0) / N;
		m.painter.drawLine(X0, Y, X0 + LW, Y);
	}

	// draw the text
	m.painter.setPen(Qt::black);
	for (int i = 0; i < N; ++i)
	{
		CPlotData& plot = *data.m_data[i];
		int Y = Y0 + i * (Y1 - Y0) / N;
		m.painter.drawText(X1, Y + fa / 3, plot.label());
	}
}

void ChartPainter::DrawAllData(const CGraphData& data)
{
	m.painter.setRenderHint(QPainter::Antialiasing, data.m_bsmoothLines);

	int N = (int)data.m_data.size();
	for (int i = 0; i < N; ++i)
	{
		switch (data.m_chartStyle)
		{
		case LINECHART_PLOT: DrawLineChart(*data.m_data[i]); break;
		case BARCHART_PLOT : DrawBarChart (*data.m_data[i]); break;
		case PIECHART_PLOT : DrawPieChart (*data.m_data[i]); break;
		}
	}

	m.painter.setRenderHint(QPainter::Antialiasing, true);
}

void ChartPainter::DrawLineChart(CPlotData& data)
{
	int N = data.size();
	if (N == 0) return;

	QColor col = data.lineColor();
	QPen pen(col, data.lineWidth());
	m.painter.setPen(pen);

	QPointF p0 = m.ViewToPlot(data.Point(0)), p1(p0);
	m.painter.setBrush(Qt::NoBrush);
	for (int i = 1; i < N; ++i)
	{
		p1 = m.ViewToPlot(data.Point(i));
		m.painter.drawLine(p0, p1);
		p0 = p1;
	}

	int n = data.markerSize();
	int n2 = n / 2 + 1;

	// draw the marks
	if (data.markerType() > 0)
	{
		m.painter.setBrush(data.fillColor());
		for (int i = 0; i < N; ++i)
		{
			p1 = m.ViewToPlot(data.Point(i));
			drawMarker(m.painter, p1, data.markerSize(), data.markerType());
		}
	}
}

void ChartPainter::DrawBarChart(CPlotData& data)
{
	int N = data.size();
	if (N == 0) return;

	int W = m.plotRect.width();
	int w = W / N;
	if (w < 1) w = 1;

	m.painter.setPen(Qt::NoPen);
	m.painter.setBrush(data.fillColor());
	for (int i = 0; i < N; ++i)
	{
		QPointF& pi = data.Point(i);
		QPointF p0 = m.ViewToPlot(pi);
		QPointF p1 = m.ViewToPlot(QPointF(pi.x(), 0.0));
		QRect r(p0.x() - w / 2, p0.y(), w, p1.y() - p0.y());
		m.painter.drawRect(r);
	}
}

void ChartPainter::DrawPieChart(CPlotData& data)
{
	m.painter.setRenderHint(QPainter::RenderHint::Antialiasing);
	m.painter.setPen(Qt::NoPen);
	m.painter.setBrush(data.fillColor());

	// make sure the rectangle is square
	QRect r = m.plotRect;
	int W = r.width();
	int H = r.height();
	if (W > H)
	{
		r.setLeft(r.left() + (W - H) / 2);
		r.setWidth(H);
	}
	else if (H > W)
	{
		r.setTop(r.top() + (H - W) / 2);
		r.setHeight(W);
	}
	H = W = r.width();

	// get the center point
	QPointF c = r.center();

	// find sum of all values
	double vsum = 0;
	for (int i = 0; i < data.size(); ++i)
	{
		double v = data.Point(i).y();
		vsum += v;
	}
	if (vsum <= 0) vsum = 1;

	double start = 0;
	for (int i = 0; i < data.size(); ++i)
	{
		CPlotData::DataPoint pt = data.Data(i);
		QPointF& pi = pt.pos;
		double v = pi.y();
		if (v < 0) v = 0;

		double span = v / vsum;

		int n0 = (int)(5760.0 * start);
		int n1 = (int)(5760.0 * span);

		if (pt.color.isValid())
			m.painter.setBrush(pt.color);

		m.painter.drawPie(r, n0, n1);

		if (!pt.label.isEmpty() && (span > 2.0 / 360.0))
		{
			double a = 2 * PI * (start + span * 0.5);
			double ca = cos(a);
			double sa = sin(a);
			double x = c.x() + W * 0.35 * ca;
			double y = c.y() - H * 0.35 * sa;
			QFont f = m.painter.font();
			f.setPixelSize(10);
			m.painter.setFont(f);
			m.painter.setPen(QPen(Qt::black));
			m.painter.drawText(x, y, pt.label);
		}

		start += span;
	}
}
