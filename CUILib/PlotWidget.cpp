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
#include <QDropEvent>
#include <QComboBox>
#include <QLabel>
#include <QtCore/QMimeData>
#include <FSCore/LoadCurve.h>

using namespace std;

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
	QFont f("Times", m_data.m_titleFontSize, QFont::Bold);
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

		double gy = 1;
		if (m_bscaleAxisLabels)
		{
			int nydiv = (int)log10(m_yscale);
			if (nydiv != 0)
			{
				gy = pow(10.0, nydiv);
			}
		}

		if ((m_data.m_yAxis.labelPosition == LOW) && (m_data.m_yAxis.labelAlignment == ALIGN_LABEL_LEFT))
		{
			QFont f("Arial", m_data.m_axesFontSize);
			QFontMetrics fm(f);

			int y1 = m_plotRect.bottom();
			int maxWidth = 0;
			char sz[256] = { 0 };
			double fy = m_yscale * floor(m_viewRect.top() / m_yscale);
			while (fy < m_viewRect.bottom())
			{
				int iy = ViewToScreen(QPointF(0.0, fy)).y();
				if (iy < y1)
				{
					double g = fy / gy;
					if (fabs(g) < 1e-7) g = 0;
					snprintf(sz, sizeof sz, "%lg", g);
					QString s(sz);

					int w = fm.horizontalAdvance(s);
					if (w > maxWidth) maxWidth = w;
				}
				fy += m_yscale;
			}
			maxWidth += 10;
			if (maxWidth > m_plotRect.left()) m_plotRect.setLeft(m_screenRect.left() + maxWidth);
		}

		if (m_data.m_bshowLegend && !m_data.m_data.empty())
		{
			int maxWidth = 0;
			int N = (int)m_data.m_data.size();
			QFont f("Arial", m_data.m_legendFontSize);
			QFontMetrics fm(f);

			for (int i = 0; i < N; ++i)
			{
				CPlotData& plot = *m_data.m_data[i];
				int w = fm.horizontalAdvance(plot.label());
				if (w > maxWidth) maxWidth = w;
			}

			maxWidth += 10 + 40; // 40 is the space taken up by the line (=25 + 15 padding)
			double W = m_screenRect.right() - m_plotRect.right();
			if (maxWidth > W) m_plotRect.setRight(m_screenRect.right() - maxWidth);
		}

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

	QFont f("Arial", m_data.m_legendFontSize);
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
	QFont f("Times", m_data.m_titleFontSize, QFont::Bold);
	p.setFont(f);
	QFontMetrics fm(f);
	p.drawText(m_titleRect, Qt::AlignCenter, m_data.m_title);
}

//-----------------------------------------------------------------------------
void CPlotWidget::drawAxesLabels(QPainter& p)
{
	QFont f("Arial", m_data.m_axesFontSize);
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
	QFont f("Arial", m_data.m_axesFontSize);
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
			p.drawText(x0 - 30, y0 - fm.descent() - 5, QString(sz));
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

		double fy = yscale*floor(m_viewRect.top() / yscale);
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

		double fx = xscale*floor(m_viewRect.left() / xscale);
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
		double fy = yscale*floor(m_viewRect.top()/yscale);
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
		double fx = xscale*floor(m_viewRect.left() / xscale);
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
