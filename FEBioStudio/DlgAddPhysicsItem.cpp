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
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QFormLayout>
#include <QBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QHeaderView>
#include <FEMLib/FEMKernel.h>
#include "MeshTools/FEProject.h"
#include "DlgAddPhysicsItem.h"
#include <FEBioLink/FEBioClass.h>

class UIDlgAddPhysicsItem
{
public:
	QTreeWidget* type;
	QLineEdit* name;
	QComboBox* step;
	QLineEdit* flt;
	QToolButton* tb;

	int m_superID;
	bool	m_modDepends;

public:
	void setup(CDlgAddPhysicsItem* dlg)
	{
		// Setup UI
		QString placeHolder = "(leave blank for default)";
		name = new QLineEdit; name->setPlaceholderText(placeHolder);
		name->setMinimumWidth(name->fontMetrics().size(Qt::TextSingleLine, placeHolder).width() * 1.3);

		step = new QComboBox;
		type = new QTreeWidget;
		type->setColumnCount(2);
		type->setHeaderLabels(QStringList() << "Type" << "Module");
		type->header()->setStretchLastSection(true);
		type->header()->resizeSection(0, 400);

		QFormLayout* form = new QFormLayout;
		form->setLabelAlignment(Qt::AlignRight);
		form->addRow("Name:", name);
		form->addRow("Step:", step);

		QVBoxLayout* layout = new QVBoxLayout;

		QHBoxLayout* h = new QHBoxLayout;
		h->addWidget(new QLabel("Filter:"));
		h->addWidget(flt = new QLineEdit());
		flt->setPlaceholderText("enter filter text");
		h->addWidget(tb = new QToolButton); tb->setText("Aa"); tb->setToolTip("Match case"); tb->setCheckable(true);

		layout->addLayout(form);
		layout->addLayout(h);
		layout->addWidget(type);

		dlg->SetLeftSideLayout(layout);

		QObject::connect(type, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), dlg, SLOT(accept()));
		QObject::connect(type, &QTreeWidget::currentItemChanged, dlg, &CHelpDialog::LoadPage);
		QObject::connect(flt, SIGNAL(textChanged(const QString&)), dlg, SLOT(Update()));
		QObject::connect(tb, SIGNAL(clicked()), dlg, SLOT(Update()));

		flt->setFocus();
	}
};

CDlgAddPhysicsItem::CDlgAddPhysicsItem(QString windowName, int superID, FEProject& prj, bool includeModuleDependencies, QWidget* parent)
	: CHelpDialog(prj, parent), ui(new UIDlgAddPhysicsItem)
{
	setWindowTitle(windowName);
	setMinimumSize(600, 400);

	ui->m_superID = superID;
	ui->m_modDepends = includeModuleDependencies;
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

void CDlgAddPhysicsItem::Update()
{
	ui->type->clear();

	QString filter = ui->flt->text();

	unsigned int searchFlags = (ui->m_modDepends ? FEBio::ClassSearchFlags::IncludeModuleDependencies : 0);

	// set the types
	vector<FEBio::FEBioClassInfo> l = FEBio::FindAllClasses(m_module, ui->m_superID, -1, searchFlags);
	for (int i = 0; i < (int)l.size(); ++i)
	{
		FEBio::FEBioClassInfo& fac = l[i];

		QString type = QString(fac.sztype);

		if (filter.isEmpty() || type.contains(filter, (ui->tb->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive)))
		{
			QTreeWidgetItem* item = new QTreeWidgetItem(ui->type);
			item->setText(0, type);
			item->setText(1, fac.szmod);
			item->setData(0, Qt::UserRole, fac.classId);
		}
	}
	ui->type->model()->sort(0);
}

std::string CDlgAddPhysicsItem::GetName()
{
	return ui->name->text().toStdString();
}

int CDlgAddPhysicsItem::GetStep()
{
	return ui->step->currentIndex();
}

int CDlgAddPhysicsItem::GetClassID()
{
	return ui->type->currentItem()->data(0, Qt::UserRole).toInt();
}

void CDlgAddPhysicsItem::SetURL()
{
	int classID = ui->type->currentItem()->data(0, Qt::UserRole).toInt();
//	m_url = FEMKernel::FindClass(m_module, m_superID, classID)->GetHelpURL();
}
