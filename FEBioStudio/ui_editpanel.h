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
#include <MeshLib/FESurfaceMesh.h>
#include "MainWindow.h"
#include "Document.h"
#include "InputWidgets.h"

class Ui::CEditPanel
{
	// make sure these values correspond to the order of the panel
	enum {
		OBJECT_PANEL,
		POSITION_PANEL,
		EDITMESH_PANEL,
		PARAMS_PANEL
	};

	enum {
		EMPTY_LAYOUT,

	};

public:
	CObjectPanel* obj;
	CPropertyListForm*	form;
	::CMeshButtonBox*	buttons;
	CToolBox* tool;

	CPropertyList* m_pl;

	CFloatInput* r[3];

	int						m_nid;	// current button selected
	FESurfaceModifier*		m_mod;	// temporary modifier

	GObject*	m_currenObject;

public:
	void setupUi(QWidget* parent, ::CMainWindow* mainWindow)
	{
		m_pl = 0;
		m_mod = 0;
		m_currenObject = 0;

		obj = new CObjectPanel(mainWindow); obj->setObjectName("objectPanel");

		QWidget* pw = new QWidget;
		QVBoxLayout* pl = new QVBoxLayout;
		QPushButton* applyButton = new QPushButton("Apply");
		applyButton->setObjectName("apply");
		pl->addWidget(form = new CPropertyListForm);
		form->setObjectName("form");
		pl->addWidget(applyButton);
		pw->setLayout(pl);

		QWidget* pos = new QWidget;
		QFormLayout* posForm = new QFormLayout;
		QHBoxLayout* ph = nullptr;
		ph = new QHBoxLayout; ph->addWidget(r[0] = new CFloatInput); ph->addStretch(); ph->setContentsMargins(0, 0, 0, 0); posForm->addRow("X:", ph);
		ph = new QHBoxLayout; ph->addWidget(r[1] = new CFloatInput); ph->addStretch(); ph->setContentsMargins(0, 0, 0, 0); posForm->addRow("Y:", ph);
		ph = new QHBoxLayout; ph->addWidget(r[2] = new CFloatInput); ph->addStretch(); ph->setContentsMargins(0, 0, 0, 0); posForm->addRow("Z:", ph);
		pos->setLayout(posForm);

		r[0]->setObjectName("posX");
		r[1]->setObjectName("posY");
		r[2]->setObjectName("posZ");

		buttons = new ::CMeshButtonBox(CLASS_SURFACE_MODIFIER);
		buttons->setObjectName("buttons");

		tool = new CToolBox;
		tool->addTool("Object", obj);
		tool->addTool("Position", pos);
		tool->addTool("Edit Surface Mesh", buttons);
		tool->addTool("Parameters", pw);

		showPositionPanel(false);
		showParametersPanel(false);
		showButtonsPanel(false);

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->setContentsMargins(0,0,0,0);
		mainLayout->addWidget(tool);
		parent->setLayout(mainLayout);

		QMetaObject::connectSlotsByName(parent);
	}

	void setPropertyList(CPropertyList* pl)
	{
		form->setPropertyList(0);
		if (m_pl) delete m_pl;
		m_pl = pl;
		if (m_pl) form->setPropertyList(m_pl);
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

	void ShowObjectInfoPanel(bool b)
	{
		tool->getToolItem(OBJECT_PANEL)->setVisible(b);
	}

	void showButtonsPanel(bool b)
	{
		tool->getToolItem(EDITMESH_PANEL)->setVisible(b);
	}

	void showParametersPanel(bool b)
	{
		tool->getToolItem(PARAMS_PANEL)->setVisible(b);
	}

	void showPositionPanel(bool b)
	{
		tool->getToolItem(POSITION_PANEL)->setVisible(b);
	}
};
