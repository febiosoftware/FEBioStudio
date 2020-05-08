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

	QColor	m_bgCol;
	QColor	m_gridCol;
	QColor	m_xCol;
	QColor	m_yCol;

	CPlotAxis			m_xAxis;
	CPlotAxis			m_yAxis;
};

