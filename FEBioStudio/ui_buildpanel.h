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
		l->setMargin(0);
		l->addWidget(tab);

		parent->setLayout(l);

		QMetaObject::connectSlotsByName(parent);
	}

	CCommandPanel* currentPanel()
	{
		return dynamic_cast<CCommandPanel*>(tab->currentWidget());
	}
};
