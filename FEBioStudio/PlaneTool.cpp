/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
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
#include <CUILib/InputWidgets.h>
#include <QApplication>
#include <QClipboard>
#include <QBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QComboBox>
#include <QStackedWidget>
#include "GLDocument.h"
#include <PostGL/GLModel.h>
#include "MainWindow.h"
#include "DragBox.h"
using namespace Post;

class CPlaneToolUI : public QWidget
{
public:
	FSMeshBase*	m_mesh;		// selected mesh
	BOX		m_box;			// bounding box of mesh
	double	m_range[2];		// offset range defined by box and current plane normal

public:
	QComboBox*	input;
	QStackedWidget* stack;

	// point coordinates
	QLineEdit*	 pt[3];
	QPushButton* pick[3];

	// plane input
	CDragBox*	plane[4];


	// plane output
	QLineEdit* norm[3];
	QLineEdit* off;

public:
	CPlaneToolUI(QObject* parent)
	{
		m_mesh = nullptr;

		QVBoxLayout* pv = new QVBoxLayout;

		input = new QComboBox;
		input->addItem("Point coordinates");
		input->addItem("Plane coordinates");
		pv->addWidget(input);

		stack = new QStackedWidget;

		// point coordinates input
		QGroupBox* pg = new QGroupBox;
		QFormLayout* pform = new QFormLayout;

		for (int i = 0; i < 3; ++i)
		{
			pt[i] = new QLineEdit;
			pick[i] = new QPushButton("Pick");
			pick[i]->setCheckable(true);
			QHBoxLayout* h = new QHBoxLayout;
			h->setContentsMargins(0,0,0,0);
			h->addWidget(pt[i]);
			h->addWidget(pick[i]);
			pform->addRow(QString("point %1:").arg(i+1), h);
		}
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
		pform->addRow("Normal-x:", norm[0] = new QLineEdit); norm[0]->setReadOnly(true);
		pform->addRow("Normal-y:", norm[1] = new QLineEdit); norm[1]->setReadOnly(true);
		pform->addRow("Normal-z:", norm[2] = new QLineEdit); norm[2]->setReadOnly(true);
		pform->addRow("Offset:", off   = new QLineEdit); off->setReadOnly(true);
		pg->setLayout(pform);
		pv->addWidget(pg);

		// align view push button
		QHBoxLayout* h = new QHBoxLayout;
		QPushButton* alignView = new QPushButton("Align View");
		h->addWidget(alignView);

		// export to clipboard
		QPushButton* copy = new QPushButton("Copy to Clipboard");
		h->addWidget(copy);

		pv->addLayout(h);

		pv->addStretch();

		setLayout(pv);

		// initialize the UI to valid input
		SetPoints(vec3d(0, 0, 0), vec3d(1, 0, 0), vec3d(0, 1, 0));
		SetPlaneCoordinates(0, 0, 1, 0);

		QObject::connect(pt[0], SIGNAL(editingFinished()), parent, SLOT(onNodeChanged()));
		QObject::connect(pt[1], SIGNAL(editingFinished()), parent, SLOT(onNodeChanged()));
		QObject::connect(pt[2], SIGNAL(editingFinished()), parent, SLOT(onNodeChanged()));

		QObject::connect(plane[0], SIGNAL(valueChanged(double)), parent, SLOT(onPlaneChanged()));
		QObject::connect(plane[1], SIGNAL(valueChanged(double)), parent, SLOT(onPlaneChanged()));
		QObject::connect(plane[2], SIGNAL(valueChanged(double)), parent, SLOT(onPlaneChanged()));
		QObject::connect(plane[3], SIGNAL(valueChanged(double)), parent, SLOT(onPlaneChanged()));

		QObject::connect(pick[0], SIGNAL(toggled(bool)), parent, SLOT(onEditToggled(bool)));
		QObject::connect(pick[1], SIGNAL(toggled(bool)), parent, SLOT(onEditToggled(bool)));
		QObject::connect(pick[2], SIGNAL(toggled(bool)), parent, SLOT(onEditToggled(bool)));

		QObject::connect(input, SIGNAL(currentIndexChanged(int)), stack, SLOT(setCurrentIndex(int)));

		QObject::connect(alignView, SIGNAL(clicked()), parent, SLOT(onAlignView()));
		QObject::connect(copy     , SIGNAL(clicked()), parent, SLOT(onCopy()));
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

	void SetPoint(int i, const vec3d& r)
	{
		pt[i]->setText(Vec3dToString(r));
	}

	vec3d GetPoint(int i)
	{
		return StringToVec3d(pt[i]->text());
	}

	void SetPoints(const vec3d& r1, const vec3d& r2, const vec3d& r3)
	{
		SetPoint(0, r1);
		SetPoint(1, r2);
		SetPoint(2, r3);
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

	void UpdatePlaneOffsetRange();

	int activeEditField()
	{
		if (pick[0]->isChecked()) return 0;
		if (pick[1]->isChecked()) return 1;
		if (pick[2]->isChecked()) return 2;
		return -1;
	}
};

CPlaneTool::CPlaneTool(CMainWindow* wnd) : CAbstractTool(wnd, "Plane")
{
	SetInfo("Tool to calculate plane based on three nodes.");
}

void CPlaneTool::onEditToggled(bool b)
{
	QPushButton* pb = dynamic_cast<QPushButton*>(QObject::sender()); assert(pb);
	if (pb == nullptr) return;
	if (b)
	{
		// make sure only one button is toggled
		if (ui->pick[0] != pb) ui->pick[0]->setChecked(false);
		if (ui->pick[1] != pb) ui->pick[1]->setChecked(false);
		if (ui->pick[2] != pb) ui->pick[2]->setChecked(false);
	}
}

void CPlaneTool::onNodeChanged()
{
	// get the nodal positions
	vec3d r1 = ui->GetPoint(0);
	vec3d r2 = ui->GetPoint(1);
	vec3d r3 = ui->GetPoint(2);

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
	ui->UpdatePlaneOffsetRange();

	double d0 = ui->m_range[0];
	double d1 = ui->m_range[1];
	double dr = 2.0*(d - d0) / (d1 - d0) - 1.0;

	// update the plane coordinates
	ui->SetPlaneCoordinates(N.x, N.y, N.z, dr);

	// add a decoration
	UpdateDecoration();
}

void CPlaneTool::UpdateDecoration()
{
	vec3d r[3];
	r[0] = ui->GetPoint(0);
	r[1] = ui->GetPoint(1);
	r[2] = ui->GetPoint(2);
	vec3d N = ui->GetNormal();
	double d = ui->GetOffset();
	GPlaneCutDecoration* planeCut = new GPlaneCutDecoration;
	planeCut->setColor(GLColor(255, 255, 0, 128));
	planeCut->setColor2(GLColor(255, 255, 0));
	planeCut->setBoundingBox(ui->m_box);
	planeCut->setPlane(N.x, N.y, N.z, d);
	GCompositeDecoration* deco = new GCompositeDecoration;
	for (int i = 0; i < 3; ++i)
	{
		GPointDecoration* pdeco = new GPointDecoration(to_vec3f(r[i]));
		if (i == 0) pdeco->setColor(GLColor(255, 0, 0));
		if (i == 1) pdeco->setColor(GLColor(0, 255, 0));
		if (i == 2) pdeco->setColor(GLColor(0, 0, 255));
		pdeco->setColor2(GLColor::White());
		pdeco->renderAura(true);
		deco->AddDecoration(pdeco);
	}
	deco->AddDecoration(planeCut);
	SetDecoration(deco);
}

void CPlaneTool::onPlaneChanged()
{
	if (ui->inputOption() != 1) return;

	double a[4];
	ui->GetPlaneCoordinates(a);

	vec3d N(a[0], a[1], a[2]); 
	N.Normalize();

	ui->SetNormal(N);
	ui->UpdatePlaneOffsetRange();

	double w = a[3];
	double w0 = ui->m_range[0];
	double w1 = ui->m_range[1];
	double d = 0.5*w0 * (1.0 - w) + 0.5*w1 * (1.0 + w);
	ui->SetOffset(d);

	// re-position the points
	vec3d r1 = ui->GetPoint(0);
	vec3d r2 = ui->GetPoint(1);
	vec3d r3 = ui->GetPoint(2);

	vec3d e1 = r2 - r1;
	vec3d e2 = r3 - r1;
	vec3d N0 = e1 ^ e2;
	N0.Normalize();

	quatd Q(N0, N);
	Q.RotateVector(e1);
	Q.RotateVector(e2);

	r1 = r1 - N*(r1*N - d);
	r2 = r1 + e1;
	r3 = r1 + e2;
	ui->SetPoints(r1, r2, r3);

	// add a decoration
	UpdateDecoration();
}

void CPlaneToolUI::UpdatePlaneOffsetRange()
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

	vec3d r = ui->GetNormal();

	GLCamera& cam = doc->GetView()->GetCamera();
	cam.SetViewDirection(r);

	GetMainWindow()->RedrawGL();
}

void CPlaneTool::onCopy()
{
	QClipboard* clipboard = QApplication::clipboard();

	vec3d r1 = ui->GetPoint(0);
	vec3d r2 = ui->GetPoint(1);
	vec3d r3 = ui->GetPoint(2);
	vec3d N = ui->GetNormal();
	double d = ui->GetOffset();

	QString s;
	s += Vec3dToString(r1) + QString("\n");
	s += Vec3dToString(r2) + QString("\n");
	s += Vec3dToString(r3) + QString("\n");
	s += QString("%1, %2, %3, %4\n").arg(N.x).arg(N.y).arg(N.z).arg(d);

	clipboard->setText(s);
}

QWidget* CPlaneTool::createUi()
{
	return (ui = new CPlaneToolUI(this));
}

//-----------------------------------------------------------------------------
void CPlaneTool::Update()
{
	FSMeshBase* mesh = GetActiveEditMesh();
	if (mesh == nullptr)
	{
		ui->m_mesh = nullptr;
		SetDecoration(nullptr);
		return;
	}

	if ((ui->m_mesh == nullptr) || (mesh != ui->m_mesh))
	{
		ui->m_mesh = mesh;

		// the mesh returns the local box
		BOX box = mesh->GetBoundingBox();

		// we want a global box
		vec3d r0 = mesh->LocalToGlobal(box.r0());
		vec3d r1 = mesh->LocalToGlobal(box.r1());
		if (r1.x < r0.x) { double x = r1.x; r1.x = r0.x; r0.x = x; }
		if (r1.y < r0.y) { double y = r1.y; r1.y = r0.y; r0.y = y; }
		if (r1.z < r0.z) { double z = r1.z; r1.z = r0.z; r0.z = z; }
		ui->m_box = BOX(r0, r1);
		ui->UpdatePlaneOffsetRange();
	}

	if (ui->inputOption() == 0)
	{
		int activeField = ui->activeEditField();
		if (activeField == -1)
		{
			int nsel = 0;
			int N = mesh->Nodes();
			for (int i = 0; i < N; ++i)
			{
				FSNode& node = mesh->Node(i);
				if (node.IsSelected())
				{
					vec3d r = mesh->LocalToGlobal(node.pos());
					ui->SetPoint(nsel, r);
					nsel++;
					if (nsel == 3) break;
				}
			}
		}
		else
		{
			int selNode = -1;
			int N = mesh->Nodes();
			for (int i = 0; i < N; ++i)
			{
				FSNode& node = mesh->Node(i);
				if (node.IsSelected())
				{
					vec3d r = mesh->LocalToGlobal(node.pos());
					ui->SetPoint(activeField, r);
					break;
				}
			}
		}

		onNodeChanged();
	}
}
