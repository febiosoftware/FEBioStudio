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
#include <vector>
#include <QDialog>
#include "GraphData.h"
#include "CommandManager.h"
#include <FECore/MathObject.h>

//-----------------------------------------------------------------------------
class QPainter;
class QAction;
class CPlotWidget;
class QImage;
class LoadCurve;

//-----------------------------------------------------------------------------
enum ChartStyle
{
	LINECHART_PLOT,
	BARCHART_PLOT,
	DENSITY_PLOT
};

//-----------------------------------------------------------------------------
//! This class implements a plotting widget. 
class CPlotWidget : public QWidget
{
	Q_OBJECT

public:
	struct Selection
	{
		int			ndataIndex;
		int			npointIndex;
	};

public:
	//! constructor
	CPlotWidget(QWidget* parent = 0, int w = 0, int h = 0);

	//! Set the plot title
	void setTitle(const QString& s);
	QString title() const { return m_data.m_title; }

	// size hint
	QSize sizeHint() const { return m_sizeHint; }
	QSize minimumSizeHint() const { return QSize(200, 200); }

	// clear plot data
	// This just clears the data for each plot
	// but does not delete the plots
	void clearData();

	// get the graph data
	const CGraphData& GetGraphData() const;

	// set the data
	void SetGraphData(const CGraphData& data);

	// clears everything
	void clear();

	void Resize(int n);

	// get/set show legend
	bool showLegend() const { return m_data.m_bshowLegend; }
	void showLegend(bool b) { m_data.m_bshowLegend = b; }

	// change the view so that it fits the data
	void fitWidthToData();
	void fitHeightToData();
	void fitToData(bool downSize = true);
	void fitToRect(const QRect& rt);

	void setViewRect(const QRectF& rt);

	// add a data field
	void addPlotData(CPlotData* p);

	// get a data field
	int plots() { return (int)m_data.m_data.size(); }
	CPlotData& getPlotData(int i) { return *m_data.m_data[i]; }

	// is view locked
	bool isViewLocked() const { return m_bviewLocked; }
	void setViewLocked(bool b) { m_bviewLocked = b; }

	// popup menu
	void showPopup(bool b) { m_bshowPopup = b; }

	// tool tip (showing selected data point)
	void showToolTip(bool b) { m_bshowToolTip = b; }

	// save data to file
	bool Save(const QString& fileName);

	// set the chart style
	void setChartStyle(int chartStyle);

	void showHorizontalGridLines(bool b) { m_data.m_bdrawYLines = b; }
	void showVerticalGridLines(bool b) { m_data.m_bdrawXLines = b; }

	void showXAxis(bool b) { m_data.m_xAxis.visible = b; }
	void ShowYAxis(bool b) { m_data.m_yAxis.visible = b; }

	bool lineSmoothing() const { return m_data.m_bsmoothLines; }
	void setLineSmoothing(bool b) { m_data.m_bsmoothLines = b; }

	bool drawGrid() const { return m_data.m_bdrawGrid; }
	void setDrawGrid(bool b) { m_data.m_bdrawGrid = b; }

	bool drawTitle() const { return m_data.m_bdrawTitle; }
	void setDrawTitle(bool b) { m_data.m_bdrawTitle = b; }

	bool drawAxesLabels() const { return m_data.m_bdrawAxesLabels; }
	void setDrawAxesLabels(bool b) { m_data.m_bdrawAxesLabels = b; }

	void setBoxColor(const QColor& c) { m_data.m_boxColor = c; }

	void scaleAxisLabels(bool b) { m_bscaleAxisLabels = b; }

	bool autoRangeUpdate() const { return m_bautoRngUpdate; }
	void setAutoRangeUpdate(bool b) { m_bautoRngUpdate = b; }

	QPointF SnapToGrid(const QPointF& p);

	QPointF dataPoint(int ndata, int npoint);

	void setFullScreenMode(bool b) { m_bfullScreenMode = b; }

	void setXAxisLabelAlignment(AxisLabelAlignment a);
	void setYAxisLabelAlignment(AxisLabelAlignment a);

