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
#include <QCloseEvent>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMimeData>
#include <QPlainTextEdit>
#include <QTextBlock>
#include <QTextCursor>
#include <QUrl>

namespace
{
	const QString PYTHON_INDENT("    ");

	int firstSelectedBlock(const QTextCursor& cursor)
	{
		return cursor.document()->findBlock(cursor.selectionStart()).blockNumber();
	}

	int lastSelectedBlock(const QTextCursor& cursor)
	{
		if (!cursor.hasSelection()) return cursor.block().blockNumber();

		int end = cursor.selectionEnd();
		if (end > cursor.selectionStart()) end--;
		return cursor.document()->findBlock(end).blockNumber();
	}

	int leadingWhitespaceLength(const QString& text)
	{
		int n = 0;
		while (n < text.length() && text[n].isSpace() && text[n] != '\n') n++;
		return n;
	}

	QString normalizeLeadingIndentation(const QString& text)
	{
		int indentLength = leadingWhitespaceLength(text);
		QString indent = text.left(indentLength);
		indent.replace("\t", PYTHON_INDENT);
		return indent + text.mid(indentLength);
	}
}

CPythonEditor::CPythonEditor(CMainWindow* wnd) : QMainWindow(wnd), mainWnd(wnd), ui(new Ui::CPythonEditor)
{
	setWindowTitle("Python Editor");
	setMinimumSize(800, 600);
	ui->setup(this, wnd->usingDarkTheme());
	ui->edit->appendPlainText("from fbs import *\n");
	ui->edit->installEventFilter(this);
	ui->edit->viewport()->installEventFilter(this);

	// Hook this up to the python runner (remember, this lives on a separate thread)
	CPythonRunner* pyrun = CPythonRunner::GetInstance(); assert(pyrun);
	connect(this, &CPythonEditor::runScript, pyrun, &CPythonRunner::runScript);
	connect(pyrun, &CPythonRunner::runScriptFinished, this, &CPythonEditor::on_python_finished);
}

CPythonEditor::~CPythonEditor()
{
	delete ui;
}

void CPythonEditor::closeEvent(QCloseEvent* ev)
{
	if (saveModifiedScript())
	{
		QMainWindow::closeEvent(ev);
	}
	else
	{
		ev->ignore();
	}
}

bool CPythonEditor::eventFilter(QObject* obj, QEvent* ev)
{
	if (((obj == ui->edit) || (obj == ui->edit->viewport())) && ((ev->type() == QEvent::DragEnter) || (ev->type() == QEvent::DragMove)))
	{
		QDragMoveEvent* dragEvent = static_cast<QDragMoveEvent*>(ev);
		const QMimeData* mimeData = dragEvent->mimeData();
		if (mimeData && mimeData->hasUrls())
		{
			const QList<QUrl> urls = mimeData->urls();
			if ((urls.size() == 1) && urls[0].isLocalFile() && (QFileInfo(urls[0].toLocalFile()).suffix().compare("py", Qt::CaseInsensitive) == 0))
			{
				dragEvent->acceptProposedAction();
				return true;
			}
		}
	}
	else if (((obj == ui->edit) || (obj == ui->edit->viewport())) && (ev->type() == QEvent::Drop))
	{
		QDropEvent* dropEvent = static_cast<QDropEvent*>(ev);
		const QMimeData* mimeData = dropEvent->mimeData();
		if (mimeData && mimeData->hasUrls())
		{
			const QList<QUrl> urls = mimeData->urls();
			if ((urls.size() == 1) && urls[0].isLocalFile())
			{
				QString filePath = urls[0].toLocalFile();
				if (QFileInfo(filePath).suffix().compare("py", Qt::CaseInsensitive) == 0)
				{
					dropEvent->acceptProposedAction();
					if (saveModifiedScript()) openScript(filePath);
					return true;
				}
			}
		}
	}
	else if ((obj == ui->edit) && (ev->type() == QEvent::KeyPress))
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(ev);
		if ((keyEvent->key() == Qt::Key_Return) || (keyEvent->key() == Qt::Key_Enter))
		{
			if (keyEvent->modifiers() == Qt::NoModifier)
			{
				autoIndent();
				return true;
			}
		}
		else if (keyEvent->key() == Qt::Key_Tab)
		{
			if (ui->edit->textCursor().hasSelection())
			{
				indentSelection();
				return true;
			}
		}
		else if (keyEvent->key() == Qt::Key_Backtab)
		{
			unindentSelection();
			return true;
		}
	}

	return QMainWindow::eventFilter(obj, ev);
}

