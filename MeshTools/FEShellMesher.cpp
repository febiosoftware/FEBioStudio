#include "stdafx.h"
#include "FEShellMesher.h"
#include <GeomLib/GSurfaceMeshObject.h>

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
FEMesh*	FEShellMesher::BuildMesh()
{
	GSurfaceMeshObject* po = dynamic_cast<GSurfaceMeshObject*>(m_po);
	if (po == nullptr) return nullptr;

	FESurfaceMesh* surfaceMesh = po->GetSurfaceMesh();
	if (surfaceMesh == nullptr) return nullptr;

	int NF = surfaceMesh->Faces();
	int NN = surfaceMesh->Nodes();
	int NC = surfaceMesh->Edges();

	double h0 = GetFloatValue(0);

	// allocate mesh
	FEMesh* mesh = new FEMesh;
	mesh->Create(NN, NF, NF, NC);

	// create nodes
	for (int i = 0; i < NN; ++i)
	{
		FENode& sn = surfaceMesh->Node(i);
		FENode& dn = mesh->Node(i);
		dn = sn;
	}

	// create edges
	for (int i = 0; i < NC; ++i)
	{
		FEEdge& se = surfaceMesh->Edge(i);
		FEEdge& de = mesh->Edge(i);
		de = se;
	}

	// create faces
	for (int i = 0; i < NF; ++i)
	{
		FEFace& sf = surfaceMesh->Face(i);
		FEFace& df = mesh->Face(i);
		df = sf;
		df.m_elem[0].eid = i;
		df.m_elem[0].lid = 0;
		df.m_elem[1].eid = -1;
	}

	// create elements
	for (int i = 0; i < NF; ++i)
	{
		FEFace& sf = surfaceMesh->Face(i);
		FEElement& el = mesh->Element(i);

		el.m_gid = 0;
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

	mesh->Update();

	return mesh;
}
