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

#include "LineMesh2D.h"

//-----------------------------------------------------------------------------
// Constructs an empty 2D line mesh
LineMesh2D::LineMesh2D()
{

}

//-----------------------------------------------------------------------------
LineMesh2D::Node LineMesh2D::AddNode(const vec2d& r)
{
	m_Node.push_back(NODE(r));
	return --m_Node.end();
}

//-----------------------------------------------------------------------------
LineMesh2D::Edge LineMesh2D::AddEdge(LineMesh2D::Node node0, LineMesh2D::Node node1)
{
	// Make sure this edge does not exist yet
	for (Edge e = m_Edge.begin(); e != m_Edge.end(); ++e)
	{
		if ((e->node[0] == node0) && (e->node[1] == node1)) return e;
		if ((e->node[0] == node1) && (e->node[1] == node0)) return e;
	}

	// if we get here, this edge is not defined yet so let's create a new one
	EDGE edgeData;

	// set the nodes
	edgeData.node[0] = node0;
	edgeData.node[1] = node1;

	// set the edges to invalid values
	edgeData.edge[0] = m_Edge.end();
	edgeData.edge[1] = m_Edge.end();

	// add the edge to the list
	m_Edge.push_back(edgeData);

	// get reference to this edge
	Edge newEdge = --m_Edge.end();

	// Next, we need to find the neighbors of the edges.
	// find an unassigned edge that contains these two nodes
	for (Edge e = m_Edge.begin(); e != m_Edge.end(); ++e)
	{
		// make sure we don't compare the same edges
		if (e != newEdge)
		{
			// check node 0 (of the new edge)
			if (newEdge->edge[0] == m_Edge.end())
			{
				if ((e->node[0] == node0) && (e->edge[0] == m_Edge.end()))
				{
					e->edge[0] = newEdge;
					newEdge->edge[0] = e;	
				}
				else if ((e->node[1] == node0) && (e->edge[1] == m_Edge.end()))
				{
					e->edge[1] = newEdge;
					newEdge->edge[0] = e;
				}
			}

			// check node 1 (of the new edge)
			if (newEdge->edge[1] == m_Edge.end())
			{
				if ((e->node[0] == node1) && (e->edge[0] == m_Edge.end()))
				{
					e->edge[0] = newEdge;
					newEdge->edge[1] = e;
				}
				else if ((e->node[1] == node1) && (e->edge[1] == m_Edge.end()))
				{
					e->edge[1] = newEdge;
					newEdge->edge[1] = e;
				}
			}
		}
	}

	// return the new edge
	return newEdge;
}
