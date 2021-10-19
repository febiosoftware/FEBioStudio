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
#include "DlgEditProject.h"
#include <MeshTools/FEProject.h>
#include <QListWidget>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>

class Ui::CDlgEditProject
{
public:
	QListWidget* list;
	
public:
	void setup(QDialog* dlg)
	{
		list = new QListWidget;

		QPushButton* all  = new QPushButton("All");
		QPushButton* none = new QPushButton("None");

		QVBoxLayout* l = new QVBoxLayout;
		l->setContentsMargins(0,0,0,0);
		l->addWidget(all);
		l->addWidget(none);
		l->addStretch();

		QHBoxLayout* h = new QHBoxLayout;
		h->setContentsMargins(0,0,0,0);
		h->addWidget(list);
		h->addLayout(l);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		QLabel* m = new QLabel("Modules:");
		m->setAlignment(Qt::AlignLeft);

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->addWidget(m);
		mainLayout->addLayout(h);
		mainLayout->addWidget(bb);

		dlg->setLayout(mainLayout);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
		QObject::connect(all, SIGNAL(clicked(bool)), dlg, SLOT(onAllClicked()));
		QObject::connect(none, SIGNAL(clicked(bool)), dlg, SLOT(onNoneClicked()));
	}

	void addModule(const QString& name, int id, bool checked)
	{
		QListWidgetItem* item = new QListWidgetItem;
		item->setText(name);
		item->setData(Qt::UserRole, id);
		item->setCheckState(checked? Qt::Checked : Qt::Unchecked);

		list->addItem(item);
	}

	bool isRowChecked(int nrow)
	{
		QListWidgetItem* item = list->item(nrow);
		if (item)
		{
			return (item->checkState() == Qt::Checked);
		}
		else
		{
			assert(false);
			return false;
		}
	}

	unsigned int getModule()
	{
		unsigned int mod = 0;
		int N = list->count();
		for (int i=0; i<N; ++i)
		{
			QListWidgetItem* item = list->item(i);
			if (item->checkState() == Qt::Checked)
			{
				mod |= item->data(Qt::UserRole).toInt();
			}
		}
		return mod;
	}

	void checkAll(bool checked)
	{
		int N = list->count();
		for (int i = 0; i<N; ++i)
		{
			QListWidgetItem* item = list->item(i);
			item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
		}
	}
};

CDlgEditProject::CDlgEditProject(FEProject& prj, QWidget* parent): QDialog(parent), ui(new Ui::CDlgEditProject), m_prj(prj)
{
	ui->setup(this);

	unsigned int mod = prj.GetModule();

	ui->addModule("Mechanics"			, MODULE_MECH				, (mod & MODULE_MECH));
	ui->addModule("Heat Transer"		, MODULE_HEAT				, (mod & MODULE_HEAT));
	ui->addModule("Biphasic"			, MODULE_BIPHASIC			, (mod & MODULE_BIPHASIC));
	ui->addModule("Solutes"				, MODULE_SOLUTES			, (mod & MODULE_SOLUTES));
	ui->addModule("Multiphasic"			, MODULE_MULTIPHASIC		, (mod & MODULE_MULTIPHASIC));
	ui->addModule("Fluid"				, MODULE_FLUID				, (mod & MODULE_FLUID));
	ui->addModule("Reactions"			, MODULE_REACTIONS			, (mod & MODULE_REACTIONS));
	ui->addModule("Reaction-Diffusion"	, MODULE_REACTION_DIFFUSION	, (mod & MODULE_REACTION_DIFFUSION));
    ui->addModule("Fluid-FSI"			, MODULE_FLUID_FSI  , (mod & MODULE_FLUID_FSI));
}

void CDlgEditProject::accept()
{
	unsigned int module = ui->getModule();

	if (module == 0)
	{
		QMessageBox::critical(this, "Edit Properties", "You must select at least one module.");
		return;
	}
	
	m_prj.SetModule(module);

	QDialog::accept();
}

void CDlgEditProject::onAllClicked()
{
	ui->checkAll(true);
}

void CDlgEditProject::onNoneClicked()
{
	ui->checkAll(false);
}

