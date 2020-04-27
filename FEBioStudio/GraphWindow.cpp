#include "GraphWindow.h"
#include "PlotWidget.h"
#include "DataFieldSelector.h"
#include <QToolBar>
#include <qstackedwidget.h>
#include <QLabel>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QCheckBox>
#include <QFormLayout>
#include <QRadioButton>
#include <QGuiApplication>
#include <QScreen>
#include "MainWindow.h"
#include "Document.h"
#include <PostLib/FEPostModel.h>
#include <QToolBox>
#include <QLineEdit>
#include <PostGL/GLDataMap.h>
#include <PostGL/GLModel.h>
#include "version.h"
#include <QValidator>
#include <QComboBox>
#include <MathLib/LinearRegression.h>
#include "CColorButton.h"
#include <GLWLib/convert.h>
#include "PostDoc.h"

OptionsUi::OptionsUi(CGraphWidget* graph, QWidget* parent) : CPlotTool(parent)
{
	QVBoxLayout* l = new QVBoxLayout;
	l->addWidget(timeOption[0] = new QRadioButton("Time step range"));
	l->addWidget(timeOption[1] = new QRadioButton("Current time step"));
	l->addWidget(timeOption[2] = new QRadioButton("User range:"));
	l->addWidget(timeRange = new QLineEdit);
	l->addWidget(smoothLines = new QCheckBox("Smooth lines"));
	l->addWidget(dataMarks   = new QCheckBox("Show data marks"));
	l->addWidget(autoRange   = new QCheckBox("auto update plot range"));
	l->addStretch();
	setLayout(l);

	smoothLines->setChecked(true);
	dataMarks->setChecked(true);
	autoRange->setChecked(true);

	timeOption[0]->setChecked(true);

	QObject::connect(timeOption[0], SIGNAL(clicked()), this, SLOT(onOptionsChanged()));
	QObject::connect(timeOption[1], SIGNAL(clicked()), this, SLOT(onOptionsChanged()));
	QObject::connect(timeOption[2], SIGNAL(clicked()), this, SLOT(onOptionsChanged()));
	QObject::connect(timeRange    , SIGNAL(editingFinished()), this, SLOT(onOptionsChanged()));
	QObject::connect(smoothLines  , SIGNAL(stateChanged(int)), SLOT(onOptionsChanged()));
	QObject::connect(dataMarks    , SIGNAL(stateChanged(int)), SLOT(onOptionsChanged()));
	QObject::connect(autoRange  ,   SIGNAL(stateChanged(int)), SLOT(onOptionsChanged()));
}

void OptionsUi::onOptionsChanged()
{
	emit optionsChanged();
}

int OptionsUi::currentOption()
{
	if (timeOption[0]->isChecked()) return 0;
	if (timeOption[1]->isChecked()) return 1;
	if (timeOption[2]->isChecked()) return 2;
	return -1;
}

bool OptionsUi::lineSmoothing()
{
	return smoothLines->isChecked();
}

bool OptionsUi::showDataMarks()
{
	return dataMarks->isChecked();
}

bool OptionsUi::autoRangeUpdate()
{
	return autoRange->isChecked();
}

void OptionsUi::setUserTimeRange(int imin, int imax)
{
	timeRange->setText(QString("%1:%2").arg(imin).arg(imax));
}

void OptionsUi::getUserTimeRange(int& imin, int& imax)
{
	QStringList l = timeRange->text().split(':');
	imin = imax = 0;
	if (l.size() == 1)
	{
		imin = imax = l.at(0).toInt();
	}
	else if (l.size() > 1)
	{
		imin = l.at(0).toInt();
		imax = l.at(1).toInt();
	}
}

//=================================================================================================

RegressionUi::RegressionUi(CGraphWidget* graph, QWidget* parent) : CPlotTool(parent), m_graph(graph)
{
	m_src = new QComboBox;

	m_fnc = new QComboBox;
	m_fnc->addItem("Linear");
	m_fnc->addItem("Quadratic");
	m_fnc->addItem("Exponential");

	m_col.setRgb(0,0,0);

	CColorButton* cb = new CColorButton;
	cb->setColor(m_col);
	cb->setMaximumWidth(20);

	QHBoxLayout* h = new QHBoxLayout;
	h->setMargin(0);
	h->addWidget(m_fnc);
	h->addWidget(cb);

	QFormLayout* f = new QFormLayout;
	f->setLabelAlignment(Qt::AlignRight);
	f->setMargin(0);
	f->addRow("Source", m_src);
	f->addRow("Function", h);
	f->addRow("", m_math = new QLabel(""));
	f->addRow(m_lbl[0] = new QLabel("a"), m_par[0] = new QLineEdit); m_lbl[0]->setBuddy(m_par[0]);
	m_par[0]->setValidator(new QDoubleValidator);
	m_par[0]->setReadOnly(true);
	f->addRow(m_lbl[1] = new QLabel("b"), m_par[1] = new QLineEdit); m_lbl[1]->setBuddy(m_par[1]);
	m_par[1]->setValidator(new QDoubleValidator);
	m_par[1]->setReadOnly(true);
	f->addRow(m_lbl[2] = new QLabel("c"), m_par[2] = new QLineEdit); m_lbl[2]->setBuddy(m_par[2]);
	m_par[2]->setValidator(new QDoubleValidator);
	m_par[2]->setReadOnly(true);

	QVBoxLayout* v = new QVBoxLayout;
	v->addLayout(f);

	QPushButton* pb = new QPushButton("Calculate");
	v->addWidget(pb);

	v->addStretch();
	setLayout(v);

	QObject::connect(pb, SIGNAL(clicked()), this, SLOT(onCalculate()));
	QObject::connect(m_fnc, SIGNAL(currentIndexChanged(int)), this, SLOT(onFunctionChanged(int)));
	QObject::connect(cb, SIGNAL(colorChanged(QColor)), this, SLOT(onColorChanged(QColor)));

	onFunctionChanged(0);

	Update();
}

