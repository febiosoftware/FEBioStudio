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

#include "FSMeshBuilder.h"
#include "FSMesh.h"
#include <GeomLib/GObject.h>
#include <MeshLib/FSFaceEdgeList.h>
#include <memory>
using namespace std;

FSMeshBuilder::FSMeshBuilder(FSMesh& mesh) : m_mesh(mesh)
{

}

//-----------------------------------------------------------------------------
FSNode* FSMeshBuilder::AddNode(const vec3d& r)
{
	// create a new node
	FSNode node;
	node.r = r;
	node.SetExterior(true);	// we'll assume this node is not attached to anything

	// Partition the node
	int ng = m_mesh.CountNodePartitions();
	node.m_gid = ng;

	// add it to the list
	m_mesh.m_Node.push_back(node);

	// Bounding box may have changed
	m_mesh.UpdateBoundingBox();

	return &m_mesh.m_Node[m_mesh.m_Node.size() - 1];
}

// Add a triangle
void FSMeshBuilder::AddTriangle(int n0, int n1, int n2)
{
	FSElement elem;
	elem.SetType(FE_TRI3);
	elem.m_node[0] = n0;
	elem.m_node[1] = n1;
	elem.m_node[2] = n2;
	m_mesh.m_Elem.push_back(elem);
	RebuildMesh();
}

void FSMeshBuilder::AddTriangles(const std::vector<int>& nodes)
{
	if (nodes.size() < 3) return;
	for (int i = 0; i < nodes.size(); i += 3)
	{
		FSElement elem;
		elem.SetType(FE_TRI3);
		elem.m_node[0] = nodes[i  ];
		elem.m_node[1] = nodes[i+1];
		elem.m_node[2] = nodes[i+2];
		m_mesh.m_Elem.push_back(elem);
	}
	RebuildMesh();
}

//-----------------------------------------------------------------------------
// Remove nodes that are not attached to anything
int FSMeshBuilder::RemoveIsolatedNodes()
{
	// find the isolated nodes
	m_mesh.TagAllNodes(-1);
	for (int i = 0; i<m_mesh.Elements(); ++i)
	{
		FSElement& el = m_mesh.Element(i);
		int n = el.Nodes();
		for (int j = 0; j<n; ++j) m_mesh.Node(el.m_node[j]).m_ntag = 1;
	}

	// exclude any required nodes
	for (int i = 0; i < m_mesh.Nodes(); ++i)
	{
		FSNode& node = m_mesh.Node(i);
		if ((node.m_gid>=0) || node.IsRequired()) node.m_ntag = 1;
	}

	// exclude nodes in node sets
	GObject* po = m_mesh.GetGObject();
	if (po)
	{
		for (int i = 0; i < po->FENodeSets(); ++i)
		{
			FSNodeSet* pi = po->GetFENodeSet(i);
			std::vector<int> items = pi->CopyItems();
			for (int j = 0; j < items.size(); ++j)
			{
				FSNode* pn = m_mesh.NodePtr(items[j]);
				pn->m_ntag = 1;
			}
		}
	}

	// reindex the nodes
	int n = 0;
	for (int i = 0; i<m_mesh.Nodes(); ++i)
	{
		FSNode& node = m_mesh.Node(i);
		if (node.m_ntag == 1) node.m_ntag = n++;
	}

	// fix element node numbering
	for (int i = 0; i<m_mesh.Elements(); ++i)
	{
		FSElement& el = m_mesh.Element(i);
		int n = el.Nodes();
		for (int j = 0; j<n; ++j) el.m_node[j] = m_mesh.Node(el.m_node[j]).m_ntag;
	}

	// fix face node numbering
	for (int i = 0; i<m_mesh.Faces(); ++i)
	{
		FSFace& face = m_mesh.Face(i);
		int n = face.Nodes();
		for (int j = 0; j<n; ++j) face.n[j] = m_mesh.Node(face.n[j]).m_ntag;
	}

	// fix edge node numbering
	for (int i = 0; i<m_mesh.Edges(); ++i)
	{
		FSEdge& edge = m_mesh.Edge(i);
		int n = edge.Nodes();
		for (int j = 0; j<n; ++j) edge.n[j] = m_mesh.Node(edge.n[j]).m_ntag;
	}

	// fix node numbers in node sets
	if (po)
	{
		for (int i = 0; i < po->FENodeSets(); ++i)
		{
			FSNodeSet* pi = po->GetFENodeSet(i);
			std::vector<int> items = pi->CopyItems();
			for (int j = 0; j < items.size(); ++j)
			{
				FSNode* pn = m_mesh.NodePtr(items[j]); assert(pn->m_ntag >= 0);
				items[j] = pn->m_ntag;
			}
			pi->clear();
			pi->add(items);
		}
	}

	// remove the isolated nodes
	n = 0;
	int removedNodes = 0;
	for (int i = 0; i<m_mesh.Nodes(); ++i)
	{
		FSNode& n1 = m_mesh.Node(i);
		FSNode& n2 = m_mesh.Node(n);

		if (n1.m_ntag >= 0)
		{
			n2 = n1;
			++n;
		}
		else removedNodes++;
	}

	// adjust the node container size
	m_mesh.m_Node.resize(n);

	// If the deleted nodes had GIDs set, we need to readjust the GIDs.
	m_mesh.UpdateNodePartitions();

	// The bounding box may have changed as well
	m_mesh.UpdateBoundingBox();

	return removedNodes;
}

//-----------------------------------------------------------------------------
// Delete selected nodes
void FSMeshBuilder::DeleteSelectedNodes()
{
	// tag all selected nodes
	for (int i = 0; i<m_mesh.Nodes(); ++i)
	{
		FSNode& node = m_mesh.Node(i);
		node.m_ntag = (node.IsSelected() ? 1 : 0);
	}

	// delete tagged nodes
	DeleteTaggedNodes(1);
}

//-----------------------------------------------------------------------------
// This function deletes tagged nodes by deleting all the elements that contain this node
void FSMeshBuilder::DeleteTaggedNodes(int tag)
{
	// tag all the elements that has this node
	for (int i = 0; i<m_mesh.Elements(); ++i)
	{
		FSElement& el = m_mesh.Element(i);
		el.m_ntag = 0;
		int ne = el.Nodes();
		for (int j = 0; j<ne; ++j)
		{
			if (m_mesh.Node(el.m_node[j]).m_ntag == tag)
			{
				el.m_ntag = 1;
				break;
			}
		}
	}

	// now delete all the tagged elements
	DeleteTaggedElements(1);
}

//-----------------------------------------------------------------------------
// Delete tagged edges
void FSMeshBuilder::DeleteTaggedEdges(int ntag)
{
	// TODO: identify elements which should be deleted instead.
	assert(false);

	DeleteTaggedElements(1);
}


//-----------------------------------------------------------------------------
// Delete selected faces
void FSMeshBuilder::DeleteSelectedFaces()
{
	// tag all selected faces
	for (int i = 0; i<m_mesh.Faces(); ++i)
	{
		FSFace& face = m_mesh.Face(i);
		face.m_ntag = (face.IsSelected() ? 1 : 0);
	}

	// delete tagged faces
	DeleteTaggedFaces(1);

	// make sure none of the faces are selected
	for (int i = 0; i < m_mesh.Faces(); ++i)
		m_mesh.Face(i).Unselect();
}

//-----------------------------------------------------------------------------
// Delete tagged faces
void FSMeshBuilder::DeleteTaggedFaces(int tag)
{
	m_mesh.TagAllElements(0);
	for (int i = 0; i<m_mesh.Faces(); ++i)
	{
		FSFace& face = m_mesh.Face(i);
		if (face.m_ntag == tag)
		{
			if (face.m_elem[0].eid >= 0)
			{
				m_mesh.Element(face.m_elem[0].eid).m_ntag = 1;
			}
			if (face.m_elem[1].eid >= 0)
			{
				m_mesh.Element(face.m_elem[1].eid).m_ntag = 1;
			}
			if (face.m_elem[2].eid >= 0)
			{
				m_mesh.Element(face.m_elem[2].eid).m_ntag = 1;
			}
		}
	}

	DeleteTaggedElements(1);
}

//-----------------------------------------------------------------------------
// Delete selected elements
void FSMeshBuilder::DeleteSelectedElements()
{
	// tag all selected elements
	m_mesh.TagAllElements(0);
	for (int i = 0; i<m_mesh.Elements(); ++i)
		if (m_mesh.Element(i).IsSelected()) m_mesh.Element(i).m_ntag = 1;

	// delete tagged elements
	DeleteTaggedElements(1);
}

