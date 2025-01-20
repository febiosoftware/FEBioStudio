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

#include "FSCurveMesh.h"
#include <stack>
using namespace std;

//-----------------------------------------------------------------------------
// Creates an empty curve mesh
FSCurveMesh::FSCurveMesh() : m_type(FSCurveMesh::EMPTY_CURVE)
{
}

//-----------------------------------------------------------------------------
// Allocates storage for mesh (no initialization of nodes or edges is done)
void FSCurveMesh::Create(int nodes, int edges)
{
	if (nodes > 0) m_Node.resize(nodes);
	if (edges > 0) m_Edge.resize(edges);
}

//-----------------------------------------------------------------------------
FSCurveMesh::CurveType FSCurveMesh::Type() const { return m_type; }

//-----------------------------------------------------------------------------
void FSCurveMesh::Clear()
{
	m_Node.clear();
	m_Edge.clear();
	m_type = FSCurveMesh::EMPTY_CURVE;
}

//-----------------------------------------------------------------------------
// calculte the bounding box of the mesh
BOX FSCurveMesh::BoundingBox() const
{
	if (m_Node.empty()) return BOX();
	BOX b(m_Node[0].r, m_Node[0].r);
	for (size_t i=1; i<m_Node.size(); ++i) b += m_Node[i].r;
	return b;
}

//-----------------------------------------------------------------------------
int FSCurveMesh::AddNode(const vec3d& r, bool bsnap, double snapTolerance)
{
	if (bsnap && (m_Node.empty() == false))
	{
		double R2 = snapTolerance*snapTolerance;

		// see if this node already exists
		for (size_t i=0; i<m_Node.size(); ++i)
		{
			vec3d ri = m_Node[i].r;
			if ((ri - r).SqrLength() <= R2)
			{
				// node already exists
				return (int) i;
			}
		}
	}

	// node does not exist or needs to be added regardless
	FSNode nd;
	nd.r = r;
	m_Node.push_back(nd);

	// this changes the type of the mesh
	m_type = COMPLEX_CURVE;

	return (int) m_Node.size() - 1;
}

//-----------------------------------------------------------------------------
int FSCurveMesh::AddEdge(int n0, int n1)
{
	// make sure the nodes are different
	if (n0 == n1) return -1;

	// see if the edge already exists
	if (m_Edge.empty() == false)
	{
		for (size_t i=0; i<m_Edge.size(); ++i)
		{
			FSEdge& edge = m_Edge[i];
			if (((edge.n[0] == n0) && (edge.n[1] == n1)) ||
				((edge.n[0] == n0) && (edge.n[1] == n1))) return (int) i;
		}	
	}
	
	// edge does not exist yet so let's add it
	FSEdge edge;
	edge.SetType(FE_EDGE2);
	edge.m_gid = -1;
	edge.n[0] = n0;
	edge.n[1] = n1;
	edge.m_nbr[0] = -1;
	edge.m_nbr[1] = -1;
	m_Edge.push_back(edge);

	// the type needs to be reevaluated so set it to invalid for now
	m_type = INVALID_CURVE;

	return (int) m_Edge.size() - 1;
}

//-----------------------------------------------------------------------------
// This currently updates all edge neighbors, edge IDs, and node IDs
// Edge IDs are based on connectivity. Node IDs are assigned to all nodes
// that define end points of edges
void FSCurveMesh::BuildMesh()
{
	// first assign node IDs
	// This is used by the update neighbors and therefore must be done first
	UpdateNodeIDs();

	// update all edge neighbors
	UpdateEdgeNeighbors();

	// assign IDs to edges
	UpdateEdgeIDs();

	// finally, identify the type of the curve
	UpdateType();
}

