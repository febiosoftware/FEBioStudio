#include "stdafx.h"
#include "SphereFitTool.h"
#include <MeshTools/SphereFit.h>
#include <GeomLib/GObject.h>
#include <MeshLib/FEMesh.h>

// constructor
CSphereFitTool::CSphereFitTool(CMainWindow* wnd) : CBasicTool(wnd, "Sphere Fit", HAS_APPLY_BUTTON)
{
	m_bsel = false;
	m_x = m_y = m_z = 0.0;
	m_R = 0.0;
	m_obj = 0;
	addBoolProperty(&m_bsel, "selection only");
	addDoubleProperty(&m_x, "x")->setFlags(CProperty::Visible);
	addDoubleProperty(&m_y, "y")->setFlags(CProperty::Visible);
	addDoubleProperty(&m_z, "z")->setFlags(CProperty::Visible);
	addDoubleProperty(&m_R, "R")->setFlags(CProperty::Visible);
	addDoubleProperty(&m_obj, "obj")->setFlags(CProperty::Visible);

	SetApplyButtonText("Fit");
}

bool CSphereFitTool::OnApply()
{
	// get the nodal coordinates (surface only)
	GObject* po = GetActiveObject();
	FEMesh* activeMesh = (po ? po->GetFEMesh() : nullptr);
	if (activeMesh == nullptr)
	{
		SetErrorString("You must select an object that has a mesh.");
		return false;
	}

	FEMesh& mesh = *activeMesh;

	int N = mesh.Nodes();
	int F = mesh.Faces();
	for (int i=0; i<N; ++i) mesh.Node(i).m_ntag = 0;
	for (int i=0; i<F; ++i)
	{
		FEFace& f = mesh.Face(i);
		if ((m_bsel == false) || (f.IsSelected()))
		{
			int nf = f.Nodes();
			for (int j=0; j<nf; ++j) mesh.Node(f.n[j]).m_ntag = 1;
		}
	}

	vector<vec3d> y;
	for (int i=0; i<N; ++i)
	{
		if (mesh.Node(i).m_ntag == 1) y.push_back(po->GetTransform().LocalToGlobal(mesh.Node(i).r));
	}

	// find the best fit sphere
	SphereFit fit;
	fit.Fit(y, 50);
	vec3d sc = fit.m_rc;
	double R = fit.m_R;

	// calculate the objective function
	double objs = fit.ObjFunc(y);

	// update GUI
	m_x = sc.x;
	m_y = sc.y;
	m_z = sc.z;
	m_R = R;
	m_obj = objs;

	return true;
}
