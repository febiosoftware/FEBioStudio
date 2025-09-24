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
#include "WindowPanel.h"
#include "DlgStartThread.h"

class CMainWindow;

namespace Ui {
	class CEditPanel;
}

class CMainWindow;
class CModelDocument;
class FESurfaceModifier;
class FSMesh;
class GSurfaceMeshObject;
class FEMesher;
class FSGroup;
class FSSurfaceMesh;

class SurfaceModifierThread : public CustomThread
{
public:
	SurfaceModifierThread(FESurfaceModifier* mod, GSurfaceMeshObject* po, FSGroup* pg);

	void run() Q_DECL_OVERRIDE;

public:
	bool hasProgress() override;

	double progress() override;

	const char* currentTask() override;

	void stop() override;

	FSSurfaceMesh* newMesh() { return m_newMesh; }

private:
	CModelDocument*		m_doc;
	GSurfaceMeshObject*	m_po;
	FESurfaceModifier*	m_mod;
	FSGroup*			m_pg;
	FSSurfaceMesh*		m_newMesh;
};


class CEditPanel : public CWindowPanel
{
	Q_OBJECT

public:
	CEditPanel(CMainWindow* wnd, QWidget* parent = 0);

	// update mesh panel
	void Update(bool breset = true) override;

	void Apply() override;

private slots:
	void on_apply_clicked(bool b);
	void on_menu_triggered(QAction* pa);
	void on_buttons_idClicked(int id);
	void on_posX_editingFinished();
	void on_posY_editingFinished();
	void on_posZ_editingFinished();
	void on_modParams_apply();
	void on_form_dataChanged(bool itemModified, int index);

private:
	void updateObjectPosition();

private:
	Ui::CEditPanel*	ui;
};
