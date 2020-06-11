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
#include "CreateP2PlinePane.h"
#include <QLabel>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QValidator>
#include <GeomLib/GCurveMeshObject.h>
#include <MeshLib/FECurveMesh.h>
#include "GLHighlighter.h"
#include <MeshTools/GObject2D.h>

//=============================================================================
CCreateP2PLinePane::CCreateP2PLinePane(CCreatePanel* parent) : CCreatePane(parent)
{
	m_tmp = 0;
	m_lastNode = -1;

	QLabel* pl;
	QGridLayout* grid = new QGridLayout;
	QPushButton* b;
	QLineEdit* e;
	grid->addWidget(pl = new QLabel("X"), 0, 0); grid->addWidget(e = m_in[0] = new QLineEdit, 0, 1); pl->setBuddy(e); e->setValidator(new QDoubleValidator); e->setObjectName("x");
	grid->addWidget(pl = new QLabel("Y"), 1, 0); grid->addWidget(e = m_in[1] = new QLineEdit, 1, 1); pl->setBuddy(e); e->setValidator(new QDoubleValidator); e->setObjectName("y");
	grid->addWidget(pl = new QLabel("Z"), 2, 0); grid->addWidget(e = m_in[2] = new QLineEdit, 2, 1); pl->setBuddy(e); e->setValidator(new QDoubleValidator); e->setObjectName("z");
	grid->addWidget(b = new QPushButton("Add Node"), 3, 1); b->setObjectName("getNode");
	grid->addWidget(m_newCurve = new QPushButton("New curve segment")); m_newCurve->setObjectName("newCurve");
	grid->addWidget(m_cap = new QCheckBox("Create Surface"));

	m_cap->setChecked(false);

	QVBoxLayout* l = new QVBoxLayout;
	l->setMargin(0);
	l->addLayout(grid);
	l->addStretch();

	setLayout(l);

	QMetaObject::connectSlotsByName(this);
}

void CCreateP2PLinePane::hideEvent(QHideEvent* ev)
{
	if (m_tmp)
	{
		GLHighlighter::ClearHighlights();
		m_parent->SetTempObject(0);
		delete m_tmp;
		m_tmp = 0;
		m_lastNode = -1;
	}

	m_in[0]->setText("");
	m_in[1]->setText("");
	m_in[2]->setText("");
}

void CCreateP2PLinePane::on_newCurve_clicked()
{
	GLHighlighter::PickActiveItem();
	m_lastNode = -1;
}

void CCreateP2PLinePane::on_getNode_clicked()
{
	double x = m_in[0]->text().toDouble();
	double y = m_in[1]->text().toDouble();
	double z = m_in[2]->text().toDouble();

	AddPoint(vec3d(x, y, z));
}

void CCreateP2PLinePane::setInput(const vec3d& r)
{
	m_in[0]->setText(QString::number(r.x));
	m_in[1]->setText(QString::number(r.y));
	m_in[2]->setText(QString::number(r.z));

	AddPoint(r);
}

void CCreateP2PLinePane::AddPoint(const vec3d& r)
{
	if (m_tmp == 0) m_tmp = new GCurveMeshObject(new FECurveMesh);
	m_parent->SetTempObject(m_tmp);

	FECurveMesh* curveMesh = m_tmp->GetCurveMesh();

	int nodes = curveMesh->Nodes();
	int newNode = curveMesh->AddNode(r);
	if (newNode == nodes)
	{
		curveMesh->Node(newNode).m_gid = nodes;
	}

	bool addEdge = false;
	if ((m_lastNode != -1) && (newNode != m_lastNode))
	{
		int edges = curveMesh->Edges();
		int newEdge = curveMesh->AddEdge(m_lastNode, newNode);
		if (newEdge == edges)
		{
			curveMesh->Edge(newEdge).m_gid = edges;
			addEdge = true;
		}
	}
	m_lastNode = newNode;

	// update the GMeshObject
	m_tmp->Update();

	GLHighlighter::ClearHighlights();
	int N = m_tmp->Edges();
	if (N > 0)
	{
		for (int i=0; i<N-1; ++i)
			GLHighlighter::PickItem(m_tmp->Edge(i));
		if (addEdge) 
			GLHighlighter::SetActiveItem(m_tmp->Edge(m_tmp->Edges() - 1));
		else
		{
			GLHighlighter::SetActiveItem(0);
			GLHighlighter::PickItem(m_tmp->Edge(m_tmp->Edges() - 1));
		}
	}
}

