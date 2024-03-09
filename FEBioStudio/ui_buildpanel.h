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
#include "BuildPanel.h"
#include "CreatePanel.h"
#include "EditPanel.h"
#include "MeshPanel.h"
#include "ToolsPanel.h"
#include <QTabWidget>
#include <QBoxLayout>

class Ui::CBuildPanel
{
public:
	::CCreatePanel*	create;
	::CEditPanel*	edit;
	::CMeshPanel*	mesh;
	::CToolsPanel*	tools;
	::CMainWindow*	mainWindow;

	QTabWidget*	tab;

	enum BUILD_PANELS {
		CREATE_PANEL = 0,
		EDIT_PANEL = 1,
		MESH_PANEL = 2,
		TOOLS_PANEL = 3
	};

public:
	void setup(QWidget* parent, ::CMainWindow* wnd)
	{
		mainWindow = wnd;

		tab = new QTabWidget; tab->setObjectName("buildTab");

		tab->addTab(create = new ::CCreatePanel(wnd, parent), "Create");
		tab->addTab(edit   = new ::CEditPanel  (wnd, parent), "Edit"  );
		tab->addTab(mesh   = new ::CMeshPanel  (wnd, parent), "Mesh"  );
		tab->addTab(tools  = new ::CToolsPanel (wnd, parent), "Tools" );

		QHBoxLayout* l = new QHBoxLayout;
		l->setContentsMargins(0,0,0,0);
		l->addWidget(tab);

		parent->setLayout(l);

		QMetaObject::connectSlotsByName(parent);
	}

	CWindowPanel* currentPanel()
	{
		return dynamic_cast<CWindowPanel*>(tab->currentWidget());
	}
};
