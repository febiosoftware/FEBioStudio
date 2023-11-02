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

//=============================================================================
class CDlgCreatePluginUI
{
public:
	QComboBox* m_type; // type of plugin
	QComboBox* m_mod;  // FEBio module
	QLineEdit* m_name; // name of plugin
	CResourceEdit* m_path; // path to plugin code
	CMainWindow* m_wnd;

public:
	void setup(QDialog* dlg)
	{
		QVBoxLayout* l = new QVBoxLayout;

		QFormLayout* f = new QFormLayout;
		f->setLabelAlignment(Qt::AlignRight);
		f->addRow("Plugin name:", m_name = new QLineEdit());
		f->addRow("FEBio module:", m_mod = new QComboBox());
		f->addRow("Plugin type:", m_type = new QComboBox());
		f->addRow("Path:", m_path = new CResourceEdit());

		m_type->addItems(QStringList() << "Elastic material");
		m_mod->addItems(QStringList() << "solid");
		m_path->setResourceType(CResourceEdit::FOLDER_RESOURCE);

		QPushButton* create = new QPushButton("Create");
		QPushButton* cancel = new QPushButton("Cancel");
		QHBoxLayout* h = new QHBoxLayout;
		h->addStretch();
		h->addWidget(create);
		h->addWidget(cancel);

		l->addLayout(f);
		l->addLayout(h);

		dlg->setLayout(l);

		QObject::connect(create, SIGNAL(clicked()), dlg, SLOT(accept()));
		QObject::connect(cancel, SIGNAL(clicked()), dlg, SLOT(reject()));
	}
};

CDlgCreatePlugin::CDlgCreatePlugin(CMainWindow* parent) : QDialog(parent), ui(new CDlgCreatePluginUI)
{
	setWindowTitle("Create FEBio Plugin");
	setMinimumSize(QSize(600, 300));
	ui->m_wnd = parent;
	ui->setup(this);
}

const char* szmain = \
"#include <FECore\\FECoreKernel.h>\n" \
"#include \"$(PLUGIN_NAME).h\"\n\n"\
"FECORE_EXPORT unsigned int GetSDKVersion()\n" \
"{\n" \
"	return FE_SDK_VERSION;\n" \
"}\n\n"\
"FECORE_EXPORT void PluginInitialize(FECoreKernel & febio)\n"\
"{\n"\
"	FECoreKernel::SetInstance(&febio);\n\n"\
"	febio.SetActiveModule(\"$(PLUGIN_MODULE)\");\n\n"
"	REGISTER_FECORE_CLASS($(PLUGIN_NAME), \"$(PLUGIN_NAME)\");\n"\
"}\n"
;

const char* szhdr = \
"#include <FEBioMech\\FEElasticMaterial.h>\n\n" \
"class $(PLUGIN_NAME) : public FEElasticMaterial\n" \
"{\n" \
"public:\n" \
"	// class constructor\n"
"	$(PLUGIN_NAME)(FEModel* fem);\n\n"
"	// evaluate Cauchy stress\n"
"	mat3ds Stress(FEMaterialPoint& mp) override;\n\n" \
"	// evaluate spatial elasticity tangent\n"
"	tens4ds Tangent(FEMaterialPoint& mp) override;\n\n"
"private:\n"
"	// TODO: Add member variables here\n\n"\
"	DECLARE_FECORE_CLASS();\n"
"};\n";

const char* szsrc = \
"#include \"$(PLUGIN_NAME).h\"\n\n" \
"BEGIN_FECORE_CLASS($(PLUGIN_NAME), FEElasticMaterial)\n"\
"	// TODO: Add parameters\n"\
"END_FECORE_CLASS();\n\n"\
"$(PLUGIN_NAME)::$(PLUGIN_NAME)(FEModel* fem) : FEElasticMaterial(fem)\n"\
"{\n" \
"	// TODO: initialize all class member variables\n" \
"}\n\n" \
"mat3ds $(PLUGIN_NAME)::Stress(FEMaterialPoint& mp)\n" \
"{\n" \
"	// TODO: implement stress\n" \
"	mat3ds s;\n" \
"	return s;\n" \
"}\n\n" \
"tens4ds $(PLUGIN_NAME)::Tangent(FEMaterialPoint& mp)\n" \
"{\n" \
"	// TODO: implement tangent\n" \
"	tens4ds c;\n" \
"	return c;\n" \
"}\n";

const char* szcmake = \
"cmake_minimum_required(VERSION 3.5.0)\n\n" \
"set(CMAKE_CXX_STANDARD 11)\n" \
"set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n" \
"project($(PLUGIN_NAME))\n\n"\
"add_definitions(-DWIN32 -DFECORE_DLL /wd4251)\n\n"\
"include_directories(\"$(PLUGIN_SDK_INCLUDE)\")\n\n"\
"link_directories(\"$(PLUGIN_SDK_LIBS)/$<CONFIG>\")\n\n"\
"add_library($(PLUGIN_NAME) SHARED $(PLUGIN_NAME).h $(PLUGIN_NAME).cpp main.cpp)\n\n"\
"target_link_libraries($(PLUGIN_NAME) fecore.lib febiomech.lib)\n\n"\
"set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT $(PLUGIN_NAME))\n\n"\
"";

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
	QString headerText = QString(szhdr).replace("$(PLUGIN_NAME)", config.name);
	if (!GenerateFile(header, headerText)) return false;

	// create the source file
	QString source = config.path + "\\" + config.name + ".cpp";
	QString sourceText = QString(szsrc).replace("$(PLUGIN_NAME)", config.name);
	if (!GenerateFile(source, sourceText)) return false;

	// create the main file
	QString main = config.path + "\\main.cpp";
	QString mainText = QString(szmain).replace("$(PLUGIN_NAME)", config.name);
	mainText = mainText.replace("$(PLUGIN_MODULE)", config.module);
	if (!GenerateFile(main, mainText)) return false;

	// get the SDK paths
	QString SDKInclude = ui->m_wnd->GetSDKIncludePath();
	QString SDKLibs    = ui->m_wnd->GetSDKLibraryPath();

	// create the CMake file
	QString cmake = config.path + "\\CMakeLists.txt";
	QString cmakeText = QString(szcmake).replace("$(PLUGIN_NAME)", config.name);
	cmakeText = cmakeText.replace("$(PLUGIN_SDK_INCLUDE)", SDKInclude);
	cmakeText = cmakeText.replace("$(PLUGIN_SDK_LIBS)", SDKLibs);
	if (!GenerateFile(cmake, cmakeText)) return false;

	return true;
}

void CDlgCreatePlugin::accept()
{
	QString name = ui->m_name->text();
	QString path = ui->m_path->resourceName();
	QString mod  = ui->m_mod->currentText();
	int type = ui->m_type->currentIndex();

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
		dir.cd(name);
		path = QDir::toNativeSeparators(dir.absolutePath());
	}

	// generate the plugin files
	PluginConfig config;
	config.name   = name;
	config.module = mod;
	config.path   = path;
	if (GeneratePlugin(config) == false)
	{
		QMessageBox::critical(this, "FEBio Studio", "Failed to create plugin.");
		return;
	}

	// all is well
	QMessageBox::information(this, "FEBio Studio", QString("The plugin was created successfully in\n%1").arg(path));

	QDialog::accept();
}
