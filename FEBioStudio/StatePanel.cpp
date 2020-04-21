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
#include <QDialogButtonBox>
#include "PostDoc.h"

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
		return 2;
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
			}
		}
		return QAbstractTableModel::headerData(section, orient, role);
	}

	QVariant data(const QModelIndex& index, int role) const
	{
		if (m_fem == 0) return QVariant();

		if (!index.isValid()) return QVariant();
		if (role == Qt::DisplayRole)
		{
			if (index.column() == 0) return index.row()+1;
			else 
			{
				Post::FEState* s = m_fem->GetState(index.row());
				if (s) return s->m_time;
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

public:
	void setupUi(QDialog *parent)
	{
		QVBoxLayout* pv = new QVBoxLayout(parent);

		QFormLayout* pform = new QFormLayout;
		pform->addRow("states:"    , pstates = new QLineEdit); pstates->setValidator(new QIntValidator(1, 1000));
		pform->addRow("start time:", pstart  = new QLineEdit); pstart ->setValidator(new QDoubleValidator(-1e99, 1e99, 6));
		pform->addRow("end time"   , pend    = new QLineEdit); pend   ->setValidator(new QDoubleValidator(-1e99, 1e99, 6));

		pv->addLayout(pform);

		QDialogButtonBox* b = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		pv->addWidget(b);

		QObject::connect(b, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(b, SIGNAL(rejected()), parent, SLOT(reject()));
	}
};

CDlgAddState::CDlgAddState(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgAddState)
{
	ui->setupUi(this);
}

void CDlgAddState::accept()
{
	m_nstates = ui->pstates->text().toInt();
	m_minTime = ui->pstart->text().toDouble();
	m_maxTime = ui->pend  ->text().toDouble();

	QDialog::accept();
}

class Ui::CStatePanel
{
public:
	CStateModel*	data;
	QTableView*		list;

public:
	void setupUi(::CStatePanel* parent)
	{
		QVBoxLayout* pg = new QVBoxLayout(parent);
		pg->setMargin(0);

		QPushButton* pAdd  = new QPushButton("Add..." ); pAdd ->setObjectName("addButton"   );// pAdd ->setFixedWidth(60);
		QPushButton* pEdit = new QPushButton("Edit..."); pEdit->setObjectName("editButton"  ); //pEdit->setFixedWidth(60);
		QPushButton* pDel  = new QPushButton("Delete" ); pDel ->setObjectName("deleteButton"); //pDel ->setFixedWidth(60);
		QHBoxLayout* ph = new QHBoxLayout;
		ph->setSpacing(0);
		ph->addWidget(pAdd);
		ph->addWidget(pEdit);
		ph->addWidget(pDel);
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

CPostDoc* CStatePanel::GetActiveDocument()
{
	CDocument* doc = GetMainWindow()->GetDocument();
	if (doc->FEBioJobs() == 0) return nullptr;
	return GetMainWindow()->GetActiveDocument();
}

void CStatePanel::Update()
{
	Update(true);
}

void CStatePanel::Update(bool breset)
{
	if (breset)
	{
		CPostDoc* pdoc = GetActiveDocument();
		Post::FEPostModel* fem = (pdoc ? pdoc->GetFEModel() : nullptr);
		ui->data->SetFEModel(fem);
	}
}

void CStatePanel::on_stateList_doubleClicked(const QModelIndex& index)
{
	GetMainWindow()->SetCurrentState(index.row());
}

void CStatePanel::on_addButton_clicked()
{
	CPostDoc& doc = *GetActiveDocument();
	if (doc.IsValid() == false) return;

	CDlgAddState dlg(this);
	if (dlg.exec())
	{
		Post::FEPostModel& fem = *doc.GetFEModel();
		int N = dlg.m_nstates;
		int M = (N < 2 ? 1 : N - 1);
		double t0 = dlg.m_minTime;
		double t1 = dlg.m_maxTime;
		for (int i=0; i<N; ++i)
		{
			double f = t0 + i*(t1 - t0)/M;
			fem.AddState(f);
		}
		Update(true);
	}
}

void CStatePanel::on_editButton_clicked()
{
	CPostDoc& doc = *GetActiveDocument();
	if (doc.IsValid() == false) return;

	Post::FEPostModel& fem = *doc.GetFEModel();
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
	CPostDoc& doc = *GetActiveDocument();
	if (doc.IsValid() == false) return;

	Post::FEPostModel& fem = *doc.GetFEModel();
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

		TIMESETTINGS& ts = doc.GetTimeSettings();
		if (ts.m_end >= states) ts.m_end = states - 1;
		int n = doc.GetActiveState();
		if (n >= states - 1) n = states - 1;
		doc.SetActiveState(n);
		GetMainWindow()->UpdatePostToolbar();
		GetMainWindow()->Update(this);
	}
}
