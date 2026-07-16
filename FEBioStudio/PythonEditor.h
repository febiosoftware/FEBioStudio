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
#pragma once
#include <QElapsedTimer>
#include <QMainWindow>
#include <vector>

namespace Ui {
	class CPythonEditor;
}

class CMainWindow;
class QCloseEvent;

class CPythonEditor : public QMainWindow
{
	Q_OBJECT

public:
	CPythonEditor(CMainWindow* wnd);
	~CPythonEditor();

protected:
	void closeEvent(QCloseEvent* ev) override;
	bool eventFilter(QObject* obj, QEvent* ev) override;

private slots:
	void on_actionNew_triggered();
	void on_actionOpen_triggered();
	void on_actionSave_triggered();
	void on_actionSaveAs_triggered();
	void on_actionClose_triggered();
	void on_actionFind_triggered();
	void on_actionFindNext_triggered();
	void on_actionFindPrevious_triggered();
	void on_actionReplace_triggered();
	void on_actionReplaceAll_triggered();
	void on_actionGoToLine_triggered();
	void on_actionToggleComment_triggered();
	void on_actionIndent_triggered();
	void on_actionUnindent_triggered();
	void on_actionNormalizeIndentation_triggered();
	void on_actionDuplicateLine_triggered();
	void on_actionRun_triggered();
	void on_actionStop_triggered();
	void on_actionWordWrap_toggled(bool checked);
	void on_actionShowWhitespace_toggled(bool checked);
	void on_edit_textChanged();
	void on_python_finished(bool b);

signals:
	void runScript(QString script);

private:
	void updateWindowTitle();
	bool openScript(const QString& filePath);
	bool saveScript();
	bool saveScriptAs();
	bool saveModifiedScript();
	bool findText(bool forward);
	bool promptFindText();
	bool promptReplaceText();
	void replaceCurrent();
	int replaceAll();
	void goToLine(int line);
	void toggleCommentSelection();
	void indentSelection();
	void unindentSelection();
	void normalizeIndentation();
	void autoIndent();

private:
	Ui::CPythonEditor* ui;
	CMainWindow* mainWnd;
	QString fileName;
	QString m_findText;
	QString m_replaceText;
	QElapsedTimer m_runTimer;
};
