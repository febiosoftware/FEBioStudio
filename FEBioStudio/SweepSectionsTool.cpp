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
#include "SweepSectionsTool.h"
#include "ModelDocument.h"
#include "MainWindow.h"
#include "Commands.h"
#include <GeomLib/GObject.h>
#include <GeomLib/GModel.h>
#include <GeomLib/GMeshObject.h>
#include <MeshLib/FSMesh.h>
#include <MeshLib/FSFace.h>
#include <GLLib/GLMesh.h>
#include <MeshTools/FESelection.h>
#include <MeshTools/ShellSweeper.h>
#include <FECore/FETransform.h>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QApplication>
#include <algorithm>
#include <functional>
#include <map>
#include <cmath>
#include <tuple>

class UISweepSectionsTool : public QWidget
{
public:
	QPushButton* setStart;
	QLabel* startLabel;
	QPushButton* setTargets;
	QPushButton* addTargets;
	QPushButton* clearTargets;
	QLabel* targetLabel;
	QSpinBox* layers;
	QComboBox* tangentMode;
	QCheckBox* matchOrientation;
	QCheckBox* scale;
	QCheckBox* autoPartition;
	QDoubleSpinBox* crease;
	QDoubleSpinBox* collapseTol;
	QComboBox* original;
	QComboBox* targetObjs;
	QPushButton* sweep;

public:
	UISweepSectionsTool(CSweepSectionsTool* w)
	{
		QVBoxLayout* l = new QVBoxLayout;

		l->addWidget(new QLabel("1. Select the start surface (or faces), then:"));
		l->addWidget(setStart = new QPushButton("Set start section"));
		l->addWidget(startLabel = new QLabel("(none)"));

		l->addWidget(new QLabel("2. Select the target surface(s) / faces to sweep across, then:"));
		QHBoxLayout* h = new QHBoxLayout;
		h->addWidget(setTargets = new QPushButton("Set"));
		h->addWidget(addTargets = new QPushButton("Add"));
		h->addWidget(clearTargets = new QPushButton("Clear"));
		l->addLayout(h);
		l->addWidget(targetLabel = new QLabel("(none)"));

		QFormLayout* f = new QFormLayout;
		layers = new QSpinBox; layers->setRange(1, 10000); layers->setValue(10);
		layers->setToolTip("Number of layers between consecutive sections.");
		f->addRow("layers between sections", layers);
		tangentMode = new QComboBox;
		tangentMode->addItems(QStringList() << "section normals" << "smooth curve through centroids");
		tangentMode->setToolTip("section normals: each section is swept perpendicular to the target section's plane (the path tangent equals "
			"the section normal). smooth curve: the path is a smooth curve through the centroids and the target orientations are ignored; "
			"use this if the target sections are tilted irregularly relative to the line joining them.");
		f->addRow("path tangent", tangentMode);
		matchOrientation = new QCheckBox; matchOrientation->setChecked(true);
		matchOrientation->setToolTip("Only for the 'smooth curve' path: tilt the swept section so that it lies exactly in the plane of each "
			"target section (the layers are then not necessarily perpendicular to the path). If this folds elements, switch it off.");
		f->addRow("match target planes (smooth path)", matchOrientation);
		scale = new QCheckBox; scale->setChecked(true);
		scale->setToolTip("Scale the swept section so its size matches each target section (sqrt of area ratio).");
		f->addRow("scale to match targets", scale);
		autoPartition = new QCheckBox; autoPartition->setChecked(false);
		f->addRow("auto-partition surfaces", autoPartition);
		crease = new QDoubleSpinBox; crease->setRange(0.1, 179.9); crease->setValue(30.0);
		f->addRow("crease angle (degrees)", crease);
		collapseTol = new QDoubleSpinBox; collapseTol->setDecimals(10); collapseTol->setRange(0.0, 1e12); collapseTol->setValue(0.0);
		collapseTol->setSpecialValueText("auto");
		f->addRow("degeneracy tolerance", collapseTol);
		original = new QComboBox;
		original->addItems(QStringList() << "keep" << "hide" << "delete (replace with swept mesh)");
		original->setCurrentIndex(2);
		original->setToolTip("What to do with the object that contains the start section. Hidden objects are still part of the model "
			"(and are written to the FEBio file); deleting can be undone with Undo.");
		f->addRow("original object", original);
		targetObjs = new QComboBox;
		targetObjs->addItems(QStringList() << "keep" << "hide" << "delete");
		targetObjs->setCurrentIndex(1);
		targetObjs->setToolTip("What to do with the objects that contain the target sections (the whole object is hidden or deleted, so "
			"only use delete if those objects are just the cross-section disks). Deleting can be undone with Undo.");
		f->addRow("target section objects", targetObjs);
		l->addLayout(f);

		l->addWidget(new QLabel("3. Sweep:"));
		l->addWidget(sweep = new QPushButton("Sweep"));
		l->addStretch();
		setLayout(l);

		QObject::connect(setStart, SIGNAL(clicked()), w, SLOT(OnSetStart()));
		QObject::connect(setTargets, SIGNAL(clicked()), w, SLOT(OnSetTargets()));
		QObject::connect(addTargets, SIGNAL(clicked()), w, SLOT(OnAddTargets()));
		QObject::connect(clearTargets, SIGNAL(clicked()), w, SLOT(OnClearTargets()));
		QObject::connect(sweep, SIGNAL(clicked()), w, SLOT(OnSweep()));
	}
};

