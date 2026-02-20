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

#include <QLineEdit>
#include <QComboBox>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QFormLayout>
#include <QBoxLayout>
#include <QMessageBox>
#include <QLabel>
#include <QToolButton>
#include <QHeaderView>
#include <QDialogButtonBox>
#include <QListWidget>
#include <unordered_map>
#include <FEMLib/FEMKernel.h>
#include <FEMLib/FSProject.h>
#include <FEMLib/FEMaterial.h>
#include "DlgAddPhysicsItem.h"
#include <FEBioLink/FEBioClass.h>
#include <FEBioLink/FEBioModule.h>
#include <FSCore/FSCore.h>
#include "HelpFeature.h"

using namespace std;

class UIDlgAddPhysicsItem
{
public:
	QTreeWidget* type;
	QLineEdit* name;
	QComboBox* step;
	QLineEdit* flt;
	QToolButton* tb;
	QComboBox* cat;

	QWidget* nameAndCategory;

	int		m_superID;
	int		m_baseClassID;
	bool	m_modDepends;

	std::unordered_map<int, std::vector<FEBio::FEBioClassInfo> > classList;

public:
	void setup(CDlgAddPhysicsItem* dlg, bool showStepList)
	{
		// Setup UI
		QString placeHolder = "(leave blank for default)";
		name = new QLineEdit; name->setPlaceholderText(placeHolder);
		name->setMinimumWidth(name->fontMetrics().size(Qt::TextSingleLine, placeHolder).width() * 1.3);

		type = new QTreeWidget;
		type->setColumnCount(3);
		type->setHeaderLabels(QStringList() << "Type" << "Category" << "Module");
		type->header()->setStretchLastSection(true);
		type->header()->resizeSection(0, 400);
		type->header()->resizeSection(1, 200);

		QFormLayout* form = new QFormLayout;
		form->setLabelAlignment(Qt::AlignRight);
		form->addRow("Name:", name);

		step = nullptr;
		if (showStepList)
		{
			step = new QComboBox;
			form->addRow("Step:", step);
		}
		form->addRow("Category:", cat = new QComboBox());

		nameAndCategory = new QWidget;
		nameAndCategory->setLayout(form);

		QVBoxLayout* layout = new QVBoxLayout;

		QHBoxLayout* h = new QHBoxLayout;
		h->addWidget(new QLabel("Filter:"));
		h->addWidget(flt = new QLineEdit());
		flt->setPlaceholderText("enter filter text");
		h->addWidget(tb = new QToolButton); tb->setText("Aa"); tb->setToolTip("Match case"); tb->setCheckable(true);
		
		layout->addWidget(nameAndCategory);
		layout->addLayout(h);
		layout->addWidget(type);

        QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help);

		layout->addWidget(bb);

		dlg->setLayout(layout);
        
        QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
		QObject::connect(bb, SIGNAL(helpRequested()), dlg, SLOT(on_help_clicked()));
		QObject::connect(type, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), dlg, SLOT(accept()));
		QObject::connect(flt, SIGNAL(textChanged(const QString&)), dlg, SLOT(Update()));
		QObject::connect(tb, SIGNAL(clicked()), dlg, SLOT(Update()));
		QObject::connect(cat, SIGNAL(currentIndexChanged(int)), dlg, SLOT(Update()));

		flt->setFocus();
	}
};

CDlgAddPhysicsItem::CDlgAddPhysicsItem(QString windowName, int superID, int baseClassID, FSModel* fem, bool includeModuleDependencies, bool showStepList, QWidget* parent)
	: QDialog(parent), ui(new UIDlgAddPhysicsItem)
{
	setWindowTitle(windowName);
	setMinimumSize(800, 600);

	ui->m_superID = superID;
	ui->m_baseClassID = baseClassID;
	ui->m_modDepends = includeModuleDependencies;
	ui->setup(this, showStepList);

	// add the steps
	if (ui->step)
	{
		assert(fem);
		for (int i = 0; i < fem->Steps(); ++i)
		{
			ui->step->addItem(QString::fromStdString(fem->GetStep(i)->GetName()));
		}
	}

	unsigned int searchFlags = 0;
	if (ui->m_modDepends) searchFlags |= FEBio::ClassSearchFlags::IncludeModuleDependencies;

	// set the types
	vector<FEBio::FEBioClassInfo> l = FEBio::FindAllClasses(FEBio::GetActiveModule(), ui->m_superID, ui->m_baseClassID, searchFlags);
	for (int i = 0; i < (int)l.size(); ++i)
	{
		FEBio::FEBioClassInfo& fac = l[i];
		if ((fac.spec == -1) || (fac.spec >= 0x400))
			ui->classList[fac.baseClassId].push_back(fac);
	}

	ui->cat->clear();
	ui->cat->addItem("(all)", -2);
	for (auto& it : ui->classList)
	{
		int id = it.first;
		std::string s = FEBio::GetBaseClassName(id); assert(s.empty() == false);

		std::string bs = FSCore::beautify_string(s.c_str());

		ui->cat->addItem(QString::fromStdString(bs), id);
	}

	Update();
}

