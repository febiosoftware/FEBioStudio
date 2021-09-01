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
#include "MeasureVolumeTool.h"
#include <MeshLib/FEMesh.h>

//-----------------------------------------------------------------------------
QVariant CMeasureVolumeTool::GetPropertyValue(int i)
{
	switch (i)
	{
	case 0: return m_nformula; break;
	case 1: return m_vol; break;
	}
	return QVariant();
}

//-----------------------------------------------------------------------------
void CMeasureVolumeTool::SetPropertyValue(int i, const QVariant& v)
{
	if (i == 2) m_nformula = v.toInt();
}

//-----------------------------------------------------------------------------
CMeasureVolumeTool::CMeasureVolumeTool(CMainWindow* wnd) : CBasicTool(wnd, "Surface Volume", CBasicTool::HAS_APPLY_BUTTON)
{
	addProperty("symmetry", CProperty::Enum)->setEnumValues(QStringList() << "(None)" << "X" << "Y" << "Z");
	addProperty("volume", CProperty::Float)->setFlags(CProperty::Visible);

	m_vol = 0.0;
	m_nformula = 0;
}

//-----------------------------------------------------------------------------
bool CMeasureVolumeTool::OnApply()
{
	m_vol = 0.0;

	FEMesh* mesh = GetActiveMesh();
	if (mesh == nullptr) return false;

	int NF = mesh->Faces();
	for (int i = 0; i<NF; ++i)
	{
		FEFace& f = mesh->Face(i);

		// get the average position, area and normal
		vec3d r = mesh->FaceCenter(f);
		double area = mesh->FaceArea(f);
		vec3d N = to_vec3d(f.m_fn);

		switch (m_nformula)
		{
		case 0: m_vol += area*(N*r) / 3.f; break;
		case 1: m_vol += 2.f*area*(r.x*N.x); break;
		case 2: m_vol += 2.f*area*(r.y*N.y); break;
		case 3: m_vol += 2.f*area*(r.z*N.z); break;
		}
	}

	m_vol = fabs(m_vol);
	return true;
}
