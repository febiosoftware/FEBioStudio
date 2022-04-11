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
#include "DlgFEBioPlugins.h"
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QTreeWidget>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QSplitter>
#include <FEBioLib/plugin.h>
#include <FEBioLib/febio.h>
#include <FEBioLink/FEBioClass.h>
#include <FECore/FEModule.h>

class CDlgFEBioPluginsUI
{
public:
	QTreeWidget* plugins;
	QTreeWidget* features;

public:
	void setup(QDialog* dlg)
	{
		QPushButton* loadPlugin = new QPushButton("Load ...");

		QHBoxLayout* h = new QHBoxLayout;
		h->addWidget(loadPlugin);
		h->addStretch();

		plugins = new QTreeWidget;
		plugins->setColumnCount(3);
		plugins->setHeaderLabels(QStringList() << "name" << "version" << "path");
		QVBoxLayout* l = new QVBoxLayout;

		features = new QTreeWidget;
		features->setColumnCount(5);
		features->setHeaderLabels(QStringList() << "type string" << "super class" << "class name" << "base class" << "module");

		QSplitter* split = new QSplitter;
		split->setOrientation(Qt::Vertical);
		split->addWidget(plugins);
		split->addWidget(features);
		split->setStretchFactor(0, 1);
		split->setStretchFactor(1, 2);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Close);
		
		l->addLayout(h);
		l->addWidget(split);
		l->addWidget(bb);

		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
		QObject::connect(plugins, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), dlg, SLOT(updateFeaturesList()));
		QObject::connect(loadPlugin, SIGNAL(clicked(bool)), dlg, SLOT(onLoadPlugin()));
		dlg->setLayout(l);
	}

	void addPlugin(const QString& name, const QString& version, const QString& path)
	{
		QTreeWidgetItem* twi = new QTreeWidgetItem(plugins);
		twi->setText(0, name);
		twi->setText(1, version);
		twi->setText(2, path);
	}

	void updatePluginsList()
	{
		FEBioPluginManager* pm = FEBioPluginManager::GetInstance();
		plugins->clear();

		for (int i = 0; i < pm->Plugins(); ++i)
		{
			const FEBioPlugin& pi = pm->GetPlugin(i);

			QString name = pi.GetName();
			FEBioPlugin::Version v = pi.GetVersion();
			QString version = QString("%1.%2.%3").arg(v.major).arg(v.minor).arg(v.patch);
			QString path = QString::fromStdString(pi.GetFilePath());
			addPlugin(name, version, path);
		}
	}

	int pluginCount()
	{
		return plugins->topLevelItemCount();
	}

	void selectPlugin(int n)
	{
		int m = plugins->topLevelItemCount();
		if ((n < 0) || (n >= m)) plugins->setCurrentItem(nullptr);
		else
		{
			QTreeWidgetItem* twi = plugins->topLevelItem(n);
			plugins->setCurrentItem(twi);
		}
	}

	void addFeature(const QString& type, const QString& superClass, const QString& className, const QString& baseClass, const QString moduleName)
	{
		QTreeWidgetItem* twi = new QTreeWidgetItem(features);
		twi->setText(0, type);
		twi->setText(1, superClass);
		twi->setText(2, className);
		twi->setText(3, baseClass);
		twi->setText(4, moduleName);
	}

	void updateFeaturesList()
	{
		FEBioPluginManager* pm = FEBioPluginManager::GetInstance(); assert(pm);

		features->clear();
		int n = plugins->currentIndex().row();
		if ((n < 0) || (n >= pm->Plugins())) return;

		const FEBioPlugin& pi = pm->GetPlugin(n);

		std::vector<FEBio::FEBioClassInfo> classList = FEBio::FindAllPluginClasses(pi.GetAllocatorID());
		for (FEBio::FEBioClassInfo& ci : classList)
		{
			QString baseClass = QString::fromStdString(FEBio::GetBaseClassName(ci.baseClassId));
			QString superClass = FEBio::GetSuperClassString(ci.superClassId);
			addFeature(ci.sztype, superClass, ci.szclass, baseClass, ci.szmod);
		}
	}
};

CDlgFEBioPlugins::CDlgFEBioPlugins(QWidget* parent) : QDialog(parent), ui(new CDlgFEBioPluginsUI)
{
	setWindowTitle("FEBio Plugins");

	setMinimumSize(800, 600);

	ui->setup(this);
	ui->updatePluginsList();
}

void CDlgFEBioPlugins::updateFeaturesList()
{
	ui->updateFeaturesList();
}

void CDlgFEBioPlugins::onLoadPlugin()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Load Plugin", "", "FEBio Plugins (*.dll)");
	if (fileName.isEmpty() == false)
	{
		std::string sfile = fileName.toStdString();

		// get the currently active module
		// We need this, since importing the plugin might change this.
		FECoreKernel& fecore = FECoreKernel::GetInstance();
		int modId = fecore.GetActiveModule()->GetModuleID();

		// try to import the plugin
		bool bsuccess = febio::ImportPlugin(sfile.c_str());

		// restore active module
		fecore.SetActiveModule(modId);

		if (bsuccess == false)
		{
			QMessageBox::critical(this, "Load Plugin", QString("The plugin failed to load:\n%1").arg(fileName));
		}
		else
		{
			QMessageBox::information(this, "Load Plugin", QString("The plugin loaded successfully:\n%1").arg(fileName));
			ui->updatePluginsList();
			ui->selectPlugin(ui->pluginCount() - 1);
		}
	}
}
