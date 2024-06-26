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
#pragma once
#include <QWizardPage>
#include <QListWidget>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QStackedWidget>
#include "ResourceEdit.h"
#include "PropertyList.h"
#include "PropertyListForm.h"

class CMainWindow;

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

	QString GetInfo() const { return m_info; }

protected:
	void SetInfo(const QString& info) { m_info = info; }

public:
	QString	m_pluginType;
	QString m_header;
	QString	m_source;
	QString	m_info;
};

class CMainPage : public QWizardPage
{
	Q_OBJECT

public:
	QListWidget* m_type; // type of plugin
	QLabel* m_desc;

private slots:
	void on_selection_changed(int n);

public:
	CMainPage();
};

class CConfigPage : public QWizardPage
{
public:
	QComboBox* m_mod;  // FEBio module
	QLineEdit* m_name; // name of plugin
	CResourceEdit* m_path; // path to plugin code
	QLineEdit* m_typeString; // name of plugin

public:
	CConfigPage();

	void initializePage() override;
};

class COptionsPage : public QWizardPage
{
public:
	QStackedWidget* stack;
	CPropertyListForm* props;

	CPluginTemplate* pl = nullptr;

public:
	COptionsPage();

	void initializePage() override;

	QStringList GetOptions();
};

class CDlgCreatePluginUI
{
public:
	CMainWindow* m_wnd;

	CMainPage* mainPage;
	CConfigPage* configPage;
	COptionsPage* opsPage;

public:
	void setup(QWizard* dlg)
	{
		dlg->addPage(mainPage = new CMainPage);
		dlg->addPage(configPage = new CConfigPage);
		dlg->addPage(opsPage = new COptionsPage);
	}

	CPluginTemplate* GetPluginTemplate();
};

struct PluginConfig
{
	QString name;
	QString path;
	QString febioModule;
	QString typeString;
	QStringList args;

	QString cmakeFile;
	QString mainFile;
	QString headerFile;
	QString sourceFile;

	QString headerTxt;
	QString sourceTxt;

	QString sdkInc;
	QString sdkLib;
};

bool GeneratePluginFiles(const PluginConfig& config);
