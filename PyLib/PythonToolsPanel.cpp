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



#ifdef HAS_PYTHON
#include <PyLib/pyBindtest.cpp>
#include <pybind11/embed.h>

#include "PythonToolsPanel.h"
#include "ui_pythontoolspanel.h"
#include <PyLib/PythonTool.h>
#include <QFileDialog>
#include <FEBioStudio/MainWindow.h>
#include <PyLib/PyThread.h>
#include "PyOutput.h"

CPythonToolsPanel::CPythonToolsPanel(CMainWindow* wnd, QWidget* parent) 
	: CCommandPanel(wnd, parent), ui(new Ui::CPythonToolsPanel), inputHandler(this)
{
	m_activeTool = 0;
	ui->setupUi(this);
}

void CPythonToolsPanel::initPython()
{
	pybind11::initialize_interpreter();
 
	// setup output
 	auto sysm = pybind11::module::import("sys");
	auto output = pybind11::module::import("fbs").attr("PyOutput");
	sysm.attr("stdout") = output();
	sysm.attr("stderr") = output();
}

CPythonToolsPanel::~CPythonToolsPanel()
{
    finalizePython();
}

void CPythonToolsPanel::Update(bool breset)
{
	if (m_activeTool)
	{
		m_activeTool->Update();
	}
}

CPythonDummyTool* CPythonToolsPanel::addDummyTool(const char* name, pybind11::function func)
{
	CPythonDummyTool* tool = new CPythonDummyTool(name, func);

	dummyTools.push_back(tool);

	return tool;
}

CPythonTool* CPythonToolsPanel::addTool(std::string name, pybind11::function func)
{
	CPythonTool* tool = new CPythonTool(GetMainWindow(), name, func);

	tools.push_back(tool);

	return tool;
}

void CPythonToolsPanel::runScript(QString filename)
{
	FILE* file;
    file = fopen(filename.toStdString().c_str(), "r");
    
	
	PyRun_SimpleFile(file, filename.toStdString().c_str());
}

void CPythonToolsPanel::finalizePython()
{
	pybind11::finalize_interpreter();
}

void CPythonToolsPanel::finalizeTools()
{
	for(auto dummyTool : dummyTools)
	{
		auto tool = addTool(dummyTool->name, dummyTool->func);

		for(auto type : dummyTool->propOrder)
		{
			switch (type)
			{
			case CProperty::Bool:
				tool->addBoolProperty(dummyTool->boolProps.front().first, dummyTool->boolProps.front().second);
				dummyTool->boolProps.pop();
				break;
			case CProperty::Int:
				tool->addIntProperty(dummyTool->intProps.front().first, dummyTool->intProps.front().second);
				dummyTool->intProps.pop();
				break;
			case CProperty::Float:
				tool->addDoubleProperty(dummyTool->dblProps.front().first, dummyTool->dblProps.front().second);
				dummyTool->dblProps.pop();
				break;
			case CProperty::Enum:
				tool->addEnumProperty(dummyTool->enumProps.front().first, dummyTool->enumLabels.front(), dummyTool->enumProps.front().second);
				dummyTool->enumProps.pop();
				dummyTool->enumLabels.pop();
				break;
			case CProperty::Vec3:
				tool->addVec3Property(dummyTool->vec3Props.front().first, dummyTool->vec3Props.front().second);
				dummyTool->vec3Props.pop();
				break;
			case CProperty::String:
				tool->addStringProperty(dummyTool->strProps.front().first, dummyTool->strProps.front().second);
				dummyTool->strProps.pop();
				break;
			case CProperty::Resource:
				tool->addResourceProperty(dummyTool->rscProps.front().first, dummyTool->rscProps.front().second);
				dummyTool->rscProps.pop();
				break;
			default:
				break;
			}
		}

		ui->addTool(tool);

		delete dummyTool;
	}

	dummyTools.clear();
}

