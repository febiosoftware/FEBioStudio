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
#include <PyLib/fbsmodule.h>
#include "PythonToolsPanel.h"
#include "ui_pythontoolspanel.h"
#include <PyLib/PythonTool.h>
#include <QFileDialog>
#include "MainWindow.h"


CPythonToolsPanel::CPythonToolsPanel(CMainWindow* wnd, QWidget* parent) : CCommandPanel(wnd, parent), ui(new Ui::CPythonToolsPanel)
{
	m_activeTool = 0;
	ui->setupUi(this);

	QMetaObject::connectSlotsByName(this);

	PyImport_AppendInittab("fbs", &PyInit_fbs);
    Py_Initialize();
}

// CPythonToolsPanel::~CPythonToolsPanel()
// {
//     Py_Finalize();
// }

void CPythonToolsPanel::Update(bool breset)
{
	if (m_activeTool)
	{
		m_activeTool->Update();
	}
}

CPythonTool* CPythonToolsPanel::addTool(const char* name, PyObject* func)
{
	CPythonTool* tool = new CPythonTool(GetMainWindow(), name, func);

	tools.push_back(tool);

	return tool;
}

void CPythonToolsPanel::finalizeTool(CPythonTool* tool)
{
	ui->addTool(tool);
}

void CPythonToolsPanel::on_importScript_triggered()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Python Script", "", "Python scripts (*.py)");

	FILE* file;
	file = fopen(fileName.toStdString().c_str(), "r");
	PyRun_SimpleFile(file, fileName.toStdString().c_str());
}

void CPythonToolsPanel::on_buttons_buttonClicked(int id)
{
	// deactivate the active tool
	if (m_activeTool) m_activeTool->Deactivate();
	m_activeTool = 0;

	// find the tool
	QList<CAbstractTool*>::iterator it = tools.begin();
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
