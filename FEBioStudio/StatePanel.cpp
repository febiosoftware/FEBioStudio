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
#include "StatePanel.h"
#include "MainWindow.h"
#include "Document.h"
#include <PostLib/FEPostModel.h>
#include <QBoxLayout>
#include <QTableWidget>
#include <QLabel>
#include <QHeaderView>
#include <QtCore/QAbstractTableModel>
#include <QPushButton>
#include <QMessageBox>
#include <QInputDialog>
#include <QFormLayout>
#include <QCheckBox>
#include <QDialogButtonBox>
#include "PostDocument.h"
#include "GLModelDocument.h"

class CStateModel : public QAbstractTableModel
{
public:
	CStateModel(QWidget* parent) : QAbstractTableModel(parent), m_fem(0)
	{
	}

	int rowCount(const QModelIndex& parent) const
	{
		if (m_fem) return m_fem->GetStates();
		return 0;
	}

	int columnCount(const QModelIndex& parent) const
	{
		return 3;
	}

	void SetFEModel(Post::FEPostModel* pfem)
	{
		beginResetModel();
		m_fem = pfem;
		endResetModel();
	}

	QVariant headerData(int section, Qt::Orientation orient, int role) const
	{
		if ((orient == Qt::Horizontal)&&(role == Qt::DisplayRole))
		{
			switch (section)
			{
			case 0: return QVariant(QString("State")); break;
			case 1: return QVariant(QString("Time")); break;
			case 2: return QVariant(QString("Status")); break;
			}
		}
		return QAbstractTableModel::headerData(section, orient, role);
	}

	QVariant data(const QModelIndex& index, int role) const
	{
		if (m_fem == 0) return QVariant();

		if (!index.isValid()) return QVariant();

		int c = index.column();
		int r = index.row();
		if (role == Qt::DisplayRole)
		{
			switch (c)
			{
			case 0: return r + 1; break;
			case 1:
			{
				Post::FEState* s = m_fem->GetState(r);
				if (s) return s->m_time;
			}
			break;
			case 2:
			{
				Post::FEState* s = m_fem->GetState(r);
				if (s) return s->m_status;
			}
			break;
			}
		}
		if (role == Qt::DecorationRole)
		{
			if (c == 2)
			{
				Post::FEState* s = m_fem->GetState(r);
				if (s)
				{
					switch (s->m_status)
					{
					case 0: return QColor(Qt::green ); break;
					case 1: return QColor(Qt::red   ); break;
					case 2: return QColor(Qt::yellow); break;
					}
				}
			}
		}
		return QVariant();
	}

private:
	Post::FEPostModel*	m_fem;
};

class Ui::CDlgAddState
{
public:
	QLineEdit* pstates;
	QLineEdit* pstart;
	QLineEdit* pend;
	QLineEdit* pstatus;
	QCheckBox* pinter;

public:
	void setupUi(QDialog *parent)
	{
		QVBoxLayout* pv = new QVBoxLayout(parent); 

		QFormLayout* pform = new QFormLayout;
		pform->addRow("states:"    , pstates = new QLineEdit); pstates->setValidator(new QIntValidator(1, 1000));
		pform->addRow("start time:", pstart  = new QLineEdit); pstart ->setValidator(new QDoubleValidator(-1e99, 1e99, 6));
		pform->addRow("end time:"   , pend    = new QLineEdit); pend   ->setValidator(new QDoubleValidator(-1e99, 1e99, 6));
		pform->addRow("status flag:", pstatus = new QLineEdit); pstatus->setValidator(new QIntValidator(0, 100));
		pform->addRow("", pinter = new QCheckBox("Interpolate data")); pinter->setChecked(true);
		pstatus->setText(QString::number(0));

		pv->addLayout(pform);

		QDialogButtonBox* b = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		pv->addWidget(b);

		QObject::connect(b, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(b, SIGNAL(rejected()), parent, SLOT(reject()));
	}
};

CDlgAddState::CDlgAddState(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgAddState)
{
	m_nstates = 0;
	m_minTime = 0;
	m_maxTime = 1;
	m_status = 0;
	m_interpolate = true;
	ui->setupUi(this);
}

void CDlgAddState::accept()
{
	m_nstates = ui->pstates->text().toInt();
	m_minTime = ui->pstart ->text().toDouble();
	m_maxTime = ui->pend   ->text().toDouble();
	m_status  = ui->pstatus->text().toInt();
	m_interpolate = ui->pinter->isChecked();

	QDialog::accept();
}

class QFilterDialog : public QDialog
{
public:
	QFilterDialog(QWidget* parent = nullptr) : QDialog(parent)
	{
		QVBoxLayout* l = new QVBoxLayout;

		l->addWidget(new QLabel("Enter a scale and offset to adjust the time values of all states."));

		QFormLayout* f = new QFormLayout;
		f->addRow("scale" , m_scale  = new QLineEdit); m_scale ->setValidator(new QDoubleValidator);
		f->addRow("offset", m_offset = new QLineEdit); m_offset->setValidator(new QDoubleValidator);
		l->addLayout(f);

		m_scale ->setText(QString::number(1.0));
		m_offset->setText(QString::number(0.0));

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		l->addWidget(bb);

		setLayout(l);

		QObject::connect(bb, SIGNAL(accepted()), this, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), this, SLOT(reject()));
	}

