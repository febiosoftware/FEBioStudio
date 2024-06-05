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
#include "PlotWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QFontDatabase>
#include <QMouseEvent>
#include <QAction>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <assert.h>
#include <math.h>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QLineEdit>
#include <QValidator>
#include <QImage>
#include <QFileDialog>
#include <QRgb>
#include <QDropEvent>
#include <QToolButton>
#include <QComboBox>
#include <QLabel>
#include <QMessageBox>
#include <QtCore/QMimeData>
#include <FSCore/LoadCurve.h>
#include "MainWindow.h"	// for CResource
#include "DlgFormula.h"
#include "Command.h"
#include "DlgImportData.h"
#include "IconProvider.h"

using namespace std;

#ifdef WIN32
#include <float.h>
#define ISNAN(x) _isnan(x)
#endif

#ifdef LINUX
#ifdef CENTOS
#define ISNAN(x) isnan(x)
#else
#define ISNAN(x) std::isnan(x)
#endif
#endif

#ifdef __APPLE__
#include <math.h>
#define ISNAN(x) isnan(x)
#endif


//-----------------------------------------------------------------------------
class CDlgPlotWidgetProps_Ui
{
public:
	QLineEdit*	x[2];
	QLineEdit*	y[2];

public:
	void setup(CDlgPlotWidgetProps* dlg)
	{
		QFormLayout* form = new QFormLayout;

		form->addRow("X min:", x[0] = new QLineEdit); x[0]->setValidator(new QDoubleValidator);
		form->addRow("X max:", x[1] = new QLineEdit); x[1]->setValidator(new QDoubleValidator);

		form->addRow("Y min:", y[0] = new QLineEdit); y[0]->setValidator(new QDoubleValidator);
		form->addRow("Y max:", y[1] = new QLineEdit); y[1]->setValidator(new QDoubleValidator);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(form);
		l->addWidget(bb);

		dlg->setLayout(l);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
	}
};

CDlgPlotWidgetProps::CDlgPlotWidgetProps(QWidget* parent) : QDialog(parent), ui(new CDlgPlotWidgetProps_Ui)
{
	setWindowTitle("Map region");
	ui->setup(this);
}

void CDlgPlotWidgetProps::SetRange(double xmin, double xmax, double ymin, double ymax)
{
	if (ui)
	{
		ui->x[0]->setText(QString::number(xmin));
		ui->x[1]->setText(QString::number(xmax));

		ui->y[0]->setText(QString::number(ymin));
		ui->y[1]->setText(QString::number(ymax));
	}
}

void CDlgPlotWidgetProps::accept()
{
	m_xmin = ui->x[0]->text().toDouble();
	m_xmax = ui->x[1]->text().toDouble();
	m_ymin = ui->y[0]->text().toDouble();
	m_ymax = ui->y[1]->text().toDouble();

	QDialog::accept();
}

//=============================================================================

class ColorList
{
public:
	enum { Colors = 16 };

public:
	static QColor color(int i) { return m_col[i%m_col.size()]; }
	static void init()
	{
		QStringList colorNames = QColor::colorNames();
		m_col.resize(Colors);
		m_col[ 0] = QColor(colorNames[13]);
		m_col[ 1] = QColor(colorNames[14]);
		m_col[ 2] = QColor(colorNames[15]);
		m_col[ 3] = QColor(colorNames[16]);
		m_col[ 4] = QColor(colorNames[17]);
		m_col[ 5] = QColor(colorNames[19]);
		m_col[ 6] = QColor(colorNames[20]);
		m_col[ 7] = QColor(colorNames[21]);
		m_col[ 8] = QColor(colorNames[22]);
		m_col[ 9] = QColor(colorNames[23]);
		m_col[10] = QColor(colorNames[24]);
		m_col[11] = QColor(colorNames[25]);
		m_col[12] = QColor(colorNames[27]);
		m_col[13] = QColor(colorNames[28]);
		m_col[14] = QColor(colorNames[29]);
		m_col[15] = QColor(colorNames[30]);
	}

private:
	ColorList() {}
	ColorList(const ColorList&) {}

private:
	static vector<QColor>	m_col;
};
vector<QColor> ColorList::m_col;

//-----------------------------------------------------------------------------
double findScale(double fmin, double fmax)
{
	double dx = fmax - fmin;
	double p = floor(log10(dx));
	double f = pow(10.0, p - 1);
	double m = floor(dx / f);

	double dd = f;
	if      (m > 75) dd = 10*f;
	else if (m > 30) dd = 5*f;
	else if (m > 15) dd = 2*f;

	return dd;
}

//-----------------------------------------------------------------------------
void drawDiamond(QPainter& painter, const QRect& rt)
{
	QPoint c = rt.center();
	const QPointF points[4] = {
		QPointF(c.x(), rt.top()),
		QPointF(rt.left(), c.y()),
		QPointF(c.x(), rt.bottom()),
		QPointF(rt.right(), c.y())
	};

	painter.drawConvexPolygon(points, 4);
}

//-----------------------------------------------------------------------------
void drawTriangle(QPainter& painter, const QRect& rt)
{
	QPoint c = rt.center();
	const QPointF points[3] = {
		QPointF(c.x(), rt.top()),
		QPointF(rt.left(), rt.bottom()),
		QPointF(rt.right(), rt.bottom()),
	};

	painter.drawConvexPolygon(points, 3);
}

//-----------------------------------------------------------------------------
void drawCross(QPainter& painter, const QRect& rt)
{
	painter.drawLine(rt.topLeft(), rt.bottomRight());
	painter.drawLine(rt.topRight(), rt.bottomLeft());
}

//-----------------------------------------------------------------------------
void drawPlus(QPainter& painter, const QRect& rt)
{
	QPoint c = rt.center();
	painter.drawLine(c.x(), rt.top(), c.x(), rt.bottom());
	painter.drawLine(rt.left(), c.y(), rt.right(), c.y());
}

//-----------------------------------------------------------------------------
void drawMarker(QPainter& painter, const QPointF& pt, int nsize, int type)
{
	int n2 = nsize / 2;
	QRect rect(pt.x() - n2, pt.y() - n2, nsize, nsize);
	switch (type)
	{
	case 0: break;
	case 1: painter.drawRect(rect); break;
	case 2: painter.drawEllipse(rect); break;
	case 3: drawDiamond(painter, rect); break;
	case 4: drawTriangle(painter, rect); break;
	case 5: drawCross(painter, rect); break;
	case 6: drawPlus(painter, rect); break;
	}
}

//=============================================================================

CPlotData::CPlotData()
{
	m_lineWidth = 2;
	m_markerSize = 5;
	m_markerType = 1;
}

//-----------------------------------------------------------------------------
CPlotData::~CPlotData()
{
}

//-----------------------------------------------------------------------------
CPlotData::CPlotData(const CPlotData& d)
{
	m_data = d.m_data;
	m_label = d.m_label;
	m_lineColor = d.m_lineColor;
	m_fillColor = d.m_fillColor;
	m_lineWidth = d.m_lineWidth;
	m_markerSize = d.m_markerSize;
	m_markerType = d.m_markerType;
}

//-----------------------------------------------------------------------------
CPlotData& CPlotData::operator = (const CPlotData& d)
{
	m_data = d.m_data;
	m_label = d.m_label;
	m_lineColor = d.m_lineColor;
	m_fillColor = d.m_fillColor;
	m_lineWidth = d.m_lineWidth;
	m_markerSize = d.m_markerSize;
	m_markerType = d.m_markerType;
	return *this;
}

//-----------------------------------------------------------------------------
void CPlotData::clear()
{ 
	m_data.clear(); 
}

//-----------------------------------------------------------------------------
QRectF CPlotData::boundRect() const
{
	if (m_data.empty()) return QRectF(0., 0., 0., 0.);

	QRectF r(m_data[0].x(), m_data[0].y(), 0.0, 0.0);
	for (int i=1; i<(int)m_data.size(); ++i)
	{
		const QPointF& p = m_data[i];
		if (p.x() < r.left  ()) r.setLeft  (p.x());
		if (p.x() > r.right ()) r.setRight (p.x());
		if (p.y() > r.bottom()) r.setBottom(p.y());
		if (p.y() < r.top   ()) r.setTop   (p.y());
	}
	return r;
}

//-----------------------------------------------------------------------------
void CPlotData::addPoint(double x, double y)
{
	QPointF p(x, y);
	m_data.push_back(p);
}

//-----------------------------------------------------------------------------
int compare(const void* p1, const void* p2)
{
	const QPointF& r1 = *((const QPointF*)(p1));
	const QPointF& r2 = *((const QPointF*)(p2));

	if (r1.x() < r2.x()) return -1;
	else if (r1.x() > r2.x()) return 1;
	else return 0;
}

void CPlotData::sort()
{
	if (m_data.size() > 0)
		qsort(&m_data[0], m_data.size(), sizeof(QPointF), compare);
}

//-----------------------------------------------------------------------------
CPlotWidget::CPlotWidget(QWidget* parent, int w, int h) : QWidget(parent)
{
	ColorList::init();

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_bregionSelect = false;

	m_bautoRngUpdate = true;

	m_bviewLocked = false;
	m_bshowPopup = true;
	m_bscaleAxisLabels = true;
	m_bfullScreenMode = false;
	m_bshowToolTip = true;

	m_chartStyle = LINECHART_PLOT;

	// set default colors
	m_selCol = QColor::fromRgb(255, 255, 0);

	m_viewRect = QRectF(0.0, 0.0, 1.0, 1.0);
	m_xscale = findScale(m_viewRect.left(), m_viewRect.right());
	m_yscale = findScale(m_viewRect.top(), m_viewRect.bottom());

	if (w < 200) w = 200;
	if (h < 200) h = 200;
	m_sizeHint = QSize(w, h);

	m_hlrng[0] = m_hlrng[1] = 0.0;

	// allow drop events
	setAcceptDrops(true);

	m_img = nullptr;

	m_pZoomToFit = new QAction(QIcon(QString(":/icons/zoom_fit.png")), tr("Zoom to fit"), this);
	connect(m_pZoomToFit, SIGNAL(triggered()), this, SLOT(OnZoomToFit()));

	m_pShowProps = new QAction(QIcon(QString(":/icons/properties.png")), tr("Properties"), this);
	connect(m_pShowProps, SIGNAL(triggered()), this, SLOT(OnShowProps()));

	m_pCopyToClip = new QAction(QIcon(QString(":/icons/clipboard.png")), tr("Copy to clipboard"), this);
	connect(m_pCopyToClip, SIGNAL(triggered()), this, SLOT(OnCopyToClipboard()));

	m_pickBGImage = new QAction(QIcon(QString(":/icons/bgimage.png")), tr("Select Background Image"), this);
	connect(m_pickBGImage, SIGNAL(triggered()), this, SLOT(OnBGImage()));

	m_clrBGImage = new QAction(tr("Clear Background Image"), this);
	connect(m_clrBGImage, SIGNAL(triggered()), this, SLOT(OnClearBGImage()));
}

//-----------------------------------------------------------------------------
void CPlotWidget::setChartStyle(int chartStyle)
{
	m_chartStyle = chartStyle;
}

//-----------------------------------------------------------------------------
void CPlotWidget::contextMenuEvent(QContextMenuEvent* ev)
{
	if (m_bshowPopup)
	{
		QMenu menu(this);
		menu.addAction(m_pZoomToFit);
		menu.addAction(m_pCopyToClip);
		menu.addSeparator();
		menu.addAction(m_pShowProps);
		menu.addAction(m_pickBGImage);
		menu.addAction(m_clrBGImage);
		menu.exec(ev->globalPos());
	}
}

//-----------------------------------------------------------------------------
void CPlotWidget::OnZoomToWidth()
{
	fitWidthToData();
	repaint();
}

//-----------------------------------------------------------------------------
void CPlotWidget::OnZoomToHeight()
{
	fitHeightToData();
	repaint();
}

