/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
#include "DlgKinemat.h"
#include "CIntInput.h"
#include <QListWidget>
#include <QBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialogButtonBox>
#include "MainWindow.h"
#include "Document.h"
#include "PostDocument.h"
#include <PostLib/FEKinemat.h>
#include <PostLib/FELSDYNAimport.h>
#include "PostDocument.h"
#include <PostGL/GLModel.h>

class CDlgKinematUI
{
public:
	CMainWindow*	m_wnd;

	QLineEdit* modelFile;
	QLineEdit* kineFile;
	CIntInput *start, *end, *stride;

public:
	void setup(QDialog* dlg)
	{
		QVBoxLayout* pv = new QVBoxLayout;
		QLabel* pl = new QLabel("Model file:");
		modelFile = new QLineEdit; pl->setBuddy(modelFile);
		QPushButton* browse1 = new QPushButton("..."); browse1->setFixedWidth(30);
		QHBoxLayout* ph = new QHBoxLayout;
		ph->addWidget(modelFile);
		ph->addWidget(browse1);
		pv->addWidget(pl);
		pv->addLayout(ph);

		pl = new QLabel("Kine file:");
		kineFile = new QLineEdit; pl->setBuddy(kineFile);
		QPushButton* browse2 = new QPushButton("..."); browse2->setFixedWidth(30);
		ph = new QHBoxLayout;
		ph->addWidget(kineFile);
		ph->addWidget(browse2);
		pv->addWidget(pl);
		pv->addLayout(ph);

		ph = new QHBoxLayout;
		ph->addWidget(pl = new QLabel("From:"));
		ph->addWidget(start = new CIntInput); pl->setBuddy(start); start->setFixedWidth(70); start->setValue(1);
		ph->addWidget(pl = new QLabel("To:"));
		ph->addWidget(end = new CIntInput); pl->setBuddy(end); end->setFixedWidth(70); end->setValue(999);
		ph->addWidget(pl = new QLabel("Stride:"));
		ph->addWidget(stride = new CIntInput); pl->setBuddy(stride); stride->setFixedWidth(70); stride->setValue(1);
		pv->addLayout(ph);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		pv->addWidget(bb);
		dlg->setLayout(pv);

		QObject::connect(browse1, SIGNAL(clicked()), dlg, SLOT(OnBrowse1()));
		QObject::connect(browse2, SIGNAL(clicked()), dlg, SLOT(OnBrowse2()));
		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(OnApply()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
	}
};

CDlgKinemat::CDlgKinemat(CMainWindow* parent) : QDialog(parent), ui(new CDlgKinematUI)
{
	setWindowTitle("Kinemat");
	ui->m_wnd = parent;
	ui->setup(this);
}

void CDlgKinemat::OnBrowse1()
{
	QString filename = QFileDialog::getOpenFileName(0, "Open file", 0, "LSDYNA Keyword (*.k)");
	if (filename.isEmpty() == false)
	{
		ui->modelFile->setText(filename);
	}
}

void CDlgKinemat::OnBrowse2()
{
	QString filename = QFileDialog::getOpenFileName(0, "Open file", 0, "All files(*)");
	if (filename.isEmpty() == false)
	{
		ui->kineFile->setText(filename);
	}
}

void CDlgKinemat::OnApply()
{
	int n0 = ui->start->value();
	int n1 = ui->end->value();
	int ni = ui->stride->value();

	// create a new document
	CMainWindow* wnd = ui->m_wnd;
	CPostDocument* postDoc = new CPostDocument(wnd);

	FEKinemat kine;
	kine.SetRange(n0, n1, ni);

	string modelFile = ui->modelFile->text().toStdString();
	string kineFile = ui->kineFile->text().toStdString();

	// load the file
	Post::FELSDYNAimport* preader = new Post::FELSDYNAimport(postDoc->GetFEModel());
	preader->read_displacements(true);
	if (preader->Load(modelFile.c_str())==false)
	{
		QMessageBox::critical(wnd, "PostView2", "Failed to load model file");
		delete postDoc;
		return;
	}

	if (kine.Apply(postDoc->GetFEModel(), kineFile.c_str()) == false)
	{
		QMessageBox::critical(0, "Kinemat", "Failed applying Kinemat tool");
	}

	postDoc->SetDocFilePath(modelFile);
	postDoc->Initialize();

	// update displacements on all states
	Post::CGLModel& mdl = *postDoc->GetGLModel();
	if (mdl.GetDisplacementMap() == nullptr)
	{
		mdl.AddDisplacementMap("Displacement");
	}

	int nstates = mdl.GetFEModel()->GetStates();
	for (int i = 0; i < nstates; ++i) mdl.UpdateDisplacements(i, true);


	wnd->UpdateModel();
	wnd->Update();
	wnd->AddDocument(postDoc);

	accept();
}