	double GetScaleFactor() { return m_scale->text().toDouble(); }
	double GetOffset() { return m_offset->text().toDouble(); }

private:
	QLineEdit* m_scale;
	QLineEdit* m_offset;
};

class Ui::CStatePanel
{
public:
	CStateModel*	data;
	QTableView*		list;

public:
	void setupUi(::CStatePanel* parent)
	{
		QVBoxLayout* pg = new QVBoxLayout(parent);
		pg->setContentsMargins(0,0,0,0);

		QPushButton* pAdd  = new QPushButton("Add..." ); pAdd ->setObjectName("addButton"   );
		QPushButton* pEdit = new QPushButton("Edit..."); pEdit->setObjectName("editButton"  );
		QPushButton* pDel  = new QPushButton("Delete" ); pDel ->setObjectName("deleteButton");
		QPushButton* pFlt  = new QPushButton("Filter..." ); pFlt->setObjectName("filterButton");
		QHBoxLayout* ph = new QHBoxLayout;
		ph->setSpacing(0);
		ph->addWidget(pAdd);
		ph->addWidget(pEdit);
		ph->addWidget(pDel);
		ph->addWidget(pFlt);
		ph->addStretch();

		pg->addLayout(ph);

		list = new QTableView;
		list->setObjectName(QStringLiteral("stateList"));
		list->setSelectionBehavior(QAbstractItemView::SelectRows);
		list->setSelectionMode(QAbstractItemView::ExtendedSelection);
		list->horizontalHeader()->show();
		list->horizontalHeader()->setStretchLastSection(true);
		list->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
//		list->verticalHeader()->setDefaultSectionSize(24);
		list->verticalHeader()->hide();

		data = new CStateModel(list);
		list->setModel(data);

		pg->addWidget(list);


		QMetaObject::connectSlotsByName(parent);
	}
};

CStatePanel::CStatePanel(CMainWindow* pwnd, QWidget* parent) : CCommandPanel(pwnd, parent), ui(new Ui::CStatePanel)
{
	ui->setupUi(this);
}

CGLModelDocument* CStatePanel::GetActiveDocument()
{
	return dynamic_cast<CGLModelDocument*>(GetMainWindow()->GetDocument());
}

void CStatePanel::Update(bool breset)
{
	if (breset)
	{
		CGLModelDocument* pdoc = GetActiveDocument();
		Post::FEPostModel* fem = (pdoc ? pdoc->GetFSModel() : nullptr);
		ui->data->SetFEModel(fem);
	}
}

void CStatePanel::on_stateList_doubleClicked(const QModelIndex& index)
{
	GetMainWindow()->SetCurrentState(index.row());
}

