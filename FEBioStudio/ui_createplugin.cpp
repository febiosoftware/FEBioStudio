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
#include "ui_createplugin.h"
#include <QBoxLayout>
#include <QFormLayout>
#include <QtCore/QFile>
#include <FEBioLink/FEBioModule.h>
#include "plugin_templates.h"
#include "version.h"

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

class CElasticMaterialProps : public CPluginTemplate
{
public:
	CElasticMaterialProps() : CPluginTemplate("Elastic material", szhdr_mat, szsrc_mat)
	{
		m_baseClass = 0;
		addEnumProperty(&m_baseClass, "Base class: ")->setEnumValues(QStringList() << "FEElasticMaterial" << "FEUncoupledMaterial");

		SetInfo("Create a plugin that implements a constitutive formulation for an elastic material.");
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
	CElemDataGeneratorProps() : CPluginTemplate("Element data generator", szhdr_mdg, szsrc_mdg) 
	{
		SetInfo("Create a plugin that implements an algorithm for generating element data maps.");
	}
};

class CPlotNodeDataProps : public CPluginTemplate
{
public:
	CPlotNodeDataProps() : CPluginTemplate("Plot node data", szhdr_npd, szsrc_npd)
	{
		m_datatype = 0;
		addEnumProperty(&m_datatype, "Data type")->setEnumValues(QStringList() << "PLT_FLOAT" << "PLT_VEC3F");

		SetInfo("Implement a new node data plot variable that can be written to the FEBio plot file.");
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
	CPlotSurfaceDataProps() : CPluginTemplate("Plot surface data", szhdr_spd, szsrc_spd)
	{
		m_datatype = 0;
		m_datafmt = 1;
		addEnumProperty(&m_datatype, "Data type:")->setEnumValues(QStringList() << "PLT_FLOAT" << "PLT_VEC3F");
		addEnumProperty(&m_datafmt, "Data format:")->setEnumValues(QStringList() << "FMT_NODE" << "FMT_ITEM" << "FMT_MULT" << "FMT_REGION");

		SetInfo("Implement a new surface data plot variable that can be written to the FEBio plot file.");
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
	CPlotElemDataProps() : CPluginTemplate("Plot element data", szhdr_epd, szsrc_epd)
	{
		m_datatype = 0;
		m_datafmt = 1;
		addEnumProperty(&m_datatype, "Data type:")->setEnumValues(QStringList() << "PLT_FLOAT" << "PLT_VEC3F");
		addEnumProperty(&m_datafmt, "Data format:")->setEnumValues(QStringList() << "FMT_NODE" << "FMT_ITEM" << "FMT_MULT" << "FMT_REGION");

		SetInfo("Implement a new element data plot variable that can be written to the FEBio plot file.");
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
	CSurfaceLoadProps() : CPluginTemplate("Surface load", szhdr_sl, szsrc_sl) 
	{
		SetInfo("Create a plugin that implements a new surface load.");
	}
};

class CLogNodeDataProps : public CPluginTemplate
{
public:
	CLogNodeDataProps() : CPluginTemplate("Log node data", szhdr_nld, szsrc_nld)
	{
		SetInfo("Implement a new node data log variable that can be written to the FEBio log file.");
	}
};

class CLogElemDataProps : public CPluginTemplate
{
public:
	CLogElemDataProps() : CPluginTemplate("Log element data", szhdr_eld, szsrc_eld)
	{
		SetInfo("Implement a new element data log variable that can be written to the FEBio log file.");
	}
};

class CCallbackProps : public CPluginTemplate
{
public:
	CCallbackProps() : CPluginTemplate("Callback", szhdr_cb, szsrc_cb)
	{
		SetInfo("Create a callback plugin that can be used to interface with FEBio's solution pipeline.");
	}
};

class CTaskProps : public CPluginTemplate
{
public:
	CTaskProps() : CPluginTemplate("Task", szhdr_task, szsrc_task)
	{
		SetInfo("Take complete control of FEBio by implementing a new task plugin.");
	}
};

//=============================================================================
// Try to keep this in alphabetical order
const int PLUGIN_TEMPLATES = 10;
CPluginTemplate* pluginTemplates[PLUGIN_TEMPLATES] = {
	new CCallbackProps(),
	new CElasticMaterialProps(),
	new CElemDataGeneratorProps(),
	new CLogElemDataProps(),
	new CLogNodeDataProps(),
	new CPlotElemDataProps(),
	new CPlotNodeDataProps(),
	new CPlotSurfaceDataProps(),
	new CSurfaceLoadProps(),
	new CTaskProps()
};

CMainPage::CMainPage()
{
	setTitle("Create FEBio plugin");
	setSubTitle("This wizard will take you through the process of creating an FEBio plugin.");

	m_type = new QListWidget();

	QStringList pluginTemplateNames;
	for (int i = 0; i < PLUGIN_TEMPLATES; ++i)
	{
		pluginTemplateNames << pluginTemplates[i]->m_pluginType;
	}
	m_type->addItems(pluginTemplateNames);

	QVBoxLayout* l1 = new QVBoxLayout;
	l1->addWidget(new QLabel("Select a plugin type:"));
	l1->addWidget(m_type);

	QHBoxLayout* h = new QHBoxLayout;
	h->addLayout(l1, 2);
	h->addWidget(m_desc = new QLabel(), 1);
	m_desc->setWordWrap(true);
	m_desc->setAlignment(Qt::AlignTop);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addLayout(h);
	setLayout(mainLayout);

	registerField("plugin.type", m_type);

	QObject::connect(m_type, &QListWidget::currentRowChanged, this, &CMainPage::on_selection_changed);
}

void CMainPage::on_selection_changed(int n)
{
	QString info;
	if (n >= 0)
	{
		info = pluginTemplates[n]->GetInfo();
	}
	else info = QString("(select an option)");

	m_desc->setText(QString("<h2>Description</h2>%1").arg(info));
}

CConfigPage::CConfigPage()
{
	setTitle("Configure plugin");

	QFormLayout* f = new QFormLayout;
	f->setLabelAlignment(Qt::AlignRight);
	f->addRow("Plugin name:", m_name = new QLineEdit());
	f->addRow("FEBio module:", m_mod = new QComboBox());
	f->addRow("Path:", m_path = new CResourceEdit());
	f->addRow("Type string:", m_typeString = new QLineEdit());
	m_typeString->setPlaceholderText("(leave blank for default)");

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
}

void CConfigPage::initializePage()
{
	int n = field("plugin.type").toInt();
	setSubTitle(QString("Configure <b>%1</b> plugin.").arg(pluginTemplates[n]->m_pluginType));
}

COptionsPage::COptionsPage()
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

void COptionsPage::initializePage()
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

QStringList COptionsPage::GetOptions()
{
	QStringList l;
	if (pl) l = pl->GetOptions();
	return l;
}

CPluginTemplate* CDlgCreatePluginUI::GetPluginTemplate()
{
	int type = mainPage->m_type->currentRow();
	return pluginTemplates[type];
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

bool GeneratePluginFiles(const PluginConfig& config)
{
	QString comment = QString("Generated by FEBio Studio %1.%2.%3").arg(FBS_VERSION).arg(FBS_SUBVERSION).arg(FBS_SUBSUBVERSION);
	QString cppcomment = QString("/*%1*/\n").arg(comment);
	QString cmakecomment = QString("# %1\n").arg(comment);

	// create the header file
	QString headerText(cppcomment + config.headerTxt);
	headerText = headerText.replace("$(PLUGIN_NAME)", config.name);
	int n = 1;
	for (QString arg : config.args)
	{
		QString tmp = QString("$(ARG%1)").arg(n++);
		headerText = headerText.replace(tmp, arg);
	}
	if (!GenerateFile(config.headerFile, headerText)) return false;

	// create the source file
	QString sourceText(cppcomment + config.sourceTxt);
	sourceText = sourceText.replace("$(PLUGIN_NAME)", config.name);
	n = 1;
	for (QString arg : config.args)
	{
		QString tmp = QString("$(ARG%1)").arg(n++);
		sourceText = sourceText.replace(tmp, arg);
	}
	if (!GenerateFile(config.sourceFile, sourceText)) return false;

	// create the main file
	QString mainText = cppcomment + QString(szmain);
	mainText = mainText.replace("$(PLUGIN_NAME)", config.name);
	mainText = mainText.replace("$(PLUGIN_MODULE)", config.febioModule);
	mainText = mainText.replace("$(PLUGIN_TYPESTRING)", config.typeString);
	if (!GenerateFile(config.mainFile, mainText)) return false;

	// get the SDK paths
	QString SDKInc = config.sdkInc;
	QString SDKLib = config.sdkLib;
	SDKInc.replace("\\", "/");
	SDKLib.replace("\\", "/");
	if (SDKLib.last(1) != "/") SDKLib += "/";
	SDKLib += "$<CONFIG>";

	// create the CMake file
	QString cmakeText = cmakecomment + QString(szcmake);
	cmakeText = cmakeText.replace("$(PLUGIN_NAME)", config.name);
	cmakeText = cmakeText.replace("$(PLUGIN_SDK_INCLUDE)", SDKInc);
	cmakeText = cmakeText.replace("$(PLUGIN_SDK_LIBS)", SDKLib);
	if (!GenerateFile(config.cmakeFile, cmakeText)) return false;

	return true;
}
