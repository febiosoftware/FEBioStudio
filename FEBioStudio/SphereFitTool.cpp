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
#include "SphereFitTool.h"
#include <MeshTools/SphereFit.h>
#include <GeomLib/GObject.h>
#include <MeshLib/FSMesh.h>
#include <GLLib/GDecoration.h>

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

	SetInfo("Fits the selected nodes to a sphere.");
}

bool CSphereFitTool::OnApply()
{
	SetDecoration(nullptr);

	// get the nodal coordinates (surface only)
	GObject* po = GetActiveObject();
	FSMesh* activeMesh = (po ? po->GetFEMesh() : nullptr);
	if (activeMesh == nullptr)
	{
		SetErrorString("You must select an object that has a mesh.");
		return false;
	}

	FSMesh& mesh = *activeMesh;

	int N = mesh.Nodes();
	int F = mesh.Faces();
	for (int i=0; i<N; ++i) mesh.Node(i).m_ntag = 0;
	for (int i=0; i<F; ++i)
	{
		FSFace& f = mesh.Face(i);
		if ((m_bsel == false) || (f.IsSelected()))
		{
			int nf = f.Nodes();
			for (int j=0; j<nf; ++j) mesh.Node(f.n[j]).m_ntag = 1;
		}
	}

	std::vector<vec3d> y;
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

	SetDecoration(new GSphereDecoration(to_vec3f(sc), R));

	return true;
}
