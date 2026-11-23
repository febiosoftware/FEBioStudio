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
#include "ShellMeshFacesTool.h"
#include "ModelDocument.h"
#include "MainWindow.h"
#include "Commands.h"
#include <GeomLib/GOCCObject.h>
#include <GeomLib/GModel.h>
#include <GeomLib/GMeshObject.h>
#include <MeshTools/NetGenOCCMesher.h>
#include <MeshTools/NetGenFacetMesher.h>
#include <MeshTools/FacePatch.h>
#include <MeshTools/ShellExtruder.h>
#include <MeshLib/FSMesh.h>
#include <MeshLib/FSMeshBuilder.h>
#include <MeshTools/FESelection.h>
#include <QApplication>
#include <QMessageBox>

CShellMeshFacesTool::CShellMeshFacesTool(CMainWindow* wnd) : CBasicTool(wnd, "Shell Mesh Faces", HAS_APPLY_BUTTON)
{
	m_elemSize = 0.0;
	m_quad = true;
	m_hideSource = true;
	m_extrude = false;
	m_dirMode = 0;
	m_dir = vec3d(0, 0, 1);
	m_flip = false;
	m_distance = 1.0;
	m_layers = 1;
	m_bias = 1.0;
	m_keepShell = false;
	m_autoPartition = false;
	m_creaseAngle = 30.0;
	SetApplyButtonText("Mesh");
	addDoubleProperty(&m_elemSize, "max element size (0 = use object's mesher)");
	addBoolProperty(&m_quad, "quad dominant");
	addBoolProperty(&m_hideSource, "hide original object");
	addBoolProperty(&m_extrude, "extrude into solid (hex/penta)");
	addEnumProperty(&m_dirMode, "extrude direction")->setEnumValues(QStringList() << "local normal" << "average normal" << "user-defined");
	addVec3Property(&m_dir, "user direction (global)");
	addDoubleProperty(&m_distance, "extrude distance (negative = opposite direction)");
	addIntProperty(&m_layers, "extrude layers");
	addDoubleProperty(&m_bias, "layer bias (1 = uniform)");
	addBoolProperty(&m_keepShell, "keep shell object when extruding");
	addBoolProperty(&m_autoPartition, "auto-partition surfaces");
	addDoubleProperty(&m_creaseAngle, "crease angle (degrees)");
	SetInfo("Select one or more faces of any object, then press Mesh. "
		"A new shell object with a NetGen quad-dominant mesh of those faces is created. "
		"Faces of CAD (STEP/BREP) objects are meshed exactly; for other objects, planar faces are rebuilt as CAD faces, "
		"and curved faces are meshed from their triangulation.");
}

