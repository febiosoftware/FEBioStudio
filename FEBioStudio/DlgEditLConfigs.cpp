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
#include <QAction>
#include <QToolButton>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QSplitter>
#include <QLabel>
#include <QInputDialog>
#include <QGroupBox>
#include "DlgEditLConfigs.h"
#include "ObjectProps.h"
#include "PropertyListForm.h"
#include "LaunchConfig.h"

class Ui::CDlgEditLaunchConfigs
{
public:
	// left page
	QListWidget* launchConfigList;

	QToolButton* addConfigBtn;
	QToolButton* delConfigBtn;

	// right page
	QLabel*	launchType;
	CPropertyListForm* props;
	QPlainTextEdit* edit;

	std::vector<CLaunchConfig*>& launchConfigs;

	CDlgEditLaunchConfigs(std::vector<CLaunchConfig*>& launchConfigs) : launchConfigs(launchConfigs){}

	void setup(QDialog* dlg)
	{
		// build the left, right panes
		QWidget* leftPane = buildLeftPane();
		QWidget* rightPane = buildRightPane();

		// build the splitter
		QSplitter* split = new QSplitter;
		split->setOrientation(Qt::Horizontal);
		split->addWidget(leftPane);
		split->addWidget(rightPane);
		split->setStretchFactor(0, 1);
		split->setStretchFactor(1, 2);

		// build the main layout
		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->addWidget(split);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok);
		mainLayout->addWidget(bb);

		dlg->setLayout(mainLayout);

		QObject::connect(addConfigBtn, SIGNAL(clicked()), dlg, SLOT(on_addConfigBtn_Clicked()));
		QObject::connect(delConfigBtn, SIGNAL(clicked()), dlg, SLOT(on_delConfigBtn_Clicked()));

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
		QObject::connect(launchConfigList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), dlg, SLOT(on_selection_change(QListWidgetItem*, QListWidgetItem*)));
		QObject::connect(launchConfigList, SIGNAL(itemChanged(QListWidgetItem*)), dlg, SLOT(on_nameChanged(QListWidgetItem*)));

		QObject::connect(edit, SIGNAL(textChanged()), dlg, SLOT(on_textChanged()));
	}

	QWidget* buildLeftPane()
	{
		// build the launch configuration widget
		launchConfigList = new QListWidget;
		launchConfigList->setEditTriggers(QAbstractItemView::DoubleClicked);
		launchConfigList->setDragDropMode(QAbstractItemView::InternalMove);
		launchConfigList->setDropIndicatorShown(true);
		for (int i=0; i<launchConfigs.size(); ++i)
		{
			CLaunchConfig* lc = launchConfigs[i];
			QListWidgetItem* item = new QListWidgetItem(lc->name().c_str(), launchConfigList);
			item->setFlags(item->flags() | Qt::ItemIsEditable);
			item->setData(Qt::UserRole, i);
		}

		// build the add config button
		QAction* addConfig = new QAction;
		addConfig->setIcon(QIcon(":/icons/selectAdd.png"));
		addConfigBtn = new QToolButton;
		addConfigBtn->setDefaultAction(addConfig);
		addConfigBtn->setToolTip("Add a new launch configuration");

		// build the delete config button
		QAction* delConfig = new QAction;
		delConfig->setIcon(QIcon(":/icons/selectSub.png"));
		delConfigBtn = new QToolButton;
		delConfigBtn->setDefaultAction(delConfig);
		delConfigBtn->setToolTip("Remove launch configuration");

		// build the layouts
		QHBoxLayout* h1 = new QHBoxLayout;
		h1->setContentsMargins(0, 0, 0, 0);
		h1->addWidget(addConfigBtn);
		h1->addWidget(delConfigBtn);
		h1->setAlignment(Qt::AlignLeft);

		QVBoxLayout* v2 = new QVBoxLayout;
		v2->setContentsMargins(0, 0, 0, 0);
		v2->addWidget(launchConfigList);
		v2->addLayout(h1);

		QWidget* leftPane = new QWidget;
		leftPane->setLayout(v2);

		return leftPane;
	}

	QWidget* buildRightPane()
	{
		// form for right-hand side pane
		QVBoxLayout* l = new QVBoxLayout;
		QHBoxLayout* h = new QHBoxLayout;
		h->addWidget(new QLabel("Launch type:"));
		h->addWidget(launchType = new QLabel);
		launchType->setAlignment(Qt::AlignLeft);
		h->addStretch();
		l->addLayout(h);

		QGroupBox* b = new QGroupBox("Settings:");
		QVBoxLayout* bl = new QVBoxLayout;
		bl->addWidget(props = new CPropertyListForm);
		b->setLayout(bl);

		l->addWidget(b);
		l->addWidget(edit = new QPlainTextEdit);
		l->addStretch();

		QWidget* rightPane = new QWidget;
		rightPane->setLayout(l);

		return rightPane;
	}

	CLaunchConfig* currentConfig()
	{
		int n = launchConfigList->currentIndex().row();
		if ((n >= 0) && (n < (int)launchConfigs.size()))
		{
			return launchConfigs[n];
		}
		return nullptr;
	}
};