//-----------------------------------------------------------------------------
// Delete tagged elements
int FSMeshBuilder::DeleteTaggedElements(int tag)
{
	// Let's go ahead and remove all tagged elements
	int removedElems = m_mesh.RemoveElements(tag);
	if (removedElems > 0)
	{
		m_mesh.UpdateElementPartitions();

		// remove isolated nodes
		RemoveIsolatedNodes();

		// rebuild mesh
		m_mesh.RebuildMesh();
	}
	return removedElems;
}

FSMesh* FSMeshBuilder::DeleteParts(FSMesh& mesh, std::vector<int> partIds)
{
	const int TAG = 1;

	// copy to set for faster lookup
	std::set<int> partset;
	for (int id : partIds) partset.insert(id);
	auto setend = partset.end();

	mesh.TagAllElements(0);
	int NE = mesh.Elements();
	for (int i = 0; i < NE; ++i)
	{
		FSElement& el = mesh.Element(i);
		if (partset.find(el.m_gid) != setend)
		{
			el.m_ntag = TAG;
		}
	}
	return DeleteTaggedParts(mesh, TAG);
}

FSMesh* FSMeshBuilder::DeletePart(FSMesh& mesh, int partId)
{
	const int TAG = 1;
	mesh.TagAllElements(0);
	int NE = mesh.Elements();
	for (int i = 0; i < NE; ++i)
	{
		FSElement& el = mesh.Element(i);
		if (el.m_gid == partId) el.m_ntag = TAG;
	}
	return DeleteTaggedParts(mesh, TAG);
}

FSMesh* FSMeshBuilder::DeleteTaggedParts(FSMesh& oldMesh, int TAG)
{
	FSMesh* newMesh = new FSMesh(oldMesh);
	FSMesh& mesh = *newMesh;

	// First, figure out all nodes that we will have to remove
	// This approach ensures that isolated nodes are not removed
	mesh.TagAllNodes(0);
	int NE = mesh.Elements();
	for (int i = 0; i < NE; ++i)
	{
		FSElement& el = mesh.Element(i);
		if (el.m_ntag == TAG)
		{
			int ne = el.Nodes();
			for (int j = 0; j < ne; ++j)
			{
				FSNode& node = mesh.Node(el.m_node[j]);
				node.m_ntag = -1;
			}
		}
	}
	for (int i = 0; i < NE; ++i)
	{
		FSElement& el = mesh.Element(i);
		if (el.m_ntag != TAG)
		{
			int ne = el.Nodes();
			for (int j = 0; j < ne; ++j) mesh.Node(el.m_node[j]).m_ntag = 0;
		}
	}

	// figure out which faces to remove
	mesh.TagAllFaces(0);
	for (int i = 0; i < mesh.Faces(); ++i)
	{
		FSFace& face = mesh.Face(i);

		FSElement_* pe0 = mesh.ElementPtr(face.m_elem[0].eid);
		if (pe0 == nullptr) { delete newMesh; return nullptr; }
		FSElement_* pe1 = mesh.ElementPtr(face.m_elem[1].eid);
		FSElement_* pe2 = mesh.ElementPtr(face.m_elem[2].eid);

		if ((pe0->m_ntag == TAG) && 
			((pe1 == nullptr) || (pe1->m_ntag == TAG)) && 
			((pe2 == nullptr) || (pe2->m_ntag == TAG)))
		{
			face.m_ntag = TAG;
		}
	}

	// create element-edge list
	EdgeList EL; EL.BuildFromMeshEdges(mesh);
	FSElementEdgeList EEL(mesh, EL);

	// figure out which edges to remove
	mesh.TagAllEdges(0);
	for (int i = 0; i < mesh.Elements(); ++i)
	{
		FSElement& el = mesh.Element(i);
		if (el.m_ntag == TAG)
		{
			int nval = EEL.Valence(i);
			for (int j = 0; j < nval; ++j)
			{
				int nedge = EEL.EdgeIndex(i, j);
				if (nedge >= 0)
				{
					FSEdge& edge = mesh.Edge(nedge);
					edge.m_ntag = TAG;
				}
			}
		}
	}
	for (int i = 0; i < mesh.Elements(); ++i)
	{
		FSElement& el = mesh.Element(i);
		if (el.m_ntag != TAG)
		{
			int nval = EEL.Valence(i);
			for (int j = 0; j < nval; ++j)
			{
				int nedge = EEL.EdgeIndex(i, j);
				if (nedge >= 0)
				{
					FSEdge& edge = mesh.Edge(nedge);
					edge.m_ntag = 0;
				}
			}
		}
	}

	// Let's go ahead and remove all tagged items
	mesh.RemoveElements(TAG);
	mesh.RemoveFaces(TAG);
	mesh.RemoveEdges(TAG);

	// remove tagged nodes
	// note that we do not remove required nodes
	int n = 0;
	int NN = mesh.Nodes();
	for (int i = 0; i < NN; ++i)
	{
		FSNode& node = mesh.Node(i);
		if ((node.m_ntag >= 0) || node.IsRequired()) node.m_ntag = n++;
	}

	// fix element node numbering
	for (int i = 0; i < mesh.Elements(); ++i)
	{
		FSElement& el = mesh.Element(i);
		int n = el.Nodes();
		for (int j = 0; j < n; ++j)
		{
			int nj = mesh.Node(el.m_node[j]).m_ntag;
			if (nj < 0) { delete newMesh; return nullptr; }
			el.m_node[j] = nj;
		}
	}

	// fix face node numbering
	for (int i = 0; i < mesh.Faces(); ++i)
	{
		FSFace& face = mesh.Face(i);
		int n = face.Nodes();
		for (int j = 0; j < n; ++j)
		{
			int nj = mesh.Node(face.n[j]).m_ntag;
			if (nj < 0) { delete newMesh; return nullptr; }
			face.n[j] = nj;
		}
	}

	// fix edge node numbering
	for (int i = 0; i < mesh.Edges(); ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		int n = edge.Nodes();
		for (int j = 0; j < n; ++j)
		{
			int nj = mesh.Node(edge.n[j]).m_ntag;
			if (nj < 0) { delete newMesh; return nullptr; }
			edge.n[j] = nj;
		}
	}

	// remove the nodes
	n = 0;
	for (int i = 0; i < mesh.Nodes(); ++i)
	{
		FSNode& n1 = mesh.Node(i);
		FSNode& n2 = mesh.Node(n);

		if (n1.m_ntag >= 0)
		{
			n2 = n1;
			++n;
		}
	}
	mesh.m_Node.resize(n);

	// update the element neighbours
	mesh.UpdateElementNeighbors();

	// update face data
	mesh.RebuildFaceData();

	// update edge data
	mesh.RebuildEdgeData();

	// It is possible that edges were cut and that new nodes need to be promoted
	int ng = mesh.CountNodePartitions();
	for (int i = 0; i < mesh.Edges(); ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		if (edge.m_gid >= 0)
		{
			for (int j = 0; j < 2; ++j)
			{
				if (edge.m_nbr[j] < 0)
				{
					FSNode& node = mesh.Node(edge.n[j]);
					if (node.m_gid < 0) node.m_gid = ng++;
				}
			}
		}
	}

	// update node data
	mesh.RebuildNodeData();

	// update the mesh
	mesh.UpdateMesh();

	return newMesh;
}

