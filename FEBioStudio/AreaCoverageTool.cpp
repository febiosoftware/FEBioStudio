#include "stdafx.h"
#include "AreaCoverageTool.h"
#include <QBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <vector>
#include "PostDoc.h"
#include <PostLib/FEAreaCoverage.h>
#include <PostGL/GLModel.h>
using namespace std;
using namespace Post;

class CAreaCoverageToolUI : public QWidget
{
public:
	QPushButton* p1;
	QPushButton* p2;
	QLineEdit*	name;

	FEAreaCoverage	m_tool;

public:
	CAreaCoverageToolUI(CAreaCoverageTool* tool)
	{
		name = new QLineEdit; name->setPlaceholderText("Enter name here");
		p1 = new QPushButton("Assign to surface 1");
		p2 = new QPushButton("Assign to surface 2");

		QPushButton* apply = new QPushButton("Apply");

		QHBoxLayout* h = new QHBoxLayout;
		h->addWidget(new QLabel("Name:"));
		h->addWidget(name);

		QVBoxLayout* pv = new QVBoxLayout;
		pv->addLayout(h);
		pv->addWidget(p1);
		pv->addWidget(p2);
		pv->addWidget(apply);
		pv->addStretch();

		setLayout(pv);

		QObject::connect(p1, SIGNAL(clicked(bool)), tool, SLOT(OnAssign1()));
		QObject::connect(p2, SIGNAL(clicked(bool)), tool, SLOT(OnAssign2()));
		QObject::connect(apply, SIGNAL(clicked(bool)), tool, SLOT(OnApply()));
	}
};

//=============================================================================

CAreaCoverageTool::CAreaCoverageTool(CMainWindow* wnd) : CAbstractTool(wnd, "Area Coverage")
{
	ui = 0;
}

QWidget* CAreaCoverageTool::createUi()
{
	return ui = new CAreaCoverageToolUI(this);
}

void CAreaCoverageTool::OnAssign1()
{
	CPostDoc* doc = GetPostDoc();
	if (doc && doc->IsValid())
	{
		vector<int> sel;
		doc->GetGLModel()->GetSelectionList(sel, SELECT_FACES);
		ui->m_tool.SetSelection1(sel);
		int n = (int)sel.size();
		ui->p1->setText(QString("Assign to surface 1 (%1 faces)").arg(n));
	}
}

void CAreaCoverageTool::OnAssign2()
{
	CPostDoc* doc = GetPostDoc();
	if (doc && doc->IsValid())
	{
		vector<int> sel;
		doc->GetGLModel()->GetSelectionList(sel, SELECT_FACES);
		ui->m_tool.SetSelection2(sel);
		int n = (int)sel.size();
		ui->p2->setText(QString("Assign to surface 2 (%1 faces)").arg(n));
	}
}

bool CAreaCoverageTool::OnApply()
{
	CPostDoc* doc = GetPostDoc();
	if (doc && doc->IsValid())
	{
		FEAreaCoverage& tool = ui->m_tool;

		tool.SetDataFieldName("");
		QString name = ui->name->text();
		if (name.isEmpty() == false)
		{
			tool.SetDataFieldName(name.toStdString());
		}
		tool.Apply(*doc->GetFEModel());
		updateUi();
	}
	return true;
}
