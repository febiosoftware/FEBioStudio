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
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QTreeWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>
#include <QComboBox>
#include <QMessageBox>
#include <QSplitter>
#include <FEBioLib/plugin.h>
#include <FEBioLib/febio.h>
#include <FEBioLink/FEBioClass.h>
#include <FECore/FEModule.h>
#include <QMenu>
#include "MainWindow.h"
#include "ResourceEdit.h"
#include "plugin_templates.h"
#include "FEBioStudioProject.h"
#include "PropertyListForm.h"
#include <QStackedWidget>
#include <QLabel>
#include "PropertyList.h"

class CPluginProps : public CPropertyList
{
public:
	CPluginProps() {}

	virtual QStringList GetOptions() = 0;
};

class CPlotNodeDataProps : public CPluginProps
{
public:
	CPlotNodeDataProps()
	{
		m_datatype = 0;
		addProperty("Data type:", CProperty::Enum)->setEnumValues(QStringList() << "PLT_FLOAT" << "PLT_VEC3F");
	}

	QVariant GetPropertyValue(int i)
	{
		switch (i)
		{
		case 0: return m_datatype;
		}
		return QVariant();
	}

	void SetPropertyValue(int i, const QVariant& v)
	{
		switch (i)
		{
		case 0: m_datatype = v.toInt(); break;
		}
	}

	QStringList GetOptions() override
	{
		QStringList l;
		switch (m_datatype)
		{
		case 0: l << "PLT_FLOAT" << "double"; break;
		case 1: l << "PLT_VEC3F" << "vec3d"; break;
		}
		return l;
	}

private:
	int	m_datatype;
};

//=============================================================================
class CDlgCreatePluginUI
{
public:
	class CMainPage : public QWizardPage
	{
	public:
		QComboBox* m_type; // type of plugin
		QComboBox* m_mod;  // FEBio module
		QLineEdit* m_name; // name of plugin
		CResourceEdit* m_path; // path to plugin code
		QLineEdit* m_typeString; // name of plugin

	public:
		CMainPage()
		{
			setTitle("Create FEBio plugin");

			QFormLayout* f = new QFormLayout;
			f->setLabelAlignment(Qt::AlignRight);
			f->addRow("Plugin name:", m_name = new QLineEdit());
			f->addRow("FEBio module:", m_mod = new QComboBox());
			f->addRow("Plugin type:", m_type = new QComboBox());
			f->addRow("Path:", m_path = new CResourceEdit());
			f->addRow("Type string:", m_typeString = new QLineEdit());
			m_typeString->setPlaceholderText("(leave blank for default)");

			m_type->addItems(QStringList() << "Elastic material" << "Uncoupled material" << "Element data generator" << "Node plot data");
			m_mod->addItems(QStringList() << "solid");
			m_path->setResourceType(CResourceEdit::FOLDER_RESOURCE);

			setLayout(f);

			registerField("plugintype", m_type);
		}
	};

	class COptionsPage : public QWizardPage
	{
	public:
		QStackedWidget* stack;
		CPluginProps* pl;

	public:
		COptionsPage()
		{
			setTitle("Set options");

			stack = new QStackedWidget;
			stack->addWidget(new QLabel("(No properties)"));

			CPropertyListForm* props = new CPropertyListForm;
			stack->addWidget(props);

			pl = new CPlotNodeDataProps;
			props->setPropertyList(pl);

			QVBoxLayout* l = new QVBoxLayout;
			l->addWidget(stack);

			setLayout(l);
		}

		void initializePage() override
		{
			int n = field("plugintype").toInt();
			if (n != 3) stack->setCurrentIndex(0);
			else stack->setCurrentIndex(1);
		}

		QStringList GetOptions()
		{
			QStringList l;
			int n = field("plugintype").toInt();
			if (n == 3) l = pl->GetOptions();
			return l;
		}
	};

public:
	CMainWindow* m_wnd;

	CMainPage* mainPage;
	COptionsPage* opsPage;

public:
	void setup(QWizard* dlg)
	{
		dlg->addPage(mainPage = new CMainPage);
		dlg->addPage(opsPage = new COptionsPage);
	}
};

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
	}
	else
	{
		createPath = parent->GetCreatePluginPath();
	}
	ui->mainPage->m_path->setResourceName(createPath);
}

bool GenerateFile(const QString& fileName, const QString& content)
{
	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
	QTextStream out(&file);
	out << content;
	file.close();
	return true;
}