void CPythonEditor::on_actionNew_triggered()
{
	if (saveModifiedScript())
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
	if (!saveModifiedScript()) return;

	QString filePath = QFileDialog::getOpenFileName(this, "Open Python file", "", "Python files (*.py)");
	if (!filePath.isEmpty()) openScript(filePath);
}

static bool SaveScript(const QString& filePath, const QString& fileText)
{
	if (filePath.isEmpty()) return false;

	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		return false;
	}

	QTextStream out(&file);
	out << fileText;

	file.close();

	return true;
}

void CPythonEditor::updateWindowTitle()
{
	if (fileName.isEmpty())
	{
		setWindowTitle(ui->isModified ? "Python Editor*" : "Python Editor");
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

bool CPythonEditor::openScript(const QString& filePath)
{
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox::critical(this, "Open Python file", "Failed to open the file!");
		return false;
	}

	QTextStream in(&file);
	QString fileContent = in.readAll();
	file.close();

	ui->edit->setPlainText(fileContent);

	fileName = filePath;
	ui->editCwd->setText(QFileInfo(fileName).absolutePath());
	ui->isModified = false;
	updateWindowTitle();
	return true;
}

void CPythonEditor::on_actionSave_triggered()
{
	saveScript();
}

void CPythonEditor::on_actionSaveAs_triggered()
{
	saveScriptAs();
}

bool CPythonEditor::saveScript()
{
	if (fileName.isEmpty()) return saveScriptAs();

	QString script = ui->edit->toPlainText();
	if (!SaveScript(fileName, script))
	{
		QMessageBox::critical(this, "Python Editor", "Failed to save the script to file.");
		return false;
	}

	ui->isModified = false;
	updateWindowTitle();
	return true;
}

bool CPythonEditor::saveScriptAs()
{
	QString filePath = QFileDialog::getSaveFileName(this, "Save Python file", "", "Python files (*.py)");
	if (!filePath.isEmpty())
	{
		if (QFileInfo(filePath).suffix().isEmpty()) filePath += ".py";

		QString script = ui->edit->toPlainText();
		if (SaveScript(filePath, script))
		{
			fileName = filePath;
			ui->isModified = false;
			updateWindowTitle();
			return true;
		}
		else
		{
			QMessageBox::critical(this, "Python Editor", "Failed to save the script to file.");
		}
	}

	return false;
}

bool CPythonEditor::saveModifiedScript()
{
	if (!ui->isModified) return true;

	QMessageBox::StandardButton ret = QMessageBox::warning(
		this,
		"Python Editor",
		"The script has been modified.\nDo you want to save your changes?",
		QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
		QMessageBox::Save);

	if (ret == QMessageBox::Save) return saveScript();
	if (ret == QMessageBox::Discard) return true;
	return false;
}

void CPythonEditor::on_actionClose_triggered()
{
	close();
}

void CPythonEditor::on_actionFind_triggered()
{
	if (promptFindText()) findText(true);
}

void CPythonEditor::on_actionFindNext_triggered()
{
	if (m_findText.isEmpty())
	{
		if (!promptFindText()) return;
	}
	findText(true);
}

void CPythonEditor::on_actionFindPrevious_triggered()
{
	if (m_findText.isEmpty())
	{
		if (!promptFindText()) return;
	}
	findText(false);
}

void CPythonEditor::on_actionReplace_triggered()
{
	if (promptFindText() && promptReplaceText()) replaceCurrent();
}

void CPythonEditor::on_actionReplaceAll_triggered()
{
	if (promptFindText() && promptReplaceText())
	{
		int count = replaceAll();
		QMessageBox::information(this, "Python Editor", QString("Replaced %1 occurrence(s).").arg(count));
	}
}

void CPythonEditor::on_actionGoToLine_triggered()
{
	bool ok = false;
	int line = QInputDialog::getInt(this, "Go to Line", "Line:", ui->edit->textCursor().blockNumber() + 1, 1, ui->edit->blockCount(), 1, &ok);
	if (ok) goToLine(line);
}

void CPythonEditor::on_actionToggleComment_triggered()
{
	toggleCommentSelection();
}

void CPythonEditor::on_actionIndent_triggered()
{
	indentSelection();
}

void CPythonEditor::on_actionUnindent_triggered()
{
	unindentSelection();
}

void CPythonEditor::on_actionNormalizeIndentation_triggered()
{
	normalizeIndentation();
}

void CPythonEditor::on_actionDuplicateLine_triggered()
{
	ui->edit->duplicateLine();
}

void CPythonEditor::on_actionRun_triggered()
{
	CPythonRunner* pyrun = CPythonRunner::GetInstance();
	if (pyrun->isBusy())
	{
		QMessageBox::critical(this, "Python", "A python script is still running. Please wait.");
		return;
	}

	QString cwd = ui->editCwd->text().trimmed();
	if (!cwd.isEmpty() && !QDir(cwd).exists())
	{
		QMessageBox::critical(this, "Python", QString("The working directory does not exist:\n%1").arg(cwd));
		return;
	}

	mainWnd->GetLogPanel()->ShowLog(CLogPanel::PYTHON_LOG);
	mainWnd->AddPythonLogEntry(QString(">>> running python ...\n"));
		
	CDocument* doc = mainWnd->GetDocument();
	PyRunContext::SetDocument(doc);
	
	ui->actionRun->setEnabled(false);
	ui->actionStop->setEnabled(true);

	pyrun->SetWorkingDirectory(cwd);

	QString script = ui->edit->toPlainText();
	m_runTimer.start();
	emit runScript(script);
}

void CPythonEditor::on_actionStop_triggered()
{
	ui->actionStop->setEnabled(false);
	mainWnd->AddPythonLogEntry(QString(">>> stopping python ...\n"));

	CPythonRunner* pyrun = CPythonRunner::GetInstance();
	pyrun->interrupt();
}

void CPythonEditor::on_actionWordWrap_toggled(bool checked)
{
	ui->edit->setLineWrapMode(checked ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
}

void CPythonEditor::on_actionShowWhitespace_toggled(bool checked)
{
	ui->edit->setShowWhitespace(checked);
}

bool CPythonEditor::promptFindText()
{
	QString selected = ui->edit->textCursor().selectedText();
	if (!selected.contains(QChar::ParagraphSeparator) && !selected.contains('\n') && !selected.isEmpty())
	{
		m_findText = selected;
	}

	bool ok = false;
	QString text = QInputDialog::getText(this, "Find", "Find:", QLineEdit::Normal, m_findText, &ok);
	if (!ok || text.isEmpty()) return false;

	m_findText = text;
	return true;
}

bool CPythonEditor::promptReplaceText()
{
	bool ok = false;
	QString text = QInputDialog::getText(this, "Replace", "Replace with:", QLineEdit::Normal, m_replaceText, &ok);
	if (!ok) return false;

	m_replaceText = text;
	return true;
}

bool CPythonEditor::findText(bool forward)
{
	if (m_findText.isEmpty()) return false;

	QTextDocument::FindFlags flags;
	if (!forward) flags |= QTextDocument::FindBackward;

	if (ui->edit->find(m_findText, flags))
	{
		ui->edit->centerCursor();
		return true;
	}

	QTextCursor cursor = ui->edit->textCursor();
	cursor.movePosition(forward ? QTextCursor::Start : QTextCursor::End);
	ui->edit->setTextCursor(cursor);

	if (ui->edit->find(m_findText, flags))
	{
		ui->edit->centerCursor();
		return true;
	}

	QMessageBox::information(this, "Python Editor", QString("Cannot find: %1").arg(m_findText));
	return false;
}

void CPythonEditor::replaceCurrent()
{
	QTextCursor cursor = ui->edit->textCursor();
	if ((QString::compare(cursor.selectedText(), m_findText, Qt::CaseInsensitive) != 0) && !findText(true)) return;

	cursor = ui->edit->textCursor();
	cursor.insertText(m_replaceText);
	ui->edit->setTextCursor(cursor);
}

int CPythonEditor::replaceAll()
{
	if (m_findText.isEmpty()) return 0;

	QTextDocument* doc = ui->edit->document();
	QTextCursor editCursor(doc);
	QTextCursor matchCursor(doc);
	int count = 0;

	editCursor.beginEditBlock();
	while (true)
	{
		matchCursor = doc->find(m_findText, matchCursor);
		if (matchCursor.isNull()) break;
		matchCursor.insertText(m_replaceText);
		count++;
	}
	editCursor.endEditBlock();

	return count;
}

void CPythonEditor::goToLine(int line)
{
	QTextBlock block = ui->edit->document()->findBlockByNumber(line - 1);
	if (!block.isValid()) return;

	QTextCursor cursor(block);
	ui->edit->setTextCursor(cursor);
	ui->edit->centerCursor();
	ui->edit->setFocus();
}

void CPythonEditor::toggleCommentSelection()
{
	QTextCursor cursor = ui->edit->textCursor();
	QTextDocument* doc = ui->edit->document();
	int firstBlock = firstSelectedBlock(cursor);
	int lastBlock = lastSelectedBlock(cursor);

	bool uncomment = true;
	for (int i = firstBlock; i <= lastBlock; ++i)
	{
		QString text = doc->findBlockByNumber(i).text();
		if (text.trimmed().isEmpty()) continue;

		int indent = leadingWhitespaceLength(text);
		if ((indent >= text.length()) || (text[indent] != '#'))
		{
			uncomment = false;
			break;
		}
	}

	cursor.beginEditBlock();
	for (int i = lastBlock; i >= firstBlock; --i)
	{
		QTextBlock block = doc->findBlockByNumber(i);
		QString text = block.text();
		if (text.trimmed().isEmpty()) continue;

		int indent = leadingWhitespaceLength(text);
		QTextCursor lineCursor(block);
		lineCursor.setPosition(block.position() + indent);

		if (uncomment)
		{
			lineCursor.deleteChar();
			if (doc->characterAt(lineCursor.position()) == ' ') lineCursor.deleteChar();
		}
		else
		{
			lineCursor.insertText("# ");
		}
	}
	cursor.endEditBlock();
}

void CPythonEditor::indentSelection()
{
	QTextCursor cursor = ui->edit->textCursor();
	QTextDocument* doc = ui->edit->document();
	int firstBlock = firstSelectedBlock(cursor);
	int lastBlock = lastSelectedBlock(cursor);

	cursor.beginEditBlock();
	for (int i = lastBlock; i >= firstBlock; --i)
	{
		QTextBlock block = doc->findBlockByNumber(i);
		QTextCursor lineCursor(block);
		lineCursor.insertText(PYTHON_INDENT);
	}
	cursor.endEditBlock();
}

void CPythonEditor::unindentSelection()
{
	QTextCursor cursor = ui->edit->textCursor();
	QTextDocument* doc = ui->edit->document();
	int firstBlock = firstSelectedBlock(cursor);
	int lastBlock = lastSelectedBlock(cursor);

	cursor.beginEditBlock();
	for (int i = lastBlock; i >= firstBlock; --i)
	{
		QTextBlock block = doc->findBlockByNumber(i);
		QString text = block.text();
		if (text.isEmpty()) continue;

		int remove = 0;
		if (text.startsWith(PYTHON_INDENT))
		{
			remove = PYTHON_INDENT.length();
		}
		else if (text.startsWith('\t'))
		{
			remove = 1;
		}
		else
		{
			while ((remove < PYTHON_INDENT.length()) && (remove < text.length()) && (text[remove] == ' ')) remove++;
		}

		if (remove > 0)
		{
			QTextCursor lineCursor(block);
			lineCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, remove);
			lineCursor.removeSelectedText();
		}
	}
	cursor.endEditBlock();
}

void CPythonEditor::normalizeIndentation()
{
	QTextCursor cursor = ui->edit->textCursor();
	QTextDocument* doc = ui->edit->document();
	int firstBlock = cursor.hasSelection() ? firstSelectedBlock(cursor) : 0;
	int lastBlock = cursor.hasSelection() ? lastSelectedBlock(cursor) : doc->blockCount() - 1;

	cursor.beginEditBlock();
	for (int i = lastBlock; i >= firstBlock; --i)
	{
		QTextBlock block = doc->findBlockByNumber(i);
		QString text = block.text();
		QString normalizedText = normalizeLeadingIndentation(text);
		if (normalizedText == text) continue;

		QTextCursor lineCursor(block);
		lineCursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
		lineCursor.insertText(normalizedText);
	}
	cursor.endEditBlock();
}

void CPythonEditor::autoIndent()
{
	QTextCursor cursor = ui->edit->textCursor();
	QString text = cursor.block().text().left(cursor.positionInBlock());
	QString indent = text.left(leadingWhitespaceLength(text));
	if (text.trimmed().endsWith(":")) indent += PYTHON_INDENT;

	cursor.beginEditBlock();
	cursor.removeSelectedText();
	cursor.insertText("\n" + indent);
	cursor.endEditBlock();
	ui->edit->setTextCursor(cursor);
}

void CPythonEditor::on_python_finished(bool b)
{
	if (b == false) QMessageBox::critical(this, "Python", "An error occurred while running the Python script.");

	ui->actionRun->setEnabled(true);
	ui->actionStop->setEnabled(false);

	mainWnd->Update(this, true);
	double elapsedSeconds = m_runTimer.elapsed() / 1000.0;
	mainWnd->AddPythonLogEntry(QString(">>> python stopped (elapsed time: %1 seconds)\n").arg(elapsedSeconds, 0, 'f', 3));
}

void CPythonEditor::on_edit_textChanged()
{
	if (!ui->isModified)
	{
		ui->isModified = true;
		updateWindowTitle();
	}
}
