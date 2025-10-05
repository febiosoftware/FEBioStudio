/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2025 University of Utah, The Trustees of Columbia University in
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
#include "FEBioBatchView.h"
#include "FEBioBatchDoc.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QMenu>
#include <QDialogButtonBox>
#include "MainWindow.h"
#include <QSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QToolButton>
#include <map>
#include <set>
#include <QRegularExpression>
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>

class BatchRunFilter
{
	enum FltType {
		INVALID,
		STRING,
		REGEX,
		CONDITION
	};

	enum CondOp {
		LESS,
		LESS_EQUAL,
		GREATER,
		GREATER_EQUAL,
		EQUAL,
		NOT_EQUAL
	};

	enum Variable {
		TIMESTEPS,
		ITERS,
		REFORMS,
		NRHS,
		RUNTIME,
		NORM,
		STATUS
	};

	struct Condition {
		Variable var = TIMESTEPS;
		int op = 0;
		double val = 0;
	};

public:
	BatchRunFilter() {}

	bool isValid() const { return (m_type != INVALID); }

	void SetFilter(const QString& flt)
	{
		m_filter = flt;
		process();
	}

private:
	void process()
	{
		m_type = INVALID;
		if (!m_filter.isEmpty())
		{
			QChar c = m_filter[0];
			if (c == '/')
			{
				m_regex.setPattern(m_filter.mid(1));
				if (m_regex.isValid()) m_type = REGEX;
			}
			else if (c == '=')
			{
				QString s = m_filter.mid(1);
				QRegularExpression re("[<>!=]");
				int n = s.indexOf(re);
				if (n > 0)
				{
					QString var = s.left(n);

					if      (var == "timesteps") m_cond.var = TIMESTEPS;
					else if (var == "iters"    ) m_cond.var = ITERS;
					else if (var == "reforms"  ) m_cond.var = REFORMS;
					else if (var == "nrhs"     ) m_cond.var = NRHS;
					else if (var == "runtime"  ) m_cond.var = RUNTIME;
					else if (var == "norm"     ) m_cond.var = NORM;
					else if (var == "status"   ) m_cond.var = STATUS;
					else return;

					QString op(s[n]);
					if (n < s.length() - 1)
					{
						QChar c1 = s[n+1];
						if (c1 == '=') {
							op += c1; n++;
						}
					}
					else return;

					if      (op == "<" ) m_cond.op = LESS;
					else if (op == "<=") m_cond.op = LESS_EQUAL;
					else if (op == "=" ) m_cond.op = EQUAL;
					else if (op == "==") m_cond.op = EQUAL;
					else if (op == "!=") m_cond.op = NOT_EQUAL;
					else if (op == ">" ) m_cond.op = GREATER;
					else if (op == ">=") m_cond.op = GREATER_EQUAL;

					if (n < s.length() - 1)
					{
						QString val = s.mid(n + 1);
						bool ok = false;
						m_cond.val = val.toDouble(&ok);

						if (ok) m_type = CONDITION;
					}
				}
			}
			else m_type = STRING;
		}
	}

public:
	bool matches(const FEBioBatchDoc::JobInfo& d)
	{
		if (m_type == INVALID) return false;

		if (m_type == STRING) return d.fileName.contains(m_filter);
		else if (m_type == REGEX)
		{
			QRegularExpressionMatch m = m_regex.match(d.fileName);
			return m.hasMatch();
		}
		else if (m_type == CONDITION)
		{
			double val = 0;
			switch (m_cond.var)
			{
			case TIMESTEPS: val = d.stats.timeSteps; break;
			case ITERS    : val = d.stats.iterations; break;
			case REFORMS  : val = d.stats.reformations; break;
			case NRHS     : val = d.stats.nrhs; break;
			case RUNTIME  : val = d.stats.elapsedTime; break;
			case NORM     : val = d.stats.solutionNorm; break;
			case STATUS   : val = (d.status == FEBioBatchDoc::FINISHED); break;
			default:
				assert(false);
				return false;
			}

			switch (m_cond.op)
			{
			case LESS         : return (val <  m_cond.val); break;
			case LESS_EQUAL   : return (val <= m_cond.val); break;
			case GREATER      : return (val >  m_cond.val); break;
			case GREATER_EQUAL: return (val >= m_cond.val); break;
			case EQUAL        : return (val == m_cond.val); break;
			case NOT_EQUAL    : return (val != m_cond.val); break;
			default:
				assert(false);
				return false;
			}
		}

		return false;
	}

private:
	QString m_filter;
	QRegularExpression m_regex;
	Condition m_cond;
	FltType m_type = INVALID;
};

