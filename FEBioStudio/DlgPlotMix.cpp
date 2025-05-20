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
#include "DlgPlotMix.h"
#include "MainWindow.h"
#include <QPushButton>
#include <QListWidget>
#include <QBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <PostLib/FEPlotMix.h>
#include <PostGL/GLModel.h>
#include "PostDocument.h"

class CDlgPlotMixUI
{
public:
	CMainWindow*	m_wnd;
	QListWidget* list;

public:
	void setup(QDialog* parent)
	{
		QVBoxLayout* pv = new QVBoxLayout;
		QHBoxLayout* ph = new QHBoxLayout;

		QPushButton* browse = new QPushButton("Add file ...");
		QPushButton* remove = new QPushButton("Remove");
		QPushButton* moveUp = new QPushButton("Move Up");
		QPushButton* moveDown = new QPushButton("Move Down");
		ph->addWidget(browse);
		ph->addWidget(remove);
		ph->addWidget(moveUp);
		ph->addWidget(moveDown);
		pv->addLayout(ph);

		list = new QListWidget;
		pv->addWidget(list);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		pv->addWidget(bb);

		parent->setLayout(pv);

		QObject::connect(browse, SIGNAL(clicked()), parent, SLOT(OnBrowse()));
		QObject::connect(remove, SIGNAL(clicked()), parent, SLOT(OnRemove()));
		QObject::connect(moveUp, SIGNAL(clicked()), parent, SLOT(OnMoveUp()));
		QObject::connect(moveDown, SIGNAL(clicked()), parent, SLOT(OnMoveDown()));
		QObject::connect(bb, SIGNAL(accepted()), parent, SLOT(OnApply()));
		QObject::connect(bb, SIGNAL(rejected()), parent, SLOT(reject()));
	}
};

CDlgPlotMix::CDlgPlotMix(CMainWindow* wnd) : QDialog(wnd), ui(new CDlgPlotMixUI)
{
	setWindowTitle("Plot Mix");
	ui->m_wnd = wnd;
	ui->setup(this);
}

void CDlgPlotMix::OnBrowse()
{
	QStringList filenames = QFileDialog::getOpenFileNames(0, "Open file", 0, "XPLT files(*.xplt)");
	if (filenames.isEmpty() == false)
	{
		for (int i = 0; i<filenames.count(); ++i)
			ui->list->addItem(filenames[i]);
	}
}

void CDlgPlotMix::OnRemove()
{
	qDeleteAll(ui->list->selectedItems());
}

void CDlgPlotMix::OnMoveUp()
{
	QList<QListWidgetItem*> items = ui->list->selectedItems();
	QList<QListWidgetItem*>::iterator it;
	for (it = items.begin(); it != items.end(); ++it)
	{
		QListWidgetItem* pi = ui->list->takeItem(ui->list->row(*it));
		ui->list->insertItem(0, pi);
	}
}

void CDlgPlotMix::OnMoveDown()
{
	QList<QListWidgetItem*> items = ui->list->selectedItems();
	QList<QListWidgetItem*>::iterator it;
	for (it = items.begin(); it != items.end(); ++it)
	{
		QListWidgetItem* pi = ui->list->takeItem(ui->list->row(*it));
		ui->list->addItem(pi);
	}
}

void CDlgPlotMix::OnApply()
{

	int nitems = ui->list->count();
	std::vector<std::string> str(nitems);
	for (int i = 0; i<nitems; ++i)
	{
		QListWidgetItem* pi = ui->list->item(i);
		QString s = pi->text();
		str[i] = s.toStdString();
	}

	std::vector<const char*> sz(nitems, 0);
	for (int i = 0; i<nitems; ++i) sz[i] = str[i].c_str();


	// Create a new document
	CPostDocument* doc = new CPostDocument(ui->m_wnd);
	doc->SetDocTitle("PlotMix");
	Post::FEPlotMix reader(doc->GetFSModel());

	if (reader.Load(&sz[0], nitems) == false)
	{
		QMessageBox::critical(0, "PlotMix Tool", "An error occured reading the plot files.");
		delete doc;
	}
	else
	{
		// update post document
		doc->Initialize();

		// a new model is created when the doc is initialized
		Post::CGLModel* glm = doc->GetGLModel();

		// update displacements on all states
		if (glm->GetDisplacementMap() == nullptr)
		{
			glm->AddDisplacementMap("Displacement");
		}
		int nstates = glm->GetFSModel()->GetStates();
		for (int i = 0; i < nstates; ++i) glm->UpdateDisplacements(i, true);


		ui->m_wnd->AddDocument(doc);
	}

	ui->list->clear();

	accept();
}
