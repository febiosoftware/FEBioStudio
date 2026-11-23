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
#include "SweepShellTool.h"
#include "ModelDocument.h"
#include "MainWindow.h"
#include "Commands.h"
#include <GeomLib/GObject.h>
#include <GeomLib/GModel.h>
#include <GeomLib/GMeshObject.h>
#include <GLLib/GLMesh.h>
#include <MeshLib/FSMesh.h>
#include <MeshLib/FSLineMesh.h>
#include <MeshTools/FESelection.h>
#include <FECore/FETransform.h>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QApplication>
#include <algorithm>
#include <functional>
#include <map>

class UISweepShellTool : public QWidget
{
public:
	QPushButton* setSection;
	QLabel* sectionLabel;
	QPushButton* setPath;
	QLabel* pathLabel;
	QSpinBox* layers;
	QCheckBox* rotate;
	QCheckBox* autoPartition;
	QDoubleSpinBox* crease;
	QCheckBox* hideSection;
	QPushButton* sweep;

public:
	UISweepShellTool(CSweepShellTool* w)
	{
		QVBoxLayout* l = new QVBoxLayout;

		l->addWidget(new QLabel("1. Select the shell object (cross-section), then:"));
		l->addWidget(setSection = new QPushButton("Set section"));
		l->addWidget(sectionLabel = new QLabel("(none)"));

		l->addWidget(new QLabel("2. Select the edge(s) defining the path, then:"));
		l->addWidget(setPath = new QPushButton("Set path"));
		l->addWidget(pathLabel = new QLabel("(none)"));

		QFormLayout* f = new QFormLayout;
		layers = new QSpinBox; layers->setRange(0, 100000); layers->setValue(0);
		layers->setToolTip("Number of layers along the path. 0 = one layer per path segment.");
		f->addRow("layers (0 = per path segment)", layers);
		rotate = new QCheckBox; rotate->setChecked(true);
		rotate->setToolTip("Rotate the section to follow the path tangent. If off, the section is only translated.");
		f->addRow("rotate section along path", rotate);
		autoPartition = new QCheckBox; autoPartition->setChecked(false);
		f->addRow("auto-partition surfaces", autoPartition);
		crease = new QDoubleSpinBox; crease->setRange(0.1, 179.9); crease->setValue(30.0);
		f->addRow("crease angle (degrees)", crease);
		hideSection = new QCheckBox; hideSection->setChecked(true);
		f->addRow("hide section object", hideSection);
		l->addLayout(f);

		l->addWidget(new QLabel("3. Sweep:"));
		l->addWidget(sweep = new QPushButton("Sweep"));
		l->addStretch();
		setLayout(l);

		QObject::connect(setSection, SIGNAL(clicked()), w, SLOT(OnSetSection()));
		QObject::connect(setPath, SIGNAL(clicked()), w, SLOT(OnSetPath()));
		QObject::connect(sweep, SIGNAL(clicked()), w, SLOT(OnSweep()));
	}
};

CSweepShellTool::CSweepShellTool(CMainWindow* wnd) : CAbstractTool(wnd, "Sweep Shell")
{
	ui = nullptr;
	m_section = nullptr;
	SetInfo("Sweeps a shell mesh (e.g. made with Shell Mesh Faces) along the selected edge(s) into a hex/penta solid.");
}

QWidget* CSweepShellTool::createUi()
{
	if (ui == nullptr) ui = new UISweepShellTool(this);
	return ui;
}

void CSweepShellTool::Deactivate()
{
	CAbstractTool::Deactivate();
}

void CSweepShellTool::Error(const QString& msg)
{
	QMessageBox::warning(GetMainWindow(), "Sweep Shell", msg);
}

bool CSweepShellTool::SectionIsValid()
{
	if (m_section == nullptr) return false;
	CModelDocument* doc = GetMainWindow()->GetModelDocument();
	if ((doc == nullptr) || !doc->IsValid()) return false;
	GModel* gm = doc->GetGModel();
	for (int i = 0; i < gm->Objects(); ++i)
		if (gm->Object(i) == m_section) return true;
	return false;
}