//-----------------------------------------------------------------------------
// Attach another to this mesh.
//
void FSMeshBuilder::Attach(FSMesh& fem)
{
	int i, j, n;

	int nn0 = m_mesh.Nodes();
	int nn1 = fem.Nodes();

	int ne0 = m_mesh.Elements();
	int ne1 = fem.Elements();

	int nf0 = m_mesh.Faces();
	int nf1 = fem.Faces();

	int nl0 = m_mesh.Edges();
	int nl1 = fem.Edges();

	int nodes = nn0 + nn1;
	int elems = ne0 + ne1;
	int faces = nf0 + nf1;
	int edges = nl0 + nl1;

	GObject* po1 = m_mesh.GetGObject();
	GObject* po2 = fem.m_pobj;

	// create the nodes
	if (nodes > 0)
	{
		// find the largest GID
		int ng = -1;
		for (i = 0; i<nn0; ++i)
		{
			FSNode& n = m_mesh.m_Node[i];
			if (n.m_gid > ng) ng = n.m_gid;
		}
		++ng;

		m_mesh.m_Node.resize(nodes);
		for (i = 0; i<nn1; ++i)
		{
			FSNode& n0 = m_mesh.m_Node[nn0 + i];
			FSNode& n1 = fem.m_Node[i];
			n0 = n1;
			if (n0.m_gid >= 0) n0.m_gid = n1.m_gid + ng;
			assert(po2);
			if (po2) n0.r = po1->GetTransform().GlobalToLocal(po2->GetTransform().LocalToGlobal(n1.r));
			else n0.r = n1.r;
		}
	}

	// create the faces
	if (faces > 0)
	{
		// find the largest GID
		int ng = -1, sg = -1;
		for (i = 0; i<nf0; ++i)
		{
			FSFace& f = m_mesh.m_Face[i];
			if (f.m_gid > ng) ng = f.m_gid;
		}
		++ng; ++sg;

		m_mesh.m_Face.resize(faces);
		for (i = 0; i<nf1; ++i)
		{
			FSFace& f0 = m_mesh.m_Face[nf0 + i];
			FSFace& f1 = fem.m_Face[i];
			f0 = f1;
			f0.m_gid = f1.m_gid + ng;

			f0.m_elem[0].eid = f1.m_elem[0].eid + ne0;
			f0.m_elem[0].lid = f1.m_elem[0].lid;

			if (f1.m_elem[1].eid == -1) f0.m_elem[1].eid = -1;
			else
			{
				f0.m_elem[1].eid = f1.m_elem[1].eid + ne0;
				f0.m_elem[1].lid = f1.m_elem[1].lid;
			}

			for (int j = 0; j<f0.Nodes(); ++j) f0.n[j] = f1.n[j] + nn0;

			f0.m_nbr[0] = f1.m_nbr[0] + nf0;
			f0.m_nbr[1] = f1.m_nbr[1] + nf0;
			f0.m_nbr[2] = f1.m_nbr[2] + nf0;
			if (f0.Nodes() == 4) f0.m_nbr[3] = f1.m_nbr[3] + nf0; else f0.m_nbr[3] = -1;
		}
	}

	// create the solid elements
	if (elems > 0)
	{
		// find the largest GID
		int ng = -1;
		for (i = 0; i<ne0; ++i)
		{
			FSElement& e = m_mesh.m_Elem[i];
			if (e.m_gid > ng) ng = e.m_gid;
		}
		++ng;

		m_mesh.m_Elem.resize(elems);
		m_mesh.m_data.Clear();
		for (i = 0; i<ne1; ++i)
		{
			FSElement& e0 = m_mesh.m_Elem[ne0 + i];
			FSElement& e1 = fem.m_Elem[i];
			e0 = e1;
			e0.m_gid = e1.m_gid + ng;

			for (j = 0; j<6; ++j)
			{
				e0.m_nbr[j] = (e1.m_nbr[j] >= 0 ? e1.m_nbr[j] + ne0 : -1);
			}

			n = e1.Nodes();
			for (j = 0; j<n; ++j) e0.m_node[j] = nn0 + e1.m_node[j];

			int nf = e1.Faces();
			for (j = 0; j<nf; ++j) e0.m_face[j] = nf0 + e1.m_face[j];

			if (e1.IsShell())
			{
				e0.m_face[0] = nf0 + e1.m_face[0];
			}
		}
	}

	// create the edges
	if (edges > 0)
	{
		// find the largest GID
		int ng = -1;
		for (i = 0; i<nl0; ++i)
		{
			FSEdge& e = m_mesh.m_Edge[i];
			if (e.m_gid > ng) ng = e.m_gid;
		}
		++ng;

		m_mesh.m_Edge.resize(edges);
		for (i = 0; i<nl1; ++i)
		{
			FSEdge& l0 = m_mesh.m_Edge[nl0 + i];
			FSEdge& l1 = fem.m_Edge[i];

			l0 = l1;
			l0.m_gid = l1.m_gid + ng;

			int ne = l1.Nodes();
			for (int j = 0; j<ne; ++j) l0.n[j] = nn0 + l1.n[j];

			l0.m_nbr[0] = (l1.m_nbr[0] >= 0 ? l1.m_nbr[0] + nl0 : -1);
			l0.m_nbr[1] = (l1.m_nbr[1] >= 0 ? l1.m_nbr[1] + nl0 : -1);
		}
	}

	// update the mesh
	m_mesh.UpdateMesh();
}

//-----------------------------------------------------------------------------
// Attach another mesh to this mesh and weld the nodes
void FSMeshBuilder::AttachAndWeld(FSMesh& mesh, double tol)
{
	// get the number of nodes
	int nn0 = m_mesh.Nodes();
	int nn1 = mesh.Nodes();

	// get the number of elements
	int ne0 = m_mesh.Elements();
	int ne1 = mesh.Elements();

	// get the number of faces
	int nf0 = m_mesh.Faces();
	int nf1 = mesh.Faces();

	// get the number of edges
	int nl0 = m_mesh.Edges();
	int nl1 = mesh.Edges();

	// calculate new counts (before welding)
	int nodes = nn0 + nn1;
	int elems = ne0 + ne1;
	int faces = nf0 + nf1;
	int edges = nl0 + nl1;

	// attach the two parts
	Attach(mesh);

	// do the weld only if tolerance is positive
	if (tol <= 0.0) return;

	// Okay, do the weld. We only weld nodes from the outer surface of one mesh
	// to the outer surface of the other mesh

	// First, build the target node list
	for (int i = 0; i<nn0; ++i) m_mesh.Node(i).m_ntag = 0;
	for (int i = 0; i<nf0; ++i)
	{
		FSFace& face = m_mesh.Face(i);
		int nf = face.Nodes();
		for (int j = 0; j<nf; ++j) m_mesh.Node(face.n[j]).m_ntag = 1;
	}
	int ntag = 0;
	for (int i = 0; i<nn0; ++i) if (m_mesh.Node(i).m_ntag == 1) ntag++;
	vector<int> trg(ntag); ntag = 0;
	for (int i = 0; i<nn0; ++i) if (m_mesh.Node(i).m_ntag == 1) trg[ntag++] = i;

	// Next, build the source node list
	for (int i = nn0; i<nodes; ++i) m_mesh.Node(i).m_ntag = 0;
	for (int i = nf0; i<faces; ++i)
	{
		FSFace& face = m_mesh.Face(i);
		int nf = face.Nodes();
		for (int j = 0; j<nf; ++j) m_mesh.Node(face.n[j]).m_ntag = 1;
	}
	ntag = 0;
	for (int i = nn0; i<nodes; ++i) if (m_mesh.Node(i).m_ntag == 1) ntag++;
	vector<int> src(ntag); ntag = 0;
	for (int i = nn0; i<nodes; ++i)
		if (m_mesh.Node(i).m_ntag == 1) src[ntag++] = i;


	// create the nodal reorder list
	vector<int> order(nodes);
	for (int i = 0; i<nodes; ++i) order[i] = i;

	// sqr distance treshold
	double tol2 = tol*tol;

	// loop over the selected nodes
	int nsrc = (int)src.size();
	int ntrg = (int)trg.size();
	for (int i = 0; i<nsrc; ++i)
		for (int j = 0; j<ntrg; ++j)
		{
			FSNode& ni = m_mesh.Node(src[i]);
			FSNode& nj = m_mesh.Node(trg[j]);

			// calculate (squared) distance between nodes
			vec3d& ri = ni.r;
			vec3d& rj = nj.r;

			int gi = ni.m_gid;
			int gj = nj.m_gid;

			double d2 = (ri - rj).SqrLength();
			if (d2 <= tol2)
			{
				// nodes coindice, so weld.
				// If one of the nodes has a gid, we don't want to loose it.
				if (gi >= 0) order[trg[j]] = src[i];
				else order[src[i]] = trg[j];
			}
		}

	// update element numbers
	for (int i = 0; i<elems; ++i)
	{
		FSElement& el = m_mesh.Element(i);
		int ne = el.Nodes();
		int* en = el.m_node;

		// reassign node numbers
		for (int j = 0; j<ne; ++j) en[j] = order[en[j]];
	}

	// update face numbers
	for (int i = 0; i<faces; ++i)
	{
		FSFace& face = m_mesh.Face(i);
		int nf = face.Nodes();
		for (int j = 0; j<nf; ++j) face.n[j] = order[face.n[j]];
	}

	// update edge numbers
	for (int i = 0; i<edges; ++i)
	{
		FSEdge& edge = m_mesh.Edge(i);
		int ne = edge.Nodes();
		for (int j = 0; j<ne; ++j) edge.n[j] = order[edge.n[j]];
	}

	// remove isolated vertices
	RemoveIsolatedNodes();

	// At this point, the welding is done but there may be several issues with the mesh
	// so, let's clean up
	m_mesh.RebuildMesh();
	/*
	// update element and face neighbors
	UpdateElementNeighbors();
	UpdateFaces();


	// Remove all duplicate edges
	RemoveDuplicateEdges();
	AutoPartitionNodes();

	// Remove all duplicate faces
	RemoveDuplicateFaces();
	*/
	// TODO: I can still have duplicate shells
	return;
}


