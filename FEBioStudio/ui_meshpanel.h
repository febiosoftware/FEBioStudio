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
#include <FSCore/ClassDescriptor.h>
#include "PropertyListForm.h"
#include "PropertyList.h"
#include "ObjectPanel.h"
#include "MeshButtonBox.h"
#include "ObjectProps.h"

enum MeshEditButtonFlags
{
	EDIT_MESH      = 0x01,
	EDIT_ELEMENT   = 0x02,
	EDIT_FACE      = 0x04,
	EDIT_EDGE      = 0x08,
	EDIT_NODE      = 0x10,
	EDIT_SAFE      = 0x20	// flag for modifiers that can safely be applied to primitives (i.e. won't affect geometry or partitioning)
};

class Ui::CMeshPanel
{
	// make sure these values correspond to the order of the panels
	enum {
		OBJECT_PANEL,		// object info panel
		EDITMESH_PANEL,		// modifiers for editables meshes
		PARAMS_PANEL,		// mesher parameters
		EDITMESH_PANEL2,	// modifiers for non-editable meshes
		PARAMS_PANEL2		// modifier parameters
	};

public:
	CObjectPanel*	obj;
	::CMeshButtonBox*	buttons;		// buttons for editables meshes
	::CMeshButtonBox*	buttons2;		// buttons for editing mesh of primitives
	CPropertyListForm*	form;
	CPropertyListForm*	form2;
	CToolBox* tool;

//	CPropertyList* m_pl;

public:
	void setupUi(QWidget* parent, ::CMainWindow* mainWindow)
	{
//		m_pl = 0;

		obj = new CObjectPanel(mainWindow); obj->setObjectName("objectPanel");

		// parameters for mesh
		QWidget* pw = new QWidget;
		QVBoxLayout* pl = new QVBoxLayout;
		QPushButton* applyButton = new QPushButton("Apply");
		applyButton->setObjectName("apply");
		pl->addWidget(form = new CPropertyListForm); form->setBackgroundRole(QPalette::Light);
		pl->addWidget(applyButton);
		pw->setLayout(pl);

		// parameters for modifiers
		QWidget* pw2 = new QWidget;
		QVBoxLayout* pl2 = new QVBoxLayout;
		QPushButton* applyButton2 = new QPushButton("Apply");
		applyButton2->setObjectName("apply2");
		pl2->addWidget(form2 = new CPropertyListForm); form2->setBackgroundRole(QPalette::Light);
		pl2->addWidget(applyButton2);
		pw2->setLayout(pl2);

		// put it all together
		tool = new CToolBox;
		tool->addTool("Object", obj);
		tool->addTool("Edit Mesh"      , buttons = new ::CMeshButtonBox(CLASS_FEMODIFIER));
		tool->addTool("Mesh Parameters", pw);
		tool->addTool("Edit Mesh"      , buttons2 = new ::CMeshButtonBox(CLASS_FEMODIFIER, EDIT_SAFE));
		tool->addTool("Parameters"     , pw2);

		showMesherParametersPanel(false);
		showButtonsPanel(false);
		showButtonsPanel2(false);
		showModifierParametersPanel(false);

		buttons->setObjectName("buttons");
		form->setObjectName("form");

		buttons2->setObjectName("buttons2");
		form2->setObjectName("form2");

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->setContentsMargins(0,0,0,0);
		mainLayout->addWidget(tool);
		parent->setLayout(mainLayout);

		QMetaObject::connectSlotsByName(parent);
	}

	void setMesherPropertyList(CPropertyList* pl)
	{
		CPropertyList* plold = form->getPropertyList();
		form->setPropertyList(0);
		if (plold) delete plold;
		if (pl) form->setPropertyList(pl);
	}

	void setActiveModifier(FEModifier* mod)
	{
		CPropertyList* pl = new CObjectProps(mod);
		setModifierPropertyList(pl);
	}

	void setModifierPropertyList(CPropertyList* pl)
	{
		CPropertyList* plold = form2->getPropertyList();
		form2->setPropertyList(0);
		if (plold) delete plold;
		if (pl) form2->setPropertyList(pl);
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
