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
#include <FEBioLink/FEBioModule.h>

// Instructions for adding a new plugin template:
// =============================================
// 
// 1. Add header and source code to plugin_templates.h (also see instructions in this file)
// 2. Create a new CPluginTemplate-derived class. 
//   2.1. Create the new class
//   2.2. Pass arguments to base-class constructor
//   2.3. Add properties in constructor
//   2.4. (Optionally) Implement GetOptions() function, which will by used to fill the $(ARGx) fields. 
// 3. Add it to the pluginTemplates list. (Make sure to adjust PLUGIN_TEMPLATES)
//

// Base class for deriving plugin templates.
class CPluginTemplate : public CDataPropertyList
{
public:
	CPluginTemplate(const QString& pluginType, const QString& header, const QString& source)
	{
		m_pluginType = pluginType;
		m_header = header;
		m_source = source;
	}

	virtual QStringList GetOptions()
	{
		return QStringList();
	}

public:
	QString	m_pluginType;
	QString m_header;
	QString	m_source;
};

class CElasticMaterialProps : public CPluginTemplate
{
public:
	CElasticMaterialProps() : CPluginTemplate("Elastic material", szhdr_mat, szsrc_mat)
	{
		m_baseClass = 0;
		addEnumProperty(&m_baseClass, "Base class: ")->setEnumValues(QStringList() << "FEElasticMaterial" << "FEUncoupledMaterial");
	}

	QStringList GetOptions() override
	{
		QStringList l;
		switch (m_baseClass)
		{
		case 0: l << "FEElasticMaterial" << "Stress" << "Tangent"; break;
		case 1: l << "FEUncoupledMaterial" << "DevStress" << "DevTangent"; break;
		}
		return l;
	}

private:
	int	m_baseClass;
};

class CElemDataGeneratorProps : public CPluginTemplate
{
public:
	CElemDataGeneratorProps() : CPluginTemplate("Element data generator", szhdr_mdg, szsrc_mdg) {}
};

