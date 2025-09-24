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
#include "MeasureCOMTool.h"
#include <GeomLib/GObject.h>
#include <MeshLib/FSMesh.h>

// constructor
CMeasureCOMTool::CMeasureCOMTool(CMainWindow* wnd) : CBasicTool(wnd, "Center of mass", CBasicTool::HAS_APPLY_BUTTON)
{
	m_com = vec3d(0,0,0);
	addVec3Property(&m_com, "center of mass:")->setFlags(CProperty::Visible);

	SetInfo("Calculates the center of mass of a meshed object or element selection.");
}

// This is called when Apply button is pressed.
bool CMeasureCOMTool::OnApply()
{
	FSMesh* mesh = GetActiveMesh();
	if (mesh == nullptr) return false;
	m_com = CalculateCOM(*mesh);
	return true;
}

// This algorithm calculates the center of mass approximately,
// by weighing the element centers by the element volumes. 
// TODO: I should look into bringing the numerical integration tools
//       from FEBio to FBS so this can be calculated accurately. 
vec3d CalculateCOM(FSMesh& mesh)
{
	vec3d rc(0, 0, 0);
	double Vtotal = 0.0;

	int selectedElements = mesh.CountSelectedElements();

	// loop over all elements
	int NE = mesh.Elements();
	for (int iel = 0; iel < NE; ++iel)
	{
		FSElement& el = mesh.Element(iel);
		if ((selectedElements == 0) || el.IsSelected())
		{
			// Volume is calculated in global frame
			double V0 = mesh.ElementVolume(el);

			// center of mass is calculated in local frame
			vec3d c_local = mesh.ElementCenter(el);
			vec3d c_global = mesh.LocalToGlobal(c_local);

			rc += c_global * V0;
			Vtotal += V0;
		}
	}
	if (Vtotal != 0.0)
	rc /= Vtotal;

	const double eps = 1e-12;
	double L = rc.Length();
	if (L > eps)
	{
		if (fabs(rc.x) < eps) rc.x = 0;
		if (fabs(rc.y) < eps) rc.y = 0;
		if (fabs(rc.z) < eps) rc.z = 0;
	}

	return rc;
}
