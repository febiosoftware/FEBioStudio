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

#pragma once
#include <vector>

class FSLineMesh;
class FSEdge;

struct NodeEdgeRef
{
	int eid; //edge id
	int nid; // local edge index of node
	FSEdge* pe; // pointer to edge
};

class FSNodeEdgeList
{
public:
	FSNodeEdgeList(FSLineMesh* mesh = nullptr);

	void Build(FSLineMesh* mesh, bool segsOnly = false);

	void Clear();

	bool IsEmpty() const;

	// Return the number of edges for a given node
	int Edges(int node) const { return (int) m_edge[node].size(); }

	// Return the edge for a given node
	const FSEdge* Edge(int node, int edge) const;

	// return the edge index
	int EdgeIndex(int node, int edge) const;

	const std::vector<NodeEdgeRef>& EdgeList(int node) const;

private:
	FSLineMesh*			m_mesh;
	std::vector< std::vector<NodeEdgeRef> >	m_edge;		// edge list
};
