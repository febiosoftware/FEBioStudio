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
// #include "PyFBS.cpp"
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

#include "PythonToolsPanel.h"
#include "ui_pythontoolspanel.h"
#include <PyLib/PythonTool.h>
#include <QFileDialog>
#include <FEBioStudio/MainWindow.h>
#include <PyLib/PyThread.h>
#include "PyOutput.h"

CPythonToolsPanel::CPythonToolsPanel(CMainWindow* wnd, QWidget* parent) 
	: CCommandPanel(wnd, parent), ui(new Ui::CPythonToolsPanel), m_wnd(wnd), inputHandler(this)
{
	m_activeTool = 0;
	ui->setupUi(this);
}

void CPythonToolsPanel::initPython()
{

    m_pythonThread = new CPyThread(this);
    connect(m_pythonThread, &CPyThread::ExecDone, this, &CPythonToolsPanel::on_pythonThread_ExecDone);

    m_pythonThread->start();
}

CPythonToolsPanel::~CPythonToolsPanel()
{
    m_pythonThread->quit();
}

void CPythonToolsPanel::Update(bool breset)
{
	if (m_activeTool)
	{
		m_activeTool->CAbstractTool::Update();
	}
}

CPythonTool* CPythonToolsPanel::addTool(std::string name, pybind11::function func)
{
	CPythonTool* tool = new CPythonTool(GetMainWindow(), name, func);

	tools.push_back(tool);

	return tool;
}

CPyThread* CPythonToolsPanel::GetThread()
{
    return m_pythonThread;
}

void CPythonToolsPanel::runScript(QString filename)
{
	FILE* file;
    file = fopen(filename.toStdString().c_str(), "r");
	
	PyRun_SimpleFile(file, filename.toStdString().c_str());
}

void CPythonToolsPanel::startRunning(const QString& msg)
{
	ui->startRunning(msg);
}

void CPythonToolsPanel::setProgressText(const QString& txt)
{
	ui->setProgressText(txt);
}

void CPythonToolsPanel::setProgress(int prog)
{
	ui->setProgress(prog);
}

void CPythonToolsPanel::on_pythonThread_ExecDone()
{
    ui->refreshPanel();

    for(CPythonTool* tool : tools)
    {
        ui->addTool(tool);
    }

	ui->stopRunning();

	m_wnd->UpdateModel();
	m_wnd->UpdateUI();
}

void CPythonToolsPanel::on_importScript_triggered()
{
	QString filename = QFileDialog::getOpenFileName(this, "Python Script", "", "Python scripts (*.py)");
    
	if(filename.isEmpty()) return;

    m_pythonThread->SetFilename(filename);
}

void CPythonToolsPanel::on_refresh_triggered()
{
	ui->refreshPanel();

	for(auto tool : tools)
	{
		delete tool;
	}
	tools.clear();

	m_activeTool = nullptr;
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
	auto it = tools.begin();
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
void CPythonToolsPanel::startRunning(const QString& msg) {}
CPythonInputHandler* CPythonToolsPanel::getInputHandler() {return nullptr;}
void CPythonToolsPanel::addInputPage(QWidget* wgt) {}
QWidget* CPythonToolsPanel::getInputWgt() {return nullptr;}
void CPythonToolsPanel::removeInputPage() {}
void CPythonToolsPanel::addLog(QString txt) {}
void CPythonToolsPanel::setProgressText(const QString& txt) {}
void CPythonToolsPanel::setProgress(int prog) {}
void CPythonToolsPanel::on_buttons_idClicked(int id) {}
void CPythonToolsPanel::hideEvent(QHideEvent* ev) {}
void CPythonToolsPanel::showEvent(QShowEvent* ev) {}
#endif