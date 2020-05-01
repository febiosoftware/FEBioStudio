#include "Tool.h"
#include "PropertyListForm.h"
#include <QBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include "MainWindow.h"
#include "PostDocument.h"
#include "Document.h"
#include <PostGL/GLModel.h>
#include <GeomLib/GObject.h>

//-----------------------------------------------------------------------------
CAbstractTool::CAbstractTool(CMainWindow* wnd, const QString& s) : m_name(s)
{
	m_wnd = wnd;
	m_deco = nullptr;
}

CDocument* CAbstractTool::GetDocument()
{ 
	if (m_wnd == nullptr) return nullptr;
	return m_wnd->GetDocument(); 
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

void CAbstractTool::Activate()
{
	Update();
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
}

// get the active mesh
FEMesh* CAbstractTool::GetActiveMesh()
{
	GObject* po = GetActiveObject();
	if (po) return po->GetFEMesh();
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
		CMainWindow* wnd = GetMainWindow();
		QString err = GetErrorString();
		if (err.isEmpty()) err = "An unknown error has occurred";
		QMessageBox::critical(wnd, "Error", err);
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
