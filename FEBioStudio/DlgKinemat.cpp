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
#include "DlgKinemat.h"
#include <CUILib/InputWidgets.h>
#include <QListWidget>
#include <QBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialogButtonBox>
#include "MainWindow.h"

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
		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
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

int CDlgKinemat::StartIndex() const
{
	return ui->start->value();
}

int CDlgKinemat::EndIndex() const
{
	return ui->end->value();
}

int CDlgKinemat::Increment() const
{
	return ui->stride->value();
}

QString CDlgKinemat::GetModelFile() const
{
	return ui->modelFile->text();
}

QString CDlgKinemat::GetKineFile() const
{
	return ui->kineFile->text();
}
