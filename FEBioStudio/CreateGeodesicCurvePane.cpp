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
#include "CreateGeodesicCurvePane.h"
#include <QLabel>
#include <QGridLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QValidator>
#include <QSpinBox>
#include <GeomLib/GCurveMeshObject.h>
#include <MeshLib/FSCurveMesh.h>
#include "GLHighlighter.h"
#include <MeshTools/FEGeodesic.h>
#include <GLLib/GLMesh.h>
#include "PropertyList.h" // for Vec3dToString
#include "ModelDocument.h"

//=============================================================================
CCreateGeodesicCurvePane::CCreateGeodesicCurvePane(CCreatePanel* parent) : CCreatePane(parent)
{
	m_tmp = nullptr;
	m_input = 0;

	QFormLayout* form = new QFormLayout;
	form->addRow("Point 1: ", m_in[0] = new QLineEdit);
	form->addRow("Point 2: ", m_in[1] = new QLineEdit);
	form->addRow("Divisions: ", m_div = new QSpinBox);
	m_div->setRange(1, 100);
	m_div->setValue(25); m_div->setObjectName("divs");

	QVBoxLayout* l = new QVBoxLayout;
	l->setContentsMargins(0, 0, 0, 0);
	l->addLayout(form);
	l->addStretch();

	setLayout(l);

	QMetaObject::connectSlotsByName(this);
}

void CCreateGeodesicCurvePane::hideEvent(QHideEvent* ev)
{
	if (m_tmp)
	{
		GLHighlighter::ClearHighlights();
		m_parent->SetTempObject(0);
		delete m_tmp;
		m_tmp = nullptr;
	}
}

void CCreateGeodesicCurvePane::on_divs_editingFinished()
{
	if (m_tmp) BuildGeodesic();
}

void CCreateGeodesicCurvePane::setInput(const vec3d& r)
{
	if (m_input == 0)
	{
		m_parent->SetTempObject(nullptr);
		GLHighlighter::ClearHighlights();
		if (m_tmp) { delete m_tmp; m_tmp = nullptr; }
		m_r[0] = r;
		m_in[0]->setText(Vec3dToString(r));
		m_input = 1;
	}
	else if (m_input == 1)
	{
		m_r[1] = r;
		m_in[1]->setText(Vec3dToString(r));
		m_input = 0;
		BuildGeodesic();
	}
	else { assert(false); m_input = 0; }
}

void CCreateGeodesicCurvePane::Activate()
{
}

void CCreateGeodesicCurvePane::Deactivate()
{
	m_parent->SetTempObject(nullptr);
	GLHighlighter::ClearHighlights();
	delete m_tmp;
	m_tmp = nullptr;
}

bool CCreateGeodesicCurvePane::Clear()
{
	if (m_tmp)
	{
		m_tmp->Clear();
		m_tmp->Update();
		GLHighlighter::ClearHighlights();
		return true;
	}
	else return false;
}

FSObject* CCreateGeodesicCurvePane::Create()
{
	static int n = 1;

	if (m_tmp == nullptr) return nullptr;

	GObject* po = m_tmp;
	m_tmp = nullptr;

	char sz[256] = { 0 };
	snprintf(sz, 256, "Geodesic%d", n++);
	po->SetName(sz);

	GLHighlighter::ClearHighlights();
	m_parent->SetTempObject(nullptr);

	m_in[0]->clear();
	m_in[1]->clear();
	m_input = 0;

	return po;
}

void CCreateGeodesicCurvePane::BuildGeodesic()
{
	GLHighlighter::ClearHighlights();
	m_parent->SetTempObject(nullptr);
	if (m_tmp) delete m_tmp;
	m_tmp = nullptr;

	CModelDocument* doc = dynamic_cast<CModelDocument*>(m_parent->GetDocument());
	if (doc == nullptr) return;

	GModel& mdl = doc->GetProject().GetFSModel().GetModel();

	vec3d r0 = m_r[0];
	vec3d r1 = m_r[1];

	// build the tri-mesh
	int nfaces = 0;
	for (int i = 0; i < mdl.Objects(); ++i)
	{
		GObject* po = mdl.Object(i);
		GLMesh* pm = po->GetRenderMesh();
		if (pm) nfaces += pm->Faces();
	}
	if (nfaces == 0) return;

	FSTriMesh triMesh;
	triMesh.Create(nfaces);
	nfaces = 0;
	for (int i = 0; i < mdl.Objects(); ++i)
	{
		GObject* po = mdl.Object(i);
		GLMesh* pm = po->GetRenderMesh();
		if (pm)
		{
			Transform& T = po->GetTransform();
			for (int j = 0; j < pm->Faces(); ++j)
			{
				FSTriMesh::FACE& face = triMesh.Face(nfaces++);
				GLMesh::FACE& fj = pm->Face(j);
				face.r[0] = T.LocalToGlobal(to_vec3d(pm->Node(fj.n[0]).r));
				face.r[1] = T.LocalToGlobal(to_vec3d(pm->Node(fj.n[1]).r));
				face.r[2] = T.LocalToGlobal(to_vec3d(pm->Node(fj.n[2]).r));

				face.fn = (face.r[1] - face.r[0]) ^ (face.r[2] - face.r[0]);
				face.fn.Normalize();
				face.tag = i;
			}
		}
	}

	// create initial (straight) path
	int STEPS = m_div->value();
	if (STEPS < 1) STEPS = 1;
	std::vector<vec3d> pt;
	pt.push_back(r0);
	for (int i = 1; i < STEPS; ++i)
	{
		double w = (double)i / (double)STEPS;
		vec3d ri = r0 + (r1 - r0) * w;
		pt.push_back(ri);
	}
	pt.push_back(r1);

	// calculate geodesic
	PathOnMesh path = ProjectToGeodesic(triMesh, pt, 100, 1e-6);

	// create the curve mesh
	FSCurveMesh* pc = new FSCurveMesh;
	pc->Create(path.Points(), path.Points() - 1);
	for (int i = 0; i < path.Points(); ++i)
	{
		pc->Node(i).r = path[i].r;
		if (i > 0)
		{
			FSEdge& e = pc->Edge(i - 1);
			e.SetType(FE_EDGE2);
			e.n[0] = i - 1;
			e.n[1] = i;
			e.m_gid = 0;
		}
	}
	pc->Node(0).m_gid = 0;
	pc->Node(path.Points()-1).m_gid = 1;
	pc->UpdateMesh();

	// add points and edges
	m_tmp = new GCurveMeshObject(pc);

	m_parent->SetTempObject(m_tmp);

	// highlight edges
	GLHighlighter::ClearHighlights();
	int N = m_tmp->Edges();
	if (N > 0)
	{
		for (int i = 0; i < N; ++i) GLHighlighter::PickItem(m_tmp->Edge(i));
	}
}