//-----------------------------------------------------------------------------
void CPlotWidget::OnZoomToFit()
{
	fitToData();
	repaint();
}

//-----------------------------------------------------------------------------
void CPlotWidget::OnShowProps()
{
	CDlgPlotWidgetProps dlg(this);
	QRectF rt = m_viewRect;
	dlg.SetRange(rt.left(), rt.right(), rt.top(), rt.bottom());

	if (dlg.exec())
	{
		rt = QRectF(QPointF(dlg.m_xmin, dlg.m_ymin), QPointF(dlg.m_xmax, dlg.m_ymax));

		setViewRect(rt);
		repaint();
	}
}

//-----------------------------------------------------------------------------
QString CPlotWidget::OnCopyToClipboard()
{
	QClipboard* clipboard = QApplication::clipboard();

	// get the number of plots
	const int nplots = plots();
	if (nplots == 0) return "";

	// if all plots have the same number of data points we'll output x, y1, y2, y3, ...
	// otherwise we'll output x1,y1,x2,y1,x3, y3
	CPlotData& d = *m_data.m_data[0];
	int max_size = d.size();
	bool equalSize = true;
	for (int i = 1; i<nplots; ++i)
	{
		CPlotData& di = *m_data.m_data[i];
		if (di.size() != max_size)
		{
			max_size = (di.size() > max_size ? di.size() : max_size);
			equalSize = false;
		}
		else if (equalSize)
		{
			// see if x-coordinates match
			for (int j = 0; j<max_size; ++j)
			{
				if (d.Point(j).x() != di.Point(j).x())
				{
					equalSize = false;
					break;
				}
			}
		}
	}

	// make sure there is data
	if (max_size == 0) return "";

	if (equalSize)
	{
		QString s("x");
		for (int i = 0; i<nplots; ++i) s += "\t" + m_data.m_data[i]->label();
		s += '\n';
		for (int i = 0; i<d.size(); ++i)
		{
			QPointF& pi = d.Point(i);
			s.append(QString::asprintf("%lg", pi.x()));

			for (int j = 0; j<plots(); ++j)
			{
				QPointF& pi = m_data.m_data[j]->Point(i);
				s.append(QString::asprintf("\t%lg", pi.y()));
			}

			s += '\n';
		}
		clipboard->setText(s);

        return s;
	}
	else
	{
		QString s;
		for (int i = 0; i<nplots; ++i)
		{
			s += QString("x%1\t%2\t").arg(i + 1).arg(m_data.m_data[i]->label());
		}
		s += '\n';

		for (int i = 0; i<max_size; ++i)
		{
			for (int j = 0; j<nplots; ++j)
			{
				if (i < m_data.m_data[j]->size())
				{
					QPointF& pi = m_data.m_data[j]->Point(i);
					s.append(QString::asprintf("%lg\t%lg\t", pi.x(), pi.y()));
				}
				else s.append("\t\t");
			}

			s += '\n';
		}
		clipboard->setText(s);

        return s;
	}
}

//-----------------------------------------------------------------------------
bool CPlotWidget::HasBackgroundImage() const
{
	return (m_img != nullptr);
}

//-----------------------------------------------------------------------------
void CPlotWidget::mapToUserRect(QRect rt, QRectF rng)
{
	QRect src = m_plotRect;// rect();
	double rx = rng.width() / rt.width();
	double xmin = rng.left() - rx*(rt.left() - src.left());
	double xmax = rng.right() + rx*(src.right() - rt.right());

	double ry = rng.height() / rt.height();
	int y0 = src.top();
	int y1 = src.bottom();
	double ymax = rng.bottom() + ry*(rt.top() - src.top());
	double ymin = rng.top()    - ry*(src.bottom() - rt.bottom());

	setViewRect(QRectF(xmin, ymin, xmax - xmin, ymax - ymin));
}

//-----------------------------------------------------------------------------
void CPlotWidget::dragEnterEvent(QDragEnterEvent* e)
{
	if (e->mimeData()->hasUrls()) {
		e->acceptProposedAction();
	}
}

//-----------------------------------------------------------------------------
void CPlotWidget::dropEvent(QDropEvent* e)
{
	QList<QUrl> urls = e->mimeData()->urls();
	if (urls.empty()) return;
	QString fileName = urls.at(0).toLocalFile();
	if (fileName.isEmpty() == false)
	{
		LoadBackgroundImage(fileName);
	}
}

//-----------------------------------------------------------------------------
void CPlotWidget::OnBGImage()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Select Image", "", "PNG Images (*png)");
	if (fileName.isEmpty() == false)
	{
		LoadBackgroundImage(fileName);
	}
}

//-----------------------------------------------------------------------------
bool CPlotWidget::LoadBackgroundImage(const QString& fileName)
{
	QImage* img = new QImage(fileName);
	SetBackgroundImage(img);

	emit backgroundImageChanged();

	repaint();

	return true;
}

//-----------------------------------------------------------------------------
void CPlotWidget::OnClearBGImage()
{
	SetBackgroundImage(nullptr);
	emit backgroundImageChanged();
	repaint();
}

//-----------------------------------------------------------------------------
void CPlotWidget::setTitle(const QString& t)
{
	m_data.m_title = t;
}

//-----------------------------------------------------------------------------
void CPlotWidget::clearData()
{
	m_selection.clear();
	for (int i=0; i<(int)m_data.m_data.size(); ++i) m_data.m_data[i]->clear();
}

//-----------------------------------------------------------------------------
void CPlotWidget::Resize(int n)
{
	for (int i = n; i < plots(); ++i) delete m_data.m_data[i];
	m_data.m_data.resize(n);
}

//-----------------------------------------------------------------------------
void CPlotWidget::clear()
{
	m_selection.clear();
	m_data.ClearData();
	repaint();
}

//-----------------------------------------------------------------------------
// set the data
void CPlotWidget::SetGraphData(const CGraphData& data)
{
	m_data = data;
}

//-----------------------------------------------------------------------------
// get the graph data
const CGraphData& CPlotWidget::GetGraphData() const
{
	return m_data;
}

//-----------------------------------------------------------------------------
void CPlotWidget::addPlotData(CPlotData* p)
{
	int N = (int)m_data.m_data.size();

	p->setLineColor(ColorList::color(N));
	p->setFillColor(ColorList::color(N));

	m_data.m_data.push_back(p);

	if (m_bautoRngUpdate)
	{
		fitToData(false);
	}
}

//-----------------------------------------------------------------------------
QPointF CPlotWidget::dataPoint(int ndata, int npoint)
{
	return m_data.m_data[ndata]->Point(npoint);
}

//-----------------------------------------------------------------------------
// helper function for evaluating the union of rects
// (Qt's QRectF::united doesn't always seem to work correctly)
QRectF rectUnion(const QRectF& r1, const QRectF& r2)
{
	QRectF r(r1);
	if (r2.left() < r.left()) r.setLeft(r2.left());
	if (r2.right() > r.right()) r.setRight(r2.right());
	if (r2.top() < r.top()) r.setTop(r2.top());
	if (r2.bottom() > r.bottom()) r.setBottom(r2.bottom());
	return r;
}

//-----------------------------------------------------------------------------
void CPlotWidget::fitWidthToData()
{
	if (m_data.m_data.empty()) return;

	QRectF r = m_data.m_data[0]->boundRect();
	for (int i = 1; i<(int)m_data.m_data.size(); ++i)
	{
		QRectF ri = m_data.m_data[i]->boundRect();
		r = rectUnion(r, ri);
	}

	r.setTop(m_viewRect.top());
	r.setBottom(m_viewRect.bottom());
	setViewRect(r);
}

//-----------------------------------------------------------------------------
void CPlotWidget::fitHeightToData()
{
	if (m_data.m_data.empty()) return;

	QRectF r = m_data.m_data[0]->boundRect();
	for (int i = 1; i<(int)m_data.m_data.size(); ++i)
	{
		QRectF ri = m_data.m_data[i]->boundRect();
		r = rectUnion(r, ri);
	}

	r.setLeft(m_viewRect.left());
	r.setRight(m_viewRect.right());
	setViewRect(r);
}

//-----------------------------------------------------------------------------
inline bool ContainsRect(QRectF& r0, QRectF& r1)
{
	if ((r1.left() >= r0.left()) && (r1.right() <= r0.right()) &&
		(r1.top() >= r0.top()) && (r1.bottom() <= r0.bottom())) return true;
	return false;
}

//-----------------------------------------------------------------------------
inline QRectF UnitedRect(QRectF& r0, QRectF& r1)
{
	QRectF r(r0);
	if (r1.left() < r.left()) r.setLeft(r1.left());
	if (r1.right() > r.right()) r.setRight(r1.right());
	if (r1.top() < r.top()) r.setTop(r1.top());
	if (r1.bottom() > r.bottom()) r.setBottom(r1.bottom());
	return r;
}

//-----------------------------------------------------------------------------
void CPlotWidget::fitToData(bool downSize)
{
	if (m_data.m_data.empty()) return;

	QRectF r = m_data.m_data[0]->boundRect();
	for (int i = 1; i<(int)m_data.m_data.size(); ++i)
	{
		QRectF ri = m_data.m_data[i]->boundRect();
		r = rectUnion(r, ri);
	}

	if (r.height() == 0.0)
	{
		double y = r.top();
		if (y == 0.0) r = QRectF(r.left(), 0.0, r.width(), 1.0);
		else if (y > 0) r = QRectF(r.left(), 0, r.width(), y);
		else if (y < 0) r = QRectF(r.left(), y, r.width(), -y);
	}

	if (downSize == false)
	{
		// only update the rect if the new rect does not fit in the old rect
		if (ContainsRect(m_viewRect, r) == false)
		{
			setViewRect(UnitedRect(m_viewRect, r));
		}
	}
	else setViewRect(r);
}

//-----------------------------------------------------------------------------
void CPlotWidget::setViewRect(const QRectF& rt)
{
	m_viewRect = rt;
	if (fabs(rt.height()) < 1e-30) m_viewRect.setHeight(1.0);
	if (fabs(rt.width()) < 1e-30) m_viewRect.setWidth(1.0);

	/*	double dx = 0.05*m_viewRect.width();
	double dy = 0.05*m_viewRect.height();
	m_viewRect.adjust(0.0, 0.0, dx, dy);
	*/
	m_xscale = findScale(m_viewRect.left(), m_viewRect.right());
	m_yscale = findScale(m_viewRect.top(), m_viewRect.bottom());
}

//-----------------------------------------------------------------------------
void CPlotWidget::fitToRect(const QRect& rt)
{
	QPoint p0(rt.topLeft());
	QPoint p1(rt.bottomRight());

	QPointF r0 = ScreenToView(p0);
	QPointF r1 = ScreenToView(p1);
	QRectF rf = QRectF(r0.x(), r1.y(), r1.x() - r0.x(), r0.y() - r1.y());

	setViewRect(rf);
}

//-----------------------------------------------------------------------------
void CPlotWidget::mousePressEvent(QMouseEvent* ev)
{
	m_mousePos = ev->pos();
	m_mouseInitPos = m_mousePos;

	m_bdragging = false;
	m_bregionSelect = false;

	bool bshift = ev->modifiers() & Qt::ShiftModifier;

	if (ev->button() == Qt::LeftButton)
	{
		// first, see if a point is selected
		QPoint pt = ev->pos();
		const int eps = 3 * devicePixelRatio();

		m_newSelect = false;
		for (int i = 0; i < (int)m_data.m_data.size(); ++i)
		{
			CPlotData& plot = *m_data.m_data[i];
			for (int j = 0; j < plot.size(); ++j)
			{
				QPointF& rj = plot.Point(j);
				QPointF p = ViewToScreen(rj);
				if ((fabs(p.x() - pt.x()) <= eps) && (fabs(p.y() - pt.y()) <= eps))
				{
					// see if this point is selected
					if (isSelected(i, j) == false)
					{
						if (bshift == false) m_selection.clear();
						addToSelection(i, j);
						emit pointSelected(j);
					}
					m_newSelect = true;
					break;
				}
			}
			if (m_newSelect) break;
		}

		// if no new selection was added and shift is not pressed, clear the selection
		if ((bshift == false) && (m_newSelect == false)) m_selection.clear();
	}

	ev->accept();
}

