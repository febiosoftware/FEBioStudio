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
#include "DiscreteElementNetworkTool.h"
#include <QWidget>
#include <QPushButton>
#include <QBoxLayout>
#include <QMessageBox>
#include <MeshLib/FEMesh.h>
#include <GeomLib/GModel.h>
#include <GeomLib/GMeshObject.h>
#include <FEMLib/FEDiscreteMaterial.h>
#include <MeshLib/FENodeNodeList.h>
#include "MainWindow.h"
#include "ModelDocument.h"
#include "Commands.h"

class DENToolUI : public QWidget
{
public:
	DENToolUI(CAbstractTool* w)
	{
		QPushButton* apply = new QPushButton("Apply");
		QVBoxLayout* l = new QVBoxLayout;
		l->addWidget(apply);
		l->addStretch();
		setLayout(l);

		QObject::connect(apply, SIGNAL(clicked()), w, SLOT(OnApply()));
	}
};

CDiscreteElementNetworkTool::CDiscreteElementNetworkTool(CMainWindow* wnd) : CAbstractTool(wnd, "Discrete Element Network")
{
	ui = nullptr;
}

QWidget* CDiscreteElementNetworkTool::createUi()
{
	if (ui == nullptr) ui = new DENToolUI(this);
	return ui;
}

void CDiscreteElementNetworkTool::Activate()
{

}

void CDiscreteElementNetworkTool::Deactivate()
{

}

void CDiscreteElementNetworkTool::OnApply()
{
	GObject* po = GetActiveObject();
	if (po == nullptr)
	{
		QMessageBox::critical(GetMainWindow(), "Discrete Element Network tool", "You need to select an object first.");
		return;
	}

	CModelDocument* doc = GetMainWindow()->GetModelDocument();
	if (doc == nullptr) return;

	FSModel* fem = doc->GetFSModel();
	if (fem == nullptr) return;

	GModel& gm = fem->GetModel();

	FSLinearSpringMaterial* mat = new FSLinearSpringMaterial(fem);
	GDiscreteSpringSet* pg = new GDiscreteSpringSet(&gm);
	pg->SetMaterial(mat);
	pg->SetName(po->GetName());

	GMeshObject* pmo = nullptr;

	if (po->GetFEMesh() == nullptr)
	{
		int nodes = po->Nodes();
		int edges = po->Edges();
		if ((nodes == 0) || (edges == 0))
		{
			QMessageBox::critical(GetMainWindow(), "Discrete Element Network tool", "The object is not valid.");
			return;
		}

		FSMesh* pm = new FSMesh;
		pm->Create(nodes, 0);

		for (int i = 0; i < nodes; ++i)
		{
			FSNode& node = pm->Node(i);
			node.r = po->Node(i)->LocalPosition();
			node.m_gid = po->Node(i)->GetLocalID();
		}

		pmo = new GMeshObject(pm);

		string s = po->GetName();
		s += "_nodes";
		pmo->SetName(s);

		for (int i = 0; i < edges; ++i)
		{
			GEdge* edge = po->Edge(i);
			GNode* pn0 = pmo->Node(edge->m_node[0]);
			GNode* pn1 = pmo->Node(edge->m_node[1]);

			pg->AddElement(pn0, pn1);
		}
		pmo->CopyTransform(po);

		CCmdGroup* cmd = new CCmdGroup("Create Discrete Element Network");
		cmd->AddCommand(new CCmdAddDiscreteObject(&gm, pg));
		cmd->AddCommand(new CCmdSwapObjects(&gm, po, pmo));

		doc->DoCommand(cmd);

		GetMainWindow()->UpdateModel(pg);
	}
	else
	{
		FSMesh* pms = po->GetFEMesh();
		int nodes = pms->Nodes();
		FSNodeNodeList NNL(pms);
		for (int i = 0; i < nodes; ++i) pms->Node(i).m_ntag = i;

		int edges = 0;
		for (int i = 0; i < nodes; ++i)
		{
			FSNode& nodei = pms->Node(i);
			int nn = NNL.Valence(i);
			for (int j = 0; j < nn; ++j)
			{
				int nj = NNL.Node(i, j);
				FSNode& nodej = pms->Node(nj);
				if (nodei.m_ntag < nodej.m_ntag) edges++;
			}
		}
		
		FSMesh* pm = new FSMesh;
		pm->Create(nodes, 0);

		for (int i = 0; i < nodes; ++i)
		{
			FSNode& node = pm->Node(i);
			node.r = pms->Node(i).r;
			node.m_gid = i;
		}


		pmo = new GMeshObject(pm);

		string s = po->GetName();
		s += "_nodes";
		pmo->SetName(s);

		edges = 0;
		for (int i = 0; i < nodes; ++i)
		{
			FSNode& nodei = pms->Node(i);
			int nn = NNL.Valence(i);
			for (int j = 0; j < nn; ++j)
			{
				int nj = NNL.Node(i, j);
				FSNode& nodej = pms->Node(nj);
				if (nodei.m_ntag < nodej.m_ntag)
				{
					GNode* pn0 = pmo->Node(nodei.m_ntag);
					GNode* pn1 = pmo->Node(nodej.m_ntag);

					pg->AddElement(pn0, pn1);

					edges++;
				}
			}
		}

		// copy node sets
		for (int i = 0; i < po->FENodeSets(); ++i)
		{
			FSNodeSet* pns = po->GetFENodeSet(i);

			FSNodeSet* pnewSet = new FSNodeSet(pm);
			pnewSet->SetName(pns->GetName());
			pnewSet->add(pns->CopyItems());
			pm->AddFENodeSet(pnewSet);
		}
	}

	pmo->CopyTransform(po);
	CCmdGroup* cmd = new CCmdGroup("Create Discrete Element Network");
	cmd->AddCommand(new CCmdAddDiscreteObject(&gm, pg));
	cmd->AddCommand(new CCmdSwapObjects(&gm, po, pmo));
	doc->DoCommand(cmd);
	GetMainWindow()->UpdateModel(pg);
}
