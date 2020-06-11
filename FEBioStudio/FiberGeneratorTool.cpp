/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
#include "FiberGeneratorTool.h"
#include "Document.h"
#include <MeshTools/GradientMap.h>
#include <GeomLib/GObject.h>
#include <MeshTools/FENodeData.h>

CFiberGeneratorTool::CFiberGeneratorTool(CMainWindow* wnd) : CBasicTool(wnd, "Fiber generator", HAS_APPLY_BUTTON)
{
	m_niter = 0;
	m_ndata = 0;

	addIntProperty(&m_ndata, "Data field");
	addIntProperty(&m_niter, "smoothing iterations");
}

bool CFiberGeneratorTool::OnApply()
{
	CDocument* pdoc = GetDocument();
	GObject* po = pdoc->GetActiveObject();
	if (po == 0) 
	{
		SetErrorString("You must select an object first.");
		return false;
	}

	FEMesh* pm = po->GetFEMesh();
	if (pm == 0) 
	{
		SetErrorString("The selected object does not have a mesh.");
		return false;
	}

	int N = pm->MeshDataFields();
	if ((m_ndata >= 0) && (m_ndata < N))
	{
		// get node data field
		FENodeData& D = dynamic_cast<FENodeData&>(*pm->GetMeshDataField(m_ndata));

		// calculate gradient and assign to element fiber
		vector<vec3d> grad;
		GradientMap G;
		G.Apply(D, grad, m_niter);

		// assign to element fibers
		int NE = pm->Elements();
		for (int i = 0; i<NE; ++i)
		{
			FEElement& el = pm->Element(i);
			el.m_fiber = grad[i];
		}
	}

	return true;
}