void RegressionUi::showParameters(int numParam)
{
	if (numParam == 2) { m_lbl[2]->hide(); m_par[2]->hide(); }
	else { m_lbl[2]->show(); m_par[2]->show(); }
}

void RegressionUi::clearParameters()
{
	m_bvalid = false;
	m_par[0]->clear();
	m_par[1]->clear();
	m_par[2]->clear();
}

void RegressionUi::onFunctionChanged(int n)
{
	switch (n)
	{
	case 0: showParameters(2); m_math->setText("<p>y = <b>a</b>*x + <b>b</b></p>"); break;
	case 1: showParameters(3); m_math->setText("<p>y = <b>a</b>*x^2 + <b>b</b>*x + <b>c</b></p>"); break;
	case 2: showParameters(2); m_math->setText("<p>y = <b>a</b>*exp(<b>b</b>*x)</p>"); break;
	}

	clearParameters();
	m_graph->repaint();
}

void RegressionUi::onColorChanged(QColor c)
{
	m_col = c;
	m_graph->repaint();
}

void RegressionUi::onCalculate()
{
	clearParameters();

	if (m_graph == 0) return;

	// get the first data field
	int plots = m_graph->plots();
	if (plots == 0) return;

	int n = m_src->currentIndex();
	if ((n < 0) || (n >= plots)) return;

	CPlotData& data = m_graph->getPlotData(n);
	int N = data.size();
	vector<pair<double, double> > pt(N);
	for (int i=0; i<N; ++i)
	{	
		QPointF p = data.Point(i);
		pt[i].first = p.x();
		pt[i].second = p.y();
	}

	int nfc = m_fnc->currentIndex();
	if (nfc == 0)
	{
		pair<double, double> ans;
		if (LinearRegression(pt, ans))
		{
			m_a = ans.first;
			m_b = ans.second;
			m_par[0]->setText(QString::number(m_a));
			m_par[1]->setText(QString::number(m_b));
			m_bvalid = true;
			m_graph->repaint();
		}
	}
	else if (nfc == 1)
	{
		vector<double> ans(3, 0.0);
		if (NonlinearRegression(pt, ans, 1))
		{
			m_a = ans[0];
			m_b = ans[1];
			m_c = ans[2];
			m_par[0]->setText(QString::number(m_a));
			m_par[1]->setText(QString::number(m_b));
			m_par[2]->setText(QString::number(m_c));
			m_bvalid = true;
			m_graph->repaint();
		}
	}
	else if (nfc == 2)
	{
		// do a linear regression on the log
		for (int i=0; i<N; ++i)
		{
			pair<double, double>& pi = pt[i];
			if (pi.second > 0) pi.second = log(pi.second);
			else return;
		}
		pair<double, double> ans;
		if (LinearRegression(pt, ans))
		{
			m_a = exp(ans.second);
			m_b = ans.first;
			m_par[0]->setText(QString::number(m_a));
			m_par[1]->setText(QString::number(m_b));
			m_bvalid = true;
			m_graph->repaint();
		}
	}
}

void RegressionUi::draw(QPainter& p)
{
	if (m_bvalid == false) return;

	p.setPen(QPen(m_col, 2));

	QRectF vr = m_graph->m_viewRect;
	QRect sr = m_graph->ScreenRect();

	int func = m_fnc->currentIndex();

	QPoint p0, p1;
	int ierr = 0;
	for (int i = sr.left(); i < sr.right(); i += 2)
	{
		double x = vr.left() + (i - sr.left())*(vr.right() - vr.left()) / (sr.right() - sr.left());

		double y = 0;
		switch (func)
		{
		case 0: y = m_a*x + m_b; break;
		case 1: y = m_a*x*x + m_b*x + m_c; break;
		case 2: y = m_a*exp(m_b*x); break;
		}

		p1 = m_graph->ViewToScreen(QPointF(x, y));

		if (i != sr.left())
		{
			p.drawLine(p0, p1);
		}

		p0 = p1;
	}
}

void RegressionUi::Update()
{
	int nplots = m_graph->plots();
	m_src->clear();
	for (int i = 0; i < nplots; ++i)
	{
		QString l = m_graph->getPlotData(i).label();
		m_src->addItem(l);
	}
	clearParameters();
}

void RegressionUi::hideEvent(QHideEvent* ev)
{
	if (m_bvalid)
	{
		m_bvalid = false;
		m_graph->repaint();
	}
}

//=================================================================================================
MathPlot::MathPlot(CGraphWidget* graph, QWidget* parent) : CPlotTool(parent), m_graph(graph)
{
	m_bvalid = false;

	m_col.setRgb(0, 0, 0);

	CColorButton* cb = new CColorButton;
	cb->setColor(m_col);
	cb->setMaximumWidth(20);

	m_edit = new QLineEdit;

	QHBoxLayout* h = new QHBoxLayout;
	h->setMargin(0);
	h->addWidget(new QLabel("y(x) = "));
	h->addWidget(m_edit);
	h->addWidget(cb);

	QVBoxLayout* l = new QVBoxLayout;
	l->addLayout(h);

	QPushButton* b = new QPushButton("Plot");
	l->addWidget(b);
	l->addStretch();
	setLayout(l);

	QObject::connect(b, SIGNAL(clicked()), this, SLOT(onCalculate()));
	QObject::connect(cb, SIGNAL(colorChanged(QColor)), this, SLOT(onColorChanged(QColor)));
}

void MathPlot::onColorChanged(QColor c)
{
	m_col = c;
	m_graph->repaint();
}

void MathPlot::onCalculate()
{
	Update();

	QString t = m_edit->text();
	if (t.isEmpty() == false)
	{
		m_math = t.toStdString();
		m_bvalid = true;
		if (m_bvalid) m_graph->repaint();
	}
}

void MathPlot::draw(QPainter& p)
{
	if (m_bvalid == false) return;

	p.setPen(QPen(m_col, 2));

	CMathParser mp;

	QRectF vr = m_graph->m_viewRect;
	QRect sr = m_graph->ScreenRect();

	QPoint p0, p1;
	int ierr = 0;
	for (int i=sr.left(); i < sr.right(); i += 2)
	{
		double x = vr.left() + (i - sr.left())*(vr.right() - vr.left())/ (sr.right() - sr.left());
		mp.set_variable("x", x);

		double y = mp.eval(m_math.c_str(), ierr);
		
		p1 = m_graph->ViewToScreen(QPointF(x,y));

		if (i != sr.left())
		{
			p.drawLine(p0, p1);
		}

		p0 = p1;
	}
}

