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
#include "DlgAddRigidConnector.h"
#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QListWidget>
#include <QFormLayout>
#include <QMessageBox>
#include <FEMLib/FSModel.h>
#include <FEMLib/FSProject.h>
#include <FEMLib/FEMultiMaterial.h>
#include <FEMLib/FEMKernel.h>
#include <FEBioLink/FEBioInterface.h>
#include <FEBioLink/FEBioClass.h>
#include <FSCore/FSCore.h>

class Ui::CDlgAddRigidConnector
{
public:
	QVBoxLayout* mainLayout;
	QLineEdit* name;
	QComboBox* step;
	QComboBox* matA;
	QComboBox* matB;
	QListWidget* list;

	int		m_ntype;
	int		m_matA, m_matB;

public:
	void setup(QDialog* dlg)
	{
		m_ntype = -1;
		m_matA = m_matB = -1;

		name = new QLineEdit;
		name->setPlaceholderText("(leave blank for default)");

		step = new QComboBox;

		matA = new QComboBox;
		matB = new QComboBox;

		list = new QListWidget;

		QFormLayout* form = new QFormLayout;
		form->addRow("Name:", name);
		form->addRow("Step:", step);
		form->addRow("Rigid material A:", matA);
		form->addRow("Rigid material B:", matB);

		mainLayout = new QVBoxLayout;
		mainLayout->addLayout(form);
		mainLayout->addWidget(list);

		QObject::connect(list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), dlg, SLOT(accept()));
	}
};

CDlgAddRigidConnector::CDlgAddRigidConnector(FSProject& prj, QWidget* parent) : CHelpDialog(parent), ui(new Ui::CDlgAddRigidConnector)
{
	setWindowTitle("Add Rigid Connector");

	ui->setup(this);
	SetLeftSideLayout(ui->mainLayout);

	// add the steps
	FSModel& fem = prj.GetFSModel();
	for (int i = 0; i<fem.Steps(); ++i)
	{
		ui->step->addItem(QString::fromStdString(fem.GetStep(i)->GetName()));
	}

	// set the materials
	for (int i = 0; i<fem.Materials(); ++i)
	{
		GMaterial* pm = fem.GetMaterial(i);
		if (pm->GetMaterialProperties()->IsRigid())
		{
			ui->matA->addItem(QString::fromStdString(pm->GetName()), pm->GetID());
			ui->matB->addItem(QString::fromStdString(pm->GetName()), pm->GetID());
		}
	}

	// add the rigid connectors
	unsigned int mod = prj.GetModule();
//	vector<FEClassFactory*> v =  FEMKernel::FindAllClasses(mod, FE_RIGID_CONNECTOR);
	int rigidConnectorId = FEBio::GetBaseClassIndex("FERigidConnector"); assert(rigidConnectorId != -1);
	std::vector<FEBio::FEBioClassInfo> v = FEBio::FindAllClasses(m_module, FENLCONSTRAINT_ID, rigidConnectorId);
	for (int i=0; i<(int)v.size(); ++i)
	{
		FEBio::FEBioClassInfo& fac = v[i];

		const char* sztype = fac.sztype;
		QString typeStr = QString::fromStdString(FSCore::beautify_string(sztype));

		QListWidgetItem* item = new QListWidgetItem;
		item->setText(typeStr);
		item->setData(Qt::UserRole, fac.classId);

		ui->list->addItem(item);
	}

	ui->list->setCurrentRow(0);
}

void CDlgAddRigidConnector::SetURL()
{
	int classID = ui->list->currentItem()->data(Qt::UserRole).toInt();

	// m_url = FEMKernel::FindClass(m_module, FENLCONSTRAINT_ID, classID)->GetHelpURL();
}

void CDlgAddRigidConnector::accept()
{
	QListWidgetItem* item = ui->list->currentItem();
	if (item == 0)
	{
		QMessageBox::critical(this, "Add Rigid Connector", "Please select a rigid connector");
		return;
	}

	ui->m_ntype = item->data(Qt::UserRole).toInt();
	ui->m_matA = ui->matA->currentData().toInt();
	ui->m_matB = ui->matB->currentData().toInt();

	QDialog::accept();
}

int CDlgAddRigidConnector::GetType()
{
	return ui->m_ntype;
}

std::string CDlgAddRigidConnector::GetName()
{
	QString t = ui->name->text();
	return t.toStdString();
}

int CDlgAddRigidConnector::GetStep()
{
	return ui->step->currentIndex();
}

int CDlgAddRigidConnector::GetMaterialA()
{
	return ui->m_matA;
}

int CDlgAddRigidConnector::GetMaterialB()
{
	return ui->m_matB;
}