bool CCreateP2PLinePane::IsCurveCapped() const
{
	return m_cap->isChecked();
}

void CCreateP2PLinePane::Activate()
{
}

void CCreateP2PLinePane::Deactivate()
{
	m_parent->SetTempObject(0);
	GLHighlighter::ClearHighlights();
	delete m_tmp;
	m_tmp = 0;
	m_lastNode = -1;
}

bool CCreateP2PLinePane::Clear()
{
	if (m_tmp)
	{
		FECurveMesh* cm = m_tmp->GetCurveMesh();
		if (cm == 0) return false;

		if ((cm->Nodes() == 0) && (cm->Edges() == 0)) return false;

		cm->Clear();
		m_lastNode = -1;
		m_tmp->Update();
		GLHighlighter::ClearHighlights();

		return true;
	}
	else return false;
}

bool CCreateP2PLinePane::Undo()
{
	if (m_tmp)
	{
		FECurveMesh* cm = m_tmp->GetCurveMesh();
		if (cm == 0) return false;

		if ((cm->Nodes() == 0) && (cm->Edges() == 0)) return false;

		// remove the last point
		cm->RemoveNode(cm->Nodes() - 1);
		m_lastNode = cm->Nodes() - 1;

		// update the object
		m_tmp->Update();

		GLHighlighter::ClearHighlights();
		int N = m_tmp->Edges();
		if (N > 0)
		{
			for (int i = 0; i<N - 1; ++i) GLHighlighter::PickItem(m_tmp->Edge(i));
			GLHighlighter::SetActiveItem(m_tmp->Edge(N - 1));
		}

		return true;
	}
	else return false;
}

FSObject* CCreateP2PLinePane::Create()
{
	// make sure we have a temp object
	if (m_tmp == 0) return 0;

	// make sure we have a mesh
	FECurveMesh* cm = m_tmp->GetCurveMesh();
	if (cm == 0) return 0;

	// make sure we have at least one edge
	if (cm->Edges() == 0) return 0;

	static int n = 1;

	FSObject* newObject = 0;
	if (IsCurveCapped() && (cm->Type() == FECurveMesh::CLOSED_CURVE))
	{
		// create new object
		GObject2D* po = new GObject2D;

		// add nodes
		for (int i = 0; i<cm->Nodes(); ++i) po->AddNode(cm->Node(i).r);

		// add edges
		for (int i = 0; i<cm->Nodes(); ++i)
		{
			GEdge* edge = new GEdge(po);
			edge->m_node[0] = i;
			edge->m_node[1] = (i + 1) % cm->Nodes();
			edge->m_ntype = EDGE_LINE;
			po->AddEdge(edge);
		}

		// create a surface
		int N = po->Nodes();
		GFace* face = new GFace(po);
		face->m_ntype = FACE_POLYGON;
		face->m_node.resize(N);
		for (int i = 0; i<N; ++i) face->m_node[i] = i;
		face->m_edge.resize(N);
		for (int i = 0; i<N; ++i)
		{
			face->m_edge[i].nid = i;
			face->m_edge[i].nwn = 1;
		}
		face->m_nPID[0] = 0;
		po->AddFace(face);
		po->AddPart();
		po->Update();

		newObject = po;

		delete m_tmp;
		m_tmp = 0;
		m_lastNode = -1;
	}
	else
	{
		m_tmp->GetCurveMesh()->BuildMesh();
		m_tmp->Update();

		newObject = m_tmp;
		
		m_tmp = 0;
		m_lastNode = -1;
	}

	if (newObject)
	{
		char sz[256] = { 0 };
		sprintf(sz, "PointCurve%d", n++);
		newObject->SetName(sz);
	}

	GLHighlighter::SetActiveItem(0);
	GLHighlighter::ClearHighlights();
	m_parent->SetTempObject(0);

	return newObject;
}
