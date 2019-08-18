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
		mainLayout->setMargin(0);
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
