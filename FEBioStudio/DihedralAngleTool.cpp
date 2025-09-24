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
#include "DihedralAngleTool.h"
#include <GLLib/GDecoration.h>
#include <GeomLib/GObject.h>
#include <MeshLib/FSMesh.h>

QVariant CDihedralAngleTool::GetPropertyValue(int i)
{
	switch (i)
	{
	case 0: return m_face1; break;
	case 1: return m_face2; break;
	case 2: return m_angle; break;
	}
	return QVariant();
}

void CDihedralAngleTool::SetPropertyValue(int i, const QVariant& v)
{
	if (i == 0) m_face1 = v.toInt();
	if (i == 1) m_face2 = v.toInt();
}

CDihedralAngleTool::CDihedralAngleTool(CMainWindow* wnd) : CBasicTool(wnd, "Dihedral angle")
{
	m_face1 = 0;
	m_face2 = 0;
	m_angle = 0.0;

	addProperty("face 1", CProperty::Int);
	addProperty("face 2", CProperty::Int);
	addProperty("angle (deg.)", CProperty::Float)->setFlags(CProperty::Visible);

	SetInfo("Calculates the angle between the normals of two selected faces.");
}

void CDihedralAngleTool::Activate()
{
	CBasicTool::Activate();
	Update();
}

void CDihedralAngleTool::Update()
{
	SetDecoration(nullptr);
	m_angle = 0;

	FSMeshBase* mesh = GetActiveEditMesh();
	if (mesh == nullptr) return;

	int nsel = 0;
	for (int i = 0; i < mesh->Faces(); ++i)
	{
		FSFace& face = mesh->Face(i);
		int nid = i + 1;
		if (face.IsSelected())
		{
			nsel++;
			if (m_face1 == 0) m_face1 = nid;
			else if (m_face2 == 0) m_face2 = nid;
			else
			{
				m_face1 = m_face2;
				m_face2 = nid;
			}
		}
	}

	if (nsel == 0)
	{
		m_face1 = m_face2 = 0;
	}
	else if ((m_face1 > 0) && (m_face2 > 0))
	{
		FSFace& face1 = mesh->Face(m_face1 - 1);
		FSFace& face2 = mesh->Face(m_face2 - 1);

		double cosa = face1.m_fn * face2.m_fn;

		m_angle = acos(cosa) * RAD2DEG;
	}
}