//-----------------------------------------------------------------------------
void CPlotWidget::mouseMoveEvent(QMouseEvent* ev)
{
	if (ev->buttons() & Qt::LeftButton)
	{
		QPoint p = ev->pos();

		bool bshift = ev->modifiers() & Qt::ShiftModifier;
		if (bshift && (m_bregionSelect == false)) m_bregionSelect = true;

		if (m_bregionSelect)
		{
			m_mousePos = p;
			repaint();
		}
		else
		{
			if (m_newSelect)
			{
				if (m_bdragging == false)
				{
					m_bdragging = true;
					emit draggingStart(m_mouseInitPos);
				}

				emit pointDragged(p);
			}
			else
			{
				if (m_bviewLocked == false)
				{
					QPointF r0 = ScreenToView(m_mousePos);
					QPointF r1 = ScreenToView(p);
					m_viewRect.translate(r0.x() - r1.x(), r0.y() - r1.y());
				}
				m_mousePos = p;
				repaint();
			}
		}
	}
	ev->accept();
}

//-----------------------------------------------------------------------------
void CPlotWidget::mouseReleaseEvent(QMouseEvent* ev)
{
	int X0 = m_mouseInitPos.x();
	int Y0 = m_mouseInitPos.y();
	int X1 = m_mousePos.x();
	int Y1 = m_mousePos.y();
	if (X1 < X0) { X0 ^= X1; X1 ^= X0; X0 ^= X1; }
	if (Y1 < Y0) { Y0 ^= Y1; Y1 ^= Y0; Y0 ^= Y1; }

	if ((X0 == X1) && (Y0 == Y1)) m_bregionSelect = false;

	if (m_bregionSelect)
	{
		QRect rt(X0, Y0, X1 - X0 + 1, Y1 - Y0 + 1);
		emit regionSelected(rt);

		m_bregionSelect = false;

		repaint();
	}
	else
	{
		if ((X0 == X1) && (Y0 == Y1))
		{
			if (ev->button() == Qt::LeftButton)
			{
				if (m_newSelect == false)
					emit pointSelected(-1);

				if (m_newSelect == false)
				{
					QPointF fp = ScreenToView(m_mousePos);
					emit pointClicked(fp, ev->modifiers() & Qt::ShiftModifier);
				}

				repaint();
			}
		}

		if (m_bdragging)
		{
			emit draggingEnd(ev->pos());
			m_bdragging = false;
		}
	}
	ev->accept();
}

//-----------------------------------------------------------------------------
void CPlotWidget::wheelEvent(QWheelEvent* ev)
{
	if (m_bviewLocked == false)
	{
		double W = m_viewRect.width();
		double H = m_viewRect.height();
		double dx = W*0.05;
		double dy = H*0.05;

		bool bctrl  = (ev->modifiers() & Qt::ControlModifier ? true : false);
		bool bshift = (ev->modifiers() & Qt::ShiftModifier ? true : false);
		if (bctrl && !bshift) dy = 0;
		if (bctrl && bshift) dx = 0;

		if ((ev->pixelDelta().y() < 0) || (ev->angleDelta().y() < 0))
		{
			m_viewRect.adjust(-dx, -dy, dx, dy);
		}
		else if ((ev->pixelDelta().y() > 0) || (ev->angleDelta().y() > 0))
		{
			m_viewRect.adjust(dx, dy, -dx, -dy);
		}

		m_xscale = findScale(m_viewRect.left(), m_viewRect.right());
		m_yscale = findScale(m_viewRect.top(), m_viewRect.bottom());
		repaint();
		ev->accept();
	}
}

//-----------------------------------------------------------------------------
void CPlotWidget::regionSelect(QRect rt)
{
	m_newSelect = false;
//	m_selection.clear();
	for (int i = 0; i < (int)m_data.m_data.size(); ++i)
	{
		CPlotData& plot = *m_data.m_data[i];
		for (int j = 0; j < plot.size(); ++j)
		{
			QPointF& rj = plot.Point(j);
			QPointF p = ViewToScreen(rj);
			if (rt.contains(QPoint((int)p.x(), (int)p.y())))
			{
				addToSelection(i, j);
			}
		}
	}
}

//-----------------------------------------------------------------------------
QPointF CPlotWidget::ScreenToView(const QPointF& p)
{
	qreal x = m_viewRect.left  () + (m_viewRect.width ()*(p.x() - m_plotRect.left())/(m_plotRect.width ()));
	qreal y = m_viewRect.bottom() + (m_viewRect.height()*(m_plotRect.top() - p.y() )/(m_plotRect.height()));
	return QPointF(x, y);
}

//-----------------------------------------------------------------------------
QRectF CPlotWidget::ScreenToView(const QRect& rt)
{
	QPoint p0 = rt.topLeft();
	QPoint p1 = rt.bottomRight();

	QPointF r0 = ScreenToView(p0);
	QPointF r1 = ScreenToView(p1);

	return QRectF(r0, r1);
}

//-----------------------------------------------------------------------------
QPointF CPlotWidget::ViewToScreen(const QPointF& p)
{
	double x = ViewToScreenX(p.x());
	double y = ViewToScreenY(p.y());
	return QPointF(x, y);
}

//-----------------------------------------------------------------------------
double CPlotWidget::ViewToScreenX(double x) const
{
	return m_plotRect.left() + (m_plotRect.width() * (x - m_viewRect.left()) / (m_viewRect.width()));
}

//-----------------------------------------------------------------------------
double CPlotWidget::ViewToScreenY(double y) const
{
	return m_plotRect.top() - (m_plotRect.height() * (y - m_viewRect.bottom()) / (m_viewRect.height()));
}

//-----------------------------------------------------------------------------
void CPlotWidget::SetBackgroundImage(QImage* img)
{
	if (m_img) delete m_img;
	m_img = img;
}

//-----------------------------------------------------------------------------
void CPlotWidget::SetHighlightInterval(double rngMin, double rngMax)
{
	m_hlrng[0] = rngMin;
	m_hlrng[1] = rngMax;
	repaint();
}

//-----------------------------------------------------------------------------
void CPlotWidget::GetHighlightInterval(double& rngMin, double& rngMax)
{
	rngMin = m_hlrng[0];
	rngMax = m_hlrng[1];
}

//-----------------------------------------------------------------------------
void CPlotWidget::paintEvent(QPaintEvent* pe)
{
	// Process base event first
	QWidget::paintEvent(pe);

	// store the current rectangle
	m_screenRect = rect();
	if ((m_screenRect.width()==0)||
		(m_screenRect.height()==0)) return;

	// Create the painter class
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing, true);

	if (m_img)
		p.drawImage(m_screenRect, *m_img);
	else 
		// clear the background
		p.fillRect(m_screenRect, m_data.m_bgCol);

	if (m_hlrng[1] > m_hlrng[0])
	{
		int x0 = ViewToScreenX(m_hlrng[0]);
		int x1 = ViewToScreenX(m_hlrng[1]);

		if ((x0 > m_screenRect.left()) && (x0 < m_screenRect.right()))
		{
			QColor c = m_data.m_bgCol.darker();
			QRect rl = m_screenRect;
			rl.setRight(x0);
			p.fillRect(rl, c);
		}

		if ((x1 > m_screenRect.left()) && (x1 < m_screenRect.right()))
		{
			QColor c = m_data.m_bgCol.darker();
			QRect rr = m_screenRect;
			rr.setLeft(x1);
			p.fillRect(rr, c);
		}

		if ((m_screenRect.left() >= x1) || (m_screenRect.right() <= x0))
		{
			QColor c = m_data.m_bgCol.darker();
			p.fillRect(m_screenRect, c);
		}
	}

	// render the title
	QFont f("Times", 12, QFont::Bold);
	p.setFont(f);
	QFontMetrics fm(f);
	m_titleRect = m_screenRect;
	m_titleRect.setHeight(fm.height() + 10);
	if (m_data.m_bdrawTitle) drawTitle(p);

	// figure out some metrics
	fm = p.fontMetrics();
	int fontHeight = fm.height(); // height in pixels

	// adjust the screen rectangle where the data will be drawn
	m_plotRect = m_screenRect;
	if (m_bfullScreenMode == false)
	{
		m_plotRect.setTop(m_titleRect.bottom());
		m_plotRect.adjust(50, 0, -90, -2*fontHeight - 2);
		p.setPen(QPen(m_data.m_boxColor));
		p.setBrush(Qt::NoBrush);
		p.drawRect(m_plotRect);
	}

	// draw the grid
	if (m_data.m_bdrawGrid) drawGrid(p);

	// draw the grid axes
	drawAxes(p);

	// draw the axes tick marks
	drawAxesTicks(p);

	// draw the axes labels
	if (m_data.m_bdrawAxesLabels) drawAxesLabels(p);

	// draw the legend
	if (m_data.m_bshowLegend) drawLegend(p);

	// render the data
	p.setClipRect(m_plotRect);
	drawAllData(p);

	if (m_bregionSelect)
	{
		int l = m_data.m_bgCol.lightness();
		QColor penCol(l > 100 ? QColor(Qt::black) : QColor(Qt::white));
		QRect rt(m_mouseInitPos, m_mousePos);
		p.setBrush(Qt::NoBrush);
		p.setPen(QPen(penCol, 1, Qt::DashLine));
		p.drawRect(rt);
	}

	// render the selection
	if (m_selection.empty() == false) drawSelection(p);
}

void CPlotWidget::drawLegend(QPainter& p)
{
	int N = (int)m_data.m_data.size();
	if (N == 0) return;

	QRect legendRect = m_plotRect;
	legendRect.setLeft(m_plotRect.right());
	legendRect.setRight(rect().right());

	QFont f("Arial", 10);
	QFontMetrics fm(f);
	p.setFont(f);

	int fh = fm.height();
	int fa = fm.ascent();

	int X0 = legendRect.left() + 10;
	int LW = 25;
	int X1 = X0 + LW + 5;
	int YC = legendRect.center().y();
	int H = legendRect.height() - 10;
	int Y0 = YC - N/2*(fh + 2);
	int Y1 = Y0 + N*(fh + 2);

	// draw the lines
	for (int i=0; i<N; ++i)
	{
		CPlotData& plot = *m_data.m_data[i];
		p.setPen(QPen(plot.lineColor(), 2));
		int Y = Y0 + i*(Y1 - Y0)/N;
		p.drawLine(X0, Y, X0 + LW, Y);
	}

	// draw the text
	p.setPen(Qt::black);
	for (int i=0; i<N; ++i)
	{
		CPlotData& plot = *m_data.m_data[i];
		int Y = Y0 + i*(Y1 - Y0)/N;
		p.drawText(X1, Y + fa/3, plot.label());
	}
}

