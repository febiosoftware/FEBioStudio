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
#include "CommandPanel.h"
#include <QDialog>
#include "PropertyListForm.h"
#include <FSCore/math3d.h>
#include <vector>

class QLineEdit;
class QCheckBox;
class FSObject;
class GObject;
class GObject2D;
class ClassDescriptor;
class QGridLayout;
class QButtonGroup;
class FESelection;
class GEdge;
class QComboBox;
class QSpinBox;
class QListWidget;
class GItem;
class GCurveMeshObject;
class QPushButton;
class GModifier;

namespace Ui {
	class CCreatePanel;
}

//-----------------------------------------------------------------------------
// The Create panel allows users to create geometrical objects
class CCreatePanel : public CCommandPanel
{
	Q_OBJECT

public:
	CCreatePanel(CMainWindow* wnd, QWidget* parent = 0);

	GObject* GetTempObject();
	void SetTempObject(GObject* po);

	void setInput(const vec3d& p);
	void setInput(FESelection* sel);

	bool OnEscapeEvent() override;
	bool OnDeleteEvent() override;

	void Apply() override;

protected slots:
	void on_create_clicked();
	void select_option(int id);

private:
	Ui::CCreatePanel* ui;
	GObject*	m_tmpObject;	//!< temporary object that is being created
};

//-----------------------------------------------------------------------------
// This class shows a group of icons for creating different objects
class CCreateButtonPanel : public QWidget
{
	Q_OBJECT

public:
	QGridLayout* buttonGrid;
	QButtonGroup* buttonGroup;

public:
	CCreateButtonPanel(QWidget* parent = 0);

	void AddCreateButton(const QString& txt, const QIcon& icon, int nid);

protected slots:
	void on_button_clicked(int id);

signals:
	void idClicked(int id);
};


//-----------------------------------------------------------------------------
//! Base class for create panes
//! A create pane shows the options for creating an instance of the selected object type
class CCreatePane : public QWidget
{
public:
	enum CREATE_POLICY
	{
		ADD_NEW_OBJECT,
		REPLACE_ACTIVE_OBJECT
	};

public:
	CCreatePane(CCreatePanel* parent) : QWidget(parent), m_parent(parent) { m_createPolicy = ADD_NEW_OBJECT; }
	virtual ~CCreatePane() {}

	virtual FSObject* Create() = 0;

	// TODO: I think QT's show and hide events are better suited than the activate/deactive

	// called when this pane is activated
	virtual void Activate() {}

	// called when this pane is deactivated
	virtual void Deactivate() {}

	// called when the input of the pane has to be reset
	// return false if nothing needs to be reset.
	virtual bool Clear() { return false; }

	// called when the last action has to be undone
	// return false to ignore this event
	virtual bool Undo() { return false; }

	// set the input event for this pane
	virtual void setInput(const vec3d& r) {}
	virtual void setInput(FESelection* sel) {}

	int createPolicy() const { return m_createPolicy; }
	void setCreatePolicy(CREATE_POLICY policy) { m_createPolicy = policy; }

protected:
	CCreatePanel*	m_parent;
	int				m_createPolicy;
};

//-----------------------------------------------------------------------------
//! A default create pane (used for primitives and objects with simple parameter lists)
class CCreatePosition;
class CCreateParams;
class GDiscreteElementSet;

class CDefaultCreatePane : public CCreatePane
{
	Q_OBJECT

public:
	CDefaultCreatePane(CCreatePanel* parent, ClassDescriptor* pcd);

	FSObject* Create() override;

	void Activate() override;

	void setInput(const vec3d& r) override;

private:
	void CreateObject();

public:
	ClassDescriptor*	m_pcd;
	CCreatePosition*	position;
	CCreateParams*		params;
	GObject*			m_po;
};

class CNewDiscreteSetDlg : public QDialog
{
public:
	CNewDiscreteSetDlg(QWidget* parent = 0);

	void accept();

public:
	QString		m_name;
	int			m_type;

private:
	QLineEdit*	m_edit;
	QComboBox*	m_combo;
};

//=============================================================================
// CAD Loft surface

class CCreateLoftSurface : public CCreatePane
{
	Q_OBJECT

public:
	CCreateLoftSurface(CCreatePanel* parent);

	void Activate() override;
	void Deactivate() override;

	FSObject* Create() override;

	void setInput(FESelection* sel) override;

	void showEvent(QShowEvent* ev) override;

	void hideEvent(QHideEvent* ev) override;

private slots:
	void itemPicked(GItem*);

protected:
	std::vector<GEdge*>	m_edge;
	QComboBox*			m_combo;
	QSpinBox*			m_divs;
	QListWidget*		m_list;
	QCheckBox*			m_smooth;
};

//=============================================================================
// Geometry modifiers (TODO: should I put this on the Edit panel instead?

class CGeoModifierPane : public CCreatePane
{
	Q_OBJECT

public:
	CGeoModifierPane(CCreatePanel* parent, ClassDescriptor* pcd);

	void Activate();
	void Deactivate();

	FSObject* Create();

protected:
	void CreateModifier();

protected:
	ClassDescriptor*	m_pcd;
	CPropertyListForm*  m_params;
	GModifier*			m_mod;
};