void MathPlot::Update()
{
	m_bvalid = false;
}

void MathPlot::hideEvent(QHideEvent* ev)
{
	if (m_bvalid)
	{
		m_bvalid = false;
		m_graph->repaint();
	}
}

//=============================================================================
void CGraphWidget::paintEvent(QPaintEvent* pe)
{
	CPlotWidget::paintEvent(pe);

	QPainter p(this);
	p.setClipRect(m_screenRect);
	p.setRenderHint(QPainter::Antialiasing, true);
	for (size_t i = 0; i<m_tools.size(); ++i)
	{
		CPlotTool* tool = m_tools[i];
		tool->draw(p);
	}
}

void CGraphWidget::Update()
{
	for (size_t i = 0; i<m_tools.size(); ++i)
		m_tools[i]->Update();
	repaint();
}

//=============================================================================
class Ui::CGraphWindow
{
public:
	CGraphWidget*			plot;			// central graph widget
	QToolBar*				toolBar;		// top tool bar
	QToolBar*				zoomBar;		// bottom tool bar
	QComboBox*				selectPlot;		// choose the plot type
	CDataSelectorButton*	selectX;		// select the X data field
	CDataSelectorButton*	selectY;		// select the Y data field
	QToolBox*				tools;			// the tools panel

	QAction* actionSave;
	QAction* actionClipboard;
	QAction* actionSnapshot;
	QAction* actionProps;
	QAction* actionZoomSelect;

	QAction* actionSelectX;
	QAction* actionSelectY;

	OptionsUi*	ops;

	CPostDoc*	doc;

	QAction* actionType;
	QAction* actionPlot;

public:
	void setupUi(::CGraphWindow* parent)
	{
		QSplitter* centralWidget = new QSplitter;

		plot = new CGraphWidget(parent);
		plot->setObjectName("plot");

		centralWidget->addWidget(plot);
		centralWidget->addWidget(tools = new QToolBox); tools->hide();

		ops = new OptionsUi(plot); ops->setObjectName("options");
		tools->addItem(ops, "Options");
		plot->addTool(ops);

		CPlotTool* tool = new RegressionUi(plot);
		tools->addItem(tool, "Curve fitting");
		plot->addTool(tool);

		tool = new MathPlot(plot);
		tools->addItem(tool, "Math plot");
		plot->addTool(tool);

		// create plot selection combobox
		selectPlot = new QComboBox;
		selectPlot->setObjectName("selectPlot");
		selectPlot->addItem("Line");
		selectPlot->addItem("Scatter");
		selectPlot->addItem("Time-Scatter");

		// create X data selection box
		selectX = new CDataFieldSelector;
		selectX->setObjectName("selectX");
		selectX->setMinimumWidth(150);

		QWidget* x = new QWidget;
		QHBoxLayout* hx = new QHBoxLayout;
		hx->setMargin(0);
		hx->addWidget(new QLabel(" X: "));
		hx->addWidget(selectX);
		x->setLayout(hx);

		// create Y data selection box
		selectY = new CDataFieldSelector;
		selectY->setObjectName("selectY");
		selectY->setMinimumWidth(150);

		QWidget* y = new QWidget;
		QHBoxLayout* hy = new QHBoxLayout;
		hy->setMargin(0);
		hy->addWidget(new QLabel(" Y: "));
		hy->addWidget(selectY);
		y->setLayout(hy);

		toolBar = new QToolBar(parent);
		actionSave = toolBar->addAction(QIcon(QString(":/icons/save.png")), "Save"); actionSave->setObjectName("actionSave");
		actionClipboard = toolBar->addAction(QIcon(QString(":/icons/clipboard.png")), "Copy to clipboard"); actionClipboard->setObjectName("actionClipboard");
		actionSnapshot = toolBar->addAction(QIcon(QString(":/icons/bgimage.png")), "Save picture"); actionSnapshot->setObjectName("actionSnapshot");

		actionType = toolBar->addWidget(new QLabel("Type: "));
		actionPlot = toolBar->addWidget(selectPlot);
		actionSelectX = toolBar->addWidget(x);
		actionSelectY = toolBar->addWidget(y);
		QPushButton* showTools = new QPushButton("Tools");
		showTools->setCheckable(true);
		showTools->setChecked(false);
		toolBar->addWidget(showTools);

		zoomBar = new QToolBar(parent);
		QAction* actionZoomWidth  = zoomBar->addAction(QIcon(QString(":/icons/zoom_x.png" )), "Zoom Width" ); actionZoomWidth->setObjectName("actionZoomWidth" );
		QAction* actionZoomHeight = zoomBar->addAction(QIcon(QString(":/icons/zoom_y.png")), "Zoom Height"); actionZoomHeight->setObjectName("actionZoomHeight");
		QAction* actionZoomFit    = zoomBar->addAction(QIcon(QString(":/icons/zoom_fit.png"   )), "Zoom Fit"   ); actionZoomFit->setObjectName("actionZoomFit"   );
		actionZoomSelect = zoomBar->addAction(QIcon(QString(":/icons/zoom_select.png")), "Zoom Select"); actionZoomSelect->setObjectName("actionZoomSelect"); actionZoomSelect->setCheckable(true);
		zoomBar->addSeparator();
		actionProps = zoomBar->addAction(QIcon(QString(":/icons/properties.png")), "Properties"); actionProps->setObjectName("actionProps");

		QWidget* mainWidget = new QWidget;
		QVBoxLayout* layout = new QVBoxLayout;

		layout->addWidget(toolBar);
		layout->addWidget(centralWidget);
		layout->addWidget(zoomBar);

		mainWidget->setLayout(layout);

		parent->setWidget(mainWidget);

		parent->setFloating(true);

		QObject::connect(showTools, SIGNAL(clicked(bool)), tools, SLOT(setVisible(bool)));

		QMetaObject::connectSlotsByName(parent);
	}
};