bool CDlgCreatePlugin::GeneratePlugin(const PluginConfig& config)
{
	const char* szhdr = nullptr;
	const char* szsrc = nullptr;
	switch (config.type)
	{
	case PluginConfig::ELASTICMATERIAL_PLUGIN  : szhdr = szhdr_mat; szsrc = szsrc_mat; break;
	case PluginConfig::UNCOUPLEDMATERIAL_PLUGIN: szhdr = szhdr_ucm; szsrc = szsrc_ucm; break;
	case PluginConfig::ELEMDATAGENERATOR_PLUGIN: szhdr = szhdr_mdg; szsrc = szsrc_mdg; break; 
	case PluginConfig::NODEPLOTDATA_PLUGIN     : szhdr = szhdr_npd; szsrc = szsrc_npd; break; 
	default:
		return false;
		break;
	}

	// create the header file
	QString header = config.path + "\\" + config.name + ".h";
	QString headerText = QString(szhdr).replace("$(PLUGIN_NAME)", config.name);
	int n = 1;
	for (QString arg : config.args)
	{
		QString tmp = QString("$(ARG%1)").arg(n++);
		headerText = headerText.replace(tmp, arg);
	}
	if (!GenerateFile(header, headerText)) return false;

	// create the source file
	QString source = config.path + "\\" + config.name + ".cpp";
	QString sourceText = QString(szsrc).replace("$(PLUGIN_NAME)", config.name);
	n = 1;
	for (QString arg : config.args)
	{
		QString tmp = QString("$(ARG%1)").arg(n++);
		sourceText = sourceText.replace(tmp, arg);
	}
	if (!GenerateFile(source, sourceText)) return false;

	// create the main file
	QString main = config.path + "\\main.cpp";
	QString mainText = QString(szmain).replace("$(PLUGIN_NAME)", config.name);
	mainText = mainText.replace("$(PLUGIN_MODULE)", config.module);
	mainText = mainText.replace("$(PLUGIN_TYPESTRING)", config.typeString);
	if (!GenerateFile(main, mainText)) return false;

	// get the SDK paths
	QString SDKInc = ui->m_wnd->GetSDKIncludePath();
	QString SDKLib = ui->m_wnd->GetSDKLibraryPath();
	SDKInc.replace("\\", "/");
	SDKLib.replace("\\", "/");
	if (SDKLib.last(1) != "/") SDKLib += "/";
	SDKLib += "$<CONFIG>";

	// create the CMake file
	QString cmake = config.path + "\\CMakeLists.txt";
	QString cmakeText = QString(szcmake).replace("$(PLUGIN_NAME)", config.name);
	cmakeText = cmakeText.replace("$(PLUGIN_SDK_INCLUDE)", SDKInc);
	cmakeText = cmakeText.replace("$(PLUGIN_SDK_LIBS)", SDKLib);
	if (!GenerateFile(cmake, cmakeText)) return false;

	// add plugin to project
	FEBioStudioProject* prj = ui->m_wnd->GetProject();
	if (prj)
	{
		FEBioStudioProject::ProjectItem* grp = prj->AddPlugin(config.name);
		if (grp)
		{
			grp->AddFile(cmake);

			FEBioStudioProject::ProjectItem& headerFiles = grp->AddGroup("Include");
			headerFiles.AddFile(header);

			FEBioStudioProject::ProjectItem& sourceFiles = grp->AddGroup("Source");
			sourceFiles.AddFile(main);
			sourceFiles.AddFile(source);
		}
	}

	return true;
}

void CDlgCreatePlugin::accept()
{
	QString name = ui->mainPage->m_name->text();
	QString path = ui->mainPage->m_path->resourceName();
	QString mod  = ui->mainPage->m_mod->currentText();
	QString typeStr = ui->mainPage->m_typeString->text();
	if (typeStr.isEmpty()) typeStr = name;
	int type = ui->mainPage->m_type->currentIndex();

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

	// create the folder for the plugin
	QDir dir(path);
	if (dir.mkdir(name) == false)
	{
		QMessageBox::critical(this, "FEBio Studio", "The folder already exists.");
		return;
	}
	else
	{
		// save this as the default path
		ui->m_wnd->SetCreatePluginPath(QDir::toNativeSeparators(dir.absolutePath()));
		dir.cd(name);
		path = QDir::toNativeSeparators(dir.absolutePath());
	}

	// generate the plugin files
	PluginConfig config;
	config.name   = name;
	config.module = mod;
	config.path   = path;
	config.typeString = typeStr;
	switch (type)
	{
	case 0: config.type = PluginConfig::PluginType::ELASTICMATERIAL_PLUGIN; break;
	case 1: config.type = PluginConfig::PluginType::UNCOUPLEDMATERIAL_PLUGIN; break;
	case 2: config.type = PluginConfig::PluginType::ELEMDATAGENERATOR_PLUGIN; break;
	case 3: config.type = PluginConfig::PluginType::NODEPLOTDATA_PLUGIN; break;
	default:
		QMessageBox::critical(this, "FEBio Studio", "Don't know how to build this type of plugin.");
		return;
	}

	config.args = ui->opsPage->GetOptions();

	if (GeneratePlugin(config) == false)
	{
		QMessageBox::critical(this, "FEBio Studio", "Failed to create plugin.");
		return;
	}

	// automatically save the project file
	FEBioStudioProject* prj = ui->m_wnd->GetProject();
	if (prj && (prj->GetProjectPath().isEmpty()))
	{
		QString prjPath = path + "\\" + name + ".fsp";
		prj->Save(prjPath);
	}

	// all is well
	QMessageBox::information(this, "FEBio Studio", QString("The plugin was created successfully in\n%1").arg(path));

	QDialog::accept();
}
