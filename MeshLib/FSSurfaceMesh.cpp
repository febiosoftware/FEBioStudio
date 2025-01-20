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

#include "FSSurfaceMesh.h"
#include "TriMesh.h"
#include <FSCore/Serializable.h>
#include <GeomLib/GObject.h>
#include "FSFaceEdgeList.h"
#include "FSNodeEdgeList.h"
#include "FSNodeFaceList.h"
using namespace std;

FSSurfaceMesh::FSSurfaceMesh()
{
	
}

FSSurfaceMesh::FSSurfaceMesh(const FSSurfaceMesh& mesh)
{
	m_Node = mesh.m_Node;
	m_Edge = mesh.m_Edge;
	m_Face = mesh.m_Face;
	m_box = mesh.m_box;
}

FSSurfaceMesh::~FSSurfaceMesh()
{
}

void FSSurfaceMesh::Clear()
{
	m_Node.clear();
	m_Edge.clear();
	m_Face.clear();
	m_data.Clear();
}

FSSurfaceMesh::FSSurfaceMesh(TriMesh& dyna)
{
	TriMesh::NodeIterator nodePtr(dyna);
	for (; nodePtr.isValid(); ++nodePtr) nodePtr->ntag = -1;

	TriMesh::FaceIterator facePtr(dyna);
	for (; facePtr.isValid(); ++facePtr)
	{
		facePtr->node[0]->ntag = 1;
		facePtr->node[1]->ntag = 1;
		facePtr->node[2]->ntag = 1;
	}

	nodePtr.reset();
	int nodes = 0;
	for (; nodePtr.isValid(); ++nodePtr)
		if (nodePtr->ntag == 1) nodePtr->ntag = nodes++;

	int NN = nodes;
	int NE = dyna.Edges();
	int NF = dyna.Faces();

	Create(NN, NE, NF);

	nodePtr.reset();
	for (; nodePtr.isValid(); ++nodePtr)
	{
		if (nodePtr->ntag >= 0)
		{
			FSNode& node = Node(nodePtr->ntag);
			node.r = nodePtr->r;
			node.m_gid = nodePtr->gid;
		}
	}

	TriMesh::EdgeIterator edgePtr(dyna);
	for (int i=0; i<NE; ++i, ++edgePtr)
	{
		FSEdge& edge = Edge(i);
		edge.SetType(FE_EDGE2);
		edge.n[0] = edgePtr->node[0]->ntag; assert(edge.n[0] >= 0);
		edge.n[1] = edgePtr->node[1]->ntag; assert(edge.n[1] >= 0);
		edge.m_gid = edgePtr->gid;
	}

	facePtr.reset();
	for (int i = 0; i<NF; ++i, ++facePtr)
	{
		FSFace& face = Face(i);
		face.SetType(FE_FACE_TRI3);
		face.n[0] = facePtr->node[0]->ntag; assert((face.n[0] >= 0) && (face.n[0] < NN));
		face.n[1] = facePtr->node[1]->ntag; assert((face.n[1] >= 0) && (face.n[1] < NN));
		face.n[2] = facePtr->node[2]->ntag; assert((face.n[2] >= 0) && (face.n[2] < NN));
		face.m_gid = facePtr->gid; assert(facePtr->gid >= 0);
	}
	UpdateFacePartitions();
	UpdateEdgePartitions();
	UpdateNodePartitions();

	// update the mesh
	UpdateFaceNeighbors();
	UpdateEdgeNeighbors();
	UpdateFaceEdges();
	UpdateNormals();
	UpdateBoundingBox();

	// let's update the new nodes
	AutoPartitionNodes();
}

FSSurfaceMesh& FSSurfaceMesh::operator = (const FSSurfaceMesh& mesh)
{
	m_Node = mesh.m_Node;
	m_Edge = mesh.m_Edge;
	m_Face = mesh.m_Face;

	return (*this);
}

bool FSSurfaceMesh::IsType(FEFaceType type)
{
	int NF = Faces();
	for (int i=0; i<NF; ++i)
	{
		if (Face(i).Type() != type) return false;
	}
	return true;
}

void FSSurfaceMesh::Create(unsigned int nodes, unsigned int edges, unsigned int faces)
{
	if (nodes) m_Node.resize(nodes);
	if (edges) m_Edge.resize(edges);
	if (faces)
	{
		m_Face.resize(faces);
		for (unsigned int i = 0; i < faces; ++i)
		{
			m_Face[i].m_gid = 0;
			m_Face[i].m_sid = 0;
		}
	}
}

//-----------------------------------------------------------------------------
void FSSurfaceMesh::Update()
{
	UpdateFaceNeighbors();
	UpdateEdgeNeighbors();
	UpdateNormals();
	UpdateBoundingBox();
}

//-----------------------------------------------------------------------------
// Build the mesh data structures
// It is assumed that all faces have been assigned to a partition
void FSSurfaceMesh::BuildMesh()
{
	// -- Build face data ---
	// find all the face neighbors
	UpdateFaceNeighbors();

	// -- Build edge data ---
	// Build edges (Depends on face partitioning!)
	BuildEdges();

	// update edge neighbors (requires initial edge partitioning!)
	UpdateEdgeNeighbors();

	// partition edges (Depends on edge neighbors!)
	AutoPartitionEdges();

	// -- Build node data ---
	// partition the nodes (Depends on edge partitioning)
	AutoPartitionNodes();

	// update other mesh data
	UpdateNormals();
	UpdateBoundingBox();
}

//-----------------------------------------------------------------------------
void FSSurfaceMesh::RebuildMesh(double smoothingAngle)
{
	// We need to build the neighbors before we can call autopartition
	UpdateFaceNeighbors();

	// partition
	AutoPartition(smoothingAngle);

	// update the bounding box
	UpdateBoundingBox();
}

//-----------------------------------------------------------------------------
// This assumes that face neighbors are determined.
void FSSurfaceMesh::AutoPartition(double smoothingAngle)
{
	// auto-smooth the surface
	if (smoothingAngle > 0.0)
	{
		AutoSmooth(smoothingAngle);

		// now, assign face groupd IDs from the smoothing IDs
		int NF = Faces();
		for (int i = 0; i < NF; ++i)
		{
			FSFace& face = Face(i);
			face.m_gid = face.m_sid;
		}
	}

	// -- Build edge data ---
	// Build edges (Depends on face partitioning!)
	BuildEdges();

	// update edge neighbors (requires initial edge partitioning! This is done in BuildEdges.)
	UpdateEdgeNeighbors();

	// partition edges (Depends on edge neighbors!)
	AutoPartitionEdges();

	// -- Build node data ---
	// partitioned the nodes (Depends on edge partitioning)
	AutoPartitionNodes();

	// update other mesh data
	UpdateNormals();
}

