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

#include "FENodeEdgeList.h"
#include "FELineMesh.h"
#include <assert.h>
using namespace std;

FENodeEdgeList::FENodeEdgeList(FELineMesh* mesh) : m_mesh(mesh)
{
	if (mesh) Build(mesh);
}

void FENodeEdgeList::Clear()
{
	m_edge.clear();
}

bool FENodeEdgeList::IsEmpty() const
{
	return m_edge.empty();
}

void FENodeEdgeList::Build(FELineMesh* pmesh, bool segsOnly)
{
	m_mesh = pmesh;
	assert(pmesh);
	FELineMesh& mesh = *m_mesh;

	// allocate valence array
	int N = mesh.Nodes();
	if (N == 0) return;
	m_edge.resize(N);

	// fill edge array
	int NE = mesh.Edges();
	for (int i=0; i<NE; ++i)
	{
		const FEEdge& edge = mesh.Edge(i);
		if ((segsOnly == false) || (edge.IsExterior()))
		{
			int n0 = edge.n[0];
			int n1 = edge.n[1];

			vector<int>& l0 = m_edge[n0];
			vector<int>& l1 = m_edge[n1];

			l0.push_back(i);
			l1.push_back(i);
		}
	}
}

// Return the edge for a given node
const FEEdge* FENodeEdgeList::Edge(int node, int edge) const
{
	return m_mesh->EdgePtr(m_edge[node][edge]);
}

int FENodeEdgeList::EdgeIndex(int node, int edge) const 
{ 
	return m_edge[node][edge]; 
}

const std::vector<int>& FENodeEdgeList::EdgeIndexList(int node) const
{
	return m_edge[node];
}