class FEBioBatchViewUI
{
public:
	QPushButton* start = nullptr;
	QPushButton* cancel = nullptr;
	QPushButton* ops = nullptr;
	QTableWidget* table = nullptr;
	QLineEdit* filter = nullptr;
	QToolButton* add = nullptr;
	QToolButton* del = nullptr;
	QToolButton* copy = nullptr;

	std::map<int, int> rowmap;

	BatchRunFilter m_flt;

public:
	void setup(FEBioBatchView* w)
	{
		QHBoxLayout* h = new QHBoxLayout;
		h->setContentsMargins(0, 0, 0, 0);

		ops = new QPushButton("Options ...");
		h->addWidget(ops);

		start = new QPushButton("Start");
		QMenu* menu = new QMenu(start);
		QAction* runAll = menu->addAction("Run all");
		QAction* runSel = menu->addAction("Run selected");
		start->setMenu(menu);
		h->addWidget(start);

		cancel = new QPushButton("Cancel");
		menu = new QMenu(cancel);
		QAction* cancelAll = menu->addAction("Cancel all");
		QAction* cancelPen = menu->addAction("Cancel pending");
		cancel->setMenu(menu);
		cancel->setEnabled(false);
		h->addWidget(cancel);

		h->addWidget(new QLabel("filter:"));
		filter = new QLineEdit;
		filter->setPlaceholderText("string, /regex, =condition");
		filter->setToolTip("Filter the list of files");
		h->addWidget(filter);

		h->addStretch();

		add = new QToolButton;
		add->setIcon(QIcon(":/icons/selectAdd.png"));
		add->setToolTip("Add files");
		h->addWidget(add);

		del = new QToolButton;
		del->setIcon(QIcon(":/icons/selectSub.png"));
		del->setToolTip("Remove selected files");
		h->addWidget(del);

		copy = new QToolButton; 
		copy->setIcon(QIcon(":/icons/clipboard.png")); 
		copy->setToolTip("Copy to clipboard");
		h->addWidget(copy);

		table = new QTableWidget(w);
		table->setColumnCount(8);
		QStringList headers;
		headers << "file" << "status" << "timesteps" << "iters" << "nrhs" << "reforms" << "norm" << "runtime (sec.)";
		table->setHorizontalHeaderLabels(headers);
		table->setEditTriggers(QAbstractItemView::NoEditTriggers);
		table->setSelectionBehavior(QAbstractItemView::SelectRows);
		table->setSelectionMode(QAbstractItemView::SingleSelection);
		table->setShowGrid(false);
		table->setAlternatingRowColors(true);
//		table->verticalHeader()->setVisible(false);
		table->setSelectionMode(QAbstractItemView::ExtendedSelection);
		table->setContextMenuPolicy(Qt::CustomContextMenu);

		table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

		for (int i = 1; i < table->columnCount(); ++i)
		{
			table->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Fixed);
			table->horizontalHeader()->resizeSection(i, 120);
		}

		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(h);
		l->addWidget(table);
		w->setLayout(l);

