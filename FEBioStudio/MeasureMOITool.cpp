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
#include "MeasureMOITool.h"
#include "MeasureCOMTool.h" // for center of mass calculation
#include <MeshLib/FSMesh.h>

mat3d CalculateMOI(FSMesh& mesh);

// constructor
CMeasureMOITool::CMeasureMOITool(CMainWindow* wnd) : CBasicTool(wnd, "Moment of inertia", CBasicTool::HAS_APPLY_BUTTON)
{
	m_moi.zero();
    m_evec.zero();
    m_eval = vec3d(0,0,0);
	addMat3Property(&m_moi, "moment of inertia:")->setFlags(CProperty::Visible);
    addMat3Property(&m_evec, "eigenvectors of MOI:")->setFlags(CProperty::Visible);
    addVec3Property(&m_eval, "eigenvalues of MOI:")->setFlags(CProperty::Visible);

	SetInfo("Calculates the moment of inertia of a meshed object or element selection. The MOI is calculated with respect to the center of mass of the selection.");
}

// This is called when Apply button is pressed.
bool CMeasureMOITool::OnApply()
{
	FSMesh* mesh = GetActiveMesh();
	if (mesh == nullptr) return false;
	m_moi = CalculateMOI(*mesh);
    double eval[3];
    vec3d evec[3];
    mat3ds mois = m_moi.sym();
    mois.eigen2(eval,evec);
    m_evec = mat3d(evec[0].x, evec[1].x, evec[2].x,
                   evec[0].y, evec[1].y, evec[2].y,
                   evec[0].z, evec[1].z, evec[2].z);
    m_eval = vec3d(eval[0],eval[1],eval[2]);
	return true;
}

// This calculates the moment of inertia, but only approximately!
// Requires a fine mesh for accurate results. 
mat3d CalculateMOI(FSMesh& mesh)
{
	mat3dd I(1);        // identity tensor

	mat3d moi; moi.zero();

	// calculate the COM
	vec3d com = CalculateCOM(mesh);

	// loop over all elements
	int NE = mesh.Elements();
	for (int iel = 0; iel < NE; ++iel)
	{
		FSElement& el = mesh.Element(iel);

		// ElementCenter is calculated in local frame
		vec3d c_local = mesh.ElementCenter(el);
		vec3d c_global = mesh.LocalToGlobal(c_local);

		// Element volume is calculated in global frame
		double V0 = mesh.ElementVolume(el);

		vec3d r = c_global - com;
		mat3d Iij = (r * r) * I - (r & r);
		moi += Iij * V0;
	}

	const double eps = 1e-12;
	double L = moi.norm();
	if (L > eps)
	{
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
				if (fabs(moi[i][j]) < eps) moi[i][j] = 0.0;
	}
	return moi;
}
