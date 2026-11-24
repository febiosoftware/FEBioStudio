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
#include "VesselSectionsTool.h"
#include "SweepSectionsTool.h"
#include "ModelDocument.h"
#include "MainWindow.h"
#include "Commands.h"
#include <GeomLib/GObject.h>
#include <GeomLib/GModel.h>
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GOCCObject.h>
#include <MeshTools/NetGenOCCMesher.h>
#include <MeshTools/NetGenFacetMesher.h>
#include <MeshTools/FacePatch.h>
#include <MeshTools/VesselSections.h>
#include <MeshTools/FESelection.h>
#include <MeshLib/FSMesh.h>
#include <MeshLib/FSMeshBase.h>
#include <MeshLib/FSFace.h>
#include <FECore/FETransform.h>
#include <map>
#include <algorithm>
#include <QApplication>
#include <QMessageBox>

CVesselSectionsTool::CVesselSectionsTool(CMainWindow* wnd) : CBasicTool(wnd, "Vessel Sections", HAS_APPLY_BUTTON)
{
	m_intervals = 20;
	m_dIntervals = 0;
	m_dElemSize = 0.0;
	m_smoothCaps = true;
	m_branchFromTrunk = false;
	m_adapt = 0.0;
	m_elemSize = 0.0;
	m_quadratic = false;
	m_action = 1;
	m_layers = 2;
	m_keepSections = false;
	m_alignTwist = false;
	m_autoPartition = false;
	m_creaseAngle = 30.0;
	m_collapseTol = 0.0;
	m_hideSource = true;
	SetApplyButtonText("Create");
	addIntProperty(&m_intervals, "number of intervals (sections = intervals + 1)");
	addIntProperty(&m_dIntervals, "bifurcation: intervals in each daughter branch (0 = auto, same spacing as the trunk end)");
	addDoubleProperty(&m_dElemSize, "bifurcation: element size at the branch outlets (0 = auto, matches the trunk at the junction)");
	addBoolProperty(&m_smoothCaps, "smooth the outline of the end caps (removes jagged STL cap rims)");
	addBoolProperty(&m_branchFromTrunk, "bifurcation: start branches from the trunk's end faces instead of the outlet caps");
	addDoubleProperty(&m_adapt, "adaptivity (0 = uniform spacing)");
	addDoubleProperty(&m_elemSize, "start section element size (0 = auto)");
	addBoolProperty(&m_quadratic, "quadratic elements");
	addEnumProperty(&m_action, "action")->setEnumValues(QStringList() << "create sections only" << "create sections and sweep");
	addIntProperty(&m_layers, "sweep: layers between sections");
	addBoolProperty(&m_keepSections, "sweep: keep the section objects");
	addBoolProperty(&m_alignTwist, "sweep: align twist to outlines");
	addBoolProperty(&m_autoPartition, "sweep: auto-partition surfaces");
	addDoubleProperty(&m_creaseAngle, "sweep: crease angle (degrees)");
	addDoubleProperty(&m_collapseTol, "sweep: degeneracy tolerance (0 = auto)");
	addBoolProperty(&m_hideSource, "hide the original object");
	SetInfo("Select a vessel-like surface (e.g. an artery STL). If the surface is closed with flat end caps, select the faces of the "
		"start cap (the other cap is found automatically); for a bifurcation select all three caps (the largest is the inlet): the "
		"trunk and the two daughter branches are then created as three separate solids. Otherwise the surface must have two open ends. The centerline between the ends is traced "
		"automatically, and cross-sections are cut perpendicular to it. With adaptivity > 0, more sections are placed where the "
		"centerline curvature (relative to the vessel radius) or the change of the vessel radius is large. The first section "
		"gets a quad-dominant mesh; with 'create sections and sweep' it is swept through the others (see Sweep Sections).");
}

