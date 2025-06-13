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
#include <FEBioStudio/ModelDocument.h>
#include <FEBioStudio/LogPanel.h>
#include <QMessageBox>
#include "PyRunContext.h"

CPythonToolsPanel::CPythonToolsPanel(CMainWindow* wnd, QWidget* parent) 
	: CWindowPanel(wnd, parent), ui(new Ui::CPythonToolsPanel)
{
	ui->setupUi(this);

	ui->m_pyRunner = new CPythonRunner;
	ui->m_pyRunner->moveToThread(&ui->m_pyThread);

	connect(&ui->m_pyThread, &QThread::finished, ui->m_pyRunner, &QObject::deleteLater);
	connect(this, &CPythonToolsPanel::runFile, ui->m_pyRunner, &CPythonRunner::runFile);
	connect(this, &CPythonToolsPanel::runTool, ui->m_pyRunner, &CPythonRunner::runTool);
	connect(ui->m_pyRunner, &CPythonRunner::runFileFinished, this, &CPythonToolsPanel::on_pyRunner_runFileFinished);
	connect(ui->m_pyRunner, &CPythonRunner::runToolFinished, this, &CPythonToolsPanel::on_pyRunner_runToolFinished);

	ui->m_pyThread.start();
}

CPythonToolsPanel::~CPythonToolsPanel()
{
	if (ui->m_pyThread.isRunning())
	{
		ui->m_pyThread.quit();
		ui->m_pyThread.wait();
	}
}

void CPythonToolsPanel::Update(bool breset)
{
	if (ui->m_activeTool)
	{
		ui->m_activeTool->CAbstractTool::Update();
	}
}

void CPythonToolsPanel::addPythonTool(QString toolName, CCachedPropertyList* props, QString info)
{
	CPythonTool* tool = new CPythonTool(GetMainWindow(), toolName);
	tool->SetProperties(props);
	tool->SetInfo(info);
	ui->addTool(tool);
	Update();
}

void CPythonToolsPanel::on_importScript_triggered()
{
	if (ui->m_pyRunner->isBusy())
	{
		QMessageBox::information(this, "Python", "A Python script is still running. Please wait.");
		return;
	}

	QString filePath = QFileDialog::getOpenFileName(this, "Python Script", "", "Python scripts (*.py)");
	if (filePath.isEmpty()) return;

	CMainWindow* wnd = GetMainWindow();
	wnd->GetLogPanel()->ShowLog(CLogPanel::PYTHON_LOG);
	wnd->AddPythonLogEntry(QString(">>> running file \"%1\"\n").arg(filePath));

	emit runFile(filePath);
}

void CPythonToolsPanel::on_refresh_triggered()
{
/*	if ((ui->m_pythonThread == nullptr) && ui->m_activeTool)
	{
		LoadToolFromFile(ui->m_activeTool->GetFilePath());
	}
*/
}

void CPythonToolsPanel::on_pyRunner_runFileFinished(bool b)
{
	if (b == false) QMessageBox::critical(this, "Python", "An error occurred while running the Python script.");
	CMainWindow* wnd = GetMainWindow();
	wnd->Update(this, true);
	wnd->AddPythonLogEntry(QString(">>> python stopped\n"));
}

void CPythonToolsPanel::on_pyRunner_runToolFinished(bool b)
{
	if (b == false) QMessageBox::critical(this, "Python", "An error occurred while running the Python script.");
	CMainWindow* wnd = GetMainWindow();
	wnd->Update(this, true);
	wnd->AddPythonLogEntry(QString(">>> python stopped\n"));
}

void CPythonToolsPanel::on_buttons_idClicked(int id)
{
	// deactivate the active tool
	if (ui->m_activeTool) ui->m_activeTool->Deactivate();
	ui->m_activeTool = nullptr;

	if ((id <= 0) || (id > ui->tools.size()))
	{
		return;
	}

	// activate the tool
	ui->m_activeTool = ui->tools[id - 1];
	ui->m_activeTool->Activate();

	// show the tab
	ui->paramStack->setCurrentIndex(id);
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
