#pragma once
#include <QMainWindow>
#include <QDockWidget>
#include <MathLib/MathParser.h>
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
// Graph options
class GraphOptionsUI;

class GraphOptions : public CPlotTool
{
	Q_OBJECT

public:
	int currentOption();
	void setUserTimeRange(int imin, int imax);
	void getUserTimeRange(int& imin, int& imax);

	bool lineSmoothing();

	bool autoRangeUpdate();

	bool drawGrid();

public slots:
	void onOptionsChanged();

signals:
	void optionsChanged();

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
	CGraphWidget*	m_graph;
	QLineEdit*		m_edit;

	bool			m_bvalid;
	std::string		m_math;
	QColor			m_col;
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
class CGraphWindow : public QDockWidget, public CDocObserver
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
		SHOW_TYPE_OPTIONS = 1,
		SHOW_ALL_OPTIONS = 0xFF
	};

public:
	CGraphWindow(CMainWindow* wnd, CPostDocument* postDoc, int flags = SHOW_ALL_OPTIONS);

	virtual void Update(bool breset = true, bool bfit = false) = 0;

	void closeEvent(QCloseEvent* closeEvent) override;

	static QRect preferredSize();
	static void setPreferredSize(const QRect& rt);

	// get the plot widget
	CPlotWidget* GetPlotWidget();

	// get the post doc
	CPostDocument* GetPostDoc();

public:
	int GetTimeTrackOption();
	void GetUserTimeRange(int& userMin, int& userMax);
	void GetTimeRange(int& minTime, int& maxTime);

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

private:
	// from CDocObserver
	void DocumentUpdate(bool newDoc) override;
	void DocumentDelete() override;

private slots:
	void on_selectX_currentValueChanged(int);
	void on_selectY_currentValueChanged(int);
	void on_selectPlot_currentIndexChanged(int);
	void on_actionSave_triggered();
	void on_actionClipboard_triggered();
	void on_actionSnapshot_triggered();
	void on_actionProps_triggered();
	void on_actionZoomWidth_triggered();
	void on_actionZoomHeight_triggered();
	void on_actionZoomFit_triggered();
	void on_actionZoomSelect_toggled(bool bchecked);
	void on_plot_doneZoomToRect();
	void on_options_optionsChanged();

private:
	CMainWindow*		m_wnd;
	Ui::CGraphWindow*	ui;

	int		m_nTrackTime;
	int		m_nUserMin, m_nUserMax;	//!< manual time step range

	static QRect	m_preferredSize;
};

//=================================================================================================
class CDataGraphWindow : public CGraphWindow
{
public:
	CDataGraphWindow(CMainWindow* wnd, CPostDocument* postDoc);

	void SetData(const std::vector<double>& data, QString title);

	void Update(bool breset = true, bool bfit = false);

private:
	QString				m_title;
	std::vector<double>	m_data;
};

//=================================================================================================
// Specialized graph for displaying data from a model's selection
class CModelGraphWindow : public CGraphWindow
{
public:
	CModelGraphWindow(CMainWindow* wnd, CPostDocument* postDoc);

	void Update(bool breset = true, bool bfit = false);

private:
	// track mesh data
	void TrackElementHistory(int nelem, float* pval, int nfield, int nmin = 0, int nmax = -1);
	void TrackFaceHistory(int nface, float* pval, int nfield, int nmin = 0, int nmax = -1);
	void TrackEdgeHistory(int edge, float* pval, int nfield, int nmin = 0, int nmax = -1);
	void TrackNodeHistory(int node, float* pval, int nfield, int nmin = 0, int nmax = -1);

private:
	void addSelectedNodes();
	void addSelectedEdges();
	void addSelectedFaces();
	void addSelectedElems();

	CLineChartData* nextData();

private: // temporary variables used during update
	int	m_xtype, m_xtypeprev;			// x-plot field option (0=time, 1=steps, 2=data field)
	int	m_firstState, m_lastState;		// first and last time step to be evaluated
	int	m_dataX, m_dataY;				// X and Y data field IDs
	int	m_dataXPrev, m_dataYPrev;		// Previous X, Y data fields
	int	m_pltCounter;
};