class CPlotNodeDataProps : public CPluginTemplate
{
public:
	CPlotNodeDataProps() : CPluginTemplate("Node plot data", szhdr_npd, szsrc_npd)
	{
		m_datatype = 0;
		addEnumProperty(&m_datatype, "Data type")->setEnumValues(QStringList() << "PLT_FLOAT" << "PLT_VEC3F");
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


class CPlotSurfaceDataProps : public CPluginTemplate
{
public:
	CPlotSurfaceDataProps() : CPluginTemplate("Surface plot data", szhdr_spd, szsrc_spd)
	{
		m_datatype = 0;
		m_datafmt = 1;
		addEnumProperty(&m_datatype, "Data type:")->setEnumValues(QStringList() << "PLT_FLOAT" << "PLT_VEC3F");
		addEnumProperty(&m_datafmt, "Data format:")->setEnumValues(QStringList() << "FMT_NODE" << "FMT_ITEM" << "FMT_MULT" << "FMT_REGION");
	}

	QStringList GetOptions() override
	{
		// fill $(ARG1) and $(ARG2)
		QStringList l;
		l << Property(0).values.at(m_datatype);
		l << Property(1).values.at(m_datafmt);

		// fill $(ARG3)
		switch (m_datafmt)
		{
		case 1: l << QString(szspd_snippet_item); break;
		default:
			l << QString("//TODO: Write surface data (or return false to skip this surface.)");
		}

		// fill $(ARG4)
		switch (m_datatype)
		{
		case 0: l << "double"; break;
		case 1: l << "vec3d"; break;
		}

		return l;
	}

private:
	int m_datatype;
	int m_datafmt;
};

class CPlotElemDataProps : public CPluginTemplate
{
public:
	CPlotElemDataProps() : CPluginTemplate("Element plot data", szhdr_epd, szsrc_epd)
	{
		m_datatype = 0;
		m_datafmt  = 1;
		addEnumProperty(&m_datatype, "Data type:"  )->setEnumValues(QStringList() << "PLT_FLOAT" << "PLT_VEC3F");
		addEnumProperty(&m_datafmt , "Data format:")->setEnumValues(QStringList() << "FMT_NODE" << "FMT_ITEM" << "FMT_MULT" << "FMT_REGION");
	}

	QStringList GetOptions() override
	{
		// fill $(ARG1) and $(ARG2)
		QStringList l;
		l << Property(0).values.at(m_datatype);
		l << Property(1).values.at(m_datafmt);

		// fill $(ARG3)
		switch (m_datafmt)
		{
		case 1: l << QString(szepd_snippet_item); break;
		case 3: l << QString(szepd_snippet_region); break;
		default:
			l << QString("");
		}
		
		// fill $(ARG4)
		switch (m_datatype)
		{
		case 0: l << "double"; break;
		case 1: l << "vec3d"; break;
		}
		return l;
	}

private:
	int m_datatype;
	int m_datafmt;
};

class CSurfaceLoadProps : public CPluginTemplate
{
public:
	CSurfaceLoadProps() : CPluginTemplate("Surface load", szhdr_sl, szsrc_sl) {}
};

//=============================================================================
const int PLUGIN_TEMPLATES = 6;
CPluginTemplate* pluginTemplates[PLUGIN_TEMPLATES] = {
	new CElasticMaterialProps(),
	new CElemDataGeneratorProps(),
	new CPlotNodeDataProps(),
	new CPlotSurfaceDataProps(),
	new CPlotElemDataProps(),
	new CSurfaceLoadProps()
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

			QStringList pluginTemplateNames;
			for (int i = 0; i < PLUGIN_TEMPLATES; ++i)
			{
				pluginTemplateNames << pluginTemplates[i]->m_pluginType;
			}
			m_type->addItems(pluginTemplateNames);

			QStringList modList;
			std::vector<FEBio::FEBioModule> modules = FEBio::GetAllModules();
			for (auto& mod : modules)
			{
				modList.append(mod.m_szname);
			}
			m_mod->addItems(modList);

			m_path->setResourceType(CResourceEdit::FOLDER_RESOURCE);

			setLayout(f);

			registerField("plugin.name*", m_name); // asterisk denotes this is a required property
			registerField("plugin.type", m_type);
		}
	};

	class COptionsPage : public QWizardPage
	{
	public:
		QStackedWidget* stack;
		CPropertyListForm* props;

		CPluginTemplate* pl = nullptr;

	public:
		COptionsPage()
		{
			setTitle("Set options");

			stack = new QStackedWidget;
			QLabel* label = new QLabel("(No properties)");
			label->setAlignment(Qt::AlignTop);
			stack->addWidget(label);

			props = new CPropertyListForm;
			stack->addWidget(props);

			QVBoxLayout* l = new QVBoxLayout;
			l->addWidget(stack);

			setLayout(l);
		}

		void initializePage() override
		{
			int n = field("plugin.type").toInt();
			setSubTitle(QString("Set the properties for the <b>%1</b> plugin.").arg(pluginTemplates[n]->m_pluginType));

			if (pl) props->setPropertyList(nullptr);
			pl = pluginTemplates[n];
			if (pl && pl->Properties())
			{
				props->setPropertyList(pl);
				stack->setCurrentIndex(1);
			}
			else stack->setCurrentIndex(0);
		}

		QStringList GetOptions()
		{
			QStringList l;
			if (pl) l = pl->GetOptions();
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
	// create the header file
	QString header = config.path + "\\" + config.name + ".h";
	QString headerText(config.header);
	headerText = headerText.replace("$(PLUGIN_NAME)", config.name);
	int n = 1;
	for (QString arg : config.args)
	{
		QString tmp = QString("$(ARG%1)").arg(n++);
		headerText = headerText.replace(tmp, arg);
	}
	if (!GenerateFile(header, headerText)) return false;

	// create the source file
	QString source = config.path + "\\" + config.name + ".cpp";
	QString sourceText(config.source);
	sourceText = sourceText.replace("$(PLUGIN_NAME)", config.name);
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
	config.header = pluginTemplates[type]->m_header;
	config.source = pluginTemplates[type]->m_source;
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
