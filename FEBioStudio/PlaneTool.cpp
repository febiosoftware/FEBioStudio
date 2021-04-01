/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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

#include "stdafx.h"
#include "PlaneTool.h"
#include "CIntInput.h"
#include <QBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QStackedWidget>
#include "Document.h"
#include <PostGL/GLModel.h>
#include <PostLib/FEPostMesh.h>
#include "MainWindow.h"
#include "GLView.h"
#include "DragBox.h"
using namespace Post;

class CPlaneToolUI : public QWidget
{
public:
	int		m_node[3];
	vec3d	m_pos[3];
	FEMeshBase*	m_mesh;
	BOX		m_box;
	double	m_range[2];

public:
	QComboBox*	input;
	QStackedWidget* stack;

	// node input
	CIntInput* 	node[3];

	// plane input
	CDragBox*	plane[4];

	// plane output
	QLineEdit* norm[3];
	QLineEdit* off;

public:
	CPlaneToolUI(QObject* parent)
	{
		m_node[0] = 0;
		m_node[1] = 0;
		m_node[2] = 0;

		m_mesh = nullptr;

		QVBoxLayout* pv = new QVBoxLayout;

		input = new QComboBox;
		input->addItem("Select nodes");
		input->addItem("Plane coordinates");
//		input->addItem("Spherical angles/offset");
		pv->addWidget(input);

		stack = new QStackedWidget;

		// select nodes input
		QGroupBox* pg = new QGroupBox;
		QFormLayout* pform = new QFormLayout;
		pform->addRow("node 1:", node[0] = new CIntInput);
		pform->addRow("node 2:", node[1] = new CIntInput);
		pform->addRow("node 3:", node[2] = new CIntInput);
		pg->setLayout(pform);
		stack->addWidget(pg);

		// plane coordinates input
		pg = new QGroupBox;
		pform = new QFormLayout;
		pform->addRow("a:", plane[0] = new CDragBox); plane[0]->setRange(-1, 1); plane[0]->setSingleStep(0.01);
		pform->addRow("b:", plane[1] = new CDragBox); plane[1]->setRange(-1, 1); plane[1]->setSingleStep(0.01);
		pform->addRow("c:", plane[2] = new CDragBox); plane[2]->setRange(-1, 1); plane[2]->setSingleStep(0.01);
		pform->addRow("d:", plane[3] = new CDragBox); plane[3]->setRange(-1, 1); plane[3]->setSingleStep(0.01);
		pg->setLayout(pform);
		stack->addWidget(pg);

		// add the stack widget
		pv->addWidget(stack);

		// plane definition output
		pg = new QGroupBox("Plane definition:");
		pform = new QFormLayout;
		pform->addRow("x:", norm[0] = new QLineEdit); norm[0]->setReadOnly(true);
		pform->addRow("y:", norm[1] = new QLineEdit); norm[1]->setReadOnly(true);
		pform->addRow("z:", norm[2] = new QLineEdit); norm[2]->setReadOnly(true);
		pform->addRow("d:", off   = new QLineEdit); off->setReadOnly(true);
		pg->setLayout(pform);
		pv->addWidget(pg);

		// align view push button
		QPushButton* pb = new QPushButton("Align View");
		pv->addWidget(pb);

		pv->addStretch();

		setLayout(pv);

		QObject::connect(node[0], SIGNAL(editingFinished()), parent, SLOT(onNodeChanged()));
		QObject::connect(node[1], SIGNAL(editingFinished()), parent, SLOT(onNodeChanged()));
		QObject::connect(node[2], SIGNAL(editingFinished()), parent, SLOT(onNodeChanged()));

		QObject::connect(plane[0], SIGNAL(valueChanged(double)), parent, SLOT(onPlaneChanged()));
		QObject::connect(plane[1], SIGNAL(valueChanged(double)), parent, SLOT(onPlaneChanged()));
		QObject::connect(plane[2], SIGNAL(valueChanged(double)), parent, SLOT(onPlaneChanged()));
		QObject::connect(plane[3], SIGNAL(valueChanged(double)), parent, SLOT(onPlaneChanged()));

		QObject::connect(input, SIGNAL(currentIndexChanged(int)), stack, SLOT(setCurrentIndex(int)));

		QObject::connect(pb, SIGNAL(clicked()), parent, SLOT(onAlignView()));
	}

	int inputOption()
	{
		return input->currentIndex();
	}

	void SetNormal(const vec3d& r)
	{
		norm[0]->setText(QString::number(r.x));
		norm[1]->setText(QString::number(r.y));
		norm[2]->setText(QString::number(r.z));
	}

	vec3d GetNormal() const
	{
		double x = norm[0]->text().toDouble();
		double y = norm[1]->text().toDouble();
		double z = norm[2]->text().toDouble();
		return vec3d(x, y, z);
	}

	void SetOffset(double d)
	{
		off->setText(QString::number(d));
	}

	double GetOffset() const
	{
		double d = off->text().toDouble();
		return d;
	}

	void SetPoints(const vec3d& r1, const vec3d& r2, const vec3d& r3)
	{
		m_pos[0] = r1;
		m_pos[1] = r2;
		m_pos[2] = r3;
	}

	void GetPlaneCoordinates(double a[4])
	{
		a[0] = plane[0]->value();
		a[1] = plane[1]->value();
		a[2] = plane[2]->value();
		a[3] = plane[3]->value();
	}

	void SetPlaneCoordinates(double nx, double ny, double nz, double d)
	{
		plane[0]->setValue(nx);
		plane[1]->setValue(ny);
		plane[2]->setValue(nz);
		plane[3]->setValue(d);
	}

