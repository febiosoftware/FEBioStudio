/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2024 University of Utah, The Trustees of Columbia University in
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
#include "FSCurveObjectMesher.h"
#include <GeomLib/GCurveObject.h>
#include <MeshLib/FSMesh.h>

FSCurveObjectMesher::FSCurveObjectMesher() : m_po(nullptr)
{
	AddDoubleParam(0, "element size");
	AddIntParam(0, "element type")->SetEnumNames("linear\0quadratic\0");
}

FSMesh* FSCurveObjectMesher::BuildMesh(GObject* po)
{
	m_po = dynamic_cast<GCurveObject*>(po);
	if (m_po == nullptr) return nullptr;
	if (m_po->Edges() == 0) return nullptr;

	double elemSize = GetFloatValue(0);
	int elemType = GetIntValue(1);

	// calculate mesh size
	int totalElems = 0;
	int totalNodes = m_po->Nodes();
	std::vector<int> elemsPerEdge(m_po->Edges(), 0);
	for (int n = 0; n < m_po->Edges(); ++n)
	{
		GEdge* edge = m_po->Edge(n);

		// get the position of the edge nodes
		vec3d ra = edge->Node(0)->LocalPosition();
		vec3d rb = edge->Node(1)->LocalPosition();

		// get the number of elements, based on element size
		double L = (rb - ra).Length();
		int elems = 1;
		if (elemSize > 0) elems = (int)(L / elemSize + 0.5);
		if (elems < 1) elems = 1;
		elemsPerEdge[n] = elems;

		totalNodes += elems - 1;
		if (elemType == QUADRATIC_ELEM) totalNodes += elems; // quadratic elements
		totalElems += elems;
	}

	// allocate mesh
	FSMesh* mesh = new FSMesh;
	mesh->Create(totalNodes, totalElems, 0, totalElems);
	mesh->TagAllNodes(-1);

	// process geometry nodes
	int nodeIndex = 0;
	for (int i = 0; i < m_po->Nodes(); ++i)
	{
		GNode* gn = m_po->Node(i);
		FSNode& node = mesh->Node(nodeIndex++);
		node.r = gn->LocalPosition();
		gn->SetNodeIndex(i);
		node.m_gid = i;
	}

	// process geometry edges
	int elemIndex = 0;
	for (int n=0; n<m_po->Edges(); ++n)
	{
		GEdge* edge = m_po->Edge(n);

		// get the position of the edge nodes
		vec3d ra = edge->Node(0)->LocalPosition();
		vec3d rb = edge->Node(1)->LocalPosition();

		int na = edge->Node(0)->GetNodeIndex();
		int nb = edge->Node(1)->GetNodeIndex();
		

		int m0 = na, m1 = nb;
		int elems = elemsPerEdge[n];
		for (int i = 0; i < elems; ++i)
		{
			if (i < (elems - 1))
			{
				m1 = nodeIndex;
				vec3d ri = ra + (rb - ra) * ((i + 1.0) / elems);
				FSNode& node = mesh->Node(nodeIndex++);
				node.r = ri;
			}
			else m1 = nb;

			if (elemType == LINEAR_ELEM)
			{
				FSElement& el = mesh->Element(elemIndex++);
				el.SetType(FE_BEAM2);
				el.m_node[0] = m0;
				el.m_node[1] = m1;
				el.m_gid = 0;
			}
			else if (elemType == QUADRATIC_ELEM)
			{
				// add middle node
				int m2 = nodeIndex;
				vec3d ri = ra + (rb - ra) * ((i + 0.5) / elems);
				FSNode& node = mesh->Node(nodeIndex++);
				node.r = ri;

				FSElement& el = mesh->Element(elemIndex++);
				el.SetType(FE_BEAM3);
				int m = totalNodes + i;
				el.m_node[0] = m0;
				el.m_node[1] = m1;
				el.m_node[2] = m2;
				el.m_gid = 0;
			}

			m0 = m1;
		}
	}

	// update edges
	for (int i = 0; i < totalElems; ++i)
	{
		FSElement& el = mesh->Element(i);
		FSEdge& ed = mesh->Edge(i);
		if (el.Type() == FE_BEAM2)
		{
			ed.SetType(FSEdgeType::FE_EDGE2);
			ed.n[0] = el.m_node[0];
			ed.n[1] = el.m_node[1];
			ed.n[2] = ed.n[3] = -1;
			ed.m_gid = el.m_gid;
		}
		else if (el.Type() == FE_BEAM3)
		{
			ed.SetType(FSEdgeType::FE_EDGE3);
			ed.n[0] = el.m_node[0];
			ed.n[1] = el.m_node[1];
			ed.n[2] = el.m_node[2];
			ed.n[3] = -1;
			ed.m_gid = el.m_gid;
		}
	}

	mesh->UpdateEdgeNeighbors();
	mesh->UpdateMesh();

	return mesh;
}
