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
#include "PlaneCutTool.h"
#include "ModelDocument.h"
#include <GeomLib/GMeshObject.h>
#include <MeshTools/FEModifier.h>

CPlaneCutTool::CPlaneCutTool(CMainWindow* wnd) : CBasicTool(wnd, "Plane cut", HAS_APPLY_BUTTON)
{
	addDoubleProperty(&m_r0.x, "X1");	
	addDoubleProperty(&m_r0.y, "Y1");
	addDoubleProperty(&m_r0.z, "Z1");

	addDoubleProperty(&m_r1.x, "X2");
	addDoubleProperty(&m_r1.y, "Y2");
	addDoubleProperty(&m_r1.z, "Z2");

	addDoubleProperty(&m_r2.x, "X3");
	addDoubleProperty(&m_r2.y, "Y3");
	addDoubleProperty(&m_r2.z, "Z3");
}

bool CPlaneCutTool::OnApply()
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());

	// get the currently selected object
	GObject* po = pdoc->GetActiveObject();
	if (po == 0)
	{
		SetErrorString("You must first select an object.");
		return false;
	}

	// make sure this is an editable mesh
	GMeshObject* pmo = dynamic_cast<GMeshObject*>(po);
	if (pmo == 0)
	{
		SetErrorString("This tool cannot be applied to this object.");
		return false;
	}

	// calculate the plane coefficients
	vec3d r0 = pmo->GetTransform().GlobalToLocal(m_r0);
	vec3d r1 = pmo->GetTransform().GlobalToLocal(m_r1);
	vec3d r2 = pmo->GetTransform().GlobalToLocal(m_r2);
	double a[4];
	vec3d n = (r1 - r0) ^ (r2 - r0);
	a[0] = n.x;
	a[1] = n.y;
	a[2] = n.z;
	a[3] = n*r0;

	// create the modifier
	FEPlaneCut mod;
	mod.SetPlaneCoefficients(a);
	if (pdoc->ApplyFEModifier(mod, pmo) == false)
	{
		SetErrorString(QString::fromStdString(mod.GetErrorString()));
		return false;
	}

	return true;
}
