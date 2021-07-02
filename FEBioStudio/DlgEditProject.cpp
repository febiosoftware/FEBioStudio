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
#include "DlgEditProject.h"
#include <MeshTools/FEProject.h>
#include <QListWidget>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include <FEBioLink/FEBioClass.h>
#include <FEBioLink/FEBioModule.h>

class Ui::CDlgEditProject
{
public:
	QListWidget* list;
	
public:
	void setup(QDialog* dlg)
	{
		list = new QListWidget;

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		QLabel* m = new QLabel("Modules:");
		m->setAlignment(Qt::AlignLeft);

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->addWidget(m);
		mainLayout->addWidget(list);
		mainLayout->addWidget(bb);

		dlg->setLayout(mainLayout);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
	}

	void addModule(const QString& name, int id)
	{
		QListWidgetItem* item = new QListWidgetItem;
		item->setText(name);
		item->setData(Qt::UserRole, id);
		list->addItem(item);
	}

	unsigned int getModule()
	{
		QListWidgetItem* pw = list->currentItem();
		if (pw == nullptr) return 0;
		unsigned int mod = pw->data(Qt::UserRole).toInt();
		return mod;
	}
};

CDlgEditProject::CDlgEditProject(FEProject& prj, QWidget* parent): QDialog(parent), ui(new Ui::CDlgEditProject), m_prj(prj)
{
	ui->setup(this);

	unsigned int modid = prj.GetModule();

	vector<FEBio::FEBioModule> mods = FEBio::GetAllModules();
	int current = -1;
	for (int i = 0; i < mods.size(); ++i)
	{
		FEBio::FEBioModule& mod = mods[i];
		if (mod.m_id == modid) current = i;
		ui->addModule(mod.m_szname, mod.m_id);
	}
	ui->list->setCurrentRow(current);
}

void CDlgEditProject::accept()
{
	unsigned int moduleId = ui->getModule();

	if (moduleId == 0)
	{
		QMessageBox::critical(this, "Edit Properties", "You must select at least one module.");
		return;
	}
	
	m_prj.SetModule(moduleId);

	QDialog::accept();
}
