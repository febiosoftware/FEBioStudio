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
#include <QGridLayout>
#include <QMenu>
#include <QValidator>
#include "ClassDescriptor.h"
#include "PropertyListForm.h"
#include "PropertyList.h"
#include "ObjectPanel.h"
#include "MeshButtonBox.h"

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

	CPropertyList* m_pl;

public:
	void setupUi(QWidget* parent, ::CMainWindow* mainWindow)
	{
		m_pl = 0;

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
		mainLayout->setMargin(0);
		mainLayout->addWidget(tool);
		parent->setLayout(mainLayout);

		QMetaObject::connectSlotsByName(parent);
	}

	void setMesherPropertyList(CPropertyList* pl)
	{
		form->setPropertyList(0);
		if (m_pl) delete m_pl;
		m_pl = pl;
		if (m_pl) form->setPropertyList(m_pl);
	}

	void setModifierPropertyList(CPropertyList* pl)
	{
		form2->setPropertyList(0);
		if (m_pl) delete m_pl;
		m_pl = pl;
		if (m_pl) form2->setPropertyList(m_pl);
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
