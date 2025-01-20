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
#include "FEShellMesher.h"
#include <GeomLib/GSurfaceMeshObject.h>
#include <MeshLib/FSMesh.h>
#include <FSCore/ClassDescriptor.h>

REGISTER_CLASS3(FEShellMesher, CLASS_MESHER, Shell_Mesher, "shell_mesher", 0, 0);

FEShellMesher::FEShellMesher()
{
	m_po = nullptr;
}

FEShellMesher::FEShellMesher(GObject* po)
{
	AddDoubleParam(0.01, "shell thickness");
	m_po = po;
}

// build the mesh
FSMesh*	FEShellMesher::BuildMesh()
{
	GSurfaceMeshObject* po = dynamic_cast<GSurfaceMeshObject*>(m_po);
	if (po == nullptr) return nullptr;

	FSSurfaceMesh* surfaceMesh = po->GetSurfaceMesh();
	if (surfaceMesh == nullptr) return nullptr;

	int NF = surfaceMesh->Faces();
	int NN = surfaceMesh->Nodes();
	int NC = surfaceMesh->Edges();

	double h0 = GetFloatValue(0);

	// allocate mesh
	FSMesh* mesh = new FSMesh;
	mesh->Create(NN, NF, NF, NC);

	// create nodes
	for (int i = 0; i < NN; ++i)
	{
		FSNode& sn = surfaceMesh->Node(i);
		FSNode& dn = mesh->Node(i);
		dn = sn;
	}

	// create edges
	for (int i = 0; i < NC; ++i)
	{
		FSEdge& se = surfaceMesh->Edge(i);
		FSEdge& de = mesh->Edge(i);
		de = se;
	}

	// create faces
	for (int i = 0; i < NF; ++i)
	{
		FSFace& sf = surfaceMesh->Face(i);
		FSFace& df = mesh->Face(i);
		df = sf;
		df.m_elem[0].eid = i;
		df.m_elem[0].lid = 0;
		df.m_elem[1].eid = -1;
		df.m_elem[2].eid = -1;
	}

	// create elements
	for (int i = 0; i < NF; ++i)
	{
		FSFace& sf = surfaceMesh->Face(i);
		FSElement& el = mesh->Element(i);

		assert(sf.m_gid >= 0);
		GFace* pf = po->Face(sf.m_gid);
		assert(pf->m_nPID[0] >= 0);
		int pid = po->Part(pf->m_nPID[0])->GetLocalID();

		el.m_gid = pid;
		switch (sf.Type())
		{
		case FE_FACE_TRI3: el.SetType(FE_TRI3); break;
		case FE_FACE_TRI6: el.SetType(FE_TRI6); break;
		case FE_FACE_TRI7: el.SetType(FE_TRI7); break;
		case FE_FACE_TRI10: el.SetType(FE_TRI10); break;
		case FE_FACE_QUAD4: el.SetType(FE_QUAD4); break;
		case FE_FACE_QUAD8: el.SetType(FE_QUAD8); break;
		case FE_FACE_QUAD9: el.SetType(FE_QUAD9); break;
		}

		int nn = sf.Nodes();
		for (int j = 0; j < nn; ++j) el.m_node[j] = sf.n[j];

		for (int j = 0; j < nn; ++j) el.m_h[j] = h0;
	}

	mesh->BuildMesh();

	return mesh;
}
