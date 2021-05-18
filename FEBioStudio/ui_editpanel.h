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
#include "EditPanel.h"
#include <QLineEdit>
#include <QBoxLayout>
#include <QPushButton>
#include <QAction>
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

class Ui::CEditPanel
{
	// make sure these values correspond to the order of the panel
	enum {
		OBJECT_PANEL,
		EDITMESH_PANEL,
		PARAMS_PANEL
	};

public:
	CObjectPanel* obj;
	CPropertyListForm*	form;
	::CMeshButtonBox*	buttons;
	CToolBox* tool;

	CPropertyList* m_pl;

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
		pl->addWidget(applyButton);
		pw->setLayout(pl);

		buttons = new ::CMeshButtonBox(CLASS_SURFACE_MODIFIER);
		buttons->setObjectName("buttons");

		tool = new CToolBox;
		tool->addTool("Object", obj);
		tool->addTool("Edit Surface Mesh", buttons);
		tool->addTool("Parameters", pw);

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
};