namespace {
	// shell object with a triangulated planar polygon
	GObject* MakeTriShell(const VesselSections::Poly& P)
	{
		std::vector<int> tri;
		if (!VesselSections::TriangulatePolygon(P.pts, tri)) return nullptr;
		FSMesh* mesh = new FSMesh();
		mesh->Create((int)P.pts.size(), (int)tri.size() / 3);
		for (size_t i = 0; i < P.pts.size(); ++i) mesh->Node((int)i).r = P.pts[i];
		for (size_t i = 0; i < tri.size() / 3; ++i)
		{
			FSElement& el = mesh->Element((int)i);
			el.m_gid = 0;
			el.SetType(FE_TRI3);
			for (int k = 0; k < 3; ++k) el.m_node[k] = tri[3 * i + k];
		}
		mesh->RebuildMesh();
		return new GMeshObject(mesh);
	}

	// quad-dominant meshed shell object of a planar polygon (rebuilt as a CAD face and meshed with NetGen; falls back to
	// the triangulation-based mesher)
	GObject* MakeMeshedSection(const VesselSections::Poly& P, double size, bool quadratic, GObject* source, QString& err, const FacePatch* capPatch = nullptr)
	{
		FacePatch patch;
		if (capPatch && !capPatch->empty()) patch = *capPatch;
		else
		{
			patch.pts = P.pts;
			if (!VesselSections::TriangulatePolygon(P.pts, patch.tri)) { err = "Could not triangulate the start section."; return nullptr; }
		}
		std::vector<FacePatch> patches;
		patches.push_back(patch);

		GOCCObject* shell = BuildPlanarOCCShell(patches);
		NetGenMesher* mesher = nullptr;
		NetGenFacetMesher* facet = nullptr;
		if (shell) mesher = dynamic_cast<NetGenOCCMesher*>(shell->GetFEMesher());
		if (mesher == nullptr)
		{
			delete shell; shell = nullptr;
			facet = new NetGenFacetMesher(*source);
			facet->SetPatch(patch);
			mesher = facet;
		}
		mesher->SetBoolValue(NetGenMesher::QUADMESH, true);
		mesher->SetBoolValue(NetGenMesher::SURFREFINE, false);
		mesher->SetBoolValue(NetGenMesher::SECONDORDER, quadratic);
		mesher->SetFloatValue(NetGenMesher::MAXELEMSIZE, size);
		mesher->SetFloatValue(NetGenMesher::MINELEMSIZE, size * 0.01);

		FSMesh* mesh = (shell ? shell->BuildMesh() : facet->BuildMesh());
		if (mesh == nullptr)
		{
			std::string e = mesher->GetErrorString();
			err = QString::fromStdString(e.empty() ? "NetGen failed to mesh the start section." : e);
			delete shell;
			delete facet;
			return nullptr;
		}
		GObject* obj = shell;
		if (obj == nullptr) obj = new GMeshObject(mesh);
		delete facet;
		return obj;
	}
}

