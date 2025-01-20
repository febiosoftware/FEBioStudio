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
#include "DlgAddRigidConstraint.h"
#include <QBoxLayout>
#include <QComboBox>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QLabel>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <FEMLib/FSModel.h>
#include <FEMLib/FSProject.h>
#include <FEMLib/FERigidConstraint.h>
#include <FEMLib/FEMultiMaterial.h>
#include <FEMLib/FEMKernel.h>
#include <FEBioLink/FEBioInterface.h>
#include <FEBioLink/FEBioClass.h>
#include <FSCore/FSCore.h>

class Ui::CDlgAddRigidConstraint
{
public:
	QVBoxLayout* mainLayout;
	QLineEdit* name;
	QComboBox* step;
	QComboBox* mat;
	QListWidget* list;

public:
	void setupUi(QWidget* parent)
	{
		name = new QLineEdit;
		name->setPlaceholderText("(leave blank for default)");
		step = new QComboBox;

		mat = new QComboBox;

		list = new QListWidget;

		QFormLayout* form = new QFormLayout;
		form->setLabelAlignment(Qt::AlignRight);
		form->addRow("Name:", name);
		form->addRow("Step:", step);
		form->addRow("Rigid material:", mat);

		mainLayout = new QVBoxLayout;
		mainLayout->addLayout(form);
		mainLayout->addWidget(list);

		QObject::connect(list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), parent, SLOT(accept()));
	}
};

CDlgAddRigidConstraint::CDlgAddRigidConstraint(FSProject& prj, QWidget* parent) : CHelpDialog(parent), ui(new Ui::CDlgAddRigidConstraint)
{
	setWindowTitle("Add Rigid Constraint");

	ui->setupUi(this);

	SetLeftSideLayout(ui->mainLayout);

	// add the steps
	FSModel& fem = prj.GetFSModel();
	for (int i = 0; i<fem.Steps(); ++i)
	{
		ui->step->addItem(QString::fromStdString(fem.GetStep(i)->GetName()));
	}

	// set the materials
	m_selMat = 0;
	for (int i=0; i<fem.Materials(); ++i)
	{
		GMaterial* pm = fem.GetMaterial(i);
		if (pm->GetMaterialProperties()->IsRigid())
		{
			ui->mat->addItem(QString::fromStdString(pm->GetName()));
			m_mat.push_back(pm);
		}
	}
	if (m_mat.empty() == false) m_selMat = m_mat[0];

	// add the DOFs
	int mod = prj.GetModule();
//	vector<FEClassFactory*> v = FEMKernel::FindAllClasses(mod, FE_RIGID_CONSTRAINT);
	int rigidBCId = FEBio::GetBaseClassIndex("FERigidBC"); assert(rigidBCId != -1);
	std::vector<FEBio::FEBioClassInfo> v = FEBio::FindAllClasses(m_module, FEBC_ID, rigidBCId);
	for (int i=0; i<(int)v.size(); ++i)
	{
		FEBio::FEBioClassInfo& fac = v[i];

		const char* sztype = fac.sztype;
		QString typeStr = QString::fromStdString(FSCore::beautify_string(sztype));

		QListWidgetItem* item = new QListWidgetItem(ui->list);
		item->setText(typeStr);
		item->setData(Qt::UserRole, fac.classId);
	}

	ui->list->setCurrentRow(0);
}

void CDlgAddRigidConstraint::SetURL()
{
	int classID = ui->list->currentItem()->data(Qt::UserRole).toInt();

	// m_url = FEMKernel::FindClass(m_module, FERIGIDBC_ID, classID)->GetHelpURL();
}

void CDlgAddRigidConstraint::accept()
{
	// get the name
	QString name = ui->name->text();
	m_name = name.toStdString();

	// get the step
	m_nstep = ui->step->currentIndex();

	if (m_mat.empty() == false)
	{
		m_selMat = m_mat[ui->mat->currentIndex()];
	}

	QListWidgetItem* item = ui->list->currentItem();
	if (item == 0)
	{
		QMessageBox::critical(this, windowTitle(), "Please select a rigid constraint");
		return;
	}

	m_type = item->data(Qt::UserRole).toInt();

	QDialog::accept();
}
