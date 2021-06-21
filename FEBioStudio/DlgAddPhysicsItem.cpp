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

#include <QLineEdit>
#include <QComboBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QFormLayout>
#include <QBoxLayout>
#include <QLabel>
#include <FEMLib/FEMKernel.h>
#include "MeshTools/FEProject.h"
#include "DlgAddPhysicsItem.h"
#include "FEBioClass.h"

CDlgAddPhysicsItem::CDlgAddPhysicsItem(QString windowName, int superID, FEProject& prj, QWidget* parent)
	: CHelpDialog(prj, parent), m_superID(superID)
{
	setWindowTitle(windowName);

	// Setup UI
	QString placeHolder = "(leave blank for default)";
	name = new QLineEdit; name->setPlaceholderText(placeHolder);
	name->setMinimumWidth(name->fontMetrics().size(Qt::TextSingleLine, placeHolder).width()*1.3);

	step = new QComboBox;
	type = new QListWidget;

	QFormLayout* form = new QFormLayout;
	form->setLabelAlignment(Qt::AlignRight);
	form->addRow("Name:", name);
	form->addRow("Step:", step);

	QVBoxLayout* layout = new QVBoxLayout;

	layout->addLayout(form);
	layout->addWidget(type);

	SetLeftSideLayout(layout);


	// add the steps
	FEModel& fem = prj.GetFEModel();
	for (int i = 0; i<fem.Steps(); ++i)
	{
		step->addItem(QString::fromStdString(fem.GetStep(i)->GetName()));
	}

	m_module = prj.GetModule();

	// set the types
	vector<FEClassFactory*> l = FEMKernel::FindAllClasses(m_module, superID);
	for (int i=0; i<(int)l.size(); ++i)
	{
		FEClassFactory* fac = l[i];

		QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(fac->GetTypeStr()));
		item->setData(Qt::UserRole, fac->GetClassID());
		type->addItem(item);
	}

	type->setCurrentRow(0);

	QObject::connect(type, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(accept()));
	QObject::connect(type, &QListWidget::currentRowChanged, this, &CHelpDialog::LoadPage);
}

std::string CDlgAddPhysicsItem::GetName()
{
	return name->text().toStdString();
}

int CDlgAddPhysicsItem::GetStep()
{
	return step->currentIndex();
}

int CDlgAddPhysicsItem::GetClassID()
{
	return type->currentItem()->data(Qt::UserRole).toInt();
}

void CDlgAddPhysicsItem::SetURL()
{
	int classID = type->currentItem()->data(Qt::UserRole).toInt();

	m_url = FEMKernel::FindClass(m_module, m_superID, classID)->GetHelpURL();
}



//=================================================================================================

class UIDlgAddPhysicsItem2
{
public:
	QListWidget* type;
	QLineEdit* name;
	QComboBox* step;
	QLineEdit* flt;

	int m_superID;

public:
	void setup(CDlgAddPhysicsItem2* dlg)
	{
		// Setup UI
		QString placeHolder = "(leave blank for default)";
		name = new QLineEdit; name->setPlaceholderText(placeHolder);
		name->setMinimumWidth(name->fontMetrics().size(Qt::TextSingleLine, placeHolder).width() * 1.3);

		step = new QComboBox;
		type = new QListWidget;

		QFormLayout* form = new QFormLayout;
		form->setLabelAlignment(Qt::AlignRight);
		form->addRow("Name:", name);
		form->addRow("Step:", step);

		QVBoxLayout* layout = new QVBoxLayout;

		QHBoxLayout* h = new QHBoxLayout;
		h->addWidget(new QLabel("Filter:"));
		h->addWidget(flt = new QLineEdit());
		flt->setPlaceholderText("enter filter text");

		layout->addLayout(form);
		layout->addLayout(h);
		layout->addWidget(type);

		dlg->SetLeftSideLayout(layout);

		QObject::connect(type, SIGNAL(itemDoubleClicked(QListWidgetItem*)), dlg, SLOT(accept()));
		QObject::connect(type, &QListWidget::currentRowChanged, dlg, &CHelpDialog::LoadPage);
		QObject::connect(flt, SIGNAL(textChanged(const QString&)), dlg, SLOT(OnFilterChanged()));

		flt->setFocus();
	}
};

CDlgAddPhysicsItem2::CDlgAddPhysicsItem2(QString windowName, int superID, FEProject& prj, QWidget* parent)
	: CHelpDialog(prj, parent), ui(new UIDlgAddPhysicsItem2)
{
	setWindowTitle(windowName);
	ui->m_superID = superID;
	ui->setup(this);

	// add the steps
	FEModel& fem = prj.GetFEModel();
	for (int i = 0; i < fem.Steps(); ++i)
	{
		ui->step->addItem(QString::fromStdString(fem.GetStep(i)->GetName()));
	}

	m_module = prj.GetModule();

	Update();
}

void CDlgAddPhysicsItem2::Update()
{
	ui->type->clear();

	QString filter = ui->flt->text();

	// set the types
	vector<FEBio::FEBioClassInfo> l = FEBio::FindAllClasses(m_module, ui->m_superID);
	for (int i = 0; i < (int)l.size(); ++i)
	{
		FEBio::FEBioClassInfo& fac = l[i];

		QString type = QString(fac.sztype);

		if (filter.isEmpty() || type.contains(filter, Qt::CaseInsensitive))
		{
			QString name = QString("%1 (%2)").arg(type).arg(fac.szmod);
			QListWidgetItem* item = new QListWidgetItem(name);
			item->setData(Qt::UserRole, fac.classId);
			ui->type->addItem(item);
		}
	}
	ui->type->model()->sort(0);
	if (ui->type->count() > 0) ui->type->setCurrentRow(0);
}

std::string CDlgAddPhysicsItem2::GetName()
{
	return ui->name->text().toStdString();
}

int CDlgAddPhysicsItem2::GetStep()
{
	return ui->step->currentIndex();
}

int CDlgAddPhysicsItem2::GetClassID()
{
	return ui->type->currentItem()->data(Qt::UserRole).toInt();
}

void CDlgAddPhysicsItem2::SetURL()
{
	int classID = ui->type->currentItem()->data(Qt::UserRole).toInt();

//	m_url = FEMKernel::FindClass(m_module, m_superID, classID)->GetHelpURL();
}

void CDlgAddPhysicsItem2::OnFilterChanged()
{
	Update();
}