void CSweepShellTool::UpdateLabels()
{
	if (!SectionIsValid()) m_section = nullptr;
	ui->sectionLabel->setText(m_section ? QString::fromStdString(m_section->GetName()) : QString("(none)"));

	if (m_segs.empty()) ui->pathLabel->setText("(none)");
	else
	{
		double L = 0;
		for (const PathSegment& s : m_segs) L += (s.b - s.a).norm();
		ui->pathLabel->setText(QString("%1 segments, length %2").arg((int)m_segs.size()).arg(L));
	}
}

void CSweepShellTool::OnSetSection()
{
	GObject* po = GetActiveObject();
	if ((po == nullptr) || (po->GetFEMesh() == nullptr))
	{
		Error("Select the shell object (an object with a shell mesh) first.");
		return;
	}
	FSMesh* pm = po->GetFEMesh();
	for (int i = 0; i < pm->Elements(); ++i)
	{
		int t = pm->Element(i).Type();
		if ((t != FE_TRI3) && (t != FE_QUAD4))
		{
			Error("The selected object is not a linear shell mesh (triangles/quads only).");
			return;
		}
	}
	m_section = po;
	UpdateLabels();
}

void CSweepShellTool::OnSetPath()
{
	CModelDocument* doc = GetMainWindow()->GetModelDocument();
	if ((doc == nullptr) || !doc->IsValid()) return;

	FESelection* cursel = doc->GetCurrentSelection();
	GEdgeSelection* gsel = dynamic_cast<GEdgeSelection*>(cursel);
	FEEdgeSelection* fsel = dynamic_cast<FEEdgeSelection*>(cursel);

	std::vector<PathSegment> segs;
	if (gsel && (gsel->Count() > 0))
	{
		// geometry edges: use the object's render mesh
		for (int n = 0; n < gsel->Count(); ++n)
		{
			GEdge* pe = gsel->Edge(n);
			GObject* po = (pe ? dynamic_cast<GObject*>(pe->Object()) : nullptr);
			if (po == nullptr) continue;
			GLMesh* gm = po->GetRenderMesh();
			if (gm == nullptr) continue;
			const Transform& T = po->GetTransform();
			// An edge shared by several faces is often tessellated separately by each face, each time with
			// its own nodes and subdivision. Those copies overlap, so group the segments into connected
			// pieces (by node index) and use only the finest piece.
			std::vector<int> segIndex;
			for (int i = 0; i < gm->Edges(); ++i)
				if (gm->Edge(i).pid == pe->GetLocalID()) segIndex.push_back(i);
			if (segIndex.empty()) continue;

			std::map<int, int> parent;
			std::function<int(int)> findRoot = [&](int x) {
				if (parent.find(x) == parent.end()) parent[x] = x;
				while (parent[x] != x) { parent[x] = parent[parent[x]]; x = parent[x]; }
				return x;
			};
			for (int i : segIndex)
			{
				int ra = findRoot(gm->Edge(i).n[0]), rb = findRoot(gm->Edge(i).n[1]);
				if (ra != rb) parent[ra] = rb;
			}
			std::map<int, int> count;
			for (int i : segIndex) count[findRoot(gm->Edge(i).n[0])]++;
			int best = -1, bestCount = 0;
			for (auto& c : count) if (c.second > bestCount) { best = c.first; bestCount = c.second; }

			for (int i : segIndex)
			{
				const GLMesh::EDGE& e = gm->Edge(i);
				if (findRoot(e.n[0]) != best) continue;
				vec3f a = gm->Node(e.n[0]).r, b = gm->Node(e.n[1]).r;
				PathSegment s;
				s.a = T.LocalToGlobal(vec3d(a.x, a.y, a.z));
				s.b = T.LocalToGlobal(vec3d(b.x, b.y, b.z));
				segs.push_back(s);
			}
		}
	}
	else if (fsel && (fsel->Count() > 0))
	{
		// mesh edges
		FSLineMesh* pm = fsel->GetMesh();
		GObject* po = (pm ? pm->GetGObject() : nullptr);
		if (po == nullptr) { Error("Could not find the object of the selected edges."); return; }
		const Transform& T = po->GetTransform();
		FEEdgeSelection::Iterator it(fsel);
		for (int n = 0; n < fsel->Size(); ++n, ++it)
		{
			FSEdge* e = it;
			if (e == nullptr) continue;
			PathSegment s;
			if (e->Nodes() == 3)
			{
				PathSegment s2;
				s.a = T.LocalToGlobal(pm->Node(e->n[0]).r);
				s.b = T.LocalToGlobal(pm->Node(e->n[2]).r);
				s2.a = s.b;
				s2.b = T.LocalToGlobal(pm->Node(e->n[1]).r);
				segs.push_back(s);
				segs.push_back(s2);
			}
			else
			{
				s.a = T.LocalToGlobal(pm->Node(e->n[0]).r);
				s.b = T.LocalToGlobal(pm->Node(e->n[1]).r);
				segs.push_back(s);
			}
		}
	}
	else
	{
		Error("No edges are selected. Select one or more edges (curves) that define the sweep path.");
		return;
	}

	if (segs.empty())
	{
		Error("Could not get any path segments from the selected edges.");
		return;
	}
	m_segs = segs;
	UpdateLabels();
}