QRect CGraphWindow::m_preferredSize;

CGraphWindow::CGraphWindow(CMainWindow* pwnd, CPostDoc* postDoc, int flags) : m_wnd(pwnd), QDockWidget(pwnd), ui(new Ui::CGraphWindow), CDocObserver(pwnd->GetDocument())
{
	m_nTrackTime = TRACK_TIME;
	m_nUserMin = 1;
	m_nUserMax = -1;

	// delete the window when it's closed
	setAttribute(Qt::WA_DeleteOnClose);

	ui->setupUi(this);
	ui->doc = postDoc;

	if ((flags & SHOW_TYPE_OPTIONS) == 0)
	{
		ui->actionType->setVisible(false);
		ui->actionPlot->setVisible(false);
	}

	// hide the selectors by default
	ui->actionSelectX->setVisible(false);
	ui->actionSelectY->setVisible(false);

	ui->ops->setUserTimeRange(m_nUserMin, m_nUserMax);
	setMinimumWidth(500);

	if (m_preferredSize.isValid())
	{
		setGeometry(m_preferredSize);
	}
	else resize(800, 600);
}

//-----------------------------------------------------------------------------
void CGraphWindow::closeEvent(QCloseEvent* closeEvent)
{
	m_wnd->RemoveGraph(this);
	m_preferredSize = geometry();
}

//-----------------------------------------------------------------------------
QRect CGraphWindow::preferredSize()
{
	return m_preferredSize;
}

//-----------------------------------------------------------------------------
void CGraphWindow::setPreferredSize(const QRect& rt)
{
	m_preferredSize = rt;
}

//-----------------------------------------------------------------------------
CPlotWidget* CGraphWindow::GetPlotWidget()
{
	return ui->plot;
}

//-----------------------------------------------------------------------------
// get the post doc
CPostDoc* CGraphWindow::GetPostDoc()
{
	return ui->doc;
}

//-----------------------------------------------------------------------------
int CGraphWindow::GetTimeTrackOption()
{
	return m_nTrackTime;
}

//-----------------------------------------------------------------------------
void CGraphWindow::GetUserTimeRange(int& minTime, int& maxTime)
{
	minTime = m_nUserMin - 1;
	maxTime = (m_nUserMax != -1 ? m_nUserMax - 1 : -1);
}

//-----------------------------------------------------------------------------
void CGraphWindow::GetTimeRange(int& minTime, int& maxTime)
{
	CPostDoc* doc = GetPostDoc();
	if (doc == nullptr) return;

	// get the document and current time point and time steps
	int ntime = doc->GetActiveState();
	int nsteps = doc->GetStates();

	// Figure out the time range
	int nmin = 0, nmax = 0;
	int trackTime = GetTimeTrackOption();
	if (trackTime == TRACK_USER_RANGE)
	{
		// get the user defined range
		GetUserTimeRange(nmin, nmax);
	}
	else if (trackTime == TRACK_TIME)
	{
		TIMESETTINGS& timeSettings = doc->GetTimeSettings();
		nmin = timeSettings.m_start;
		nmax = timeSettings.m_end;
	}
	else if (trackTime == TRACK_CURRENT_TIME)
	{
		// simply set the min and max to the same value
		nmin = nmax = ntime;
	}

	// validate range
	if (nmin <       0) nmin = 0;
	if (nmax == -1) nmax = nsteps - 1;
	if (nmax >= nsteps) nmax = nsteps - 1;
	if (nmax <    nmin) nmax = nmin;

	minTime = nmin;
	maxTime = nmax;
}

//-----------------------------------------------------------------------------
void CGraphWindow::ClearPlots()
{
	ui->plot->clear();
}

//-----------------------------------------------------------------------------
void CGraphWindow::AddPlotData(CPlotData* plot)
{
	ui->plot->addPlotData(plot);
}

//-----------------------------------------------------------------------------
void CGraphWindow::UpdatePlots()
{
	ui->plot->Update();
}

//-----------------------------------------------------------------------------
// redraw the plot widget
void CGraphWindow::RedrawPlot()
{
	ui->plot->repaint();
}

//-----------------------------------------------------------------------------
void CGraphWindow::FitPlotsToData()
{
	ui->plot->fitToData();
}

//-----------------------------------------------------------------------------
void CGraphWindow::SetPlotTitle(const QString& title)
{
	ui->plot->setTitle(title);
}

//-----------------------------------------------------------------------------
void CGraphWindow::SetXDataSelector(CDataSelector* sel, int nval)
{
	ui->selectX->SetDataSelector(sel);
	if (sel)
	{
		ui->actionSelectX->setVisible(true);
		if (nval != -1) ui->selectX->setCurrentValue(nval);
	}
	else ui->actionSelectX->setVisible(false);
}

//-----------------------------------------------------------------------------
void CGraphWindow::SetYDataSelector(CDataSelector* sel, int nval)
{
	ui->selectY->SetDataSelector(sel);
	if (sel)
	{
		ui->actionSelectY->setVisible(true);
		if (nval != -1) ui->selectY->setCurrentValue(nval);
	}
	else ui->actionSelectY->setVisible(false);
}

//-----------------------------------------------------------------------------
// get the text of the current selection in the X data selector
QString CGraphWindow::GetCurrentXText()
{
	return ui->selectX->text();
}

//-----------------------------------------------------------------------------
// get the data value associated with the current X selection
int CGraphWindow::GetCurrentXValue()
{
	return ui->selectX->currentValue();
}

//-----------------------------------------------------------------------------
// get the text of the current selection in the Y data selector
QString CGraphWindow::GetCurrentYText()
{
	return ui->selectY->text();
}

//-----------------------------------------------------------------------------
// get the data value associated with the current Y selection
int CGraphWindow::GetCurrentYValue()
{
	return ui->selectY->currentValue();
}

//-----------------------------------------------------------------------------
// get the current plot type
int CGraphWindow::GetCurrentPlotType()
{
	return ui->selectPlot->currentIndex();
}

