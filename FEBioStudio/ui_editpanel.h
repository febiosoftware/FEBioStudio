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
#include "EditPanel.h"
#include <QLineEdit>
#include <QBoxLayout>
#include <QPushButton>
#include <QAction>
#include <QFormLayout>
#include "PropertyListForm.h"
#include "ToolBox.h"
#include "PropertyList.h"
#include "ObjectPanel.h"
#include "MeshButtonBox.h"
#include <MeshTools/FEModifier.h>
#include <MeshTools/FECurveIntersect.h>
#include <MeshLib/FSSurfaceMesh.h>
#include "MainWindow.h"
#include "Document.h"
#include <CUILib/InputWidgets.h>
#include "Tool.h"
#include "ButtonBox.h"
#include "ToolParamsPanel.h"
#include "ModelDocument.h"
#include "CurveIntersectProps.h"
#include "ObjectProps.h"

class CPartitionProps : public CDataPropertyList
{
public:
	CPartitionProps(FESurfacePartitionSelection* mod) : m_mod(mod)
	{
		m_createNew = true;
		m_partition = 0;

		addBoolProperty(&m_createNew, "Create New");
		addIntProperty(&m_partition, "Partition");

		if (m_createNew) mod->assignToPartition(-1);
	}

	void SetPropertyValue(int i, const QVariant& value)
	{
		switch (i)
		{
		case 0: m_createNew = value.toBool(); break;
		case 1: m_partition = value.toInt(); break;
		}

		if (m_createNew) m_mod->assignToPartition(-1);
		else m_mod->assignToPartition(m_partition);
	}

	QVariant GetPropertyValue(int i)
	{
		switch (i)
		{
		case 0: return m_createNew; break;
		case 1: return m_partition; break;
		}

		return QVariant();
	}


public:
	bool	m_createNew;
	int		m_partition;
	FESurfacePartitionSelection* m_mod;
};

class CEditParamsPanel : public QWidget
{
public:
	CEditParamsPanel(QWidget* parent = nullptr) : QWidget(parent)
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

	void updateData() { form->updateData(); }

private:
	CPropertyListForm* form;
};

class CEditPositionWidget : public QWidget
{
public:
	CEditPositionWidget(QWidget* parent = nullptr) : QWidget(parent)
	{
		QFormLayout* posForm = new QFormLayout;
		QHBoxLayout* ph = nullptr;
		ph = new QHBoxLayout; ph->addWidget(r[0] = new CFloatInput); ph->addStretch(); ph->setContentsMargins(0, 0, 0, 0); posForm->addRow("X:", ph);
		ph = new QHBoxLayout; ph->addWidget(r[1] = new CFloatInput); ph->addStretch(); ph->setContentsMargins(0, 0, 0, 0); posForm->addRow("Y:", ph);
		ph = new QHBoxLayout; ph->addWidget(r[2] = new CFloatInput); ph->addStretch(); ph->setContentsMargins(0, 0, 0, 0); posForm->addRow("Z:", ph);
		setLayout(posForm);

		r[0]->setObjectName("posX");
		r[1]->setObjectName("posY");
		r[2]->setObjectName("posZ");
	}

	void SetObjectPosition(const vec3d& p)
	{
		r[0]->setValue(p.x);
		r[1]->setValue(p.y);
		r[2]->setValue(p.z);
	}

	vec3d objectPosition()
	{
		vec3d p;
		p.x = r[0]->value();
		p.y = r[1]->value();
		p.z = r[2]->value();
		return p;
	}

public:
	CFloatInput* r[3];
};

class SurfaceModifierTool : public CAbstractTool
{
public:
	SurfaceModifierTool(CMainWindow* wnd, ClassDescriptor* cd) : CAbstractTool(wnd, cd->GetName()), m_cd(cd)
	{
		m_mod = nullptr;
		ui = nullptr;
	}

	void Activate() override
	{
		CAbstractTool::Activate();

		if (m_cd) {
			m_mod = dynamic_cast<FESurfaceModifier*>(m_cd->Create());
			assert(m_mod);

			CMainWindow* wnd = GetMainWindow();
			CModelDocument* doc = wnd->GetModelDocument();
			GModel* geo = &doc->GetFSModel()->GetModel();

			CPropertyList* pl = 0;
			if      (dynamic_cast<FECurveIntersect*>(m_mod)) pl = new CCurveIntersectProps(geo, dynamic_cast<FECurveIntersect*>(m_mod));
			else if (dynamic_cast<FESurfacePartitionSelection*>(m_mod)) pl = new CPartitionProps(dynamic_cast<FESurfacePartitionSelection*>(m_mod));
			else pl = new CObjectProps(m_mod);

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

	FESurfaceModifier* GetModifier() { return m_mod; }

	unsigned int flags() const { return (m_cd ? m_cd->Flag() : 0); }

private:
	CModelDocument* m_doc;
	ClassDescriptor* m_cd;
	FESurfaceModifier* m_mod;
	CPropertyListForm* ui;
};

class Ui::CEditPanel
{
	// make sure these values correspond to the order of the panel
	enum {
		OBJECT_PANEL,
		POSITION_PANEL,
		OBJ_PARAMS_PANEL,
		EDITMESH_PANEL,
		MOD_PARAMS_PANEL
	};

	enum {
		EMPTY_LAYOUT,

	};

public:

	CToolBox* tool;
	CObjectPanel* obj;
	CEditPositionWidget* pos;
	CEditParamsPanel* editParams;
	CToolParamsPanel* modParams;

	CAbstractTool* m_activeTool = nullptr;
	QList<CAbstractTool*>	tools;

	GObject* m_currentObject = nullptr;

	::CMainWindow* m_wnd;

public:
	void setupUi(QWidget* parent, ::CMainWindow* mainWindow)
	{
		m_wnd = mainWindow;
		obj = new CObjectPanel(mainWindow); obj->setObjectName("objectPanel");

		editParams = new CEditParamsPanel(mainWindow); 

		pos = new CEditPositionWidget;

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
			SurfaceModifierTool* modTool = dynamic_cast<SurfaceModifierTool*>(tool);
			if (modTool)
			{
				box->AddButton(modTool->name(), tool->GetID());
			}
		}

		tool = new CToolBox;
		tool->addTool("Object", obj);
		tool->addTool("Position", pos);
		tool->addTool("Parameters", editParams);
		tool->addTool("Edit Surface Mesh", box);
		tool->addTool("Parameters", modParams);

		showPositionPanel(false);
		showObjectParametersPanel(false);
		showButtonsPanel(false);
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
			if (pcd->GetType() == CLASS_SURFACE_MODIFIER)
			{
				tools.push_back(new SurfaceModifierTool(m_wnd, pcd));
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

	void ShowObjectInfoPanel(bool b)
	{
		tool->getToolItem(OBJECT_PANEL)->setVisible(b);
	}

	void showPositionPanel(bool b)
	{
		tool->getToolItem(POSITION_PANEL)->setVisible(b);
	}

	void showObjectParametersPanel(bool b)
	{
		tool->getToolItem(OBJ_PARAMS_PANEL)->setVisible(b);
	}

	void showButtonsPanel(bool b)
	{
		tool->getToolItem(EDITMESH_PANEL)->setVisible(b);
	}

	void showModifierParametersPanel(bool b)
	{
		tool->getToolItem(MOD_PARAMS_PANEL)->setVisible(b);
	}
};