void CDlgAddPhysicsItem::ShowNameAndCategoryFields(bool b)
{
	ui->nameAndCategory->setVisible(b);
}

void CDlgAddPhysicsItem::Update()
{
	ui->type->clear();

	QString filter = ui->flt->text();

	int classId = ui->cat->currentData().toInt();

	// set the types
	for (auto& it : ui->classList)
	{
		std::vector<FEBio::FEBioClassInfo>& facList = it.second;

		for (auto& fac : facList)
		{
			if ((classId == -2) || (fac.baseClassId == classId))
			{
				QString type = QString::fromStdString(FSCore::beautify_string(fac.sztype));

				if (filter.isEmpty() || type.contains(filter, (ui->tb->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive)))
				{
					std::string s = FEBio::GetBaseClassName(fac.baseClassId); assert(s.empty() == false);
					std::string bs = FSCore::beautify_string(s.c_str());

					QTreeWidgetItem* item = new QTreeWidgetItem(ui->type);
					item->setText(0, type);
					item->setText(1, QString::fromStdString(bs));
					const char* szmod = fac.szmod;
					if ((szmod == nullptr) || (szmod[0] == 0)) szmod = "(core)";
					item->setText(2, szmod);
					item->setData(0, Qt::UserRole, fac.classId);
				}
			}
		}
	}
	ui->type->model()->sort(0);
}

void CDlgAddPhysicsItem::on_help_clicked()
{
    if(ui->type->selectedItems().size() > 0)
    {
        int classID = ui->type->currentItem()->data(0, Qt::UserRole).toInt();

        ShowHelp(ClassIDToURL(classID));
    }
    else
    {
		QMessageBox::information(this, "Help", "Please select an item before clicking Help.");
    }
}

std::string CDlgAddPhysicsItem::GetName()
{
	return ui->name->text().toStdString();
}

int CDlgAddPhysicsItem::GetStep()
{
	return (ui->step ? ui->step->currentIndex() : -1);
}

int CDlgAddPhysicsItem::GetClassID()
{
	QTreeWidgetItem* it = ui->type->currentItem();
	return (it ? it->data(0, Qt::UserRole).toInt() : -1);
}

class UIDlgCopyPhysicsItem
{
private:
	QListWidget* list;
	std::vector<FSModelComponent*> m_comp;

public:
	void setup(QDialog* dlg)
	{
		QVBoxLayout* l = new QVBoxLayout;
		list = new QListWidget;

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		l->addWidget(list);
		l->addWidget(bb);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));

		dlg->setLayout(l);
	}

	void buildList(FSModelComponent* src, FSModel* fem, int superID, int baseClassID)
	{
		for (int i = 0; i < fem->Materials(); ++i)
		{
			GMaterial* gmat = fem->GetMaterial(i);
			FSModelComponent* pmat = gmat->GetMaterialProperties();
			QString name = QString::fromStdString(gmat->GetName());
			if (pmat && (pmat != src)) addToList(pmat, superID, baseClassID, name);
		}
	}

	void addToList(FSModelComponent* pc, int superID, int baseClassID, QString rootName)
	{
		if (pc == nullptr) return;

		if (pc->GetSuperClassID() == superID)
		{
			int baseId = FEBio::GetBaseClassIndex(superID, pc->GetTypeString());
			if (baseId == baseClassID)
			{
				QString name = rootName + QString(" [%1]").arg(pc->GetTypeString());
				list->addItem(name);
				m_comp.push_back(pc);
			}
		}
		for (int i = 0; i < pc->Properties(); ++i)
		{
			FSProperty& prop = pc->GetProperty(i);
			QString propName = QString::fromStdString(prop.GetName());
			for (int j = 0; j < prop.Size(); ++j)
			{
				FSModelComponent* pj = dynamic_cast<FSModelComponent*>(prop.GetComponent(j));
				if (pj)
				{
					QString name = rootName + "." + propName;
					if (prop.Size() > 1) name += QString("[%1]").arg(j);
					addToList(pj, superID, baseClassID, name);
				}
			}
		}
	}

	FSModelComponent* selectedComponent()
	{
		int n = list->currentIndex().row();
		if ((n >= 0) && (n < m_comp.size())) return m_comp[n];
		return nullptr;
	}
};

CDlgCopyPhysicsItem::CDlgCopyPhysicsItem(QString windowTitle, int superID, int baseClassID, FSModelComponent* src, FSModel* fem, QWidget* parent) : QDialog(parent), ui(new UIDlgCopyPhysicsItem)
{
	setMinimumSize(600, 400);
	ui->setup(this);
	if (fem) ui->buildList(src, fem, superID, baseClassID);
}

FSModelComponent* CDlgCopyPhysicsItem::GetSelectedComponent()
{
	return ui->selectedComponent();
}
