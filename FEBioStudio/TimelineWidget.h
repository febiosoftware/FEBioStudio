/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in
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
#include <QWidget>

//-----------------------------------------------------------------------------
// Widget that shows the time points, current time point, and current time
class CTimelineWidget : public QWidget
{
	Q_OBJECT

public:
	CTimelineWidget(QWidget* parent = 0);

	void clearData();

	// set the time points
	void setTimePoints(const std::vector<double>& time);

	void UpdateScale();

	void setSelection(int i);
	void setCurrentTime(float ftime);
	void setRange(int nmin, int nmax);

protected:
	void paintEvent(QPaintEvent* ev);
	void resizeEvent(QResizeEvent* ev);
	void mousePressEvent(QMouseEvent* ev);
	void mouseMoveEvent(QMouseEvent* ev);
	void mouseReleaseEvent(QMouseEvent* ev);

signals:
	void pointClicked(int n);
	void rangeChanged(int nmin, int nmax);

private:
	std::vector<double>	m_data;
	double	m_dataMin, m_dataMax;
	double	m_min, m_max, m_inc;
	double	m_ftime;
	int	    m_first, m_last;
	QRect	m_dataRect, m_timeRect;
	QRect	m_leftBox, m_rightBox;
	int		m_nselect;
	int		m_ndrag;
};

