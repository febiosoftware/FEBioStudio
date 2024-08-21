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

#include "Tool.h"
#include "PropertyListForm.h"
#include <QBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include "MainWindow.h"
#include "PostDocument.h"
#include "Document.h"
#include "GLView.h"
#include <PostGL/GLModel.h>
#include <GeomLib/GObject.h>
#include "Logger.h"

//-----------------------------------------------------------------------------
CAbstractTool::CAbstractTool(CMainWindow* wnd, const QString& s) : m_name(s)
{
	m_id = -1;
	m_wnd = wnd;
	m_deco = nullptr;
}

CGLDocument* CAbstractTool::GetDocument()
{ 
	if (m_wnd == nullptr) return nullptr;
	return m_wnd->GetGLDocument(); 
}

CPostDocument* CAbstractTool::GetPostDoc()
{
	if (m_wnd == nullptr) return nullptr;
	return m_wnd->GetPostDocument();
}

// get the main window
CMainWindow* CAbstractTool::GetMainWindow()
{
	return m_wnd;
}

void CAbstractTool::updateUi()
{
	m_wnd->repaint();
}

// Update the tool
void CAbstractTool::Update()
{

}

void CAbstractTool::Reset()
{
	updateUi();
	SetDecoration(nullptr);
}

void CAbstractTool::Activate()
{
	Update();
	updateUi();
}

void CAbstractTool::Deactivate()
{
	SetDecoration(nullptr);
}

// set the decoration
void CAbstractTool::SetDecoration(GDecoration* deco)
{
	CGLView* view = m_wnd->GetGLView();
	if (m_deco)
	{
		view->RemoveDecoration(m_deco);
		delete m_deco;
	}
	m_deco = deco;
	if (m_deco) view->AddDecoration(m_deco);
	view->repaint();
}

// get the active mesh
FSMesh* CAbstractTool::GetActiveMesh()
{
	GObject* po = GetActiveObject();
	if (po) return po->GetFEMesh();
	else return nullptr;
}

FSMeshBase* CAbstractTool::GetActiveEditMesh()
{
	GObject* po = GetActiveObject();
	if (po) return po->GetEditableMesh();
	else return nullptr;
}

GObject* CAbstractTool::GetActiveObject()
{
	CMainWindow* wnd = GetMainWindow();
	return wnd->GetActiveObject();
}

//-----------------------------------------------------------------------------
CBasicTool::CBasicTool(CMainWindow* wnd, const QString& s, unsigned int flags) : CAbstractTool(wnd, s)
{
	m_form = 0;
	m_flags = flags;
	m_applyText = "Apply";
}

//-----------------------------------------------------------------------------
void CBasicTool::SetApplyButtonText(const QString& text)
{
	m_applyText = text;
}

//-----------------------------------------------------------------------------
QWidget* CBasicTool::createUi()
{
	QWidget* pw = new QWidget;
	QVBoxLayout* pl = new QVBoxLayout(pw);
	pw->setLayout(pl);

	m_form = new CPropertyListForm;
	m_form->setBackgroundRole(QPalette::Light);
	m_form->setPropertyList(this);
	pl->addWidget(m_form);

	if (m_flags & HAS_APPLY_BUTTON)
	{
		QHBoxLayout* pg = new QHBoxLayout;
		pg->addStretch();
		
		QPushButton* pb = new QPushButton(m_applyText);
		pg->addWidget(pb);
		pl->addLayout(pg);

        connect(pb, &QPushButton::clicked, this, &CBasicTool::on_button_clicked);
	}
	pl->addStretch();

	return pw;
}

//-----------------------------------------------------------------------------
bool CBasicTool::SetErrorString(const QString& err)
{
	if (err.isEmpty() == false)
	{
		QString msg = QString("[%1]%2\n").arg(name()).arg(err);
		CLogger::AddLogEntry(msg);
	}
	m_err = err;
	return err.isEmpty();
}

//-----------------------------------------------------------------------------
QString CBasicTool::GetErrorString()
{
	return m_err;
}

//-----------------------------------------------------------------------------
void CBasicTool::on_button_clicked()
{
	SetErrorString("");
	bool ret = OnApply();
	QString err = GetErrorString();
	CMainWindow* wnd = GetMainWindow();
	QString toolName = name();
	if (ret == false)
	{
		if (err.isEmpty()) err = "An unknown error has occurred";
		if (toolName.isEmpty()) toolName = "ERROR";
		QMessageBox::critical(wnd, toolName, err);
	}
	else if (err.isEmpty() == false)
	{
		if (toolName.isEmpty()) toolName = "WARNING";
		QMessageBox::warning(wnd, toolName, err);
	}
	updateUi();
	GetMainWindow()->UpdateModel();
}

//-----------------------------------------------------------------------------
bool CBasicTool::OnApply()
{
	SetErrorString("Inside CBasicTool::OnApply. Derived class did not override this function.");
	return false;
}

//-----------------------------------------------------------------------------
void CBasicTool::updateUi()
{
	if (m_form) 
	{
		m_form->updateData();
	}
	CAbstractTool::updateUi();
}
