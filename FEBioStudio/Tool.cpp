#include "Tool.h"
#include "PropertyListForm.h"
#include <QApplication>
#include <QBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include "MainWindow.h"
#include "PostDoc.h"
#include "Document.h"
#include <PostGL/GLModel.h>

//-----------------------------------------------------------------------------
CAbstractTool::CAbstractTool(const QString& s) : m_name(s) 
{
	m_wnd = nullptr;
}

CDocument* CAbstractTool::GetDocument()
{ 
	if (m_wnd == nullptr) return nullptr;
	return m_wnd->GetDocument(); 
}

CPostDoc* CAbstractTool::GetPostDoc()
{
	if (m_wnd == nullptr) return nullptr;
	return m_wnd->GetActiveDocument();
}

// get the main window
CMainWindow* CAbstractTool::GetMainWindow()
{
	return m_wnd;
}

void CAbstractTool::updateUi()
{
	QApplication::activeWindow()->repaint();
}

void CAbstractTool::activate(CMainWindow* wnd)
{
	m_wnd = wnd;
}

void CAbstractTool::deactivate()
{
	m_wnd = nullptr;
}

GObject* CAbstractTool::GetActiveObject()
{
	CMainWindow* wnd = GetMainWindow();
	CPostDoc* postDoc = wnd->GetActiveDocument();
	if (postDoc)
	{
		return postDoc->GetPostObject();
	}
	else
	{
		CDocument* doc = GetDocument();
		return doc->GetActiveObject();
	}
}

//-----------------------------------------------------------------------------
CBasicTool::CBasicTool(const QString& s, unsigned int flags) : CAbstractTool(s)
{
	m_list = 0;
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
	m_list = this;
	if (m_list == 0) return 0;

	QWidget* pw = new QWidget;
	QVBoxLayout* pl = new QVBoxLayout(pw);
	pw->setLayout(pl);

	m_form = new CPropertyListForm;
	m_form->setBackgroundRole(QPalette::Light);
	m_form->setPropertyList(m_list);
	pl->addWidget(m_form);

	if (m_flags & HAS_APPLY_BUTTON)
	{
		QHBoxLayout* pg = new QHBoxLayout;
		pg->addStretch();
		
		QPushButton* pb = new QPushButton(m_applyText);
		pg->addWidget(pb);
		pl->addLayout(pg);

		QObject::connect(pb, SIGNAL(clicked(bool)), this, SLOT(on_button_clicked()));
	}
	pl->addStretch();

	return pw;
}

//-----------------------------------------------------------------------------
bool CBasicTool::SetErrorString(const QString& err)
{
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
	if (ret == false)
	{
		QWidget* wnd = QApplication::activeWindow();
		QString err = GetErrorString();
		if (err.isEmpty()) err = "An unknown error has occurred";
		QMessageBox::critical(wnd, "Error", err);
	}
	updateUi();
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
