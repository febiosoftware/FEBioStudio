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
#include <vector>
#include "DlgStartThread.h"

//using namespace std;
class CMainWindow;
class CModelDocument;
class FEModifier;
class FSMesh;
class GObject;
class FEMesher;
class FSGroup;
class FESelection;
class CAbstractTool;

namespace Ui {
	class CMeshPanel;
};

class CMeshPanel : public CCommandPanel
{
	Q_OBJECT

public:
	CMeshPanel(CMainWindow* wnd, QWidget* parent = 0);

	// update mesh panel
	void Update(bool breset = true) override;

	void Apply() override;

	bool OnPickEvent(const FESelection& sel) override;

private:
	void initTools();
	void activateTool(int id);

private slots:
	void on_buttons2_buttonSelected(int n);
	void on_buttons_idClicked(int id);
	void on_apply_clicked(bool b);
	void on_apply2_clicked(bool b);
	void on_menu_triggered(QAction* pa);

private:
	CAbstractTool*			m_activeTool;
	QList<CAbstractTool*>	tools;

	GObject*			m_currentObject;
	Ui::CMeshPanel*		ui;

	friend class Ui::CMeshPanel;
};

class MeshingThread : public CustomThread
{
public:
	MeshingThread(GObject* po);

	void run() Q_DECL_OVERRIDE;

public:
	bool hasProgress() override;

	double progress() override;

	const char* currentTask() override;

	void stop() override;

private:
	GObject*	m_po;
	FEMesher*	m_mesher;
};

class ModifierThread : public CustomThread
{
public:
	ModifierThread(CModelDocument* doc, FEModifier* mod, GObject* po, FESelection* sel);

	void run() Q_DECL_OVERRIDE;

public:
	bool hasProgress() override;

	double progress() override;

	const char* currentTask() override;

	void stop() override;

private:
	CModelDocument*	m_doc;
	GObject*	m_po;
	FEModifier*	m_mod;
	FESelection*	m_sel;
};
