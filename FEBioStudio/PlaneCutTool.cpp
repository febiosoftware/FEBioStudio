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
#include "PlaneCutTool.h"
#include "ModelDocument.h"
#include <GeomLib/GMeshObject.h>
#include <MeshTools/FEModifier.h>
#include <GLLib/GDecoration.h>

CPlaneCutTool::CPlaneCutTool(CMainWindow* wnd) : CBasicTool(wnd, "Plane cut", HAS_APPLY_BUTTON)
{
	m_pick = 0;

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

void CPlaneCutTool::Activate()
{
	CBasicTool::Activate();
	m_pick = 0;
}


bool CPlaneCutTool::onPickEvent(const FESelection& sel)
{
	const FENodeSelection* nodeSel = dynamic_cast<const FENodeSelection*>(&sel);
	if (nodeSel && (nodeSel->Count() == 1))
	{
		int nid = nodeSel->NodeIndex(0);
		const FSLineMesh* mesh = nodeSel->GetMesh();

		vec3d r = mesh->NodePosition(nid);

		if (m_pick >= 3) m_pick = 0;

		switch (m_pick)
		{
		case 0: SetPropertyValue(0, r.x); SetPropertyValue(1, r.y); SetPropertyValue(2, r.z); break;
		case 1: SetPropertyValue(3, r.x); SetPropertyValue(4, r.y); SetPropertyValue(5, r.z); break;
		case 2: SetPropertyValue(6, r.x); SetPropertyValue(7, r.y); SetPropertyValue(8, r.z); break;
		}
		m_pick++;

		BuildDecoration();

		updateUi();

		return true;
	}
	return false;
}

void CPlaneCutTool::BuildDecoration()
{
	GCompositeDecoration* deco = new GCompositeDecoration;
	if (m_pick == 1)
	{
		vec3f r0;
		r0.x = GetPropertyValue(0).toDouble();
		r0.y = GetPropertyValue(1).toDouble();
		r0.z = GetPropertyValue(2).toDouble();
		deco->AddDecoration(new GPointDecoration(r0));
	}
	else if (m_pick == 2)
	{
		vec3f r0, r1;
		r0.x = GetPropertyValue(0).toDouble();
		r0.y = GetPropertyValue(1).toDouble();
		r0.z = GetPropertyValue(2).toDouble();
		r1.x = GetPropertyValue(3).toDouble();
		r1.y = GetPropertyValue(4).toDouble();
		r1.z = GetPropertyValue(5).toDouble();
		GPointDecoration* p0 = new GPointDecoration(r0);
		GPointDecoration* p1 = new GPointDecoration(r1);
		deco->AddDecoration(new GLineDecoration(p0, p1));
	}
	else if (m_pick == 3)
	{
		vec3f r0, r1, r2;
		r0.x = GetPropertyValue(0).toDouble();
		r0.y = GetPropertyValue(1).toDouble();
		r0.z = GetPropertyValue(2).toDouble();
		r1.x = GetPropertyValue(3).toDouble();
		r1.y = GetPropertyValue(4).toDouble();
		r1.z = GetPropertyValue(5).toDouble();
		r2.x = GetPropertyValue(6).toDouble();
		r2.y = GetPropertyValue(7).toDouble();
		r2.z = GetPropertyValue(8).toDouble();
		deco->AddDecoration(new GTriangleDecoration(r0, r1, r2));
	}
	SetDecoration(deco);
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