bool CVesselSectionsTool::OnApply()
{
	CModelDocument* doc = GetMainWindow()->GetModelDocument();
	if ((doc == nullptr) || !doc->IsValid()) { SetErrorString("No valid model document."); return false; }

	GObject* po = GetActiveObject();
	FESelection* cursel = doc->GetCurrentSelection();
	GFaceSelection* gsel = dynamic_cast<GFaceSelection*>(cursel);
	FEFaceSelection* fsel = dynamic_cast<FEFaceSelection*>(cursel);
	if ((po == nullptr) && gsel && (gsel->Count() > 0)) { GFace* gf = gsel->Face(0); po = (gf ? dynamic_cast<GObject*>(gf->Object()) : nullptr); }
	if ((po == nullptr) && fsel && (fsel->Count() > 0) && fsel->GetMesh()) po = fsel->GetMesh()->GetGObject();
	if (po == nullptr) { SetErrorString("Select the vessel surface object first."); return false; }
	if (m_intervals < 1) { SetErrorString("The number of intervals must be at least 1."); return false; }
	if (m_layers < 1) { SetErrorString("The number of layers must be at least 1."); return false; }

	FSMeshBase* mb = po->GetEditableMesh();
	if ((mb == nullptr) || (mb->Faces() == 0)) mb = po->GetFEMesh();
	if ((mb == nullptr) || (mb->Faces() == 0)) { SetErrorString("The selected object has no surface mesh."); return false; }

	// global triangulation of the surface
	const Transform& T = po->GetTransform();
	std::vector<vec3d> pts(mb->Nodes());
	for (int i = 0; i < mb->Nodes(); ++i) pts[i] = T.LocalToGlobal(mb->Node(i).r);
	// the selected faces (start cap), if any
	std::vector<char> selFace(mb->Faces(), 0);
	bool haveSel = false;
	if (gsel && (gsel->Count() > 0))
	{
		for (int n = 0; n < gsel->Count(); ++n)
		{
			GFace* gf = gsel->Face(n);
			if ((gf == nullptr) || (dynamic_cast<GObject*>(gf->Object()) != po)) continue;
			for (int i = 0; i < mb->Faces(); ++i)
				if (mb->Face(i).m_gid == gf->GetLocalID()) { selFace[i] = 1; haveSel = true; }
		}
	}
	else if (fsel && (fsel->Count() > 0) && (fsel->GetMesh() == mb))
	{
		for (int n = 0; n < (int)fsel->Size(); ++n)
		{
			int fi = fsel->FaceIndex(n);
			if ((fi >= 0) && (fi < mb->Faces())) { selFace[fi] = 1; haveSel = true; }
		}
	}

	std::vector<int> tri;
	std::vector<int> startTris;
	for (int i = 0; i < mb->Faces(); ++i)
	{
		const FSFace& f = mb->Face(i);
		bool quad = (f.Type() == FE_FACE_QUAD4) || (f.Type() == FE_FACE_QUAD8) || (f.Type() == FE_FACE_QUAD9);
		if (selFace[i]) startTris.push_back((int)tri.size() / 3);
		tri.push_back(f.n[0]); tri.push_back(f.n[1]); tri.push_back(f.n[2]);
		if (quad)
		{
			if (selFace[i]) startTris.push_back((int)tri.size() / 3);
			tri.push_back(f.n[0]); tri.push_back(f.n[2]); tri.push_back(f.n[3]);
		}
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);
	VesselSections vs;
	std::string err;
	std::vector<VesselSections::Poly> secs;
	bool ok = true;
	FacePatch capPatch;
	std::vector<std::vector<int>> allCaps;
	std::vector<vec3d> allCapC;
	if (haveSel)
	{
		vec3d sc;
		std::vector<int> capTris;
		ok = VesselSections::RemoveCaps(pts, tri, startTris, sc, capTris, err, &allCaps, &allCapC);
		if (ok)
		{
			// the start cap's own triangulation (the same input the Shell Mesh Faces tool uses)
			std::map<int, int> remap;
			for (int v : capTris)
			{
				auto it = remap.find(v);
				if (it == remap.end()) { it = remap.insert({ v, (int)capPatch.pts.size() }).first; capPatch.pts.push_back(pts[v]); }
				capPatch.tri.push_back(it->second);
			}
			capPatch = MergePatches(std::vector<FacePatch>(1, capPatch));	// weld coincident points
		}
		if (ok) vs.SetStartPoint(sc);
	}
	bool branched = false;
	VesselSections::Bifurcation bif;
	ok = ok && vs.Init(pts, tri, err);
	if (ok)
	{
		branched = vs.Branched();
		if (branched)
		{
			const int nd0 = (m_dIntervals > 0 ? m_dIntervals : 12);
			ok = vs.BuildBifurcation(m_intervals, nd0, m_adapt, bif, err);
			if (ok && (m_dIntervals <= 0) && (bif.trunk.size() >= 2))
			{
				// automatic spacing: the axial spacing of the trunk at the junction
				double hax = (bif.trunk[bif.trunk.size() - 1].c - bif.trunk[bif.trunk.size() - 2].c).norm();
				for (int j = 0; j < 2 && hax > 0; ++j)
				{
					double L = 0;
					for (size_t k = 0; k + 1 < bif.daughter[j].size(); ++k) L += (bif.daughter[j][k + 1].c - bif.daughter[j][k].c).norm();
					int nj = std::min(80, std::max(3, (int)(L / hax + 0.5)));
					if (nj == nd0) continue;
					VesselSections::Bifurcation b2;
					std::string e2;
					if (vs.BuildBifurcation(m_intervals, nj, m_adapt, b2, e2)) bif.daughter[j] = b2.daughter[j];
				}
			}
			if (ok) secs = bif.trunk;
		}
		else ok = vs.TraceCenterline(err) && vs.BuildSections(m_intervals, m_adapt, secs, err);
	}
	QApplication::restoreOverrideCursor();
	if (!ok) { SetErrorString(QString::fromStdString(err)); return false; }

	// the start section (meshed)
	const VesselSections::Poly& P0 = secs.front();
	double req = sqrt(P0.area / 3.141592653589793);
	double size = (m_elemSize > 0.0 ? m_elemSize : req / 4.0);
	QString qerr;
	QApplication::setOverrideCursor(Qt::WaitCursor);
	GObject* startObj = MakeMeshedSection(P0, size, m_quadratic, po, qerr, &capPatch);
	QApplication::restoreOverrideCursor();
	if (startObj == nullptr) { SetErrorString(qerr); return false; }
	QString base = QString::fromStdString(po->GetName());
	startObj->SetName((base + "_section0").toStdString());

	const bool sweep = (m_action == 1);
	if (!sweep || m_keepSections)
	{
		// create the section objects
		CCmdGroup* cmd = new CCmdGroup("Create vessel sections");
		cmd->AddCommand(new CCmdAddObject(doc->GetGModel(), startObj));
		for (size_t j = 1; j < secs.size(); ++j)
		{
			GObject* o = MakeTriShell(secs[j]);
			if (o == nullptr) continue;
			o->SetName((base + QString("_section%1").arg((int)j)).toStdString());
			cmd->AddCommand(new CCmdAddObject(doc->GetGModel(), o));
		}
		if (branched)
		{
			for (int j = 0; j < 2; ++j)
				for (size_t k = 0; k < bif.daughter[j].size(); ++k)
				{
					GObject* o = MakeTriShell(bif.daughter[j][k]);
					if (o == nullptr) continue;
					o->SetName((base + QString("_branch%1_section%2").arg(j + 1).arg((int)k)).toStdString());
					cmd->AddCommand(new CCmdAddObject(doc->GetGModel(), o));
				}
		}
		if (m_hideSource && !sweep) cmd->AddCommand(new CCmdHideObject(po));
		doc->DoCommand(cmd, "vessel sections");
	}

	if (sweep)
	{
		QString serr;
		CSweepSectionsTool::Section start;
		if (!CSweepSectionsTool::SectionFromShellObject(startObj, start, serr))
		{
			if (!m_keepSections) delete startObj;
			SetErrorString("Start section: " + serr);
			return false;
		}
		// the faces of the start section point out of the vessel; the targets point along the direction of travel
		start.n = -P0.n * start.area;
		std::vector<CSweepSectionsTool::Section> targets;
		for (size_t j = 1; j < secs.size(); ++j)
		{
			CSweepSectionsTool::Section s;
			CSweepSectionsTool::SectionFromPolygon(secs[j].pts, secs[j].n, s);
			targets.push_back(s);
		}

		CSweepSectionsTool::SweepOptions opt;
		opt.layers = m_layers;
		opt.tangentMode = 1;
		opt.matchOrientation = true;
		opt.shapeMode = 3;
		opt.alignTwist = m_alignTwist;
		opt.smoothStart = m_smoothCaps;
		opt.autoPartition = m_autoPartition;
		opt.crease = m_creaseAngle;
		opt.collapseTol = m_collapseTol;
		opt.originalAction = 0;
		opt.targetAction = 0;

		CSweepSectionsTool sweeper(GetMainWindow());
		if (!branched)
		{
			bool done = sweeper.RunSweep(start, targets, opt);
			if (!m_keepSections) delete startObj;
			if (done && m_hideSource) doc->DoCommand(new CCmdHideObject(po), "hide vessel surface");
			GetMainWindow()->UpdateModel();
			return done;
		}

		// bifurcation: sweep the trunk, then each daughter from its half of the trunk's end face
		QString report;
		GObject* trunk = nullptr;
		QString msg;
		opt.quiet = true;
		opt.result = &trunk;
		opt.message = &msg;
		opt.name = (base + "_trunk").toStdString();
		bool done = sweeper.RunSweep(start, targets, opt);
		if (!m_keepSections) delete startObj;
		if (!done || (trunk == nullptr)) { GetMainWindow()->UpdateModel(); return false; }
		if (!msg.isEmpty()) report += "Trunk: " + msg + "\n\n";

		// the faces of the trunk solid on the end plane F, split by the divider plane
		FSMesh* tm = trunk->GetFEMesh();
		const Transform& TT = trunk->GetTransform();
		std::vector<int> halfFaces[2];
		double Lf = sqrt(secs.back().area);
		for (int i = 0; i < tm->Faces(); ++i)
		{
			const FSFace& f = tm->Face(i);
			int nc = ((f.Type() == FE_FACE_QUAD4) || (f.Type() == FE_FACE_QUAD8) || (f.Type() == FE_FACE_QUAD9)) ? 4 : 3;
			vec3d c(0, 0, 0);
			bool onF = true;
			for (int k = 0; k < nc; ++k)
			{
				vec3d r = TT.LocalToGlobal(tm->Node(f.n[k]).r);
				if (fabs((r - bif.fp) * bif.ft) > 1e-3 * Lf) onF = false;
				c += r;
			}
			if (!onF) continue;
			c /= (double)nc;
			halfFaces[((c - bif.dp) * bif.dn >= 0) ? 0 : 1].push_back(i);
		}
		for (int j = 0; j < 2; ++j)
		{
			CSweepSectionsTool::Section hs;
			QString herr;
			GObject* tmpStart = nullptr;
			std::vector<CSweepSectionsTool::Section> dt;
			const std::vector<VesselSections::Poly>& dj = bif.daughter[j];
			if (!m_branchFromTrunk)
			{
				// sweep from the outlet cap back towards the divider: the start is the (meshed) cap, the last target is the half section
				const VesselSections::Poly& Pj = dj.back();
				FacePatch cp;
				int best = -1;
				double dmin = 1e300;
				for (size_t i = 0; i < allCapC.size(); ++i)
				{
					double d = (allCapC[i] - Pj.c).norm();
					if (d < dmin) { dmin = d; best = (int)i; }
				}
				if (best >= 0)
				{
					std::map<int, int> remap;
					for (int v : allCaps[best])
					{
						auto it = remap.find(v);
						if (it == remap.end()) { it = remap.insert({ v, (int)cp.pts.size() }).first; cp.pts.push_back(pts[v]); }
						cp.tri.push_back(it->second);
					}
					cp = MergePatches(std::vector<FacePatch>(1, cp));
				}
				QString merr;
				double sizeJ = m_dElemSize;
				if (sizeJ <= 0.0)
				{
					// match the in-plane element size of the trunk where the branch meets it
					auto perim = [](const std::vector<vec3d>& p) {
						double s = 0;
						for (size_t i = 0; i < p.size(); ++i) s += (p[(i + 1) % p.size()] - p[i]).norm();
						return s;
					};
					double p0 = perim(bif.trunk.front().pts), p1 = perim(bif.trunk.back().pts);
					double pc = perim(Pj.pts), ph = perim(dj.front().pts);
					sizeJ = size;
					if ((p0 > 0) && (p1 > 0) && (pc > 0) && (ph > 0)) sizeJ = size * (p1 / p0) * (pc / ph);
					sizeJ = std::min(3.0 * size, std::max(0.3 * size, sizeJ));
				}
				tmpStart = MakeMeshedSection(Pj, sizeJ, m_quadratic, po, merr, cp.empty() ? nullptr : &cp);
				if (tmpStart == nullptr) { SetErrorString(QString("Branch %1 start section: ").arg(j + 1) + merr); GetMainWindow()->UpdateModel(); return false; }
				if (!CSweepSectionsTool::SectionFromShellObject(tmpStart, hs, herr)) { delete tmpStart; SetErrorString("Branch start section: " + herr); GetMainWindow()->UpdateModel(); return false; }
				hs.n = Pj.n * hs.area;		// the outward normal of the vessel (along the flow); the sweep goes the other way
				for (int k = (int)dj.size() - 2; k >= 0; --k)
				{
					CSweepSectionsTool::Section s;
					CSweepSectionsTool::SectionFromPolygon(dj[k].pts, -dj[k].n, s);
					dt.push_back(s);
				}
			}
			else
			{
				if (halfFaces[j].size() >= 8)
				{
					// start from the trunk's own end faces (nodes shared with the trunk)
					if (!CSweepSectionsTool::SectionFromFaces(trunk, halfFaces[j], hs, herr)) { SetErrorString("Daughter start section: " + herr); GetMainWindow()->UpdateModel(); return false; }
					report += QString("Branch %1 starts from %2 faces of the trunk's end (of %3 + %4).\n").arg(j + 1).arg((int)halfFaces[j].size()).arg((int)halfFaces[0].size()).arg((int)halfFaces[1].size());
				}
				else
				{
					// too few trunk faces found on the end plane: mesh the half cross-section directly
					QString merr;
					tmpStart = MakeMeshedSection(dj[0], size, m_quadratic, po, merr);
					if (tmpStart == nullptr) { SetErrorString("Daughter start section: " + merr); GetMainWindow()->UpdateModel(); return false; }
					if (!CSweepSectionsTool::SectionFromShellObject(tmpStart, hs, herr)) { delete tmpStart; SetErrorString("Daughter start section: " + herr); GetMainWindow()->UpdateModel(); return false; }
					report += QString("Branch %1: its start section was meshed separately (%2 trunk end faces found).\n").arg(j + 1).arg((int)halfFaces[j].size());
				}
				hs.n = -bif.ft * hs.area;		// pointing away from the daughter that grows from it
				for (size_t k = 1; k < dj.size(); ++k)
				{
					CSweepSectionsTool::Section s;
					CSweepSectionsTool::SectionFromPolygon(dj[k].pts, dj[k].n, s);
					dt.push_back(s);
				}
			}
			QString m2;
			opt.result = nullptr;
			opt.message = &m2;
			opt.name = (base + QString("_branch%1").arg(j + 1)).toStdString();
			bool bdone = sweeper.RunSweep(hs, dt, opt);
			delete tmpStart;
			if (!bdone) { GetMainWindow()->UpdateModel(); return false; }
			if (!m2.isEmpty()) report += QString("Branch %1: ").arg(j + 1) + m2 + "\n\n";
		}
		if (m_hideSource) doc->DoCommand(new CCmdHideObject(po), "hide vessel surface");
		GetMainWindow()->UpdateModel();
		QMessageBox::information(GetMainWindow(), "Vessel Sections",
			"Created the trunk and the two branches of the bifurcation as three solid objects (" + base + "_trunk, " + base + "_branch1, " + base + "_branch2).\n" +
			(m_branchFromTrunk ? QString("The branches start from the trunk's end and meet each other on the divider plane, ") :
				QString("Each branch is swept from its outlet cap towards the junction, ending on a half section at the divider plane. The branches meet each other and the trunk there, ")) +
			"where the meshes are not node-matched (use tied interfaces, or merge/weld the nodes where they coincide).\n\n" + report);
		return true;
	}

	GetMainWindow()->UpdateModel(startObj);
	QMessageBox::information(GetMainWindow(), "Vessel Sections", QString("Created %1 sections along the vessel.").arg((int)secs.size()));
	return true;
}
