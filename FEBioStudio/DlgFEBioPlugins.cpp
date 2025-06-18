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
#include <QLabel>
#include <QToolBar>
#include <QProgressBar>
#include "IconProvider.h"
#include <FEBioLib/plugin.h>
#include <FEBioLib/febio.h>
#include <FEBioLink/FEBioClass.h>
#include <FECore/FEModule.h>
#include <QMenu>
#include "PluginListWidget.h"
#include "PublicationWidgetView.h"
#include "MainWindow.h"
#include "PluginManager.h"
#include "DynamicStackedWidget.h"
#include <string>

#include <QDebug>
class CDlgFEBioPluginsUI
{
public:
	CMainWindow* m_wnd = nullptr;

	QTreeWidget* plugins;
	QTreeWidget* features;

	QPushButton* unloadPlugin;

	QMenu* recentPlugins;

public:
	void setup(CDlgFEBioPlugins* dlg)
	{
		QVBoxLayout* l = new QVBoxLayout;

        
        QPushButton* loadPlugin = new QPushButton("Load ...");

		recentPlugins = new QMenu(dlg);
		loadPlugin->setMenu(recentPlugins);

		unloadPlugin = new QPushButton("Unload ...");
		unloadPlugin->setDisabled(true);

		QHBoxLayout* h = new QHBoxLayout;
		h->addWidget(loadPlugin);
		h->addWidget(unloadPlugin);
		h->addStretch();

		plugins = new QTreeWidget;
		plugins->setColumnCount(3);
		plugins->setHeaderLabels(QStringList() << "name" << "version" << "path");
		

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

		QObject::connect(bb, &QDialogButtonBox::rejected, dlg, &CDlgFEBioPlugins::reject);
		QObject::connect(plugins, &QTreeWidget::currentItemChanged, dlg, &CDlgFEBioPlugins::updateFeaturesList);
		// QObject::connect(loadPlugin, &QPushButton::clicked, dlg, &CDlgFEBioPlugins::onLoadPlugin);
		QObject::connect(recentPlugins, &QMenu::triggered, dlg, &CDlgFEBioPlugins::onMenuTriggered);
		QObject::connect(unloadPlugin, &QPushButton::clicked, dlg, &CDlgFEBioPlugins::onUnloadPlugin);
		dlg->setLayout(l);

	}

	void addPlugin(const QString& name, const QString& version, const QString& path)
	{
		QTreeWidgetItem* twi = new QTreeWidgetItem(plugins);
		twi->setText(0, name);
		twi->setText(1, version);
		twi->setText(2, path);
	}

	void updateRecentPlugins()
	{
		recentPlugins->clear();
		QStringList l = m_wnd->GetRecentPluginsList();
		for (int i = 0; i < l.size(); ++i)
		{
			recentPlugins->addAction(l[i]);
		}
		recentPlugins->addAction("<other...>");
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
		if ((n < 0) || (n >= pm->Plugins()))
		{
			unloadPlugin->setDisabled(true);
			return;
		}
		unloadPlugin->setEnabled(true);

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

CDlgFEBioPlugins::CDlgFEBioPlugins(CMainWindow* parent) 
    : QDialog(parent), ui(new CDlgFEBioPluginsUI)
{
	setWindowTitle("FEBio Plugins");

	setMinimumSize(800, 600);

	ui->m_wnd = parent;
	ui->setup(this);
	ui->updateRecentPlugins();
	ui->updatePluginsList();

    
}


void CDlgFEBioPlugins::updateFeaturesList()
{
	ui->updateFeaturesList();
}

void CDlgFEBioPlugins::onMenuTriggered(QAction* action)
{
	QString t = action->text();
	if (t == "<other...>")
		onLoadPlugin();
	else
	{
		LoadPlugin(t);
	}
}

void CDlgFEBioPlugins::onLoadPlugin()
{
#ifdef WIN32
    QString fileName = QFileDialog::getOpenFileName(this, "Load Plugin", "", "FEBio Plugins (*.dll)");
#elif __APPLE__
    QString fileName = QFileDialog::getOpenFileName(this, "Load Plugin", "", "FEBio Plugins (*.dylib)");
#else
    QString fileName = QFileDialog::getOpenFileName(this, "Load Plugin", "", "FEBio Plugins (*.so)");
#endif
	
	if (fileName.isEmpty() == false)
	{
		fileName = QDir::toNativeSeparators(fileName);
		LoadPlugin(fileName);
	}
}

bool LoadFEBioPlugin(const QString& pluginFile)
{
	std::string sfile = pluginFile.toStdString();

	// get the currently active module
	// We need this, since importing the plugin might change this.
	FECoreKernel& fecore = FECoreKernel::GetInstance();

	FEModule* activeMod = fecore.GetActiveModule();
	int modId = -1;
	if (activeMod) modId = activeMod->GetModuleID();

	// try to import the plugin
	bool bsuccess = febio::ImportPlugin(sfile.c_str());

	// restore active module
	fecore.SetActiveModule(modId);

	return bsuccess;
}

void CDlgFEBioPlugins::LoadPlugin(const QString& fileName)
{
	bool bsuccess = LoadFEBioPlugin(fileName);

	if (bsuccess == false)
	{
		QMessageBox::critical(this, "Load Plugin", QString("The plugin failed to load:\n%1").arg(fileName));
	}
	else
	{
		QMessageBox::information(this, "Load Plugin", QString("The plugin loaded successfully:\n%1").arg(fileName));
		ui->m_wnd->AddRecentPlugin(fileName);
		ui->updateRecentPlugins();
		ui->updatePluginsList();
		ui->selectPlugin(ui->pluginCount() - 1);
	}
}

void CDlgFEBioPlugins::onUnloadPlugin()
{
	QTreeWidgetItem* it = ui->plugins->currentItem();
	if (it == nullptr)
	{
		QMessageBox::information(this, "Unload plugin", QString("Please select the plugin to unload."));
		return;
	}
	
	QString name = it->text(0);

	QString msg = QString("Are you sure you want to unload the plugin %1.\nAny open model that uses it may not work correctly anymore.").arg(name);
	if (QMessageBox::question(this, "Unload plugin", msg) == QMessageBox::Yes)
	{
		FEBioPluginManager* pm = FEBioPluginManager::GetInstance(); assert(pm);
		if (pm->UnloadPlugin(name.toStdString()))
		{
			QMessageBox::information(this, "Unload plugin", QString("Plugin %1 unloaded successfully.").arg(name));
		}
		else
		{
			QMessageBox::critical(this, "Unload plugin", QString("Failed to unload plugin %1.").arg(name));
		}

		ui->updatePluginsList();
	}
}
