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
#include <QWidget>

class CRangeSlider : public QWidget
{
	Q_OBJECT

public:
	CRangeSlider(QWidget* parent = nullptr);

	void setLeftPosition(double newVal);
	void setRightPosition(double newVal);
	void setPositions(double leftVal, double rightVal);
	void setRange(double minVal, double maxVal);

	void setSelectionColor(QColor c);

	double leftPosition() const;
	double rightPosition() const;

	QSize sizeHint() const override;
	QSize minimumSizeHint() const override;

signals:
	void positionChanged();

protected:
	void paintEvent(QPaintEvent* event) override;
	void mousePressEvent(QMouseEvent* ev) override;
	void mouseReleaseEvent(QMouseEvent* ev) override;
	void mouseMoveEvent(QMouseEvent* ev) override;

private:
	QRect leftHandleRect();
	QRect rightHandleRect();
	QRect grooveRect();
	QRect selectionRect();

private:
	double	m_min, m_max;
	double	m_val1, m_val2;
	int		m_sel;
	QPoint	m_p0;
	QColor	m_selColor;
};
