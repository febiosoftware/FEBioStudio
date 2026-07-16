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
#include "PythonEditor.h"
#include "TextEditor.h"
#include <QToolBar>
#include <QMessageBox>
#include <QFileDialog>
#include "IconProvider.h"
#include <QMenuBar>
#include <QMenu>
#include <QLineEdit>
#include <QKeySequence>

class Ui::CPythonEditor
{
public:
	CTextEditor* edit = nullptr;
	bool isModified = false;
	QAction* actionFind = nullptr;
	QAction* actionFindNext = nullptr;
	QAction* actionFindPrevious = nullptr;
	QAction* actionReplace = nullptr;
	QAction* actionReplaceAll = nullptr;
	QAction* actionGoToLine = nullptr;
	QAction* actionToggleComment = nullptr;
	QAction* actionIndent = nullptr;
	QAction* actionUnindent = nullptr;
	QAction* actionNormalizeIndentation = nullptr;
	QAction* actionDuplicateLine = nullptr;
	QAction* actionMoveLineUp = nullptr;
	QAction* actionMoveLineDown = nullptr;
	QAction* actionRun = nullptr;
	QAction* actionStop = nullptr;
	QAction* actionWordWrap = nullptr;
	QAction* actionShowWhitespace = nullptr;
	QLineEdit* editCwd = nullptr;

public:
	void setup(QMainWindow* wnd, bool darkTheme)
	{
		edit = new CTextEditor(wnd);
		edit->setObjectName("edit");
		edit->useDarkTheme(darkTheme);

		QTextDocument* doc = new QTextDocument(edit);
		doc->setDocumentLayout(new QPlainTextDocumentLayout(doc));

		QFont font;
		font.setFamily("Consolas");
		font.setPointSize(14);
		font.setWeight(QFont::Medium);
		font.setFixedPitch(true);
		doc->setDefaultFont(font);

		QTextOption ops = doc->defaultTextOption();
		QFontInfo fi(font);
		ops.setTabStopDistance(2 * fi.pixelSize());
		doc->setDefaultTextOption(ops);

		edit->SetDocument(doc, CTextEditor::PYTHON);
		edit->setLineWrapMode(QPlainTextEdit::NoWrap);
		wnd->setCentralWidget(edit);

		QAction* actionNew    = new QAction("New" , wnd); actionNew ->setObjectName("actionNew" ); actionNew ->setIcon(CIconProvider::GetIcon("new"));
		QAction* actionOpen   = new QAction("Open ...", wnd); actionOpen->setObjectName("actionOpen"); actionOpen->setIcon(CIconProvider::GetIcon("open"));
		QAction* actionSave   = new QAction("Save ...", wnd); actionSave->setObjectName("actionSave"); actionSave->setIcon(CIconProvider::GetIcon("save")); actionSave->setShortcut(Qt::Key_S | Qt::ControlModifier);
		QAction* actionSaveAs = new QAction("Save as ...", wnd); actionSaveAs->setObjectName("actionSaveAs");
		QAction* actionClose  = new QAction("Close", wnd); actionClose->setObjectName("actionClose");
		actionFind = new QAction("Find ...", wnd); actionFind->setObjectName("actionFind"); actionFind->setShortcut(QKeySequence::Find);
		actionFindNext = new QAction("Find Next", wnd); actionFindNext->setObjectName("actionFindNext"); actionFindNext->setShortcut(QKeySequence::FindNext);
		actionFindPrevious = new QAction("Find Previous", wnd); actionFindPrevious->setObjectName("actionFindPrevious"); actionFindPrevious->setShortcut(QKeySequence::FindPrevious);
		actionReplace = new QAction("Replace ...", wnd); actionReplace->setObjectName("actionReplace"); actionReplace->setShortcut(QKeySequence::Replace);
		actionReplaceAll = new QAction("Replace All ...", wnd); actionReplaceAll->setObjectName("actionReplaceAll");
		actionGoToLine = new QAction("Go to Line ...", wnd); actionGoToLine->setObjectName("actionGoToLine"); actionGoToLine->setShortcut(Qt::CTRL | Qt::Key_L);
		actionToggleComment = new QAction("Comment/Uncomment Selection", wnd); actionToggleComment->setObjectName("actionToggleComment"); actionToggleComment->setShortcut(Qt::CTRL | Qt::Key_Slash);
		actionIndent = new QAction("Indent Selection", wnd); actionIndent->setObjectName("actionIndent"); actionIndent->setShortcut(Qt::CTRL | Qt::Key_BracketRight);
		actionUnindent = new QAction("Unindent Selection", wnd); actionUnindent->setObjectName("actionUnindent"); actionUnindent->setShortcut(Qt::CTRL | Qt::Key_BracketLeft);
		actionNormalizeIndentation = new QAction("Normalize Indentation", wnd); actionNormalizeIndentation->setObjectName("actionNormalizeIndentation");
		actionDuplicateLine = new QAction("Duplicate Line", wnd); actionDuplicateLine->setObjectName("actionDuplicateLine"); actionDuplicateLine->setShortcut(Qt::CTRL| Qt::Key_D);
		actionMoveLineUp = new QAction("Move Line Up", wnd); actionMoveLineUp->setObjectName("actionMoveLineUp"); actionMoveLineUp->setShortcut(Qt::ALT | Qt::Key_Up);
		actionMoveLineDown = new QAction("Move Line Down", wnd); actionMoveLineDown->setObjectName("actionMoveLineDown"); actionMoveLineDown->setShortcut(Qt::ALT | Qt::Key_Down);
		actionRun  = new QAction("Run script" , wnd); actionRun ->setObjectName("actionRun" ); actionRun ->setIcon(CIconProvider::GetIcon("play"));
		actionStop = new QAction("Stop script" , wnd); actionStop->setObjectName("actionStop" ); actionStop->setIcon(CIconProvider::GetIcon("stop"));
		actionWordWrap = new QAction("Word Wrap", wnd); actionWordWrap->setObjectName("actionWordWrap");
		actionWordWrap->setShortcut(Qt::ALT | Qt::Key_Z);
		actionWordWrap->setCheckable(true);
		actionWordWrap->setChecked(edit->lineWrapMode() != QPlainTextEdit::NoWrap);
		actionShowWhitespace = new QAction("Show Whitespace", wnd); actionShowWhitespace->setObjectName("actionShowWhitespace");
		actionShowWhitespace->setShortcut(Qt::ALT | Qt::Key_W);
		actionShowWhitespace->setCheckable(true);

		QList<QKeySequence> runShortcuts;
		runShortcuts << QKeySequence(Qt::Key_F5) << QKeySequence(Qt::CTRL | Qt::Key_R);
		actionRun->setShortcuts(runShortcuts);
		actionStop->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F5));

		actionStop->setEnabled(false);

		QMenuBar* menuBar = wnd->menuBar();
		QMenu* menuFile = new QMenu("File", menuBar);
		menuBar->addAction(menuFile->menuAction());

		menuFile->addAction(actionNew);
		menuFile->addAction(actionOpen);
		menuFile->addAction(actionSave);
		menuFile->addAction(actionSaveAs);
		menuFile->addAction(actionClose);

		QMenu* menuEdit = new QMenu("Edit", menuBar);
		menuBar->addAction(menuEdit->menuAction());
		menuEdit->addAction(actionFind);
		menuEdit->addAction(actionFindNext);
		menuEdit->addAction(actionFindPrevious);
		menuEdit->addSeparator();
		menuEdit->addAction(actionReplace);
		menuEdit->addAction(actionReplaceAll);
		menuEdit->addSeparator();
		menuEdit->addAction(actionGoToLine);
		menuEdit->addSeparator();
		menuEdit->addAction(actionToggleComment);
		menuEdit->addAction(actionIndent);
		menuEdit->addAction(actionUnindent);
		menuEdit->addAction(actionNormalizeIndentation);
		menuEdit->addSeparator();
		menuEdit->addAction(actionDuplicateLine);
		menuEdit->addAction(actionMoveLineUp);
		menuEdit->addAction(actionMoveLineDown);

		QMenu* menuView = new QMenu("View", menuBar);
		menuBar->addAction(menuView->menuAction());
		menuView->addAction(actionWordWrap);
		menuView->addAction(actionShowWhitespace);

		QMenu* menuPython = new QMenu("Python", menuBar);
		menuBar->addAction(menuPython->menuAction());
		menuPython->addAction(actionRun);
		menuPython->addAction(actionStop);

		QToolBar* mainToolBar = wnd->addToolBar("mainToolBar");
		mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
		mainToolBar->addAction(actionNew);
		mainToolBar->addAction(actionOpen);
		mainToolBar->addAction(actionSave);
		mainToolBar->addAction(actionRun);
		mainToolBar->addAction(actionStop);
		mainToolBar->addWidget(editCwd = new QLineEdit);
		editCwd->setPlaceholderText("set working directory");

		QMetaObject::connectSlotsByName(wnd);
	}
};