void CStatePanel::on_addButton_clicked()
{
	CPostDocument* doc = dynamic_cast<CPostDocument*>(GetActiveDocument());
	if ((doc == nullptr) || (doc->IsValid() == false)) return;

	CDlgAddState dlg(this);
	if (dlg.exec())
	{
		Post::FEPostModel& fem = *doc->GetFSModel();
		int N = dlg.m_nstates;
		int M = (N < 2 ? 1 : N - 1);
		double t0 = dlg.m_minTime;
		double t1 = dlg.m_maxTime;
		for (int i=0; i<N; ++i)
		{
			double f = t0 + i*(t1 - t0)/M;
			fem.AddState(f, dlg.m_status, dlg.m_interpolate);
		}

		// reset everything that depends on the number of states
		TIMESETTINGS& ts = doc->GetTimeSettings();
		ts.m_end = fem.GetStates() - 1;
		GetMainWindow()->UpdatePostToolbar();
		GetMainWindow()->Update(this, true);
		Update(true);
	}
}

void CStatePanel::on_editButton_clicked()
{
	CPostDocument* doc = dynamic_cast<CPostDocument*>(GetActiveDocument());
	if ((doc == nullptr) || (doc->IsValid() == false)) return;

	Post::FEPostModel& fem = *doc->GetFSModel();
	QItemSelectionModel* selection = ui->list->selectionModel();
	QModelIndexList selRows = selection->selectedRows();
	int ncount = selRows.count();
	if (ncount != 1) return;

	QModelIndex sel = selRows.at(0);
	int nrow = sel.row();

	Post::FEState* pstate = fem.GetState(nrow);
	double ftime = pstate->m_time;

	bool bok = false;
	double newTime = QInputDialog::getDouble(this, QString("Edit state %1").arg(nrow), "new time:", ftime, -1e99, 1e99, 6, &bok);
	if (bok)
	{
		pstate->m_time = newTime;
		fem.DeleteState(nrow);
		fem.InsertState(pstate, newTime);
		Update(true);
	}
}

void CStatePanel::on_deleteButton_clicked()
{
	CPostDocument* doc = dynamic_cast<CPostDocument*>(GetActiveDocument());
	if ((doc == nullptr) || (doc->IsValid() == false)) return;

	Post::FEPostModel& fem = *doc->GetFSModel();
	QItemSelectionModel* selection = ui->list->selectionModel();
	QModelIndexList selRows = selection->selectedRows();

	QString s = QString("Are you sure you want to delete %1 state(s)?").arg(selRows.count());
	if (QMessageBox::question(this, "Delete States", s, QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		// delete states
		int m = 0;
		for (int i=0; i<selRows.count(); ++i)
		{
			int nrow = selRows.at(i).row();
			fem.DeleteState(nrow-m); 
			++m;
		}
		Update(true);

		int states = fem.GetStates();

		TIMESETTINGS& ts = doc->GetTimeSettings();
		if (ts.m_end >= states) ts.m_end = states - 1;
		int n = doc->GetActiveState();
		if (n >= states - 1) n = states - 1;
		doc->SetActiveState(n);
		GetMainWindow()->UpdatePostToolbar();
		GetMainWindow()->Update(this, true);
	}
}

void CStatePanel::on_filterButton_clicked()
{
	CPostDocument* doc = dynamic_cast<CPostDocument*>(GetActiveDocument());
	if ((doc == nullptr) || (doc->IsValid() == false)) return;

	Post::FEPostModel& fem = *doc->GetFSModel();
	QFilterDialog dlg;
	if (dlg.exec())
	{
		float scale = dlg.GetScaleFactor();
		float offset = dlg.GetOffset();

		if (scale <= 0) { QMessageBox::critical(this, "Filter", "Scale factor must be positive number."); return; }
		if (offset < 0) { QMessageBox::critical(this, "Filter", "Offset must be non-negative number."); return; }

		int states = fem.GetStates();
		for (int i = 0; i < states; ++i)
		{
			Post::FEState* ps = fem.GetState(i);
			float t = ps->m_time;
			ps->m_time = scale * t + offset;
		}

		GetMainWindow()->UpdatePostToolbar();
		GetMainWindow()->Update(this, true);
	}
}
