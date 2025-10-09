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
#include "DlgBatchRun.h"
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QPushButton>
#include <QMenu>
#include <QFileDialog>
#include "MainWindow.h"
#include "DocManager.h"
#include "FEBioStudioProject.h"

class UIDlgBatchRun
{
public:
	QStringList m_fileList;
	CMainWindow* m_wnd = nullptr;

public:
	UIDlgBatchRun() {}

	void setup(CDlgBatchRun* dlg)
	{
		QVBoxLayout* l = new QVBoxLayout;

		QHBoxLayout* hl = new QHBoxLayout;

		QPushButton* b = new QPushButton("Import ...");
		QMenu* menu = new QMenu(b);
		QAction* a1 = menu->addAction("Add files from current project");
		QAction* a2 = menu->addAction("Add files from folder ...");
		QAction* a3 = menu->addAction("Add files ...");
		b->setMenu(menu);
		hl->addWidget(b);

		QPushButton* del = new QPushButton("Remove");
		hl->addWidget(del);

		hl->addStretch();

		l->addLayout(hl);

		QListWidget* list = new QListWidget;
		list->setSelectionMode(QAbstractItemView::ExtendedSelection);
		l->addWidget(list);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		l->addWidget(bb);

		QObject::connect(bb, &QDialogButtonBox::accepted, dlg, &CDlgBatchRun::accept);
		QObject::connect(bb, &QDialogButtonBox::rejected, dlg, &CDlgBatchRun::reject);
		QObject::connect(a1, &QAction::triggered, [=]() {
			if (m_wnd == nullptr) return;
			FEBioStudioProject* prj = m_wnd->GetProject();
			if (prj)
			{
				QStringList files = prj->GetFilePaths();
				for (const QString& file : files)
				{
					if (!m_fileList.contains(file))
					{
						m_fileList.append(file);
						list->addItem(file);
					}
				}
			}
			});
		QObject::connect(a2, &QAction::triggered, [=]() {
			if (m_wnd == nullptr) return;
			QString dir = m_wnd->CurrentWorkingDirectory();
			QString folder = QFileDialog::getExistingDirectory(dlg, "Select folder", dir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
			if (!folder.isEmpty())
			{
				QDir d(folder);
				QStringList files = d.entryList(QStringList("*.feb"), QDir::Files);
				for (const QString& file : files)
				{
					QString f = d.absoluteFilePath(file);
					if (!m_fileList.contains(f))
					{
						m_fileList.append(f);
						list->addItem(f);
					}
				}
			}
			});
		QObject::connect(a3, &QAction::triggered, [=]() {
			if (m_wnd == nullptr) return;
			QString dir = m_wnd->CurrentWorkingDirectory();
			QStringList files = QFileDialog::getOpenFileNames(dlg, "Select FEBio input files", dir, "FEBio input files (*.feb)");
			if (files.count() > 0)
			{
				for (const QString& file : files)
				{
					if (!m_fileList.contains(file))
					{
						m_fileList.append(file);
						list->addItem(file);
					}
				}
			}
			});
		QObject::connect(del, &QPushButton::clicked, [=]() {
			QList<QListWidgetItem*> items = list->selectedItems();
			for (QListWidgetItem* item : items)
			{
				m_fileList.removeAll(item->text());
				delete item;
			}
			});

		dlg->setLayout(l);
	}
};

CDlgBatchRun::CDlgBatchRun(CMainWindow* wnd, QWidget* parent) : QDialog(parent), ui(new UIDlgBatchRun)
{
	ui->m_wnd = wnd;
	setMinimumSize(600, 400);
	ui->setup(this);
}

CDlgBatchRun::~CDlgBatchRun()
{
	delete ui;
}

void CDlgBatchRun::accept()
{
	QDialog::accept();
}

QStringList CDlgBatchRun::GetFileList() const
{
	return ui->m_fileList;
}
