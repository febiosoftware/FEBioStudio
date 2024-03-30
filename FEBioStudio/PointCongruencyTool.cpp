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
#include "PointCongruencyTool.h"
#include <QWidget>
#include <QBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QFormLayout>
#include <QLineEdit>
#include "Document.h"
#include <PostLib/FEPostModel.h>
#include <MeshTools/SphereFit.h>
#include "PropertyListView.h"
#include <PostLib/FEPointCongruency.h>

// constructor
CPointCongruencyTool::CPointCongruencyTool(CMainWindow* wnd) : CBasicTool(wnd, "Pt. Congruency", CBasicTool::HAS_APPLY_BUTTON)
{
	addIntProperty(&m_node, "Node");
	addDoubleProperty(&m_smooth, "Smoothness");

	addIntProperty(&m_face, "Face")->setFlags(CProperty::Visible);
	addDoubleProperty(&m_H1, "H1")->setFlags(CProperty::Visible);
	addDoubleProperty(&m_G1, "G1")->setFlags(CProperty::Visible);
	addDoubleProperty(&m_H2, "H2")->setFlags(CProperty::Visible);
	addDoubleProperty(&m_G2, "G2")->setFlags(CProperty::Visible);
	addDoubleProperty(&m_alpha, "alpha")->setFlags(CProperty::Visible);
	addDoubleProperty(&m_delta, "delta")->setFlags(CProperty::Visible);
	addDoubleProperty(&m_Kemin, "Ke_min")->setFlags(CProperty::Visible);
	addDoubleProperty(&m_Kemax, "Ke_max")->setFlags(CProperty::Visible);
	addDoubleProperty(&m_cong, "Congruency")->setFlags(CProperty::Visible);

	SetInfo("Calculates the congruency at a point.");
}

bool CPointCongruencyTool::OnApply()
{
	// get the nodal coordinates (surface only)
	FSMesh* mesh = GetActiveMesh();
	if (mesh == nullptr) return false;

	int node = m_node - 1;
	if ((node >= 0)&&(node<mesh->Nodes()))
	{
		Post::FEPointCongruency tool;
		tool.SetLevels(1);
		Post::FEPointCongruency::CONGRUENCY_DATA d = tool.Congruency(mesh, node);

		m_face = d.nface + 1;
		m_H1 = d.H1;
		m_G1 = d.G1;
		m_H2 = d.H2;
		m_G2 = d.G2;

		m_alpha = 180 * d.a / PI;
		m_delta = d.D;

		m_Kemin = d.Kemin;
		m_Kemax = d.Kemax;
		m_cong = d.Ke;
        return true;
	}
    return false;
}
