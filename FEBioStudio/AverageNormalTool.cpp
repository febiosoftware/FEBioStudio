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
#include "AverageNormalTool.h"
#include <GLLib/GDecoration.h>
#include <GeomLib/GObject.h>
#include <MeshLib/FEMesh.h>

QVariant CAverageNormalTool::GetPropertyValue(int i)
{
	switch (i)
	{
	case 0: return m_N.x; break;
	case 1: return m_N.y; break;
	case 2: return m_N.z; break;
	}
	return QVariant();
}

void CAverageNormalTool::SetPropertyValue(int i, const QVariant& v)
{
}

CAverageNormalTool::CAverageNormalTool(CMainWindow* wnd) : CBasicTool(wnd, "Average Normal")
{
	m_N = vec3d(0, 0, 0);

	addProperty("Nx", CProperty::Float)->setFlags(CProperty::Visible);
	addProperty("Ny", CProperty::Float)->setFlags(CProperty::Visible);
	addProperty("Nz", CProperty::Float)->setFlags(CProperty::Visible);

	SetInfo("Calculates the average normal of the currently selected faces.");
}

void CAverageNormalTool::Activate()
{
	CBasicTool::Activate();
	Update();
}

void CAverageNormalTool::Update()
{
	SetDecoration(nullptr);
	m_N = vec3d(0, 0, 0);

	FSMeshBase* mesh = GetActiveEditMesh();
	if (mesh == nullptr) return;

	vec3f N(0,0,0);
	for (int i = 0; i < mesh->Faces(); ++i)
	{
		FSFace& face = mesh->Face(i);
		if (face.IsSelected())
		{
			N += face.m_fn;
		}
	}
	N.Normalize();

	m_N = to_vec3d(N);
}
