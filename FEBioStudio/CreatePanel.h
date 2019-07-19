#pragma once
#include "CommandPanel.h"
#include <QDialog>
#include <MathLib/math3d.h>

class QLineEdit;
class QCheckBox;
class FEObject;
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
	void buttonClicked(int id);
};


//-----------------------------------------------------------------------------
//! Base class for create panes
//! A create pane shows the options for creating an instance of the selected object type
class CCreatePane : public QWidget
{
public:
	CCreatePane(CCreatePanel* parent) : QWidget(parent), m_parent(parent) {}
	virtual ~CCreatePane(){}

	virtual FEObject* Create() = 0;

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

protected:
	CCreatePanel*	m_parent;
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

	FEObject* Create() override;

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

	FEObject* Create() override;

	void setInput(FESelection* sel) override;

	void showEvent(QShowEvent* ev) override;

	void hideEvent(QHideEvent* ev) override;

private slots:
	void itemPicked(GItem*);

protected:
	vector<GEdge*>		m_edge;
	QComboBox*			m_combo;
	QSpinBox*			m_divs;
	QListWidget*		m_list;
};

//=============================================================================
// CAD extrude surface

class CCreateExtrude : public CCreatePane
{
	Q_OBJECT

public:
	CCreateExtrude(CCreatePanel* parent);

	void Activate();
	void Deactivate();

	FEObject* Create();

protected:
	QLineEdit*	m_distance;
};