//-----------------------------------------------------------------------------
void CGraphWindow::DocumentDelete()
{
	CDocObserver::DocumentDelete();
	Update();
}

//-----------------------------------------------------------------------------
void CGraphWindow::DocumentUpdate(bool newDoc)
{
	Update(newDoc, true);
}

//-----------------------------------------------------------------------------
void CGraphWindow::on_selectX_currentValueChanged(int)
{
	Update(false);
}

//-----------------------------------------------------------------------------
void CGraphWindow::on_selectY_currentValueChanged(int)
{
	Update(false);
}

//-----------------------------------------------------------------------------
void CGraphWindow::on_selectPlot_currentIndexChanged(int index)
{
	Update(true);
}

//-----------------------------------------------------------------------------
void CGraphWindow::on_actionSave_triggered()
{
	QString fileName = QFileDialog::getSaveFileName(this, "Save Graph Data", QDir::currentPath(), QString("All files (*)"));
	if (fileName.isEmpty() == false)
	{
		if (ui->plot->Save(fileName) == false)
			QMessageBox::critical(this, "Save Graph Data", "A problem occurred saving the data.");
	}
}

//-----------------------------------------------------------------------------
void CGraphWindow::on_actionClipboard_triggered()
{
	ui->plot->OnCopyToClipboard();
}

//-----------------------------------------------------------------------------
void CGraphWindow::on_actionSnapshot_triggered()
{
	CPlotWidget* plot = this->GetPlotWidget();
	QImage pixmap(plot->size(), QImage::Format_ARGB32);
	plot->render(&pixmap);
	m_wnd->SaveImage(pixmap);
}

//-----------------------------------------------------------------------------
void CGraphWindow::on_actionProps_triggered()
{
	ui->plot->OnShowProps();
}

//-----------------------------------------------------------------------------
void CGraphWindow::on_actionZoomWidth_triggered()
{
	ui->plot->OnZoomToWidth();
}

//-----------------------------------------------------------------------------
void CGraphWindow::on_actionZoomHeight_triggered()
{
	ui->plot->OnZoomToHeight();
}

//-----------------------------------------------------------------------------
void CGraphWindow::on_actionZoomFit_triggered()
{
	ui->plot->OnZoomToFit();
}

//-----------------------------------------------------------------------------
void CGraphWindow::on_actionZoomSelect_toggled(bool bchecked)
{
	ui->plot->ZoomToRect(bchecked);
}

//-----------------------------------------------------------------------------
void CGraphWindow::on_plot_doneZoomToRect()
{
	ui->actionZoomSelect->setChecked(false);
}

//-----------------------------------------------------------------------------
void CGraphWindow::on_options_optionsChanged()
{
	int a = ui->ops->currentOption();
	switch (a)
	{
	case 0: m_nTrackTime = TRACK_TIME; break;
	case 1: m_nTrackTime = TRACK_CURRENT_TIME; break;
	case 2: m_nTrackTime = TRACK_USER_RANGE; 
		{
			ui->ops->getUserTimeRange(m_nUserMin, m_nUserMax);

			CPostDoc* doc = GetPostDoc();
			if (doc)
			{
				// check the range. Note that user min and max are one-based!
				Post::FEPostModel* fem = doc->GetFEModel();
				int N = fem->GetStates();
				if (m_nUserMin < 1) m_nUserMin = 1;
				if (m_nUserMin > N) m_nUserMin = N;

				if (m_nUserMax != -1)
				{
					if (m_nUserMax < 1) m_nUserMax = 1;
					if (m_nUserMax > N) m_nUserMax = N;
					if (m_nUserMax < m_nUserMin)
					{
						int tmp = m_nUserMin;
						m_nUserMin = m_nUserMax;
						m_nUserMax = tmp;
					}
				}
			}

			ui->ops->setUserTimeRange(m_nUserMin, m_nUserMax);
		}
		break;
	default:
		assert(false);
		m_nTrackTime = TRACK_TIME;
	}

	bool smooth = ui->ops->lineSmoothing();
	ui->plot->setLineSmoothing(smooth);

	bool marks = ui->ops->showDataMarks();
	ui->plot->showDataMarks(marks);

	bool autoRng = ui->ops->autoRangeUpdate();
	ui->plot->setAutoRangeUpdate(autoRng);

	Update(false);
}

//=============================================================================
CDataGraphWindow::CDataGraphWindow(CMainWindow* wnd, CPostDoc* postDoc) : CGraphWindow(wnd, postDoc)
{

}

void CDataGraphWindow::SetData(const std::vector<double>& data, QString title)
{
	m_title = title;
	m_data = data;
	Update(true, true);
}

void CDataGraphWindow::Update(bool breset, bool bfit)
{
	ClearPlots();
	CPostDoc* doc = GetPostDoc();
	if (doc)
	{
		Post::FEPostModel* fem = doc->GetFEModel();
		int nsteps = fem->GetStates();
		CLineChartData* plot = new CLineChartData;
		for (int j = 0; j < nsteps; ++j)
		{
			Post::FEState* ps = fem->GetState(j);
			double yj = 0.0;
			if (j < m_data.size()) yj = m_data[j];
			plot->addPoint(ps->m_time, yj);
		}
		plot->setLabel(m_title);
		AddPlotData(plot);
		FitPlotsToData();
	}
	UpdatePlots();
}


//=============================================================================
CModelGraphWindow::CModelGraphWindow(CMainWindow* wnd, CPostDoc* postDoc) : CGraphWindow(wnd, postDoc)
{
	m_firstState = -1;
	m_lastState = -1;

	m_dataX = -1;
	m_dataY = -1;
	m_dataXPrev = -1;
	m_dataYPrev = -1;

	m_xtype = m_xtypeprev = -1;
}

