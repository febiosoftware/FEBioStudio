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
#include <PyLib/PythonThread.h>
#include "PyOutput.h"
#include <FEBioStudio/Logger.h>
#include <FEBioStudio/LogPanel.h>
#include <QMessageBox>

CPythonToolsPanel::CPythonToolsPanel(CMainWindow* wnd, QWidget* parent) 
	: CWindowPanel(wnd, parent), ui(new Ui::CPythonToolsPanel)
{
	ui->setupUi(this);
	ui->m_pythonThread = nullptr;
}

void CPythonToolsPanel::startThread()
{
	if (ui->m_pythonThread == nullptr)
	{
		ui->m_pythonThread = new CPyThread();
		connect(ui->m_pythonThread, &CPyThread::ExecDone, this, &CPythonToolsPanel::on_pythonThread_ExecDone);
		connect(ui->m_pythonThread, &CPyThread::Restarted, this, &CPythonToolsPanel::on_pythonThread_Restarted);
		connect(ui->m_pythonThread, &CPyThread::ToolFinished, this, &CPythonToolsPanel::on_pythonThread_ToolFinished);

		ui->m_pythonThread->start();
	}
}

CPythonToolsPanel::~CPythonToolsPanel()
{
	if (ui->m_pythonThread)
	{
		ui->m_pythonThread->Stop();
		ui->m_pythonThread->wait();
	}
}

void CPythonToolsPanel::Update(bool breset)
{
	if (ui->m_activeTool)
	{
		ui->m_activeTool->CAbstractTool::Update();
	}
}

void CPythonToolsPanel::addDummyTool(CPythonToolProps* tool)
{
	int id = ui->tools.size() + ui->dummyTools.size();
	tool->SetID(id);
	ui->dummyTools.push_back(tool);
}

CPythonTool* CPythonToolsPanel::CreateTool(CPythonToolProps* p)
{
	CPythonTool* tool = new CPythonTool(GetMainWindow(), p->GetName());
	tool->SetProperties(p);
	ui->tools.push_back(tool);
	return tool;
}

void CPythonToolsPanel::setProgressText(const QString& txt)
{
	ui->runPane->setProgressText(txt);
}

void CPythonToolsPanel::setProgress(int prog)
{
	ui->runPane->setProgress(prog);
}

void CPythonToolsPanel::BuildTools()
{
	for (auto p : ui->dummyTools)
	{
		// Note that tool takes ownership of the properties
		auto tool = CreateTool(p);
		ui->addTool(tool);
	}
	ui->dummyTools.clear();
}

void CPythonToolsPanel::on_importScript_triggered()
{
	QString filename = QFileDialog::getOpenFileName(this, "Python Script", "", "Python scripts (*.py)");

	if (filename.isEmpty()) return;

	if (ui->m_pythonThread == nullptr) startThread();

	GetMainWindow()->GetLogPanel()->ShowLog(CLogPanel::PYTHON_LOG);
	CLogger::AddPythonLogEntry(QString(">>> running file %1\n").arg(filename));
	ui->m_pythonThread->SetFilename(filename);
}

void CPythonToolsPanel::on_run_clicked()
{
	if (ui->m_activeTool == nullptr) return;
	QString msg = QString("Running %1").arg(ui->m_activeTool->name());
	ui->runPane->startRunning(msg);

	if (ui->m_pythonThread == nullptr) startThread();

	CMainWindow* wnd = GetMainWindow();
	wnd->GetLogPanel()->ShowLog(CLogPanel::PYTHON_LOG);
	CLogger::AddPythonLogEntry(QString(">>> running python tool \"%1\"\n").arg(ui->m_activeTool->name()));
	ui->m_pythonThread->SetTool(ui->m_activeTool->GetProperties());
}

void CPythonToolsPanel::on_pythonThread_ExecDone()
{
	BuildTools();

	ui->runPane->stopRunning();

	CMainWindow* wnd = GetMainWindow();
	wnd->UpdateModel();
	wnd->UpdateUI();
}

void CPythonToolsPanel::on_pythonThread_Restarted()
{
	ui->refreshPanel();

	for(auto tool : ui->tools)
	{
		delete tool;
	}
	ui->tools.clear();

	ui->m_activeTool = nullptr;
	ui->runPane->hide();
}

void CPythonToolsPanel::on_pythonThread_ToolFinished(bool b)
{
	if (b == false) QMessageBox::critical(this, "Python", "An error occurred while running the Python script.");
	ui->runPane->stopRunning();
}

void CPythonToolsPanel::on_refresh_triggered()
{
	if (ui->m_pythonThread)
		ui->m_pythonThread->Restart();
}

/*
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
*/

void CPythonToolsPanel::addLog(QString txt)
{
	CLogger::AddPythonLogEntry(txt);
}

void CPythonToolsPanel::on_buttons_idClicked(int id)
{
	// deactivate the active tool
	if (ui->m_activeTool) ui->m_activeTool->Deactivate();
	ui->m_activeTool = nullptr;

	if (id == -1)
	{
		ui->runPane->hide();
		return;
	}

	// find the tool
	auto it = ui->tools.begin();
	for (int i = 0; i<id - 1; ++i, ++it);

	// activate the tool
	ui->m_activeTool = *it;
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
