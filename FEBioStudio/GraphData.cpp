#include "stdafx.h"
#include "GraphData.h"

CGraphData::CGraphData()
{
	m_bshowLegend = true;
	m_bdrawXLines = true;
	m_bdrawYLines = true;
	m_bsmoothLines = true;
	m_bdrawGrid = true;
	m_bdrawTitle = true;

	m_bgCol = QColor(255, 255, 255);
	m_gridCol = QColor(192, 192, 192);
	m_xCol = QColor(0, 0, 0);
	m_yCol = QColor(0, 0, 0);

	// X-axis format
	m_xAxis.visible = true;
	m_xAxis.labelPosition = LOW;
	m_xAxis.labelAlignment = ALIGN_LABEL_BOTTOM;

	// Y-axis format
	m_yAxis.visible = true;
	m_yAxis.labelPosition = LOW;
	m_yAxis.labelAlignment = ALIGN_LABEL_LEFT;
}

CGraphData::~CGraphData()
{
	ClearData();
}

//-----------------------------------------------------------------------------
CGraphData::CGraphData(const CGraphData& data)
{
	SetName(data.GetName());

	m_title = data.m_title;
	m_bshowLegend = data.m_bshowLegend;
	m_bdrawXLines = data.m_bdrawXLines;
	m_bdrawYLines = data.m_bdrawYLines;
	m_bsmoothLines = data.m_bsmoothLines;
	m_bdrawGrid = data.m_bdrawGrid;
	m_bdrawTitle = data.m_bdrawTitle;
	m_bgCol = data.m_bgCol;
	m_gridCol = data.m_gridCol;
	m_xCol = data.m_xCol;
	m_yCol = data.m_yCol;
	m_xAxis = data.m_xAxis;
	m_yAxis = data.m_yAxis;

	for (int i = 0; i < data.m_data.size(); ++i)
	{
		CPlotData* di = new CPlotData(*data.m_data[i]);
		m_data.push_back(di);
	}
}

//-----------------------------------------------------------------------------
void CGraphData::operator = (const CGraphData& data)
{
	SetName(data.GetName());

	ClearData();

	m_title = data.m_title;
	m_bshowLegend = data.m_bshowLegend;
	m_bdrawXLines = data.m_bdrawXLines;
	m_bdrawYLines = data.m_bdrawYLines;
	m_bsmoothLines = data.m_bsmoothLines;
	m_bdrawGrid = data.m_bdrawGrid;
	m_bdrawTitle = data.m_bdrawTitle;
	m_bgCol = data.m_bgCol;
	m_gridCol = data.m_gridCol;
	m_xCol = data.m_xCol;
	m_yCol = data.m_yCol;
	m_xAxis = data.m_xAxis;
	m_yAxis = data.m_yAxis;

	for (int i = 0; i < data.m_data.size(); ++i)
	{
		CPlotData* di = new CPlotData(*data.m_data[i]);
		m_data.push_back(di);
	}
}

//-----------------------------------------------------------------------------
void CGraphData::ClearData()
{
	for (int i = 0; i < m_data.size(); ++i) delete m_data[i];
	m_data.clear();
}