//-----------------------------------------------------------------------------
// If breset==true, a new model was loaded. 
// If breset==false, the selection has changed
void CModelGraphWindow::Update(bool breset, bool bfit)
{
	CPostDoc* doc = GetPostDoc();
	if (doc==nullptr) return;

	if (breset)
	{
		int plot = GetCurrentPlotType();
		if (plot == LINE_PLOT)
			SetXDataSelector(new CTimeStepSelector(), 0);
		else
			SetXDataSelector(new CModelDataSelector(doc->GetFEModel(), Post::DATA_SCALAR));

		SetYDataSelector(new CModelDataSelector(doc->GetFEModel(), Post::DATA_SCALAR));

		m_dataXPrev = -1;
		m_dataYPrev = -1;
	}

	// Currently, when the time step changes, Update is called with breset set to false.
	// Depending on the time range settings, we may or may not need to update the track view.

	// when the user sets the range, we don't have to do anything so let's return
	//	if (m_bUserRange && (breset == false)) return;

	// Get the time step range
	int nmin = 0, nmax = 0;
	GetTimeRange(nmin, nmax);

	// plot type
	int nplotType = GetCurrentPlotType();

	// data selector values
	m_dataX = GetCurrentXValue();
	m_dataY = GetCurrentYValue();

	int ncx = m_dataX;
	switch (nplotType)
	{
	case LINE_PLOT: m_xtype = ncx; break;
	case SCATTER_PLOT: m_xtype = 2; break;
	case TIME_SCATTER_PLOT: m_xtype = 3; break;
	}

	// get the field data
	m_dataX = GetCurrentXValue();
	m_dataY = GetCurrentYValue();
	if ((nplotType != LINE_PLOT) && (m_dataX <= 0))
	{
		ClearPlots();
		return;
	}
	if (m_dataY <= 0)
	{
		ClearPlots();
		return;
	}

	// TODO: When the selection changed, breset is false, but we do need to update
	//       This if statement would prevent that, so I commented it out, but now
	//       we might be doing unneccessary updates. 
	// When a reset is not required, see if we actually need to update anything
//	if ((breset == false) && (bfit == false))
//	{
//		if ((nmin == m_firstState) && (nmax == m_lastState) && (m_dataX == m_dataXPrev) && (m_dataY == m_dataYPrev) && (m_xtype == m_xtypeprev)) return;
//	}

	// set current time point index (TODO: Not sure if this is still used)
	//	pview->SetCurrentTimeIndex(ntime);

	Post::CGLModel* po = doc->GetGLModel();
	Post::FEPostModel& fem = *doc->GetFEModel();

	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);

	// get the title
	if (nplotType == LINE_PLOT)
	{
		SetPlotTitle(GetCurrentYText());
	}
	else
	{
		QString xtext = GetCurrentXText();
		QString ytext = GetCurrentYText();

		SetPlotTitle(QString("%1 --- %2").arg(xtext).arg(ytext));
	}

	m_firstState = nmin;
	m_lastState = nmax;

	// we need to update the displacement map for all time steps
	// since the strain calculations depend on it
	int nsteps = doc->GetStates();
	Post::CGLDisplacementMap* pdm = po->GetDisplacementMap();
	if (pdm)
	{
		for (int i = 0; i<nsteps; ++i) po->GetDisplacementMap()->UpdateState(i);
	}

	// get the graph of the track view and clear it
	ClearPlots();

	// add selections
	addSelectedNodes();
	addSelectedEdges();
	addSelectedFaces();
	addSelectedElems();

	// redraw
	if ((m_dataX != m_dataXPrev) || (m_dataY != m_dataYPrev) || (m_xtype != m_xtypeprev) || bfit)
	{
		FitPlotsToData();
	}

	m_dataXPrev = m_dataX;
	m_dataYPrev = m_dataY;
	m_xtypeprev = m_xtype;

	UpdatePlots();
}

//-----------------------------------------------------------------------------
void CModelGraphWindow::addSelectedNodes()
{
	CPostDoc* doc = GetPostDoc();
	Post::FEPostModel& fem = *doc->GetFEModel();
	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);

	int nsteps = m_lastState - m_firstState + 1;
	vector<float> xdata(nsteps);
	vector<float> ydata(nsteps);

	// get the selected nodes
	int NN = mesh.Nodes();
	switch (m_xtype)
	{
	case 0: // time values
	{
		for (int i = 0; i<NN; i++)
		{
			FENode& node = mesh.Node(i);
			if (node.IsSelected())
			{
				for (int j = 0; j<nsteps; j++) xdata[j] = fem.GetState(j + m_firstState)->m_time;

				// evaluate y-field
				TrackNodeHistory(i, &ydata[0], m_dataY, m_firstState, m_lastState);

				CLineChartData* plot = new CLineChartData;
				plot->setLabel(QString("N%1").arg(i + 1));
				for (int j = 0; j<nsteps; ++j) plot->addPoint(xdata[j], ydata[j]);
				AddPlotData(plot);
			}
		}
	}
	break;
	case 1: // step values
	{
		for (int i = 0; i<NN; i++)
		{
			FENode& node = mesh.Node(i);
			if (node.IsSelected())
			{
				for (int j = 0; j<nsteps; j++) xdata[j] = (float)j + 1.f + m_firstState;

				// evaluate y-field
				TrackNodeHistory(i, &ydata[0], m_dataY, m_firstState, m_lastState);

				CLineChartData* plot = new CLineChartData;
				plot->setLabel(QString("N%1").arg(i + 1));
				for (int j = 0; j<nsteps; ++j) plot->addPoint(xdata[j], ydata[j]);
				AddPlotData(plot);
			}
		}
	}
	break;
	case 2: // scatter
	{
		for (int i = 0; i<NN; i++)
		{
			FENode& node = mesh.Node(i);
			if (node.IsSelected())
			{
				TrackNodeHistory(i, &xdata[0], m_dataX, m_firstState, m_lastState);

				// evaluate y-field
				TrackNodeHistory(i, &ydata[0], m_dataY, m_firstState, m_lastState);

				CLineChartData* plot = new CLineChartData;
				plot->setLabel(QString("N%1").arg(i + 1));
				for (int j = 0; j<nsteps; ++j) plot->addPoint(xdata[j], ydata[j]);
				AddPlotData(plot);
			}
		}
	}
	break;
	case 3: // time-scatter
	{
		vector<int> sel;
		for (int i = 0; i < NN; i++)
		{
			FENode& node = mesh.Node(i);
			if (node.IsSelected()) sel.push_back(i);
		}

		if (sel.empty() == false)
		{
			int states = fem.GetStates();

			int state0 = m_firstState;
			int state1 = m_lastState;

			if (state0 < 0) state0 = 0;
			if (state0 >= states) state0 = states - 1;

			if (state1 < 0) state1 = 0;
			if (state1 >= states) state1 = states - 1;

			if (state1 < state0)
			{
				int tmp = state0;
				state0 = state1;
				state1 = tmp;
			}

			int nsteps = state1 - state0 + 1;
			if (nsteps > 32) nsteps = 32;
			for (int i = state0; i < state0 + nsteps; ++i)
			{
				CLineChartData* plot = new CLineChartData;
				plot->setLabel(QString("%1").arg(fem.GetState(i)->m_time));
				AddPlotData(plot);
			}

			for (int i = 0; i < (int)sel.size(); i++)
			{
				FENode& node = mesh.Node(sel[i]);
				if (node.IsSelected())
				{
					// evaluate x-field
					TrackNodeHistory(sel[i], &xdata[0], m_dataX, state0, state0 + nsteps - 1);

					// evaluate y-field
					TrackNodeHistory(sel[i], &ydata[0], m_dataY, state0, state0 + nsteps - 1);

					for (int j = 0; j < nsteps; ++j)
					{
						CPlotData& p = GetPlotWidget()->getPlotData(j);
						p.addPoint(xdata[j], ydata[j]);
					}
				}
			}

			// sort the plots 
			int nplots = GetPlotWidget()->plots();
			for (int i = 0; i < nplots; ++i)
			{
				CPlotData& data = GetPlotWidget()->getPlotData(i);
				data.sort();
			}
		}
	}
	break;
	}
}