bool CShellMeshFacesTool::OnApply()
{
	CModelDocument* doc = GetMainWindow()->GetModelDocument();
	if ((doc == nullptr) || !doc->IsValid())
	{
		SetErrorString("No valid model document.");
		return false;
	}

	GObject* po = GetActiveObject();
	if (po == nullptr)
	{
		SetErrorString("You need to select an object.");
		return false;
	}

	// The faces can be selected as geometry surfaces (GFace) or as mesh faces
	FESelection* cursel = doc->GetCurrentSelection();
	GFaceSelection* sel = dynamic_cast<GFaceSelection*>(cursel);
	FEFaceSelection* fsel = dynamic_cast<FEFaceSelection*>(cursel);
	if (((sel == nullptr) || (sel->Count() == 0)) && ((fsel == nullptr) || (fsel->Count() == 0)))
	{
		QString msg = "No faces are selected. Select one or more surfaces (or mesh faces) of the object first.";
		msg += QString(" (current selection type: %1, items: %2)").arg(cursel ? cursel->Type() : -1).arg(cursel ? cursel->Size() : 0);
		SetErrorString(msg);
		return false;
	}

	// collect the selected faces that belong to this object
	std::vector<int> faceIds;
	FacePatch meshPatch;	// patch built directly from selected mesh faces
	bool useMeshFaces = false;
	if (sel && (sel->Count() > 0))
	{
		GFaceSelection::Iterator it(sel);
		for (int i = 0; i < sel->Size(); ++i, ++it)
		{
			GFace* gf = it;
			if (gf && (gf->Object() == po)) faceIds.push_back(gf->GetLocalID());
		}
		if (faceIds.empty())
		{
			SetErrorString("The selected faces do not belong to the active object.");
			return false;
		}
	}
	else
	{
		FSMeshBase* pm = fsel->GetMesh();
		if ((pm == nullptr) || ((pm != po->GetFEMesh()) && (pm != po->GetEditableMesh())))
		{
			SetErrorString("The selected mesh faces do not belong to the active object.");
			return false;
		}
		if (!ExtractMeshFacePatch(pm, fsel->ItemList(), meshPatch))
		{
			SetErrorString("Could not get the selected mesh faces.");
			return false;
		}
		useMeshFaces = true;
	}

	GOCCObject* occ = useMeshFaces ? nullptr : dynamic_cast<GOCCObject*>(po);
	GOCCObject* shell = nullptr;		// result if the faces could be turned into an OCC shell
	FSMesh* facetMesh = nullptr;		// result if the triangulation-based mesher was used
	double diag = 0.0;
	FacePatch patch;					// only used for the triangulation fallback

	if (occ)
	{
		// exact CAD faces
		shell = occ->CreateShellFromFaces(faceIds);
		if (shell == nullptr)
		{
			SetErrorString("Failed to create a shell from the selected faces.");
			return false;
		}
	}
	else
	{
		std::vector<FacePatch> patches;
		if (useMeshFaces) patches.push_back(meshPatch);
		if (!useMeshFaces && !ExtractFacePatches(po, faceIds, patches))
		{
			SetErrorString("Could not get a surface triangulation for the selected faces.");
			return false;
		}
		diag = PatchesDiagonal(patches);
		if (diag <= 0.0)
		{
			SetErrorString("The selected faces have no extent.");
			return false;
		}

		// planar faces --> rebuild as CAD faces; otherwise fall back to the triangulation
		shell = BuildPlanarOCCShell(patches);
		if (shell == nullptr) patch = MergePatches(patches);
	}

	// default element size for objects that don't have mesher settings
	double autoSize = (diag > 0.0 ? diag / 10.0 : 0.0);

	// configure the mesher
	NetGenMesher* mesher = nullptr;
	NetGenFacetMesher* facetMesher = nullptr;
	if (shell)
	{
		mesher = dynamic_cast<NetGenOCCMesher*>(shell->GetFEMesher());
		NetGenOCCMesher* parent = (occ ? dynamic_cast<NetGenOCCMesher*>(occ->GetFEMesher()) : nullptr);
		if (mesher && parent) mesher->CopyParams(*parent);
	}
	else
	{
		facetMesher = new NetGenFacetMesher(*po);
		facetMesher->SetPatch(patch);
		mesher = facetMesher;
	}
	if (mesher == nullptr)
	{
		delete shell;
		SetErrorString("Failed to create the NetGen mesher.");
		return false;
	}

	mesher->SetBoolValue(NetGenMesher::QUADMESH, m_quad);
	mesher->SetBoolValue(NetGenMesher::SURFREFINE, false);
	if (m_elemSize > 0.0)
	{
		mesher->SetFloatValue(NetGenMesher::MAXELEMSIZE, m_elemSize);
		if (!occ) mesher->SetFloatValue(NetGenMesher::MINELEMSIZE, m_elemSize * 0.01);
	}
	else if (!occ && (autoSize > 0.0))
	{
		mesher->SetFloatValue(NetGenMesher::MAXELEMSIZE, autoSize);
		mesher->SetFloatValue(NetGenMesher::MINELEMSIZE, autoSize * 0.01);
	}

	// mesh it
	QApplication::setOverrideCursor(Qt::WaitCursor);
	FSMesh* mesh = nullptr;
	if (shell) mesh = shell->BuildMesh();
	else mesh = facetMesh = facetMesher->BuildMesh();
	QApplication::restoreOverrideCursor();

	if (mesh == nullptr)
	{
		std::string err = mesher->GetErrorString();
		delete shell;
		delete facetMesher;
		SetErrorString(err.empty() ? "NetGen failed to mesh the selected faces." : QString::fromStdString(err));
		return false;
	}

	if (m_autoPartition && ((m_creaseAngle <= 0.0) || (m_creaseAngle >= 180.0)))
	{
		if (shell) { delete shell; shell = nullptr; }
		delete facetMesher;
		delete facetMesh;
		SetErrorString("The crease angle must be between 0 and 180 degrees.");
		return false;
	}

	// optional extrusion of the shell mesh into a solid mesh
	FSMesh* solid = nullptr;
	if (m_extrude)
	{
		ShellExtruder ext;
		ext.m_mode = (m_dirMode == 1 ? ShellExtruder::AVERAGE_NORMAL : (m_dirMode == 2 ? ShellExtruder::USER_DIRECTION : ShellExtruder::LOCAL_NORMAL));
		// the user direction is in global coordinates; the mesh is in the object's local coordinates
		const Transform& T = po->GetTransform();
		ext.m_direction = T.GlobalToLocal(m_dir) - T.GlobalToLocal(vec3d(0, 0, 0));
		ext.m_flip = m_flip;
		ext.m_distance = m_distance;
		ext.m_layers = m_layers;
		ext.m_bias = m_bias;
		ext.m_autoPartition = m_autoPartition;
		ext.m_creaseAngle = m_creaseAngle;

		FSMesh* shellMesh = mesh;
		solid = ext.Apply(*shellMesh);
		if (solid == nullptr)
		{
			QString err = QString::fromStdString(ext.GetErrorString());
			delete shell;
			delete facetMesher;
			delete facetMesh;
			SetErrorString("Extrusion failed: " + err);
			return false;
		}
	}

	int nelems = (solid ? solid->Elements() : mesh->Elements());

	// the new object(s)
	GObject* newObj = shell;
	if (newObj == nullptr)
	{
		// (CAD shells already have their faces from the CAD geometry)
		if (m_autoPartition)
		{
			// same as the Auto Partition modifier in the Mesh panel
			FSMeshBuilder meshBuilder(*facetMesh);
			meshBuilder.AutoPartition(m_creaseAngle);
		}
		newObj = new GMeshObject(facetMesh);
	}
	delete facetMesher;

	static int n = 1;
	QString name = QString("%1_shell%2").arg(QString::fromStdString(po->GetName())).arg(n++);
	newObj->SetName(name.toStdString());
	newObj->CopyTransform(po);

	GObject* solidObj = nullptr;
	if (solid)
	{
		QString sname = QString("%1_extrude%2").arg(QString::fromStdString(po->GetName())).arg(n - 1);
		solidObj = new GMeshObject(solid);
		solidObj->SetName(sname.toStdString());
		solidObj->CopyTransform(po);
		name = sname;
	}

	CCmdGroup* cmd = new CCmdGroup("Shell mesh faces");
	if (solidObj)
	{
		if (m_keepShell)
		{
			cmd->AddCommand(new CCmdAddObject(doc->GetGModel(), newObj));
		}
		else delete newObj;
		cmd->AddCommand(new CCmdAddAndSelectObject(doc->GetGModel(), solidObj));
	}
	else cmd->AddCommand(new CCmdAddAndSelectObject(doc->GetGModel(), newObj));
	if (m_hideSource) cmd->AddCommand(new CCmdHideObject(po));
	doc->DoCommand(cmd, name.toStdString());

	QMessageBox::information(GetMainWindow(), "Shell Mesh Faces",
		QString("Created %1 (%2 elements)").arg(name).arg(nelems));
	return true;
}
