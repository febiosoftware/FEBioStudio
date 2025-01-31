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
#include "GraphData.h"

CGraphData::CGraphData()
{
	m_bshowLegend = true;
	m_bdrawXLines = true;
	m_bdrawYLines = true;
	m_bsmoothLines = true;
	m_bdrawGrid = true;
	m_bdrawTitle = true;
	m_bdrawAxesLabels = false;

	m_titleFontSize = 12;
	m_legendFontSize = 10;
	m_axesFontSize = 10;

	m_bgCol = QColor(255, 255, 255);
	m_gridCol = QColor(192, 192, 192);
	m_xAxisCol = QColor(0, 0, 0);
	m_yAxisCol = QColor(0, 0, 0);
	m_xAxisTickCol = QColor(0, 0, 0);
	m_yAxisTickCol = QColor(0, 0, 0);
	m_boxColor = QColor(0, 0, 0);

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
	m_bdrawAxesLabels = data.m_bdrawAxesLabels;
	m_bgCol = data.m_bgCol;
	m_gridCol = data.m_gridCol;
	m_xAxisCol = data.m_xAxisCol;
	m_yAxisCol = data.m_yAxisCol;
	m_xAxisTickCol = data.m_xAxisTickCol;
	m_yAxisTickCol = data.m_yAxisTickCol;
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
	m_bdrawAxesLabels = data.m_bdrawAxesLabels;
	m_bgCol = data.m_bgCol;
	m_gridCol = data.m_gridCol;
	m_xAxisCol = data.m_xAxisCol;
	m_yAxisCol = data.m_yAxisCol;
	m_xAxisTickCol = data.m_xAxisTickCol;
	m_yAxisTickCol = data.m_yAxisTickCol;
	m_xAxis = data.m_xAxis;
	m_yAxis = data.m_yAxis;

	for (int i = 0; i < data.m_data.size(); ++i)
	{
		CPlotData* di = new CPlotData(*data.m_data[i]);
		m_data.push_back(di);
	}
}

void CGraphData::ClearData()
{
	for (int i = 0; i < m_data.size(); ++i) delete m_data[i];
	m_data.clear();
}

void CGraphData::AddPlotData(CPlotData* plot)
{
	m_data.push_back(plot);
}