//-----------------------------------------------------------------------------
void CModelGraphWindow::addSelectedEdges()
{
	CPostDoc* doc = GetPostDoc();
	Post::FEPostModel& fem = *doc->GetFEModel();
	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);

	int nsteps = m_lastState - m_firstState + 1;
	vector<float> xdata(nsteps);
	vector<float> ydata(nsteps);

	// get the selected nodes
	int NL = mesh.Edges();
	for (int i = 0; i<NL; i++)
	{
		FEEdge& edge = mesh.Edge(i);
		if (edge.IsSelected())
		{
			// evaluate x-field
			switch (m_xtype)
			{
			case 0:
				for (int j = 0; j<nsteps; j++) xdata[j] = fem.GetState(j + m_firstState)->m_time;
				break;
			case 1:
				for (int j = 0; j<nsteps; j++) xdata[j] = (float)j + 1.f + m_firstState;
				break;
			default:
				TrackEdgeHistory(i, &xdata[0], m_dataX, m_firstState, m_lastState);
			}

			// evaluate y-field
			TrackEdgeHistory(i, &ydata[0], m_dataY, m_firstState, m_lastState);

			CLineChartData* plot = new CLineChartData;
			plot->setLabel(QString("L%1").arg(i + 1));
			for (int j = 0; j<nsteps; ++j) plot->addPoint(xdata[j], ydata[j]);
			AddPlotData(plot);
		}
	}
}

//-----------------------------------------------------------------------------
void CModelGraphWindow::addSelectedFaces()
{
	CPostDoc* doc = GetPostDoc();
	Post::FEPostModel& fem = *doc->GetFEModel();
	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);

	int nsteps = m_lastState - m_firstState + 1;
	vector<float> xdata(nsteps);
	vector<float> ydata(nsteps);

	// get the selected faces
	int NF = mesh.Faces();
	for (int i = 0; i<NF; ++i)
	{
		FEFace& f = mesh.Face(i);
		if (f.IsSelected())
		{
			// evaluate x-field
			switch (m_xtype)
			{
			case 0:
				for (int j = 0; j<nsteps; j++) xdata[j] = fem.GetState(j + m_firstState)->m_time;
				break;
			case 1:
				for (int j = 0; j<nsteps; j++) xdata[j] = (float)j + 1.f + m_firstState;
				break;
			default:
				TrackFaceHistory(i, &xdata[0], m_dataX, m_firstState, m_lastState);
			}

			// evaluate y-field
			TrackFaceHistory(i, &ydata[0], m_dataY, m_firstState, m_lastState);

			CLineChartData* plot = new CLineChartData;
			plot->setLabel(QString("F%1").arg(i + 1));
			for (int j = 0; j<nsteps; ++j) plot->addPoint(xdata[j], ydata[j]);
			AddPlotData(plot);
		}
	}
}

