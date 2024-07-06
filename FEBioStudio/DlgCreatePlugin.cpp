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

//=============================================================================
CDlgCreatePlugin::CDlgCreatePlugin(CMainWindow* parent) : QWizard(parent), ui(new CDlgCreatePluginUI)
{
	setWindowTitle("Create FEBio Plugin");
	setMinimumSize(QSize(600, 300));
	ui->m_wnd = parent;

	if (parent->currentTheme() == 1)
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
		ui->configPage->m_path->setResourceName(createPath);
		ui->configPage->m_path->setEnabled(false);
	}
	else
	{
		createPath = parent->GetCreatePluginPath();
		ui->configPage->m_path->setResourceName(createPath);
	}
}

void CDlgCreatePlugin::accept()
{
	// collect input fields
	QString name = ui->configPage->m_name->text();
	QString path = ui->configPage->m_path->resourceName();
	QString mod  = ui->configPage->m_mod->currentText();
	QString typeStr = ui->configPage->m_typeString->text();
	if (typeStr.isEmpty()) typeStr = name;

	// check input
	if (name.isEmpty())
	{
		QMessageBox::critical(this, "FEBio Studio", "Please choose a name for the plugin.");
		return;
	}

	if (path.isEmpty())
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
	QDir dir(path);
	if (!prj->GetProjectPath().isEmpty())
	{
		assert(path == prj->GetProjectPath());
		if (dir.mkdir(name) == false)
		{
			QMessageBox::critical(this, "FEBio Studio", "The folder already exists.");
			return;
		}
		dir.cd(name);
	}
	else
	{
		// create a folder to save the project in
		if (dir.mkdir(name) == false)
		{
			QMessageBox::critical(this, "FEBio Studio", "The folder already exists.");
			return;
		}

		// save this as the default path
		ui->m_wnd->SetCreatePluginPath(QDir::toNativeSeparators(dir.absolutePath()));

		// automatically save the project file
		dir.cd(name);
		path = QDir::toNativeSeparators(dir.absolutePath());
		QString prjPath = path + "\\" + name + ".fsp";
		prj->Save(prjPath);

		// then, add a folder for the plugin
		if (dir.mkdir(name) == false)
		{
			QMessageBox::critical(this, "FEBio Studio", "The folder already exists.");
			return;
		}
		dir.cd(name);
	}
	path = QDir::toNativeSeparators(dir.absolutePath());


	CPluginTemplate* pluginTemplate = ui->GetPluginTemplate();
	if (pluginTemplate == nullptr)
	{
		QMessageBox::critical(this, "FEBio Studio", "No valid plugin template found.");
		return;
	}

	// generate the plugin meta data
	PluginConfig config;
	config.name        = name;
	config.febioModule = mod;
	config.path        = path;
	config.typeString  = typeStr;
	config.headerTxt   = pluginTemplate->m_header;
	config.sourceTxt   = pluginTemplate->m_source;
	config.args        = ui->opsPage->GetOptions();
	config.sdkInc      = ui->m_wnd->GetSDKIncludePath();
	config.sdkLib      = ui->m_wnd->GetSDKLibraryPath();
	config.headerFile  = config.path + "\\" + config.name + ".h";
	config.sourceFile  = config.path + "\\" + config.name + ".cpp";
	config.mainFile    = config.path + "\\main.cpp";
	config.cmakeFile   = config.path + "\\CMakeLists.txt";

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
	QMessageBox::information(this, "FEBio Studio", QString("The plugin was created successfully in\n%1").arg(path));

	QDialog::accept();
}
