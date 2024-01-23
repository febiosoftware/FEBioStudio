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
#include <GeomLib/GCurveObject.h>

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
	l->setContentsMargins(0,0,0,0);
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
	if (m_tmp == 0)
	{
		m_tmp = new GObject(GCURVE);
		m_tmp->AddPart();
	}
	m_parent->SetTempObject(m_tmp);

	// Add the new point
	// this will check if the point already coincides with a node
	// of the curve, in which case it will return the (local) node ID. 
	int newNode = m_tmp->AddNode(r)->GetLocalID();

	// add a new edge connecting the last added node to this new node.
	bool addEdge = false;
	if ((m_lastNode != -1) && (newNode != m_lastNode))
	{
		int edges = m_tmp->Edges();
		m_tmp->AddLine(m_lastNode, newNode);
		if (m_tmp->Edges() > edges)
		{
			addEdge = true;
		}
	}

	// update the last added node
	m_lastNode = newNode;

	// update the Object
	m_tmp->Update();


	// highlight edges
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
		m_tmp->Clear();
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
		GLHighlighter::ClearHighlights();

		// remove the last point
		// this will also remove any edges that attach to this node
		m_tmp->RemoveNode(m_tmp->Nodes() - 1);
		m_lastNode = m_tmp->Nodes() - 1;

		// update highlighter
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

	// make sure we have at least one edge
	int NE = m_tmp->Edges();
	if (NE == 0) return 0;

	// see if the curve is closed
	// TODO: this will need some better heuristics
	bool isClosed = false;
	if (NE > 1)
	{
		if (m_tmp->Edge(0)->m_node[0] == m_tmp->Edge(NE - 1)->m_node[1])
		{
			isClosed = true;
		}
	}

	static int n = 1;

	FSObject* newObject = 0;
	if (IsCurveCapped() && isClosed)
	{
		// create new object
		GObject2D* po = new GObject2D;

		// add nodes
		for (int i = 0; i<m_tmp->Nodes(); ++i) po->AddNode(m_tmp->Node(i)->LocalPosition());

		// add edges
		for (int i = 0; i<m_tmp->Nodes(); ++i)
		{
			GEdge* edge = new GEdge(po);
			edge->m_node[0] = i;
			edge->m_node[1] = (i + 1) % m_tmp->Nodes();
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
		// create new object
		GCurveObject* po = new GCurveObject;

		// add nodes
		for (int i = 0; i < m_tmp->Nodes(); ++i) po->AddNode(m_tmp->Node(i)->LocalPosition());

		// add edges
		for (int i = 0; i < m_tmp->Edges(); ++i)
		{
			GEdge* edge = new GEdge(po);
			edge->m_node[0] = m_tmp->Edge(i)->m_node[0];
			edge->m_node[1] = m_tmp->Edge(i)->m_node[1];
			edge->m_cnode = m_tmp->Edge(i)->m_cnode;
			edge->m_ntype = m_tmp->Edge(i)->m_ntype;
			edge->m_orient = m_tmp->Edge(i)->m_orient;
			po->AddEdge(edge);
		}

		po->AddBeamPart();
		po->Update();

		newObject = po;

		delete m_tmp;
		m_tmp = 0;
		m_lastNode = -1;
	}

	if (newObject)
	{
		char sz[256] = { 0 };
		snprintf(sz, 256, "PointCurve%d", n++);
		newObject->SetName(sz);
	}

	GLHighlighter::ClearHighlights();
	m_parent->SetTempObject(0);

	return newObject;
}
