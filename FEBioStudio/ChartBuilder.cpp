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
#include "ChartBuilder.h"
#include <CUILib/ChartPainter.h>

#ifndef PI
#define PI 3.141592653589793
#endif


//==========================================================================================
ChartBuilder::ChartBuilder(size_t w, size_t h) : pix((int)w, (int)h)
{

}

void ChartBuilder::SetTitle(const QString& title)
{
	data.m_title = title;
}

void ChartBuilder::SetXAxisLabel(const QString& label)
{ 
	data.m_xAxis.label = label; 
}

//==========================================================================================
PieChartBuilder::PieChartBuilder(size_t w, size_t h) : ChartBuilder(w, h)
{
	pix.fill(QColor::fromRgb(0, 0, 0, 0));
	data.m_chartStyle = PIECHART_PLOT;
	data.m_bdrawAxesLabels = false;
	data.m_bdrawGrid = false;
	data.m_bdrawXLines = false;
	data.m_bdrawYLines = false;
	data.m_bshowLegend = false;
	data.m_bfullScreenMode = true;
	data.AddPlotData(new CPlotData);
}

void PieChartBuilder::AddSlice(double span, const QColor& col, const QString& label)
{
	CPlotData& plot = *data.GetPlotData(0);
	plot.addPoint(QPointF(plot.size(), span), col, label);
}

void PieChartBuilder::Build()
{
	QPainter p(&pix);
	ChartPainter painter(p);
	painter.DrawChart(data, pix.rect());
}

//====================================================================
BarChartBuilder::BarChartBuilder(size_t w, size_t h) : ChartBuilder(w, h)
{
	pix.fill(Qt::white);
	data.m_chartStyle = BARCHART_PLOT;
	data.m_bshowLegend = false;
	data.AddPlotData(new CPlotData);
}

void BarChartBuilder::AddBar(double val, QColor c)
{
	CPlotData* plot = data.GetPlotData(0);
	plot->setFillColor(c);
	plot->addPoint((double)plot->size() + 1, val);
}

void BarChartBuilder::Build()
{
	// figure out the range of the data and the margins
	CPlotData* plot = data.GetPlotData(0);
	QRectF rt = plot->boundRect();

	// make sure the (0,0) point is included in the y-range
	if (rt.top() > 0) rt.setTop(0);
	if (rt.bottom() < 0) rt.setBottom(0);

	QPainter p(&pix);
	ChartPainter painter(p);
	painter.SetViewRect(rt);

	painter.DrawChart(data, pix.rect());
}

//====================================================================
LineChartBuilder::LineChartBuilder(size_t w, size_t h) : ChartBuilder(w, h)
{
	pix.fill(Qt::white);
	data.m_chartStyle = LINECHART_PLOT;
}

void LineChartBuilder::AddLine(const std::vector<QPointF>& points, const QString& label, const QPen& pen)
{
	CPlotData* plot = new CPlotData;
	for (const auto& pt : points) plot->addPoint(pt.x(), pt.y());
	plot->setLabel(label);
	plot->setLineColor(pen.color());
	plot->setLineWidth(pen.width());
	data.AddPlotData(plot);
}

void LineChartBuilder::Build()
{
	// figure out the range of the data and the margins
	QRectF rt;
	for (size_t i = 0; i < data.m_data.size(); ++i)
	{
		CPlotData* plot = data.GetPlotData((int)i);
		QRectF r = plot->boundRect();
		if (i == 0) rt = r;
		else rt = rt.united(r);
	}
	// inflate a bit to add some margins
	rt.adjust(-0.05 * rt.width(), -0.05 * rt.height(), 0.05 * rt.width(), 0.05 * rt.height());

	QPainter p(&pix);
	ChartPainter painter(p);
	painter.SetViewRect(rt);
	painter.DrawChart(data, pix.rect());
}
