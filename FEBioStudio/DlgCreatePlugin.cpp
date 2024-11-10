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
#include "DlgCreatePlugin.h"
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include "MainWindow.h"
#include "FEBioStudioProject.h"
#include <QtCore/QDir>
#include "ui_createplugin.h"

#ifdef WIN32
    const char* sep = "\\";
#else
    const char* sep = "/";
#endif

//=============================================================================
CDlgCreatePlugin::CDlgCreatePlugin(CMainWindow* parent) : QWizard(parent), ui(new CDlgCreatePluginUI)
{
	setWindowTitle("Create FEBio Plugin");
	setMinimumSize(QSize(600, 300));
	ui->m_wnd = parent;

	// TODO: Do we still need this?
	if (parent->usingDarkTheme())
	{
		setWizardStyle(QWizard::ClassicStyle);
		setStyleSheet("background-color:#353535");
	}

	ui->setup(this);

	QString createPath;
	FEBioStudioProject* prj = ui->m_wnd->GetProject();

	if (prj && (prj->GetProjectPath().isEmpty() == false))
	{
		createPath = prj->GetProjectPath();
		ui->mainPage->m_path->setResourceName(createPath);
		ui->mainPage->m_path->setEnabled(false);
	}
	else
	{
		createPath = parent->GetCreatePluginPath();
		ui->mainPage->m_path->setResourceName(createPath);
	}
}

void CDlgCreatePlugin::accept()
{
	// collect input fields
	QString pluginName   = ui->mainPage->m_name->text();
	QString pluginPath   = ui->mainPage->m_path->resourceName();

	int modid = ui->mainPage->m_mod->currentData().toInt();
	QString pluginModule = (modid != -1 ? ui->mainPage->m_mod->currentText() : "");

	QString className = ui->configPage->m_className->text();
	QString typeStr = ui->configPage->m_typeString->text();
	if (typeStr.isEmpty()) typeStr = className;

	// check input
	if (pluginName.isEmpty())
	{
		QMessageBox::critical(this, "FEBio Studio", "Please choose a name for the plugin.");
		return;
	}

	if (pluginPath.isEmpty())
	{
		QMessageBox::critical(this, "FEBio Studio", "Please select a path where the plugin files will be created.");
		return;
	}

	// get the active project
	FEBioStudioProject* prj = ui->m_wnd->GetProject();
	if (prj == nullptr)
	{
		QMessageBox::critical(this, "FEBio Studio", "No project active. Cannot add plugin.");
		return;
	}

	// if the project was already saved, we'll add a new folder to the project's folder
	QDir dir(pluginPath);
	if (!prj->GetProjectPath().isEmpty())
	{
		assert(pluginPath == prj->GetProjectPath());
		if (dir.mkdir(pluginName) == false)
		{
			QMessageBox::critical(this, "FEBio Studio", "The folder already exists.");
			return;
		}
		dir.cd(pluginName);
	}
	else
	{
		// create a folder to save the project in
		if (dir.mkdir(pluginName) == false)
		{
			QMessageBox::critical(this, "FEBio Studio", "The folder already exists.");
			return;
		}

		// save this as the default path
		ui->m_wnd->SetCreatePluginPath(QDir::toNativeSeparators(dir.absolutePath()));

		// automatically save the project file
		dir.cd(pluginName);
		pluginPath = QDir::toNativeSeparators(dir.absolutePath());
		QString prjPath = pluginPath + sep + pluginName + ".fsp";
		prj->Save(prjPath);

		// then, add a folder for the plugin
		if (dir.mkdir(pluginName) == false)
		{
			QMessageBox::critical(this, "FEBio Studio", "The folder already exists.");
			return;
		}
		dir.cd(pluginName);
	}
	pluginPath = QDir::toNativeSeparators(dir.absolutePath());


	CPluginTemplate* pluginTemplate = ui->GetPluginTemplate();
	if (pluginTemplate == nullptr)
	{
		QMessageBox::critical(this, "FEBio Studio", "No valid plugin template found.");
		return;
	}

	// generate the plugin meta data
	PluginConfig config;
	config.name        = pluginName;
	config.febioModule = pluginModule;
	config.path        = pluginPath;
	config.className   = className;
	config.typeString  = typeStr;
	config.headerTxt   = pluginTemplate->m_header;
	config.sourceTxt   = pluginTemplate->m_source;
	config.args        = ui->opsPage->GetOptions();
	config.sdkInc      = ui->m_wnd->GetSDKIncludePath();
	config.sdkLib      = ui->m_wnd->GetSDKLibraryPath();
	config.headerFile  = config.path + sep + config.className + ".h";
	config.sourceFile  = config.path + sep + config.className + ".cpp";
	config.mainFile    = config.path + sep + "main.cpp";
	config.cmakeFile   = config.path + sep + "CMakeLists.txt";

	// generate all files
	if (GeneratePluginFiles(config) == false)
	{
		QMessageBox::critical(this, "FEBio Studio", "Failed to create plugin.");
		return;
	}

	// add plugin to project
	FEBioStudioProject::ProjectItem* grp = prj->AddPlugin(config.name);
	if (grp)
	{
		grp->AddFile(config.cmakeFile);

		FEBioStudioProject::ProjectItem& headerFiles = grp->AddGroup("Include");
		headerFiles.AddFile(config.headerFile);

		FEBioStudioProject::ProjectItem& sourceFiles = grp->AddGroup("Source");
		sourceFiles.AddFile(config.mainFile);
		sourceFiles.AddFile(config.sourceFile);

		prj->Save();
	}

	// all is well
	QMessageBox::information(this, "FEBio Studio", QString("The plugin was created successfully in\n%1").arg(pluginPath));

	QDialog::accept();
}