//-----------------------------------------------------------------------------
// Detach the selection and return it as a new object
FSMesh* FSMeshBuilder::DetachSelectedMesh()
{
	int i, j, n;

	// count selected elements
	int elems = 0;
	for (i = 0; i<m_mesh.Elements(); ++i) if (m_mesh.ElementPtr(i)->IsSelected()) ++elems;

	// make sure there is a selection
	if (elems == 0) return 0;

	// make sure there are fewer selected elements than elements
	// otherwise we don't have anything left.
	if (elems == m_mesh.Elements()) return 0;

	// tag nodes that will be moved to the new mesh
	FSNode* pn = m_mesh.NodePtr();
	for (i = 0; i<m_mesh.Nodes(); ++i, ++pn) pn->m_ntag = -1;

	for (i = 0; i<m_mesh.Elements(); ++i)
	{
		FSElement_* pe = m_mesh.ElementPtr(i);
		if (pe->IsSelected())
		{
			n = pe->Nodes();
			for (j = 0; j<n; ++j) m_mesh.Node(pe->m_node[j]).m_ntag = 1;
		}
	}

	// count nodes
	int nodes = 0;
	pn = m_mesh.NodePtr();
	for (i = 0; i<m_mesh.Nodes(); ++i, ++pn) if (pn->m_ntag > 0) ++nodes;

	// create a new mesh
	FSMesh* pm = new FSMesh();
	pm->Create(nodes, elems);

	// create the new nodes
	pn = m_mesh.NodePtr();
	n = 0;
	for (i = 0; i<m_mesh.Nodes(); ++i, ++pn)
	{
		if (pn->m_ntag > 0)
		{
			FSNode& node = pm->Node(n);
			node.r = pn->r;
			pn->m_ntag = n++;
		}
	}
	assert(n == nodes);

	// create the new elements
	n = 0;
	for (i = 0; i<m_mesh.Elements(); ++i)
	{
		FSElement_* pe = m_mesh.ElementPtr(i);

		if (pe->IsSelected())
		{
			FSElement& el = pm->Element(n);
			el.SetType(pe->Type());
			el.m_gid = pe->m_gid;
			el.m_MatID = pe->m_MatID;

			for (j = 0; j<pe->Nodes(); ++j)
			{
				el.m_node[j] = m_mesh.Node(pe->m_node[j]).m_ntag;
				assert(el.m_node[j] >= 0);
			}

			if (el.IsShell() && pe->IsShell())
			{
				for (j = 0; j < pe->Nodes(); ++j)
				{
					el.m_h[j] = pe->m_h[j];
				}
			}

			++n;
		}
	}
	assert(n == elems);
	pm->UpdateElementPartitions();

	// update the new mesh (is done later)
	//	pm->Update();

	vector<int> ELT;
	ELT.assign(m_mesh.Elements(), -1);

	// the new mesh is created, so let's clean up this mesh
	// delete selected elements
	n = 0;
	for (i = 0; i<m_mesh.Elements(); ++i)
	{
		FSElement& e0 = m_mesh.Element(i);
		FSElement& e1 = m_mesh.Element(n);

		if (!e0.IsSelected())
		{
			e1 = e0;
			ELT[i] = n;
			++n;
		}
	}
	m_mesh.m_Elem.resize(n);
	m_mesh.m_data.Clear();

	// tag nodes which will be kept
	pn = m_mesh.NodePtr();
	for (i = 0; i<m_mesh.Nodes(); ++i, ++pn) pn->m_ntag = -1;

	for (i = 0; i<m_mesh.Elements(); ++i)
	{
		FSElement_* pe = m_mesh.ElementPtr(i);
		n = pe->Nodes();
		for (j = 0; j<n; ++j) m_mesh.Node(pe->m_node[j]).m_ntag = 1;
	}

	// reindex the nodes
	n = 0;
	pn = m_mesh.NodePtr();
	for (i = 0; i<m_mesh.Nodes(); ++i, ++pn) if (pn->m_ntag > 0) pn->m_ntag = n++;

	for (i = 0; i<m_mesh.Elements(); ++i)
	{
		FSElement_* pe = m_mesh.ElementPtr(i);
		n = pe->Nodes();
		for (j = 0; j<n; ++j)	pe->m_node[j] = m_mesh.Node(pe->m_node[j]).m_ntag;
	}

	// delete untagged nodes
	n = 0;
	for (i = 0; i<m_mesh.Nodes(); ++i)
	{
		FSNode& n0 = m_mesh.Node(i);
		FSNode& n1 = m_mesh.Node(n);

		if (n0.m_ntag >= 0)
		{
			n1 = n0;
			++n;
		}
	}
	m_mesh.m_Node.resize(n);

	// update the mesh (is done later)
	///	Update();

	m_mesh.RebuildMesh();
	pm->RebuildMesh();

	// Done!
	return pm;
}

