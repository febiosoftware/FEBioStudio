#include "stdafx.h"
#include "DlgMeasure.h"
#include "MainWindow.h"
#include <QLabel>
#include <QBoxLayout>
#include <QGroupBox>
#include <QListWidget>
#include <QLineEdit>
#include <QStackedWidget>
#include <GLLib/GDecoration.h>
#include "Document.h"
#include "Tool.h"
#include "PointDistanceTool.h"
#include "3PointAngleTool.h"
#include "4PointAngleTool.h"
#include "MeasureAreaTool.h"
#include "SphereFitTool.h"
#include "TetOverlapTool.h"
#include "ElementVolumeTool.h"
#include "MeasureVolumeTool.h"
#include "PlaneTool.h"

class Ui::CDlgMeasure
{
public:
	::CMainWindow*	m_wnd;

	QListWidget*			m_list;
	QStackedWidget*			m_stack;
	vector<CAbstractTool*>	m_tools;

	QRect	m_rt;

	CAbstractTool*	m_activeTool;

public:
	void setup(QDialog* dlg)
	{
		QVBoxLayout* l = new QVBoxLayout;

		m_activeTool = nullptr;

		// Measures tools pane
		m_list = new QListWidget;
		for (int i = 0; i < m_tools.size(); ++i)
		{
			QString name = m_tools[i]->name();
			m_list->addItem(name);
		}
		QVBoxLayout* l1 = new QVBoxLayout;
		l1->setMargin(0);
		l1->addWidget(m_list);
		l->addLayout(l1);

		// Results pane
		m_stack = new QStackedWidget();
		QLabel* label = new QLabel("(No tool selected)");
		label->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
		m_stack->addWidget(label);

		for (int i = 0; i < m_tools.size(); ++i)
		{
			CAbstractTool* tool = m_tools[i];
			QWidget* pw = tool->createUi();
			if (pw == 0)
			{
				QLabel* pl = new QLabel("(no properties)");
				pl->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
				m_stack->addWidget(pl);
			}
			else m_stack->addWidget(pw);
		}

		QVBoxLayout* l2 = new QVBoxLayout;
		l2->addWidget(m_stack);
		l2->setMargin(0);
		l->addLayout(l2);

		dlg->setLayout(l);

		QObject::connect(m_list, SIGNAL(currentRowChanged(int)), dlg, SLOT(onMeasureChanged(int)));
	}

	void initTools()
	{
		m_tools.push_back(new CPointDistanceTool(m_wnd));
		m_tools.push_back(new C3PointAngleTool(m_wnd));
		m_tools.push_back(new C4PointAngleTool(m_wnd));
		m_tools.push_back(new CMeasureAreaTool(m_wnd));
		m_tools.push_back(new CElementVolumeTool(m_wnd));
		m_tools.push_back(new CSphereFitTool(m_wnd));
		m_tools.push_back(new CTetOverlapTool(m_wnd));
		m_tools.push_back(new CMeasureVolumeTool(m_wnd));
		m_tools.push_back(new CPlaneTool(m_wnd));
	}

	void clearTools()
	{
		for (int i = 0; i < m_tools.size(); ++i) delete m_tools[i];
		m_tools.clear();
	}
};

CDlgMeasure::CDlgMeasure(CMainWindow* wnd) : QDialog(wnd, Qt::Tool), ui(new Ui::CDlgMeasure)
{
	setWindowTitle("Measure");
	ui->m_wnd = wnd;
	ui->initTools();
	ui->setup(this);
}

CDlgMeasure::~CDlgMeasure()
{
	ui->clearTools();
}

void CDlgMeasure::showEvent(QShowEvent* ev)
{
	Update();
	if (ui->m_rt.isValid()) setGeometry(ui->m_rt);
}

void CDlgMeasure::closeEvent(QCloseEvent* ev)
{
	ui->m_rt = geometry();
	if (ui->m_activeTool) ui->m_activeTool->Deactivate();
	ui->m_activeTool = nullptr;
}

void CDlgMeasure::Update()
{
	if (ui->m_activeTool)
	{
		ui->m_activeTool->Update();
		ui->m_activeTool->updateUi();
	}
}

void CDlgMeasure::onMeasureChanged(int n)
{
	if (ui->m_activeTool) ui->m_activeTool->Deactivate();
	ui->m_activeTool = nullptr;

	if (n < 0)
	{
		ui->m_stack->setCurrentIndex(0);
	}
	else
	{
		ui->m_activeTool = ui->m_tools[n];
		ui->m_activeTool->Activate();
		ui->m_stack->setCurrentIndex(n + 1);
	}
}