//-----------------------------------------------------------------------------
void CModelGraphWindow::addSelectedElems()
{
	CPostDoc* doc = GetPostDoc();
	Post::FEPostModel& fem = *doc->GetFEModel();
	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);

	int nsteps = m_lastState - m_firstState + 1;
	vector<float> xdata(nsteps);
	vector<float> ydata(nsteps);

	// get the selected elements
	int NE = mesh.Elements();
	switch (m_xtype)
	{
	case 0:
		for (int i = 0; i < NE; i++)
		{
			FEElement_& e = mesh.ElementRef(i);
			if (e.IsSelected())
			{
				// evaluate x-field
				for (int j = 0; j < nsteps; j++) xdata[j] = fem.GetState(j + m_firstState)->m_time;

				// evaluate y-field
				TrackElementHistory(i, &ydata[0], m_dataY, m_firstState, m_lastState);

				CLineChartData* plot = new CLineChartData;
				for (int j = 0; j < nsteps; ++j) plot->addPoint(xdata[j], ydata[j]);
				plot->setLabel(QString("E%1").arg(i + 1));
				AddPlotData(plot);
			}
		}
		break;
	case 1:
		for (int i = 0; i < NE; i++)
		{
			FEElement_& e = mesh.ElementRef(i);
			if (e.IsSelected())
			{
				// evaluate x-field
				for (int j = 0; j < nsteps; j++) xdata[j] = (float)j + 1.f + m_firstState;

				// evaluate y-field
				TrackElementHistory(i, &ydata[0], m_dataY, m_firstState, m_lastState);

				CLineChartData* plot = new CLineChartData;
				for (int j = 0; j < nsteps; ++j) plot->addPoint(xdata[j], ydata[j]);
				plot->setLabel(QString("E%1").arg(i + 1));
				AddPlotData(plot);
			}
		}
		break;
	case 2:
		for (int i = 0; i < NE; i++)
		{
			FEElement_& e = mesh.ElementRef(i);
			if (e.IsSelected())
			{
				// evaluate x-field
				TrackElementHistory(i, &xdata[0], m_dataX, m_firstState, m_lastState);

				// evaluate y-field
				TrackElementHistory(i, &ydata[0], m_dataY, m_firstState, m_lastState);

				CLineChartData* plot = new CLineChartData;
				for (int j = 0; j < nsteps; ++j) plot->addPoint(xdata[j], ydata[j]);
				plot->setLabel(QString("E%1").arg(i + 1));
				AddPlotData(plot);
			}
		}
		break;
	case 3:	// time-scatter
	{
		vector<int> sel;
		for (int i = 0; i < NE; i++)
		{
			FEElement_& e = mesh.ElementRef(i);
			if (e.IsSelected()) sel.push_back(i);
		}

		if (sel.empty() == false)
		{
			int nsteps = m_lastState - m_firstState + 1;
			if (nsteps > 32) nsteps = 32;
			for (int i = m_firstState; i < m_firstState + nsteps; ++i)
			{
				CLineChartData* plot = new CLineChartData;
				plot->setLabel(QString("%1").arg(fem.GetState(i)->m_time));
				AddPlotData(plot);
			}

			for (int i = 0; i < (int)sel.size(); i++)
			{
				FEElement_& e = mesh.ElementRef(sel[i]);
				if (e.IsSelected())
				{
					// evaluate x-field
					TrackElementHistory(i, &xdata[0], m_dataX, m_firstState, m_lastState);

					// evaluate y-field
					TrackElementHistory(i, &ydata[0], m_dataY, m_firstState, m_lastState);

					for (int j = 0; j < nsteps; ++j)
					{
						CPlotData& p = GetPlotWidget()->getPlotData(j);
						p.addPoint(xdata[j], ydata[j]);
					}
				}
			}

			// sort the plots 
			CPlotWidget* w = GetPlotWidget();
			int nplots = w->plots();
			for (int i = 0; i < nplots; ++i)
			{
				CPlotData& data = GetPlotWidget()->getPlotData(i);
				data.sort();
			}

			if (w->autoRangeUpdate())
				w->fitToData(false);
		}
	}
	break;
	}
}

//-----------------------------------------------------------------------------
// Calculate time history of a node
void CModelGraphWindow::TrackNodeHistory(int node, float* pval, int nfield, int nmin, int nmax)
{
	CPostDoc* doc = GetPostDoc();
	Post::FEPostModel& fem = *doc->GetFEModel();

	int nsteps = fem.GetStates();
	if (nmin <       0) nmin = 0;
	if (nmax == -1) nmax = nsteps - 1;
	if (nmax >= nsteps) nmax = nsteps - 1;
	if (nmax <    nmin) nmax = nmin;
	int nn = nmax - nmin + 1;

	Post::NODEDATA nd;
	for (int n = 0; n<nn; n++)
	{
		fem.EvaluateNode(node, n + nmin, nfield, nd);
		pval[n] = nd.m_val;
	}
}

//-----------------------------------------------------------------------------
// Calculate time history of a edge
void CModelGraphWindow::TrackEdgeHistory(int edge, float* pval, int nfield, int nmin, int nmax)
{
	CPostDoc* doc = GetPostDoc();
	Post::FEPostModel& fem = *doc->GetFEModel();

	int nsteps = fem.GetStates();
	if (nmin <       0) nmin = 0;
	if (nmax == -1) nmax = nsteps - 1;
	if (nmax >= nsteps) nmax = nsteps - 1;
	if (nmax <    nmin) nmax = nmin;
	int nn = nmax - nmin + 1;

	Post::EDGEDATA nd;
	for (int n = 0; n<nn; n++)
	{
		fem.EvaluateEdge(edge, n + nmin, nfield, nd);
		pval[n] = nd.m_val;
	}
}

//-----------------------------------------------------------------------------
// Calculate time history of a face
void CModelGraphWindow::TrackFaceHistory(int nface, float* pval, int nfield, int nmin, int nmax)
{
	CPostDoc* doc = GetPostDoc();
	Post::FEPostModel& fem = *doc->GetFEModel();

	int nsteps = fem.GetStates();
	if (nmin <       0) nmin = 0;
	if (nmax == -1) nmax = nsteps - 1;
	if (nmax >= nsteps) nmax = nsteps - 1;
	if (nmax <    nmin) nmax = nmin;
	int nn = nmax - nmin + 1;

	float data[FEFace::MAX_NODES], val;
	for (int n = 0; n<nn; n++)
	{
		fem.EvaluateFace(nface, n + nmin, nfield, data, val);
		pval[n] = val;
	}
}

//-----------------------------------------------------------------------------
// Calculate time history of an element
void CModelGraphWindow::TrackElementHistory(int nelem, float* pval, int nfield, int nmin, int nmax)
{
	CPostDoc* doc = GetPostDoc();
	Post::FEPostModel& fem = *doc->GetFEModel();

	int nsteps = fem.GetStates();
	if (nmin <       0) nmin = 0;
	if (nmax == -1) nmax = nsteps - 1;
	if (nmax >= nsteps) nmax = nsteps - 1;
	if (nmax <    nmin) nmax = nmin;
	int nn = nmax - nmin + 1;

	float data[FEElement::MAX_NODES] = { 0.f }, val;
	for (int n = 0; n<nn; n++)
	{
		fem.EvaluateElement(nelem, n + nmin, nfield, data, val);
		pval[n] = val;
	}
}
