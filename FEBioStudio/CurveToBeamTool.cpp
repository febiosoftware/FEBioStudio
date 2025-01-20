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
#include "CurveToBeamTool.h"
#include "ModelDocument.h"
#include <GeomLib/GMeshObject.h>
#include <MeshLib/FSCurveMesh.h>
#include <GeomLib/GCurveMeshObject.h>
#include <QMessageBox>
#include "MainWindow.h"

CCurveToBeamTool::CCurveToBeamTool(CMainWindow* wnd) : CBasicTool(wnd, "Curve To Beam", HAS_APPLY_BUTTON)
{

}

bool CCurveToBeamTool::OnApply()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return false;

	GObject* po = GetActiveObject();
	if (po == nullptr) return false;

	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) return false;

	GEdgeSelection* sel = dynamic_cast<GEdgeSelection*>(doc->GetCurrentSelection());
	if ((sel == nullptr) || (sel->Count() == 0)) return false;

	pm->TagAllNodes(-1);
	int NC = pm->Edges();
	int nc = 0;
	for (int i = 0; i < NC; ++i)
	{
		FSEdge& edge = pm->Edge(i);
		edge.m_ntag = -1;
		int gid = edge.m_gid;
		if (gid >= 0)
		{
			GEdge* pe = po->Edge(gid);
			if (pe->IsSelected())
			{
				edge.m_ntag = nc++;
				int ne = edge.Nodes();
				for (int j = 0; j < ne; ++j) pm->Node(edge.n[j]).m_ntag = 1;
			}
		}
	}

	int nn = 0;
	for (int i = 0; i < pm->Nodes(); ++i)
	{
		FSNode& node = pm->Node(i);
		if (node.m_ntag != -1) node.m_ntag = nn++;
	}

	FSMesh* curveMesh = new FSMesh;
	curveMesh->Create(nn, nc);
	nn = 0;
	for (int i = 0; i < pm->Nodes(); ++i)
	{
		FSNode& snode = pm->Node(i);
		if (snode.m_ntag >= 0)
		{
			FSNode& dnode = curveMesh->Node(nn++);
			dnode.r = snode.r;
		}
	}

	nc = 0;
	for (int i = 0; i < NC; ++i)
	{
		FSEdge& sedge = pm->Edge(i);
		if (sedge.m_ntag >= 0)
		{
			FSElement& el = curveMesh->Element(nc++);
			if (sedge.Type() == FE_EDGE2) el.SetType(FE_BEAM2);
			else if (sedge.Type() == FE_EDGE3) el.SetType(FE_BEAM3);
			else assert(false);


			int ne = sedge.Nodes();
			for (int j = 0; j < ne; ++j)
			{
				el.m_node[j] = pm->Node(sedge.n[j]).m_ntag;
				assert((el.m_node[j] >= 0) && (el.m_node[j] < nn));
			}
		}
	}
	curveMesh->RebuildMesh();

	static int n = 1;
	QString name = QString("ExtractedCurve%1").arg(n++);
	GMeshObject* pco = new GMeshObject(curveMesh);
	pco->SetName(name.toStdString());
	pco->CopyTransform(po);

	GModel* gm = doc->GetGModel();
	gm->AddObject(pco);

	QMessageBox::information(GetMainWindow(), "Curve To Beam", QString("Successfully created %1").arg(name));

	return true;
}
