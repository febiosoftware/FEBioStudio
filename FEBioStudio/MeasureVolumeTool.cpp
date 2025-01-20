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
#include "MeasureVolumeTool.h"
#include <MeshLib/FSMesh.h>
#include <GeomLib/GObject.h>

//-----------------------------------------------------------------------------
QVariant CMeasureVolumeTool::GetPropertyValue(int i)
{
	switch (i)
	{
	case 0: return m_nformula; break;
	case 1: return m_vol; break;
	case 2: return m_faceCount; break;
	}
	return QVariant();
}

//-----------------------------------------------------------------------------
void CMeasureVolumeTool::SetPropertyValue(int i, const QVariant& v)
{
	if (i == 0) m_nformula = v.toInt();
}

//-----------------------------------------------------------------------------
CMeasureVolumeTool::CMeasureVolumeTool(CMainWindow* wnd) : CBasicTool(wnd, "Surface Volume", CBasicTool::HAS_APPLY_BUTTON)
{
	addProperty("symmetry", CProperty::Enum)->setEnumValues(QStringList() << "(None)" << "X" << "Y" << "Z");
	addProperty("volume", CProperty::Float)->setFlags(CProperty::Visible);
	addProperty("faces", CProperty::Int)->setFlags(CProperty::Visible);

	m_vol = 0.0;
	m_nformula = 0;
	m_faceCount = 0;

	SetInfo("Calculates the enclosed volume of the selected surface faces.");
}

//-----------------------------------------------------------------------------
bool CMeasureVolumeTool::OnApply()
{
	m_vol = 0.0;

	FSMesh* mesh = GetActiveMesh();
	if (mesh == nullptr) return false;

	GObject* po = mesh->GetGObject();
	if (po == nullptr) return false;

	const Transform& T = po->GetTransform();

	int selectedFaces = mesh->CountSelectedFaces();

	int NF = mesh->Faces();
	double z[2] = { 0,0 };
	m_faceCount = 0;
	for (int i = 0; i<NF; ++i)
	{
		FSFace& f = mesh->Face(i);
		if ((selectedFaces == 0) || (f.IsSelected()))
		{
			m_faceCount++;

			std::vector<vec3d> rt(f.Nodes());
			for (int n = 0; n < f.Nodes(); ++n) rt[n] = T.LocalToGlobal(mesh->Node(f.n[n]).r);

			// get the average position, area and normal
			double area = mesh->FaceArea(rt, f.Nodes());
			vec3d r = T.LocalToGlobal(mesh->FaceCenter(f));
			vec3d N = T.LocalToGlobalNormal(to_vec3d(f.m_fn));

			if (i == 0) z[0] = z[1] = r.z;
			else
			{
				if (r.z < z[0]) z[0] = r.z;
				if (r.z > z[1]) z[1] = r.z;
			}

			switch (m_nformula)
			{
			case 0: m_vol += area * (N * r) / 3.f; break;
			case 1: m_vol += 2.f * area * (r.x * N.x); break;
			case 2: m_vol += 2.f * area * (r.y * N.y); break;
			case 3: m_vol += 2.f * area * (r.z * N.z); break;
			}
		}
	}

	m_vol = -m_vol;	// we want inward facing surfaces to have a positive volume
	return true;
}
