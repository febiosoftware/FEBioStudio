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
#include "CommandPanel.h"

namespace Ui {
	class CToolsPanel;
}

class CMainWindow;
class CAbstractTool;

class CToolsPanel : public CCommandPanel
{
	Q_OBJECT

public:
	CToolsPanel(CMainWindow* wnd, QWidget* parent = 0);

	// update the tools panel
	void Update(bool breset = true) override;

private:
	void initTools();

	void hideEvent(QHideEvent* event) override;
	void showEvent(QShowEvent* event) override;

private slots:
	void on_buttons_idClicked(int id);

private:
	Ui::CToolsPanel*	ui;

	CAbstractTool*			m_activeTool;
	QList<CAbstractTool*>	tools;

	friend class Ui::CToolsPanel;
};