void CSweepShellTool::OnSweep()
{
	CModelDocument* doc = GetMainWindow()->GetModelDocument();
	if ((doc == nullptr) || !doc->IsValid()) return;

	if (!SectionIsValid()) { m_section = nullptr; UpdateLabels(); Error("Set the section (shell object) first."); return; }
	if (m_segs.empty()) { Error("Set the path (select edges and press 'Set path') first."); return; }
	FSMesh* shell = m_section->GetFEMesh();
	if (shell == nullptr) { Error("The section object has no mesh."); return; }

	// section center (global) --> choose the path end closest to it as the start
	vec3d c(0, 0, 0);
	for (int i = 0; i < shell->Nodes(); ++i) c += shell->Node(i).r;
	c /= (double)std::max(1, shell->Nodes());
	c = m_section->GetTransform().LocalToGlobal(c);

	ShellSweeper sw;
	std::string err;
	if (!ShellSweeper::ChainSegments(m_segs, c, sw.m_path, err))
	{
		Error(QString::fromStdString(err));
		return;
	}
	sw.m_layers = ui->layers->value();
	sw.m_rotate = ui->rotate->isChecked();
	sw.m_autoPartition = ui->autoPartition->isChecked();
	sw.m_creaseAngle = ui->crease->value();

	QApplication::setOverrideCursor(Qt::WaitCursor);
	FSMesh* solid = sw.Apply(*shell, m_section->GetTransform());
	QApplication::restoreOverrideCursor();
	if (solid == nullptr)
	{
		Error("Sweep failed: " + QString::fromStdString(sw.GetErrorString()));
		return;
	}

	static int n = 1;
	QString name = QString("%1_sweep%2").arg(QString::fromStdString(m_section->GetName())).arg(n++);
	GMeshObject* po = new GMeshObject(solid);
	po->SetName(name.toStdString());

	CCmdGroup* cmd = new CCmdGroup("Sweep shell");
	cmd->AddCommand(new CCmdAddAndSelectObject(doc->GetGModel(), po));
	if (ui->hideSection->isChecked()) cmd->AddCommand(new CCmdHideObject(m_section));
	doc->DoCommand(cmd, name.toStdString());

	QMessageBox::information(GetMainWindow(), "Sweep Shell", QString("Created %1 (%2 elements)").arg(name).arg(solid->Elements()));
}
