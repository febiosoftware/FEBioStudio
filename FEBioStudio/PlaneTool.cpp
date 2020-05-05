#include "stdafx.h"
#include "PlaneTool.h"
#include "CIntInput.h"
#include <QBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include "Document.h"
#include <PostGL/GLModel.h>
#include <PostLib/FEPostMesh.h>
#include "MainWindow.h"
#include "GLView.h"
using namespace Post;

//-----------------------------------------------------------------------------
class CPlaneDecoration : public GDecoration
{
public:
	CPlaneDecoration()
	{
		point[0] = new GPointDecoration(vec3f(0,0,0));
		point[1] = new GPointDecoration(vec3f(0,0,0));
		point[2] = new GPointDecoration(vec3f(0,0,0));
		center = new GPointDecoration(vec3f(0, 0, 0));
		tip = new GPointDecoration(vec3f(0, 0, 0));
		line[0] = new GLineDecoration(point[0], point[1]);
		line[1] = new GLineDecoration(point[1], point[2]);
		line[2] = new GLineDecoration(point[2], point[0]);
		norm = new GLineDecoration(center, tip);
		tri = new GTriangleDecoration(point[0], point[1], point[2]);
		tri->setColor(GLColor(255, 255, 0, 128));
		setVisible(false);
	}

	~CPlaneDecoration()
	{
		delete line[2]; 
		delete line[1]; 
		delete line[0];
		delete point[2];
		delete point[1];
		delete point[0];
	}

	void setPosition(const vec3f& a, const vec3f& b, const vec3f& c)
	{
		point[0]->setPosition(a);
		point[1]->setPosition(b);
		point[2]->setPosition(c);

		vec3f N = (b - a) ^ (c - a);
		double L = N.Length(); N /= 2.f*(float)sqrt(L);

		vec3f o = (a + b + c) / 3.f;
		center->setPosition(o);
		tip->setPosition(o + N);
	}

	void render()
	{
		tri->render();
		point[0]->render();
		point[1]->render();
		point[2]->render();
		line[0]->render();
		line[1]->render();
		line[2]->render();
		norm->render();
	}

private:
	GPointDecoration*	point[3];
	GPointDecoration*	center, *tip;
	GLineDecoration*	line[3];
	GLineDecoration*	norm;
	GTriangleDecoration*	tri;
};

class CPlaneToolUI : public QWidget
{
public:
	int	m_node[3];

public:
	CIntInput* 	node1;
	CIntInput* 	node2;
	CIntInput* 	node3;

	CFloatInput* normx;
	CFloatInput* normy;
	CFloatInput* normz;

public:
	CPlaneToolUI(QObject* parent)
	{
		m_node[0] = 0;
		m_node[1] = 0;
		m_node[2] = 0;

		QVBoxLayout* pv = new QVBoxLayout;
		QGroupBox* pg = new QGroupBox("Select nodes");
		QFormLayout* pform = new QFormLayout;
		pform->addRow("node 1:", node1 = new CIntInput);
		pform->addRow("node 2:", node2 = new CIntInput);
		pform->addRow("node 3:", node3 = new CIntInput);
		pg->setLayout(pform);
		pv->addWidget(pg);

		pg = new QGroupBox("Plane normal:");
		pform = new QFormLayout;
		pform->addRow("x:", normx = new CFloatInput); normx->setReadOnly(true);
		pform->addRow("y:", normy = new CFloatInput); normy->setReadOnly(true);
		pform->addRow("z:", normz = new CFloatInput); normz->setReadOnly(true);
		pg->setLayout(pform);
		pv->addWidget(pg);

		QPushButton* pb = new QPushButton("Align View");
		pv->addWidget(pb);

		pv->addStretch();

		setLayout(pv);

		QObject::connect(node1, SIGNAL(editingFinished()), parent, SLOT(on_change_node1()));
		QObject::connect(node2, SIGNAL(editingFinished()), parent, SLOT(on_change_node2()));
		QObject::connect(node3, SIGNAL(editingFinished()), parent, SLOT(on_change_node3()));

		QObject::connect(pb, SIGNAL(clicked()), parent, SLOT(onAlignView()));
	}
};