CDlgEditLaunchConfigs::CDlgEditLaunchConfigs(QWidget* parent, std::vector<CLaunchConfig*>* launchConfigs, int select)
	: QDialog(parent), ui(new Ui::CDlgEditLaunchConfigs(*launchConfigs))
{
	setMinimumSize(800, 400);
	ui->setup(this);
	if (select != -1) ui->launchConfigList->setCurrentRow(select);
}

void CDlgEditLaunchConfigs::on_textChanged()
{
	QListWidgetItem* it = ui->launchConfigList->currentItem();
	if (it)
	{
		int index = it->data(Qt::UserRole).toInt();
		if (index >= 0)
		{
			CLaunchConfig* lc = ui->launchConfigs[index];
			Param* p = lc->GetParam("text");
			if (p)
			{
				QString s = ui->edit->toPlainText();
				p->SetStringValue(s.toStdString());
			}
		}
	}
}

void CDlgEditLaunchConfigs::on_selection_change(QListWidgetItem* current, QListWidgetItem* previous)
{
	if(!current) return;

	// Change to the current config
	int index = current->data(Qt::UserRole).toInt();
	if (index >= 0)
	{
		CLaunchConfig* lc = ui->launchConfigs[index];
		CObjectProps* props = new CObjectProps(lc, false);
		ui->props->setPropertyList(props);

		ui->launchType->setText(QString::fromStdString(lc->typeString()));
		ui->launchType->show();

		Param* p = lc->GetParam("text");
		if (p)
		{
			ui->edit->show();
			ui->edit->setPlainText(p->GetStringValue().c_str());
		}
		else ui->edit->hide();
	}
	else
	{
		ui->launchType->hide();
		ui->props->setPropertyList(nullptr);
		ui->edit->hide();
	}
}

void CDlgEditLaunchConfigs::on_addConfigBtn_Clicked()
{
	QStringList types = { "local", "remote", "PBS", "SLURM", "custom" };
	bool ok = false;
	QString choice = QInputDialog::getItem(this, "Create new configuration", "Select the configuration type:", types, 0, false, &ok);
	if (!ok) return;
	
	std::string defaultName = "new";
	CLaunchConfig* lc = nullptr;
	if (choice == "local" ) lc = new CLocalLaunchConfig(defaultName);
	if (choice == "remote") lc = new CRemoteLaunchConfig(defaultName);
	if (choice == "PBS"   ) lc = new CPBSLaunchConfig(defaultName);
	if (choice == "SLURM" ) lc = new CSLURMLaunchConfig(defaultName);
	if (choice == "custom") lc = new CCustomLaunchConfig(defaultName);
	if (lc == nullptr) return;

	int n = ui->launchConfigList->count();
	QListWidgetItem* newItem = new QListWidgetItem();
	newItem->setText(QString::fromStdString(defaultName));
	newItem->setData(Qt::UserRole, n);
	newItem->setFlags(newItem->flags () | Qt::ItemIsEditable);
	ui->launchConfigList->addItem(newItem);

	ui->launchConfigs.push_back(lc);
	ui->launchConfigList->setCurrentItem(newItem);
	ui->launchConfigList->editItem(newItem);
}

void CDlgEditLaunchConfigs::on_delConfigBtn_Clicked()
{
	if(ui->launchConfigs.size() == 1)
	{
		QMessageBox::critical(this, "FEBio Studio", "You cannot delete the last launch configuration.");
		return;
	}

	QListWidgetItem* current = ui->launchConfigList->takeItem(ui->launchConfigList->currentRow());
	int n = current->data(Qt::UserRole).toInt();
	ui->launchConfigs.erase(ui->launchConfigs.begin() + n);
	delete current;
	ui->launchConfigList->setCurrentRow(0);
}

void CDlgEditLaunchConfigs::on_nameChanged(QListWidgetItem* item)
{
	if (item == nullptr) return;

	std::string s = item->text().toStdString();
	CLaunchConfig* lc = ui->currentConfig();
	if (lc && (lc->name() != s))
	{
		lc->setName(s);
	}
}

int CDlgEditLaunchConfigs::GetLCIndex()
{
	return ui->launchConfigList->currentRow();
}

