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
#include "MeasureAreaTool.h"
#include <MeshLib/FSMesh.h>

//-----------------------------------------------------------------------------
CMeasureAreaTool::CMeasureAreaTool(CMainWindow* wnd) : CBasicTool(wnd, "Measure Area")
{
	addProperty("selected faces", CProperty::Int)->setFlags(CProperty::Visible);
	addProperty("area", CProperty::Float)->setFlags(CProperty::Visible);

	m_nsel = 0;
	m_area = 0.0;

	SetInfo("Calculates the total area of the selected faces.");
}

//-----------------------------------------------------------------------------
QVariant CMeasureAreaTool::GetPropertyValue(int i)
{
	switch (i)
	{
	case 0: return m_nsel; break;
	case 1: return m_area; break;
	}
	return QVariant();
}

//-----------------------------------------------------------------------------
void CMeasureAreaTool::SetPropertyValue(int i, const QVariant& v)
{
}

//-----------------------------------------------------------------------------
void CMeasureAreaTool::Update()
{
	m_nsel = 0;
	m_area = 0.0;
	FSMesh* mesh = GetActiveMesh();
	if (mesh == nullptr) return;

	int NF = mesh->Faces();
	for (int i = 0; i<NF; ++i)
	{
		FSFace& f = mesh->Face(i);
		if (f.IsSelected())
		{
			++m_nsel;
			m_area += mesh->FaceArea(f);
		}
	}
}
