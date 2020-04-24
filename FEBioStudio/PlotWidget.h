#pragma once
#include <QWidget>
#include <vector>
#include <QDialog>
using namespace std;

//-----------------------------------------------------------------------------
class QPainter;
class QAction;
class CPlotWidget;
class QImage;

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
	int size() const { return (int) m_data.size(); }

	// get a data point
	QPointF& Point(int i) { return m_data[i]; }

	// get the bounding rectangle
	QRectF boundRect() const;

	// set/get the label
	const QString& label() const { return m_label; }
	void setLabel(const QString& label) { m_label = label; }

	// set/get color
	QColor color() const { return m_col; }
	void setColor(const QColor& col) { m_col = col; }

	// sort the data
	void sort();

public:
	virtual	void draw(QPainter& painter, CPlotWidget& plt) = 0;

protected:
	vector<QPointF>	m_data;
	QString			m_label;
	QColor			m_col;
};

//-----------------------------------------------------------------------------
class CLineChartData : public CPlotData
{
public:
	void draw(QPainter& painter, CPlotWidget& plt);
};

//-----------------------------------------------------------------------------
class CBarChartData : public CPlotData
{
public:
	void draw(QPainter& painter, CPlotWidget& plt);
};

//-----------------------------------------------------------------------------
struct CAxisFormat
{
	bool	visible;
	int		labelPosition;
	int		labelAlignment;
};

//-----------------------------------------------------------------------------
//! This class implements a plotting widget. 
class CPlotWidget : public QWidget
{
	Q_OBJECT

public:
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
	QString title() const { return m_title; }

	// size hint
	QSize sizeHint() const { return m_sizeHint; }
	QSize minimumSizeHint() const { return QSize(200, 200); }

	// clear plot data
	// This just clears the data for each plot
	// but does not delete the plots
	void clearData();

	// clears everything
	void clear();

	// get/set show legend
	bool showLegend() const { return m_bshowLegend; }
	void showLegend(bool b) { m_bshowLegend = b; }

	// change the view so that it fits the data
	void fitWidthToData();
	void fitHeightToData();
	void fitToData(bool downSize = true);
	void fitToRect(const QRect& rt);

	void setViewRect(const QRectF& rt);

	// add a data field
	void addPlotData(CPlotData* p);

	// get a data field
	int plots() { return (int) m_data.size(); }
	CPlotData& getPlotData(int i) { return *m_data[i]; }

	// turn on/off zoom-to-rect mode
	void ZoomToRect(bool b = true);

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

	void showHorizontalGridLines(bool b) { m_bdrawYLines = b; }
	void showVerticalGridLines(bool b) { m_bdrawXLines = b; }

	void showXAxis(bool b) { m_xAxis.visible = b; }
	void ShowYAxis(bool b) { m_yAxis.visible = b; }

	bool lineSmoothing() const { return m_bsmoothLines; }
	void setLineSmoothing(bool b) { m_bsmoothLines = b; }

	bool showDataMarks() const { return m_bshowDataMarks; }
	void showDataMarks(bool b) { m_bshowDataMarks = b; }

	void setDataMarkSize(int n) { m_dataMarkSize = n; }
	int getDataMarkSize() const { return m_dataMarkSize; }

	void scaleAxisLabels(bool b) { m_bscaleAxisLabels = b; }

	bool autoRangeUpdate() const { return m_bautoRngUpdate; }
	void setAutoRangeUpdate(bool b) { m_bautoRngUpdate = b; }

	QPointF SnapToGrid(const QPointF& p);

	QPointF dataPoint(int ndata, int npoint);

	void setFullScreenMode(bool b) { m_bfullScreenMode = b; }

	void setXAxisLabelAlignment(AxisLabelAlignment a);
	void setYAxisLabelAlignment(AxisLabelAlignment a);

	void setBackgroundColor(const QColor& c) { m_bgCol = c; }
	void setGridColor(const QColor& c) { m_gridCol = c; }
	void setXAxisColor(const QColor& c) { m_xCol = c; }
	void setYAxisColor(const QColor& c) { m_yCol = c; }
	void setSelectionColor(const QColor& c) { m_selCol = c; }

	void selectPoint(int ndata, int npoint);

	QRect ScreenRect() const { return m_screenRect; }

	bool HasBackgroundImage() const;

	void mapToUserRect();

	void mapToUserRect(QRect rt, QRectF rng);

	vector<Selection> selection() const { return m_selection; }

signals:
	void doneZoomToRect();
	void doneSelectingRect(QRect rt);
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

	void regionSelect(QRect rt);

	void addToSelection(int ndata, int npoint);

public:
	QString	m_title;
	QRectF	m_viewRect;
	QRect	m_screenRect, m_titleRect;
	QPoint	m_mousePos, m_mouseInitPos;
	QColor	m_bgCol;
	QColor	m_gridCol;
	QColor	m_xCol;
	QColor	m_yCol;
	QColor	m_selCol;
	double	m_xscale, m_yscale;

	bool		m_bzoomRect;
	bool		m_bvalidRect;
	bool		m_mapToRect;
	bool		m_newSelect;
	bool		m_bdragging;
	bool		m_bregionSelect;
	bool		m_bsmoothLines;
	bool		m_bshowDataMarks;
	int			m_dataMarkSize;

	vector<Selection>	m_selection;

	QPointF ScreenToView(const QPoint& p);
	QRectF ScreenToView(const QRect& rt);
	QPoint ViewToScreen(const QPointF& p);

	void SetBackgroundImage(QImage* img);

protected:
	//! render the plot
	void paintEvent(QPaintEvent* pe);

public slots:
	void OnZoomToWidth();
	void OnZoomToHeight();
	void OnZoomToFit();
	void OnShowProps();
	void OnCopyToClipboard();
	void OnBGImage();
	void OnClearBGImage();

private: // drawing helper functions
	void drawAxes(QPainter& p);
	void drawAllData(QPainter& p);
	void drawData(QPainter& p, CPlotData& data);
	void drawGrid(QPainter& p);
	void drawAxesLabels(QPainter& p);
	void drawTitle(QPainter& p);
	void drawSelection(QPainter& p);
	void drawLegend(QPainter& p);

private:
	vector<CPlotData*>	m_data;
	bool				m_bshowLegend;
	bool				m_bviewLocked;
	bool				m_bshowPopup;
	bool				m_bshowToolTip;
	bool				m_bdrawXLines;
	bool				m_bdrawYLines;
	bool				m_bscaleAxisLabels;
	bool				m_bfullScreenMode;
	bool				m_bautoRngUpdate;

	int		m_chartStyle;
	CAxisFormat		m_xAxis;
	CAxisFormat		m_yAxis;

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