//-----------------------------------------------------------------------------
// This partitions the edges. 
// It is assumed that all edge neighbors have been identified and that all edges 
// that are part of the contour are identified with a gid >= 0.
// Note that any preexisting partitioning will be overwritten.
void FSSurfaceMesh::AutoPartitionEdges()
{
	// Tag candidate edges
	for (int i = 0; i < Edges(); ++i)
	{
		FSEdge& edge = Edge(i);
		if (edge.m_gid >= 0) edge.m_ntag = 0;
		else edge.m_ntag = -1;
	}

	// loop over candidate edges
	int ng = 0, n0 = 0;
	stack<int> s;
	do
	{
		for (int i=n0; i<Edges(); ++i, ++n0)
		{
			if (Edge(i).m_ntag == 0)
			{
				s.push(i);
				break;
			}
		}

		if (s.empty()) break;

		while (s.empty() == false)
		{
			int edgeId = s.top(); s.pop();

			FSEdge& edge = Edge(edgeId);
			edge.m_ntag = 1;
			edge.m_gid = ng;
			
			if (edge.m_nbr[0] != -1)
			{
				FSEdge& e0 = Edge(edge.m_nbr[0]);
				if (e0.m_ntag == 0) s.push(edge.m_nbr[0]);
			}

			if (edge.m_nbr[1] != -1)
			{
				FSEdge& e1 = Edge(edge.m_nbr[1]);
				if (e1.m_ntag == 0) s.push(edge.m_nbr[1]);
			}
		}

		ng++;
	}
	while (1);
}