//-----------------------------------------------------------------------------
void FSMeshBuilder::RemoveDuplicateEdges()
{
	// clear tags
	m_mesh.TagAllEdges(0);

	int ng = m_mesh.CountEdgePartitions();

	// loop over all edges
	int NE = m_mesh.Edges();
	for (int i = 0; i<NE; ++i)
	{
		FSEdge& ei = m_mesh.Edge(i);
		for (int j = i + 1; j<NE; ++j)
		{
			FSEdge& ej = m_mesh.Edge(j);
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

	m_mesh.RemoveEdges(1);
	m_mesh.UpdateEdgeNeighbors();
	m_mesh.UpdateEdgePartitions();
}

//-----------------------------------------------------------------------------
void FSMeshBuilder::RemoveDuplicateFaces()
{
	// clear tags
	m_mesh.TagAllFaces(0);

	// loop over all faces
	int NF = m_mesh.Faces();
	for (int i = 0; i<NF; ++i)
	{
		FSFace& fi = m_mesh.Face(i);
		for (int j = i + 1; j<NF; ++j)
		{
			FSFace& fj = m_mesh.Face(j);
			if (fi == fj)
			{
				fi.m_ntag = 1;

				// we set the element indices to 0 to avoid deleting these elements
				fi.m_elem[0].eid = fi.m_elem[1].eid = fi.m_elem[2].eid = -1;
			}
		}
	}

	// delete tagged faces
	m_mesh.RemoveFaces(1);
	m_mesh.UpdateFaceElementTable();
	m_mesh.UpdateFaceNeighbors();
	m_mesh.UpdateFacePartitions();
}

//-----------------------------------------------------------------------------
// Invert selected elements (or all if none are selected)
void FSMeshBuilder::InvertSelectedElements()
{
	// tag all selected elements
	int nsel = 0;
	for (int i = 0; i<m_mesh.Elements(); ++i)
	{
		FSElement& e = m_mesh.Element(i);
		if (e.IsSelected()) { e.m_ntag = 1; nsel++; }
		else e.m_ntag = 0;
	}
	if (nsel == 0) m_mesh.TagAllElements(1);

	// invert the tagged elements
	InvertTaggedElements(1);
}

//-----------------------------------------------------------------------------
void FSMeshBuilder::InvertSelectedFaces()
{
	// tag all selected faces
	int nsel = 0;
	for (int i = 0; i<m_mesh.Faces(); ++i)
	{
		FSFace& f = m_mesh.Face(i);
		if (f.IsSelected()) { f.m_ntag = 1; nsel++; }
		else f.m_ntag = 0;
	}
	if (nsel == 0) m_mesh.TagAllFaces(1);

	// invert the tagged elements
	InvertTaggedFaces(1);
}

//-----------------------------------------------------------------------------
// Invert selected faces
void FSMeshBuilder::InvertTaggedFaces(int ntag)
{
	// invert tagged elements
	for (int i = 0; i<m_mesh.Faces(); ++i)
	{
		FSFace& f = m_mesh.Face(i);
		if (f.m_ntag == ntag)
		{
			int n = f.Nodes(), m;

			switch (f.Type())
			{
			case FE_FACE_QUAD4:
			case FE_FACE_TRI3:
			{
				m = f.n[0]; f.n[0] = f.n[2]; f.n[2] = m;
			}
			break;
            case FE_FACE_TRI6:
            case FE_FACE_TRI7:
            {
                m = f.n[0]; f.n[0] = f.n[2]; f.n[2] = m;
                m = f.n[3]; f.n[3] = f.n[4]; f.n[4] = m;
            }
            break;
            case FE_FACE_QUAD8:
            {
                m = f.n[0]; f.n[0] = f.n[2]; f.n[2] = m;
                m = f.n[4]; f.n[4] = f.n[5]; f.n[5] = m;
                m = f.n[6]; f.n[6] = f.n[7]; f.n[7] = m;
            }
            break;
			default:
				assert(false);
			}
		}
	}

	m_mesh.UpdateElementNeighbors();
	m_mesh.UpdateFaceElementTable();
	m_mesh.UpdateFaceNeighbors();
}

//-----------------------------------------------------------------------------
// Invert selected elements.
void FSMeshBuilder::InvertTaggedElements(int ntag)
{
	// invert tagged elements
	for (int i = 0; i<m_mesh.Elements(); ++i)
	{
		FSElement& e = m_mesh.Element(i);
		if (e.m_ntag == ntag)
		{
			int n = e.Nodes(), m;

			switch (e.Type())
			{
			case FE_QUAD4:
			case FE_TRI3:
			{
				for (int j = 0; j<n / 2; ++j)
				{
					m = e.m_node[j];
					e.m_node[j] = e.m_node[n - j - 1];
					e.m_node[n - j - 1] = m;
				}
			}
			break;
			case FE_TET4:
			case FE_TET5:
			{
				m = e.m_node[0]; e.m_node[0] = e.m_node[3]; e.m_node[3] = m;
			}
			break;
			case FE_TET10:
			{
				m = e.m_node[1]; e.m_node[1] = e.m_node[2]; e.m_node[2] = m;
				m = e.m_node[4]; e.m_node[4] = e.m_node[6]; e.m_node[6] = m;
				m = e.m_node[8]; e.m_node[8] = e.m_node[9]; e.m_node[9] = m;
			}
			break;
			case FE_PENTA6:
			{
				m = e.m_node[0]; e.m_node[0] = e.m_node[2]; e.m_node[2] = m;
				m = e.m_node[3]; e.m_node[3] = e.m_node[5]; e.m_node[5] = m;
			}
			break;
			case FE_PENTA15:
			{
				m = e.m_node[1]; e.m_node[1] = e.m_node[2]; e.m_node[2] = m;
				m = e.m_node[4]; e.m_node[4] = e.m_node[5]; e.m_node[5] = m;
				m = e.m_node[6]; e.m_node[6] = e.m_node[8]; e.m_node[8] = m;
				m = e.m_node[9]; e.m_node[9] = e.m_node[11]; e.m_node[11] = m;
				m = e.m_node[13]; e.m_node[13] = e.m_node[14]; e.m_node[14] = m;
			}
			break;
			case FE_HEX8:
			{
				m = e.m_node[0]; e.m_node[0] = e.m_node[4]; e.m_node[4] = m;
				m = e.m_node[1]; e.m_node[1] = e.m_node[5]; e.m_node[5] = m;
				m = e.m_node[2]; e.m_node[2] = e.m_node[6]; e.m_node[6] = m;
				m = e.m_node[3]; e.m_node[3] = e.m_node[7]; e.m_node[7] = m;
			}
			break;
			case FE_HEX20:
			{
				m = e.m_node[1]; e.m_node[1] = e.m_node[3]; e.m_node[3] = m;
				m = e.m_node[5]; e.m_node[5] = e.m_node[7]; e.m_node[7] = m;
				m = e.m_node[8]; e.m_node[8] = e.m_node[11]; e.m_node[11] = m;
				m = e.m_node[9]; e.m_node[9] = e.m_node[10]; e.m_node[10] = m;
				m = e.m_node[12]; e.m_node[12] = e.m_node[15]; e.m_node[15] = m;
				m = e.m_node[13]; e.m_node[13] = e.m_node[14]; e.m_node[14] = m;
				m = e.m_node[17]; e.m_node[17] = e.m_node[19]; e.m_node[19] = m;
			}
			break;
			case FE_PYRA5:
			{
				m = e.m_node[1]; e.m_node[1] = e.m_node[3]; e.m_node[3] = m;
			}
			break;
            case FE_PYRA13:
            {
                m = e.m_node[1];  e.m_node[1]  = e.m_node[3];  e.m_node[3] = m;
                m = e.m_node[5];  e.m_node[5]  = e.m_node[8];  e.m_node[8] = m;
                m = e.m_node[6];  e.m_node[6]  = e.m_node[7];  e.m_node[7] = m;
                m = e.m_node[10]; e.m_node[10] = e.m_node[12]; e.m_node[12] = m;
            }
            break;
			case FE_TRI6:
			case FE_TRI7:
			{
				m = e.m_node[0]; e.m_node[0] = e.m_node[2]; e.m_node[2] = m;
				m = e.m_node[3]; e.m_node[3] = e.m_node[4]; e.m_node[4] = m;
			}
			break;
			default:
				assert(false);
			}
		}
	}

	// mirror the faces
	for (int i = 0; i<m_mesh.Faces(); ++i)
	{
		FSFace& f = m_mesh.Face(i);
		FSElement_* pe = m_mesh.ElementPtr(f.m_elem[0].eid);

		if (pe->m_ntag == ntag)
		{
			FSFace g;
			m_mesh.FindFace(pe, f, g);

			for (int j = 0; j<FSFace::MAX_NODES; ++j)
				f.n[j] = g.n[j];
		}
	}

	m_mesh.UpdateElementNeighbors();
	m_mesh.UpdateFaceElementTable();
	m_mesh.UpdateFaceNeighbors();
	m_mesh.UpdateMesh();
}

//-----------------------------------------------------------------------------
// Partition the selected faces
//
void FSMeshBuilder::PartitionFaceSelection(int gid)
{
	// get the number of face partitions
	int nsg = m_mesh.CountFacePartitions();
	if ((gid >= 0) && (gid < nsg)) nsg = gid;;

	// assign selected faces to a new partition
	int N = m_mesh.Faces();
	for (int i = 0; i<N; ++i)
	{
		FSFace& f = m_mesh.Face(i);
		if (f.IsSelected())
		{
			f.m_gid = nsg;
		}
	}

	// update partitions
	m_mesh.UpdateFacePartitions();

	// rebuild edge and node data
	BuildEdges();
	AutoPartitionEdges();
	AutoPartitionNodes();
	m_mesh.RebuildNodeData();

	// update mesh
	m_mesh.UpdateMesh();
}

//-----------------------------------------------------------------------------
// Partition the selected edges
//
void FSMeshBuilder::PartitionEdgeSelection(int gid)
{
	int nsg = m_mesh.CountEdgePartitions();
	if ((gid >= 0) && (gid < nsg)) nsg = gid;

	// tag all selected edges
	int NE = m_mesh.Edges();
	for (int i = 0; i<NE; ++i)
	{
		FSEdge& e = m_mesh.Edge(i);
		if (e.IsSelected())
		{
			e.m_gid = nsg;
			e.SetExterior(true);
		}
	}
	m_mesh.UpdateEdgePartitions();

	// recalculate edge neighbors
	m_mesh.UpdateEdgeNeighbors();

	// partition the nodes
	AutoPartitionNodes();

	// update the nodes
	m_mesh.MarkExteriorNodes();
}

//-----------------------------------------------------------------------------
void FSMeshBuilder::PartitionElementSelection(int gid)
{
	// decide a partition ID
	int partId = m_mesh.CountElementPartitions();
	if ((gid >= 0) && (gid < partId)) partId = gid;

	// Tag selected elements
	int NE = m_mesh.Elements();
	for (int i = 0; i<NE; ++i)
	{
		FSElement& el = m_mesh.Element(i);
		if (el.IsSelected()) el.m_ntag = 1; else el.m_ntag = 0;
	}

	vector<int> s(NE, -1); int ne = 0;
	for (int i = 0; i<NE; ++i)
	{
		FSElement& el = m_mesh.Element(i);
		if (el.m_ntag == 1)
		{
			el.m_ntag = 0;
			s[ne++] = i;
			while (ne > 0)
			{
				FSElement& el = m_mesh.Element(s[--ne]);
				int nbr = (el.IsSolid() ? el.Faces() : el.Edges());
				for (int j = 0; j < nbr; ++j)
				{
					FSElement_* pe = m_mesh.ElementPtr(el.m_nbr[j]);
					if (pe && (pe->m_gid == el.m_gid) && (pe->m_ntag == 1))
					{
						s[ne++] = el.m_nbr[j];
						pe->m_ntag = 0;
					}
				}
				el.m_gid = partId;
			}
		}
	}

	// add new faces, if found
	int nfp = m_mesh.CountFacePartitions();
	int newFaces = 0;
	for (int i = 0; i<NE; ++i)
	{
		FSElement& el = m_mesh.Element(i);
		if (el.IsSelected())
		{
			// solid elements
			int nf = el.Faces();
			for (int j = 0; j<nf; ++j)
			{
				if (el.m_face[j] == -1)
				{
					assert(el.m_nbr[j] >= 0);
					if (el.m_nbr[j] >= 0)
					{
						FSElement& ej = m_mesh.Element(el.m_nbr[j]);
						if (ej.m_gid != el.m_gid)
						{
							FSFace fj = el.GetFace(j);
							fj.m_gid = -1;
							m_mesh.AddFace(fj);
							newFaces++;
						}
					}
				}
				else
				{
					FSFace& face = m_mesh.Face(el.m_face[j]);
					face.m_gid += nfp + 1;
				}
			}

			// shell elements
			int ne = el.Edges();
			if (ne > 0)
			{
				assert(el.m_face[0] != -1);
				m_mesh.Face(el.m_face[0]).m_gid += nfp + 1;
			}
		}
	}
	// we need to update all the face neighbors
	// This updates also the element's face indices
	m_mesh.UpdateFacePartitions();
	m_mesh.UpdateFaceElementTable();
	m_mesh.UpdateFaceNeighbors();

	// assign new face partitions where necessary
	nfp = m_mesh.CountFacePartitions();
	for (int i = 0; i<m_mesh.Faces(); ++i)
	{
		FSFace& face = m_mesh.Face(i);
		if (face.m_gid == -1)
		{
			stack<int> s; s.push(i);
			face.m_gid = nfp;
			while (s.empty() == false)
			{
				FSFace& face = m_mesh.Face(s.top()); s.pop();
				for (int j = 0; j<face.Edges(); ++j)
				{
					if (face.m_nbr[j] >= 0)
					{
						FSFace& fj = m_mesh.Face(face.m_nbr[j]);
						if (fj.m_gid == -1)
						{
							fj.m_gid = nfp;
							s.push(face.m_nbr[j]);
						}
					}
				}
			}
			nfp++;
		}
	}

	// repartition the edges and nodes
	BuildEdges();
	AutoPartitionEdges();

	// we don't want to lose current nodal partitioning.
	int N = m_mesh.Nodes();
	int np = m_mesh.CountNodePartitions();
	vector<int> idlist(np, -1);
	for (int i = 0; i < N; ++i)
	{
		FSNode& node = m_mesh.Node(i);
		if (node.m_gid >= 0)
		{
			assert(idlist[node.m_gid] == -1);
			idlist[node.m_gid] = i;
		}
	}

	AutoPartitionNodes();

	// restore partitioning
	int np1 = m_mesh.CountNodePartitions();
	if (np1 < np) np1 = np;
	for (int i = 0; i < N; ++i)
	{
		FSNode& node = m_mesh.Node(i);
		if (node.m_gid != -1)
		{
			node.m_gid = np1++;
		}
	}
	for (int i = 0; i < np; ++i)
	{
		m_mesh.Node(idlist[i]).m_gid = i;
	}
	m_mesh.UpdateNodePartitions();
}

//-----------------------------------------------------------------------------
void FSMeshBuilder::PartitionNodeSet(FSNodeSet* pg)
{
	if ((pg == 0) || (pg->size() == 0)) return;

	FSItemListBuilder::Iterator it = pg->begin();
	for (; it != pg->end(); ++it)
	{
		PartitionNode(*it);
	}
}

//-----------------------------------------------------------------------------
void FSMeshBuilder::PartitionNode(int nodeIndex)
{
	// get the node
	FSNode& node = m_mesh.Node(nodeIndex);

	// see if this node is already partitioned or not
	// If it is, we don't need to do anything
	if (node.m_gid >= 0) return;

	// okay, partition the node then
	int ng = m_mesh.CountNodePartitions();
	node.m_gid = ng;

	// It's possible this node splits an edge, so let's check for that
	ng = m_mesh.CountEdgePartitions();
	for (int i = 0; i<m_mesh.Edges(); ++i)
	{
		FSEdge& edge = m_mesh.Edge(i);
		if (edge.m_gid >= 0)
		{
			if (edge.n[0] == nodeIndex)
			{
				FSEdge* pe = m_mesh.EdgePtr(edge.m_nbr[0]);
				edge.m_nbr[0] = -1;
				if (pe)
				{
					if (pe->n[0] == nodeIndex) pe->m_nbr[0] = -1;
					else if (pe->n[1] == nodeIndex) pe->m_nbr[1] = -1;
					else assert(false);
				}

				int m = edge.n[1];
				edge.m_gid = ng;
				FSEdge* e = m_mesh.EdgePtr(edge.m_nbr[1]);
				while (e)
				{
					e->m_gid = ng;
					if (e->n[0] == m) { m = e->n[1]; e = m_mesh.EdgePtr(e->m_nbr[1]); }
					else { assert(e->n[1] == m); m = e->n[0]; e = m_mesh.EdgePtr(e->m_nbr[0]); }
				}

				break;
			}
			else if (edge.n[1] == nodeIndex)
			{
				FSEdge* pe = m_mesh.EdgePtr(edge.m_nbr[1]);
				edge.m_nbr[1] = -1;
				if (pe)
				{
					if (pe->n[0] == nodeIndex) pe->m_nbr[0] = -1;
					else if (pe->n[1] == nodeIndex) pe->m_nbr[1] = -1;
					else assert(false);
				}

				int m = edge.n[0];
				edge.m_gid = ng;
				FSEdge* e = m_mesh.EdgePtr(edge.m_nbr[0]);
				while (e)
				{
					e->m_gid = ng;
					if (e->n[0] == m) { m = e->n[1]; e = m_mesh.EdgePtr(e->m_nbr[1]); }
					else { assert(e->n[1] == m); m = e->n[0]; e = m_mesh.EdgePtr(e->m_nbr[0]); }
				}

				break;
			}
		}
	}
}

// auto-partition selections
bool FSMeshBuilder::AutoPartitionEdges(double w, FSEdgeSet* pg)
{
	if (pg == nullptr) return false;

	// build the edge list
	vector<int> edgeList;
	FSItemListBuilder::Iterator it = pg->begin();
	int N = (int)pg->size();
	for (int i = 0; i<N; ++i, ++it)
	{
		edgeList.push_back(*it);
	}

	// make sure there is something to do
	if (edgeList.empty()) return false;

	// make sure all edges are on the same curve
	int gid = m_mesh.Edge(edgeList[0]).m_gid;
	if (gid == -1) return false;
	for (int i = 1; i < edgeList.size(); ++i)
	{
		if (m_mesh.Edge(edgeList[i]).m_gid != gid) return false;
	}

	// get partition count
	int ng = m_mesh.CountEdgePartitions();

	double cw = cos(w*DEG2RAD);

	// start
	m_mesh.TagAllEdges(-1);
	for (int i = 0; i < edgeList.size(); ++i)
	{
		// find an unprocessed edge
		FSEdge& edge = m_mesh.Edge(edgeList[i]);
		if (edge.m_ntag == -1)
		{
			stack<FSEdge*> s;
			edge.m_ntag = 1;
			s.push(&edge);
			while (s.empty() == false)
			{
				FSEdge* pe = s.top(); s.pop();
				pe->m_gid = ng;

				vec3d t = m_mesh.Node(pe->n[1]).r - m_mesh.Node(pe->n[0]).r;
				t.Normalize();

				for (int j = 0; j < 2; ++j)
				{
					FSEdge* pej = m_mesh.EdgePtr(pe->m_nbr[j]);
					if (pej && (pej->m_ntag == -1))
					{
						assert((pej->n[0] == pe->n[j]) || (pej->n[1] == pe->n[j]));
						vec3d tj = m_mesh.Node(pej->n[1]).r - m_mesh.Node(pej->n[0]).r;
						if (pej->n[j] == pe->n[j]) tj = -tj;
						tj.Normalize();

						if (tj*t >= cw)
						{
							pej->m_ntag = 1;
							s.push(pej);
						}
					}
				}
			}
			ng++;
		}
	}
	m_mesh.UpdateEdgePartitions();
	m_mesh.RebuildEdgeData();
	AutoPartitionNodes();
	m_mesh.RebuildNodeData();

	return true;
}

//-----------------------------------------------------------------------------
bool FSMeshBuilder::AutoPartitionFaces(double w, FSSurface* pg)
{
	// sanity checks
	if (pg == nullptr) return false;
	if (pg->size() == 0) return false;

	// Tag all faces to partition
	m_mesh.TagAllFaces(0);
	for (FSItemListBuilder::Iterator it = pg->begin(); it != pg->end(); ++it)
	{
		m_mesh.Face(*it).m_ntag = 1;
	}

	// smoothing threshold
	double eps = (double)cos(w * DEG2RAD);

	// stack for tracking unprocessed faces
	vector<FSFace*> stack(m_mesh.Faces(), nullptr);
	int ns = 0;

	// process all faces
	int ng = m_mesh.CountFacePartitions();
	for (FSItemListBuilder::Iterator it = pg->begin(); it != pg->end(); ++it)
	{
		FSFace* pf = m_mesh.FacePtr(*it);
		if (pf->m_ntag == 1)
		{
			vec3d Nf = m_mesh.FaceNormal(*pf);
			FSElement_* pe = m_mesh.ElementPtr(pf->m_elem[0].eid);
			int pid = (pe == nullptr ? -1 : pe->m_gid);
			pg->m_ntag = 0;
			stack[ns++] = pf;
			while (ns > 0)
			{
				// pop a face
				pf = stack[--ns];

				// assign GID
				pf->m_gid = ng;

				// loop over neighbors
				int n = pf->Edges();
				for (int j = 0; j < n; ++j)
				{
					FSFace* pf2 = m_mesh.FacePtr(pf->m_nbr[j]);
					FSElement_* pe2 = m_mesh.ElementPtr(pf2->m_elem[0].eid);
					int pid2 = (pe2 == nullptr ? -1 : pe2->m_gid);

					// push unprocessed neighbour
					if (pf2 && (pf2->m_ntag == 1) && (Nf * m_mesh.FaceNormal(*pf2) >= eps) && (pid == pid2))
					{
						pf2->m_ntag = 0;
						stack[ns++] = pf2;
					}
				}
			}
			++ng;
		}
	}
	m_mesh.UpdateFacePartitions();

	// we need to rebuild the edges
	BuildEdges();
	AutoPartitionEdges();

	// partition the nodes
	AutoPartitionNodes();
	m_mesh.RebuildNodeData();

	return true;
}

//-----------------------------------------------------------------------------
void FSMeshBuilder::AutoPartition(double smoothingAngle)
{
	// partition the surface based on connectivity and smoothing IDs
	AutoPartitionSurface(smoothingAngle, true);

	// we need to rebuild the edges
	BuildEdges();
	AutoPartitionEdges();

	// partition the nodes
	AutoPartitionNodes();
	m_mesh.RebuildNodeData();
}

//-----------------------------------------------------------------------------
void FSMeshBuilder::AutoPartitionSurface(double angleDegrees, bool creaseInternal)
{
	// smoothing threshold
	double eps = (double)cos(angleDegrees * DEG2RAD);

	// clear face group ID's
	int NF = m_mesh.Faces();
	for (int i = 0; i < NF; ++i)
	{
		FSFace* pf = m_mesh.FacePtr(i);
		pf->m_gid = -1;
	}

	// stack for tracking unprocessed faces
	vector<FSFace*> stack(NF);
	int ns = 0;

	// process all faces
	int ngid = 0;
	for (int i = 0; i < NF; ++i)
	{
		FSFace* pf = m_mesh.FacePtr(i);
		if (pf->m_gid == -1)
		{
			stack[ns++] = pf;
			while (ns > 0)
			{
				// pop a face
				pf = stack[--ns];

				// mark as processed
				pf->m_gid = ngid;

				vec3d Nf = m_mesh.FaceNormal(*pf);

				// get the element part ID's
				assert(pf->m_elem[0].eid >= 0);
				FSElement_* pe11 = m_mesh.ElementPtr(pf->m_elem[0].eid);
				FSElement_* pe12 = m_mesh.ElementPtr(pf->m_elem[1].eid);
				int gid11 = (pe11 ? pe11->m_gid : -1);
				int gid12 = (pe12 ? pe12->m_gid : -1);

				// loop over neighbors
				int n = pf->Edges();
				for (int j = 0; j < n; ++j)
				{
					FSFace* pf2 = m_mesh.FacePtr(pf->m_nbr[j]);

					// push unprocessed neighbour
					if (pf2 && (pf2->m_gid == -1))
					{
						bool badd = false;
						if ((pf->IsExternal() == false) && (pf2->IsExternal() == false))
						{
							if ((creaseInternal == false) || (Nf * m_mesh.FaceNormal(*pf2) >= eps))
								badd = true;
						}
						else if (Nf * m_mesh.FaceNormal(*pf2) >= eps)
						{
							badd = true;
						}

						assert(pf2->m_elem[0].eid >= 0);
						FSElement_* pe21 = m_mesh.ElementPtr(pf2->m_elem[0].eid);
						FSElement_* pe22 = m_mesh.ElementPtr(pf2->m_elem[1].eid);
						{
							int gid21 = (pe21 ? pe21->m_gid : -1);
							int gid22 = (pe22 ? pe22->m_gid : -1);

							if ((gid11 != gid21) || (gid12 != gid22)) badd = false;

							if (badd)
							{
								pf2->m_gid = -2;
								stack[ns++] = pf2;
							}
						}
					}
				}
			}
			++ngid;
		}
	}
}

//-----------------------------------------------------------------------------
// This function builds the surface, edges and node of the mesh
void FSMeshBuilder::RebuildMesh(double smoothingAngle, bool partitionMesh, bool creaseInternal)
{
	m_mesh.ClearMeshTopo();

	// update the element neighbours
	m_mesh.UpdateElementNeighbors();

	// update element partitioning
	if (partitionMesh)
	{
		// partition the elements based on element connectivity
		AutoPartitionElements();
	}
	else m_mesh.UpdateElementPartitions();

	// build the faces
	BuildFaces();

	// partition the surface based on the smoothing
	AutoPartitionSurface(smoothingAngle, creaseInternal);

	// build the edges
	BuildEdges();

	// partition the edges
	AutoPartitionEdges();

	// partition the nodes
	AutoPartitionNodes();
	m_mesh.RebuildNodeData();

	// update the mesh
	m_mesh.UpdateMesh();
}

//-----------------------------------------------------------------------------
// This partitions elements based on connectivity
void FSMeshBuilder::AutoPartitionElements()
{
	int NE = m_mesh.Elements();
	for (int i = 0; i<NE; ++i) m_mesh.Element(i).m_gid = -1;
	m_mesh.TagAllElements(0);

	int i0 = 0;
	int np = 0;
	while (true)
	{
		// find an unprocessed element
		int n = -1;
		for (int i = i0; i<NE; ++i)
		{
			FSElement& el = m_mesh.Element(i);
			if (el.m_gid == -1)
			{
				i0 = i + 1;
				n = i;
				break;
			}
		}
		if (n == -1) break;

		// find all connected elements
		stack<int> s;
		s.push(n);
		while (s.empty() == false)
		{
			n = s.top(); s.pop();

			FSElement& el = m_mesh.Element(n);
			el.m_ntag = 1;
			el.m_gid = np;

			// solid elements
			if (el.IsSolid())
			{
				int nf = el.Faces();
				for (int i = 0; i < nf; ++i)
				{
					FSElement* pj = (el.m_nbr[i] == -1 ? 0 : &m_mesh.Element(el.m_nbr[i]));
					if (pj && (pj->m_ntag == 0))
					{
						pj->m_ntag = 1;
						s.push(el.m_nbr[i]);
					}
				}
			}
			else if (el.IsShell())
			{
				// shells
				int ne = el.Edges();
				for (int i = 0; i < ne; ++i)
				{
					FSElement* pj = (el.m_nbr[i] == -1 ? 0 : &m_mesh.Element(el.m_nbr[i]));
					if (pj && (pj->m_ntag == 0))
					{
						pj->m_ntag = 1;
						s.push(el.m_nbr[i]);
					}
				}
			}
		}
		++np;
	}

	// group all beam elements together
	for (int i = 0; i < NE; ++i)
	{
		FSElement& el = m_mesh.Element(i);
		if (el.IsBeam())
		{
			el.m_gid = np;
		}
	}
	m_mesh.UpdateElementPartitions();
}

//-----------------------------------------------------------------------------
void FSMeshBuilder::AutoPartitionEdges()
{
	// Tag candidate edges
	for (int i = 0; i < m_mesh.Edges(); ++i)
	{
		FSEdge& edge = m_mesh.Edge(i);
		if (edge.m_gid >= 0) edge.m_ntag = 0;
		else edge.m_ntag = -1;
	}

	// loop over candidate edges
	int ng = 0, n0 = 0;
	stack<int> s;
	do
	{
		for (int i = n0; i<m_mesh.Edges(); ++i, ++n0)
		{
			if (m_mesh.Edge(i).m_ntag == 0)
			{
				s.push(i);
				break;
			}
		}

		if (s.empty()) break;

		while (s.empty() == false)
		{
			int edgeId = s.top(); s.pop();

			FSEdge& edge = m_mesh.Edge(edgeId);
			edge.m_ntag = 1;
			edge.m_gid = ng;

			if (edge.m_nbr[0] != -1)
			{
				FSEdge& e0 = m_mesh.Edge(edge.m_nbr[0]);
				if (e0.m_ntag == 0) s.push(edge.m_nbr[0]);
			}

			if (edge.m_nbr[1] != -1)
			{
				FSEdge& e1 = m_mesh.Edge(edge.m_nbr[1]);
				if (e1.m_ntag == 0) s.push(edge.m_nbr[1]);
			}
		}

		ng++;
	} while (1);
}

//-----------------------------------------------------------------------------
// Assign gids to the nodes based on the edge gids.
void FSMeshBuilder::AutoPartitionNodes()
{
	// get the mesh
	int NE = m_mesh.Edges();

	// reset node tags
	int nn = 0;
	for (int i = 0; i<m_mesh.Nodes(); ++i) m_mesh.Node(i).m_gid = -1;

	// get the edge pointer
	FSEdge* pe = m_mesh.EdgePtr();

	// loop over all edges
	for (int i = 0; i<NE; ++i, ++pe)
	{
		if (pe->m_gid >= 0)
		{
			assert(pe->IsExterior());
			if ((pe->m_nbr[0] == -1) || (m_mesh.EdgePtr(pe->m_nbr[0])->m_gid != pe->m_gid))
			{
				FSNode& node = m_mesh.Node(pe->n[0]);
				if (node.m_gid == -1) { node.m_gid = nn++; }
			}
			if ((pe->m_nbr[1] == -1) || (m_mesh.EdgePtr(pe->m_nbr[1])->m_gid != pe->m_gid))
			{
				FSNode& node = m_mesh.Node(pe->n[1]);
				if (node.m_gid == -1) { node.m_gid = nn++; }
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FSMeshBuilder::BuildFaces()
{
	// let's count them first
	int faces = 0;
	int elems = m_mesh.Elements();
	for (int i = 0; i<elems; i++)
	{
		FSElement& el = m_mesh.Element(i);

		// solid elements
		// we create a face if an element does not have a neighbor
		// or if the neighbor has a different gid and is not a shell.
		// Note that we need to make sure we don't double-count.
		int n = el.Faces();
		for (int j = 0; j<n; ++j)
		{
			if (el.m_nbr[j] == -1) ++faces;
			else
			{
				FSElement_* pen = m_mesh.ElementPtr(el.m_nbr[j]);
				if ((el.m_gid < pen->m_gid) && (pen->IsShell() == false))
				{
					++faces;
				}
			}
		}

		// shell elements always add a face
		if (el.IsShell())
		{
			++faces;
		}
	}

	// make sure we have faces
	if (faces == 0)
	{
		m_mesh.m_Face.clear();
		return;
	}

	// allocate storage
	m_mesh.m_Face.resize(faces);

	// create the faces
	FSFace* pf = m_mesh.FacePtr();
	int nf = 0;

	// solid elements
	for (int i = 0; i<elems; i++)
	{
		FSElement& el = m_mesh.Element(i);

		int n = el.Faces();
		for (int j = 0; j<n; j++)
		{
			el.m_face[j] = -1;
			int nbid = el.m_nbr[j];
			FSElement_* pen = (nbid == -1 ? 0 : m_mesh.ElementPtr(nbid));
			if (pen == 0)
			{
				el.GetFace(j, *pf);
				pf->SetExterior(true);
				pf->SetID(nf + 1);
				++pf; ++nf;
			}
			else if ((el.m_gid < pen->m_gid) && (pen->IsShell() == false))
			{
				*pf = el.GetFace(j);
				pf->SetExterior(false);
				pf->SetID(nf + 1);
				++pf; ++nf;
			}
		}
	}

	// shell elements
	for (int i = 0; i<elems; i++)
	{
		FSElement& el = m_mesh.Element(i);

		// shell elements
		int n = el.Edges();
		if (n>0)
		{
			el.GetShellFace(*pf);
			pf->SetExterior(true);
			pf->SetID(nf + 1);
			++pf; ++nf;
		}
	}

	// update face data
	m_mesh.RebuildFaceData();
}

//-----------------------------------------------------------------------------
void FSMeshBuilder::BuildEdges()
{
	// delete edges
	m_mesh.m_Edge.clear();

	// tag all faces
	for (int i = 0; i < m_mesh.Faces(); ++i) m_mesh.Face(i).m_ntag = i;

	// keep node-edge table to prevent adding duplicate edges
	int NN = m_mesh.Nodes();
	vector<vector<int> > NET(NN);

	// loop over all faces
	int NF = m_mesh.Faces();
	for (int i = 0; i<NF; ++i)
	{
		FSFace& f = m_mesh.Face(i);
		int ne = f.Edges();
		for (int j = 0; j<ne; ++j)
		{
			FSFace* pfn = m_mesh.FacePtr(f.m_nbr[j]);
			if (((pfn == 0) && f.IsExternal()) || (pfn && (f.m_ntag < pfn->m_ntag)))
			{
				FSEdge e = f.GetEdge(j);

				// see if this edge already exists
				bool bfound = false;
				vector<int>& net = NET[e.n[0]];
				for (int l = 0; l < net.size(); ++l)
				{
					FSEdge& el = m_mesh.Edge(net[l]);
					if (el == e)
					{
						// the edge already exists, so don't add it
						bfound = true;
						break;
					}
				}

				// If not, then process and add
				if (bfound == false)
				{
					e.m_gid = ((pfn == 0) || (f.m_gid != pfn->m_gid) ? 0 : -1);
					e.SetID((int)m_mesh.m_Edge.size() + 1);
					e.SetExterior(e.m_gid == 0);

					int edgeIndex = m_mesh.Edges();
					m_mesh.m_Edge.push_back(e);
					NET[e.n[0]].push_back(edgeIndex);
					NET[e.n[1]].push_back(edgeIndex);
				}
			}
		}
	}

	// loop over beam elements
	for (int i = 0; i < m_mesh.Elements(); ++i)
	{
		FSElement& el = m_mesh.Element(i);
		if (el.IsBeam())
		{
			FSEdge e;
			e.SetType(FE_EDGE2);
			e.n[0] = el.m_node[0];
			e.n[1] = el.m_node[1];

			// see if this edge already exists
			bool bfound = false;
			vector<int>& net = NET[e.n[0]];
			for (int l = 0; l < net.size(); ++l)
			{
				FSEdge& el = m_mesh.Edge(net[l]);
				if (el == e)
				{
					// the edge already exists, so don't add it
					bfound = true;
					break;
				}
			}

			// If not, then process and add
			if (bfound == false)
			{
				e.m_gid = 0;
				e.m_elem = i;
				e.SetID((int)m_mesh.m_Edge.size() + 1);
				e.SetExterior(e.m_gid == 0);

				int edgeIndex = m_mesh.Edges();
				m_mesh.m_Edge.push_back(e);
				NET[e.n[0]].push_back(edgeIndex);
				NET[e.n[1]].push_back(edgeIndex);
			}
		}
	}

	m_mesh.RebuildEdgeData();
}

void FSMeshBuilder::RepairEdges()
{
	BuildEdges();
	AutoPartitionEdges();
	AutoPartitionNodes();
	m_mesh.RebuildNodeData();
}