		QObject::connect(runAll, &QAction::triggered, w, &FEBioBatchView::on_runAll);
		QObject::connect(runSel, &QAction::triggered, w, &FEBioBatchView::on_runSelected);
		QObject::connect(cancelAll, &QAction::triggered, w, &FEBioBatchView::on_cancelAll);
		QObject::connect(cancelPen, &QAction::triggered, w, &FEBioBatchView::on_cancelPending);
		QObject::connect(table, &QTableWidget::customContextMenuRequested, w, &FEBioBatchView::showContextMenu);
		QObject::connect(ops, &QPushButton::clicked, w, &FEBioBatchView::on_options);
		QObject::connect(filter, &QLineEdit::textChanged, w, &FEBioBatchView::on_filter);
		QObject::connect(copy, &QToolButton::clicked, w, &FEBioBatchView::on_clipboard);
		QObject::connect(add, &QToolButton::clicked, w, &FEBioBatchView::on_addBatchFiles);
		QObject::connect(del, &QToolButton::clicked, w, &FEBioBatchView::on_removeSelectedFiles);
	}

	void enableButtons(bool b)
	{
		start->setEnabled(b);
		ops->setEnabled(b);
		filter->setEnabled(b);
		add->setEnabled(b);
		del->setEnabled(b);
		copy->setEnabled(b);

		cancel->setEnabled(!b);
	}
};

FEBioBatchView::FEBioBatchView(CMainWindow* wnd, QWidget* parent) : CDocumentView(wnd, parent), ui(new FEBioBatchViewUI)
{
	ui->setup(this);
}

FEBioBatchView::~FEBioBatchView()
{
	delete ui;
}

void FEBioBatchView::setDocument(CDocument* doc)
{
	CDocumentView::setDocument(doc);

	m_doc = dynamic_cast<FEBioBatchDoc*>(doc);
	if (m_doc == nullptr) return;

	QObject::connect(m_doc, &FEBioBatchDoc::jobStatusChanged, this, &FEBioBatchView::UpdateJobStatus, Qt::UniqueConnection);
	QObject::connect(m_doc, &FEBioBatchDoc::batchFinished, this, &FEBioBatchView::on_batchFinished, Qt::UniqueConnection);

	UpdateTable();
	UpdateView();
}

void FEBioBatchView::on_runAll()
{
	if (m_doc == nullptr) return;

	QTableWidget* table = ui->table;
	std::vector<int> jobIds;
	for (int i=0; i<table->rowCount(); ++i)
	{
		int jobId = table->item(i, 0)->data(Qt::UserRole).toInt();
		jobIds.push_back(jobId);
	}

	if (!jobIds.empty())
	{
		ui->enableButtons(false);
		m_doc->StartBatch(jobIds);
		UpdateView();
	}
}

void FEBioBatchView::on_runSelected()
{
	if (m_doc == nullptr) return;

	QTableWidget* table = ui->table;

	QList<QTableWidgetItem*> sel = table->selectedItems();
	if (sel.empty()) return;

	std::vector<int> jobs;
	for (QTableWidgetItem* it : sel)
	{
		if (it->column() == 0)
		{
			int row = it->row();
			jobs.push_back(row);
		}
	}

	if (!jobs.empty())
	{
		ui->enableButtons(false);
		m_doc->StartBatch(jobs);
		UpdateView();
	}
}

