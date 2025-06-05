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
#include <QMainWindow>
#include "PlotWidget.h"
#include "Document.h"

class CMainWindow;
class CGraphWidget;
class QLineEdit;
class QRadioButton;
class QCheckBox;
class QComboBox;
class QStackedWidget;
class QLabel;
class CDataSelector;
class CPostDocument;
class CColorButton;

namespace Ui {
	class CGraphWindow;
};

namespace Post {
	class ModelDataField;
};

//=================================================================================================
// Base class for graph tools
class CPlotTool : public QWidget
{
public:
	CPlotTool(QWidget* parent = 0) : QWidget(parent){}
	virtual ~CPlotTool(){}

	virtual void draw(QPainter& p) {}

	virtual void Update() {}
};

//=================================================================================================
// Time range options
class TimeRangeOptionsUI;

class TimeRangeOptions : public CPlotTool
{
	Q_OBJECT

public:
	int currentOption();
	void setUserTimeRange(int imin, int imax, int istep);
	void getUserTimeRange(int& imin, int& imax, int& istep);
	bool autoRangeUpdate();

public slots:
	void onOptionsChanged();

signals:
	void optionsChanged();

public:
	TimeRangeOptions(CGraphWidget* graph, QWidget* parent = 0);

private:
	TimeRangeOptionsUI*	ui;
};

//=================================================================================================
// Graph options
class GraphOptionsUI;

class GraphOptions : public CPlotTool
{
	Q_OBJECT

public:
	bool lineSmoothing();
	bool drawGrid();
	bool drawTitle();
	bool drawAxesLabels();
	bool drawLegend();
	QColor backgroundColor();
	QColor selectionColor();

	void Update();

public slots:
	void onOptionsChanged();

public:
	GraphOptions(CGraphWidget* graph, QWidget* parent = 0);

private:
	GraphOptionsUI*	ui;
};

//=================================================================================================
// data options
class DataOptionsUI;
class DataOptions : public CPlotTool
{
	Q_OBJECT

public slots:
	void onIndexChange(int n);
	void onDataChange(int n);
	void onLabelChanged();

public:
	DataOptions(CGraphWidget* graph, QWidget* parent = 0);

	void Update();

private:
	DataOptionsUI*	ui;
};

//=================================================================================================
// Linear regression tool
class RegressionUi : public CPlotTool
{
	Q_OBJECT

public:
	RegressionUi(CGraphWidget* graph, QWidget* parent = 0);

	void draw(QPainter& p) override;

	void Update() override;

	void hideEvent(QHideEvent* ev) override;

private:
	void showParameters(int numParam);
	void clearParameters();

public slots:
	void onCalculate();
	void onFunctionChanged(int n);
	void onColorChanged(QColor c);

private:
	CGraphWidget* m_graph;
	double	m_a, m_b, m_c;
	bool	m_bvalid;

private:
	QComboBox*	m_src;
	QComboBox*	m_fnc;
	QLabel*		m_math;

	QLabel*		m_lbl[3];
	QLineEdit*	m_par[3];
	QColor		m_col;
};

//=================================================================================================
// Mathematical plotting tool
class MathPlot : public CPlotTool
{
	Q_OBJECT

public:
	MathPlot(CGraphWidget* graph, QWidget* parent = 0);

	void draw(QPainter& p) override;

	void Update() override;

	void hideEvent(QHideEvent* ev) override;

public slots:
	void onCalculate();
	void onColorChanged(QColor c);

private:
	static double graphdata(double x);

private:
	CGraphWidget*	m_graph;
	QLineEdit*		m_edit;

	bool			m_bvalid;
	std::string		m_math;
	QColor			m_col;

	static MathPlot* m_pThis;
	LoadCurve	m_data;
};

//=================================================================================================
class CGraphWidget : public CPlotWidget
{
public:
	CGraphWidget(QWidget *parent, int w = 0, int h = 0) : CPlotWidget(parent, w, h){}

	void addTool(CPlotTool* tool) { m_tools.push_back(tool); }

	void paintEvent(QPaintEvent* pe);

	void Update();

public:
	vector<CPlotTool*>	m_tools;
};

//=================================================================================================
class CGraphWindow : public QMainWindow, public CDocObserver
{
	Q_OBJECT

public:
	enum TimeRange
	{
		TRACK_TIME,
		TRACK_CURRENT_TIME,
		TRACK_USER_RANGE
	};

	enum PlotType
	{
		LINE_PLOT,
		SCATTER_PLOT,
		TIME_SCATTER_PLOT
	};

	enum GraphOptions
	{
		SHOW_TYPE_OPTIONS = 0x01,
		SHOW_TIME_RANGE = 0x02,
		SHOW_DATA_SOURCE = 0x04,
		SHOW_ALL_OPTIONS = 0xFF
	};

public:
	CGraphWindow(CMainWindow* wnd, CPostDocument* postDoc, int flags = SHOW_ALL_OPTIONS);

	virtual void Update(bool breset = true, bool bfit = false) = 0;

	void closeEvent(QCloseEvent* closeEvent) override;
    void resizeEvent(QResizeEvent* resizeEvent) override;
    void moveEvent(QMoveEvent* moveEvent) override;

	void EnablePasteButton(bool b);

	static QRect preferredSize();
	static void setPreferredSize(const QRect& rt);

