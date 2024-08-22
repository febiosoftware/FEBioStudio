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
#include "MeshPanel.h"
#include "ToolBox.h"
#include <QLineEdit>
#include <QLabel>
#include <QBoxLayout>
#include <QFormLayout>
#include <QToolButton>
#include <QGroupBox>
#include <QCheckBox>
#include <QGridLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QMenu>
#include <QValidator>
#include <QStackedWidget>
#include <FSCore/ClassDescriptor.h>
#include "PropertyListForm.h"
#include "PropertyList.h"
#include "ObjectPanel.h"
#include "MeshButtonBox.h"
#include "ObjectProps.h"
#include "Tool.h"
#include "ButtonBox.h"
#include "ToolParamsPanel.h"
#include <GLLib/GDecoration.h>

enum MeshEditButtonFlags
{
	EDIT_MESH      = 0x01,
	EDIT_ELEMENT   = 0x02,
	EDIT_FACE      = 0x04,
	EDIT_EDGE      = 0x08,
	EDIT_NODE      = 0x10,
	EDIT_SAFE      = 0x20	// flag for modifiers that can safely be applied to primitives (i.e. won't affect geometry or partitioning)
};

class CMesherParamsPanel : public QWidget
{
public:
	CMesherParamsPanel(QWidget* parent = nullptr) : QWidget(parent)
	{
		QVBoxLayout* pl = new QVBoxLayout;
		QPushButton* applyButton = new QPushButton("Apply");
		applyButton->setObjectName("apply");
		pl->addWidget(form = new CPropertyListForm); form->setBackgroundRole(QPalette::Light);
		form->setObjectName("form2");
		pl->addWidget(applyButton);
		setLayout(pl);
	}

	void SetPropertyList(CPropertyList* pl)
	{
		CPropertyList* plold = form->getPropertyList();
		form->setPropertyList(nullptr);
		if (plold) delete plold;
		if (pl) form->setPropertyList(pl);
	}

	CPropertyList* GetPropertyList() { return form->getPropertyList(); }

private:
	CPropertyListForm* form;
};


class ModifierTool : public CAbstractTool
{
public:
	ModifierTool(CMainWindow* wnd, ClassDescriptor* cd) : CAbstractTool(wnd, cd->GetName()), m_cd(cd)
	{
		m_mod = nullptr;
		ui = nullptr;
	}

	void Activate() override
	{
		CAbstractTool::Activate();

		if (m_cd) {
			m_mod = dynamic_cast<FEModifier*>(m_cd->Create());
			assert(m_mod);

			CPropertyList* pl = new CObjectProps(m_mod);
			ui->setPropertyList(pl);
		}
	}

	void Deactivate() override
	{
		if (ui) ui->setPropertyList(nullptr);
		if (m_mod) delete m_mod;
		m_mod = nullptr;
		CAbstractTool::Deactivate();
	}

	void updateUi() override
	{
		if (ui) ui->updateData();
		CAbstractTool::updateUi();
	}

	QWidget* createUi() override
	{
		ui = new CPropertyListForm();
		ui->setBackgroundRole(QPalette::Light);
		return ui;
	}

	FEModifier* GetModifier() { return m_mod; }

	unsigned int flags() const { return (m_cd ? m_cd->Flag() : 0); }

private:
	ClassDescriptor* m_cd;
	FEModifier* m_mod;
	CPropertyListForm* ui;
};

class CAddTriangleTool : public ModifierTool
{
public:
	CAddTriangleTool(CMainWindow* wnd, ClassDescriptor* cd) : ModifierTool(wnd, cd) { m_pick = 0; }

	bool onPickEvent(const FESelection& sel)
	{
		const FENodeSelection* nodeSel = dynamic_cast<const FENodeSelection*>(&sel);
		if (nodeSel && (nodeSel->Count() == 1))
		{
			int nid = nodeSel->NodeIndex(0);
			FEAddTriangle* mod = dynamic_cast<FEAddTriangle*>(GetModifier());
			if (mod)
			{
				const FSLineMesh* mesh = nodeSel->GetMesh();
				points.push_back(to_vec3f(mesh->NodePosition(nid)));
				mod->SetIntValue(m_pick, nid + 1);
				for (int i = m_pick + 1; i < 3; ++i) mod->SetIntValue(i, 0);

				m_pick++;
				if (m_pick >= 3)
				{
					mod->push_stack();
					m_pick = 0;
				}

				BuildDecoration();

				return true;
			}
		}
		return false;
	}

	void BuildDecoration()
	{
		GCompositeDecoration* deco = new GCompositeDecoration;
		int n = (int)points.size();
		int nf = n / 3;
		for (int i = 0; i < nf; i++)
		{
			GDecoration* di = new GTriangleDecoration(points[3 * i], points[3 * i + 1], points[3 * i + 2]);
			if (i < nf - 1) di->setColor(GLColor(200, 200, 0));
			deco->AddDecoration(di);
		}
		if ((n % 3) == 2)
		{
			deco->AddDecoration(new GLineDecoration(points[n - 2], points[n - 1]));
		}
		else if ((n % 3) == 1)
		{
			deco->AddDecoration(new GPointDecoration(points[n - 1]));
		}
		SetDecoration(deco);
	}

	bool onUndoEvent() override
	{
		FEAddTriangle* mod = dynamic_cast<FEAddTriangle*>(GetModifier());
		if (mod)
		{
			if (points.size() > 0) points.pop_back();
			else return false;

			m_pick--;
			if (m_pick < 0)
			{
				m_pick = 2;
				mod->pop_stack();
			}
			BuildDecoration();
			return false; // fall through so selection is also undone
		}
		else return false;
	}

