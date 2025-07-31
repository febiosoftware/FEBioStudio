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
#include "FEModifier.h"
#include <GeomLib/GObject.h>
#include <GeomLib/GGroup.h>


FEExtrudeToSurface::FEExtrudeToSurface() : FEModifier("Project Nodes")
{
	AddIntParam(1, "divisions");
}

FSMesh* FEExtrudeToSurface::Apply(GObject* po, FESelection* pg)
{
	if ((pg == nullptr) || (po == nullptr)) return nullptr;

	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) return nullptr;

	GFaceSelection* pfs = dynamic_cast<GFaceSelection*>(pg);
	if ((pfs == nullptr) || (pfs->Size() == 0)) return nullptr;

	// make sure all faces belong to the same object
	GObject* pgObj = dynamic_cast<GObject*>(pfs->Face(0)->Object());
	for (int i= 1; i < pfs->Size(); ++i)
	{
		GFace* pf = pfs->Face(i);
		if (pf == nullptr) return nullptr;
		if (pf->Object() != pgObj) return nullptr;
	}

	FSMesh* pgMesh = pgObj->GetFEMesh();
	if (pgMesh == nullptr) return nullptr;
	pgMesh->TagAllNodes(0);
	std::vector<int> FLT(pgObj->Faces(), 0);
	for (int i = 0; i < pfs->Size(); ++i)
	{
		GFace* pf = pfs->Face(i);
		FLT[pf->GetLocalID()] = 1;
	}

	for (int i = 0; i < pgMesh->Faces(); ++i)
	{
		FSFace& f = pgMesh->Face(i);
		if (FLT[f.m_gid] == 1)
		{
			for (int j = 0; j < f.Nodes(); ++j)
			{
				int nid = f.n[j];
				FSNode& node = pgMesh->Node(nid);
				node.m_ntag = 1; // tag the node
			}
		}
	}

	Transform& T1 = pgObj->GetTransform();
	Transform& T2 = po->GetTransform();
	std::vector<vec3d> trgPoints;
	for (int i = 0; i < pgMesh->Nodes(); ++i)
	{
		FSNode& node = pgMesh->Node(i);
		if (node.m_ntag == 1)
		{
			vec3d r = T1.LocalToGlobal(node.r);
			r = T2.GlobalToLocal(r);
			trgPoints.push_back(r);
		}
	}

	int ndiv = GetIntValue(0);
	if (ndiv < 1) ndiv = 1;

	int NN = pm->Nodes();
	int NF = pm->Faces();
	if (trgPoints.size() != NN) return nullptr;

	std::vector<vec3d> srcPoints;
	for (int i = 0; i < NN; ++i)
	{
		FSNode& node = pm->Node(i);
		srcPoints.push_back(node.r);
	}

	FSMesh* newMesh = new FSMesh;
	newMesh->Create(NN * (ndiv + 1), NF * ndiv);
	for (int l = 0; l<= ndiv; ++l)
	{
		double s = double(l) / double(ndiv);
		for (int i = 0; i < NN; ++i)
		{
			vec3d r = srcPoints[i] * (1 - s) + trgPoints[i] * s;
			newMesh->Node(i + l * NN).pos(r);
		}
	}

	for (int l = 0; l < ndiv; ++l)
	{
		for (int i = 0; i < NF; ++i)
		{
			FSElement& el = newMesh->Element(l * NF + i);
			el.SetType(FE_HEX8);
			el.m_gid = 0;

			FSFace& f = pm->Face(i);
			el.m_node[0] = l * NN + f.n[0];
			el.m_node[1] = l * NN + f.n[1];
			el.m_node[2] = l * NN + f.n[2];
			el.m_node[3] = l * NN + f.n[3];

			el.m_node[4] = (l+1) * NN + f.n[0];
			el.m_node[5] = (l+1) * NN + f.n[1];
			el.m_node[6] = (l+1) * NN + f.n[2];
			el.m_node[7] = (l+1) * NN + f.n[3];
		}
	}

	newMesh->RebuildMesh(30.0);

	return newMesh;
}
