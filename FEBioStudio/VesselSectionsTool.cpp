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
#include <QApplication>
#include <QMessageBox>

CVesselSectionsTool::CVesselSectionsTool(CMainWindow* wnd) : CBasicTool(wnd, "Vessel Sections", HAS_APPLY_BUTTON)
{
	m_intervals = 20;
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
		"start cap (the other cap is found automatically); otherwise the surface must have two open ends. The centerline between the ends is traced "
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
	if (haveSel)
	{
		vec3d sc;
		std::vector<int> capTris;
		ok = VesselSections::RemoveCaps(pts, tri, startTris, sc, capTris, err);
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
	ok = ok && vs.Init(pts, tri, err) && vs.TraceCenterline(err) && vs.BuildSections(m_intervals, m_adapt, secs, err);
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
		opt.autoPartition = m_autoPartition;
		opt.crease = m_creaseAngle;
		opt.collapseTol = m_collapseTol;
		opt.originalAction = 0;
		opt.targetAction = 0;

		CSweepSectionsTool sweeper(GetMainWindow());
		bool done = sweeper.RunSweep(start, targets, opt);
		if (!m_keepSections) delete startObj;
		if (done && m_hideSource) doc->DoCommand(new CCmdHideObject(po), "hide vessel surface");
		GetMainWindow()->UpdateModel();
		return done;
	}

	GetMainWindow()->UpdateModel(startObj);
	QMessageBox::information(GetMainWindow(), "Vessel Sections", QString("Created %1 sections along the vessel.").arg((int)secs.size()));
	return true;
}
