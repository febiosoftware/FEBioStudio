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

#include <PyLib/pyBindtest.cpp>
#include <pybind11/embed.h>

#include "PythonToolsPanel.h"
#include "ui_pythontoolspanel.h"
#include <PyLib/PythonTool.h>
#include <QFileDialog>
#include "MainWindow.h"
#include <PyLib/PyThread.h>


CPythonToolsPanel::CPythonToolsPanel(CMainWindow* wnd, QWidget* parent) 
	: CCommandPanel(wnd, parent), ui(new Ui::CPythonToolsPanel), inputHandler(this)
{
	m_activeTool = 0;
	ui->setupUi(this);

	pybind11::initialize_interpreter();
}

CPythonToolsPanel::~CPythonToolsPanel()
{
    // Py_Finalize();
	pybind11::finalize_interpreter();
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

void CPythonToolsPanel::finalizeTools()
{
	for(auto dummyTool : dummyTools)
	{
		auto tool = addTool(dummyTool->name, dummyTool->func);

		for(auto prop : dummyTool->boolProps)
		{
			tool->addBoolProperty(prop.first, prop.second);
		}

		for(auto prop : dummyTool->intProps)
		{
			tool->addIntProperty(prop.first, prop.second);
		}

		for(auto prop : dummyTool->dblProps)
		{
			tool->addDoubleProperty(prop.first, prop.second);
		}

		for(auto prop : dummyTool->enumProps)
		{
			tool->addEnumProperty(prop.first, dummyTool->enumLabels[prop.first], prop.second);
		}

		for(auto prop : dummyTool->vec3Props)
		{
			tool->addVec3Property(prop.first, prop.second);
		}

		for(auto prop : dummyTool->strProps)
		{
			tool->addStringProperty(prop.first, prop.second);
		}

		for(auto prop : dummyTool->rscProps)
		{
			tool->addResourceProperty(prop.first, prop.second);
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