	void Reset() override
	{
		m_pick = 0;
		points.clear();
		FEAddTriangle* mod = dynamic_cast<FEAddTriangle*>(GetModifier());
		if (mod)
		{
			mod->SetIntValue(0, 0);
			mod->SetIntValue(1, 0);
			mod->SetIntValue(2, 0);
		}
		CAbstractTool::Reset();
	}

private:
	int m_pick;
	std::vector<vec3f> points;
};

class Ui::CMeshPanel
{
	// make sure these values correspond to the order of the panels
	enum {
		OBJECT_PANEL,		// object info panel
		PARAMS_PANEL,		// mesher parameters
		EDITMESH_PANEL,		// modifiers for editables meshes
		EDITMESH_PANEL2,	// modifiers for non-editable meshes
		PARAMS_PANEL2		// modifier parameters
	};

public:
	::CMainWindow* m_wnd;

	CToolBox*			tool;
	CObjectPanel*		obj;
	CMesherParamsPanel*	mesherParams;
	CToolParamsPanel*	modParams;

	CAbstractTool* m_activeTool = nullptr;
	QList<CAbstractTool*>	tools;

	GObject* m_currentObject = nullptr;

public:
	void setupUi(::CMeshPanel* parent, ::CMainWindow* mainWindow)
	{
		m_wnd = mainWindow;

		obj = new CObjectPanel(mainWindow); obj->setObjectName("objectPanel");

		// parameters for meshers
		mesherParams = new CMesherParamsPanel;

		// UIs for tools
		initTools();
		modParams = new CToolParamsPanel;
		modParams->setObjectName("modParams");
		int n = 1;
		for (CAbstractTool* tool : tools)
		{
			tool->SetID(n++);
			modParams->AddTool(tool);
		}

		// buttons for mesh modifiers
		CButtonBox* box = new CButtonBox("buttons");
		for (CAbstractTool* tool : tools)
		{
			ModifierTool* modTool = dynamic_cast<ModifierTool*>(tool);
			if (modTool)
			{
				box->AddButton(modTool->name(), tool->GetID());
			}
		}

		// buttons for safe mesh modifiers
		CButtonBox* box2 = new CButtonBox("buttons2");
		for (CAbstractTool* tool : tools)
		{
			ModifierTool* modTool = dynamic_cast<ModifierTool*>(tool);
			if (modTool && (modTool->flags() & EDIT_SAFE))
			{
				box2->AddButton(modTool->name(), tool->GetID());
			}
		}

		// put it all together
		tool = new CToolBox;
		tool->addTool("Object", obj);
		tool->addTool("Mesh Parameters", mesherParams);
		tool->addTool("Edit Mesh"      , box);
		tool->addTool("Edit Mesh"      , box2);
		tool->addTool("Parameters"     , modParams);

		showMesherParametersPanel(false);
		showButtonsPanel(false);
		showButtonsPanel2(false);
		showModifierParametersPanel(false);

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->setContentsMargins(0,0,0,0);
		mainLayout->addWidget(tool);
		parent->setLayout(mainLayout);

		QMetaObject::connectSlotsByName(parent);
	}

	void initTools()
	{
		for (Class_Iterator it = ClassKernel::FirstCD(); it != ClassKernel::LastCD(); ++it)
		{
			ClassDescriptor* pcd = *it;
			if (pcd->GetType() == CLASS_FEMODIFIER)
			{
				// TODO: Find a better way
				if (strcmp(pcd->GetName(), "Add Triangle") == 0)
				{
					tools.push_back(new CAddTriangleTool(m_wnd, pcd));
				}
				else tools.push_back(new ModifierTool(m_wnd, pcd));
			}
		}
	}

	void activateTool(int id)
	{
		// deactivate the active tool
		if (m_activeTool) m_activeTool->Deactivate();
		m_activeTool = nullptr;

		if (id == -1)
		{
			showModifierParametersPanel(false);
			return;
		}

		// find the tool
		for (CAbstractTool* tool : tools)
		{
			if (tool->GetID() == id)
			{
				m_activeTool = tool;
				break;
			}
		}

		// activate the tool
		if (m_activeTool)
		{
			m_activeTool->Activate();
			modParams->setCurrentIndex(id);
			showModifierParametersPanel(true);
		}
		else
		{
			modParams->setCurrentIndex(0);
			showModifierParametersPanel(false);
		}
	}

	void setMesherPropertyList(CPropertyList* pl)
	{
		mesherParams->SetPropertyList(pl);
	}

	void showMesherParametersPanel(bool b)
	{
		tool->getToolItem(PARAMS_PANEL)->setVisible(b);
	}

	void showButtonsPanel(bool b)
	{
		tool->getToolItem(EDITMESH_PANEL)->setVisible(b);
	}

	void showButtonsPanel2(bool b)
	{
		tool->getToolItem(EDITMESH_PANEL2)->setVisible(b);
	}

	void showModifierParametersPanel(bool b)
	{
		tool->getToolItem(PARAMS_PANEL2)->setVisible(b);
	}

	void hideAllPanels()
	{
		tool->getToolItem(EDITMESH_PANEL)->setVisible(false);
		tool->getToolItem(PARAMS_PANEL)->setVisible(false);
		tool->getToolItem(EDITMESH_PANEL2)->setVisible(false);
		tool->getToolItem(PARAMS_PANEL2)->setVisible(false);
	}
};
