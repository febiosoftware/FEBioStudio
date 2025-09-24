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
#include "FaceMetricsTool.h"
#include <GLLib/GDecoration.h>
#include <GeomLib/GObject.h>
#include <MeshLib/FSMesh.h>

QVariant CFaceMetricsTool::GetPropertyValue(int i)
{
	switch (i)
	{
	case 0: return m_nsel; break;
	case 1: return Vec3dToString(m_N); break;
	case 2: return Vec3dToString(m_c); break;
	}
	return QVariant();
}

void CFaceMetricsTool::SetPropertyValue(int i, const QVariant& v)
{
}

CFaceMetricsTool::CFaceMetricsTool(CMainWindow* wnd) : CBasicTool(wnd, "Surface Metrics")
{
	m_N = vec3d(0, 0, 0);

	addProperty("faces"      , CProperty::Int )->setFlags(CProperty::Visible);
	addProperty("avg. normal", CProperty::Vec3)->setFlags(CProperty::Visible);
	addProperty("centroid"   , CProperty::Vec3)->setFlags(CProperty::Visible);

	SetInfo("Calculates some metrics of the currently selected faces.");
}

void CFaceMetricsTool::Activate()
{
	CBasicTool::Activate();
	Update();
}

void CFaceMetricsTool::Update()
{
	SetDecoration(nullptr);
	m_N = vec3d(0, 0, 0);

	m_c = vec3d(0, 0, 0);
	double A = 0;

	m_nsel = 0;

	FSMeshBase* mesh = GetActiveEditMesh();
	if (mesh == nullptr) return;

	for (int i = 0; i < mesh->Faces(); ++i)
	{
		FSFace& face = mesh->Face(i);
		if (face.IsSelected())
		{
			m_nsel++;

			vec3d Ni = to_vec3d(face.m_fn);
			m_N += Ni;

			double Ai = mesh->FaceArea(face);
			vec3d ci = mesh->FaceCenter(face);

			A += Ai;
			m_c += ci * Ai;
		}
	}
	m_N.Normalize();
	if (A != 0) m_c /= A;

	SetDecoration(new GPointDecoration(to_vec3f(m_c)));
}
