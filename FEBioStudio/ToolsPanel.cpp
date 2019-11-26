#include "stdafx.h"
#include "ToolsPanel.h"
#include "ui_toolspanel.h"
#include "MainWindow.h"
#include "SphereFitTool.h"
#include "ConchoidFitTool.h"
#include "ReadCurveTool.h"
#include "FoamGeneratorTool.h"
#include "MaterialMapTool.h"
#include "ScalarFieldTool.h"
#include "PlaneCutTool.h"
#include "FiberGeneratorTool.h"
#include "AreaCalculatorTool.h"
#include "TetOverlapTool.h"

CToolsPanel::CToolsPanel(CMainWindow* wnd, QWidget* parent) : CCommandPanel(wnd, parent), ui(new Ui::CToolsPanel)
{
	m_activeTool = 0;
	initTools();
	ui->setupUi(this);
}

void CToolsPanel::Update()
{
	if (m_activeTool)
	{
		m_activeTool->deactivate();

		m_activeTool->activate(GetDocument());
	}
}

void CToolsPanel::initTools()
{
	tools.push_back(new CSphereFitTool     );
	tools.push_back(new CConchoidFitTool   );
	tools.push_back(new CReadCurveTool     );
	tools.push_back(new CFoamGeneratorTool );
	tools.push_back(new CMaterialMapTool   );
	tools.push_back(new CScalarFieldTool   );
	tools.push_back(new CPlaneCutTool      );
	tools.push_back(new CFiberGeneratorTool);
    tools.push_back(new CAreaCalculatorTool);
	tools.push_back(new CTetOverlapTool    );
}

void CToolsPanel::on_buttons_buttonClicked(int id)
{
	// deactivate the active tool
	if (m_activeTool) m_activeTool->deactivate();
	m_activeTool = 0;

	// find the tool
	QList<CAbstractTool*>::iterator it = tools.begin();
	for (int i = 0; i<id - 1; ++i, ++it);

	// get the active document
	CDocument* doc = GetDocument();

	// activate the tool
	m_activeTool = *it;
	m_activeTool->activate(doc);

	// show the tab
	ui->stack->setCurrentIndex(id);
}

void CToolsPanel::hideEvent(QHideEvent* ev)
{
	if (m_activeTool)
	{
		m_activeTool->deactivate();
	}
	ev->accept();
}

void CToolsPanel::showEvent(QShowEvent* ev)
{
	if (m_activeTool)
	{
		m_activeTool->activate(GetDocument());
	}
	ev->accept();
}
