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
#include "PythonToolsPanel.h"
#include "ui_pythontoolspanel.h"
#include <PyLib/PythonTool.h>
#include <QFileDialog>
#include <FEBioStudio/MainWindow.h>
#include <FEBioStudio/Document.h>
#include <PyLib/PythonThread.h>
#include <FEBioStudio/LogPanel.h>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

CPythonToolsPanel::CPythonToolsPanel(CMainWindow* wnd, QWidget* parent) 
	: CWindowPanel(wnd, parent), ui(new Ui::CPythonToolsPanel)
{
	ui->setupUi(this);
	ui->m_pythonThread = nullptr;
}

void CPythonToolsPanel::startThread()
{
	CPythonTool* tool = ui->m_activeTool;
	if (tool == nullptr) return;

	if (ui->m_pythonThread == nullptr)
	{
		CGLDocument* doc = GetDocument();
		CCachedPropertyList* props = tool->GetProperties();
		ui->m_pythonThread = new CPyThread(doc, props);
		connect(ui->m_pythonThread, &CPyThread::threadFinished, this, &CPythonToolsPanel::on_pythonThread_threadFinished);
		ui->m_pythonThread->runFile(tool->GetFilePath());
	}
}

CPythonToolsPanel::~CPythonToolsPanel()
{
	if (ui->m_pythonThread)
	{
		// TODO: user is trying to quit FEBio Studio with a tool still running.
		//       Should we do anything? 
	}
}

void CPythonToolsPanel::Update(bool breset)
{
	if (ui->m_activeTool)
	{
		ui->m_activeTool->CAbstractTool::Update();
	}
}

void CPythonToolsPanel::setProgressText(const QString& txt)
{
	ui->runPane->setProgressText(txt);
}

void CPythonToolsPanel::setProgress(int prog)
{
	ui->runPane->setProgress(prog);
}

bool parsePythonFile(const QString& filename, QJsonObject& js)
{
	QFile f(filename);
	if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

	QString jsonString;
	QTextStream ts(&f);
	bool foundStart = false;
	while (!ts.atEnd()) {
		QString line = ts.readLine();
		if (line[0] == '#')
		{
			if (!foundStart)
			{
				int n = line.indexOf("@fbs");
				if (n >= 0)
				{
					foundStart = true;
				}
				jsonString += line.sliced(n + 4);
			}
			else
			{
				jsonString += line.sliced(1);
			}
		}
		else break;
	}
	f.close();

	QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
	js = doc.object();
	return true;
}

vec3d jsonToVec3d(QJsonArray& a)
{
	double d[3] = { 0,0,0 };
	if (a.size() == 3)
	{
		for (int i = 0; i < 3; ++i)
		{
			QJsonValue ai = a[i];
			if (ai.isDouble()) d[i] = ai.toDouble();
		}
	}
	return vec3d(d[0], d[1], d[2]);
}

QColor jsonToQColor(QJsonArray& a)
{
	double d[3] = { 0,0,0 };
	if (a.size() == 3)
	{
		for (int i = 0; i < 3; ++i)
		{
			QJsonValue ai = a[i];
			if (ai.isDouble()) d[i] = ai.toDouble();
		}
	}

	return QColor::fromRgb((int)d[0], (int)d[1], (int)d[2]);
}

QStringList jsonToStringList(QJsonArray& a)
{
	QStringList l;
	for (int i = 0; i < a.size(); ++i)
	{
		QJsonValue ai = a[i];
		if (ai.isString()) l.push_back(ai.toString());
	}
	return l;
}

void BuildPropertyList(QJsonObject& o, CCachedPropertyList& props)
{
	QStringList keys = o.keys();
	for (QString key : keys)
	{
		QJsonValue v = o[key];
		if (v.isDouble())
		{
			double d = v.toDouble();
			props.addDoubleProperty(d, key);
		}
		else if (v.isString())
		{
			QString s = v.toString();
			props.addStringProperty(s, key);
		}
		else if (v.isArray())
		{
			QJsonArray a = v.toArray();
			props.addVec3Property(jsonToVec3d(a), key);
		}
		else if (v.isObject())
		{
			QJsonObject a = v.toObject();
			int propType = -1;
			QJsonValue jtype = a["type"];
			if (jtype.isString())
			{
				QString stype = jtype.toString();
				if      (stype == "int"   ) propType = CProperty::Int;
				else if (stype == "float" ) propType = CProperty::Float;
				else if (stype == "vec3"  ) propType = CProperty::Vec3;
				else if (stype == "url"   ) propType = CProperty::Resource;
				else if (stype == "string") propType = CProperty::String;
				else if (stype == "enum"  ) propType = CProperty::Enum;
				else if (stype == "bool"  ) propType = CProperty::Bool;
				else if (stype == "color" ) propType = CProperty::Color;
			}
			QJsonValue jval = a["value"];
			switch (propType)
			{
			case CProperty::Int     : if (jval.isDouble()) { props.addIntProperty((int)jval.toDouble(), key); } break;
			case CProperty::Bool    : if (jval.isDouble()) { props.addBoolProperty((bool)jval.toDouble(), key); } break;
			case CProperty::Float   : if (jval.isDouble()) { props.addDoubleProperty(jval.toDouble(), key); } break;
			case CProperty::String  : if (jval.isString()) { props.addStringProperty(jval.toString(), key); } break;
			case CProperty::Resource: if (jval.isString()) { props.addResourceProperty(jval.toString(), key); } break;
			case CProperty::Enum    : if (jval.isArray()) {
				QJsonArray a = jval.toArray();
				props.addEnumProperty(0, key)->setEnumValues(jsonToStringList(a));
			} break;
			case CProperty::Vec3: {
				if (jval.isArray()) { 
					QJsonArray a = jval.toArray();
					props.addVec3Property(jsonToVec3d(a), key);
				}
			} break;
			case CProperty::Color:
			{
				if (jval.isArray()) {
					QJsonArray a = jval.toArray();
					props.addColorProperty(jsonToQColor(a), key);
				}
			}
			break;
			}
		}
	}
}