	QColor backgroundColor() { return m_data.m_bgCol; }

	void setBackgroundColor(const QColor& c) { m_data.m_bgCol = c; }
	void setGridColor(const QColor& c) { m_data.m_gridCol = c; }
	void setXAxisColor(const QColor& c) { m_data.m_xAxisCol = c; }
	void setYAxisColor(const QColor& c) { m_data.m_yAxisCol = c; }
	void setXAxisTickColor(const QColor& c) { m_data.m_xAxisTickCol = c; }
	void setYAxisTickColor(const QColor& c) { m_data.m_yAxisTickCol = c; }

	QColor selectionColor() { return m_selCol; }
	void setSelectionColor(const QColor& c) { m_selCol = c; }

	QString XAxisLabel() { return m_data.m_xAxis.label; }
	QString YAxisLabel() { return m_data.m_yAxis.label; }
	void setXAxisLabel(const QString& label) { m_data.m_xAxis.label = label; }
	void setYAxisLabel(const QString& label) { m_data.m_yAxis.label = label; }

	void setCustomXAxisLabel(QString s) { m_customXAxisLabel = s; }
	void setCustomYAxisLabel(QString s) { m_customYAxisLabel = s; }

	void selectPoint(int ndata, int npoint);

	QRect ScreenRect() const { return m_screenRect; }

	bool HasBackgroundImage() const;

	void mapToUserRect(QRect rt, QRectF rng);

	std::vector<Selection> selection() const { return m_selection; }

	std::vector<QPointF> SelectedPoints() const;

	void clearSelection();

	bool LoadBackgroundImage(const QString& fileName);

	void regionSelect(QRect rt);

public:
	void SetHighlightInterval(double rngMin, double rngMax);

	void GetHighlightInterval(double& rngMin, double& rngMax);

signals:
	void regionSelected(QRect rt);
	void pointClicked(QPointF p, bool bshift);
	void pointSelected(int n);
	void pointDragged(QPoint p);
	void draggingStart(QPoint p);
	void draggingEnd(QPoint p);
	void backgroundImageChanged();

protected:
	void mousePressEvent  (QMouseEvent* ev);
	void mouseMoveEvent   (QMouseEvent* ev);
	void mouseReleaseEvent(QMouseEvent* ev);
	void contextMenuEvent (QContextMenuEvent* ev);
	void wheelEvent       (QWheelEvent* ev);
	void dropEvent        (QDropEvent* e);
	void dragEnterEvent   (QDragEnterEvent* e);

	// returns false if point is already selected
	bool addToSelection(int ndata, int npoint);

	// see if a point is selected
	bool isSelected(int ndata, int npoint);

public:
	QPointF ScreenToView(const QPointF& p);
	QRectF ScreenToView(const QRect& rt);
	QPointF ViewToScreen(const QPointF& p);

	double ViewToScreenX(double x) const;
	double ViewToScreenY(double x) const;

	void SetBackgroundImage(QImage* img);

protected:
	//! render the plot
	void paintEvent(QPaintEvent* pe);

	virtual void DrawPlotData(QPainter& p, CPlotData& data);

	void draw_linechart(QPainter& p, CPlotData& data);
	void draw_barchart(QPainter& p, CPlotData& data);
	void draw_densityplot(QPainter& p, CPlotData& data);

public slots:
	void OnZoomToWidth();
	void OnZoomToHeight();
	void OnZoomToFit();
	void OnShowProps();
	QString OnCopyToClipboard();
	void OnBGImage();
	void OnClearBGImage();

private: // drawing helper functions
	void drawAxes(QPainter& p);
	void drawAllData(QPainter& p);
	void drawData(QPainter& p, CPlotData& data);
	void drawGrid(QPainter& p);
	void drawAxesTicks(QPainter& p);
	void drawAxesLabels(QPainter& p);
	void drawTitle(QPainter& p);
	void drawSelection(QPainter& p);
	void drawLegend(QPainter& p);

private:
	CGraphData		m_data;

	int		m_chartStyle;

