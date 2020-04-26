#include "stdafx.h"
#include "ToolsPanel.h"
#include "ui_toolspanel.h"
#include "MainWindow.h"
#include "ConchoidFitTool.h"
#include "ReadCurveTool.h"
#include "FoamGeneratorTool.h"
#include "MaterialMapTool.h"
#include "ScalarFieldTool.h"
#include "PlaneCutTool.h"
#include "FiberGeneratorTool.h"
#include "AreaCalculatorTool.h"
#include "ImportSpringsTool.h"

CToolsPanel::CToolsPanel(CMainWindow* wnd, QWidget* parent) : CCommandPanel(wnd, parent), ui(new Ui::CToolsPanel)
{
	m_activeTool = 0;
	initTools();
	ui->setupUi(this);
}

void CToolsPanel::Update(bool breset)
{
	if (m_activeTool)
	{
		m_activeTool->Update();
	}
}

void CToolsPanel::initTools()
{
	CMainWindow* wnd = GetMainWindow();
	tools.push_back(new CConchoidFitTool   (wnd));
	tools.push_back(new CReadCurveTool     (wnd));
	tools.push_back(new CFoamGeneratorTool (wnd));
	tools.push_back(new CMaterialMapTool   (wnd));
	tools.push_back(new CScalarFieldTool   (wnd));
	tools.push_back(new CPlaneCutTool      (wnd));
	tools.push_back(new CFiberGeneratorTool(wnd));
    tools.push_back(new CAreaCalculatorTool(wnd));
	tools.push_back(new CImportSpringsTool (wnd));
}

void CToolsPanel::on_buttons_buttonClicked(int id)
{
	// deactivate the active tool
	if (m_activeTool) m_activeTool->Deactivate();
	m_activeTool = 0;

	// find the tool
	QList<CAbstractTool*>::iterator it = tools.begin();
	for (int i = 0; i<id - 1; ++i, ++it);

	// get the active document
	CDocument* doc = GetDocument();

	// activate the tool
	m_activeTool = *it;
	m_activeTool->Activate();

	// show the tab
	ui->stack->setCurrentIndex(id);
}

void CToolsPanel::hideEvent(QHideEvent* ev)
{
	if (m_activeTool)
	{
		m_activeTool->Deactivate();
	}
	ev->accept();
}

void CToolsPanel::showEvent(QShowEvent* ev)
{
	if (m_activeTool)
	{
		m_activeTool->Activate();
	}
	ev->accept();
}