//-----------------------------------------------------------------------------
void FSCurveMesh::UpdateEdgeNeighbors()
{
	int NN = Nodes();
	int NE = Edges();

	// clear all edge neighbors
	for (int i = 0; i<NE; ++i)
	{
		FSEdge& ei = Edge(i);
		ei.m_nbr[0] = ei.m_nbr[1] = -1;
	}

	// connect all the edges
	for (int i = 0; i<NE; ++i)
	{
		FSEdge& ei = Edge(i);
		int n0 = ei.n[0];
		int n1 = ei.n[1];

		int nodeID0 = m_Node[n0].m_gid;
		int nodeID1 = m_Node[n1].m_gid;

		if ((nodeID0 == -1) || (nodeID1 == -1))
		{
			for (int j = i + 1; j<NE; ++j)
			{
				FSEdge& ej = Edge(j);
				int m0 = ej.n[0];
				int m1 = ej.n[1];

				if (nodeID0 == -1)
				{
					if (n0 == m0)
					{
						ei.m_nbr[0] = j;
						ej.m_nbr[0] = i;
					}
					else if (n0 == m1)
					{
						ei.m_nbr[0] = j;
						ej.m_nbr[1] = i;
					}
				}

				if (nodeID1 == -1)
				{
					if (n1 == m0)
					{
						ei.m_nbr[1] = j;
						ej.m_nbr[0] = i;
					}
					else if (n1 == m1)
					{
						ei.m_nbr[1] = j;
						ej.m_nbr[1] = i;
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FSCurveMesh::UpdateEdgeIDs()
{
	int NE = Edges();
	if (NE == 0) return;

	// clear all IDs
	for (size_t i=0; i<NE; ++i) m_Edge[i].m_gid = -1;

	// process all edges
	int i0 = 0;
	int gid = 0;
	m_Edge[0].m_gid = 0;
	stack<int> s;
	s.push(0);
	while (s.empty() == false)
	{
		int eid = s.top(); s.pop();

		FSEdge& edge = m_Edge[eid]; assert(edge.m_gid == gid);

		// add neighbors
		if (edge.m_nbr[0] >= 0)
		{
			FSEdge& e0 = m_Edge[edge.m_nbr[0]];
			if (e0.m_gid == -1) 
			{
				e0.m_gid = gid;
				s.push(edge.m_nbr[0]);
			}
		}

		if (edge.m_nbr[1] >= 0)
		{
			FSEdge& e1 = m_Edge[edge.m_nbr[1]];
			if (e1.m_gid == -1)
			{
				e1.m_gid = gid;
				s.push(edge.m_nbr[1]);
			}
		}

		if (s.empty())
		{
			// the stack is empty but there may be some more unprocessed edges
			// they will be assigned the next edge id
			gid++;

			for (size_t i = i0; i<NE; ++i)
			{
				if (m_Edge[i].m_gid == -1)
				{
					m_Edge[i].m_gid = gid;
					s.push((int)i);
					i0 = (int)i + 1;
					break;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FSCurveMesh::UpdateNodeIDs()
{
	int NN = Nodes();
	int NE = Edges();

	// we must first calculate the valence of each node
	// since only nodes with valence != 2 will get a node ID
	vector<int> val(NN, 0);
	for (int i = 0; i<NE; ++i)
	{
		val[m_Edge[i].n[0]]++;
		val[m_Edge[i].n[1]]++;
	}

	// set all node IDs
	int gid = 0;
	for (int i=0; i<NN; ++i)
	{	
		m_Node[i].m_gid = -1;
		if (val[i] != 2) m_Node[i].m_gid = gid++;
	}
}

//-----------------------------------------------------------------------------
void FSCurveMesh::UpdateType()
{
	// see if it's empty
	if (m_Node.empty() && m_Edge.empty())
	{
		m_type = EMPTY_CURVE;
		return;
	}

	// count endpoints and segments
	int endpoints = EndPoints();
	int segments = Segments();

	// a closed curve means there are no end points and only one segment
	if ((endpoints == 0) && (segments == 1))
	{
		m_type = CLOSED_CURVE;
		return;
	}

	// a simple curve implies that there are only two end points and only one segment
	if ((endpoints == 2) && (segments == 1))
	{
		m_type = SIMPLE_CURVE;
		return;
	}

	// anything else will be tagged as complex
	m_type = COMPLEX_CURVE;
}

//-----------------------------------------------------------------------------
int FSCurveMesh::EndPoints() const
{
	int endpoints = 0;
	for (int i = 0; i<Nodes(); ++i) if (Node(i).m_gid != -1) endpoints++;
	return endpoints;
}

//-----------------------------------------------------------------------------
vector<int> FSCurveMesh::EndPointList() const
{
	vector<int> points;
	for (int i=0; i<Nodes(); ++i) if (Node(i).m_gid != -1) points.push_back(i);
	return points;
}

//-----------------------------------------------------------------------------
int FSCurveMesh::Segments() const
{
	// counts the number of segments by finding the largest edge id
	int segments = -1;
	for (int i=0; i<Edges(); ++i)
		if (Edge(i).m_gid > segments) segments = Edge(i).m_gid;

	return segments + 1;
}

//-----------------------------------------------------------------------------
void FSCurveMesh::RemoveNode(int node)
{
	// sanity check
	if ((node < 0) || (node >= Nodes()))
	{
		assert(false);
		return;
	}

	// erase the node
	m_Node.erase(m_Node.begin() + node);

	// update edges
	for (vector<FSEdge>::iterator edge = m_Edge.begin(); edge != m_Edge.end();)
	{
		if ((edge->n[0] == node) || (edge->n[1] == node))
		{
			edge = m_Edge.erase(edge);
		}
		else
		{
			if (edge->n[0] > node) edge->n[0]--;
			if (edge->n[1] > node) edge->n[1]--;
			++edge;
		}
	}

	// NOTE: This is only used for the point-to-point line creation where only the edges neighbors need to be updated
	//       I am not sure if this would work in general.
	UpdateEdgeNeighbors();
}

//-----------------------------------------------------------------------------
void FSCurveMesh::TagAllNodes(int ntag)
{
	for (int i=0; i<Nodes(); ++i) Node(i).m_ntag = ntag;
}

//-----------------------------------------------------------------------------
void FSCurveMesh::TagAllEdges(int ntag)
{
	for (int i = 0; i<Edges(); ++i) Edge(i).m_ntag = ntag;
}

//-----------------------------------------------------------------------------
void FSCurveMesh::FlipEdge(FSEdge& e)
{
	int m = e.n[0];
	e.n[0] = e.n[1];
	e.n[1] = m;

	m = e.m_nbr[0];
	e.m_nbr[0] =e.m_nbr[1];
	e.m_nbr[1] = m;
}

//-----------------------------------------------------------------------------
// reorder nodes based on look-up table
void FSCurveMesh::ReorderNodes(vector<int>& NLT)
{
	int NN = Nodes();
	assert(NN == (int) NLT.size());

	// make sure there is a one-to-one mapping between old and new indices
	vector<int> tmp(NN, -1);
	for (int i=0; i<NN; ++i)
	{
		int n = NLT[i];
		Node(i).m_ntag = n;
		assert((n>=0)&&(n<NN));
		assert(tmp[n] == -1);
		tmp[n] = i;
	}

	// update edges
	int NE = Edges();
	for (int i=0; i<NE; ++i)
	{
		FSEdge& e = Edge(i);
		e.n[0] = NLT[e.n[0]];
		e.n[1] = NLT[e.n[1]];
	}

	// process nodes
	for (int i=0; i<NN; ++i)
	{
		assert(tmp[i] != -1);
		if (i != tmp[i])
		{
			assert(tmp[i] > i);
			FSNode& oldNode = Node(i);
			FSNode& newNode = Node(tmp[i]);

			int m0 = newNode.m_ntag; assert(m0 == i);
			int m1 = oldNode.m_ntag;

			FSNode copy(oldNode);
			oldNode = newNode;
			newNode = copy;

			tmp[m1] = tmp[i];
			tmp[i] = m0;
		}
	}	
}

//-----------------------------------------------------------------------------
// reorder edges based on look-up table
void FSCurveMesh::ReorderEdges(vector<int>& ELT)
{
	int NE = Edges();
	assert(NE == (int)ELT.size());

	// make sure there is a one-to-one mapping between old and new indices
	vector<int> tmp(NE, -1);
	for (int i = 0; i<NE; ++i)
	{
		int n = ELT[i];
		Edge(i).m_ntag = n;
		assert((n >= 0) && (n<NE));
		assert(tmp[n] == -1);
		tmp[n] = i;
	}

	// update neighbors
	for (int i = 0; i<NE; ++i)
	{
		FSEdge& e = Edge(i);
		if (e.m_nbr[0] >= 0) e.m_nbr[0] = ELT[e.m_nbr[0]];
		if (e.m_nbr[1] >= 0) e.m_nbr[1] = ELT[e.m_nbr[1]];
	}
	
	// process edges
	for (int i = 0; i<NE; ++i)
	{
		assert(tmp[i] != -1);
		if (i != tmp[i])
		{
			assert(tmp[i] > i);
			FSEdge& oldEdge = Edge(i);
			FSEdge& newEdge = Edge(tmp[i]);

			int m0 = newEdge.m_ntag; assert(m0 == i);
			int m1 = oldEdge.m_ntag;

			FSEdge copy(oldEdge);
			oldEdge = newEdge;
			newEdge = copy;

			tmp[m1] = tmp[i];
			tmp[i] = m0;
		}
	}
}

//-----------------------------------------------------------------------------
void FSCurveMesh::Invert()
{
	int NN = Nodes();
	for (int i=0; i<NN/2; ++i)
	{
		if (i != NN-i-1)
		{
			FSNode tmp = m_Node[i];
			m_Node[i] = m_Node[NN-i-1];
			m_Node[NN-i-1] = tmp;
		}
	}
}

//-----------------------------------------------------------------------------
void FSCurveMesh::Sort()
{
	// we only do this for simple or closed curves
	if ((m_type != SIMPLE_CURVE) && (m_type != CLOSED_CURVE))
	{
		assert(false);
		return;
	}

	TagAllEdges(-1);
	TagAllNodes(-1);

	int NN = Nodes();
	int NE = Edges();

	// we make we have something to do
	if (NE == 1) return;

	// index counters
	int inode = 0;
	int iedge = 0;

	// repeat until done
	bool bdone = false;
	do
	{
		// let's be positive
		bdone = true;

		// find an unprocessed edge
		int nedge = -1;

		// first look for open edges
		for (int i=0; i<NE; ++i)
		{
			FSEdge& e = Edge(i);
			if (e.m_ntag == -1)
			{
				if ((e.m_nbr[0] == -1) && (e.m_nbr[1] != -1))
				{
					nedge = i;
					break;
				}
				else if ((e.m_nbr[0] != -1) && (e.m_nbr[1] == -1))
				{
					// we need to flip this edge
					FlipEdge(e);
					nedge = i;
					break;
				}
			}
		}

		// if we didn't find an open edge, maybe the curve is closed so let's look for any unprocessed edge
		if (nedge == -1)
		{
			for (int i = 0; i<NE; ++i)
			{
				FSEdge& e = Edge(i);
				if (e.m_ntag == -1)
				{
					assert((e.m_nbr[0] != -1) && (e.m_nbr[1] != -1));
					nedge = i;
					break;
				}
			}
		}

		// if we still didn't find an edge, we're done
		if (nedge != -1) bdone = false;

		// start processing the curve
		while (nedge != -1)
		{
			FSEdge& e = Edge(nedge);
			e.m_ntag = iedge++;
			int n0 = e.n[0];
			int n1 = e.n[1];

			if (Node(n0).m_ntag == -1) Node(n0).m_ntag = inode++;
			if (Node(n1).m_ntag == -1) Node(n1).m_ntag = inode++;

			nedge = -1;
			if (e.m_nbr[1] != -1)
			{
				FSEdge& en = Edge(e.m_nbr[1]);
				assert((en.n[0] == n1) || (en.n[1] == n1));
				if (en.m_ntag == -1)
				{
					nedge = e.m_nbr[1];
					if (en.n[1] == n1) FlipEdge(en);
				}
			}
		}
	}
	while (bdone == false);

	// build the reorder tables
	vector<int> NLT(NN);
	for (int i=0; i<NN; ++i) NLT[i] = Node(i).m_ntag;

	vector<int> ELT(NE);
	for (int i = 0; i<NE; ++i) ELT[i] = Edge(i).m_ntag;

	// reindex nodes and edges
	ReorderNodes(NLT);
	ReorderEdges(ELT);
}

//-----------------------------------------------------------------------------
void FSCurveMesh::Attach(const FSCurveMesh& curve)
{
	int N0 = Nodes();
	int NN = curve.Nodes();
	vector<int> tag(NN);
	int count = N0;
	for (int i=0; i<NN; ++i)
	{
		// get the next node
		vec3d ri = curve.Node(i).pos();

		// see if this node coincides with another node in the mesh
		int newIndex = -1;
		for (int j=0; j<N0; ++j)
		{
			vec3d rj = Node(j).pos();
			double D2 = (ri - rj)*(ri - rj);
			if (D2 < 1e-9)
			{
				newIndex = j;
				break;
			}
		}

		// if it doesn't add it
		if (newIndex == -1)
		{
			newIndex = count++;
			AddNode(ri);
		}

		// track the index of this new node
		tag[i] = newIndex;
	}

	// add the edges
	int NE = curve.Edges();
	for (int i=0; i<NE; ++i)
	{
		const FSEdge& e = curve.Edge(i);
		int n0 = tag[e.n[0]];
		int n1 = tag[e.n[1]];

		FSEdge newEdge;
		newEdge.n[0] = n0;
		newEdge.n[1] = n1;
		m_Edge.push_back(newEdge);
	}

	// update data structures
	BuildMesh();
}

//-----------------------------------------------------------------------------
double FSCurveMesh::Length() const
{
	double L = 0.0;
	for (int i=0; i<Edges(); ++i)
	{
		const FSEdge& e = Edge(i);
		vec3d ra = Node(e.n[0]).r;
		vec3d rb = Node(e.n[1]).r;
		L += (rb - ra).Length();
	}
	return L;
};

//-----------------------------------------------------------------------------
void FSCurveMesh::Save(OArchive& ar)
{
	int nodes = Nodes();
	int edges = Edges();

	// write the header
	ar.BeginChunk(CID_MESH_HEADER);
	{
		ar.WriteChunk(CID_MESH_NODES, nodes);
		ar.WriteChunk(CID_MESH_EDGES, edges);
	}
	ar.EndChunk();

	// write the nodes
	ar.BeginChunk(CID_MESH_NODE_SECTION);
	{
		FSNode* pn = NodePtr();
		for (int i = 0; i<nodes; ++i, ++pn)
		{
			ar.BeginChunk(CID_MESH_NODE);
			{
				ar.WriteChunk(CID_MESH_NODE_GID, pn->m_gid);
				ar.WriteChunk(CID_MESH_NODE_POSITION, pn->r);
			}
			ar.EndChunk();
		}
	}
	ar.EndChunk();

	// write the edges
	ar.BeginChunk(CID_MESH_EDGE_SECTION);
	{
		FSEdge* pe = EdgePtr();
		for (int i = 0; i<edges; ++i, ++pe)
		{
			int nn = pe->Nodes();
			int ntype = pe->Type();
			ar.BeginChunk(CID_MESH_EDGE);
			{
				ar.WriteChunk(CID_MESH_EDGE_TYPE, ntype);
				ar.WriteChunk(CID_MESH_EDGE_GID, pe->m_gid);
				ar.WriteChunk(CID_MESH_EDGE_NODES, pe->n, nn);
			}
			ar.EndChunk();
		}
	}
	ar.EndChunk();
}

//-----------------------------------------------------------------------------
void FSCurveMesh::Load(IArchive& ar)
{
	TRACE("FSSurfaceMesh::Load");

	// the first chunk must be the header
	ar.OpenChunk();
	if (ar.GetChunkID() != CID_MESH_HEADER) throw ReadError("Missing mesh header");

	// read the header
	int nodes, edges;
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_MESH_NODES: ar.read(nodes); break;
		case CID_MESH_EDGES: ar.read(edges); break;
		}
		ar.CloseChunk();
	}
	ar.CloseChunk();

	// allocate storage
	Create(nodes, edges);

	// read the rest of the mesh data
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_MESH_NODE_SECTION:
		{
			int n = 0;
			FSNode* pn = NodePtr();
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				int nid = ar.GetChunkID();
				if (nid == CID_MESH_NODE)
				{
					assert(n < nodes);
					while (IArchive::IO_OK == ar.OpenChunk())
					{
						int nid = ar.GetChunkID();
						switch (nid)
						{
						case CID_MESH_NODE_GID: ar.read(pn->m_gid); break;
						case CID_MESH_NODE_POSITION: ar.read(pn->r); break;
						}
						ar.CloseChunk();
					}
					++pn;
					++n;
				}
				ar.CloseChunk();
			}
		}
		break;
		case CID_MESH_EDGE_SECTION:
		{
			int n = 0;
			FSEdge* pe = EdgePtr();
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				int nid = ar.GetChunkID();
				if (nid != CID_MESH_EDGE) throw ReadError("error parsing CID_MESH_EDGE_SECTION");

				int ntype;
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int nid = ar.GetChunkID();
					switch (nid)
					{
					case CID_MESH_EDGE_TYPE:
						{
							ar.read(ntype);
							switch (ntype)
							{
							case FE_EDGE2: pe->SetType(FE_EDGE2);  break;
							case FE_EDGE3: pe->SetType(FE_EDGE3);  break;
							case FE_EDGE4: pe->SetType(FE_EDGE4);  break;
							default:
								assert(false);
								throw ReadError("error parsing CID_MESH_EDGE_SECTION");
							}
						}
						break;
					case CID_MESH_EDGE_GID: ar.read(pe->m_gid); break;
					case CID_MESH_EDGE_NODES:
						{
							int nn = pe->Nodes();
							assert(nn > 0);
							if (nn <= 0) throw ReadError("error parsing CID_MESH_EDGE_SECTION");
							ar.read(pe->n, nn);
						}
						break;
					}
					ar.CloseChunk();
				}

				assert(n < edges);
				++pe;
				++n;

				ar.CloseChunk();
			}
		}
		break;
		}
		ar.CloseChunk();
	}

	// rebuild mesh' data
	UpdateEdgeNeighbors();
}

void FSCurveMesh::UpdateMesh()
{
	
}
