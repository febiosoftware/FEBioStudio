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

#pragma once
#include <FEBioStudio/CommandPanel.h>
#include "PythonInputHandler.h"

namespace Ui {
	class CPythonToolsPanel;
}

namespace pybind11{
	class function;
}

class CMainWindow;
class CAbstractTool;
class CPythonTool;
class CPythonDummyTool;

class CPythonToolsPanel : public CCommandPanel
{
	Q_OBJECT

public:
	CPythonToolsPanel(CMainWindow* wnd, QWidget* parent = 0);
	~CPythonToolsPanel();

	// update the tools panel
	void Update(bool breset = true) override;

	CPythonDummyTool* addDummyTool(const char* name, pybind11::function func);

	void runScript(QString filename);

	CPythonInputHandler* getInputHandler();
	void addInputPage(QWidget* wgt);
	QWidget* getInputWgt();
	void removeInputPage();

private:
	void finalizeTools();
	CPythonTool* addTool(std::string name, pybind11::function func);

	void hideEvent(QHideEvent* event) override;
	void showEvent(QShowEvent* event) override;

private slots:
	void endThread();

	void on_buttons_idClicked(int id);
	void on_importScript_triggered();
	
private:
	Ui::CPythonToolsPanel*	ui;

	CPythonTool*			m_activeTool;
	QList<CPythonTool*>	tools;
	std::vector<CPythonDummyTool*> dummyTools;

	friend class Ui::CPythonToolsPanel;

	CPythonInputHandler inputHandler;
};