CPlaneTool::CPlaneTool(CMainWindow* wnd) : CAbstractTool(wnd, "Plane normal")
{

}

void CPlaneTool::on_change_node1()
{
	ui->m_node[0] = ui->node1->value();
	UpdateNormal();
}

void CPlaneTool::on_change_node2()
{
	ui->m_node[1] = ui->node2->value();
	UpdateNormal();
}

void CPlaneTool::on_change_node3()
{
	ui->m_node[2] = ui->node3->value();
	UpdateNormal();
}

void CPlaneTool::UpdateNormal()
{
	FEMesh* pm = GetActiveMesh();
	if (pm == nullptr) return;

	int* node = ui->m_node;
	if ((node[0] > 0) && (node[1] > 0) && (node[2] > 0))
	{

		FENode& n1 = pm->Node(node[0]-1);
		FENode& n2 = pm->Node(node[1]-1);
		FENode& n3 = pm->Node(node[2]-1);

		vec3d r1 = n1.r;
		vec3d r2 = n2.r;
		vec3d r3 = n3.r;

		vec3d rc = (r1 + r2 + r3)/3.0;

		vec3d e[3];
		e[0] = r2 - r1;
		e[1] = r3 - r1;
		e[2] = e[0] ^ e[1]; 
		e[1] = e[2] ^ e[0];

		e[0].Normalize();
		e[1].Normalize();
		e[2].Normalize();

		ui->normx->setValue(e[2].x);
		ui->normy->setValue(e[2].y);
		ui->normz->setValue(e[2].z);

		CPlaneDecoration* deco = new CPlaneDecoration;
		deco->setPosition(to_vec3f(n1.r), to_vec3f(n2.r), to_vec3f(n3.r));
		SetDecoration(deco);
	}
	else 
	{ 
		ui->normx->setValue(0.0);
		ui->normy->setValue(0.0);
		ui->normz->setValue(0.0);
	}
}

void CPlaneTool::onAlignView()
{
	int* node = ui->m_node;
	if ((node[0] > 0) && (node[1] > 0) && (node[2] > 0))
	{
		CGLView* view = GetMainWindow()->GetGLView();

		vec3f r(ui->normx->value(), ui->normy->value(), ui->normz->value());

		CGLCamera& cam = view->GetCamera();
		cam.SetViewDirection(r);

		GetMainWindow()->RedrawGL();
	}
}

QWidget* CPlaneTool::createUi()
{
	return (ui = new CPlaneToolUI(this));
}


//-----------------------------------------------------------------------------
void CPlaneTool::addPoint(int n)
{
	if (n <= 0) return;

	// see if we have this point already
	for (int i = 0; i < 3; ++i) if (ui->m_node[i] == n) return;

	// we don't so add it to the back
	if (ui->m_node[2] == 0)
	{
		int m = 2;
		while ((m > 0) && (ui->m_node[m - 1] == 0)) m--;
		ui->m_node[m] = n;
	}
	else
	{
		ui->m_node[0] = ui->m_node[1];
		ui->m_node[1] = ui->m_node[2];
		ui->m_node[2] = n;
	}
}

void CPlaneTool::Update()
{
	SetDecoration(nullptr);

	FEMesh* mesh = GetActiveMesh();
	if (mesh == nullptr) return;

	int nsel = 0;
	int N = mesh->Nodes();
	for (int i = 0; i<N; ++i)
	{
		FENode& node = mesh->Node(i);
		if (node.IsSelected())
		{
			nsel++;
			int nid = i + 1;
			addPoint(nid);
		}
	}

	if (nsel == 0)
	{
		ui->m_node[0] = ui->m_node[1] = ui->m_node[2] = 0;
	}

	ui->node1->setValue(ui->m_node[0]);
	ui->node2->setValue(ui->m_node[1]);
	ui->node3->setValue(ui->m_node[2]);

	UpdateNormal();
}
