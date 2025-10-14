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
#include "AlignMeshMOI.h"
#include "MeasureTools.h"
#include <GeomLib/GObject.h>
#include "Commands.h"
#include "GLDocument.h"

CAlignMeshMOITool::CAlignMeshMOITool(CMainWindow* wnd) : CBasicTool(wnd, "Align Mesh MOI with CS", HAS_APPLY_BUTTON)
{
	addBoolProperty(&m_useArea, "Use area MOI");
}

bool CAlignMeshMOITool::OnApply()
{
	GObject* po = GetActiveObject();
	if (po == nullptr) {
		SetErrorString("No object selected.");
		return false;
	}

	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr)
	{
		SetErrorString("The selected object does not have a mesh.");
		return false;
	}

	mat3d moi = (m_useArea ? CalculateAreaMOI(*pm) : CalculateMOI(*pm));
	double eval[3];
	vec3d evec[3];
	mat3ds mois = moi.sym();
	mois.eigen2(eval, evec);
	// sort eigenvalues in ascending order
	std::vector<int> indices(3);
	std::iota(indices.begin(), indices.end(), 0);
	std::vector<double> ev(eval, eval + 3);
	std::sort(indices.begin(), indices.end(), [&](int A, int B)->bool {return ev[A] < ev[B]; });
	// check handedness of eigenvectors and swap if needed
	if (((evec[indices[0]] ^ evec[indices[1]]) * evec[indices[2]]) < 0)
		evec[indices[2]] = -evec[indices[2]];


	mat3d fevec = mat3d(evec[indices[0]].x, evec[indices[0]].y, evec[indices[0]].z,
		evec[indices[1]].x, evec[indices[1]].y, evec[indices[1]].z,
		evec[indices[2]].x, evec[indices[2]].y, evec[indices[2]].z);
	vec3d feval = vec3d(eval[indices[0]], eval[indices[1]], eval[indices[2]]);

	// roundoff small numbers
	const double eps = 1e-12;
	fevec = CleanUp(fevec, eps);

	// create a quaternion from the matrix of eigenvectors
	quatd q(fevec);
	q.MakeUnit();

	Transform T = po->GetTransform();

    vec3d com = (m_useArea ? CalculateAreaCOM(*pm) : CalculateCOM(*pm));
	T.Rotate(q, com);
	T.Translate(-com);

	CGLDocument* doc = GetDocument();
	if (doc)
	{
		doc->DoCommand(new CCmdTransformObject(po, T));
	}
	else
	{
		// Don't think we should ever get here, but just in case
		po->GetTransform() = T;
	}

	return true;
}
