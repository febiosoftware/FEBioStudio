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