CSweepSectionsTool::CSweepSectionsTool(CMainWindow* wnd) : CAbstractTool(wnd, "Sweep Sections")
{
	ui = nullptr;
	m_haveStart = false;
	SetInfo("Sweeps a meshed cross-section (a selected surface or faces) through other selected cross-sections into a hex/penta "
		"solid mesh. Only the mesh of the start section is used; the other sections define the path, orientation and size.");
}

QWidget* CSweepSectionsTool::createUi()
{
	if (ui == nullptr) ui = new UISweepSectionsTool(this);
	return ui;
}

void CSweepSectionsTool::Error(const QString& msg)
{
	QMessageBox::warning(GetMainWindow(), "Sweep Sections", msg);
}

bool CSweepSectionsTool::ObjectExists(GObject* po)
{
	if (po == nullptr) return false;
	CModelDocument* doc = GetMainWindow()->GetModelDocument();
	if ((doc == nullptr) || !doc->IsValid()) return false;
	GModel* gm = doc->GetGModel();
	for (int i = 0; i < gm->Objects(); ++i) if (gm->Object(i) == po) return true;
	return false;
}

void CSweepSectionsTool::UpdateLabels()
{
	if (m_haveStart && !ObjectExists(m_start.po)) m_haveStart = false;
	if (m_haveStart)
		ui->startLabel->setText(QString("%1: %2 faces, area %3").arg(QString::fromStdString(m_start.po->GetName())).arg(m_start.wholeObject ? QString("all") : QString::number((int)m_start.faces.size())).arg(m_start.area));
	else ui->startLabel->setText("(none)");

	std::vector<Section> keep;
	for (const Section& s : m_targets) if (ObjectExists(s.po)) keep.push_back(s);
	m_targets = keep;
	ui->targetLabel->setText(m_targets.empty() ? QString("(none)") : QString("%1 section(s)").arg((int)m_targets.size()));
}

namespace {
	int FaceCorners(const FSFace& f)
	{
		switch (f.Type()) {
		case FE_FACE_QUAD4:
		case FE_FACE_QUAD8:
		case FE_FACE_QUAD9: return 4;
		default: return 3;
		}
	}
}

