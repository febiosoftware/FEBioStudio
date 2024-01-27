/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2024 University of Utah, The Trustees of Columbia University in
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
#include <QDialog>
#include <FECore/matrix.h>

class FEGlobalMatrix;

class CDlgMatrixInspector : public QDialog
{
	Q_OBJECT

	class Ui;

public:
	CDlgMatrixInspector(QWidget* parent = 0);

	void SetGlobalMatrix(FEGlobalMatrix* M);

public slots:
	void onViewScroll();
	void updateView(int x, int y);

private:
	Ui* ui;
};

class MatrixDensityView : public QWidget
{
	Q_OBJECT

public:
	MatrixDensityView(CDlgMatrixInspector* dlg, QWidget* parent = nullptr);

	void paintEvent(QPaintEvent* paintEvent) override;

	void drawBackground(QPainter& painter);

	void drawMatrixProfile(QPainter& painter);

	void drawSelection(QPainter& painter);

	QSize sizeHint() const override;

	void resizeEvent(QResizeEvent* ev) override;

	void SetGlobalMatrix(FEGlobalMatrix* K);

	void SetSelection(QRect sel);

	void mousePressEvent(QMouseEvent* ev) override;

	void mouseMoveEvent(QMouseEvent* ev) override;

	void updateView(size_t x, size_t y);

private:
	void Update();

public slots:
	void SetColorMode(int option);

private:
	CDlgMatrixInspector* m_dlg = nullptr;
	FEGlobalMatrix* m_K = nullptr;
	matrix m;
	QRect	m_sel;
	double m_maxM = 0;
	double m_minM = 0;
	int m_colorMode = 0;
};
