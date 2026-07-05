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
#pragma once
#include <QPixmap>
#include <QPainter>
#include <CUILib/GraphData.h>

class ChartBuilder
{
public:
	ChartBuilder(size_t w, size_t h);
	virtual ~ChartBuilder() = default;

	void SetTitle(const QString& title);

	void SetXAxisLabel(const QString& label);

	QPixmap GetPixmap() { Build(); return pix; }

protected:
	virtual void Build() = 0;

protected:
	QPixmap pix;
	CGraphData data;
};

class PieChartBuilder : public ChartBuilder
{
public:
	PieChartBuilder(size_t w, size_t h);

	void AddSlice(double span, const QColor& col, const QString& label = "");

private:
	void Build() override;
};

class BarChartBuilder : public ChartBuilder
{
public:
	BarChartBuilder(size_t w, size_t h);

	void AddBar(double val, QColor c);

private:
	void Build() override;
};

class LineChartBuilder : public ChartBuilder
{
public:
	LineChartBuilder(size_t w, size_t h);

	void AddLine(const std::vector<QPointF>& points, const QString& label = "", const QPen& pen = QPen(Qt::black));

private:
	void Build() override;
};
