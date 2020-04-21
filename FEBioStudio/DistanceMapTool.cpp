#include "stdafx.h"
#include "DistanceMapTool.h"
#include <QWidget>
#include <QBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QFormLayout>
#include <QLineEdit>
#include "Document.h"
#include "MainWindow.h"
#include <MeshTools/SphereFit.h>
#include "PropertyListView.h"
#include <PostLib/FEPointCongruency.h>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <PostGL/GLLinePlot.h>
#include <PostGL/GLModel.h>
#include <PostLib/FEDistanceMap.h>
#include <PostLib/FECurvatureMap.h>
#include "PostDoc.h"
using namespace Post;

class CDistanceMapToolUI : public QWidget
{
public:
	QCheckBox*	check;
	QPushButton* p1;
	QPushButton* p2;
	FEDistanceMap	m_map;

public:
	CDistanceMapToolUI(CDistanceMapTool* ptool)
	{
		QPushButton* apply;
		QVBoxLayout* pv = new QVBoxLayout;
		{
			p1 = new QPushButton("Assign to surface 1");
			p2 = new QPushButton("Assign to surface 2");
			check = new QCheckBox("Signed distance");
			apply = new QPushButton("Apply");

			pv->addWidget(p1);
			pv->addWidget(p2);
			pv->addWidget(check);
			pv->addWidget(apply);
			pv->addStretch();
		}
		setLayout(pv);
		QObject::connect(p1, SIGNAL(clicked(bool)), ptool, SLOT(OnAssign1()));
		QObject::connect(p2, SIGNAL(clicked(bool)), ptool, SLOT(OnAssign2()));
		QObject::connect(apply, SIGNAL(clicked(bool)), ptool, SLOT(OnApply()));
	}
};

// constructor
CDistanceMapTool::CDistanceMapTool() : CAbstractTool("Distance Map")
{
	ui = 0;
}

// get the property list
QWidget* CDistanceMapTool::createUi()
{
	return ui = new CDistanceMapToolUI(this);
}

void CDistanceMapTool::OnAssign1()
{
	CPostDoc* doc = GetPostDoc();
	if (doc && doc->IsValid())
	{
		vector<int> sel;
		doc->GetGLModel()->GetSelectionList(sel, SELECT_FACES);
		ui->m_map.SetSelection1(sel);
		int n = (int)sel.size();
		ui->p1->setText(QString("Assign to surface 1 (%1 faces)").arg(n));
	}
}

void CDistanceMapTool::OnAssign2()
{
	CPostDoc* doc = GetPostDoc();
	if (doc && doc->IsValid())
	{
		vector<int> sel;
		doc->GetGLModel()->GetSelectionList(sel, SELECT_FACES);
		ui->m_map.SetSelection2(sel);
		int n = (int)sel.size();
		ui->p2->setText(QString("Assign to surface 2 (%1 faces)").arg(n));
	}
}

void CDistanceMapTool::OnApply()
{
	CPostDoc* doc = GetPostDoc();
	if (doc && doc->IsValid())
	{
		bool bcheck = ui->check->isChecked();
		FEDistanceMap& map = ui->m_map;
		map.m_bsigned = bcheck;
		map.Apply(*doc->GetFEModel());
		updateUi();
	}
}


class CCurvatureMapToolUI : public QWidget
{
public:
	QPushButton* p1;
	QPushButton* p2;
	FECongruencyMap	m_map;

public:
	CCurvatureMapToolUI(CCurvatureMapTool* ptool)
	{
		QPushButton* apply;
		QVBoxLayout* pv = new QVBoxLayout;
		{
			p1 = new QPushButton("Assign to surface 1");
			p2 = new QPushButton("Assign to surface 2");
			apply = new QPushButton("Apply");

			pv->addWidget(p1);
			pv->addWidget(p2);
			pv->addWidget(apply);
			pv->addStretch();
		}
		setLayout(pv);
		QObject::connect(p1, SIGNAL(clicked(bool)), ptool, SLOT(OnAssign1()));
		QObject::connect(p2, SIGNAL(clicked(bool)), ptool, SLOT(OnAssign2()));
		QObject::connect(apply, SIGNAL(clicked(bool)), ptool, SLOT(OnApply()));
	}
};

// constructor
CCurvatureMapTool::CCurvatureMapTool() : CAbstractTool("Curvature Map")
{
	ui = 0;
}

// get the property list
QWidget* CCurvatureMapTool::createUi()
{
	return ui = new CCurvatureMapToolUI(this);
}

void CCurvatureMapTool::OnAssign1()
{
	CPostDoc* doc = GetPostDoc();
	if (doc && doc->IsValid())
	{
		vector<int> sel;
		doc->GetGLModel()->GetSelectionList(sel, SELECT_FACES);
		ui->m_map.SetSelection1(sel);
		int n = (int)sel.size();
		ui->p1->setText(QString("Assign to surface 1 (%1 faces)").arg(n));
	}
}

void CCurvatureMapTool::OnAssign2()
{
	CPostDoc* doc = GetPostDoc();
	if (doc && doc->IsValid())
	{
		vector<int> sel;
		doc->GetGLModel()->GetSelectionList(sel, SELECT_FACES);
		ui->m_map.SetSelection2(sel);
		int n = (int)sel.size();
		ui->p2->setText(QString("Assign to surface 2 (%1 faces)").arg(n));
	}
}

void CCurvatureMapTool::OnApply()
{
	CPostDoc* doc = GetPostDoc();
	if (doc && doc->IsValid())
	{
		FECongruencyMap& map = ui->m_map;
		map.Apply(*doc->GetFEModel());
		updateUi();
	}
}