void CPythonToolsPanel::on_importScript_triggered()
{
	QString filePath = QFileDialog::getOpenFileName(this, "Python Script", "", "Python scripts (*.py)");
	if (filePath.isEmpty()) return;

	LoadToolFromFile(filePath);
}

void CPythonToolsPanel::LoadToolFromFile(const QString& filePath)
{
	QFileInfo fi(filePath);
	QString toolName = fi.fileName();
	int n = toolName.indexOf(".");
	if (n >= 0) toolName.truncate(n);

	// parse the file for the preamble
	QJsonObject o;
	if (parsePythonFile(filePath, o) == false)
	{
		QMessageBox::critical(this, "Python", "Failed to import python file.");
		return;
	}

	// construct the tool properties
	CCachedPropertyList* props = new CCachedPropertyList();
	QString toolInfo;
	QJsonValue jname = o["name"]; if (jname.isString()) toolName = jname.toString();
	QJsonValue jinfo = o["info"]; if (jinfo.isString()) toolInfo = jinfo.toString();
	QJsonValue args = o["args"];
	if (args.isObject())
	{
		QJsonObject po = args.toObject();
		BuildPropertyList(po, *props);
	}

	// see if the tool already exists
	CPythonTool* tool = ui->findTool(filePath);
	if (tool)
	{
		ui->setToolName(tool, toolName);
		tool->SetToolInfo(toolInfo);
		tool->SetProperties(props);
	}
	else
	{
		// create the tool and add it to the UI
		CPythonTool* tool = new CPythonTool(GetMainWindow(), toolName);
		tool->SetInfo(toolInfo);
		tool->SetFilePath(filePath);
		tool->SetProperties(props);
		ui->addTool(tool);
	}
}

void CPythonToolsPanel::on_refresh_triggered()
{
	if ((ui->m_pythonThread == nullptr) && ui->m_activeTool)
	{
		LoadToolFromFile(ui->m_activeTool->GetFilePath());
	}
}

void CPythonToolsPanel::on_run_clicked()
{
	if (ui->m_activeTool == nullptr) return;
	if (ui->m_pythonThread)
	{
		QMessageBox::critical(this, "Python", "A python script is still running. Please wait.");
		return;
	}

	QString msg = QString("Running %1").arg(ui->m_activeTool->name());
	ui->runPane->startRunning(msg);

	CMainWindow* wnd = GetMainWindow();
	wnd->GetLogPanel()->ShowLog(CLogPanel::PYTHON_LOG);
	wnd->AddPythonLogEntry(QString(">>> running python tool \"%1\"\n").arg(ui->m_activeTool->name()));

	startThread();
}

void CPythonToolsPanel::on_pythonThread_threadFinished(bool b)
{
	if (b == false) QMessageBox::critical(this, "Python", "An error occurred while running the Python script.");
	ui->runPane->stopRunning();
	ui->m_pythonThread = nullptr;
	GetMainWindow()->Update(this, true);
}

void CPythonToolsPanel::on_buttons_idClicked(int id)
{
	// deactivate the active tool
	if (ui->m_activeTool) ui->m_activeTool->Deactivate();
	ui->m_activeTool = nullptr;

	if ((id <= 0) || (id > ui->tools.size()))
	{
		ui->runPane->hide();
		return;
	}

	// activate the tool
	ui->m_activeTool = ui->tools[id - 1];
	ui->m_activeTool->Activate();

	// show the tab
	ui->paramStack->setCurrentIndex(id);
	ui->runPane->show();
	ui->runPane->setCurrentIndex(0);
}

void CPythonToolsPanel::hideEvent(QHideEvent* ev)
{
	if (ui->m_activeTool)
	{
		ui->m_activeTool->Deactivate();
	}
	ev->accept();
}

void CPythonToolsPanel::showEvent(QShowEvent* ev)
{
	if (ui->m_activeTool)
	{
		ui->m_activeTool->Activate();
	}
	ev->accept();
}
