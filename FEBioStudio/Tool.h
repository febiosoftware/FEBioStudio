#pragma once
#include "PropertyList.h"

class QWidget;
class CMainWindow;
class CDocument;
class CPropertyListForm;
class CPostDocument;
class GObject;
class GDecoration;
class FEMesh;

//-----------------------------------------------------------------------------
// A tool implements a general purpose extension.
// TODO: rename to CExtension?
// TODO: Can this serve as the basis of a plugin?
class CAbstractTool : public QObject
{
public:
	// constructor. Requires a name for the plugin
	CAbstractTool(CMainWindow* wnd, const QString& s);

	// retrieve attributes
	const QString& name() { return m_name; }
	void setName(const QString& s) { m_name = s; }

	// get the main window
	CMainWindow* GetMainWindow();

	// get the currently active mesh
	GObject* GetActiveObject();

	// get the active document
	CDocument* GetDocument();

	// get the active Post doc
	CPostDocument* GetPostDoc();

	// override this to create a custum UI widget
	virtual QWidget* createUi() = 0;

	// activate the tool
	// The ui is about to be shown
	virtual void Activate();

	// deactivate the tool
	// the ui is no longer shown
	virtual void Deactivate();

	// update the Ui
	virtual void updateUi();

	// Update the tool
	virtual void Update();

	// set the decoration
	void SetDecoration(GDecoration* deco);

	// get the active mesh
	FEMesh* GetActiveMesh();

private:
	QString			m_name;
	CMainWindow*	m_wnd;
	GDecoration*	m_deco;
};

//-----------------------------------------------------------------------------
// A tool based on a property list.
// It has an optional "Apply" button. If defined then the derived class
// must implement the OnApply method. 
class CBasicTool : public CAbstractTool, public CDataPropertyList
{
	Q_OBJECT

public:
	enum Flags {
		HAS_APPLY_BUTTON = 1
	};

public:
	CBasicTool(CMainWindow* wnd, const QString& s, unsigned int flags = 0);

	// set the text to appear on the "apply" button (must be called in constructor)
	void SetApplyButtonText(const QString& text);

	// A form will be created based on the property list
	QWidget* createUi();

	// method called when user presses Apply button (optional)
	virtual bool OnApply();

	// update the Ui
	virtual void updateUi();

public:
	bool SetErrorString(const QString& err);
	QString GetErrorString();

private slots:
	void on_button_clicked();

private:
	CPropertyList*		m_list;
	CPropertyListForm*	m_form;
	unsigned int		m_flags;
	QString				m_applyText;

	QString			m_err;
};
