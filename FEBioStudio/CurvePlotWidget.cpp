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
#include "CurvePlotWidget.h"
#include <FECore/PointCurve.h>
#include <QPainter>

CCurvePlotWidget::CCurvePlotWidget(QWidget* parent) : CPlotWidget(parent)
{
	m_lc = nullptr;
	m_showDeriv = false;
	setLineSmoothing(true);

	showLegend(false);
	showToolTip(false);
	scaleAxisLabels(false);
	setFullScreenMode(true);
	setXAxisLabelAlignment(ALIGN_LABEL_TOP);
	setYAxisLabelAlignment(ALIGN_LABEL_RIGHT);
	setBackgroundColor(QColor(48, 48, 48));
	setGridColor(QColor(128, 128, 128));
	setXAxisColor(QColor(255, 255, 255));
	setYAxisColor(QColor(255, 255, 255));
	setXAxisTickColor(QColor(255, 255, 255));
	setYAxisTickColor(QColor(255, 255, 255));
	setSelectionColor(QColor(255, 255, 192));
}

void CCurvePlotWidget::SetPointCurve(PointCurve* lc)
{
	m_lc = lc;

	clear();

	if (lc)
	{
		CPlotData* data = new CPlotData;
		for (int i = 0; i < lc->Points(); ++i)
		{
			vec2d pt = lc->Point(i);
			data->addPoint(pt.x(), pt.y());
		}
		addPlotData(data);

		data->setLineColor(QColor(92, 255, 164));
		data->setFillColor(QColor(92, 255, 164));
		data->setLineWidth(2);
		data->setMarkerSize(5);
		repaint();
	}
}

void CCurvePlotWidget::ShowDeriv(bool b)
{
	m_showDeriv = b;
	update();
}

PointCurve* CCurvePlotWidget::GetPointCurve()
{
	return m_lc;
}

void CCurvePlotWidget::DrawPlotData(QPainter& painter, CPlotData& data)
{
	if (m_lc == 0) return;
	m_lc->Update();

	int N = data.size();
	QRect rt = ScreenRect();

	// draw derivative
	if (m_showDeriv)
	{
		QColor c = data.lineColor().darker();
		painter.setPen(QPen(c, data.lineWidth()));
		QPointF p0, p1;
		for (int i = rt.left(); i < rt.right(); i += 2)
		{
			p1.setX(i);
			QPointF p = ScreenToView(p1);
			p.setY(m_lc->derive(p.x()));
			p1 = ViewToScreen(p);

			if (i != rt.left())
			{
				painter.drawLine(p0, p1);
			}
			p0 = p1;
		}
	}

	// draw the line
	painter.setPen(QPen(data.lineColor(), data.lineWidth()));
	QPointF p0, p1;
	for (int i = rt.left(); i < rt.right(); i += 2)
	{
		p1.setX(i);
		QPointF p = ScreenToView(p1);
		p.setY(m_lc->value(p.x()));
		p1 = ViewToScreen(p);

		if (i != rt.left())
		{
			painter.drawLine(p0, p1);
		}

		p0 = p1;
	}

	// draw the marks
	if (data.markerType() > 0)
	{
		painter.setBrush(data.fillColor());
		for (int i = 0; i < N; ++i)
		{
			p1 = ViewToScreen(data.Point(i));
			QRect r(p1.x() - 2, p1.y() - 2, 5, 5);
			painter.drawRect(r);
		}
	}
}
