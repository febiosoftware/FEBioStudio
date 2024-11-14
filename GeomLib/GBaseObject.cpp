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

#include "GBaseObject.h"

//-----------------------------------------------------------------------------
// initialize static variables
template <> int GItem_T<GBaseObject>::m_ncount = 0;

//-----------------------------------------------------------------------------
GBaseObject::GBaseObject()
{
}

//-----------------------------------------------------------------------------
GBaseObject::~GBaseObject()
{
	ClearAll();
}

//-----------------------------------------------------------------------------
void GBaseObject::Copy(const GBaseObject* po)
{
	// clear everything
	ClearAll();

	// make sure there is something to do
	if (po == 0) return;

	// copy nodes
	int NN = po->Nodes();
	for (int i=0; i<NN; ++i)
	{
		const GNode& node = *po->Node(i);

		GNode* newNode = new GNode(this);
		newNode->LocalPosition() = node.LocalPosition();
		newNode->SetType(node.Type());
		AddNode(newNode);
	}

	// copy edges
	int NE = po->Edges();
	for (int i=0; i<NE; ++i)
	{
		const GEdge& edge = *po->Edge(i);

		GEdge* newEdge = new GEdge(this);
		newEdge->m_node[0] = edge.m_node[0];
		newEdge->m_node[1] = edge.m_node[1];
		newEdge->m_cnode   = edge.m_cnode;
		newEdge->m_ntype   = edge.m_ntype;

		AddEdge(newEdge);
	}

	// copy faces
	int NF = po->Faces();
	for (int i=0; i<NF; ++i)
	{
		const GFace& face = *po->Face(i);

		GFace* newFace = new GFace(this);
		newFace->m_ntype = face.m_ntype;
		newFace->m_nPID[0] = face.m_nPID[0];
		newFace->m_nPID[1] = face.m_nPID[1];
		newFace->m_node = face.m_node;
		newFace->m_edge = face.m_edge;
		AddFace(newFace);
	}

	// copy parts
	int NP = po->Parts();
	for (int i=0; i<NP; ++i)
	{
		AddPart();
	}
}


//-----------------------------------------------------------------------------
void GBaseObject::ClearAll()
{
	for (int i = 0; i<(int)m_Node.size(); ++i) delete m_Node[i]; m_Node.clear();
	for (int i = 0; i<(int)m_Edge.size(); ++i) delete m_Edge[i]; m_Edge.clear();
	for (int i = 0; i<(int)m_Face.size(); ++i) delete m_Face[i]; m_Face.clear();
	for (int i = 0; i<(int)m_Part.size(); ++i) delete m_Part[i]; m_Part.clear();
}

//-----------------------------------------------------------------------------
void GBaseObject::ClearFaces()
{
	for (int i = 0; i<(int)m_Face.size(); ++i) delete m_Face[i];
	m_Face.clear();
}

//-----------------------------------------------------------------------------
void GBaseObject::ClearEdges()
{
	for (int i = 0; i<(int)m_Edge.size(); ++i) delete m_Edge[i];
	m_Edge.clear();
}

//-----------------------------------------------------------------------------
void GBaseObject::ClearNodes()
{
	for (int i = 0; i<(int)m_Node.size(); ++i) delete m_Node[i];
	m_Node.clear();
}

//-----------------------------------------------------------------------------
void GBaseObject::ClearParts()
{
	for (int i = 0; i<(int)m_Part.size(); ++i) delete m_Part[i];
	m_Part.clear();
}

//-----------------------------------------------------------------------------
void GBaseObject::ResizeParts(int n)
{
	for (int i=n; i<(int)m_Part.size(); ++i) delete m_Part[i];
	m_Part.resize(n);
}

//-----------------------------------------------------------------------------
void GBaseObject::ResizeSurfaces(int n)
{
	for (int i = n; i<(int)m_Face.size(); ++i) delete m_Face[i];
	m_Face.resize(n);
}

//-----------------------------------------------------------------------------
void GBaseObject::ResizeCurves(int n)
{
	for (int i = n; i<(int)m_Edge.size(); ++i) delete m_Edge[i];
	m_Edge.resize(n);
}

//-----------------------------------------------------------------------------
void GBaseObject::ResizeNodes(int n)
{
	for (int i = n; i<(int)m_Node.size(); ++i) delete m_Node[i];
	m_Node.resize(n);
}

//-----------------------------------------------------------------------------
GPart* GBaseObject::FindPart(int nid)
{
	for (int i=0; i<Parts(); ++i) if (m_Part[i]->GetID() == nid) return Part(i);
	return 0;
}

