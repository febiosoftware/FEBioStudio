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
#include "PropertyList.h"

class QWidget;
class CMainWindow;
class CGLDocument;
class CPropertyListForm;
class CPostDocument;
class GObject;
class GDecoration;
class FSMesh;
class FSMeshBase;

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

	QString GetInfo() const { return m_info; }
	void SetInfo(const QString& info) { m_info = info; }

	// get the main window
	CMainWindow* GetMainWindow();

	// get the currently active mesh
	GObject* GetActiveObject();

	// get the active document
	CGLDocument* GetDocument();

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
	FSMesh* GetActiveMesh();
	FSMeshBase* GetActiveEditMesh();

	void SetID(int n) { m_id = n; }
	int GetID() const { return m_id; }

private:
	int				m_id;
	QString			m_name;
	QString			m_info;
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
	CPropertyListForm*	m_form;
	unsigned int		m_flags;
	QString				m_applyText;

	QString			m_err;
};
