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
#include <map>

class FEBioBatchViewUI
{
public:
	QPushButton* start = nullptr;
	QPushButton* ops = nullptr;
	QTableWidget* table = nullptr;
	QLineEdit* filter = nullptr;

	std::map<int, int> rowmap;

public:
	void setup(FEBioBatchView* w)
	{
		QHBoxLayout* h = new QHBoxLayout;
		h->setContentsMargins(0, 0, 0, 0);
		start = new QPushButton("Start");
		QMenu* menu = new QMenu(start);
		QAction* runAll = menu->addAction("Run all");
		QAction* runSel = menu->addAction("Run selected");
		start->setMenu(menu);
		h->addWidget(start);

		ops = new QPushButton("Options ...");
		h->addWidget(ops);

		h->addWidget(new QLabel("filter:"));
		filter = new QLineEdit;
		filter->setToolTip("Filter the list of files");
		h->addWidget(filter);

		h->addStretch();

		table = new QTableWidget(w);
		table->setColumnCount(8);
		QStringList headers;
		headers << "File" << "Status" << "Timesteps" << "Iters." << "RHS evals." << "Reforms." << "Solution" << "Runtime (sec.)";
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
		QObject::connect(table, &QTableWidget::customContextMenuRequested, w, &FEBioBatchView::showContextMenu);
		QObject::connect(ops, &QPushButton::clicked, w, &FEBioBatchView::on_options);
		QObject::connect(filter, &QLineEdit::textChanged, w, &FEBioBatchView::on_filter);
	}

	void enableButtons(bool b)
	{
		start->setEnabled(b);
		ops->setEnabled(b);
		filter->setEnabled(b);
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
		int row = it->row();
		jobs.push_back(row);
	}

	if (!jobs.empty())
	{
		ui->enableButtons(false);
		m_doc->StartBatch(jobs);
		UpdateView();
	}
}

void FEBioBatchView::on_filter()
{
	UpdateTable();
	UpdateView();
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
	if (n0 < 0) it->setText("");
	else if ((n1 < 0) || (n0 == n1)) it->setText(QString::asprintf("%.2f", n0));
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
	QString filter = ui->filter->text();
	for (const FEBioBatchDoc::JobInfo& file : m_doc->GetFileList())
	{
		if (filter.isEmpty() || (file.fileName.contains(filter)))
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

class CDlgBatchOptions::UI {
public:
	QSpinBox* nthreads = nullptr;
	QSpinBox* nprocs   = nullptr;
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
	}

	FEBioBatchDoc::Options options()
	{
		ops.nthreads = nthreads->value();
		ops.nprocs = nprocs->value();
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
