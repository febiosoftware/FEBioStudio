#include "stdafx.h"
#include "PostToolsPanel.h"
#include <QBoxLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QButtonGroup>
#include <QLabel>
#include <QFormLayout>
#include <QSpinBox>
#include <QCloseEvent>
#include <QShowEvent>
#include "Tool.h"
#include "ToolBox.h"
#include "MainWindow.h"
#include "Document.h"
#include <PostLib/FEMesh.h>
#include <PostLib/FEModel.h>
#include <PostLib/GDecoration.h>
#include <PostGL/GLModel.h>
#include "PointDistanceTool.h"
#include "3PointAngleTool.h"
#include "4PointAngleTool.h"
#include "AddPointTool.h"
#include "MeasureAreaTool.h"
//#include "TransformTool.h"
//#include "ShellThicknessTool.h"
//#include "SphereFitTool.h"
//#include "PointCongruencyTool.h"
#include "ImportLinesTool.h"
#include "DistanceMapTool.h"
//#include "PlaneTool.h"
#include "PlotMixTool.h"
#include "AreaCoverageTool.h"
//#include "StrainMapTool.h"
#include "MeasureVolumeTool.h"

static QList<CAbstractTool*>	tools;

class Ui::CPostToolsPanel
{
public:
	QStackedWidget*	stack;
	QButtonGroup*	group;
	CAbstractTool*		activeTool;
	int					activeID;

public:
	void setupUi(::CPostToolsPanel* parent)
	{
		activeID = -1;
		activeTool = 0;

		QVBoxLayout* pg = new QVBoxLayout(parent);
		pg->setMargin(0);
		
		QWidget* box = new QWidget;
		
		QGridLayout* grid = new QGridLayout;
		box->setLayout(grid);
		grid->setSpacing(2);

		group = new QButtonGroup(box);
		group->setObjectName("buttons");

		int ntools = tools.size();
		QList<CAbstractTool*>::Iterator it = tools.begin();
		for (int i=0; i<ntools; ++i, ++it)
		{
			CAbstractTool* tool = *it;
			QPushButton* but = new QPushButton(tool->name());
			but->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
			but->setCheckable(true);

			grid->addWidget(but, i/2, i%2);
			group->addButton(but); group->setId(but, i+1);
		}

		stack = new QStackedWidget;
		QLabel* label = new QLabel("(No tool selected)");
		label->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
		stack->addWidget(label);

		it = tools.begin();
		for (int i=0; i<ntools; ++i, ++it)
		{
			CAbstractTool* tool = *it;
			QWidget* pw = tool->createUi();
			if (pw == 0)
			{
				QLabel* pl = new QLabel("(no properties)");
				pl->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
				stack->addWidget(pl);
			}
			else stack->addWidget(pw);
		}

		// create the toolbox
		CToolBox* tool = new CToolBox;
		tool->addTool("Tools", box);
		tool->addTool("Parameters", stack);

		pg->addWidget(tool);
		pg->addStretch();

		QMetaObject::connectSlotsByName(parent);
	}
};

CPostToolsPanel::CPostToolsPanel(CMainWindow* window, QWidget* parent) : CCommandPanel(window, parent), ui(new Ui::CPostToolsPanel)
{
	initTools();
	ui->setupUi(this);
}

void CPostToolsPanel::Update()
{
	if (ui->activeTool)
	{
		ui->activeTool->deactivate();

		ui->activeTool->activate(GetMainWindow());
	}
}

void CPostToolsPanel::initTools()
{
	tools.push_back(new CPointDistanceTool  ());
	tools.push_back(new C3PointAngleTool    ());
	tools.push_back(new C4PointAngleTool    ());
//	tools.push_back(new CPlaneTool          ());
//	tools.push_back(new CPlotMixTool        ());
	tools.push_back(new CMeasureAreaTool    ());
	tools.push_back(new CImportLinesTool    ());
	tools.push_back(new CKinematTool        ());
	tools.push_back(new CDistanceMapTool    ());
//	tools.push_back(new CCurvatureMapTool   ());
//	tools.push_back(new CPointCongruencyTool());
//	tools.push_back(new CSphereFitTool      ());
//	tools.push_back(new CTransformTool      ());
//	tools.push_back(new CShellThicknessTool ());
	tools.push_back(new CAddPointTool       ());
	tools.push_back(new CImportPointsTool   ());
	tools.push_back(new CAreaCoverageTool   ());
//	tools.push_back(new CStrainMapTool      ());
	tools.push_back(new CMeasureVolumeTool  ());
}

void CPostToolsPanel::on_buttons_buttonClicked(int id)
{
	if (ui->activeID == id)
	{
		ui->stack->setCurrentIndex(0);
		ui->activeTool = 0;
		ui->activeID = -1;
	}
	else
	{
		// deactivate the active tool
		if (ui->activeTool) ui->activeTool->deactivate();

		ui->activeTool = 0;
		ui->activeID = id;

		// find the tool
		ui->activeTool = tools.at(id - 1); assert(ui->activeTool);
		ui->activeTool->activate(GetMainWindow());

		// show the tab
		ui->stack->setCurrentIndex(id);
	}

	// repaint parent
	GetMainWindow()->RedrawGL();
}

void CPostToolsPanel::hideEvent(QHideEvent* ev)
{
	if (ui->activeTool)
	{
		ui->activeTool->deactivate();
	}
	ev->accept();
}

void CPostToolsPanel::showEvent(QShowEvent* ev)
{
	if (ui->activeTool)
	{
		ui->activeTool->activate(GetMainWindow());
	}
	ev->accept();
}
