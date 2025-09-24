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
#include <FSCore/math3d.h>
#include <list>

//-----------------------------------------------------------------------------
// Class that represents a line mesh in 2D composed of linear segments
//
class LineMesh2D
{
private:
	// forward type declarations
	class NODE;
	class EDGE;

public:
	// reference types for accessing mesh items
	typedef std::list<NODE>::iterator Node;
	typedef std::list<EDGE>::iterator Edge;

private:
	// class that represents a node
	class NODE
	{
	public:
		vec2d	pos;
		explicit NODE(const vec2d& r) : pos(r) {}
	};

	// class that represents an edge
	class EDGE
	{
	public:
		Node	node[2];	// the two nodes this edge connects.
		Edge	edge[2];	// two neighbor edges
	};

public:
	// constructor. 
	LineMesh2D();

	// add a point to the mesh
	// returns a Node reference
	Node AddNode(const vec2d& r);

	// add an edge between two nodes
	Edge AddEdge(Node node0, Node node1);

private:
	std::list<NODE>	m_Node;	// list of nodes
	std::list<EDGE>	m_Edge;	// list of edges
};
