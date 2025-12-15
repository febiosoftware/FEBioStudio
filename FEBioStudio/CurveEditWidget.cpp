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
#include "CurveEditWidget.h"
#include <QComboBox>
#include <QBoxLayout>
#include <QToolButton>
#include <QLabel>
#include <QLineEdit>
#include <QApplication>
#include <QClipboard>
#include <QtCore/QMimeData>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <CUILib/PlotWidget.h>
#include "IconProvider.h"
#include <FSCore/LoadCurve.h>
#include "Command.h"
#include "DlgFormula.h"
#include "DlgImportData.h"

using namespace std;

class CCmdAddPoint : public CCommand
{
public:
	CCmdAddPoint(LoadCurve* plc, const vec2d& p);

	void Execute() override;
	void UnExecute() override;

	int Index() { return m_index; }

private:
	LoadCurve* m_lc;
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
	LoadCurve* m_lc;
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
		math->setIcon(CIconProvider::GetIcon("formula"));

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

void CCurveEditWidget::SetXRange(double xmin, double xmax)
{
	ui->plt->SetHighlightInterval(xmin, xmax);
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

	if (!clipboard->mimeData()->hasText()) return;

	QString data = clipboard->text();

	// If this data was copied from the widget itself, we need 
	// to chop off the first row since it's the row labels
	bool sameData = false;
	if (data == ui->m_clipboard_backup)
	{
		int pos = data.indexOf("\n");
		data = data.right(data.size() - pos);

		sameData = true;
	}

	CDlgImportData dlg(data, DataType::DOUBLE, 2);

	bool getValues = false;

	// If this data was copied from the widget itself, we know its
	// format is valid, and so we don't need to open the dialog
	if (sameData)
	{
		getValues = true;
	}
	else if (dlg.exec())
	{
		getValues = true;
	}

	if (getValues)
	{
		QList<QStringList> values = dlg.GetValues();

		plc->Clear();
		for (auto row : values)
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