	// get the plot widget
	CPlotWidget* GetPlotWidget();

	// get the post doc
	CPostDocument* GetPostDoc();

public:
	int GetTimeTrackOption();
	void GetUserTimeRange(int& userMin, int& userMax, int& step);
	void GetTimeRange(int& minTime, int& maxTime, int& step);

public: // convenience functions for modifying the plot widget
	// number of data plots
	int Plots();

	// get the plot data
	CPlotData* GetPlotData(int i);

	// clear all plots
	void ClearPlots();

	// resize plots
	void ResizePlots(int n);

	// clear data on plots
	void ClearPlotsData();

	// add a plot
	void AddPlotData(CPlotData* plot);

	// update all plots
	void UpdatePlots();

	// redraw the plot widget
	void RedrawPlot();

	// Fit all plots to the its data
	void FitPlotsToData();

	// set the title of the plot widget
	void SetPlotTitle(const QString& title);

	// set the axis labels
	void SetXAxisLabel(const QString& label);
	void SetYAxisLabel(const QString& label);

	QString XAxisLabel();
	QString YAxisLabel();

public:
	// set the data selector for the X field
	void SetXDataSelector(CDataSelector* sel, int nval = -1);

	// set the data selector for the Y field
	void SetYDataSelector(CDataSelector* sel, int nval = -1);

	// get the text of the current selection in the X data selector
	QString GetCurrentXText();

	// get the data value associated with the current X selection
	int GetCurrentXValue();

	// get the text of the current selection in the Y data selector
	QString GetCurrentYText();

	// get the data value associated with the current X selection
	int GetCurrentYValue();

	// get the current plot type
	int GetCurrentPlotType();

	// set source options
	void SetDataSource(const QStringList& names);

public:
	void ShowAddToModelButton(bool b);

protected:
	void AddToolBarWidget(QWidget* w);

	void AddPanel(QWidget* w);

	virtual void setDataSource(int n) {}

	int currentDataSource();

private:
	// from CDocObserver
	void DocumentUpdate(bool newDoc) override;
	void DocumentDelete() override;

private slots:
	void on_selectX_currentValueChanged(int);
	void on_selectY_currentValueChanged(int);
	void on_selectPlot_currentIndexChanged(int);
	void on_actionSave_triggered();
	void on_actionAddToModel_triggered();
	void on_actionCopy_triggered();
	void on_actionPaste_triggered();
	void on_actionSnapshot_triggered();
	void on_actionProps_triggered();
	void on_actionZoomWidth_triggered();
	void on_actionZoomHeight_triggered();
	void on_actionZoomFit_triggered();
	void on_plot_regionSelected(QRect rt);
	void on_range_optionsChanged();
	void on_dataSource_currentIndexChanged(int);

protected:
	virtual void PasteClipboardData() {}

private:
	CMainWindow*		m_wnd;
	Ui::CGraphWindow*	ui;

	int		m_nTrackTime;
	int		m_nUserMin, m_nUserMax, m_nUserStep;	//!< manual time step range

	static QRect	m_preferredSize;
};

//=================================================================================================
class CDataGraphWindow : public CGraphWindow
{
public:
	CDataGraphWindow(CMainWindow* wnd, CPostDocument* doc);

	void SetData(CGraphData* data);

	void Update(bool breset = true, bool bfit = false);

	void closeEvent(QCloseEvent* ev);

	void PasteClipboardData() override;

private:
	CGraphData*	m_data;
};

//=================================================================================================
// Specialized graph for displaying data from a model's selection

namespace Post {
	class GLPointProbe;
	class GLRuler;
	class GLMusclePath;
}

class CModelGraphWindow : public CGraphWindow
{
public:
	CModelGraphWindow(CMainWindow* wnd, CPostDocument* postDoc);

	void Update(bool breset = true, bool bfit = false) override;

private:
	// track mesh data
	void TrackElementHistory(int nelem, float* pval, int nfield, int nmin = 0, int nmax = -1);
	void TrackFaceHistory(int nface, float* pval, int nfield, int nmin = 0, int nmax = -1);
	void TrackEdgeHistory(int edge, float* pval, int nfield, int nmin = 0, int nmax = -1);
	void TrackNodeHistory(int node, float* pval, int nfield, int nmin = 0, int nmax = -1);
	void TrackObjectHistory(int nobj, float* pval, int nfield, int nmin = 0, int nmax = -1);

private:
	void addSelectedNodes();
	void addSelectedEdges();
	void addSelectedFaces();
	void addSelectedElems();
	void addObjectData(int n);
	void addGlobalData(Post::ModelDataField* pdf, int n);
	void addProbeData(Post::GLPointProbe* probe);
	void addRulerData(Post::GLRuler* ruler);
	void addMusclePathData(Post::GLMusclePath* musclePath);

	CPlotData* nextData();

	void setDataSource(int n) override;

private: // temporary variables used during update
	int	m_xtype, m_xtypeprev;			// x-plot field option (0=time, 1=steps, 2=data field)
	int	m_firstState, m_lastState, m_incState;		// first, last, and state increment for time steps to be evaluated
	int	m_dataX, m_dataY;				// X and Y data field IDs
	int	m_dataXPrev, m_dataYPrev;		// Previous X, Y data fields
	int	m_pltCounter;
};

