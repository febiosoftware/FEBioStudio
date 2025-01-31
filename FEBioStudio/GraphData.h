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

#pragma once
#include <QPoint>
#include <QRect>
#include <QColor>
#include <vector>
#include <FSCore/FSObject.h>

enum AxisLabelPosition
{
	NEXT_TO_AXIS,
	HIGH,
	LOW,
	NONE
};

enum AxisLabelAlignment
{
	ALIGN_LABEL_LEFT,
	ALIGN_LABEL_RIGHT,
	ALIGN_LABEL_TOP,
	ALIGN_LABEL_BOTTOM
};

enum MarkerType
{
	NO_MARKER,
	SQUARE_MARKER,
	CIRCLE_MARKER,
	DIAMOND_MARKER,
	TRIANGLE_MARKER,
	CROSS_MARKER,
	PLUS_MARKER
};

//-----------------------------------------------------------------------------
// Manages a set of (x,y) value pairs
// Derived classes must implement drawing function.
class CPlotData
{
public:
	CPlotData();
	CPlotData(const CPlotData& d);
	CPlotData& operator = (const CPlotData& d);

	virtual ~CPlotData();

	//! clear data
	void clear();

	// add a point to the data
	void addPoint(double x, double y);

	// number of points
	int size() const { return (int)m_data.size(); }

	// get a data point
	QPointF& Point(int i) { return m_data[i]; }
	QPointF Point(int i) const { return m_data[i]; }

	// get the bounding rectangle
	QRectF boundRect() const;

	// set/get the label
	const QString& label() const { return m_label; }
	void setLabel(const QString& label) { m_label = label; }

	// sort the data
	void sort();

public:
	QColor lineColor() const { return m_lineColor; }
	void setLineColor(const QColor& col) { m_lineColor = col; }

	QColor fillColor() const { return m_fillColor; }
	void setFillColor(const QColor& col) { m_fillColor = col; }

	int lineWidth() const { return m_lineWidth; }
	void setLineWidth(int n) { m_lineWidth = (n > 1 ? n : 1); }

	void setMarkerSize(int n) { m_markerSize = n; }
	int markerSize() const { return m_markerSize; }

	void setMarkerType(int n) { m_markerType = n; }
	int markerType() const { return m_markerType; }

protected:
	std::vector<QPointF>	m_data;
	QString			m_label;

protected:
	QColor	m_lineColor;
	int		m_lineWidth;

	QColor	m_fillColor;

	int		m_markerType;
	int		m_markerSize;
};

//-----------------------------------------------------------------------------
class CPlotAxis
{
public:
	bool	visible;
	bool	labelVisible;
	int		labelPosition;
	int		labelAlignment;
	QString	label;

	CPlotAxis() 
	{
		visible = true; 
		labelVisible = true;
		labelPosition = LOW; 
		labelAlignment = ALIGN_LABEL_LEFT;
	}

	CPlotAxis(const CPlotAxis& a)
	{
		visible = a.visible;
		labelVisible = a.labelVisible;
		labelPosition = a.labelPosition;
		labelAlignment = a.labelAlignment;
		label = a.label;
	}

	void operator = (const CPlotAxis& a)
	{
		visible = a.visible;
		labelVisible = a.labelVisible;
		labelPosition = a.labelPosition;
		labelAlignment = a.labelAlignment;
		label = a.label;
	}
};

//-----------------------------------------------------------------------------
// class that collects all the graph data
class CGraphData : public FSObject
{
public:
	CGraphData();
	~CGraphData();

	CGraphData(const CGraphData& data);
	void operator = (const CGraphData& data);

	void ClearData();

	void AddPlotData(CPlotData* plot);

public:
	QString				m_title;
	std::vector<CPlotData*>	m_data;
	bool				m_bshowLegend;
	bool				m_bdrawXLines;
	bool				m_bdrawYLines;
	bool				m_bsmoothLines;
	bool				m_bdrawGrid;
	bool				m_bdrawTitle;
	bool				m_bdrawAxesLabels;

	int		m_titleFontSize;
	int		m_legendFontSize;
	int		m_axesFontSize;

	QColor	m_bgCol;
	QColor	m_gridCol;
	QColor	m_xAxisCol;
	QColor	m_yAxisCol;
	QColor	m_xAxisTickCol;
	QColor	m_yAxisTickCol;
	QColor	m_boxColor;

	CPlotAxis			m_xAxis;
	CPlotAxis			m_yAxis;
};