// Collect the sections (connected sets of mesh faces) of the current selection.
bool CSweepSectionsTool::GetSections(std::vector<Section>& sections, bool forStart, QString& err)
{
	sections.clear();
	CModelDocument* doc = GetMainWindow()->GetModelDocument();
	if ((doc == nullptr) || !doc->IsValid()) { err = "No valid model document."; return false; }

	FESelection* cursel = doc->GetCurrentSelection();
	GFaceSelection* gsel = dynamic_cast<GFaceSelection*>(cursel);
	FEFaceSelection* fsel = dynamic_cast<FEFaceSelection*>(cursel);

	// object --> selected face indices
	std::map<GObject*, std::vector<int>> faceSets;
	std::map<GObject*, std::vector<int>> surfSets;		// unmeshed objects: selected surfaces
	if (gsel && (gsel->Count() > 0))
	{
		for (int n = 0; n < gsel->Count(); ++n)
		{
			GFace* gf = gsel->Face(n);
			GObject* po = (gf ? dynamic_cast<GObject*>(gf->Object()) : nullptr);
			if (po == nullptr) continue;
			FSMesh* pm = po->GetFEMesh();
			if ((pm == nullptr) || (pm->Faces() == 0))
			{
				if (forStart)
				{
					err = "The selected surface belongs to an object that is not meshed, so it cannot be the start section. "
						"To use a meshed surface that is a separate object (e.g. made with Shell Mesh Faces), select that object "
						"in the model tree instead of picking its surface in the viewport (hide the overlapping object if needed).";
					return false;
				}
				surfSets[po].push_back(gf->GetLocalID());
				continue;
			}
			std::vector<int>& v = faceSets[po];
			for (int i = 0; i < pm->Faces(); ++i)
				if (pm->Face(i).m_gid == gf->GetLocalID()) v.push_back(i);
		}
	}
	else if (fsel && (fsel->Count() > 0))
	{
		FSMeshBase* pm = fsel->GetMesh();
		GObject* po = (pm ? pm->GetGObject() : nullptr);
		if (po == nullptr) { err = "Could not find the object of the selected faces."; return false; }
		std::vector<int>& v = faceSets[po];
		for (int n = 0; n < fsel->Size(); ++n) v.push_back(fsel->FaceIndex(n));
	}
	else
	{
		// no faces selected: use the active object if it is a shell mesh (e.g. made with Shell Mesh Faces)
		GObject* po = GetActiveObject();
		FSMesh* pm = (po ? po->GetFEMesh() : nullptr);
		if ((pm == nullptr) || (pm->Elements() == 0))
		{
			err = "Select a surface or faces, or select a shell object (an object with a shell mesh) in the model tree.";
			return false;
		}
		const Transform& T = po->GetTransform();
		Section s;
		s.po = po;
		s.wholeObject = true;
		vec3d csum(0, 0, 0);
		for (int i = 0; i < pm->Elements(); ++i)
		{
			const FSElement& el = pm->Element(i);
			int nc = 0;
			switch (el.Type()) {
			case FE_TRI3: case FE_TRI6: nc = 3; break;
			case FE_QUAD4: case FE_QUAD8: nc = 4; break;
			default:
				err = "The selected object is not a shell mesh of TRI3, QUAD4, TRI6 or QUAD8 elements. "
					"Select a surface (or faces) of a solid mesh instead.";
				return false;
			}
			vec3d a = T.LocalToGlobal(pm->Node(el.m_node[0]).r), b = T.LocalToGlobal(pm->Node(el.m_node[1]).r), c = T.LocalToGlobal(pm->Node(el.m_node[2]).r);
			vec3d an = ((b - a) ^ (c - a)) * 0.5;
			double w = an.norm();
			s.n += an; s.area += w; csum += (a + b + c) * (w / 3.0);
			if (nc == 4)
			{
				vec3d d = T.LocalToGlobal(pm->Node(el.m_node[3]).r);
				vec3d bn = ((c - a) ^ (d - a)) * 0.5;
				double w2 = bn.norm();
				s.n += bn; s.area += w2; csum += (a + c + d) * (w2 / 3.0);
			}
		}
		if (s.area <= 0.0) { err = "The selected object has zero area."; return false; }
		s.c = csum / s.area;
		sections.push_back(s);
		return true;
	}

	// unmeshed surfaces: use the geometry (render mesh); surfaces that touch each other form one section
	for (auto& ss : surfSets)
	{
		GObject* po = ss.first;
		GLMesh* gm = po->GetRenderMesh();
		if (gm == nullptr) { err = "The selected surface has no geometry to work with."; return false; }
		const Transform& T = po->GetTransform();
		std::vector<int>& ids = ss.second;
		std::sort(ids.begin(), ids.end());
		ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
		const int NS = (int)ids.size();
		std::map<int, int> sIndex;
		for (int i = 0; i < NS; ++i) sIndex[ids[i]] = i;

		// bounding size for the touching tolerance
		vec3d lo(1e300, 1e300, 1e300), hi(-1e300, -1e300, -1e300);
		for (int i = 0; i < gm->Nodes(); ++i)
		{
			vec3f r = gm->Node(i).r;
			lo.x = std::min(lo.x, (double)r.x); lo.y = std::min(lo.y, (double)r.y); lo.z = std::min(lo.z, (double)r.z);
			hi.x = std::max(hi.x, (double)r.x); hi.y = std::max(hi.y, (double)r.y); hi.z = std::max(hi.z, (double)r.z);
		}
		double h = 1e-6 * std::max((hi - lo).norm(), 1e-12);

		std::vector<int> parent(NS);
		for (int i = 0; i < NS; ++i) parent[i] = i;
		std::function<int(int)> root = [&](int x) { while (parent[x] != x) { parent[x] = parent[parent[x]]; x = parent[x]; } return x; };
		typedef std::tuple<long long, long long, long long> Key;
		std::map<Key, int> pointSurf;

		std::vector<Section> secs(NS);
		std::vector<vec3d> csum(NS, vec3d(0, 0, 0));
		for (int i = 0; i < gm->Faces(); ++i)
		{
			const GLMesh::FACE& f = gm->Face(i);
			auto it = sIndex.find(f.pid);
			if (it == sIndex.end()) continue;
			int si = it->second;
			vec3d p[3];
			for (int k = 0; k < 3; ++k)
			{
				vec3f r = gm->Node(f.n[k]).r;
				p[k] = T.LocalToGlobal(vec3d(r.x, r.y, r.z));
				Key key((long long)std::llround(r.x / h), (long long)std::llround(r.y / h), (long long)std::llround(r.z / h));
				auto jt = pointSurf.find(key);
				if (jt == pointSurf.end()) pointSurf[key] = si;
				else { int a = root(si), b = root(jt->second); if (a != b) parent[a] = b; }
			}
			vec3d an = ((p[1] - p[0]) ^ (p[2] - p[0])) * 0.5;
			double w = an.norm();
			secs[si].n += an; secs[si].area += w; csum[si] += (p[0] + p[1] + p[2]) * (w / 3.0);
		}

		// merge the touching surfaces
		std::map<int, int> cluster;
		std::vector<Section> merged;
		std::vector<vec3d> mc;
		for (int i = 0; i < NS; ++i)
		{
			int r = root(i);
			auto it = cluster.find(r);
			if (it == cluster.end())
			{
				it = cluster.insert({ r, (int)merged.size() }).first;
				merged.push_back(Section());
				merged.back().po = po;
				merged.back().geometric = true;
				mc.push_back(vec3d(0, 0, 0));
			}
			Section& m = merged[it->second];
			m.n += secs[i].n; m.area += secs[i].area; mc[it->second] += csum[i];
		}
		for (size_t i = 0; i < merged.size(); ++i)
		{
			if (merged[i].area <= 0.0) { err = "A selected surface has no area."; return false; }
			merged[i].c = mc[i] / merged[i].area;
			sections.push_back(merged[i]);
		}
	}

	for (auto& fs : faceSets)
	{
		GObject* po = fs.first;
		FSMesh* pm = po->GetFEMesh();
		if (pm == nullptr) continue;
		std::vector<int>& ids = fs.second;
		std::sort(ids.begin(), ids.end());
		ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
		const Transform& T = po->GetTransform();

		// group the faces into connected sections (faces sharing a node)
		const int NF = (int)ids.size();
		std::vector<int> parent(NF);
		for (int i = 0; i < NF; ++i) parent[i] = i;
		std::function<int(int)> root = [&](int x) { while (parent[x] != x) { parent[x] = parent[parent[x]]; x = parent[x]; } return x; };
		std::map<int, int> nodeFace;
		for (int i = 0; i < NF; ++i)
		{
			const FSFace& f = pm->Face(ids[i]);
			for (int k = 0; k < FaceCorners(f); ++k)
			{
				auto it = nodeFace.find(f.n[k]);
				if (it == nodeFace.end()) nodeFace[f.n[k]] = i;
				else { int a = root(i), b = root(it->second); if (a != b) parent[a] = b; }
			}
		}

		std::map<int, int> clusterIndex;
		std::vector<Section> secs;
		std::vector<vec3d> csum;
		for (int i = 0; i < NF; ++i)
		{
			int r = root(i);
			auto it = clusterIndex.find(r);
			if (it == clusterIndex.end())
			{
				it = clusterIndex.insert({ r, (int)secs.size() }).first;
				secs.push_back(Section());
				secs.back().po = po;
				csum.push_back(vec3d(0, 0, 0));
			}
			Section& s = secs[it->second];
			s.faces.push_back(ids[i]);

			const FSFace& f = pm->Face(ids[i]);
			vec3d a = T.LocalToGlobal(pm->Node(f.n[0]).r), b = T.LocalToGlobal(pm->Node(f.n[1]).r), c = T.LocalToGlobal(pm->Node(f.n[2]).r);
			vec3d an = ((b - a) ^ (c - a)) * 0.5;
			double w = an.norm();
			s.n += an; s.area += w; csum[it->second] += (a + b + c) * (w / 3.0);
			if (FaceCorners(f) == 4)
			{
				vec3d d = T.LocalToGlobal(pm->Node(f.n[3]).r);
				vec3d bn = ((c - a) ^ (d - a)) * 0.5;
				double w2 = bn.norm();
				s.n += bn; s.area += w2; csum[it->second] += (a + c + d) * (w2 / 3.0);
			}
		}
		for (size_t i = 0; i < secs.size(); ++i)
		{
			if (secs[i].area <= 0.0) { err = "A selected section has zero area."; return false; }
			secs[i].c = csum[i] / secs[i].area;
			sections.push_back(secs[i]);
		}
	}
	if (sections.empty()) { err = "Could not get any faces from the selection."; return false; }
	return true;
}