	void UpdatePlaneOffsetMinMax();
};

CPlaneTool::CPlaneTool(CMainWindow* wnd) : CAbstractTool(wnd, "Plane")
{

}

void CPlaneTool::onNodeChanged()
{
	ui->m_node[0] = ui->node[0]->value();
	ui->m_node[1] = ui->node[1]->value();
	ui->m_node[2] = ui->node[2]->value();

	FEMeshBase* pm = ui->m_mesh;
	if (pm == nullptr) return;

	int* node = ui->m_node;
	if ((node[0] > 0) && (node[1] > 0) && (node[2] > 0))
	{
		FENode& n1 = pm->Node(node[0] - 1);
		FENode& n2 = pm->Node(node[1] - 1);
		FENode& n3 = pm->Node(node[2] - 1);

		// get the nodal positions
		vec3d r1 = pm->GlobalToLocal(n1.r);
		vec3d r2 = pm->GlobalToLocal(n2.r);
		vec3d r3 = pm->GlobalToLocal(n3.r);
		ui->SetPoints(r1, r2, r3);

		// calculate the normal
		vec3d e[2], N;
		e[0] = r2 - r1;
		e[1] = r3 - r1;
		N = e[0] ^ e[1];
		N.Normalize();
		ui->SetNormal(N);

		// calculate offset
		double d = N * r1;
		ui->SetOffset(d);

		// update the plane offset range
		ui->UpdatePlaneOffsetMinMax();

		double d0 = ui->m_range[0];
		double d1 = ui->m_range[1];
		double dr = 2.0*(d - d0) / (d1 - d0) - 1.0;

		// update the plane coordinates
		ui->SetPlaneCoordinates(N.x, N.y, N.z, dr);

		// add a decoration
		GPlaneCutDecoration* planeCut = new GPlaneCutDecoration;
		planeCut->setColor(GLColor(255, 255, 0, 128));
		planeCut->setColor2(GLColor(255, 255, 0));
		planeCut->setBoundingBox(ui->m_box);
		planeCut->setPlane(N.x, N.y, N.z, d);
		GCompositeDecoration* deco = new GCompositeDecoration;
		deco->AddDecoration(new GPointDecoration(to_vec3f(r1)));
		deco->AddDecoration(new GPointDecoration(to_vec3f(r2)));
		deco->AddDecoration(new GPointDecoration(to_vec3f(r3)));
		deco->AddDecoration(planeCut);
		SetDecoration(deco);
	}
	else
	{
		SetDecoration(nullptr);
		ui->SetNormal(vec3d(0, 0, 0));
		ui->SetOffset(0);
	}
}

void CPlaneTool::onPlaneChanged()
{
	if (ui->inputOption() != 1) return;

	double a[4];
	ui->GetPlaneCoordinates(a);

	vec3d N(a[0], a[1], a[2]); 
	N.Normalize();

	ui->SetNormal(N);
	ui->UpdatePlaneOffsetMinMax();

	double w = a[3];
	double w0 = ui->m_range[0];
	double w1 = ui->m_range[1];
	double d = 0.5*w0 * (1.0 - w) + 0.5*w1 * (1.0 + w);
	ui->SetOffset(d);

	// add a decoration
	GPlaneCutDecoration* deco = new GPlaneCutDecoration;
	deco->setColor(GLColor(255, 255, 0, 128));
	deco->setColor2(GLColor(255, 255, 0));
	deco->setBoundingBox(ui->m_box);
	deco->setPlane(N.x, N.y, N.z, d);
	SetDecoration(deco);
}

void CPlaneToolUI::UpdatePlaneOffsetMinMax()
{
	// get the nodal values
	BOX& box = m_box;
	vec3d a = box.r0();
	vec3d b = box.r1();
	vec3d r[8] = {
		vec3d(a.x, a.y, a.z),
		vec3d(b.x, a.y, a.z),
		vec3d(b.x, b.y, a.z),
		vec3d(a.x, b.y, a.z),
		vec3d(a.x, a.y, b.z),
		vec3d(b.x, a.y, b.z),
		vec3d(b.x, b.y, b.z),
		vec3d(a.x, b.y, b.z)
	};
	vec3d n = GetNormal();
	double dmin, dmax;
	for (int k = 0; k < 8; ++k)
	{
		double dk = r[k] * n;
		if ((k == 0) || (dk < dmin)) dmin = dk;
		if ((k == 0) || (dk > dmax)) dmax = dk;
	}
	if (dmin == dmax) dmax += 1.0;
	m_range[0] = dmin;
	m_range[1] = dmax;
}

void CPlaneTool::onAlignView()
{
	CGLDocument* doc = GetMainWindow()->GetGLDocument();
	if (doc == nullptr) return;

	int* node = ui->m_node;
	if ((node[0] > 0) && (node[1] > 0) && (node[2] > 0))
	{
		vec3f r = to_vec3f(ui->GetNormal());

		CGLCamera& cam = doc->GetView()->GetCamera();
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
	// set the active mesh
	FEMeshBase* mesh = GetActiveEditMesh();
	ui->m_mesh = mesh;
	if (mesh == nullptr)
	{
		SetDecoration(nullptr);
		return;
	}

	ui->m_box = mesh->GetBoundingBox();

	if (ui->inputOption() == 0)
	{
		int nsel = 0;
		int N = mesh->Nodes();
		for (int i = 0; i < N; ++i)
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

		ui->node[0]->setValue(ui->m_node[0]);
		ui->node[1]->setValue(ui->m_node[1]);
		ui->node[2]->setValue(ui->m_node[2]);

		onNodeChanged();
	}
}