	double	m_hlrng[2];	// range that will be highlighted on background

public:
	QRectF	m_viewRect;
	QRect	m_screenRect, m_titleRect, m_plotRect;
	QPoint	m_mousePos, m_mouseInitPos;
	QColor	m_selCol;
	double	m_xscale, m_yscale;

	bool		m_bviewLocked;
	bool		m_bshowPopup;
	bool		m_bshowToolTip;
	bool		m_bscaleAxisLabels;
	bool		m_bfullScreenMode;
	bool		m_bautoRngUpdate;
	bool		m_newSelect;
	bool		m_bdragging;

	QString		m_customXAxisLabel;
	QString		m_customYAxisLabel;

	bool		m_bregionSelect;

	std::vector<Selection>	m_selection;

private:
	QAction*	m_pZoomToFit;
	QAction*	m_pShowProps;
	QAction*	m_pCopyToClip;
	QAction*	m_pickBGImage;
	QAction*	m_clrBGImage;
	QSize		m_sizeHint;
	QImage*		m_img;
};


class CDlgPlotWidgetProps_Ui;

class CDlgPlotWidgetProps : public QDialog
{
	Q_OBJECT

public:
	CDlgPlotWidgetProps(QWidget* parent = 0);

	void SetRange(double xmin, double xmax, double ymin, double ymax);

	void accept();

public:
	double	m_xmin, m_xmax;
	double	m_ymin, m_ymax;

private:
	CDlgPlotWidgetProps_Ui*	ui;
};

class CCurvePlotWidget : public CPlotWidget
{
	Q_OBJECT

public:
	CCurvePlotWidget(QWidget* parent = nullptr);

	void DrawPlotData(QPainter& p, CPlotData& data) override;

	void SetLoadCurve(LoadCurve* lc);
	LoadCurve* GetLoadCurve();

private:
	LoadCurve* m_lc;
};

//=============================================================================
class UICurveEditWidget;

class CCurveEditWidget : public QWidget
{
	Q_OBJECT

public:
	CCurveEditWidget(QWidget* parent = nullptr);

	void Clear();

	void SetLoadCurve(LoadCurve* lc);

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

//=============================================================================
class CMathPlotWidget : public CPlotWidget
{
	Q_OBJECT

public:
	// NOTE: Make sure this matches the definition in FEMathIntervalController
	enum ExtendMode { ZERO, CONSTANT, REPEAT };

public:
	CMathPlotWidget(QWidget* parent = nullptr);
	void DrawPlotData(QPainter& p, CPlotData& data) override;

	void SetOrdinate(const std::string& x);

	void SetMath(const QString& txt);

	void ClearVariables();
	void SetVariable(const QString& name, double val);

	void setLeftExtendMode(int n);
	void setRightExtendMode(int n);

private:
	double value(double x, MVariable* var, int& region);

public slots:
	void onRegionSelected(QRect rt);
	void onPointClicked(QPointF pt, bool shift);

private:
	MSimpleExpression	m_math;
	std::string			m_ord;

	std::vector< std::pair<QString, double> >	m_Var;

	int		m_leftExtend;
	int		m_rghtExtend;
};

//=============================================================================
class UIMathEditWidget;

class CMathEditWidget : public QWidget
{
	Q_OBJECT

public:
	CMathEditWidget(QWidget* parent = nullptr);

	void SetOrdinate(const QString& ord);
	void SetMath(const QString& txt);
	void ClearVariables();
	void SetVariable(const QString& name, double val);

	void showRangeOptions(bool b);

	void setMinMaxRange(double rmin, double rmax);
	void setLeftExtend(int n);
	void setRightExtend(int n);

public slots:
	void onEditingFinished();
	void onLeftExtendChanged();
	void onRightExtendChanged();
	void onRangeMinChanged();
	void onRangeMaxChanged();

signals:
	void mathChanged(QString s);
	void leftExtendChanged(int n);
	void rightExtendChanged(int n);
	void minChanged(double vmin);
	void maxChanged(double vmax);

private:
	UIMathEditWidget* ui;
};