void CSweepSectionsTool::OnSetStart()
{
	std::vector<Section> secs;
	QString err;
	if (!GetSections(secs, true, err)) { Error(err); return; }
	if (secs.size() != 1)
	{
		Error(QString("The start section must be a single connected set of faces (the selection has %1 separate sections).").arg((int)secs.size()));
		return;
	}
	m_start = secs[0];
	m_haveStart = true;
	UpdateLabels();
}

void CSweepSectionsTool::OnSetTargets()
{
	std::vector<Section> secs;
	QString err;
	if (!GetSections(secs, false, err)) { Error(err); return; }
	m_targets = secs;
	UpdateLabels();
}

void CSweepSectionsTool::OnAddTargets()
{
	std::vector<Section> secs;
	QString err;
	if (!GetSections(secs, false, err)) { Error(err); return; }
	for (const Section& s : secs) m_targets.push_back(s);
	UpdateLabels();
}

void CSweepSectionsTool::OnClearTargets()
{
	m_targets.clear();
	UpdateLabels();
}

namespace {
	vec3d Rotate(const vec3d& v, const vec3d& k, double phi)
	{
		// Rodrigues, k is a unit vector
		return v * cos(phi) + (k ^ v) * sin(phi) + k * ((k * v) * (1.0 - cos(phi)));
	}
}