void CPlotWidget::drawSelection(QPainter& p)
{
	for (int i = 0; i < m_selection.size(); ++i)
	{
		Selection& si = m_selection[i];

		QPointF pf = dataPoint(si.ndataIndex, si.npointIndex);
		QPointF pt = ViewToScreen(pf);
		if (m_plotRect.contains(QPoint((int)pt.x(), (int)pt.y()), true))
		{
			if ((si.ndataIndex < 0) || (si.ndataIndex >= m_data.m_data.size())) return;

			p.setPen(Qt::black);
			p.setBrush(m_selCol);
			p.drawEllipse(pt, 5, 5);

			if (m_bshowToolTip && (m_selection.size() == 1))
			{
				const QString& label = m_data.m_data[si.ndataIndex]->label();

				QFont font = p.font();
				QFont boldFont = font; boldFont.setBold(true);

				QFontMetrics fm(font);
				QString sx = QString("X:%1").arg(pf.x());
				QString sy = QString("Y:%1").arg(pf.y());
				int wx = fm.horizontalAdvance(sx);
				int wy = fm.horizontalAdvance(sy);
				int d = 3;
				int W = (wx > wy ? wx : wy) + 2 * d;
				int H = 3 * fm.height() + 4 * d;
				if (W < H) W = H;
				int X = pt.x();
				int Y = pt.y();
				if (X + W > m_plotRect.right()) X = m_plotRect.right() - W;
				if (Y + H > m_plotRect.bottom()) Y = m_plotRect.bottom() - H;

				p.setBrush(Qt::yellow);
				p.drawRect(X, Y, W, H);

				p.setFont(boldFont);
				p.drawText(X + d, Y + fm.ascent() + d, label);
				p.setFont(font);
				p.drawText(X + d, Y + fm.ascent() + fm.height() + 2 * d, sx);
				p.drawText(X + d, Y + fm.ascent() + 2 * fm.height() + 3 * d, sy);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CPlotWidget::drawTitle(QPainter& p)
{
	QPen pen(Qt::black, 1);
	p.setPen(pen);
	QFont f("Times", 12, QFont::Bold);
	p.setFont(f);
	QFontMetrics fm(f);
	p.drawText(m_titleRect, Qt::AlignCenter, m_data.m_title);
}

//-----------------------------------------------------------------------------
void CPlotWidget::drawAxesLabels(QPainter& p)
{
	QFont f("Arial", 12);
	f.setBold(true);
	p.setFont(f);

	if (m_data.m_xAxis.labelVisible && (m_data.m_xAxis.label.isEmpty() == false))
	{
		p.drawText(m_screenRect, Qt::AlignHCenter | Qt::AlignBottom, m_data.m_xAxis.label);
	}

	if (m_data.m_yAxis.labelVisible && (m_data.m_yAxis.label.isEmpty() == false))
	{
		QRect rt(-m_screenRect.height(), 0, m_screenRect.height(), m_screenRect.width());
		p.save();
		p.rotate(-90);
		p.drawText(rt, Qt::AlignHCenter | Qt::AlignTop, m_data.m_yAxis.label);
		p.restore();
	}
}

//-----------------------------------------------------------------------------
void CPlotWidget::drawAxesTicks(QPainter& p)
{
	char sz[256] = { 0 };
	QFont f("Arial", 10);
	QFontMetrics fm(f);
	p.setFont(f);

	int x0 = m_plotRect.left();
	int x1 = m_plotRect.right();
	int y0 = m_plotRect.top();
	int y1 = m_plotRect.bottom();

	double xscale = m_xscale;
	double yscale = m_yscale;

	p.setPen(QPen(Qt::black, 1));

	// determine the y-scale
	double gy = 1;
	if (m_bscaleAxisLabels)
	{
		int nydiv = (int)log10(yscale);
		if (nydiv != 0)
		{
			gy = pow(10.0, nydiv);
			snprintf(sz, sizeof sz, "x 1e%03d", nydiv);
			p.setPen(QPen(m_data.m_yAxisCol));
			p.drawText(x0 - 30, y0 - fm.height() + fm.descent(), QString(sz));
		}
	}
	else if (m_customYAxisLabel.isEmpty() == false)
	{
		p.setPen(QPen(m_data.m_yAxisCol));
		p.drawText(x0 - 30, y0 - fm.height() + fm.descent(), m_customYAxisLabel);
	}

	// determine the x-scale
	double gx = 1;
	if (m_bscaleAxisLabels)
	{
		int nxdiv = (int)log10(xscale);
		if (nxdiv != 0)
		{
			gx = pow(10.0, nxdiv);
			snprintf(sz, sizeof sz, "x 1e%03d", nxdiv);
			p.setPen(QPen(m_data.m_xAxisCol));
			p.drawText(x1 + 5, y1, QString(sz));
		}
	}
	else if (m_customXAxisLabel.isEmpty() == false)
	{
		p.setPen(QPen(m_data.m_xAxisCol));
		p.drawText(x1 + 5, y1, m_customXAxisLabel);
	}

	// draw the y-labels
	if (m_data.m_yAxis.labelPosition != NONE)
	{
		p.setPen(QPen(m_data.m_yAxisTickCol));

		int xPos = 0;
		switch (m_data.m_yAxis.labelPosition)
		{
		case LOW: xPos = x0; break;
		case HIGH: xPos = x1; break;
		case NEXT_TO_AXIS:
			xPos = ViewToScreen(QPointF(0.,0.)).x();
			if (xPos < x0) xPos = x0; else if (xPos > x1) xPos = x1;
			break;
		}

		p.setPen(QPen(m_data.m_yAxisCol));

		double fy = yscale*(int)(m_viewRect.top() / yscale);
		while (fy < m_viewRect.bottom())
		{
			int iy = ViewToScreen(QPointF(0.0, fy)).y();
			if (iy < y1)
			{
				double g = fy / gy;
				if (fabs(g) < 1e-7) g = 0;
				snprintf(sz, sizeof sz, "%lg", g);
				QString s(sz);

				if (m_data.m_yAxis.labelAlignment == ALIGN_LABEL_LEFT)
				{
					int w = p.fontMetrics().horizontalAdvance(s);
					p.drawText(xPos - w - 5, iy + p.fontMetrics().height() / 3, s);
				}
				else
					p.drawText(xPos + 5, iy + p.fontMetrics().height() / 3, s);
			}
			fy += yscale;
		}
	}

	// draw the x-labels
	if (m_data.m_xAxis.labelPosition != NONE)
	{
		p.setPen(QPen(m_data.m_xAxisTickCol));

		int yPos = 0;
		switch (m_data.m_yAxis.labelPosition)
		{
		case LOW: yPos = y1; break;
		case HIGH: yPos = y0; break;
		case NEXT_TO_AXIS:
			yPos = ViewToScreen(QPointF(0., 0.)).y();
			if (yPos < y0) yPos = y0; else if (yPos > y1) yPos = y1;
			break;
		}

		p.setPen(QPen(m_data.m_xAxisCol));

		double fx = xscale*(int)(m_viewRect.left() / xscale);
		while (fx < m_viewRect.right())
		{
			int ix = ViewToScreen(QPointF(fx, 0.0)).x();
			if (ix > x0)
			{
				double g = fx / gx;
				if (fabs(g) < 1e-7) g = 0;
				snprintf(sz, sizeof sz, "%lg", g);
				QString s(sz);
				int w = p.fontMetrics().horizontalAdvance(s);
				if (m_data.m_xAxis.labelAlignment == ALIGN_LABEL_BOTTOM)
					p.drawText(ix - w / 2, yPos + p.fontMetrics().height(), s);
				else
					p.drawText(ix - w / 2, yPos - 5, s);
			}
			fx += xscale;
		}
	}
}

//-----------------------------------------------------------------------------
void CPlotWidget::drawGrid(QPainter& p)
{
	char sz[256] = {0};
	QFont f("Arial", 10);
	QFontMetrics fm(f);
	p.setFont(f);

	int x0 = m_plotRect.left();
	int x1 = m_plotRect.right();
	int y0 = m_plotRect.top();
	int y1 = m_plotRect.bottom();

	double xscale = m_xscale;
	double yscale = m_yscale;

	p.setPen(QPen(m_data.m_gridCol, 1));
	p.setRenderHint(QPainter::Antialiasing, false);

	// draw the y-grid lines
	if (m_data.m_bdrawYLines)
	{
		double fy = yscale*(int)(m_viewRect.top()/yscale);
		while (fy < m_viewRect.bottom())
		{
			int iy = ViewToScreen(QPointF(0.0, fy)).y();
			if (iy < y1)
			{
				QPainterPath path;
				path.moveTo(x0, iy);
				path.lineTo(x1-1, iy);
				p.drawPath(path);
			}
			fy += yscale;
		}
	}

	// draw the x-grid lines
	if (m_data.m_bdrawXLines)
	{
		double fx = xscale*(int)(m_viewRect.left() / xscale);
		while (fx < m_viewRect.right())
		{
			int ix = ViewToScreen(QPointF(fx, 0.0)).x();
			if (ix > x0)
			{
				QPainterPath path;
				path.moveTo(ix, y0);
				path.lineTo(ix, y1-1);
				p.drawPath(path);
			}
			fx += xscale;
		}
	}

	p.setRenderHint(QPainter::Antialiasing, true);
}

//-----------------------------------------------------------------------------
void CPlotWidget::drawAxes(QPainter& p)
{
	// get the center in screen coordinates
	QPointF c = ViewToScreen(QPointF(0.0, 0.0));

	// render the X-axis
	if (m_data.m_xAxis.visible)
	{
		if ((c.y() > m_plotRect.top   ()) && (c.y() < m_plotRect.bottom()))
		{
			p.setPen(QPen(m_data.m_xAxisCol, 2));
			QPainterPath xaxis;
			xaxis.moveTo(m_plotRect.left (), c.y());
			xaxis.lineTo(m_plotRect.right(), c.y());
			p.drawPath(xaxis);
		}
	}

	// render the Y-axis
	if (m_data.m_yAxis.visible)
	{
		if ((c.x() > m_plotRect.left ()) && (c.x() < m_plotRect.right()))
		{
			p.setPen(QPen(m_data.m_yAxisCol, 2));
			QPainterPath yaxis;
			yaxis.moveTo(c.x(), m_plotRect.top   ());
			yaxis.lineTo(c.x(), m_plotRect.bottom());
			p.drawPath(yaxis);
		}
	}
}

//-----------------------------------------------------------------------------
void CPlotWidget::drawAllData(QPainter& p)
{
	p.setRenderHint(QPainter::Antialiasing, m_data.m_bsmoothLines);

	int N = (int)m_data.m_data.size();
	for (int i=0; i<N; ++i)
	{
		DrawPlotData(p, *m_data.m_data[i]);
	}

	p.setRenderHint(QPainter::Antialiasing, true);
}

void CPlotWidget::DrawPlotData(QPainter& p, CPlotData& data)
{
	switch (m_chartStyle)
	{
	case LINECHART_PLOT: draw_linechart(p, data); break;
	case BARCHART_PLOT : draw_barchart(p, data); break;
	case DENSITY_PLOT  : draw_densityplot(p, data); break;
	}
}

//=============================================================================
void CPlotWidget::draw_linechart(QPainter& p, CPlotData& data)
{
	int N = data.size();
	if (N == 0) return;

	QColor col = data.lineColor();
	QPen pen(col, data.lineWidth());
	p.setPen(pen);

	QPointF p0 = ViewToScreen(data.Point(0)), p1(p0);
	p.setBrush(Qt::NoBrush);
	for (int i = 1; i<N; ++i)
	{
		p1 = ViewToScreen(data.Point(i));
		p.drawLine(p0, p1);
		p0 = p1;
	}

	int n = data.markerSize();
	int n2 = n / 2 + 1;

	// draw the marks
	if (data.markerType() > 0)
	{
		p.setBrush(data.fillColor());
		for (int i = 0; i<N; ++i)
		{
			p1 = ViewToScreen(data.Point(i));
			drawMarker(p, p1, data.markerSize(), data.markerType());
		}
	}
}

//-----------------------------------------------------------------------------
void CPlotWidget::draw_barchart(QPainter& p, CPlotData& data)
{
	int N = data.size();
	if (N == 0) return;

	int W = m_plotRect.width();
	int w = W / N;
	if (w < 1) w = 1;

	p.setPen(Qt::NoPen);
	p.setBrush(data.fillColor());
	for (int i = 0; i<N; ++i)
	{
		QPointF& pi = data.Point(i);
		QPointF p0 = ViewToScreen(pi);
		QPointF p1 = ViewToScreen(QPointF(pi.x(), 0.0));
		QRect r(p0.x() - w/2, p0.y(), w, p1.y() - p0.y());
		p.drawRect(r);
	}
}

void CPlotWidget::draw_densityplot(QPainter& p, CPlotData& data)
{
	int W = m_plotRect.width();
	int H = m_plotRect.height();
	QRect rt = m_plotRect;

	int NX = W / 5;
	int NY = H / 5;
	matrix m(NY, NX); m.zero();
	int N = data.size();
	double dmax = 0.0;
	for (int n = 0; n < N; ++n)
	{
		QPointF& pi = data.Point(n);
		QPointF p0 = ViewToScreen(pi);
		int i = (NY* (p0.y() - rt.top())) / H;
		int j = (NX* (p0.x() - rt.left())) / W;
		if ((i >= 0) && (i < NY) && (j >= 0) && (j < NX))
		{
			m[i][j] += 1.0;
			if (m[i][j] > dmax) dmax = m[i][j];
		}
	}
	if (dmax == 0) return;
	dmax = log(dmax);
	if (dmax == 0) dmax = 1;

	QColor c = data.fillColor();

	p.setPen(Qt::NoPen);
	for (int i=0; i<NY; ++i)
		for (int j = 0; j < NX; ++j)
		{
			if (m[i][j] > 0)
			{
				int x0 = rt.left() + (j * W) / NX;
				int x1 = rt.left() + ((j + 1) * W) / NX;

				int y0 = rt.top() + (i * H) / NY;
				int y1 = rt.top() + ((i + 1) * H) / NY;

				double w = log(m[i][j]) / dmax;
				double a = 0.1 + w * 0.9;
				c.setAlphaF(a);
				p.setBrush(c);

				QRect r(x0, y0, x1 - x0, y1 - y0);
				p.drawRect(r);
			}
		}
}

//-----------------------------------------------------------------------------
bool CPlotWidget::Save(const QString& fileName)
{
	// dump the data to a text file
	std::string sfile = fileName.toStdString();
	FILE* fp = fopen(sfile.c_str(), "wt");
	if (fp == 0) return false;

	std::string title = m_data.m_title.toStdString();
	fprintf(fp, "#Data : %s\n", title.c_str());
	fprintf(fp,"\n");

	CPlotData& plot = getPlotData(0);
	for (int i=0; i<plot.size(); i++)
	{
		fprintf(fp, "%16.9g ", plot.Point(i).x());
		for (int j=0; j<plots(); j++)
		{
			CPlotData& plotj = getPlotData(j);
			fprintf(fp,"%16.9g ", plotj.Point(i).y());
		}
		fprintf(fp,"\n");
	}
	fclose(fp);
	return true;
}

//-----------------------------------------------------------------------------
QPointF CPlotWidget::SnapToGrid(const QPointF& p)
{
	double fx = p.x();
	double fy = p.y();

	if (fx >= 0) fx = m_xscale*int((fx / m_xscale) + 0.5);
	else fx = m_xscale*int((fx / m_xscale));

	if (fy >= 0) fy = m_yscale*int((fy / m_yscale) + 0.5);
	else fy = m_yscale*int((fy / m_yscale));

	return QPointF(fx, fy);
}

//-----------------------------------------------------------------------------
void CPlotWidget::setXAxisLabelAlignment(AxisLabelAlignment a)
{
	m_data.m_xAxis.labelAlignment = a;
}

//-----------------------------------------------------------------------------
void CPlotWidget::setYAxisLabelAlignment(AxisLabelAlignment a)
{
	m_data.m_yAxis.labelAlignment = a;
}

//-----------------------------------------------------------------------------
void CPlotWidget::selectPoint(int ndata, int npoint)
{
	m_selection.clear();
	m_selection.push_back(Selection{ ndata, npoint });
	repaint();

	emit pointSelected(npoint);
}

void CPlotWidget::clearSelection()
{
	m_selection.clear();
}

bool CPlotWidget::addToSelection(int ndata, int npoint)
{
	Selection s{ ndata, npoint };
	for (int i = 0; i < m_selection.size(); ++i)
	{
		Selection& si = m_selection[i];
		if ((s.ndataIndex == si.ndataIndex) && 
			(s.npointIndex == si.npointIndex)) return false;
	}
	m_selection.push_back(s);
	return true;
}

// see if a point is selected
bool CPlotWidget::isSelected(int ndata, int npoint)
{
	for (int i = 0; i < m_selection.size(); ++i)
	{
		Selection& si = m_selection[i];
		if ((si.ndataIndex == ndata) &&
			(si.npointIndex == npoint)) return true;
	}
	return false;
}

std::vector<QPointF> CPlotWidget::SelectedPoints() const
{
	vector<CPlotWidget::Selection> sel = selection();
	vector<QPointF> pt;
	for (CPlotWidget::Selection& si : sel)
	{
		QPointF pi = m_data.m_data[si.ndataIndex]->Point(si.npointIndex);
		pt.push_back(pi);
	}
	return pt;
}

CCurvePlotWidget::CCurvePlotWidget(QWidget* parent) : CPlotWidget(parent)
{
	m_lc = nullptr;
	m_showDeriv = false;
	setLineSmoothing(true);

	showLegend(false);
	showToolTip(false);
	scaleAxisLabels(false);
	setFullScreenMode(true);
	setXAxisLabelAlignment(ALIGN_LABEL_TOP);
	setYAxisLabelAlignment(ALIGN_LABEL_RIGHT);
	setBackgroundColor(QColor(48, 48, 48));
	setGridColor(QColor(128, 128, 128));
	setXAxisColor(QColor(255, 255, 255));
	setYAxisColor(QColor(255, 255, 255));
	setXAxisTickColor(QColor(255, 255, 255));
	setYAxisTickColor(QColor(255, 255, 255));
	setSelectionColor(QColor(255, 255, 192));
}

void CCurvePlotWidget::SetLoadCurve(LoadCurve* lc) 
{ 
	m_lc = lc; 

	clear();

	if (lc)
	{
		CPlotData* data = new CPlotData;
		for (int i = 0; i < lc->Points(); ++i)
		{
			vec2d pt = lc->Point(i);
			data->addPoint(pt.x(), pt.y());
		}
		addPlotData(data);

		data->setLineColor(QColor(92, 255, 164));
		data->setFillColor(QColor(92, 255, 164));
		data->setLineWidth(2);
		data->setMarkerSize(5);
		repaint();
	}
}

void CCurvePlotWidget::ShowDeriv(bool b)
{
	m_showDeriv = b;
	update();
}

LoadCurve* CCurvePlotWidget::GetLoadCurve()
{
	return m_lc;
}

void CCurvePlotWidget::DrawPlotData(QPainter& painter, CPlotData& data)
{
	if (m_lc == 0) return;
    m_lc->Update();

	int N = data.size();
	QRect rt = ScreenRect();

	// draw derivative
	if (m_showDeriv)
	{
		QColor c = data.lineColor().darker();
		painter.setPen(QPen(c, data.lineWidth()));
		QPointF p0, p1;
		for (int i = rt.left(); i < rt.right(); i += 2)
		{
			p1.setX(i);
			QPointF p = ScreenToView(p1);
			p.setY(m_lc->derive(p.x()));
			p1 = ViewToScreen(p);

			if (i != rt.left())
			{
				painter.drawLine(p0, p1);
			}
			p0 = p1;
		}
	}

	// draw the line
	painter.setPen(QPen(data.lineColor(), data.lineWidth()));
	QPointF p0, p1;
	for (int i = rt.left(); i < rt.right(); i += 2)
	{
		p1.setX(i);
		QPointF p = ScreenToView(p1);
		p.setY(m_lc->Value(p.x()));
		p1 = ViewToScreen(p);

		if (i != rt.left())
		{
			painter.drawLine(p0, p1);
		}

		p0 = p1;
	}

	// draw the marks
	if (data.markerType() > 0)
	{
		painter.setBrush(data.fillColor());
		for (int i = 0; i < N; ++i)
		{
			p1 = ViewToScreen(data.Point(i));
			QRect r(p1.x() - 2, p1.y() - 2, 5, 5);
			painter.drawRect(r);
		}
	}
}

//=============================================================================
class CCmdAddPoint : public CCommand
{
public:
	CCmdAddPoint(LoadCurve* plc, const vec2d& p);

	void Execute() override;
	void UnExecute() override;

	int Index() { return m_index; }

private:
	LoadCurve*	m_lc;
	vec2d		m_pt;
	int			m_index;
};

class CCmdRemovePoint : public CCommand
{
public:
	CCmdRemovePoint(LoadCurve* plc, const std::vector<int>& index);

	void Execute() override;
	void UnExecute() override;

private:
	LoadCurve* m_lc;
	LoadCurve		m_copy;
	std::vector<int>		m_index;
};

class CCmdMovePoint : public CCommand
{
public:
	CCmdMovePoint(LoadCurve* plc, int index, vec2d to);

	void Execute() override;
	void UnExecute() override;

private:
	LoadCurve*	m_lc;
	vec2d		m_p;
	int			m_index;
};

class CCmdDeleteCurve : public CCommand
{
public:
	CCmdDeleteCurve(Param* pp);
	~CCmdDeleteCurve();
	void Execute() override;
	void UnExecute() override;
private:
	int			m_lc;
	Param* m_pp;
};


CCmdAddPoint::CCmdAddPoint(LoadCurve* plc, const vec2d& pt) : CCommand("Add point")
{
	m_lc = plc;
	m_pt = pt;
	m_index = -1;
}

void CCmdAddPoint::Execute()
{
	m_index = m_lc->Add(m_pt);
	m_lc->Update();
}

void CCmdAddPoint::UnExecute()
{
	m_lc->Delete(m_index);
	m_lc->Update();
}

CCmdRemovePoint::CCmdRemovePoint(LoadCurve* plc, const vector<int>& index) : CCommand("Remove point")
{
	m_lc = plc;
	m_index = index;
}

void CCmdRemovePoint::Execute()
{
	m_copy = *m_lc;
	m_lc->Delete(m_index);
	m_lc->Update();
}

void CCmdRemovePoint::UnExecute()
{
	*m_lc = m_copy;
	m_lc->Update();
}

CCmdMovePoint::CCmdMovePoint(LoadCurve* plc, int index, vec2d to) : CCommand("Move point")
{
	m_lc = plc;
	m_index = index;
	m_p = to;
	m_lc->Update();
}

void CCmdMovePoint::Execute()
{
	vec2d tmp = m_lc->Point(m_index);
	m_lc->SetPoint(m_index, m_p);
	m_p = tmp;
	m_lc->Update();
}

void CCmdMovePoint::UnExecute()
{
	Execute();
}

CCmdDeleteCurve::CCmdDeleteCurve(Param* pp) : CCommand("Delete Curve")
{
	m_lc = -1;
	m_pp = pp; assert(m_pp);
}

CCmdDeleteCurve::~CCmdDeleteCurve()
{
}

void CCmdDeleteCurve::Execute()
{
	m_lc = m_pp->GetLoadCurveID();
	m_pp->SetLoadCurveID(-1);
}

void CCmdDeleteCurve::UnExecute()
{
	m_pp->SetLoadCurveID(m_lc);
}

//=============================================================================
class UICurveEditWidget
{
public:
	QComboBox* lineType;
	QComboBox* extendMode;
	QToolButton* undo;
	QToolButton* redo;
	QToolButton* math;
	QToolButton* copy;
	QToolButton* paste;
	QToolButton* open;
	QToolButton* save;

	CCurvePlotWidget* plt;

	QLineEdit* xval;
	QLineEdit* yval;
	QToolButton* addPoint;
	QToolButton* snap2grid;
	QHBoxLayout* pltbutton;
	QToolButton* map2rect;
	QToolButton* showDeriv;

public:
	QPointF					m_dragPt;
	int						m_dragIndex;
	std::vector<QPointF>	m_p0;	// used by point dragging
	QString				m_clipboard_backup;

public:
	void setup(QWidget* w)
	{
		// toolbar
		lineType = new QComboBox; lineType->setObjectName("lineType");
		lineType->addItem("Linear");
		lineType->addItem("Step");
		lineType->addItem("Smooth");
		lineType->addItem("Cubic spline");
		lineType->addItem("Control points");
		lineType->addItem("Approximation");
		lineType->addItem("Smooth step");
        lineType->addItem("C2-smooth");

		extendMode = new QComboBox; extendMode->setObjectName("extendMode");
		extendMode->addItem("Constant");
		extendMode->addItem("Extrapolate");
		extendMode->addItem("Repeat");
		extendMode->addItem("Repeat Offset");

		undo = new QToolButton; undo->setObjectName("undo");
		undo->setIcon(QIcon(":/icons/undo.png"));

		redo = new QToolButton; redo->setObjectName("redo");
		redo->setIcon(QIcon(":/icons/redo.png"));

		math = new QToolButton; math->setObjectName("math");
		math->setIcon(CResource::Icon("formula"));

		copy = new QToolButton; copy->setObjectName("copy");
		copy->setIcon(QIcon(":/icons/clipboard.png"));

		paste = new QToolButton; paste->setObjectName("paste");
		paste->setIcon(QIcon(":/icons/paste.png"));

		open = new QToolButton; open->setObjectName("open");
		open->setIcon(QIcon(":/icons/open.png"));

		save = new QToolButton; save->setObjectName("save");
		save->setIcon(QIcon(":/icons/save.png"));

		QHBoxLayout* curveLayout = new QHBoxLayout;
		curveLayout->addWidget(new QLabel("Type:"));
		curveLayout->addWidget(lineType);
		curveLayout->addWidget(new QLabel("Extend:"));
		curveLayout->addWidget(extendMode);
		curveLayout->addWidget(undo);
		curveLayout->addWidget(redo);
		curveLayout->addWidget(math);
		curveLayout->addWidget(copy);
		curveLayout->addWidget(paste);
		curveLayout->addWidget(open);
		curveLayout->addWidget(save);
		curveLayout->addStretch();

		// plot widget
		plt = new CCurvePlotWidget; plt->setObjectName("plot");

		// bottom toolbar
		xval = new QLineEdit; xval->setObjectName("xval");
		yval = new QLineEdit; yval->setObjectName("yval");

		addPoint = new QToolButton;
		addPoint->setAutoRaise(true);
		addPoint->setIcon(QIcon(":/icons/newpoint.png"));
		addPoint->setCheckable(true);
		addPoint->setChecked(false);
		addPoint->setToolTip("<font color=\"black\">Add a new point");

		QToolButton* delPoint = new QToolButton; delPoint->setObjectName("deletePoint");
		delPoint->setAutoRaise(true);
		delPoint->setIcon(QIcon(":/icons/deletepoint.png"));
		delPoint->setToolTip("<font color=\"black\">Delete a point");

		snap2grid = new QToolButton;
		snap2grid->setAutoRaise(true);
		snap2grid->setIcon(QIcon(":/icons/snaptogrid.png"));
		snap2grid->setCheckable(true);
		snap2grid->setChecked(false);
		snap2grid->setToolTip("<font color=\"black\">Snap to grid");

		QToolButton* zoom = new QToolButton; zoom->setObjectName("zoomToFit");
		zoom->setAutoRaise(true);
		zoom->setIcon(QIcon(":/icons/zoom_fit.png"));
		zoom->setToolTip("<font color=\"black\">Zoom to fit");

		QToolButton* zoomx = new QToolButton; zoomx->setObjectName("zoomX");
		zoomx->setAutoRaise(true);
		zoomx->setIcon(QIcon(":/icons/zoom_x.png"));
		zoomx->setToolTip("<font color=\"black\">Zoom X extents");

		QToolButton* zoomy = new QToolButton; zoomy->setObjectName("zoomY");
		zoomy->setAutoRaise(true);
		zoomy->setIcon(QIcon(":/icons/zoom_y.png"));
		zoomy->setToolTip("<font color=\"black\">Zoom Y extents");

		map2rect = new QToolButton;
		map2rect->setAutoRaise(true);
		map2rect->setCheckable(true);
		map2rect->setChecked(false);
		map2rect->setIcon(QIcon(":/icons/zoom-fit-best-2.png"));
		map2rect->setToolTip("<font color=\"black\">Map to rectangle");

		showDeriv = new QToolButton;
		showDeriv->setObjectName("showDeriv");
		showDeriv->setAutoRaise(true);
		showDeriv->setCheckable(true);
		showDeriv->setChecked(false);
		showDeriv->setIcon(CIconProvider::GetIcon("deriv"));
		showDeriv->setToolTip("<font color=\"black\">Display derivative");

		QToolButton* clear = new QToolButton; clear->setObjectName("clear");
		clear->setAutoRaise(true);
		clear->setIcon(CIconProvider::GetIcon("delete"));
		clear->setToolTip("<font color=\"black\">Clear the curve");

		pltbutton = new QHBoxLayout;
		pltbutton->addWidget(xval);
		pltbutton->addWidget(yval);
		pltbutton->addWidget(addPoint);
		pltbutton->addWidget(delPoint);
		pltbutton->addWidget(snap2grid);
		pltbutton->addWidget(zoomx);
		pltbutton->addWidget(zoomy);
		pltbutton->addWidget(zoom);
		pltbutton->addWidget(map2rect);
		pltbutton->addWidget(clear);
		pltbutton->addWidget(showDeriv);
		pltbutton->addStretch();
		pltbutton->setSpacing(2);

		// main layout
		QVBoxLayout* h = new QVBoxLayout;
		h->addLayout(curveLayout);
		h->addWidget(plt);
		h->addLayout(pltbutton);

		w->setLayout(h);

		QMetaObject::connectSlotsByName(w);
	}

	void enablePointEdit(bool b)
	{
		if (xval->isEnabled() != b)
		{
			if (b == false) xval->setText("");
			xval->setEnabled(b);
		}

		if (yval->isEnabled() != b)
		{
			if (b == false) yval->setText("");
			yval->setEnabled(b);
		}
	}

	void setPointValues(double x, double y)
	{
		xval->setText(QString("%1").arg(x));
		yval->setText(QString("%1").arg(y));
	}

	bool isAddPointChecked()
	{
		return addPoint->isChecked();
	}

	bool isSnapToGrid()
	{
		return snap2grid->isChecked();
	}

	QPointF getPointValue()
	{
		return QPointF(xval->text().toDouble(), yval->text().toDouble());
	}

	void setCurveType(int line, int extend)
	{
		lineType->setCurrentIndex(line);
		extendMode->setCurrentIndex(extend);
	}

	void setCmdNames(const QString& undotxt, const QString& redotxt)
	{
		undo->setToolTip(QString("<font color=\"black\">") + undotxt);
		redo->setToolTip(QString("<font color=\"black\">") + redotxt);
	}
};

CCurveEditWidget::CCurveEditWidget(QWidget* parent) : QWidget(parent), ui(new UICurveEditWidget)
{
	ui->setup(this);
}

void CCurveEditWidget::Clear()
{
	ui->plt->clearData();
}

void CCurveEditWidget::SetLoadCurve(LoadCurve* lc)
{
	m_cmd.Clear();
	ui->plt->clear();
	ui->plt->SetLoadCurve(lc);

	if (lc)
	{
		ui->setCurveType(lc->GetInterpolator(), lc->GetExtendMode());
	}
}

void CCurveEditWidget::on_plot_pointClicked(QPointF p, bool shift)
{
	LoadCurve* lc = ui->plt->GetLoadCurve();
	if (lc == nullptr) return;

	if ((ui->isAddPointChecked() || shift))
	{
		if (ui->isSnapToGrid()) p = ui->plt->SnapToGrid(p);

		vec2d pt(p.x(), p.y());

		CCmdAddPoint* cmd = new CCmdAddPoint(lc, pt);
		m_cmd.DoCommand(cmd);

		UpdatePlotData();
		ui->plt->selectPoint(0, cmd->Index());

		emit dataChanged();
	}
}

void CCurveEditWidget::UpdateSelection()
{
	LoadCurve* plc = ui->plt->GetLoadCurve();
	if (plc == nullptr)
	{
		ui->enablePointEdit(false);
		return;
	}

	vector<CPlotWidget::Selection> sel = ui->plt->selection();

	if (sel.size() == 1)
	{
		ui->enablePointEdit(true);
		vec2d pt = plc->Point(sel[0].npointIndex);
		ui->setPointValues(pt.x(), pt.y());
	}
	else
	{
		ui->enablePointEdit(false);
	}
}

void CCurveEditWidget::on_plot_pointDragged(QPoint p)
{
	LoadCurve* plc = ui->plt->GetLoadCurve();
	if (plc == nullptr) return;

	vector<CPlotWidget::Selection> sel = ui->plt->selection();
	if (sel.size() == 0) return;

	QPointF pf = ui->plt->ScreenToView(p);
	double dx = pf.x() - ui->m_dragPt.x();
	double dy = pf.y() - ui->m_dragPt.y();

	if ((ui->m_dragIndex >= 0) && (ui->isSnapToGrid()))
	{
		QPointF p0 = ui->m_p0[ui->m_dragIndex];
		QPointF pi(p0.x() + dx, p0.y() + dy);
		pi = ui->plt->SnapToGrid(pi);
		dx = pi.x() - p0.x();
		dy = pi.y() - p0.y();
	}

	for (int i = 0; i < sel.size(); ++i)
	{
		int n = sel[i].npointIndex;

		QPointF p0 = ui->m_p0[i];
		QPointF pi(p0.x() + dx, p0.y() + dy);

		plc->SetPoint(n, pi.x(), pi.y());

		ui->plt->getPlotData(0).Point(n) = pi;

		if (sel.size() == 1) ui->setPointValues(pi.x(), pi.y());
	}
	plc->Update();
	ui->plt->repaint();
}

void CCurveEditWidget::on_plot_draggingStart(QPoint p)
{
	LoadCurve* plc = ui->plt->GetLoadCurve();
	if (plc == nullptr) return;

	ui->m_dragPt = ui->plt->ScreenToView(p);

	vector<CPlotWidget::Selection> sel = ui->plt->selection();
	ui->m_dragIndex = -1;
	ui->m_p0.clear();
	if (sel.size() > 0)
	{
		ui->m_p0.resize(sel.size());
		for (int i = 0; i < sel.size(); ++i)
		{
			vec2d pt = plc->Point(sel[i].npointIndex);
			ui->m_p0[i].setX(pt.x());
			ui->m_p0[i].setY(pt.y());

			QPointF pf(pt.x(), pt.y());
			QPointF pi = ui->plt->ViewToScreen(pf);

			double dx = fabs(pi.x() - p.x());
			double dy = fabs(pi.y() - p.y());
			if ((dx <= 5) && (dy <= 5)) ui->m_dragIndex = i;
		}
	}
}

void CCurveEditWidget::on_plot_draggingEnd(QPoint p)
{
	LoadCurve* plc = ui->plt->GetLoadCurve();
	if (plc == nullptr) return;

	vector<CPlotWidget::Selection> sel = ui->plt->selection();

	if (sel.size() == 1)
	{
		int n = sel[0].npointIndex;

		QPointF p0 = ui->m_p0[0];
		vec2d lp = plc->Point(n);
		plc->SetPoint(n, p0.x(), p0.y());

		m_cmd.DoCommand(new CCmdMovePoint(plc, n, lp));

		UpdatePlotData();

		emit dataChanged();
	}
}

void CCurveEditWidget::UpdatePlotData()
{
	LoadCurve* plc = ui->plt->GetLoadCurve();
	if (plc == nullptr) return;

	ui->plt->clearSelection();

	CPlotData& data = ui->plt->getPlotData(0);
	data.clear();
	for (int i = 0; i < plc->Points(); ++i)
	{
		vec2d pi = plc->Point(i);
		data.addPoint(pi.x(), pi.y());
	}
	ui->plt->repaint();
}

void CCurveEditWidget::on_plot_pointSelected(int n)
{
	UpdateSelection();
}

void CCurveEditWidget::on_plot_backgroundImageChanged()
{
	QColor c = QColor(255, 255, 255);
	if (ui->plt->HasBackgroundImage()) c = QColor(255, 255, 255);

	ui->plt->setXAxisColor(c);
	ui->plt->setYAxisColor(c);
	ui->plt->setXAxisTickColor(c);
	ui->plt->setYAxisTickColor(c);
}

void CCurveEditWidget::on_plot_doneZoomToRect()
{

}

void CCurveEditWidget::on_plot_regionSelected(QRect rt)
{
	if (ui->map2rect->isChecked())
	{
		CDlgPlotWidgetProps dlg;
		if (dlg.exec())
		{
			ui->plt->mapToUserRect(rt, QRectF(dlg.m_xmin, dlg.m_ymin, dlg.m_xmax - dlg.m_xmin, dlg.m_ymax - dlg.m_ymin));
		}
		ui->map2rect->setChecked(false);
	}
	else
	{
		ui->plt->regionSelect(rt);
		UpdateSelection();
	}
}

void CCurveEditWidget::on_xval_textEdited()
{
	LoadCurve* plc = ui->plt->GetLoadCurve();
	if (plc == nullptr) return;

	vector<CPlotWidget::Selection> sel = ui->plt->selection();

	if (sel.size() == 1)
	{
		QPointF p = ui->getPointValue();

		CPlotData& data = ui->plt->getPlotData(0);
		data.Point(sel[0].npointIndex) = p;
		plc->SetPoint(sel[0].npointIndex, p.x(), p.y());
		ui->plt->repaint();

		emit dataChanged();
	}
}

void CCurveEditWidget::on_yval_textEdited()
{
	LoadCurve* plc = ui->plt->GetLoadCurve();
	if (plc == nullptr) return;

	vector<CPlotWidget::Selection> sel = ui->plt->selection();

	if (sel.size() == 1)
	{
		QPointF p = ui->getPointValue();

		CPlotData& data = ui->plt->getPlotData(0);
		data.Point(sel[0].npointIndex) = p;
		plc->SetPoint(sel[0].npointIndex, p.x(), p.y());
		ui->plt->repaint();

		emit dataChanged();
	}
}

void CCurveEditWidget::on_deletePoint_clicked()
{
	LoadCurve* plc = ui->plt->GetLoadCurve();
	if (plc == nullptr) return;

	vector<CPlotWidget::Selection> sel = ui->plt->selection();

	if (sel.empty() == false)
	{
		vector<int> points;
		for (int i = 0; i < sel.size(); ++i) points.push_back(sel[i].npointIndex);

		m_cmd.DoCommand(new CCmdRemovePoint(plc, points));
		SetLoadCurve(plc);
		ui->enablePointEdit(false);

		emit dataChanged();
	}
}

void CCurveEditWidget::on_zoomToFit_clicked()
{
	ui->plt->OnZoomToFit();
}

void CCurveEditWidget::on_zoomX_clicked()
{
	ui->plt->OnZoomToWidth();
}

void CCurveEditWidget::on_zoomY_clicked()
{
	ui->plt->OnZoomToHeight();
}

void CCurveEditWidget::on_map_clicked()
{
	// TODO: This came in from a merge with develop, but 
	// I'm not sure yet how to fix this. 
//	ui->plt->mapToUserRect();
}

void CCurveEditWidget::on_lineType_currentIndexChanged(int n)
{
	LoadCurve* plc = ui->plt->GetLoadCurve();
	if (plc == nullptr) return;
	plc->SetInterpolator(n);
	plc->Update();
	ui->plt->repaint();
	emit dataChanged();
}

void CCurveEditWidget::on_extendMode_currentIndexChanged(int n)
{
	LoadCurve* plc = ui->plt->GetLoadCurve();
	if (plc == nullptr) return;
	plc->SetExtendMode(n);
	ui->plt->repaint();
	emit dataChanged();
}

void CCurveEditWidget::on_undo_clicked(bool b)
{
	LoadCurve* plc = ui->plt->GetLoadCurve();
	if (plc == nullptr) return;

	if (m_cmd.CanUndo()) m_cmd.UndoCommand();

	QString undo = m_cmd.CanUndo() ? m_cmd.GetUndoCmdName() : "(Nothing to undo)";
	QString redo = m_cmd.CanRedo() ? m_cmd.GetRedoCmdName() : "(Nothing to redo)";
	ui->setCmdNames(undo, redo);

	UpdatePlotData();
	ui->enablePointEdit(false);
	ui->plt->repaint();
	emit dataChanged();
}

void CCurveEditWidget::on_redo_clicked(bool b)
{
	LoadCurve* plc = ui->plt->GetLoadCurve();
	if (plc == nullptr) return;

	if (m_cmd.CanRedo()) m_cmd.RedoCommand();

	QString undo = m_cmd.CanUndo() ? m_cmd.GetUndoCmdName() : "(Nothing to undo)";
	QString redo = m_cmd.CanRedo() ? m_cmd.GetRedoCmdName() : "(Nothing to redo)";
	ui->setCmdNames(undo, redo);

	UpdatePlotData();
	ui->enablePointEdit(false);
	ui->plt->repaint();
	emit dataChanged();
}

void CCurveEditWidget::on_math_clicked(bool b)
{
	LoadCurve* plc = ui->plt->GetLoadCurve();
	if (plc == nullptr) return;

	CDlgFormula dlg(this);

	dlg.SetMath(plc->GetName());

	if (dlg.exec())
	{
		std::vector<vec2d> pts = dlg.GetPoints();
		QString math = dlg.GetMath();
		std::string smath = math.toStdString();

		bool insertMode = dlg.Insert();
		if (insertMode == false) plc->Clear();
		plc->SetName(smath.c_str());
		if (pts.empty() && (insertMode == false))
		{
			plc->Add(0, 0);
			plc->Add(1, 1);
		}
		else
		{
			for (int i = 0; i < (int)pts.size(); ++i)
			{
				plc->Add(pts[i]);
			}
		}

		SetLoadCurve(plc);
		ui->enablePointEdit(false);
		emit dataChanged();
	}
}

void CCurveEditWidget::on_copy_clicked(bool b)
{
	ui->m_clipboard_backup = ui->plt->OnCopyToClipboard();
}

void CCurveEditWidget::on_paste_clicked(bool b)
{
	LoadCurve* plc = ui->plt->GetLoadCurve();
	if (plc == nullptr) return;

    QClipboard* clipboard = QApplication::clipboard();

    if(!clipboard->mimeData()->hasText()) return;

    QString data = clipboard->text();

    // If this data was copied from the widget itself, we need 
    // to chop off the first row since it's the row labels
    bool sameData = false;
    if(data == ui->m_clipboard_backup)
    {
        int pos = data.indexOf("\n");
        data = data.right(data.size() - pos);

        sameData = true;
    }

    CDlgImportData dlg(data, DataType::DOUBLE, 2);

    bool getValues = false;

    // If this data was copied from the widget itself, we know its
    // format is valid, and so we don't need to open the dialog
    if(sameData)
    {
        getValues = true;
    }
    else if(dlg.exec())
    {
        getValues = true;
    }

    if(getValues)
    {
        QList<QStringList> values = dlg.GetValues();

		plc->Clear();
        for(auto row : values)
        {
            plc->Add(row[0].toDouble(), row[1].toDouble());
        }

        SetLoadCurve(plc);
	    emit dataChanged();
    }
}

class CDlgImportCurve : public QDialog
{
private:
	QLineEdit* m_skip;
	QComboBox* m_delim;
	QLineEdit* m_xColumn;
	QLineEdit* m_yColumn;

	static int	m_nskip;
	static int	m_ndelim;
	static int	m_nx;
	static int	m_ny;

public:
	CDlgImportCurve(QWidget* parent) : QDialog(parent)
	{
		QFormLayout* l = new QFormLayout;

		m_skip = new QLineEdit;	m_skip->setValidator(new QIntValidator(0, 1000)); m_skip->setText("0");
		l->addRow("Skip lines (header):", m_skip);

		m_delim = new QComboBox;
		m_delim->addItems(QStringList() << "Space" << "Comma" << "Tab");
		l->addRow("Delimiter:", m_delim);

		m_xColumn = new QLineEdit; m_xColumn->setValidator(new QIntValidator(0, 1000)); m_xColumn->setText("0");
		m_yColumn = new QLineEdit; m_yColumn->setValidator(new QIntValidator(0, 1000)); m_yColumn->setText("1");

		l->addRow("X column index:", m_xColumn);
		l->addRow("Y column index:", m_yColumn);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		QVBoxLayout* v = new QVBoxLayout;
		v->addLayout(l);
		v->addWidget(bb);

		setLayout(v);

		// set initial values
		m_skip->setText(QString::number(m_nskip));
		m_delim->setCurrentIndex(m_ndelim);
		m_xColumn->setText(QString::number(m_nx));
		m_yColumn->setText(QString::number(m_ny));

		QObject::connect(bb, SIGNAL(accepted()), this, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), this, SLOT(reject()));
	}

	char GetDelimiter()
	{
		int n = m_delim->currentIndex();
		switch (n)
		{
		case 0: return ' '; break;
		case 1: return ','; break;
		case 2: return '\t'; break;
		}
		return ' ';
	}

	int GetSkipLines() { return m_skip->text().toInt(); }

	int GetXColumnIndex() { return m_xColumn->text().toInt(); }
	int GetYColumnIndex() { return m_yColumn->text().toInt(); }

	void accept() override
	{
		m_nskip = m_skip->text().toInt();
		m_ndelim = m_delim->currentIndex();
		m_nx = m_xColumn->text().toInt();
		m_ny = m_yColumn->text().toInt();
		QDialog::accept();
	}
};

int CDlgImportCurve::m_nskip = 0;
int CDlgImportCurve::m_ndelim = 0;
int CDlgImportCurve::m_nx = 0;
int CDlgImportCurve::m_ny = 1;

bool ReadLoadCurve(LoadCurve& lc, const char* szfile, char delim = ' ', int nskip = 0, int xColumnIndex = 0, int yColumnIndex = 1)
{
	// sanity checks
	if (xColumnIndex < 0) { assert(false); return false; }
	if (yColumnIndex < 0) { assert(false); return false; }

	// create the file
	FILE* fp = fopen(szfile, "rt");
	if (fp == 0) return false;

	// make sure the load curve is empty
	lc.Clear();

	// read the file
	char szline[256];
	fgets(szline, 255, fp);
	int nlines = 0;
	while (!feof(fp))
	{
		// process line
		if (nlines >= nskip)
		{
			vector<double> d;
			const char* sz = szline;
			while (sz)
			{
				double w = atof(sz);
				d.push_back(w);
				sz = strchr(sz, delim);
				if (sz) sz++;
			}
			if (d.empty()) break;

			double x = 0.0, y = 0.0;
			if (xColumnIndex < d.size()) x = d[xColumnIndex];
			if (yColumnIndex < d.size()) y = d[yColumnIndex];

			lc.Add(x, y);
		}

		fgets(szline, 255, fp);
		nlines++;
	}

	fclose(fp);

	return true;
}

void CCurveEditWidget::on_open_clicked(bool b)
{
	LoadCurve* plc = ui->plt->GetLoadCurve();
	if (plc == nullptr) return;

	QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "All files (*)");
	if (fileName.isEmpty() == false)
	{
		CDlgImportCurve dlg(this);
		if (dlg.exec())
		{
			char delim = dlg.GetDelimiter();
			int nskip = dlg.GetSkipLines();
			int nx = dlg.GetXColumnIndex();
			int ny = dlg.GetYColumnIndex();
			LoadCurve lc;
			std::string sfile = fileName.toStdString();
			const char* szfile = sfile.c_str();
			if (ReadLoadCurve(lc, szfile, delim, nskip, nx, ny))
			{
				*plc = lc;
				SetLoadCurve(plc);
				emit dataChanged();
			}
		}
	}
}

bool WriteLoadCurve(LoadCurve& lc, const char* szfile)
{
	FILE* fp = fopen(szfile, "wt");
	if (fp == 0) return false;

	for (int i = 0; i < lc.Points(); ++i)
	{
		vec2d pt = lc.Point(i);
		fprintf(fp, "%lg %lg\n", pt.x(), pt.y());
	}
	fclose(fp);
	return true;
}

void CCurveEditWidget::on_save_clicked(bool b)
{
	LoadCurve* plc = ui->plt->GetLoadCurve();
	if (plc == nullptr) return;

	QString fileName = QFileDialog::getSaveFileName(this, "Open File", "", "All files (*)");
	if (fileName.isEmpty() == false)
	{
		std::string sfile = fileName.toStdString();
		const char* szfile = sfile.c_str();
		if (WriteLoadCurve(*plc, szfile) == false)
		{
			QMessageBox::critical(this, "Save File", QString("Failed saving curve data to file %1").arg(szfile));
		}
	}
}

void CCurveEditWidget::on_clear_clicked()
{
	LoadCurve* plc = ui->plt->GetLoadCurve();
	if (plc == nullptr) return;

	if (QMessageBox::question(this, "Clear Curve", "Are you sure you to clear all points on the curve?") == QMessageBox::Yes)
	{
		plc->Clear();
		SetLoadCurve(plc);
	}
}

void CCurveEditWidget::on_showDeriv_toggled(bool b)
{
	ui->plt->ShowDeriv(b);
}


//=============================================================================
CMathPlotWidget::CMathPlotWidget(QWidget* parent) : CPlotWidget(parent)
{
	showLegend(false);
	m_ord = "x";
	m_math.AddVariable(m_ord);
	m_math.Create("0");

	setFullScreenMode(true);
	setXAxisLabelAlignment(ALIGN_LABEL_TOP);
	setYAxisLabelAlignment(ALIGN_LABEL_RIGHT);
	setBackgroundColor(QColor(48, 48, 48));
	setGridColor(QColor(128, 128, 128));
	setXAxisColor(QColor(200, 200, 200));
	setYAxisColor(QColor(200, 200, 200));
	setXAxisTickColor(QColor(255, 255, 255));
	setYAxisTickColor(QColor(255, 255, 255));
	setSelectionColor(QColor(255, 255, 192));

	CPlotData* data = new CPlotData;
	addPlotData(data);
	data->setLineColor(QColor(255, 92, 164));

	m_leftExtend = 0;
	m_rghtExtend = 0;

	QObject::connect(this, SIGNAL(regionSelected(QRect)), this, SLOT(onRegionSelected(QRect)));
	QObject::connect(this, SIGNAL(pointClicked(QPointF, bool)), this, SLOT(onPointClicked(QPointF, bool)));
}

double CMathPlotWidget::value(double x, MVariable* var, int& region)
{
	double xmin, xmax;
	GetHighlightInterval(xmin, xmax);
	double Dx = xmax - xmin;

	if (x <= xmin)
	{
		region = -(int) ((xmin - x)/Dx) - 1;
		switch (m_leftExtend)
		{
		case ExtendMode::ZERO: return 0.0; break;
		case ExtendMode::CONSTANT: x = xmin; break;
		case ExtendMode::REPEAT: x = xmax - fmod(xmin - x, Dx); break;
		}
	}
	else if (x >= xmax)
	{
		region = (int)((x - xmin)/Dx);
		switch (m_rghtExtend)
		{
		case ExtendMode::ZERO    : return 0.0; break;
		case ExtendMode::CONSTANT: x = xmax; break;
		case ExtendMode::REPEAT  : x = xmin + fmod(x - xmin, Dx); break;
		}
	}
	else region = 0;

	var->value(x);
	double y = m_math.value();
	return y;
}

void CMathPlotWidget::DrawPlotData(QPainter& painter, CPlotData& data)
{
	MVariable* var = m_math.FindVariable(m_ord);
	if (var == nullptr) return;

	double xmin, xmax;
	GetHighlightInterval(xmin, xmax);
	bool useInterval = false;
	if (xmax > xmin) useInterval = true;

	// draw the line
	painter.setPen(QPen(data.lineColor(), data.lineWidth()));
	QRect rt = ScreenRect();
	QPointF p0, p1;
	int prevRegion = 0;
	int curRegion = 0;
	bool newSection = true;
	for (int i = rt.left(); i < rt.right(); i += 2)
	{
		p1.setX(i);
		QPointF p = ScreenToView(p1);

		double x = p.x();
		double y = 0.0;
		if (useInterval)
		{
			y = value(x, var, curRegion);
		}
		else
		{
			var->value(x);
			y = m_math.value();
		}

		if (ISNAN(y))
		{
			newSection = true;
		}
		else
		{
			p.setY(y);
			p1 = ViewToScreen(p);

			if (newSection == false)
			{
				if (curRegion != prevRegion) p0.setY(p1.y());
				painter.drawLine(p0, p1);
			}

			p0 = p1;
			newSection = false;
		}

		prevRegion = curRegion;
	}
}

void CMathPlotWidget::SetOrdinate(const std::string& x)
{
	if (x.empty()) m_ord = "x";
	else m_ord = x;
}

void CMathPlotWidget::ClearVariables()
{
	m_Var.clear();
}

void CMathPlotWidget::SetVariable(const QString& name, double val)
{
	m_Var.push_back(std::pair<QString, double>(name, val));
}

void CMathPlotWidget::SetMath(const QString& txt)
{
	std::string m = txt.toStdString();
	m_math.Clear();
	m_math.AddVariable(m_ord);

	for (int i = 0; i < m_Var.size(); ++i)
	{
		std::pair<QString, double>& vi = m_Var[i];
		std::string mi = vi.first.toStdString();
		m_math.AddVariable(mi, vi.second);
	}

	getPlotData(0).clear();

	if (m.empty()) m = "0";
	m_math.Create(m);

	repaint();
}

void CMathPlotWidget::setLeftExtendMode(int n)
{
	m_leftExtend = n;
	repaint();
}

void CMathPlotWidget::setRightExtendMode(int n)
{
	m_rghtExtend = n;
	repaint();
}

void CMathPlotWidget::onRegionSelected(QRect rt)
{
	clearSelection();
	getPlotData(0).clear();
	fitToRect(rt);
}

void CMathPlotWidget::onPointClicked(QPointF pt, bool shift)
{
	MVariable* x = m_math.FindVariable(m_ord);
	if (x == nullptr) return;

	x->value(pt.x());
	double y = m_math.value();

	QPointF px(pt.x(), y);
	QPointF p0 = ViewToScreen(pt);
	QPointF p1 = ViewToScreen(px);

	CPlotData& data = getPlotData(0);
	data.clear();

	double D = abs(p0.x() - p1.x()) + abs(p0.y() - p1.y());
	if (D < 5)
	{
		data.addPoint(pt.x(), y);
		selectPoint(0, 0);
	}
	repaint();
}

//=============================================================================
class UIMathEditWidget
{
public:
	QLineEdit* rngMin;
	QLineEdit* rngMax;
	QComboBox* leftExt;
	QComboBox* rghtExt;
	QLineEdit* edit;
	CMathPlotWidget* plot;
	QLabel* fnc;

	QWidget* rngOps;

public:
	void setup(QWidget* w)
	{
		rngMin = new QLineEdit; rngMin->setValidator(new QDoubleValidator()); rngMin->setText(QString::number(0.0));
		rngMax = new QLineEdit; rngMax->setValidator(new QDoubleValidator()); rngMax->setText(QString::number(1.0));

		leftExt = new QComboBox; leftExt->addItems(QStringList() << "zero" << "constant" << "repeat");
		rghtExt = new QComboBox; rghtExt->addItems(QStringList() << "zero" << "constant" << "repeat");

		QHBoxLayout* hx = new QHBoxLayout;
		hx->addWidget(new QLabel("min:")); hx->addWidget(rngMin);
		hx->addWidget(new QLabel("max:")); hx->addWidget(rngMax);
		hx->addWidget(new QLabel("left extend:" )); hx->addWidget(leftExt);
		hx->addWidget(new QLabel("right extend:")); hx->addWidget(rghtExt);
		hx->addStretch();

		rngOps = new QWidget;
		rngOps->setLayout(hx);
		rngOps->hide();
		rngOps->setSizePolicy(rngOps->sizePolicy().horizontalPolicy(), QSizePolicy::Fixed);

		edit = new QLineEdit;
		QHBoxLayout* editLayout = new QHBoxLayout;
		editLayout->addWidget(fnc = new QLabel("f(x) = "));
		editLayout->addWidget(edit);

		QVBoxLayout* l = new QVBoxLayout;
		l->addWidget(rngOps);
		l->addLayout(editLayout);
		l->addWidget(plot = new CMathPlotWidget);
		w->setLayout(l);

		QObject::connect(edit, SIGNAL(editingFinished()), w, SLOT(onEditingFinished()));
		QObject::connect(leftExt, SIGNAL(currentIndexChanged(int)), w, SLOT(onLeftExtendChanged()));
		QObject::connect(rghtExt, SIGNAL(currentIndexChanged(int)), w, SLOT(onRightExtendChanged()));
		QObject::connect(rngMin, SIGNAL(textChanged(const QString&)), w, SLOT(onRangeMinChanged()));
		QObject::connect(rngMax, SIGNAL(textChanged(const QString&)), w, SLOT(onRangeMaxChanged()));
	}

	void showRangeOptions(bool b)
	{
		rngOps->setVisible(b);
	}
};

CMathEditWidget::CMathEditWidget(QWidget* parent) : QWidget(parent), ui(new UIMathEditWidget)
{
	ui->setup(this);
}

void CMathEditWidget::SetOrdinate(const QString& x)
{
	QString t = QString("f(%1) = ").arg(x);
	ui->fnc->setText(t);
	ui->plot->SetOrdinate(x.toStdString());
}

void CMathEditWidget::onEditingFinished()
{
	QString s = ui->edit->text();
	ui->plot->SetMath(s);

	emit mathChanged(s);
}

void CMathEditWidget::onLeftExtendChanged()
{
	int n = ui->leftExt->currentIndex();
	ui->plot->setLeftExtendMode(n);
	emit leftExtendChanged(n);
}

void CMathEditWidget::onRightExtendChanged()
{
	int n = ui->rghtExt->currentIndex();
	ui->plot->setRightExtendMode(n);
	emit rightExtendChanged(n);
}

void CMathEditWidget::onRangeMinChanged()
{
	double v = ui->rngMin->text().toDouble();

	double xmin, xmax;
	ui->plot->GetHighlightInterval(xmin, xmax);

	ui->plot->SetHighlightInterval(v, xmax);

	emit minChanged(v);
}

void CMathEditWidget::onRangeMaxChanged()
{
	double v = ui->rngMax->text().toDouble();

	double xmin, xmax;
	ui->plot->GetHighlightInterval(xmin, xmax);

	ui->plot->SetHighlightInterval(xmin, v);

	emit maxChanged(v);
}

void CMathEditWidget::showRangeOptions(bool b)
{
	ui->showRangeOptions(b);
}

void CMathEditWidget::setMinMaxRange(double rmin, double rmax)
{
	ui->plot->SetHighlightInterval(rmin, rmax);
}

void CMathEditWidget::ClearVariables() { ui->plot->ClearVariables(); }
void CMathEditWidget::SetVariable(const QString& name, double val) { ui->plot->SetVariable(name, val); }

void CMathEditWidget::SetMath(const QString& txt)
{
	ui->edit->setText(txt);
	ui->plot->SetMath(txt);
}

void CMathEditWidget::setLeftExtend(int n)
{
	ui->leftExt->setCurrentIndex(n);
}

void CMathEditWidget::setRightExtend(int n)
{
	ui->rghtExt->setCurrentIndex(n);
}