//=============================================================================
CDlgAddPluginClass::CDlgAddPluginClass(CMainWindow* parent, FEBioStudioProject* prj, FEBioStudioProject::ProjectItem* plugin) : QWizard(parent), ui(new CDlgAddPluginClassUI)
{
	setWindowTitle("Add FEBio Feature");
	setMinimumSize(QSize(600, 300));
	ui->m_wnd = parent;

	// TODO: do we still need this?
	if (parent->usingDarkTheme())
	{
		setWizardStyle(QWizard::ClassicStyle);
		setStyleSheet("background-color:#353535");
	}
	ui->setup(this);
	ui->prj = prj;
	ui->plugin = plugin;
}

void CDlgAddPluginClass::accept()
{
	if ((ui->prj == nullptr) || (ui->plugin == nullptr)) return;

	// collect input fields
	QString className = ui->configPage->m_className->text();
	QString typeStr = ui->configPage->m_typeString->text();
	if (typeStr.isEmpty()) typeStr = className;

	CPluginTemplate* pluginTemplate = ui->GetPluginTemplate();
	if (pluginTemplate == nullptr)
	{
		QMessageBox::critical(this, "FEBio Studio", "No valid plugin template found.");
		return;
	}

	QString pluginName = ui->plugin->Name();
	QString pluginPath = ui->prj->ToAbsolutePath("./" + pluginName);

	// generate the plugin meta data
	PluginConfig config;
	config.name = ui->plugin->Name();
	config.febioModule = "";
	config.path = pluginPath;
	config.className = className;
	config.typeString = typeStr;
	config.headerTxt = pluginTemplate->m_header;
	config.sourceTxt = pluginTemplate->m_source;
	config.args = ui->opsPage->GetOptions();
	config.sdkInc = ui->m_wnd->GetSDKIncludePath();
	config.sdkLib = ui->m_wnd->GetSDKLibraryPath();
	config.headerFile = config.path + sep + config.className + ".h";
	config.sourceFile = config.path + sep + config.className + ".cpp";
	config.mainFile = config.path + sep + "main.cpp";
	config.cmakeFile = config.path + sep + "CMakeLists.txt";

	// generate all files
	if (GeneratePluginFiles(config) == false)
	{
		QMessageBox::critical(this, "FEBio Studio", "Failed to create plugin.");
		return;
	}

	// add plugin to project
	FEBioStudioProject::ProjectItem* grp = ui->plugin;

	FEBioStudioProject::ProjectItem& headerFiles = *grp->FindItem("Include");
	headerFiles.AddFile(config.headerFile);

	FEBioStudioProject::ProjectItem& sourceFiles = *grp->FindItem("Source");
	sourceFiles.AddFile(config.sourceFile);

	ui->prj->Save();

	QDialog::accept();
}
