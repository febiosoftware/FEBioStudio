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
#include <PyLib/PythonThread.h>
#include <QMenuBar>
#include <QMenu>

class Ui::CPythonEditor
{
public:
	CTextEditor* edit = nullptr;
	CPyThread* pythread = nullptr;
	bool isModified = false;

public:
	void setup(QMainWindow* wnd, bool darkTheme)
	{
		edit = new CTextEditor(wnd);
		edit->setObjectName("edit");
		edit->useDarkTheme(darkTheme);

		QTextDocument* doc = new QTextDocument;
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
		wnd->setCentralWidget(edit);

		QAction* actionNew    = new QAction("New" , wnd); actionNew ->setObjectName("actionNew" ); actionNew ->setIcon(CIconProvider::GetIcon("new"));
		QAction* actionOpen   = new QAction("Open ...", wnd); actionOpen->setObjectName("actionOpen"); actionOpen->setIcon(CIconProvider::GetIcon("open"));
		QAction* actionSave   = new QAction("Save ...", wnd); actionSave->setObjectName("actionSave"); actionSave->setIcon(CIconProvider::GetIcon("save"));
		QAction* actionSaveAs = new QAction("Save as ...", wnd); actionSaveAs->setObjectName("actionSaveAs");
		QAction* actionClose  = new QAction("Close", wnd); actionClose->setObjectName("actionClose");
		QAction* actionRun    = new QAction("Run script" , wnd); actionRun ->setObjectName("actionRun" ); actionRun ->setIcon(CIconProvider::GetIcon("play"));

		QMenuBar* menuBar = wnd->menuBar();
		QMenu* menuFile = new QMenu("File", menuBar);
		menuBar->addAction(menuFile->menuAction());

		menuFile->addAction(actionNew);
		menuFile->addAction(actionOpen);
		menuFile->addAction(actionSave);
		menuFile->addAction(actionSaveAs);
		menuFile->addAction(actionClose);

		QMenu* menuPython = new QMenu("Python", menuBar);
		menuBar->addAction(menuPython->menuAction());
		menuPython->addAction(actionRun);

		QToolBar* mainToolBar = wnd->addToolBar("mainToolBar");
		mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
		mainToolBar->addAction(actionNew);
		mainToolBar->addAction(actionOpen);
		mainToolBar->addAction(actionSave);
		mainToolBar->addAction(actionRun);

		QMetaObject::connectSlotsByName(wnd);
	}
};
