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
#include "CommandManager.h"
#include <CUILib/PlotWidget.h>

class LoadCurve;

class UICurveEditWidget;

class CCurveEditWidget : public QWidget
{
	Q_OBJECT

public:
	CCurveEditWidget(QWidget* parent = nullptr);

	void Clear();

	void SetLoadCurve(LoadCurve* lc);

	void SetXRange(double xmin, double xmax);

public slots:
	void on_plot_pointClicked(QPointF p, bool shift);
	void on_plot_draggingStart(QPoint p);
	void on_plot_pointDragged(QPoint p);
	void on_plot_draggingEnd(QPoint p);
	void on_plot_pointSelected(int n);
	void on_plot_backgroundImageChanged();
	void on_plot_doneZoomToRect();
	void on_plot_regionSelected(QRect);
	void on_xval_textEdited();
	void on_yval_textEdited();
	void on_deletePoint_clicked();
	void on_zoomToFit_clicked();
	void on_zoomX_clicked();
	void on_zoomY_clicked();
	void on_map_clicked();
	void on_lineType_currentIndexChanged(int n);
	void on_extendMode_currentIndexChanged(int n);
	void on_undo_clicked(bool b);
	void on_redo_clicked(bool b);
	void on_math_clicked(bool b);
	void on_copy_clicked(bool b);
	void on_paste_clicked(bool b);
	void on_open_clicked(bool b);
	void on_save_clicked(bool b);
	void on_clear_clicked();
	void on_showDeriv_toggled(bool b);

signals:
	void dataChanged();

private:
	void UpdateSelection();
	void UpdatePlotData();

private:
	UICurveEditWidget* ui;

	// undo stack
	CBasicCmdManager	m_cmd;
};

class CCurvePlotWidget : public CPlotWidget
{
	Q_OBJECT

public:
	CCurvePlotWidget(QWidget* parent = nullptr);

	void DrawPlotData(QPainter& p, CPlotData& data) override;

	void SetLoadCurve(LoadCurve* lc);
	LoadCurve* GetLoadCurve();

	void ShowDeriv(bool b);

private:
	LoadCurve* m_lc;
	bool	m_showDeriv;
};