void CSweepSectionsTool::OnSweep()
{
	CModelDocument* doc = GetMainWindow()->GetModelDocument();
	if ((doc == nullptr) || !doc->IsValid()) return;

	UpdateLabels();
	if (!m_haveStart) { Error("Set the start section first."); return; }
	if (m_targets.empty()) { Error("Set the target sections first."); return; }

	GObject* po = m_start.po;
	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) { Error("The start object has no mesh."); return; }
	const Transform& T = po->GetTransform();

	// build the shell mesh of the start section (from mesh faces, or the whole object if it is a shell mesh)
	struct Cell { int type; int nn; int n[8]; };
	std::vector<Cell> cells;
	if (m_start.wholeObject)
	{
		for (int i = 0; i < pm->Elements(); ++i)
		{
			const FSElement& el = pm->Element(i);
			Cell c; c.type = el.Type(); c.nn = el.Nodes();
			if ((c.type != FE_TRI3) && (c.type != FE_QUAD4) && (c.type != FE_TRI6) && (c.type != FE_QUAD8))
			{
				Error("The start object is not a shell mesh of TRI3, QUAD4, TRI6 or QUAD8 elements.");
				return;
			}
			for (int k = 0; k < c.nn; ++k) c.n[k] = el.m_node[k];
			cells.push_back(c);
		}
	}
	else
	{
		for (int fi : m_start.faces)
		{
			if ((fi < 0) || (fi >= pm->Faces())) { Error("The start section is no longer valid. Set it again."); return; }
			const FSFace& f = pm->Face(fi);
			Cell c; c.nn = f.Nodes();
			switch (f.Type()) {
			case FE_FACE_TRI3: c.type = FE_TRI3; break;
			case FE_FACE_QUAD4: c.type = FE_QUAD4; break;
			case FE_FACE_TRI6: c.type = FE_TRI6; break;
			case FE_FACE_QUAD8: c.type = FE_QUAD8; break;
			default:
				Error("The start section must consist of TRI3, QUAD4, TRI6 or QUAD8 faces.");
				return;
			}
			for (int k = 0; k < c.nn; ++k) c.n[k] = f.n[k];
			cells.push_back(c);
		}
	}
	if (cells.empty()) { Error("The start section is empty."); return; }

	std::map<int, int> nodeMap;
	std::vector<int> nodeList;
	for (const Cell& c : cells)
		for (int k = 0; k < c.nn; ++k)
			if (nodeMap.find(c.n[k]) == nodeMap.end()) { nodeMap[c.n[k]] = (int)nodeList.size(); nodeList.push_back(c.n[k]); }
	FSMesh shell;
	shell.Create((int)nodeList.size(), (int)cells.size());
	for (size_t i = 0; i < nodeList.size(); ++i) shell.Node((int)i).r = pm->Node(nodeList[i]).r;
	for (size_t i = 0; i < cells.size(); ++i)
	{
		FSElement& el = shell.Element((int)i);
		el.m_gid = 0;
		el.SetType(cells[i].type);
		for (int k = 0; k < cells[i].nn; ++k) el.m_node[k] = nodeMap[cells[i].n[k]];
	}

	// order the target sections: repeatedly go to the nearest remaining one (ignoring sections coinciding with the start)
	std::vector<Section> remaining;
	double L0 = sqrt(m_start.area);
	for (const Section& s : m_targets)
		if ((s.c - m_start.c).norm() > 1e-6 * std::max(L0, 1e-12)) remaining.push_back(s);
	if (remaining.empty()) { Error("The target sections coincide with the start section."); return; }
	std::vector<Section> st;
	st.push_back(m_start);
	while (!remaining.empty())
	{
		int best = 0;
		double dmin = 1e300;
		for (int i = 0; i < (int)remaining.size(); ++i)
		{
			double d = (remaining[i].c - st.back().c).norm();
			if (d < dmin) { dmin = d; best = i; }
		}
		st.push_back(remaining[best]);
		remaining.erase(remaining.begin() + best);
	}
	const int ns = (int)st.size() - 1;		// number of intervals

	// tangent of the path at each section
	std::vector<vec3d> tg(ns + 1);
	for (int i = 0; i <= ns; ++i)
	{
		vec3d t = st[i].n;
		if (t.norm() < 1e-12 * st[i].area) { Error("Could not determine the normal of one of the sections (it is closed or too wrinkled)."); return; }
		t /= t.norm();
		if (i == 0)
		{
			t = -t;		// faces point outward; the sweep leaves the start section inward
			vec3d d = st[1].c - st[0].c;
			if (t * d < -0.3 * d.norm()) t = -t;
		}
		else if (i < ns)
		{
			vec3d d = st[i + 1].c - st[i - 1].c;
			if (t * d < 0) t = -t;
		}
		else
		{
			vec3d d = st[i].c - st[i - 1].c;
			if (t * d < -0.3 * d.norm()) t = -t;
		}
		tg[i] = t;
	}

	if (ui->tangentMode->currentIndex() == 1)
	{
		// smooth curve through the centroids: tangent along the central difference; at the ends mirror the neighbouring tangent
		// about the end chord (which gives a circular arc for a circular path)
		if (ns == 1)
		{
			vec3d d = st[1].c - st[0].c;
			tg[0] = tg[1] = d / d.norm();
		}
		else
		{
			for (int i = 1; i < ns; ++i)
			{
				vec3d d = st[i + 1].c - st[i - 1].c;
				tg[i] = d / d.norm();
			}
			vec3d d0 = st[1].c - st[0].c; d0 /= d0.norm();
			vec3d dn = st[ns].c - st[ns - 1].c; dn /= dn.norm();
			vec3d a = d0 * (2.0 * (tg[1] * d0)) - tg[1];
			vec3d b = dn * (2.0 * (tg[ns - 1] * dn)) - tg[ns - 1];
			tg[0] = (a.norm() > 1e-9 ? a / a.norm() : d0);
			tg[ns] = (b.norm() > 1e-9 ? b / b.norm() : dn);
		}
	}

	// build the path: layers between each pair of sections; circular arc when the end tangents are mirror images
	// across the bisector of the chord, a cubic Hermite curve otherwise
	const int nl = ui->layers->value();
	std::vector<vec3d> path;
	std::vector<double> scale;
	std::vector<vec3d> orient;

	// desired plane normal of the section at each target: the target normal with the sign that agrees with the path tangent
	std::vector<vec3d> pn(ns + 1);
	for (int i = 0; i <= ns; ++i)
	{
		vec3d t = st[i].n;
		t /= t.norm();
		if (t * tg[i] < 0) t = -t;
		pn[i] = t;
	}
	const bool doOrient = (ui->tangentMode->currentIndex() == 1) && ui->matchOrientation->isChecked();
	path.push_back(st[0].c);
	scale.push_back(1.0);
	orient.push_back(pn[0]);
	for (int i = 0; i < ns; ++i)
	{
		vec3d p0 = st[i].c, p1 = st[i + 1].c;
		vec3d t0 = tg[i], t1 = tg[i + 1];
		vec3d d = p1 - p0;
		double c = d.norm();
		vec3d dh = d / c;
		double s0 = ui->scale->isChecked() ? sqrt(st[i].area / m_start.area) : 1.0;
		double s1 = ui->scale->isChecked() ? sqrt(st[i + 1].area / m_start.area) : 1.0;

		double cosA0 = std::max(-1.0, std::min(1.0, t0 * dh));
		double a0 = acos(cosA0);
		vec3d t1r = t0 - dh * (2.0 * (t0 * dh));
		vec3d k = t0 ^ dh;
		bool straight = ((t0 - dh).norm() < 1e-3) && ((t1 - dh).norm() < 1e-3);
		bool arc = !straight && (k.norm() > 1e-6) && (a0 > 1e-4) && ((t1r - t1).norm() < 0.03);

		for (int j = 1; j <= nl; ++j)
		{
			double u = (double)j / (double)nl;
			vec3d p;
			if (straight) p = p0 + d * u;
			else if (arc)
			{
				vec3d kk = k / k.norm();
				double R = c / (2.0 * sin(a0));
				vec3d v = kk ^ t0;
				double phi = 2.0 * a0 * u;
				p = p0 + v * R - Rotate(v, kk, phi) * R;
			}
			else
			{
				double ang = 2.0 * std::max(a0, acos(std::max(-1.0, std::min(1.0, t1 * dh))));
				ang = std::min(ang, 3.0);
				double mag = (ang > 1e-6) ? c * (ang / 2.0) / sin(ang / 2.0) : c;
				double u2 = u * u, u3 = u2 * u;
				p = p0 * (2 * u3 - 3 * u2 + 1) + t0 * (mag * (u3 - 2 * u2 + u)) + p1 * (-2 * u3 + 3 * u2) + t1 * (mag * (u3 - u2));
			}
			if (j == nl) p = p1;
			path.push_back(p);
			scale.push_back(s0 * (1.0 - u) + s1 * u);
			orient.push_back(pn[i] * (1.0 - u) + pn[i + 1] * u);
		}
	}

	ShellSweeper sw;
	sw.m_path = path;
	sw.m_scale = scale;
	if (doOrient) sw.m_orient = orient;
	sw.m_layers = 0;		// one layer per path segment
	sw.m_rotate = true;
	sw.m_autoPartition = ui->autoPartition->isChecked();
	sw.m_creaseAngle = ui->crease->value();
	sw.m_collapseTol = ui->collapseTol->value();

	QApplication::setOverrideCursor(Qt::WaitCursor);
	FSMesh* solid = sw.Apply(shell, T);
	QApplication::restoreOverrideCursor();
	if (solid == nullptr)
	{
		Error("Sweep failed: " + QString::fromStdString(sw.GetErrorString()));
		return;
	}

	// count elements with a non-positive Jacobian at one of their corners
	int ninverted = 0;
	for (int i = 0; i < solid->Elements(); ++i)
	{
		const FSElement& el = solid->Element(i);
		vec3d p[8];
		bool ok = true;
		if ((el.Type() == FE_HEX8) || (el.Type() == FE_HEX20))
		{
			static const int rn[8] = { 1,0,3,2,5,4,7,6 }, sn[8] = { 3,2,1,0,7,6,5,4 }, tn[8] = { 4,5,6,7,0,1,2,3 };
			static const int sr[8] = { -1,1,1,-1,-1,1,1,-1 }, ss[8] = { -1,-1,1,1,-1,-1,1,1 }, st_[8] = { -1,-1,-1,-1,1,1,1,1 };
			for (int k = 0; k < 8; ++k) p[k] = solid->Node(el.m_node[k]).r;
			for (int k = 0; k < 8 && ok; ++k)
			{
				vec3d dr = (p[k] - p[rn[k]]) * (double)sr[k], ds = (p[k] - p[sn[k]]) * (double)ss[k], dt = (p[k] - p[tn[k]]) * (double)st_[k];
				if ((dr ^ ds) * dt <= 0) ok = false;
			}
		}
		else if ((el.Type() == FE_PENTA6) || (el.Type() == FE_PENTA15))
		{
			for (int k = 0; k < 6; ++k) p[k] = solid->Node(el.m_node[k]).r;
			for (int k = 0; k < 6 && ok; ++k)
			{
				int b = (k / 3) * 3, j = k % 3;
				vec3d e1 = p[b + (j + 1) % 3] - p[k], e2 = p[b + (j + 2) % 3] - p[k];
				vec3d dt = p[(k % 3) + 3] - p[k % 3];
				if ((e1 ^ e2) * dt <= 0) ok = false;
			}
		}
		if (!ok) ninverted++;
	}

	static int n = 1;
	QString name = QString("%1_sections%2").arg(QString::fromStdString(po->GetName())).arg(n++);
	GMeshObject* pnew = new GMeshObject(solid);
	pnew->SetName(name.toStdString());

	CCmdGroup* cmd = new CCmdGroup("Sweep sections");
	cmd->AddCommand(new CCmdAddAndSelectObject(doc->GetGModel(), pnew));
	int action = ui->original->currentIndex();
	if (action == 1) cmd->AddCommand(new CCmdHideObject(po));
	else if (action == 2) cmd->AddCommand(new CCmdDeleteGObject(doc->GetGModel(), po));

	// the objects holding the target sections (each once, and not the start object)
	int tact = ui->targetObjs->currentIndex();
	if (tact != 0)
	{
		std::vector<GObject*> tobjs;
		for (const Section& sec : m_targets)
			if ((sec.po != po) && std::find(tobjs.begin(), tobjs.end(), sec.po) == tobjs.end()) tobjs.push_back(sec.po);
		for (GObject* t : tobjs)
		{
			if (tact == 1) cmd->AddCommand(new CCmdHideObject(t));
			else cmd->AddCommand(new CCmdDeleteGObject(doc->GetGModel(), t));
		}
	}
	doc->DoCommand(cmd, name.toStdString());
	GetMainWindow()->UpdateModel(pnew);
	UpdateLabels();	// refresh the model tree (abstract tools don't do this automatically)

	QString msg = QString("Created %1 (%2 elements)").arg(name).arg(solid->Elements());
	if (ninverted > 0)
		msg += QString("\n\nWARNING: %1 elements have a non-positive Jacobian. The path probably bends more sharply than the section is wide "
			"(the inside of the bend folds over), or the section normals point the wrong way. Try more spacing between sections, "
			"fewer/gentler turns, intermediate sections, or the \"smooth curve through centroids\" path option.").arg(ninverted);
	QMessageBox::information(GetMainWindow(), "Sweep Sections", msg);
}
