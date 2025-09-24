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
#include "FSMeshItem.h"
#include <FSCore/math3d.h>

//-----------------------------------------------------------------------------
// Edge types
// (NOTE: Do not change the order of these values)
enum FSEdgeType
{
	FE_EDGE2,
	FE_EDGE3,
	FE_EDGE4,
	FE_EDGE_INVALID
};

//-----------------------------------------------------------------------------
// The FSEdge class stores the edge data.
// An edge can be linear, quadratic or cubic. Note that the first two nodes 
// always represent the outside nodes. 
//
//   1                    2 
//   +--------------------+        linear element
//
//   1         3          2
//   +---------o----------+        quadratic element
//
//   1      3       4     2
//   +------o-------o-----+        cubic element
//
// An edge can have two neighors, one on each end. A neighbor is only set
// if only one other edge connects to that end. If no other edge, or multiple
// edges connect, the neighbor is set to -1.
//
class FSEdge : public FSMeshItem
{
public:
	enum { MAX_NODES = 4 };

public:
	//! constructor
	FSEdge();

	//! constructor
	FSEdge(const FSEdge& e);

	//! assignment operator
	void operator = (const FSEdge& e);

	//! edge comparison
	bool operator == (const FSEdge& e) const;

	//! return number of nodes
	int Nodes() const;

	//! find a node index
	int FindNodeIndex(int node) const;

	//! see if the edge has a node
	bool HasNode(int node) const;

	//! Get the edge type
	int Type() const { return m_type; }

	//! set the type
	void SetType(FSEdgeType type);

	// evaluate shape function at iso-parameteric point (r,s)
	void shape(double* H, double r);

	// evaluate a vector expression at iso-points (r,s)
	double eval(double* d, double r);

	// evaluate a vector expression at iso-points (r,s)
	vec3f eval(vec3f* v, double r);
	vec3d eval(vec3d* v, double r);

public:
	int	m_type;				//!< edge type
	int n[MAX_NODES];		//!< edge nodes
	int	m_elem;				//!< the element to which this edge belongs (used only by beams)
	int	m_nbr[2];			//!< the two adjacent edges (only defined for edges with gid >= 0, and if there are more edges incident to a node, the neighbour is set to -1)
	int	m_face[2];			//!< the two faces adjacent to this edge (TODO: I think I should delete this since I cannot assume that each edge is shared by only one or two faces)
};
