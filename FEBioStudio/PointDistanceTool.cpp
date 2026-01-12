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
#include "PointDistanceTool.h"
#include <GLLib/GDecoration.h>
#include <GeomLib/GObject.h>
#include <MeshTools/FESelection.h>
#include <MeshLib/FSMesh.h>

QVariant CPointDistanceTool::GetPropertyValue(int i)
{
	switch (i)
	{
	case 0: break; // group
	case 1: return m_node[0]; break;
	case 2: return m_node[1]; break;
	case 3: break; // group
	case 4: return fabs(m_d.x); break;
	case 5: return fabs(m_d.y); break;
	case 6: return fabs(m_d.z); break;
	case 7: return m_d.Length(); break;
	}
	return QVariant();
}

void CPointDistanceTool::SetPropertyValue(int i, const QVariant& v)
{
	if (i == 1)
	{
		int n = v.toInt();
		if (n != m_node[0])
		{
			m_node[0] = n;
			UpdateDistance();
			SetModified(true);
		}
	}
	if (i == 2)
	{
		int n = v.toInt();
		if (n != m_node[1])
		{
			m_node[1] = n;
			UpdateDistance();
			SetModified(true);
		}
	}
}

CPointDistanceTool::CPointDistanceTool(CMainWindow* wnd) : CBasicTool(wnd, "Point Distance")
{ 
	m_node[0] = 0; 
	m_node[1] = 0; 
	m_d = vec3d(0,0,0); 

	addProperty("Select two nodes:", CProperty::Group);
	addProperty("node 1", CProperty::Int);
	addProperty("node 2", CProperty::Int);
	addProperty("Distance Components:", CProperty::Group);
	addProperty("Dx", CProperty::Float)->setFlags(CProperty::Visible);
	addProperty("Dy", CProperty::Float)->setFlags(CProperty::Visible);
	addProperty("Dz", CProperty::Float)->setFlags(CProperty::Visible);
	addProperty("Length", CProperty::Float)->setFlags(CProperty::Visible);

	SetInfo("Calculates the distance between two nodes.");
}

void CPointDistanceTool::Activate()
{
	CBasicTool::Activate();
	Update();
}

bool CPointDistanceTool::onPickEvent(const FESelection& sel)
{
	FESelection& s = const_cast<FESelection&>(sel);
	FENodeSelection* nodeSel = dynamic_cast<FENodeSelection*>(&s);
	if (nodeSel == nullptr) return false;

	if (nodeSel->Count() == 0)
	{
		m_node[0] = m_node[1] = 0;
	}
	else
	{
		int N = nodeSel->Count();
		FENodeSelection::Iterator it = nodeSel->First();
		for (int i = 0; i < N; ++i, ++it)
		{
			int nid = it->GetID();
			if (nid == -1) nid = nodeSel->NodeIndex(i) + 1;
			if (m_node[0] == 0) m_node[0] = nid;
			else if (m_node[1] == 0) m_node[1] = nid;
			else
			{
				m_node[0] = m_node[1];
				m_node[1] = nid;
			}
		}
	}
	UpdateDistance();
	updateUi();
}

void CPointDistanceTool::Update()
{
	UpdateDistance();
}

void CPointDistanceTool::UpdateDistance()
{
	SetDecoration(nullptr);
	m_d = vec3d(0, 0, 0);

	FSMeshBase* mesh = GetActiveEditMesh();
	if (mesh == nullptr) return;

	if ((m_node[0] > 0) && (m_node[1] > 0))
	{
		int n0 = mesh->NodeIndexFromID(m_node[0]);
		int n1 = mesh->NodeIndexFromID(m_node[1]);
		if ((n0 > 0) && (n1 > 0))
		{
			vec3d a = mesh->NodePosition(n0);
			vec3d b = mesh->NodePosition(n1);

			m_d = b - a;

			GCompositeDecoration* deco = new GCompositeDecoration;
			deco->AddDecoration(new GPointDecoration(to_vec3f(a)));
			deco->AddDecoration(new GPointDecoration(to_vec3f(b)));
			deco->AddDecoration(new GLineDecoration(to_vec3f(a), to_vec3f(b)));
			SetDecoration(deco);
		}
	}
}
