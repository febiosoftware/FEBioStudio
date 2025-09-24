/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
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
#include "PythonEditor.h"
#include "MainWindow.h"
#include "LogPanel.h"
#include "ui_pythoneditor.h"
#include <PyLib/PythonRunner.h>
#include <PyLib/PyRunContext.h>

CPythonEditor::CPythonEditor(CMainWindow* wnd) : QMainWindow(wnd), mainWnd(wnd), ui(new Ui::CPythonEditor)
{
	setWindowTitle("Python Editor");
	setMinimumSize(800, 600);
	ui->setup(this, wnd->usingDarkTheme());
	ui->edit->appendPlainText("from fbs import *\n");

	// Hook this up to the python runner (remember, this lives on a separate thread)
	CPythonRunner* pyrun = CPythonRunner::GetInstance(); assert(pyrun);
	connect(this, &CPythonEditor::runScript, pyrun, &CPythonRunner::runScript);
	connect(pyrun, &CPythonRunner::runScriptFinished, this, &CPythonEditor::on_python_finished);
}

void CPythonEditor::on_actionNew_triggered()
{
	if (QMessageBox::question(this, "New Script", "Are you sure you want to start a new script?") == QMessageBox::Yes)
	{
		ui->edit->clear();
		ui->edit->appendPlainText("from fbs import *\n");
		fileName.clear();
		ui->isModified = false;
		updateWindowTitle();
	}
}

void CPythonEditor::on_actionOpen_triggered()
{
	QString filePath = QFileDialog::getOpenFileName(this, "Open Python file", "", "Python files (*.py)");
	if (!filePath.isEmpty())
	{
		QFile file(filePath);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			QMessageBox::critical(this, "Open Python file", "Failed to open the file!");
			return;
		}

		// Read the entire file contents using QTextStream
		QTextStream in(&file);
		QString fileContent = in.readAll();

		// Close the file
		file.close();

		ui->edit->setPlainText(fileContent);

		fileName = filePath;
		ui->isModified = false;
		updateWindowTitle();
	}
}

bool SaveScript(const QString& filePath, const QString& fileText)
{
	if (filePath.isEmpty()) return false;

	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		return false;
	}

	// Read the entire file contents using QTextStream
	QTextStream out(&file);
	out << fileText;

	// Close the file
	file.close();

	return true;
}

void CPythonEditor::updateWindowTitle()
{
	if (fileName.isEmpty())
	{
		setWindowTitle("Python Editor");
	}
	else
	{
		QFileInfo fi(fileName);
		if (ui->isModified)
			setWindowTitle("Python Editor [" + fi.fileName() + "*]");
		else
			setWindowTitle("Python Editor [" + fi.fileName() + "]");
	}
}

void CPythonEditor::on_actionSave_triggered()
{
	if (fileName.isEmpty())
	{
		on_actionSaveAs_triggered();
	}
	else
	{
		QString script = ui->edit->toPlainText();
		if (!SaveScript(fileName, script))
		{
			QMessageBox::critical(this, "Python Editor", "Failed to save the script to file.");
		}

		ui->isModified = false;
		updateWindowTitle();
	}
}

void CPythonEditor::on_actionSaveAs_triggered()
{
	QString filePath = QFileDialog::getSaveFileName(this, "Save Python file", "", "Python files (*.py)");
	if (!filePath.isEmpty())
	{
		QString script = ui->edit->toPlainText();
		if (SaveScript(filePath, script))
		{
			fileName = filePath;
			ui->isModified = false;
			updateWindowTitle();
		}
		else
		{
			QMessageBox::critical(this, "Python Editor", "Failed to save the script to file.");
		}
	}
}

void CPythonEditor::on_actionClose_triggered()
{
	close();
}

void CPythonEditor::on_actionRun_triggered()
{
	CPythonRunner* pyrun = CPythonRunner::GetInstance();
	if (pyrun->isBusy())
	{
		QMessageBox::critical(this, "Python", "A python script is still running. Please wait.");
		return;
	}

	mainWnd->GetLogPanel()->ShowLog(CLogPanel::PYTHON_LOG);
	mainWnd->AddPythonLogEntry(QString(">>> running python ...\n"));
		
	CDocument* doc = mainWnd->GetDocument();
	PyRunContext::SetDocument(doc);
	
	ui->actionRun->setEnabled(false);
	ui->actionStop->setEnabled(true);

	pyrun->SetWorkingDirectory(ui->editCwd->text());

	QString script = ui->edit->toPlainText();
	emit runScript(script);
}

void CPythonEditor::on_actionStop_triggered()
{
	CPythonRunner* pyrun = CPythonRunner::GetInstance();
	pyrun->interrupt();
}

void CPythonEditor::on_python_finished(bool b)
{
	if (b == false) QMessageBox::critical(this, "Python", "An error occurred while running the Python script.");

	ui->actionRun->setEnabled(true);
	ui->actionStop->setEnabled(false);

	mainWnd->Update(this, true);
	mainWnd->AddPythonLogEntry(QString(">>> python stopped\n"));
}

void CPythonEditor::on_edit_textChanged()
{
	if (!ui->isModified)
	{
		ui->isModified = true;
		updateWindowTitle();
	}
}
