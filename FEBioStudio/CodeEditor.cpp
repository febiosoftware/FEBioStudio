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
#include "CodeEditor.h"
#include "MainWindow.h"
#include "LogPanel.h"
#include "ui_codeeditor.h"

CCodeEditor::CCodeEditor(CMainWindow* wnd) : QMainWindow(wnd), mainWnd(wnd), ui(new Ui::CCodeEditor)
{
	setWindowTitle("Code Editor");
	setMinimumSize(800, 600);
	ui->setup(this, wnd->usingDarkTheme());
	ui->edit->appendPlainText("return 0.0;");
}

void CCodeEditor::closeEvent(QCloseEvent* event)
{
	if (GetDocument()) SetDocument(nullptr);
	ui->script = nullptr;
}

void CCodeEditor::DocumentDelete()
{
	CDocObserver::DocumentDelete();
	ui->script = nullptr;
	close();
}

void CCodeEditor::SetScript(CDocument* doc, FEBCodeScript* script)
{
	if (GetDocument() != doc)
	{
		SetDocument(doc);
	}

	ui->edit->clear();
	ui->script = script;
	if (script)
	{
		ui->edit->appendPlainText(QString::fromStdString(script->code));
		updateWindowTitle();
	}
	else
	{
		updateWindowTitle();
	}
}

void CCodeEditor::on_actionOpen_triggered()
{
	if (ui->script == nullptr) return;

	QString filePath = QFileDialog::getOpenFileName(this, "Open FEBCode file", "", "FEBCode files (*.febc)");
	if (!filePath.isEmpty())
	{
		QFile file(filePath);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			QMessageBox::critical(this, "Open FEBCode file", "Failed to open the file!");
			return;
		}

		// Read the entire file contents using QTextStream
		QTextStream in(&file);
		QString fileContent = in.readAll();

		// Close the file
		file.close();

		ui->edit->setPlainText(fileContent);
		updateWindowTitle();
	}
}

// in pythoneditor.cpp
bool SaveScript(const QString& filePath, const QString& fileText);

void CCodeEditor::updateWindowTitle()
{
	if (ui->script == nullptr)
	{
		setWindowTitle("FEBCode Editor");
	}
	else
	{
		QString name = QString::fromStdString(ui->script->name);
		setWindowTitle("FEBCode Editor [" + name + "]");
	}
}

void CCodeEditor::on_actionSave_triggered()
{
	QString filePath = QFileDialog::getSaveFileName(this, "Save FEBCode file", "", "FEBCode files (*.febc)");
	if (!filePath.isEmpty())
	{
		QString script = ui->edit->toPlainText();
		if (SaveScript(filePath, script))
		{
			updateWindowTitle();
		}
		else
		{
			QMessageBox::critical(this, "FEBCode Editor", "Failed to save the script to file.");
		}
	}
}

void CCodeEditor::on_edit_textChanged()
{
	if (ui->script)
	{
		ui->script->code = ui->edit->toPlainText().toStdString();
	}
}