void CPythonToolsPanel::endThread()
{
	pybind11::gil_scoped_acquire acquire;
	finalizeTools();
}

void CPythonToolsPanel::on_importScript_triggered()
{
	QString filename = QFileDialog::getOpenFileName(this, "Python Script", "", "Python scripts (*.py)");

	pybind11::gil_scoped_release release;
	CPyThread* thread = new CPyThread(this, filename);
	QObject::connect(thread, &CPyThread::finished, this, &CPythonToolsPanel::endThread);
	thread->start();
}

void CPythonToolsPanel::on_refresh_triggered()
{
	ui->removeTools();

	for(auto tool : tools)
	{
		delete tool;
	}
	tools.clear();

	m_activeTool = nullptr;
	
	finalizePython();
	initPython();
}

CPythonInputHandler* CPythonToolsPanel::getInputHandler()
{
	return &inputHandler;
}

void CPythonToolsPanel::addInputPage(QWidget* wgt)
{
	ui->addPage(wgt);
}

QWidget* CPythonToolsPanel::getInputWgt()
{
	return ui->parentStack->currentWidget();
}

void CPythonToolsPanel::removeInputPage()
{
	ui->removePage();
}

void CPythonToolsPanel::addLog(QString txt)
{
	ui->txt->moveCursor(QTextCursor::End);
	ui->txt->insertPlainText(txt);
	ui->txt->moveCursor(QTextCursor::End);
}


void CPythonToolsPanel::on_buttons_idClicked(int id)
{
	// deactivate the active tool
	if (m_activeTool) m_activeTool->Deactivate();
	m_activeTool = 0;

	// find the tool
	QList<CPythonTool*>::iterator it = tools.begin();
	for (int i = 0; i<id - 1; ++i, ++it);

	// activate the tool
	m_activeTool = *it;
	m_activeTool->Activate();

	// show the tab
	ui->stack->setCurrentIndex(id);
}

void CPythonToolsPanel::hideEvent(QHideEvent* ev)
{
	if (m_activeTool)
	{
		m_activeTool->Deactivate();
	}
	ev->accept();
}

void CPythonToolsPanel::showEvent(QShowEvent* ev)
{
	if (m_activeTool)
	{
		m_activeTool->Activate();
	}
	ev->accept();
}

#else
#include "PythonToolsPanel.h"
#include "ui_pythontoolspanel.h"

namespace pybind11
{
	class function
	{

	};
}

CPythonToolsPanel::CPythonToolsPanel(CMainWindow* wnd, QWidget* parent) : CCommandPanel(wnd, parent), ui(new Ui::CPythonToolsPanel), inputHandler(this) {}
void CPythonToolsPanel::initPython() {}
CPythonToolsPanel::~CPythonToolsPanel() {}
void CPythonToolsPanel::Update(bool breset) {}
CPythonDummyTool* CPythonToolsPanel::addDummyTool(const char* name, pybind11::function func) {return nullptr;}
CPythonTool* CPythonToolsPanel::addTool(std::string name, pybind11::function func) {return nullptr;}
void CPythonToolsPanel::runScript(QString filename) {}
void CPythonToolsPanel::finalizePython() {}
void CPythonToolsPanel::finalizeTools() {}
void CPythonToolsPanel::endThread() {}
void CPythonToolsPanel::on_importScript_triggered() {}
void CPythonToolsPanel::on_refresh_triggered() {}
CPythonInputHandler* CPythonToolsPanel::getInputHandler() {return nullptr;}
void CPythonToolsPanel::addInputPage(QWidget* wgt) {}
QWidget* CPythonToolsPanel::getInputWgt() {return nullptr;}
void CPythonToolsPanel::removeInputPage() {}
void CPythonToolsPanel::addLog(QString txt) {}
void CPythonToolsPanel::on_buttons_idClicked(int id) {}
void CPythonToolsPanel::hideEvent(QHideEvent* ev) {}
void CPythonToolsPanel::showEvent(QShowEvent* ev) {}
#endif