//-----------------------------------------------------------------------------
// Update the edge neighbors
// Only edges that have a non-negative gid are considered. The actual 
// gid values are not used. Edges are connected if they share a node that
// has a valence of exactly two. 
void FSSurfaceMesh::UpdateEdgeNeighbors()
{
	FSNodeEdgeList NET;
	NET.Build(this);

	for (int i=0; i<Edges(); ++i)
	{
		FSEdge& edge = Edge(i);
		edge.m_nbr[0] = -1;
		edge.m_nbr[1] = -1;
	}

	for (int i=0; i<Edges(); ++i)
	{
		FSEdge& edge = Edge(i);
		if (edge.m_gid >= 0)
		{
			for (int j=0; j<2; ++j)
			{
				int nj = edge.n[j]; assert(nj != -1);
				int nval = NET.Edges(nj);
				int adj = 0;
				int na[2] = { 0,0 };
				for (int k = 0; k < nval; k++)
				{
					int ek = NET.EdgeIndex(nj, k);
					if (Edge(ek).m_gid >= 0)
					{
						if (adj < 2) na[adj] = ek;
						adj++;
					}
				}
				if (adj == 2)
				{
					assert((na[0] == i) || (na[1] == i));
					int nk = na[0];
					if (nk == i) nk = na[1]; assert(nk != i);
					assert(Edge(nk).m_gid >= 0);
					edge.m_nbr[j] = nk;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FSSurfaceMesh::UpdateFaceNeighbors()
{
	// build the node-face table
	FSNodeFaceList NFT;
	NFT.Build(this);

	// find all face neighbours
	int NF = Faces();
	for (int i = 0; i<NF; ++i)
	{
		FSFace& f = Face(i);

		int nf = f.Nodes();
		int n = f.Edges();
		for (int j = 0; j<n; ++j)
		{
			int n1 = f.n[j];
			int n2 = f.n[(j + 1) % n];
			int nval = NFT.Valence(n1);
			f.m_nbr[j] = -1;
			for (int k = 0; k<nval; ++k)
			{
				if (i != NFT.FaceIndex(n1, k))
				{
					FSFace& f2 = *NFT.Face(n1, k);
					if (f2.HasEdge(n1, n2))
					{
						f.m_nbr[j] = NFT.FaceIndex(n1, k);
						break;
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Rebuilds the edge arrays for each face
void FSSurfaceMesh::UpdateFaceEdges()
{
	// build the node edge table
	FSNodeEdgeList NET(this);

	// loop over all faces
	for (int i=0; i<Faces(); ++i)
	{
		FSFace& face = Face(i);

		// clear the edges
		int ne = face.Edges();
		for (int j=0; j<ne; ++j) face.m_edge[j] = -1;

		// find the edges
		for (int j=0; j<ne; ++j)
		{
			FSEdge ej = face.GetEdge(j);

			int val = NET.Edges(ej.n[0]);
			for (int k=0; k<val; ++k)
			{
				int ek = NET.EdgeIndex(ej.n[0], k);
				FSEdge* pek = EdgePtr(ek);

				if (ej == *pek)
				{
					face.m_edge[j] = ek;
					break;
				}
			}

			assert(face.m_edge[j] != -1);
		}
	}
}

//-----------------------------------------------------------------------------
void FSSurfaceMesh::UpdateFaces()
{
	// rebuild the face neighbors
	UpdateFaceNeighbors();

	// rebuild the face edges
	UpdateFaceEdges();
}

//-----------------------------------------------------------------------------
// Builds the edges of the mesh.
// This also creates an initial partition
void FSSurfaceMesh::BuildEdges()
{
	// clear all edges
	m_Edge.clear();

	// tag faces
	for (int i=0; i<Faces(); ++i) Face(i).m_ntag = i;

	// count edges
	int nedges = 0;
	for (int i=0; i<Faces(); ++i)
	{
		FSFace& face = Face(i);
		int ne = face.Edges();
		for (int j=0; j<ne; ++j)
		{
			if (face.m_nbr[j] == -1) nedges++;
			else if (Face(face.m_nbr[j]).m_ntag > face.m_ntag) nedges++;
		}
	}

	// allocate edges
	m_Edge.resize(nedges);

	// create edges
	nedges = 0;
	for (int i = 0; i<Faces(); ++i)
	{
		FSFace& face = Face(i);
		int ne = face.Edges();
		for (int j = 0; j<ne; ++j)
		{
			FSFace* fj = (face.m_nbr[j] == -1 ? 0 : FacePtr(face.m_nbr[j]));
			if ((fj == 0) || (fj->m_ntag > face.m_ntag))
			{
				FSEdge edge = face.GetEdge(j);
				edge.m_gid = (fj == 0 ? 0 : (fj->m_gid != face.m_gid ? 0 : -1));
				edge.m_face[0] = i;
				face.m_edge[j] = nedges;
				m_Edge[nedges++] = edge;
			}
			else if (fj && (fj->m_ntag < face.m_ntag))
			{
				FSEdge edge = face.GetEdge(j);
				int m = fj->FindEdge(edge);
				assert(m != -1);
				face.m_edge[j] = fj->m_edge[m];
				Edge(fj->m_edge[m]).m_face[1] = i;
			}
			assert(face.m_edge[j] != -1);
		}
	}
}

//-----------------------------------------------------------------------------
// Partition the selected edges
//
void FSSurfaceMesh::PartitionEdgeSelection(int partition)
{
	// get the current number of partitions
	int nsg = CountEdgePartitions();

	if (partition < 0) partition = nsg;

	// assign new partition
	int N = Edges();
	for (int i = 0; i<N; ++i)
	{
		FSEdge& e = Edge(i);
		if (e.IsSelected())
		{
			e.m_gid = partition;
		}
	}

	// since this may have deleted partitions, update the partitions
	UpdateEdgePartitions();

	// we may have created more nodes so rebuild the nodes
	AutoPartitionNodes();
}

//-----------------------------------------------------------------------------
void FSSurfaceMesh::PartitionNodeSelection()
{
	int nsg = CountNodePartitions();
	vector<int> nodes;
	for (int i=0; i<Nodes(); ++i)
	{
		FSNode& node = Node(i);
		if (node.IsSelected() && (node.m_gid == -1))
		{
			node.m_gid = nsg++;
			nodes.push_back(i);
		}
	}

	// see if we need to split an edge
	nsg = CountEdgePartitions();
	FSNodeEdgeList NET(this);
	for (int i=0; i<(int)nodes.size(); ++i)
	{
		// find an edge
		int ni = nodes[i];
		int val = NET.Edges(ni);
		for (int j=0; j<val; ++j)
		{
			int eid = NET.EdgeIndex(ni, j);
			FSEdge& edge = Edge(eid);
			if (edge.m_gid >= 0)
			{
				int gid = edge.m_gid;

				FSEdge* pe = 0;
				do
				{
					int val = NET.Edges(ni);
					for (int k=0; k<val; ++k)
					{
						FSEdge& ek = Edge(NET.EdgeIndex(ni, k));
						if ((&ek != pe) && (ek.m_gid == gid))
						{
							ek.m_gid = nsg;

							if      (ek.n[0] == ni) ni = ek.n[1];
							else if (ek.n[1] == ni) ni = ek.n[0];
							else assert(false);

							pe = &ek;
							break;
						}
					}
				}
				while (Node(ni).m_gid == -1);

				break;
			}
		}

		nsg++;
	}
}

//-----------------------------------------------------------------------------
// count node partitions
int FSSurfaceMesh::CountNodePartitions() const
{
	int max_gid = -1;
	for (int i = 0; i<Nodes(); ++i)
	{
		const FSNode& node = Node(i);
		if (node.m_gid > max_gid) max_gid = node.m_gid;
	}
	return max_gid + 1;
}

//-----------------------------------------------------------------------------
int FSSurfaceMesh::CountEdgePartitions() const
{
	int max_gid = -1;
	for (int i = 0; i<Edges(); ++i)
	{
		const FSEdge& edge = Edge(i);
		if (edge.m_gid > max_gid) max_gid = edge.m_gid;
	}
	return max_gid + 1;
}

//-----------------------------------------------------------------------------
int FSSurfaceMesh::CountFacePartitions() const
{
	int max_gid = -1;
	for (int i = 0; i<Faces(); ++i)
	{
		const FSFace& face = Face(i);
		if (face.m_gid > max_gid) max_gid = face.m_gid;
	}
	return max_gid + 1;
}

//-----------------------------------------------------------------------------
int FSSurfaceMesh::CountSmoothingGroups() const
{
	int max_sg = -1;
	for (int i = 0; i<Faces(); ++i)
	{
		const FSFace& face = Face(i);
		if (face.m_sid > max_sg) max_sg = face.m_sid;
	}
	return max_sg + 1;
}

//-----------------------------------------------------------------------------
// This functions update the node GIds to make sure that no indices are skipped.
// This needs to be called after the number of nodes changes.
void FSSurfaceMesh::UpdateNodePartitions()
{
	// find the largest GID
	int max_gid = -1;
	for (int i = 0; i<Nodes(); ++i)
	{
		FSNode& node = Node(i);
		if (node.m_gid > max_gid) max_gid = node.m_gid;
	}

	// if no node has a GID we are done
	if (max_gid < 0) return;

	// build a GID lookup table
	vector<int> gid(max_gid + 1, -1);
	for (int i = 0; i<Nodes(); ++i)
	{
		FSNode& node = Node(i);
		if (node.m_gid >= 0) gid[node.m_gid] = 1;
	}

	// assign new GIDs
	int n = 0;
	for (int i = 0; i<gid.size(); ++i)
	{
		if (gid[i] != -1) gid[i] = n++;
	}

	// only update node GIDs when necessary
	if (n < gid.size())
	{
		for (int i = 0; i<Nodes(); ++i)
		{
			FSNode& node = Node(i);
			if (node.m_gid >= 0) node.m_gid = gid[node.m_gid];
		}
	}
}

//-----------------------------------------------------------------------------
// This functions update the edge GIds to make sure that no indices are skipped.
// This needs to be called after the number of edges changes.
void FSSurfaceMesh::UpdateEdgePartitions()
{
	// find the largest GID
	int max_gid = -1;
	for (int i = 0; i<Edges(); ++i)
	{
		FSEdge& edge = Edge(i);
		if (edge.m_gid > max_gid) max_gid = edge.m_gid;
	}

	// if no edge has a GID we are done
	if (max_gid < 0) return;

	// build a GID lookup table
	vector<int> gid(max_gid + 1, -1);
	for (int i = 0; i<Edges(); ++i)
	{
		FSEdge& edge = Edge(i);
		if (edge.m_gid >= 0) gid[edge.m_gid] = 1;
	}

	// assign new GIDs
	int n = 0;
	for (int i = 0; i<gid.size(); ++i)
	{
		if (gid[i] != -1) gid[i] = n++;
	}

	// only update node GIDs when necessary
	if (n < gid.size())
	{
		for (int i = 0; i<Edges(); ++i)
		{
			FSEdge& edge = Edge(i);
			if (edge.m_gid >= 0) edge.m_gid = gid[edge.m_gid];
		}
	}
}

//-----------------------------------------------------------------------------
// This functions update the face GIds to make sure that no indices are skipped.
// This needs to be called after the number of faces changes.
void FSSurfaceMesh::UpdateFacePartitions()
{
	// find the largest GID
	int max_gid = -1;
	for (int i = 0; i<Faces(); ++i)
	{
		FSFace& face = Face(i);
		if (face.m_gid > max_gid) max_gid = face.m_gid;
	}

	// if no face has a GID we are done
	if (max_gid < 0) return;

	// build a GID lookup table
	vector<int> gid(max_gid + 1, -1);
	for (int i = 0; i<Faces(); ++i)
	{
		FSFace& face = Face(i);
		if (face.m_gid >= 0) gid[face.m_gid] = 1;
	}

	// assign new GIDs
	int n = 0;
	for (int i = 0; i<gid.size(); ++i)
	{
		if (gid[i] != -1) gid[i] = n++;
	}

	// only update node GIDs when necessary
	if (n < gid.size())
	{
		for (int i = 0; i<Faces(); ++i)
		{
			FSFace& face = Face(i);
			if (face.m_gid >= 0) face.m_gid = gid[face.m_gid];
		}
	}
}

//-----------------------------------------------------------------------------
// This functions update the face smoothing Ids to make sure that no indices are skipped.
// This needs to be called after the number of faces changes.
void FSSurfaceMesh::UpdateSmoothingGroups()
{
	// find the largest SG
	int max_sg = -1;
	for (int i = 0; i<Faces(); ++i)
	{
		FSFace& face = Face(i);
		if (face.m_sid > max_sg) max_sg = face.m_sid;
	}

	// if no face has a GID we are done
	if (max_sg < 0) return;

	// build a SID lookup table
	vector<int> sg(max_sg + 1, -1);
	for (int i = 0; i<Faces(); ++i)
	{
		FSFace& face = Face(i);
		if (face.m_sid >= 0) sg[face.m_sid] = 1;
	}

	// assign new SIDs
	int n = 0;
	for (int i = 0; i<sg.size(); ++i)
	{
		if (sg[i] != -1) sg[i] = n++;
	}

	// only update node GIDs when necessary
	if (n < sg.size())
	{
		for (int i = 0; i<Faces(); ++i)
		{
			FSFace& face = Face(i);
			if (face.m_sid >= 0) face.m_sid = sg[face.m_sid];
		}
	}
}


//-----------------------------------------------------------------------------
// Delete selected nodes
void FSSurfaceMesh::DeleteSelectedNodes()
{
	// tag all selected nodes
	for (int i = 0; i<Nodes(); ++i)
	{
		FSNode& node = Node(i);
		node.m_ntag = (node.IsSelected() ? 1 : 0);
	}

	// delete tagged nodes
	DeleteTaggedNodes(1);
}

//-----------------------------------------------------------------------------
// Delete selected edges
void FSSurfaceMesh::DeleteSelectedEdges()
{
	// tag all selected edges
	for (int i = 0; i<Edges(); ++i)
	{
		FSEdge& edge = Edge(i);
		edge.m_ntag = (edge.IsSelected() ? 1 : 0);
	}

	// delete tagged edges
	DeleteTaggedEdges(1);
}

//-----------------------------------------------------------------------------
// Delete selected faces
void FSSurfaceMesh::DeleteSelectedFaces()
{
	// tag all selected faces
	for (int i = 0; i<Faces(); ++i)
	{
		FSFace& face = Face(i);
		face.m_ntag = (face.IsSelected() ? 1 : 0);
	}

	// delete tagged faces
	DeleteTaggedFaces(1);
}

//-----------------------------------------------------------------------------
// This function deletes tagged nodes by deleting all the faces that contain this node
void FSSurfaceMesh::DeleteTaggedNodes(int tag)
{
	// tag all the elements that has this node
	for (int i = 0; i<Faces(); ++i)
	{
		FSFace& face = Face(i);
		face.m_ntag = 0;
		int nf = face.Nodes();
		for (int j = 0; j<nf; ++j)
		{
			if (Node(face.n[j]).m_ntag == tag)
			{
				face.m_ntag = 1;
				break;
			}
		}
	}

	// now delete all the tagged faces
	DeleteTaggedFaces(1);
}

//-----------------------------------------------------------------------------
// This function deletes tagged edges by deleting all the faces that contain this edge
void FSSurfaceMesh::DeleteTaggedEdges(int tag)
{
	// tag all elements that reference a tagged edge
	for (int i = 0; i<Faces(); ++i)
	{
		FSFace& face = Face(i);
		face.m_ntag = 0;
		int ne = face.Edges();
		for (int j = 0; j<ne; ++j)
		{
			if (Edge(face.m_edge[j]).m_ntag == tag)
			{
				face.m_ntag = 1;
				break;
			}
		}
	}

	// now delete all the tagged faces
	DeleteTaggedFaces(1);
}

//-----------------------------------------------------------------------------
void FSSurfaceMesh::DeleteTaggedFaces(int tag)
{
	// Create a face index table
	int NF = Faces();
	vector<int> index(NF, -1);
	int c = 0;
	for (int i=0; i<NF; ++i)
	{
		FSFace& face = Face(i);
		if (face.m_ntag != tag) index[i] = c++;
	}

	// remove references to faces that are about to be deleted.
	for (int i=0; i<NF; ++i)
	{
		FSFace& face = Face(i);
		if (face.m_ntag == tag)
		{
			int ne = face.Edges();
			for (int j=0; j<ne; ++j)
			{
				FSEdge& edge = Edge(face.m_edge[j]);
				if (edge.m_face[0] == i)
				{
					edge.m_face[0] = edge.m_face[1];
					edge.m_face[1] = -1;
				}
				else if (edge.m_face[1] == i)
				{
					edge.m_face[1] = -1;
				}
			}
		}
	}

	// remove tagged faces
	RemoveFaces(tag);

	// update face neighbor indices
	NF = Faces();
	for (int i=0; i<NF; ++i)
	{
		FSFace& face = Face(i);
		for (int j=0; j<face.Edges(); ++j)
		{
			if (face.m_nbr[j] != -1) 
			{
				face.m_nbr[j] = index[face.m_nbr[j]];
			}
		}
	}

	// update face references in edges
	int NE = Edges();
	for (int i=0; i<NE; ++i)
	{
		FSEdge& edge = Edge(i);
		if (edge.m_face[0] >= 0) edge.m_face[0] = index[edge.m_face[0]];
		if (edge.m_face[1] >= 0) edge.m_face[1] = index[edge.m_face[1]];
	}

	// remove isolated edges
	RemoveIsolatedEdges();

	// remove isolated nodes
	RemoveIsolatedNodes();

	// update faces
	UpdateFacePartitions();
	UpdateSmoothingGroups();
	AutoPartitionEdges();
	UpdateEdgeNeighbors();
	AutoPartitionNodes();

	// recalculate the box
	UpdateBoundingBox();

	// and we're done!	
}

//-----------------------------------------------------------------------------
// Remove nodes that are not referenced by any faces.
void FSSurfaceMesh::RemoveIsolatedNodes()
{
	// find the isolated nodes
	TagAllNodes(-1);
	for (int i = 0; i<Faces(); ++i)
	{
		FSFace& face = Face(i);
		int n = face.Nodes();
		for (int j = 0; j<n; ++j) Node(face.n[j]).m_ntag = 1;
	}
	for (int i = 0; i<Edges(); ++i)
	{
		FSEdge& edge = Edge(i);
		int n = edge.Nodes();
		for (int j = 0; j<n; ++j) Node(edge.n[j]).m_ntag = 1;
	}

	// reindex the nodes
	int n = 0;
	for (int i = 0; i<Nodes(); ++i)
	{
		FSNode& node = Node(i);
		if (node.m_ntag == 1) node.m_ntag = n++;
	}

	// fix face node numbering
	for (int i = 0; i<Faces(); ++i)
	{
		FSFace& face = Face(i);
		int n = face.Nodes();
		for (int j = 0; j<n; ++j) face.n[j] = Node(face.n[j]).m_ntag;
	}

	// fix edge node numbering
	for (int i = 0; i<Edges(); ++i)
	{
		FSEdge& edge = Edge(i);
		int n = edge.Nodes();
		for (int j = 0; j<n; ++j) edge.n[j] = Node(edge.n[j]).m_ntag;
	}

	// remove the isolated nodes
	n = 0;
	for (int i = 0; i<Nodes(); ++i)
	{
		FSNode& n1 = Node(i);
		FSNode& n2 = Node(n);

		if (n1.m_ntag >= 0)
		{
			n2 = n1;
			++n;
		}
	}

	// adjust the node container size
	m_Node.resize(n);

	// If the deleted nodes had GIDs set, we need to readjust the GIDs.
	UpdateNodePartitions();
}

//-----------------------------------------------------------------------------
// Remove edges that are not referenced by any faces.
void FSSurfaceMesh::RemoveIsolatedEdges()
{
	// find the isolated edges
	TagAllEdges(-1);
	for (int i = 0; i<Faces(); ++i)
	{
		FSFace& face = Face(i);
		int n = face.Edges();
		for (int j = 0; j<n; ++j) Edge(face.m_edge[j]).m_ntag = 1;
	}

	// reindex the edges
	int n = 0;
	for (int i = 0; i<Edges(); ++i)
	{
		FSEdge& edge = Edge(i);
		if (edge.m_ntag == 1) edge.m_ntag = n++;
	}

	// fix face edge numbering
	for (int i = 0; i<Faces(); ++i)
	{
		FSFace& face = Face(i);
		int n = face.Edges();
		for (int j = 0; j<n; ++j) face.m_edge[j] = Edge(face.m_edge[j]).m_ntag;
	}

	// update edge neighbors
	for (int i = 0; i < Edges(); ++i)
	{
		FSEdge& e = Edge(i);
		if (e.m_ntag >= 0)
		{
			if (e.m_nbr[0] >= 0) e.m_nbr[0] = Edge(e.m_nbr[0]).m_ntag;
			if (e.m_nbr[1] >= 0) e.m_nbr[1] = Edge(e.m_nbr[1]).m_ntag;
		}
	}

	// remove the isolated edges
	n = 0;
	for (int i = 0; i<Edges(); ++i)
	{
		FSEdge& e1 = Edge(i);
		FSEdge& e2 = Edge(n);

		if (e1.m_ntag >= 0)
		{
			e2 = e1;
			++n;
		}
	}

	// adjust the edge container size
	m_Edge.resize(n);

	// If the deleted edges had GIDs set, we need to readjust the GIDs.
	UpdateEdgePartitions();
}


//-----------------------------------------------------------------------------
// Assign gids to the nodes based on the edge gids.
void FSSurfaceMesh::AutoPartitionNodes()
{
	// loop over all edges
	vector<int> tag(Nodes(), -1);
	for (int i = 0; i<Edges(); ++i)
	{
		FSEdge& edge = Edge(i);
		if (edge.m_gid >= 0)
		{
			if (edge.m_nbr[0] == -1) tag[edge.n[0]] = 1;
			if (edge.m_nbr[1] == -1) tag[edge.n[1]] = 1;
		}
	}

	int ng = 0;
	for (int i=0; i<Nodes(); ++i)
	{
		FSNode& node = Node(i);
		if (tag[i] != -1)
		{
			node.m_gid = ng++;
		}
		else node.m_gid = -1;
	}
}

//-----------------------------------------------------------------------------
Mesh_Data& FSSurfaceMesh::GetMeshData()
{
	return m_data;
}

//-----------------------------------------------------------------------------
void FSSurfaceMesh::ResizeNodes(int newSize)
{
	m_Node.resize(newSize);
}

//-----------------------------------------------------------------------------
void FSSurfaceMesh::ResizeEdges(int newSize)
{
	m_Edge.resize(newSize);
}

//-----------------------------------------------------------------------------
void FSSurfaceMesh::ResizeFaces(int newSize)
{
	m_Face.resize(newSize);
}

//-----------------------------------------------------------------------------
void FSSurfaceMesh::RemoveDuplicateEdges()
{
	// clear tags
	TagAllEdges(0);

	int ng = CountEdgePartitions();

	// loop over all edges
	int NE = Edges();
	for (int i = 0; i<NE; ++i)
	{
		FSEdge& ei = Edge(i);
		for (int j = i + 1; j<NE; ++j)
		{
			FSEdge& ej = Edge(j);
			if (ei == ej)
			{
				if (ei.m_gid == -1)
				{
					ei.m_ntag = 1;
					ej.m_gid += ng;
				}
				else if (ej.m_gid == -1)
				{
					ej.m_ntag = 1;
					ei.m_gid += ng;
				}
				else
				{
					ei.m_ntag = 1;
					ej.m_gid = (ei.m_gid * ng + ej.m_gid);
				}
			}
		}
	}

	RemoveEdges(1);
	UpdateEdgeNeighbors();
	UpdateEdgePartitions();
}

//-----------------------------------------------------------------------------
void FSSurfaceMesh::Attach(const FSSurfaceMesh& mesh)
{
	// get counts
	int nn0 = Nodes();
	int nn1 = mesh.Nodes();

	int nl0 = Edges();
	int nl1 = mesh.Edges();

	int nf0 = Faces();
	int nf1 = mesh.Faces();

	// new total counts
	int nodes = nn0 + nn1;
	int faces = nf0 + nf1;
	int edges = nl0 + nl1;

	// get the owning objects (for converting between coordinate systems)
	// TODO: I would like to get rid of this. I could pass the transform objects as parameters. Alternatively,
	//       the mesh parameter that gets passed could already be in the correct coordinate system.
	GObject* po1 = m_pobj;
	GObject* po2 = mesh.m_pobj;

	// create the nodes
	if (nodes > 0)
	{
		// find the largest GID
		int ng = CountNodePartitions();

		// resize nodes and copy node data
		m_Node.resize(nodes);
		for (int i = 0; i<nn1; ++i)
		{
			FSNode& n0 = m_Node[nn0 + i];
			const FSNode& n1 = mesh.m_Node[i];
			n0 = n1;
			if (n0.m_gid >= 0) n0.m_gid = n1.m_gid + ng;
			if (po2) n0.r = po1->GetTransform().GlobalToLocal(po2->GetTransform().LocalToGlobal(n1.r));
			else n0.r = n1.r;
		}
	}

	// create the edges
	if (edges > 0)
	{
		// find the largest GID
		int ng = CountEdgePartitions();

		// add new edges
		m_Edge.resize(edges);
		for (int i = 0; i<nl1; ++i)
		{
			FSEdge& l0 = m_Edge[nl0 + i];
			const FSEdge& l1 = mesh.m_Edge[i];

			l0 = l1;

			if (l0.m_gid >= 0) l0.m_gid = l1.m_gid + ng;

			int ne = l1.Nodes();
			for (int j = 0; j<ne; ++j) l0.n[j] = nn0 + l1.n[j];

			l0.m_nbr[0] = (l1.m_nbr[0] >= 0 ? l1.m_nbr[0] + nl0 : -1);
			l0.m_nbr[1] = (l1.m_nbr[1] >= 0 ? l1.m_nbr[1] + nl0 : -1);
		}
	}

	// create the faces
	if (faces > 0)
	{
		// find the largest GID
		int ng = CountFacePartitions();

		int nsg = CountSmoothingGroups();

		// add new faces
		// TODO: Do we need to update the smoothing IDs as well?
		m_Face.resize(faces);
		for (int i = 0; i<nf1; ++i)
		{
			FSFace& f0 = m_Face[nf0 + i];
			const FSFace& f1 = mesh.m_Face[i];
			f0 = f1;
			f0.m_gid = f1.m_gid + ng;
			f0.m_sid = f1.m_sid + nsg;

			for (int j = 0; j<f0.Nodes(); ++j) f0.n[j] = f1.n[j] + nn0;

			f0.m_nbr[0] = f1.m_nbr[0] + nf0;
			f0.m_nbr[1] = f1.m_nbr[1] + nf0;
			f0.m_nbr[2] = f1.m_nbr[2] + nf0;
			f0.m_nbr[3] = (f1.m_nbr[3] >= 0 ? f1.m_nbr[3] + nf0 : -1);
		}
	}

	// update the mesh
	UpdateMesh();
}

//-----------------------------------------------------------------------------
void FSSurfaceMesh::AttachAndWeld(const FSSurfaceMesh& mesh, double weldTolerance)
{
	// get the owning objects (for converting between coordinate systems)
	// TODO: I would like to get rid of this. I could pass the transform objects as parameters. Alternatively,
	//       the mesh parameter that gets passed could already be in the correct coordinate system.
	GObject* po1 = m_pobj;
	GObject* po2 = mesh.m_pobj;

	// node count
	int NN0 = Nodes();
	int NN1 = mesh.Nodes();

	// first create a list of the boundary nodes for this mesh
	// to speed up the search
	vector<pair<int, vec3d> > nodeList0;
	vector<int> tag(NN0, 0);
	for (int i=0; i<Edges(); ++i)
	{
		FSEdge& edge = Edge(i);
		if (edge.m_gid >= 0) 
		{
			tag[edge.n[0]] = 1;
			tag[edge.n[1]] = 1;
		}
	}
	for (int i=0; i<NN0; ++i)
	{
		if (tag[i] == 1)
		{
			FSNode& node = Node(i);
			nodeList0.push_back(pair<int, vec3d>(i, node.r));
		}
	}

	// make sure there is something to do
	if (nodeList0.empty())
	{
		// no need to weld, so just do a regular attach
		Attach(mesh);
		return;
	}

	// now we need to figure out which nodes will get welded
	// first, we tag all the edge nodes (only edge nodes will be welded)
	tag.resize(NN1, 0);
	for (int i=0; i<mesh.Edges(); ++i)
	{
		const FSEdge& edge = mesh.Edge(i);
		if (edge.m_gid >= 0)
		{
			tag[edge.n[0]] = 1;
			tag[edge.n[1]] = 1;
		}
	}

	// if a node must be welded, we'll set their index in the tag list to the welded node index
	int newNodes = NN0;
	for (int i = 0; i<NN1; ++i)
	{
		if (tag[i] == 1)
		{
			double Dmin = 1e99;
			int jmin = -1;
			const FSNode& nodei = mesh.Node(i);
			vec3d ri;
			if (po2) ri = po1->GetTransform().GlobalToLocal(po2->GetTransform().LocalToGlobal(nodei.r));
			else ri = nodei.r;

			for (size_t j=0; j<nodeList0.size(); ++j)
			{
				vec3d& rj = nodeList0[j].second;

				double D2 = (ri - rj).SqrLength();
				if (D2 < Dmin)
				{
					Dmin = D2;
					jmin = nodeList0[j].first;
				}
			}

			if (Dmin < weldTolerance*weldTolerance)
			{
				tag[i] = jmin;
			}
			else tag[i] = newNodes++;
		}
		else tag[i] = newNodes++;
	}

	// allocate nodes
	Create(newNodes, 0, 0);

	// find the largest GID
	int ng0 = CountNodePartitions();
	int ng1 = mesh.CountNodePartitions();

	// create the new nodes
	for (int i=0; i<NN1; i++)
	{
		const FSNode& node1 = mesh.Node(i);
		if (tag[i] >= NN0)
		{
			FSNode& node0 = Node(tag[i]);
			node0 = node1;

			if (node1.m_gid >= 0) node0.m_gid = node1.m_gid + ng0;
			if (po2) node0.r = po1->GetTransform().GlobalToLocal(po2->GetTransform().LocalToGlobal(node1.r));
			else node0.r = node1.r;
		}
		else
		{
			FSNode& node0 = Node(tag[i]);
			if ((node1.m_gid >= 0) && (node0.m_gid == -1)) node0.m_gid = node1.m_gid + ng0;
		}
	}

	// --- Merge edges ---
	int NE0 = Edges();
	int NE1 = mesh.Edges();

	// count the edges that will be added 
	int newEdges = NE0;
	for (int i=0; i<mesh.Edges(); ++i)
	{
		const FSEdge& edge = mesh.Edge(i);
		if (edge.m_gid == -1) newEdges++;
		else
		{
			int m0 = tag[edge.n[0]];
			int m1 = tag[edge.n[1]];
			if ((m0>=NN0) || (m1 >= NN0)) newEdges++;
			else
			{
				FSEdge* edge = FindEdge(m0, m1);
				if (edge == 0) newEdges++;
			}
		}
	}

	// allocate edges
	Create(0, newEdges, 0);

	ng0 = CountEdgePartitions();
	ng1 = mesh.CountEdgePartitions();
	int ng = ng0 + ng1;

	// insert new edges
	newEdges = NE0;

	// first copy non-feature edges
	for (int i=0; i<NE1; ++i)
	{
		const FSEdge& edge1 = mesh.Edge(i);
		if (edge1.m_gid == -1)
		{
			int m0 = tag[edge1.n[0]];
			int m1 = tag[edge1.n[1]];
			FSEdge& edge0 = Edge(newEdges++);
			edge0 = edge1;
			edge0.n[0] = m0;
			edge0.n[1] = m1;
		}
	}

	// now do the feature edges
	for (int i=0; i<ng1; ++i)
	{
		for (int j=0; j<NE1; ++j)
		{
			const FSEdge& edge1 = mesh.Edge(j);
			if (edge1.m_gid == i)
			{
				int m0 = tag[edge1.n[0]];
				int m1 = tag[edge1.n[1]];
				if ((m0 >= NN0) || (m1 >= NN0)) 
				{
					FSEdge& edge0 = Edge(newEdges++);
					edge0 = edge1;

					edge0.m_gid = edge1.m_gid + ng0;

					edge0.n[0] = m0;
					edge0.n[1] = m1;
				}
				else
				{
					// we need to mark the original edge, since it could need a new group ID
					// so find this edge
					FSEdge* edge0 = FindEdge(m0, m1);

					if (edge0)
					{
						// set it's GID
						edge0->m_gid = ng;
					}
					else
					{
						FSEdge& edge0 = Edge(newEdges++);
						edge0 = edge1;

						edge0.m_gid = edge1.m_gid + ng0;

						edge0.n[0] = m0;
						edge0.n[1] = m1;
					}
				}
			}
		}

		ng++;
	}
	UpdateEdgePartitions();

	// It's possible that merging the edges created new node partitions, so we need to find them.
	ng = CountNodePartitions();
	vector<int> val(Nodes(), 0);
	for (int i=0; i<Edges(); ++i)
	{
		FSEdge& edge = Edge(i);
		if (edge.m_gid >= 0)
		{
			val[edge.n[0]]++;
			val[edge.n[1]]++;
		}
	}
	for (int i=0; i<Nodes(); ++i)
	{
		if ((val[i] > 0) && (val[i] != 2))
		{
			FSNode& node = Node(i);
			if (node.m_gid == -1) node.m_gid = ng++;
		}
	}
	UpdateNodePartitions();

	// --- Merge faces ---
	int NF0 = Faces();
	int NF1 = mesh.Faces();

	// find the largest GID
	ng = CountFacePartitions();
	int nsg = CountSmoothingGroups();
	// allocate faces
	Create(0, 0, NF0 + NF1);
	for (int i=0; i<NF1; ++i)
	{
		FSFace& face0 = Face(NF0 + i);
		const FSFace& face1 = mesh.Face(i);
		face0 = face1;

		face0.m_gid = face1.m_gid + ng;
		face0.m_sid = face1.m_sid + nsg;

		for (int j = 0; j<face0.Nodes(); ++j) face0.n[j] = tag[face1.n[j]];
	}

	// update additional data structures
	UpdateFaceNeighbors();
//	UpdateEdgeNeighbors();

	UpdateMesh();
}

//-----------------------------------------------------------------------------
void FSSurfaceMesh::AddFacet(int n0, int n1, int n2)
{
	FSFace face;
	face.SetType(FE_FACE_TRI3);
	face.m_gid = 0;
	face.n[0] = n0; assert(n0 < Nodes());
	face.n[1] = n1; assert(n1 < Nodes());
	face.n[2] = n2; assert(n2 < Nodes());
	m_Face.push_back(face);
}

//-----------------------------------------------------------------------------
void FSSurfaceMesh::Save(OArchive& ar)
{
	int nodes = Nodes();
	int faces = Faces();
	int edges = Edges();

	// write the header
	ar.BeginChunk(CID_MESH_HEADER);
	{
		ar.WriteChunk(CID_MESH_NODES, nodes);
		ar.WriteChunk(CID_MESH_FACES, faces);
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

	// write the faces
	ar.BeginChunk(CID_MESH_FACE_SECTION);
	{
		FSFace* pf = FacePtr();
		for (int i = 0; i<faces; ++i, ++pf)
		{
			ar.BeginChunk(CID_MESH_FACE);
			{
				int nn = pf->Nodes();
				int ntype = pf->Type();
				assert(ntype != FE_FACE_INVALID_TYPE);
				ar.WriteChunk(CID_MESH_FACE_TYPE, ntype);
				ar.WriteChunk(CID_MESH_FACE_GID, pf->m_gid);
				ar.WriteChunk(CID_MESH_FACE_NODES, pf->n, pf->Nodes());
				ar.WriteChunk(CID_MESH_FACE_EDGES, pf->m_edge, pf->Edges());
				ar.WriteChunk(CID_MESH_FACE_SMOOTHID, pf->m_sid);
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
void FSSurfaceMesh::Load(IArchive& ar)
{
	TRACE("FSSurfaceMesh::Load");

	// the first chunk must be the header
	ar.OpenChunk();
	if (ar.GetChunkID() != CID_MESH_HEADER) throw ReadError("Missing mesh header");

	// read the header
	int nodes, faces, edges;
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_MESH_NODES: ar.read(nodes); break;
		case CID_MESH_FACES: ar.read(faces); break;
		case CID_MESH_EDGES: ar.read(edges); break;
		}
		ar.CloseChunk();
	}
	ar.CloseChunk();

	// allocate storage
	Create(nodes, edges, faces);

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
		case CID_MESH_FACE_SECTION:
		{
			int n = 0;
			FSFace* pf = FacePtr();
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				int nid = ar.GetChunkID();
				if (nid != CID_MESH_FACE) throw ReadError("error parsing CID_MESH_FACE_SECTION");

				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int nid = ar.GetChunkID();
					int ntype;
					switch (nid)
					{
					case CID_MESH_FACE_TYPE:
					{
						ar.read(ntype);
						switch (ntype)
						{
						case FE_FACE_TRI3 : pf->SetType(FE_FACE_TRI3 ); break;
						case FE_FACE_QUAD4: pf->SetType(FE_FACE_QUAD4); break;
						case FE_FACE_TRI6 : pf->SetType(FE_FACE_TRI6 ); break;
						case FE_FACE_QUAD8: pf->SetType(FE_FACE_QUAD8); break;
						case FE_FACE_TRI7 : pf->SetType(FE_FACE_TRI7 ); break;
						case FE_FACE_QUAD9: pf->SetType(FE_FACE_QUAD9); break;
						case FE_FACE_TRI10: pf->SetType(FE_FACE_TRI10); break;
						default:
							assert(false);
							throw ReadError("Unsupported face type in CID_MESH_FACE_SECTION.");
						}
					}
					break;
					case CID_MESH_FACE_GID     : ar.read(pf->m_gid); break;
					case CID_MESH_FACE_NODES   : ar.read(pf->n, pf->Nodes()); break;
					case CID_MESH_FACE_EDGES   : ar.read(pf->m_edge, pf->Edges()); break;
					case CID_MESH_FACE_SMOOTHID: ar.read(pf->m_sid); break;
					}
					ar.CloseChunk();
				}

				assert(n < faces);
				++pf;
				++n;

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
						ar.read(pe->n, nn); break;
					}
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
	UpdateFaceNeighbors();
	UpdateEdgeNeighbors();

	UpdateMesh();
}

// Create a TriMesh from a surface mesh
void BuildTriMesh(TriMesh& dyna, FSSurfaceMesh* pm)
{
	// build the nodes
	int NN = pm->Nodes();
	vector<TriMesh::NODEP> nodePtr;
	for (int i = 0; i<NN; ++i)
	{
		FSNode& node = pm->Node(i);
		TriMesh::NODEP n = dyna.addNode(node.r);
		n->ntag = i;
		n->gid = node.m_gid;
		nodePtr.push_back(n);
	}

	// build the edges
	int NE = pm->Edges();
	vector<TriMesh::EDGEP> edgePtr;
	for (int i = 0; i<NE; ++i)
	{
		FSEdge& edge = pm->Edge(i);
		TriMesh::EDGEP e = dyna.addEdge(nodePtr[edge.n[0]], nodePtr[edge.n[1]]);
		e->gid = edge.m_gid;
		edgePtr.push_back(e);
	}

	// build the faces
	int NF = pm->Faces();
	for (int i = 0; i<NF; ++i)
	{
		FSFace& face = pm->Face(i);
		TriMesh::FACE f;
		f.ntag = 0;
		f.node[0] = nodePtr[face.n[0]];
		f.node[1] = nodePtr[face.n[1]];
		f.node[2] = nodePtr[face.n[2]];

		f.edge[0] = edgePtr[face.m_edge[0]];
		f.edge[1] = edgePtr[face.m_edge[1]];
		f.edge[2] = edgePtr[face.m_edge[2]];

		f.gid = face.m_gid;

		// calculate normal
		vec3d r0 = f.node[0]->r;
		vec3d e1 = f.node[1]->r - r0;
		vec3d e2 = f.node[2]->r - r0;
		f.normal = e1 ^ e2;
		f.normal.Normalize();

		dyna.addFace(f);
	}
}

void FSSurfaceMesh::ShowFaces(const vector<int>& face, bool bshow)
{
	if (bshow)
        for (int i : face) Face(i).Show();
	else
        for (int i : face) Face(i).Hide();

	UpdateItemVisibility();
}

void FSSurfaceMesh::ShowAllFaces()
{
	for (int i=0; i<Nodes(); ++i) Node(i).Show();
	for (int i=0; i<Edges(); ++i) Edge(i).Show();
	for (int i=0; i<Faces(); ++i) Face(i).Show();
}

void FSSurfaceMesh::UpdateItemVisibility()
{
	// tag all visible nodes
	TagAllNodes(0);
	for (int i = 0; i<Faces(); ++i)
	{
		FSFace& face = Face(i);
		if (face.IsVisible())
		{
			int nf = face.Nodes();
			for (int j = 0; j<nf; ++j) Node(face.n[j]).m_ntag = 1;
		}
	}

	// update visibility of all other items
	for (int i = 0; i<Nodes(); ++i)
	{
		FSNode& node = Node(i);
		if (node.m_ntag == 1) node.Show(); else node.Hide();
	}

	for (int i = 0; i<Edges(); ++i)
	{
		FSEdge& edge = Edge(i);
		if ((Node(edge.n[0]).m_ntag == 0) || (Node(edge.n[1]).m_ntag == 0)) edge.Hide();
		else edge.Show();
	}
}

void FSSurfaceMesh::SelectFaces(const vector<int>& faceList)
{
    for (int i : faceList) Face(i).Select();
}

//================================================================================
// Is the mesh closed (i.e. do all faces have neighbors)
bool MeshTools::IsMeshClosed(FSSurfaceMesh& m)
{
	int NF = m.Faces();
	for (int i = 0; i < NF; ++i)
	{
		FSFace& f = m.Face(i);
		int ne = f.Edges();
		for (int j = 0; j < ne; ++j)
		{
			if (f.m_nbr[j] == -1) return false;
		}
	}
	return true;
}