//-----------------------------------------------------------------------------

GFace* GBaseObject::FindFace(int nid)
{
	for (int i = 0; i<Faces(); ++i) if (m_Face[i]->GetID() == nid) return Face(i);
	return 0;
}

//-----------------------------------------------------------------------------

GEdge* GBaseObject::FindEdge(int nid)
{
	for (int i = 0; i<Edges(); ++i) if (m_Edge[i]->GetID() == nid) return Edge(i);
	return 0;
}

//-----------------------------------------------------------------------------

GNode* GBaseObject::FindNode(int nid)
{
	for (int i = 0; i<Nodes(); ++i) if (m_Node[i]->GetID() == nid) return  Node(i);
	return 0;
}

//-----------------------------------------------------------------------------
GPart* GBaseObject::FindPartFromName(const char* szname)
{
	if (szname == nullptr) return nullptr;
	for (int i = 0; i < Parts(); ++i)
	{
		GPart* pi = Part(i);
		if (strcmp(szname, pi->GetName().c_str()) == 0) return pi;
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
void GBaseObject::AddFace(GFace* f)
{
	f->SetID(GFace::CreateUniqueID());
	f->SetLocalID((int)m_Face.size());
	char sz[256] = {0};
	sprintf(sz, "Surface%d", f->GetID());
	f->SetName(sz);
	m_Face.push_back(f);
}

//-----------------------------------------------------------------------------
GFace* GBaseObject::AddFace()
{
	GFace* f = new GFace(this);
	AddFace(f);
	return f;
}

//-----------------------------------------------------------------------------
int GBaseObject::AddNode(GNode* n)
{
	n->SetID(GNode::CreateUniqueID());
	n->SetLocalID(m_Node.size());
	char sz[256] = {0};
	sprintf(sz, "Node%d", n->GetID());
	n->SetName(sz);
	m_Node.push_back(n);
	return m_Node.size() - 1;
}

//-----------------------------------------------------------------------------
// This function adds a node to the object. If bdup == false no node duplication
// is allowed and it first checks whether the node
// has already been defined or not. If not, the new vertex is added. If so,
// the index to the existing node is returned.
// TODO: What if a shape_node is added onto a vertex node? Should I change the type?
GNode* GBaseObject::AddNode(vec3d r, int nt, bool bdup)
{
	// if node duplication is not allowed
	// check if this node is already defined
	if (bdup == false)
	{
		const double tol = 1e-6;
		// first, see if this node is already defined
		int N = (int) Nodes();
		for (int i=0; i<N; ++i)
		{
			vec3d p = Node(i)->LocalPosition();
			double L = (r - p).Length();
			if (L <= tol) return Node(i);
		}
	}

	// if we get here, the vertex does not exist yet,
	// so let's add it
	GNode* n = new GNode(this);
	n->SetType(nt);
	if (nt == NODE_VERTEX)
		n->SetID(GNode::CreateUniqueID());
	else
		n->SetID(-1);
	n->SetLocalID(m_Node.size());
	n->LocalPosition() = r;

	char sz[256] = {0};
	sprintf(sz, "Node%d", n->GetID());
	n->SetName(sz);

	m_Node.push_back(n);
	return n;
}

//-----------------------------------------------------------------------------
GEdge* GBaseObject::AddEdge()
{
	GEdge* edge = new GEdge(this);
	AddEdge(edge);
	return edge;
}

//-----------------------------------------------------------------------------
// This function adds an edge to the object, but first checks if the edge
// is already defined
int GBaseObject::AddEdge(GEdge* e)
{
	int N = (int)Edges();
	if (e->Type() != EDGE_UNKNOWN)
	{
		for (int i = 0; i < N; ++i)
		{
			GEdge& s = *Edge(i);
			if (*e == s) return i;
		}
	}

	// add the edge
	e->SetID(GEdge::CreateUniqueID());
	e->SetLocalID(m_Edge.size());

	char sz[256] = {0};
	sprintf(sz, "Curve%d", e->GetID());
	e->SetName(sz);

	m_Edge.push_back(e);

	return N;
}

//-----------------------------------------------------------------------------
int GBaseObject::AddLine(int n1, int n2)
{
	assert((n1 >= 0) && (n1 < (int) m_Node.size()));
	assert((n2 >= 0) && (n2 < (int) m_Node.size()));
	assert(n1 != n2);

	GEdge* e = new GEdge(this);
	e->m_ntype = EDGE_LINE;
	e->m_node[0] = n1;
	e->m_node[1] = n2;

	return AddEdge(e);
}

//-----------------------------------------------------------------------------
int GBaseObject::AddYArc(int n1, int n2)
{
	assert((n1 >= 0) && (n1 < (int) m_Node.size()));
	assert((n2 >= 0) && (n2 < (int) m_Node.size()));

	GEdge* e = new GEdge(this);
	e->m_ntype = EDGE_YARC;
	e->m_node[0] = n1;
	e->m_node[1] = n2;
	return AddEdge(e);
}

//-----------------------------------------------------------------------------
int GBaseObject::AddZArc(int n1, int n2)
{
	assert((n1 >= 0) && (n1 < (int) m_Node.size()));
	assert((n2 >= 0) && (n2 < (int) m_Node.size()));

	GEdge* e = new GEdge(this);
	e->m_ntype = EDGE_ZARC;
	e->m_node[0] = n1;
	e->m_node[1] = n2;
	return AddEdge(e);
}

//-----------------------------------------------------------------------------
int GBaseObject::AddCircularArc(int n1, int n2, int n3)
{
	assert((n1 >= 0) && (n1 < (int) m_Node.size()));
	assert((n2 >= 0) && (n2 < (int) m_Node.size()));
	assert((n3 >= 0) && (n3 < (int) m_Node.size()));
	assert(n1 != n2);
	assert(n1 != n3);

	GEdge* e = new GEdge(this);
	e->m_ntype = EDGE_3P_CIRC_ARC;
	e->m_cnode.push_back(n1);
	e->m_node[0] = n2;
	e->m_node[1] = n3;	
	return AddEdge(e);
}

//-----------------------------------------------------------------------------
int GBaseObject::AddArcSection(int n1, int n2, int n3)
{
	assert((n1 >= 0) && (n1 < (int) m_Node.size()));
	assert((n2 >= 0) && (n2 < (int) m_Node.size()));
	assert((n3 >= 0) && (n3 < (int) m_Node.size()));
	assert(n1 != n2);
	assert(n1 != n3);

	GEdge* e = new GEdge(this);
	e->m_ntype = EDGE_3P_ARC;
	e->m_cnode.push_back(n1);
	e->m_node[0] = n2;
	e->m_node[1] = n3;	
	return AddEdge(e);
}

int GBaseObject::AddBezierSection(const std::vector<int>& n)
{
	assert(n.size() >= 2);
	GEdge* e = new GEdge(this);
	e->m_ntype = EDGE_BEZIER;
	e->m_node[0] = n[0];
	for (int i=1; i<n.size()-1; ++i)
		e->m_cnode.push_back(n[i]);
	e->m_node[1] = n[n.size()-1];
	return AddEdge(e);
}

//-----------------------------------------------------------------------------
// Build a facet from a wire, that is an edge list. It is assumed that the edges
// are in the proper order, that is that each edge connects to its left and right
// neigbor and that the wire is closed.
void GBaseObject::AddFacet(const std::vector<int>& edge, int ntype)
{
	// allocate a new face
	GFace* f = new GFace(this);

	f->SetID(GFace::CreateUniqueID());
	f->SetLocalID(m_Face.size());
	// TODO: what if we add an interior face?
	f->m_nPID[0] = 0;

	char sz[256] = {0};
	sprintf(sz, "Surface%d", f->GetID());
	f->SetName(sz);

	// number of edges (will also be the number of nodes)
	int NE = (int) edge.size();
	assert(NE >= 3);

	// node list and winding list
	std::vector<int> node; std::vector<int> ew;

	// figure out the orientation of the first edge
	// we can decided that by looking which node connects to the second edge
	GEdge* e0 = Edge(edge[0]);
	GEdge* e1 = Edge(edge[1]);
	int np = -1;
	if (e1->HasNode(e0->m_node[0]))
	{
		node.push_back(e0->m_node[1]);
		np = e0->m_node[0];
		ew.push_back(-1);
	}
	else if (e1->HasNode(e0->m_node[1]))
	{
		node.push_back(e0->m_node[0]);
		np = e0->m_node[1];
		ew.push_back(1);
	}
	else 
	{
		assert(false);
	}
	
	// list all nodes that define this face
	// and figure out the edge windings
	for (int i=1; i<NE; ++i)
	{
		GEdge* e = Edge(edge[i]);
		if (e->m_node[0] == np)
		{
			node.push_back(e->m_node[0]);
			np = e->m_node[1];
			ew.push_back(1);
		}
		else if (e->m_node[1] == np)
		{
			node.push_back(e->m_node[1]);
			np = e->m_node[0];
			ew.push_back(-1);
		}
		else 
		{
			assert(false);
		}
	}
	assert((int)node.size() == NE);

	// let's define the face
	f->m_node = node;
	f->m_edge.resize(NE);
	for (int i=0; i<NE; ++i)
	{
		f->m_edge[i].nid = edge[i];
		f->m_edge[i].nwn = ew[i];
	}

	f->m_ntype = ntype;
	m_Face.push_back(f);
}

//-----------------------------------------------------------------------------
void GBaseObject::AddFacet(const std::vector<int>& node, const std::vector<pair<int, int> >& edge, int ntype)
{
	GFace* f = new GFace(this);
	f->SetID(GFace::CreateUniqueID());
	f->SetLocalID(m_Face.size());
	f->m_node = node;

	char sz[256] = {0};
	sprintf(sz, "Surface%d", f->GetID());
	f->SetName(sz);

	// TODO: What if we add an interior face?
	f->m_nPID[0] = 0;

	int ne = (int) edge.size();
	f->m_edge.resize(ne);
	for (int i=0; i<ne; ++i)
	{
		f->m_edge[i].nid = edge[i].first;
		f->m_edge[i].nwn = edge[i].second;
	}

	f->m_ntype = ntype;
	m_Face.push_back(f);
}

//-----------------------------------------------------------------------------
void GBaseObject::AddSurface(GFace* f)
{
	f->SetID(GFace::CreateUniqueID());
	f->SetLocalID(m_Face.size());

	char sz[256] = {0};
	sprintf(sz, "Surface%d", f->GetID());
	f->SetName(sz);

	m_Face.push_back(f);
}

//-----------------------------------------------------------------------------
GPart* GBaseObject::AddPart()
{
	GPart* p = new GPart(this);
	p->SetID(GPart::CreateUniqueID());
	p->SetLocalID(m_Part.size());
	p->SetMaterialID(-1);
	char szname[256] = {0};
	sprintf(szname, "Part%d", p->GetID());
	p->SetName(szname);
	m_Part.push_back(p);
	return p;
}

//-----------------------------------------------------------------------------
bool GBaseObject::DeletePart(GPart* pg)
{
	// let the derived class do the heavy lifting. 
	return false;
}

GPart* GBaseObject::AddSolidPart()
{
	GPart* p = AddPart();
	p->SetSection(new GSolidSection(p));
	return p;
}

GPart* GBaseObject::AddShellPart()
{
	GPart* p = AddPart();
	p->SetSection(new GShellSection(p));
	return p;
}

GPart* GBaseObject::AddBeamPart()
{
	GPart* p = AddPart();
	p->SetSection(new GBeamSection(p));
	return p;
}

//-----------------------------------------------------------------------------
//! This function updates the type of nodes. A node is either a vertex, which
//! means it is considered part of the geometry, or it is a shape-node, which 
//! are only used to define the geometry, but are not considered part of it.
//! For now, we define the distinction as follows: if a node is an end-point
//! of an edge, it is a vertex, otherwise it is a shape-node.
void GBaseObject::UpdateNodeTypes()
{
	// mark all nodes as shape-nodes
	for (int i=0; i<Nodes(); ++i) Node(i)->SetType(NODE_SHAPE);

	// loop over all edges
	for (int i=0; i<Edges(); ++i)
	{
		// get the next edge
		GEdge* pe = Edge(i);

		// mark the end-points as vertices
		Node(pe->m_node[0])->SetType(NODE_VERTEX);
		Node(pe->m_node[1])->SetType(NODE_VERTEX);
	}
}

//-----------------------------------------------------------------------------
void GBaseObject::CopyTransform(GBaseObject* po)
{
	m_transform = po->m_transform;
}

//-----------------------------------------------------------------------------
void GBaseObject::RemoveNode(int n)
{
	// delete the node
	delete m_Node[n];
	m_Node.erase(m_Node.begin() + n);

	// delete edges that contain this node
	for (int i = 0; i < Edges();)
	{
		GEdge* ei = Edge(i);
		if ((ei->m_node[0] == n) || (ei->m_node[1] == n))
		{
			delete m_Edge[i];
			m_Edge.erase(m_Edge.begin() + i);
		}
		else ++i;
	}

	assert(m_Face.empty());

	Update();
}