void FEBioBatchView::on_cancelAll()
{
	if (m_doc == nullptr) return;
	if (QMessageBox::question(this, "Cancel all", "Are you sure you want to cancel all running and pending jobs?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		m_doc->CancelAll();
		UpdateView();
	}
}

void FEBioBatchView::on_cancelPending()
{
	if (m_doc == nullptr) return;
	m_doc->CancelPending();
	UpdateView();
}

void FEBioBatchView::on_filter()
{
	QString txt = ui->filter->text();
	ui->m_flt.SetFilter(txt);
	if (ui->m_flt.isValid() || txt.isEmpty())
	{
		UpdateTable();
		UpdateView();
	}
}

void FEBioBatchView::on_clipboard()
{
	if (m_doc == nullptr) return;
	QTableWidget* table = ui->table;
	if (table->rowCount() == 0) return;
	QString s;
	for (int i = 0; i < table->columnCount(); ++i)
	{
		s += table->horizontalHeaderItem(i)->text();
		if (i < table->columnCount() - 1) s += "\t";
	}
	s += "\n";
	for (int i = 0; i < table->rowCount(); ++i)
	{
		for (int j = 0; j < table->columnCount(); ++j)
		{
			s += table->item(i, j)->text();
			if (j < table->columnCount() - 1) s += "\t";
		}
		s += "\n";
	}
	QApplication::clipboard()->setText(s);
}

void FEBioBatchView::on_batchFinished()
{
	ui->enableButtons(true);
}

void updateTableItemInt(QTableWidgetItem* it, int n0, int n1)
{
	it->setBackground(QBrush());
	if (n0 < 0) it->setText("");
	else if ((n1 == -1) || (n0 == n1)) it->setText(QString::asprintf("%d", n0));
	else
	{
		if (n0 > n1) it->setBackground(QBrush(Qt::darkRed));
		else it->setBackground(QBrush(Qt::darkYellow));

		QString s = QString::asprintf("%d (%d)", n0, n1);
		it->setText(s);
		it->setToolTip(s);
	}
}

void updateTableItemFloat(QTableWidgetItem* it, double n0, double n1)
{
	it->setBackground(QBrush());
	if (n0 <= 0) it->setText("");
	else if ((n1 <= 0) || (n0 == n1)) it->setText(QString::asprintf("%.2f", n0));
	else
	{
		QString s0 = QString::asprintf("%.2f", n0);
		QString s1 = QString::asprintf("%.2f", n1);
		if (s0 == s1)
		{
			it->setText(s0);
			it->setToolTip(QString::asprintf("%lg", n0));
		}
		else
		{
			if (n0 > n1) it->setBackground(QBrush(Qt::darkRed));
			else it->setBackground(QBrush(Qt::darkYellow));

			QString s = QString::asprintf("%.2f (%.2f)", n0, n1);
			it->setText(s);

			s = QString::asprintf("%f (%f)", n0, n1);
			it->setToolTip(s);
		}
	}
}

void updateTableItemDouble(QTableWidgetItem* it, double n0, double n1)
{
	it->setBackground(QBrush());
	if (n0 < 0) it->setText("");
	else if ((n1 < 0) || (n0 == n1)) it->setText(QString::asprintf("%.5lg", n0));
	else
	{
		QString s0 = QString::asprintf("%.5lg", n0);
		QString s1 = QString::asprintf("%.5lg", n1);
		if (s0 == s1)
		{
			it->setText(s0);
			it->setToolTip(QString::asprintf("%lg", n0));
		}
		else
		{
			if (n0 > n1) it->setBackground(QBrush(Qt::darkRed));
			else it->setBackground(QBrush(Qt::darkYellow));

			QString s = QString::asprintf("%.5lg (%.5lg)", n0, n1);
			it->setText(s);

			s = QString::asprintf("%lg (%lg)", n0, n1);
			it->setToolTip(s);
		}
	}
}


void updateTableRow(QTableWidget* table, int nrow, const FEBioBatchDoc::JobInfo& item)
{
	QString status("unknown");
	switch (item.status)
	{
	case FEBioBatchDoc::IDLE     : status = ""; break;
	case FEBioBatchDoc::PENDING  : status = "Pending"; break;
	case FEBioBatchDoc::RUNNING  : status = "Running"; break;
	case FEBioBatchDoc::FINISHED : status = "Finished"; break;
	case FEBioBatchDoc::FAILED   : status = "Failed"; break;
	case FEBioBatchDoc::CANCELLED: status = "Cancelled"; break;
	default:
		break;
	}
	table->item(nrow, 1)->setText(status);

	if ((item.status == FEBioBatchDoc::PENDING) ||
		(item.status == FEBioBatchDoc::RUNNING))
	{
		for (int i = 2; i < table->columnCount(); ++i)
		{
			QTableWidgetItem* it = table->item(nrow, i);
			it->setText("");
			it->setBackground(QBrush());
			it->setToolTip("");
		}
	}
	else
	{
		updateTableItemInt(table->item(nrow, 2), item.stats.timeSteps, item.oldStats.timeSteps);
		updateTableItemInt(table->item(nrow, 3), item.stats.iterations, item.oldStats.iterations);
		updateTableItemInt(table->item(nrow, 4), item.stats.nrhs, item.oldStats.nrhs);
		updateTableItemInt(table->item(nrow, 5), item.stats.reformations, item.oldStats.reformations);

		updateTableItemDouble(table->item(nrow, 6), item.stats.solutionNorm, item.oldStats.solutionNorm);
		updateTableItemFloat(table->item(nrow, 7), item.stats.elapsedTime, item.oldStats.elapsedTime);
	}
}

void FEBioBatchView::UpdateTable()
{
	QTableWidget* table = ui->table;
	table->setRowCount(0);
	ui->rowmap.clear();
	BatchRunFilter& flt = ui->m_flt;
	for (const FEBioBatchDoc::JobInfo& file : m_doc->GetFileList())
	{
		if (!flt.isValid() || (flt.matches(file)))
		{
			int row = table->rowCount();
			ui->rowmap[file.jobId] = row;
			table->insertRow(row);
			QTableWidgetItem* ti = nullptr;
			table->setItem(row, 0, ti = new QTableWidgetItem(file.fileName)); ti->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			ti->setData(Qt::UserRole, file.jobId);
			table->setItem(row, 1, ti = new QTableWidgetItem("")); ti->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
			table->setItem(row, 2, ti = new QTableWidgetItem("")); ti->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			table->setItem(row, 3, ti = new QTableWidgetItem("")); ti->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			table->setItem(row, 4, ti = new QTableWidgetItem("")); ti->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			table->setItem(row, 5, ti = new QTableWidgetItem("")); ti->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			table->setItem(row, 6, ti = new QTableWidgetItem("")); ti->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			table->setItem(row, 7, ti = new QTableWidgetItem("")); ti->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		}
	}
}

void FEBioBatchView::UpdateView()
{
	if (m_doc == nullptr) return;
	QTableWidget* table = ui->table;
	for (int i=0; i<table->rowCount(); ++i)
	{
		int jobId = table->item(i, 0)->data(Qt::UserRole).toInt();
		if ((jobId >= 0) && (jobId < m_doc->Files()))
		{
			const FEBioBatchDoc::JobInfo& file = m_doc->GetJobInfo(jobId);
			updateTableRow(table, i, file);
		}
	}
}

void FEBioBatchView::UpdateJobStatus(int jobId)
{
	if (m_doc == nullptr) return;
	if (jobId == -1) return;
	FEBioBatchDoc* sender = dynamic_cast<FEBioBatchDoc*>(QObject::sender());
	if (sender != m_doc) return;

	QTableWidget* table = ui->table;

	if ((jobId >= 0) && (jobId < m_doc->Files()))
	{
		const FEBioBatchDoc::JobInfo& file = m_doc->GetJobInfo(jobId);
		assert(ui->rowmap.find(jobId) != ui->rowmap.end());
		int row = ui->rowmap[jobId];
		updateTableRow(table, row, file);
	}
}

void FEBioBatchView::showContextMenu(const QPoint& pos)
{
	QTableWidget* table = ui->table;

	// Map the point to the global position
	QModelIndex index = table->indexAt(pos);
	if (!index.isValid())
		return; // Clicked outside any valid cell

	int row = index.row();  // the row that was right-clicked
	int jobId = table->item(row, 0)->data(Qt::UserRole).toInt();

	QMenu menu(this);
	menu.addAction("Open feb file", [this, jobId]() {
		if (m_doc)
		{
			CMainWindow* wnd = mainWindow();
			FEBioBatchDoc::JobInfo job = m_doc->GetJobInfo(jobId);
			wnd->OpenFile(job.fileName);
		}
		});
	menu.addAction("Open log file", [this, jobId]() {
		if (m_doc)
		{
			CMainWindow* wnd = mainWindow();
			FEBioBatchDoc::JobInfo job = m_doc->GetJobInfo(jobId);
			QString filename = job.fileName;
			filename.replace(".feb", ".log");
			wnd->OpenFile(filename);
		}
		});
	menu.addAction("Open plot file", [this, jobId]() {
		if (m_doc)
		{
			CMainWindow* wnd = mainWindow();
			FEBioBatchDoc::JobInfo job = m_doc->GetJobInfo(jobId);
			QString filename = job.fileName;
			filename.replace(".feb", ".xplt");
			wnd->OpenFile(filename);
		}
		});

	// Show menu at the cursor
	menu.exec(table->viewport()->mapToGlobal(pos));
}

void FEBioBatchView::on_options()
{
	if (m_doc == nullptr) return;

	FEBioBatchDoc::Options opt = m_doc->GetOptions();
	CDlgBatchOptions dlg(opt, this);
	if (dlg.exec())
	{
		m_doc->SetOptions(dlg.GetOptions());
	}
}

void FEBioBatchView::on_addBatchFiles()
{
	if (m_doc == nullptr) return;

	QStringList files = QFileDialog::getOpenFileNames(this, "Add FEBio files to batch", QString(), "FEBio files (*.feb)");
	if (files.empty()) return;

	for (const QString& file : files)
	{
		m_doc->AddFile(file);
	}
	
	UpdateTable();
	UpdateView();
}

void FEBioBatchView::on_removeSelectedFiles()
{
	if (m_doc == nullptr) return;
	QTableWidget* table = ui->table;
	QList<QTableWidgetItem*> sel = table->selectedItems();
	if (sel.empty()) return;
	std::set<int> jobs;
	for (QTableWidgetItem* it : sel)
	{
		if (it->column() == 0)
		{
			int row = it->row();
			int jobId = table->item(row, 0)->data(Qt::UserRole).toInt();
			jobs.insert(jobId);
		}
	}
	if (!jobs.empty())
	{
		int N = (int)jobs.size();
		QString msg = QString::asprintf("Are you sure you want to remove the %d selected file%s?", N, (N == 1) ? "" : "s");
		if (QMessageBox::question(this, "Remove selected files", msg, QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
		{
			for (int jobId : jobs) m_doc->RemoveFile(jobId);
			UpdateTable();
			UpdateView();
		}
	}
}

class CDlgBatchOptions::UI {
public:
	QSpinBox* nthreads = nullptr;
	QSpinBox* nprocs   = nullptr;
	QLineEdit* febioPath = nullptr;
	FEBioBatchDoc::Options ops;

public:
	void setup(QDialog* dlg)
	{
		QFormLayout* form = new QFormLayout;

		form->addRow("Nr. of processes:", nprocs = new QSpinBox);
		nprocs->setRange(1, 32);
		nprocs->setValue(1);

		form->addRow("Nr. of threads/process:", nthreads = new QSpinBox);
		nthreads->setRange(0, 32);
		nthreads->setValue(1);
		nthreads->setSpecialValueText("(default)");

		form->addRow("FEBio executable:", febioPath = new QLineEdit);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(form);
		l->addWidget(bb);
		dlg->setLayout(l);

		connect(bb, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
		connect(bb, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
	}

	void setOptions(FEBioBatchDoc::Options opt)
	{
		ops = opt;
		nthreads->setValue(opt.nthreads);
		nprocs->setValue(opt.nprocs);
		febioPath->setText(opt.febioPath);
	}

	FEBioBatchDoc::Options options()
	{
		ops.nthreads = nthreads->value();
		ops.nprocs = nprocs->value();
		ops.febioPath = febioPath->text();
		return ops;
	}
};

CDlgBatchOptions::CDlgBatchOptions(FEBioBatchDoc::Options opt, QWidget* parent) : QDialog(parent), ui(new UI)
{
	ui->setup(this);
	setWindowTitle("Batch Options");
	ui->setOptions(opt);
}

FEBioBatchDoc::Options CDlgBatchOptions::GetOptions() const
{
	return ui->options();
}

