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

#include "FEMesh.h"
#include <GeomLib/GObject.h>
#include "triangulate.h"
#include "FESurfaceMesh.h"
#include "MeshMetrics.h"
#include "FENodeElementList.h"
#include "FENodeFaceList.h"
#include "FENodeEdgeList.h"
#include "MeshTools/FENodeData.h"
#include "MeshTools/FESurfaceData.h"
#include "MeshTools/FEElementData.h"
#include "FEMeshBuilder.h"
#include <MeshTools/GLMesh.h>
#include <algorithm>
#include <unordered_set>
#include <map>
using namespace std;

double bias(double b, double x)
{
	const double f = 1.f / (double) log(0.5);
	return (double) pow(x, log(b)*f);
}

double gain(double g, double x)
{
	if (x < 0.5f)
		return bias(1.f-g, 2.f*x)*0.5f;
	else
		return 1.f - bias(1.f-g, 2.f - 2.f*x)*0.5f;
}

//-----------------------------------------------------------------------------
Mesh_Data::Mesh_Data()
{
	m_min = m_max = 0.0;
}

//-----------------------------------------------------------------------------
Mesh_Data::Mesh_Data(const Mesh_Data& d)
{
	m_data = d.m_data;
	m_min = d.m_min;
	m_max = d.m_max;
}

//-----------------------------------------------------------------------------
void Mesh_Data::operator = (const Mesh_Data& d)
{
	m_data = d.m_data;
	m_min = d.m_min;
	m_max = d.m_max;
}

//-----------------------------------------------------------------------------
void Mesh_Data::Clear()
{
	m_data.clear();
	m_min = m_max = 0.0;
}

//-----------------------------------------------------------------------------
void Mesh_Data::Init(FSMesh* mesh, double initVal, int initTag)
{
	int NE = mesh->Elements();
	m_data.resize(NE);
	for (int i = 0; i < NE; ++i)
	{
		FSElement& el = mesh->Element(i);
		DATA& di = m_data[i];
		int ne = el.Nodes();
		di.nval = ne;
		di.tag = initTag;
		for (int j = 0; j < ne; ++j)
		{
			di.val[j] = initVal;
		}
	}
}

//-----------------------------------------------------------------------------
void Mesh_Data::Init(FSSurfaceMesh* mesh, double initVal, int initTag)
{
	int NF = mesh->Faces();
	m_data.resize(NF);
	for (int i = 0; i < NF; ++i)
	{
		FSFace& f = mesh->Face(i);
		DATA& di = m_data[i];
		int ne = f.Nodes();
		di.nval = ne;
		di.tag = initTag;
		for (int j = 0; j < ne; ++j)
		{
			di.val[j] = initVal;
		}
	}
}

//-----------------------------------------------------------------------------
// get the average element value
double Mesh_Data::GetElementAverageValue(int elem)
{
	double v = 0.0;
	if (m_data[elem].tag != 0)
	{
		for (int i = 0; i < m_data[elem].nval; ++i) v += m_data[elem].val[i];
		v /= (double)m_data[elem].nval;
	}
	return v;
}

//-----------------------------------------------------------------------------
// set the element (average) value
void Mesh_Data::SetElementValue(int elem, double v)
{
	int ne = m_data[elem].nval;
	for (int i = 0; i < ne; ++i) m_data[elem].val[i] = v;
}

//-----------------------------------------------------------------------------
void Mesh_Data::UpdateValueRange()
{
	m_min = m_max = 0;

	// find the first active value
	int N = (int)m_data.size();
	int i = 0;
	for (i = 0; i<N; ++i)
	{
		if (m_data[i].tag != 0)
		{
			m_min = m_max = m_data[i].val[0];
			break;
		}
	}

	// update range
	for (i=0; i<N; ++i)
	{
		DATA& di = m_data[i];
		if (di.tag != 0)
		{
			for (int j = 0; j < di.nval; ++j)
			{
				if (di.val[j] > m_max) m_max = di.val[j];
				if (di.val[j] < m_min) m_min = di.val[j];
			}
		}
	}
}

//-----------------------------------------------------------------------------
void Mesh_Data::GetValueRange(double& vmin, double& vmax) const
{
	vmin = m_min;
	vmax = m_max;
}

//-----------------------------------------------------------------------------
// default constructor
FSMesh::FSMesh()
{
	m_pobj = 0;
}

//-----------------------------------------------------------------------------
// copy constructor
FSMesh::FSMesh(FSMesh& m)
{
	// create the nodes
	m_Node.resize(m.Nodes());
	for (int i=0; i<Nodes(); ++i) m_Node[i] = m.m_Node[i];

	// create the elements
	m_Elem.resize(m.Elements());
	for (int i = 0; i<Elements(); ++i) m_Elem[i] = m.m_Elem[i];

	// create the faces
	m_Face.resize(m.Faces());
	for (int i = 0; i<Faces(); ++i) m_Face[i] = m.m_Face[i];

	// create the edges
	m_Edge.resize(m.Edges());
	for (int i = 0; i<Edges(); ++i) m_Edge[i] = m.m_Edge[i];

	// copy element data
	m_data = m.m_data;

	// copy bounding box
	m_box = m.m_box;

	// don't copy object (two meshes cannot be owned by the same object)
	m_pobj = 0;
}

//-----------------------------------------------------------------------------
FSMesh::FSMesh(FSSurfaceMesh& m)
{
	int NN = m.Nodes();
	int NF = m.Faces();
	int NE = m.Edges();

	Create(NN, NF, NF, NE);
	for (int i = 0; i < NN; ++i)
	{
		FSNode& node = Node(i);
		FSNode& snode = m.Node(i);
		node = snode;
	}

	for (int i = 0; i < NE; ++i)
	{
		FSEdge& edge = Edge(i);
		FSEdge& sedge = m.Edge(i);
		edge = sedge;
	}

	for (int i = 0; i < NF; ++i)
	{
		FSElement& el = Element(i);
		FSFace& face = Face(i);
		FSFace& sface = m.Face(i);
		el.SetType(FE_TRI3);
		el.m_node[0] = sface.n[0];
		el.m_node[1] = sface.n[1];
		el.m_node[2] = sface.n[2];
		el.m_gid = sface.m_gid;

		face.SetType(FE_FACE_TRI3);
		face.n[0] = sface.n[0];
		face.n[1] = sface.n[1];
		face.n[2] = sface.n[2];
		face.m_gid = sface.m_gid;
	}

	BuildMesh();
}

//-----------------------------------------------------------------------------
// destructor
FSMesh::~FSMesh()
{
	Clear();
}

//-----------------------------------------------------------------------------
// Clear the mesh data
void FSMesh::Clear()
{
	m_Edge.clear();
	m_Face.clear();
	m_Elem.clear();
	m_Node.clear();

	ClearMeshData();
}

//-----------------------------------------------------------------------------
void FSMesh::ClearMeshData()
{
	m_data.Clear();
	for (int i = 0; i < m_meshData.size(); ++i) delete m_meshData[i];
	m_meshData.clear();
}

//-----------------------------------------------------------------------------
// Allocate storage for the mesh data. If bclear is true (default = true) all 
// existing groups are deleted.
void FSMesh::Create(int nodes, int elems, int faces, int edges)
{
	// allocate storage
	if (nodes > 0) { if (nodes) m_Node.resize(nodes); else m_Node.clear(); }
	if (elems > 0) { if (elems) m_Elem.resize(elems); else m_Elem.clear(); }
	if (faces > 0) { if (faces) m_Face.resize(faces); else m_Face.clear(); }
	if (edges > 0) { if (edges) m_Edge.resize(edges); else m_Edge.clear(); }

	// clear mesh data
	ClearMeshData();
}

//-----------------------------------------------------------------------------
void FSMesh::ResizeNodes(int newSize)
{
	m_Node.resize(newSize);
}

//-----------------------------------------------------------------------------
void FSMesh::ResizeEdges(int newSize)
{
	m_Edge.resize(newSize);
}

//-----------------------------------------------------------------------------
void FSMesh::ResizeFaces(int newSize)
{
	m_Face.resize(newSize);
}

//-----------------------------------------------------------------------------
void FSMesh::ResizeElems(int newSize)
{
	m_Elem.resize(newSize);
}

//-----------------------------------------------------------------------------
// This functions update the node GIds to make sure that no indices are skipped.
// This needs to be called after the number of nodes changes.
void FSMesh::UpdateNodePartitions()
{
	// find the largest GID
	int max_gid = -1;
	for (int i=0; i<Nodes(); ++i)
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
	for (int i=0; i<gid.size(); ++i)
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
void FSMesh::UpdateEdgePartitions()
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
void FSMesh::UpdateFacePartitions()
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
void FSMesh::UpdateSmoothingGroups()
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
// This functions update the node GIds to make sure that no indices are skipped.
// This needs to be called after the number of elements changes.
void FSMesh::UpdateElementPartitions()
{
	// find the largest GID
	int max_gid = -1;
	for (int i = 0; i<Elements(); ++i)
	{
		FSElement& elem = Element(i);
		if (elem.m_gid > max_gid) max_gid = elem.m_gid;
	}

	// if no element has a GID we are done
	if (max_gid < 0) return;

	// build a GID lookup table
	vector<int> gid(max_gid + 1, -1);
	for (int i = 0; i<Elements(); ++i)
	{
		FSElement& elem = Element(i);
		if (elem.m_gid >= 0) gid[elem.m_gid] = 1;
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
		for (int i = 0; i<Elements(); ++i)
		{
			FSElement& elem = Element(i);
			if (elem.m_gid >= 0) elem.m_gid = gid[elem.m_gid];
		}
	}
}

//-----------------------------------------------------------------------------
// Remove elements with tag ntag
void FSMesh::RemoveElements(int ntag)
{
	int n = 0;
    bool bdata = (m_data.m_data.size() > 0);
	for (int i = 0; i<Elements(); ++i)
	{
		FSElement& e1 = Element(i);
		FSElement& e2 = Element(n);

		if (e1.m_ntag != ntag)
		{
			if (i != n)
			{
				e2 = e1;
				if (bdata) m_data[n] = m_data[i];
			}
			n++;
		}
	}

	m_Elem.resize(n);
	m_data.Clear();
}

//-----------------------------------------------------------------------------
//! This function identifies duplicate faces and returns a list with the duplicates
void FSMesh::FindDuplicateFaces(vector<int>& l)
{
	l.clear();
	int NF = Faces();
	for (int i=0; i<NF; ++i)
	{
		FSFace& fi = Face(i);
		for (int j=i+1; j<NF; ++j)
		{
			FSFace& fj = Face(j);
			if (fi == fj) l.push_back(j);
		}
	}
}

//-----------------------------------------------------------------------------
//! This function identifies duplicate edges and returns a list with the duplicates
void FSMesh::FindDuplicateEdges(vector<int>& l)
{
	l.clear();
	int NL = Edges();
	for (int i=0; i<NL; ++i)
	{
		FSEdge& ei = Edge(i);
		for (int j=i+1; j<NL; ++j)
		{
			FSEdge& ej = Edge(j);
			if (ei == ej) l.push_back(j);
		}
	}
}

//-----------------------------------------------------------------------------
// Build the node-node table for surface nodes only. That is table of node indices that each node
// connects to.
void FSMesh::BuildSurfaceNodeNodeTable(vector<set<int> >& NNT)
{
	// reset node-node table
	int NN = Nodes();
	NNT.resize(NN);
	for (int i=0; i<NN; ++i) NNT[i].clear();

	// loop over all faces
	int NF = Faces();
	for (int i=0; i<NF; ++i)
	{
		FSFace& f = Face(i);
		int nf = f.Nodes();
		for (int j=0; j<nf; ++j)
		{
			int nj = f.n[j];
			for (int k=0; k<nf; ++k)
			{
				int nk = f.n[k];
				if (nj != nk) NNT[nj].insert(nk);										
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Build mesh data structures
// This assumes that at all nodes, edges, faces, and elements are created and partitioned.
void FSMesh::BuildMesh()
{
	// rebuild element data
	RebuildElementData();

	// rebuild face data
	RebuildFaceData();

	// rebuild edge data
	RebuildEdgeData();

	// rebuild node data
	RebuildNodeData();

	// for good measure, let's also update the mesh
	UpdateMesh();
}

//-----------------------------------------------------------------------------
// Convenience function that calls the mesh builder to do all the work
void FSMesh::RebuildMesh(double smoothingAngle, bool partitionMesh)
{
	FEMeshBuilder meshBuilder(*this);
	meshBuilder.RebuildMesh(smoothingAngle, partitionMesh);
}

//-----------------------------------------------------------------------------
void FSMesh::RebuildElementData()
{
#ifdef _DEBUG
	// make sure element data is valid
	assert(ValidateElements());
#endif

	// Make sure all gids are sequential
	UpdateElementPartitions();

	// Update the element neighbours
	// (Depends on element connectivity)
	UpdateElementNeighbors();

	// mark exterior elements 
	// (Depends on element neighbors)
	MarkExteriorElements();
}

//-----------------------------------------------------------------------------
void FSMesh::RebuildFaceData()
{
#ifdef _DEBUG
	assert(ValidateFaces());
#endif

	// update the face-element connectivity data
	// (Depends on element neighbors)
	UpdateFaceElementTable();

	// update face neighbours
	// (Depends on element neighbors)
	UpdateFaceNeighbors();

	// build the node-face list
	m_NFL.Build(this);
}

//-----------------------------------------------------------------------------
void FSMesh::RebuildEdgeData()
{
#ifdef _DEBUG
	assert(ValidateEdges());
#endif
	// mark the exterior edges
	MarkExteriorEdges();

	// Build the edge neighbors (Only applies to exterior edges)
	UpdateEdgeNeighbors();
}

//-----------------------------------------------------------------------------
void FSMesh::RebuildNodeData()
{
	// Figure out which nodes are interior and which are exterior
	MarkExteriorNodes();
}

//-----------------------------------------------------------------------------
// mesh validation
bool FSMesh::ValidateElements() const
{
	// loop over all elements
	int NN = Nodes();
	int NE = Elements();
	for (int i = 0; i < NE; ++i)
	{
		const FSElement& el = m_Elem[i];

		// see if all elements have IDs assigned
		if (el.m_gid < 0) return false;

		// make sure all node Ids are valid
		int nn = el.Nodes();
		if (nn <= 0) return false;
		for (int j = 0; j < nn; ++j)
		{
			int nj = el.m_node[j];
			if ((nj < 0) || (nj >= NN)) return false;
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
bool FSMesh::ValidateFaces() const
{
	// loop over all faces
	int NN = Nodes();
	int NF = Faces();
	for (int i = 0; i < NF; ++i)
	{
		const FSFace& face = m_Face[i];

		// see if all faces have IDs assigned
		if (face.m_gid < 0) return false;

		// make sure all node Ids are valid
		int nn = face.Nodes();
		if (nn <= 0) return false;
		for (int j = 0; j < nn; ++j)
		{
			int nj = face.n[j];
			if ((nj < 0) || (nj >= NN)) return false;
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
bool FSMesh::ValidateEdges() const
{
	// loop over all edges
	int NN = Nodes();
	int NE = Edges();
	for (int i = 0; i < NE; ++i)
	{
		const FSEdge& edge = m_Edge[i];

		// make sure all node Ids are valid
		int nn = edge.Nodes();
		if (nn <= 0) return false;
		for (int j = 0; j < nn; ++j)
		{
			int nj = edge.n[j];
			if ((nj < 0) || (nj >= NN)) return false;
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
// This function finds the element neighbours.
//
void FSMesh::UpdateElementNeighbors()
{
	// get number of elements
	int elems = Elements();

	// reset all element neighbor and face ptrs
	for (int i = 0; i < elems; i++)
	{
		FEElement_& el = ElementRef(i);
		el.m_ntag = i;
		for (int j = 0; j < 6; ++j)
		{
			el.m_nbr[j] = -1;
			el.m_face[j] = -1;
		}
	}

	// calculate the node-element table
	FSNodeElementList NET;
	NET.Build(this);

	// set up the element's neighbour pointers
	FSEdge edge;
	FSFace f1, f2, f3;

	// loop over all elements
	for (int i = 0; i < elems; i++)
	{
		FEElement_* pe = ElementPtr(i);
		// do the solid elements first
		int n = pe->Faces();
		for (int j = 0; j < n; j++)
		{
			// check if we already have a neighbor assigned
			if (pe->m_nbr[j] == -1)
			{
				// get the corresponding element face
				pe->GetFace(j, f1);

				// pick a node on the face
				int inode = f1.n[0];
				int nval = NET.Valence(inode);
				bool bfound = false;

				// search for shell neighbors first
				// This is necessary since a shell can share a face with a solid. 
				// This requires a bit of special handling, so we need to check for that first
				for (int k = 0; k < nval; k++)
				{
					int nbe = NET.ElementIndex(inode, k);
					FEElement_* pne = NET.Element(inode, k);
					if ((pne != pe) && pne->IsShell())
					{
						// get the shell surface facet
						pne->GetShellFace(f2);
						if (f1 == f2)
						{
							bfound = true;
							pe->m_nbr[j] = nbe;
							break;
						}
					}
				}

				if (bfound == false)
				{
					// search for solid neighbors next
					for (int k = 0; k < nval; k++)
					{
						int nbe = NET.ElementIndex(inode, k);
						FEElement_* pne = ElementPtr(nbe);
						if ((pne != pe) && pne->IsSolid())
						{
							int l = pne->FindFace(f1);
							if (l != -1)
							{
								bfound = true;
								pe->m_nbr[j] = nbe;
								pne->m_nbr[l] = i;
								break;
							}
						}
					}
				}
			}
		}

		// do the shell elements next
		n = pe->Edges();
		for (int j = 0; j < n; j++)
		{
			if (pe->m_nbr[j] == -1)
			{
				edge = pe->GetEdge(j);

				// find the neighbour element
				int inode = edge.n[0];
				int nval = NET.Valence(inode);
				bool bfound = false;
				for (int k = 0; k < nval; k++)
				{
					FEElement_* pne = NET.Element(inode, k);
					if ((pne != pe) && (pe->is_equal(*pne) == false) && pne->IsShell())
					{
						int l = pne->FindEdge(edge);
						if (l != -1)
						{
							bfound = true;
							pe->m_nbr[j] = NET.ElementIndex(inode, k);
							pne->m_nbr[l] = i;
							break;
						}
					}
				}
			}
		}

		// do the beam elements
		if (pe->IsType(FE_BEAM2))
		{
			for (int j = 0; j < 2; ++j)
			{
				pe->m_nbr[j] = -1;
				pe->m_face[j] = -1;
				int inode = pe->m_node[j];
				int nval = NET.Valence(inode);
				for (int k = 0; k < nval; ++k)
				{
					FEElement_* pne = NET.Element(inode, k);
					if (pne != pe)
					{
						if ((pne->IsType(FE_BEAM2)) && ((pne->m_node[0] == pe->m_node[j]) || (pne->m_node[1] == pe->m_node[j])))
						{
							pe->m_nbr[j] = NET.ElementIndex(inode, k);
							break;
						}
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FSMesh::MarkExteriorElements()
{
	// set exterior flags
	for (int i = 0; i < Elements(); ++i)
	{
		FSElement& el = Element(i);
		if (el.IsSolid())
		{
			el.SetExterior(false);
			int nn = el.Faces();
			for (int j = 0; j < nn; ++j)
			{
				if (el.m_nbr[j] < 0)
				{
					el.SetExterior(true);
					break;
				}
			}
		}
		else el.SetExterior(true);
	}
}

//-----------------------------------------------------------------------------
void FSMesh::MarkExteriorFaces()
{
	for (int i = 0; i < Faces(); ++i)
	{
		FSFace& face = Face(i);
		face.SetExterior(face.m_elem[1].eid == -1);
	}
}

//-----------------------------------------------------------------------------
void FSMesh::MarkExteriorEdges()
{
	for (int i = 0; i < Edges(); ++i)
	{
		FSEdge& edge = Edge(i);
		edge.SetExterior(edge.m_gid >= 0);
	}
}

//-----------------------------------------------------------------------------
// helper function for checking if two faces can be neighbours
bool isValidFaceNeighbor(FSFace& f0, FSFace& f1)
{
	// Make sure they are both external or both non-external
	if (f0.IsExternal() != f1.IsExternal()) return false;

	// Make sure they are not the same
	if (f0 == f1) return false;

	// check some special cases
	if ((f0.Nodes() == 3) && (f1.Nodes() == 4))
	{
		// see if f0 overlaps with f1
		if ((f1.FindNode(f0.n[0]) != -1) && (f1.FindNode(f0.n[1]) != -1) && (f1.FindNode(f0.n[2]) != -1)) return false;	
	}

	if ((f1.Nodes() == 3) && (f0.Nodes() == 4))
	{
		// see if f1 overlaps with f0
		if ((f0.FindNode(f1.n[0]) != -1) && (f0.FindNode(f1.n[1]) != -1) && (f0.FindNode(f1.n[2]) != -1) ) return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// This function sets the face-element connectivity
void FSMesh::UpdateFaceElementTable()
{
	int NF = Faces(); 
	int NE = Elements();
	if ((NF == 0) || (NE == 0)) return;

	// clear all face-element connectivity
	for (int i = 0; i<NF; ++i)
	{
		FSFace& f = Face(i);
		f.m_elem[0].eid = -1; f.m_elem[0].lid = -1;
		f.m_elem[1].eid = -1; f.m_elem[1].lid = -1;
		f.m_elem[2].eid = -1; f.m_elem[2].lid = -1;
	}

	for (int i = 0; i<NE; ++i)
	{
		FSElement& el = Element(i);

		// solid elements
		int nf = el.Faces();
		for (int i = 0; i<nf; ++i)
		{
			el.m_face[i] = -1;
		}

		// shell elements
		int ne = el.Edges();
		if (ne > 0)
		{
			el.m_face[0] = -1;
		}
	}

	// first build the node element table
	FSNodeElementList NET;
	NET.Build(this);

	// loop over all faces
	FSFace f2;
	for (int i = 0; i<NF; ++i)
	{
		FSFace& face = Face(i);

		int n0 = face.n[0];
		int nval = NET.Valence(n0);
		int m = 0;

		// check shell elements first
		for (int j = 0; j < nval; ++j)
		{
			int eid = NET.ElementIndex(n0, j);
			FEElement_* pej = ElementPtr(eid);

			if (pej->IsShell())
			{
				pej->GetShellFace(f2);
				if (f2 == face)
				{
					assert(m == 0);
					if (m == 0)
					{
						face.m_elem[m].eid = eid;
						face.m_elem[m++].lid = 0;
						pej->m_face[0] = i;
					}
					break;
				}
			}
		}

		// now, process solids
		for (int j=0; j<nval; ++j)
		{
			int eid = NET.ElementIndex(n0, j);
			FEElement_* pej = ElementPtr(eid);

			// solid elements
			int n = pej->Faces();
			for (int k = 0; k<n; ++k)
			{
				if (pej->m_face[k] == -1)
				{
					pej->GetFace(k, f2);
					if (f2 == face)
					{
						assert(m<3);
						if (m == 0)
						{
							face.m_elem[m  ].eid = eid;
							face.m_elem[m++].lid = k;
						}
						else if (m < 2)
						{
							// set the element with the lowest GID first (except if it is a shell)
							FEElement_* p0 = ElementPtr(face.m_elem[0].eid);
							if ((p0->m_gid < pej->m_gid) || (p0->IsShell()))
							{
								face.m_elem[m  ].eid = eid;
								face.m_elem[m++].lid = k;
							}
							else
							{
								face.m_elem[m  ].eid = face.m_elem[0].eid;
								face.m_elem[m++].lid = face.m_elem[0].lid;

								face.m_elem[0].eid = eid;
								face.m_elem[0].lid = k;
							}
						}
						else if (m < 3)
						{
							// The only way to get here is if a shell is sandwhiched between
							// two solids. The shell should have already been found.
							FEElement_* p0 = ElementPtr(face.m_elem[0].eid);
							assert(p0 && p0->IsShell());

							// for consistency, we set the solid with the lowest GID first.
							FEElement_* p1 = ElementPtr(face.m_elem[1].eid); assert(p1 && p1->IsSolid());
							if (p1->m_gid < pej->m_gid)
							{
								face.m_elem[m  ].eid = eid;
								face.m_elem[m++].lid = k;
							}
							else
							{
								face.m_elem[m  ].eid = face.m_elem[1].eid;
								face.m_elem[m++].lid = face.m_elem[1].lid;

								face.m_elem[1].eid = eid;
								face.m_elem[1].lid = k;
							}
						}
						pej->m_face[k] = i;
					}
				}
			}
		}

		assert(face.m_elem[0].eid != -1);
	}

	MarkExteriorFaces();
}

//-----------------------------------------------------------------------------
//! This function finds the face neighbours. Note that internal faces cannot
//! be neighbours of external faces. 
void FSMesh::UpdateFaceNeighbors()
{
	int NF = Faces();

	// make sure we have faces
	if (NF == 0) return;

	// Tag faces based on their part connectivity
	TagAllFaces(0);
	TagAllElements(0);
	stack<int> S;
	S.push(0);
	int np = 1;
	int i0 = 1;
	do
	{
		while (S.empty() == false)
		{
			int elem = S.top(); S.pop();
			FSElement& el = Element(elem);
			el.m_ntag = 1;

			int nf = el.Faces();
			for (int i=0; i<nf; ++i)
			{
				if (el.m_face[i] != -1)
				{
					Face(el.m_face[i]).m_ntag = np;
				}

				if (el.m_nbr[i] != -1)
				{
					FSElement& ej = Element(el.m_nbr[i]);
					if (ej.m_ntag == 0)
					{
						ej.m_ntag = 1;
						S.push(el.m_nbr[i]);
					}
				}
			}
		}
		np++;

		for (int i=i0; i<Elements(); ++i)
		{
			FSElement& el = Element(i);
			if (el.m_ntag == 0)
			{
				S.push(i);
				i0 = i + 1;
				break;
			}
		}
	}
	while (S.empty() == false);

	// build the node-face table
	FSNodeFaceList NFT;
	NFT.Build(this);

	// find all face neighbours
	int n[4];
	for (int i = 0; i<NF; ++i)
	{
		FSFace* pf = FacePtr(i);

		int ne = pf->Edges();
		for (int j = 0; j<ne; ++j)
		{
			pf->GetEdgeNodes(j, n);
			int nval = NFT.Valence(n[0]);
			pf->m_nbr[j] = -1;
			for (int k = 0; k<nval; ++k)
			{
				FSFace* pfn = NFT.Face(n[0], k);
				if (pfn != pf)
				{
					// See if the faces share an edge
					if (pfn->HasEdge(n[0], n[1]) && (pf->m_ntag == pfn->m_ntag))
					{
						// see if they are both external or both internal
						if (isValidFaceNeighbor(*pf, *pfn))
						{
							pf->m_nbr[j] = NFT.FaceIndex(n[0], k);
							break;
						}
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// This function finds the edge neighbours.
void FSMesh::UpdateEdgeNeighbors()
{
	FSNodeEdgeList NET;
	NET.Build(this, true);

	for (int i = 0; i<Edges(); ++i)
	{
		FSEdge& edge = Edge(i);
		edge.m_nbr[0] = -1;
		edge.m_nbr[1] = -1;
	}

	for (int i = 0; i<Edges(); ++i)
	{
		FSEdge& edge = Edge(i);
		if (edge.m_gid >= 0)
		{
			for (int j = 0; j<2; ++j)
			{
				int nj = edge.n[j]; assert(nj != -1);
				int val = NET.Edges(nj);
				if (val == 2)
				{
					assert((NET.EdgeIndex(nj, 0) == i) || (NET.EdgeIndex(nj, 1) == i));
					int nk = NET.EdgeIndex(nj, 0);
					if (nk == i) nk = NET.EdgeIndex(nj, 1); assert(nk != i);
					assert(Edge(nk).IsExterior());
					if (Edge(nk).m_gid == edge.m_gid)
					{
						edge.m_nbr[j] = nk;
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// select elements based on face selection
vector<int> FSMesh::GetElementsFromSelectedFaces()
{
	// tag elements for selection
	vector<bool> tag(Elements(), false);

	// find the selected faces
	int faces = Faces();
	for (int i = 0; i<faces; ++i)
	{
		FSFace& face = Face(i);
		if (face.IsSelected())
		{
			tag[face.m_elem[0].eid] = true;
		}
	}

	// select all the tagged elements
	vector<int> selection;
	for (int i = 0; i < Elements(); ++i)
	{
		if (tag[i]) selection.push_back(i);
	}

	return selection;
}

//-----------------------------------------------------------------------------
// Extract faces as a shell mesh
FSMesh* FSMesh::ExtractFaces(bool selectedOnly)
{
	// clear face tags
	TagAllFaces(0);

	// count selected faces
	int faces = 0;
	if (selectedOnly)
	{
		for (int i=0; i<Faces(); ++i) if (Face(i).IsSelected()) { Face(i).m_ntag = 1; ++faces; }
	}
	else
	{
		faces = Faces();
		for (int i=0; i<Faces(); ++i) Face(i).m_ntag = 1;
	}

	// tag nodes that need to be copied
	for (int i=0; i<Nodes(); ++i) Node(i).m_ntag = -1;
	for (int i=0; i<Faces(); ++i)
	{
		FSFace& f = Face(i);
		if (f.m_ntag == 1)
		{
			int n = f.Nodes();
			for (int j=0; j<n; ++j) Node(f.n[j]).m_ntag = 1;
		}
	}

	// count nodes
	int nodes = 0;
	for (int i=0; i<Nodes(); ++i) 
	{
		FSNode& node = Node(i);
		if (node.m_ntag == 1) 
		{
			node.m_ntag = nodes;
			++nodes;
		}
	}

	assert( (nodes>0) && (faces>0));

	// allocate new mesh
	FSMesh* pm = new FSMesh();
	pm->Create(nodes, faces);

	// create the nodes
	FSNode* pn = pm->NodePtr();
	for (int i=0; i<Nodes(); ++i)
	{
		FSNode& node = Node(i);
		if (node.m_ntag >= 0)
		{
			*pn = node;
			++pn;
		}
	}

	// create the elements
	int eid = 0;
	for (int i=0; i<Faces(); ++i)
	{
		FSFace& face = Face(i);
		if (face.m_ntag)
		{
			FEElement_* pe = pm->ElementPtr(eid++);

			int n = face.Nodes();
			switch (n)
			{
			case 3: pe->SetType(FE_TRI3 ); break;
			case 4: pe->SetType(FE_QUAD4); break;
			case 6: pe->SetType(FE_TRI6 ); break;
			case 7: pe->SetType(FE_TRI7 ); break;
			case 8: pe->SetType(FE_QUAD8); break;
			case 9: pe->SetType(FE_QUAD9); break;
			default:
				assert(false);
				delete pm;
				return 0;
			}

			for (int j=0; j<n; ++j) pe->m_node[j] = Node(face.n[j]).m_ntag;
		}
	}

	// rebuild the mesh
	pm->RebuildMesh();

	return pm;
}

//-----------------------------------------------------------------------------
// Save mesh data to archive
//
void FSMesh::Save(OArchive &ar)
{
	int nodes = Nodes();
	int elems = Elements();
	int faces = Faces();
	int edges = Edges();

	// write the header
	ar.BeginChunk(CID_MESH_HEADER);
	{
		ar.WriteChunk(CID_MESH_NODES, nodes);
		ar.WriteChunk(CID_MESH_ELEMENTS, elems);
		ar.WriteChunk(CID_MESH_FACES, faces);
		ar.WriteChunk(CID_MESH_EDGES, edges);
	}
	ar.EndChunk();

	// write the nodes
	ar.BeginChunk(CID_MESH_NODE_SECTION);
	{
		FSNode* pn = NodePtr();
		for (int i=0; i<nodes; ++i, ++pn)
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

	// write the elements
	ar.BeginChunk(CID_MESH_ELEMENT_SECTION);
	{
		for (int i=0; i<elems; ++i)
		{
			FEElement_* pe = ElementPtr(i);

			ar.BeginChunk(CID_MESH_ELEMENT);
			{
				int ntype = pe->Type();
				ar.WriteChunk(CID_MESH_ELEMENT_TYPE    , ntype);
				ar.WriteChunk(CID_MESH_ELEMENT_GID     , pe->m_gid);
				ar.WriteChunk(CID_MESH_ELEMENT_NODES   , pe->m_node, pe->Nodes());
				ar.WriteChunk(CID_MESH_ELEMENT_FIBER   , pe->m_fiber);
				ar.WriteChunk(CID_MESH_ELEMENT_Q_ACTIVE, pe->m_Qactive);
				ar.WriteChunk(CID_MESH_ELEMENT_Q       , pe->m_Q);
				if (pe->IsShell())
					ar.WriteChunk(CID_MESH_SHELL_THICKNESS , pe->m_h, pe->Nodes());
//				ar.WriteChunk(CID_MESH_ELEMENT_MATERIAL, mid);
			}
			ar.EndChunk();
		}
	}
	ar.EndChunk();

	// write the faces
	ar.BeginChunk(CID_MESH_FACE_SECTION);
	{
		FSFace* pf = FacePtr();
		for (int i=0; i<faces; ++i, ++pf)
		{
			ar.BeginChunk(CID_MESH_FACE);
			{
				int nn = pf->Nodes();
				int ntype = pf->Type();
				assert(ntype != FE_FACE_INVALID_TYPE);
				ar.WriteChunk(CID_MESH_FACE_TYPE    , ntype);
				ar.WriteChunk(CID_MESH_FACE_GID     , pf->m_gid);
				ar.WriteChunk(CID_MESH_FACE_NODES   , pf->n, pf->Nodes());
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
		for (int i=0; i<edges; ++i, ++pe)
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

	// TODO: Move this stuff to the GObject serialization
	GObject* po = GetGObject();
	int parts = po->FEParts();
	int surfs = po->FESurfaces();
	int nsets = po->FENodeSets();

	// write the parts
	if (parts > 0)
	{
		ar.BeginChunk(CID_MESH_PART_SECTION);
		{
			for (int i=0; i<parts; ++i)
			{
				// get the boundary condition
				FSPart* pg = po->GetFEPart(i);

				// store the group data
				ar.BeginChunk(CID_MESH_PART);
				{
					pg->Save(ar);
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// write the surfaces
	if (surfs > 0)
	{
		ar.BeginChunk(CID_MESH_SURF_SECTION);
		{
			for (int i=0; i<surfs; ++i)
			{
				// get the boundary condition
				FSSurface* pg = po->GetFESurface(i);

				// store the group data
				ar.BeginChunk(CID_MESH_SURFACE);
				{
					pg->Save(ar);
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// write the parts
	if (nsets > 0)
	{
		ar.BeginChunk(CID_MESH_NSET_SECTION);
		{
			for (int i=0; i<nsets; ++i)
			{
				// get the boundary condition
				FSNodeSet* pg = po->GetFENodeSet(i);

				// store the group data
				ar.BeginChunk(CID_MESH_NODESET);
				{
					pg->Save(ar);
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// write the mesh data 
	if (MeshDataFields() > 0)
	{
		ar.BeginChunk(CID_MESH_DATA_SECTION);
		{
			// node data
			for (int n = 0; n<(int)m_meshData.size(); ++n)
			{
				FEMeshData* meshData = m_meshData[n];
				switch (meshData->GetDataClass())
				{
				case FEMeshData::NODE_DATA:
				{
					FENodeData* map = dynamic_cast<FENodeData*>(meshData); assert(map);
					ar.BeginChunk(CID_MESH_NODE_DATA);
					{
						map->Save(ar);
					}
					ar.EndChunk();
				}
				break;
				case FEMeshData::SURFACE_DATA:
				{
					FESurfaceData* map = dynamic_cast<FESurfaceData*>(meshData); assert(map);
					ar.BeginChunk(CID_MESH_SURFACE_DATA);
					{
						map->Save(ar);
					}
					ar.EndChunk();
				}
				break;
				case FEMeshData::ELEMENT_DATA:
				{
					FEElementData* map = dynamic_cast<FEElementData*>(meshData); assert(map);
					ar.BeginChunk(CID_MESH_ELEM_DATA);
					{
						map->Save(ar);
					}
					ar.EndChunk();
				}
				break;
				case FEMeshData::PART_DATA:
				{
					FEPartData* map = dynamic_cast<FEPartData*>(meshData); assert(map);
					ar.BeginChunk(CID_MESH_PART_DATA);
					{
						map->Save(ar);
					}
					ar.EndChunk();
				}
				break;
				}
			}
		}
		ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
// Load mesh data from archive
//
void FSMesh::Load(IArchive& ar)
{
	TRACE("FSMesh::Load");

	int nodes;
	int elems;
	int faces;
	int edges;

	GObject* po = GetGObject();
	vec3d pos;
	quatd rot;

	// the first chunk must be the header
	ar.OpenChunk();
	assert(ar.GetChunkID() == CID_MESH_HEADER);
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch(nid)
		{
		case CID_MESH_NODES: ar.read(nodes); break;
		case CID_MESH_ELEMENTS: ar.read(elems); break;
		case CID_MESH_FACES: ar.read(faces); break;
		case CID_MESH_EDGES: ar.read(edges); break;
		}
		ar.CloseChunk();
	}
	ar.CloseChunk();

	// allocate storage
	Create(nodes, elems,faces, edges);

	// buffer for storing shell thickness
	// NOTE: Problem is that in older versions pe->Nodes() values 
	// were stored. But the max buffer size for shell thickness is 9
	// so this could crash PreView when reading shell thickness values for elements
	// that have more than 9 nodes.
	vector<double> h(FSElement::MAX_NODES);

	// read the rest of the mesh data
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch(nid)
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
		case CID_MESH_ELEMENT_SECTION:
			{
				int n = 0;
				FSElement* pe = &m_Elem[0];
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int nid = ar.GetChunkID();
					if (nid == CID_MESH_ELEMENT)
					{
						assert(n < elems);
						while (IArchive::IO_OK == ar.OpenChunk())
						{
							int nid = ar.GetChunkID();
							switch (nid)
							{
							case CID_MESH_ELEMENT_TYPE: { int ntype; ar.read(ntype); pe->SetType(ntype); }; break;
							case CID_MESH_ELEMENT_GID: ar.read(pe->m_gid); break;
							case CID_MESH_ELEMENT_NODES: 
								{
									if (ar.Version() < 0x00010008) ar.read(pe->m_node, 8);
									else ar.read(pe->m_node, pe->Nodes());
								}
								break;
							case CID_MESH_ELEMENT_FIBER   : ar.read(pe->m_fiber); break;
							case CID_MESH_ELEMENT_Q_ACTIVE: ar.read(pe->m_Qactive); break;
							case CID_MESH_ELEMENT_Q       :  ar.read(pe->m_Q); break;

							case CID_MESH_SHELL_THICKNESS:
								{
									ar.read(&h[0], pe->Nodes());
									int n = pe->Nodes();
									if (n > 9) n = 9;
									for (int i=0; i<n; ++i) pe->m_h[i] = h[i];
								}
								break;
//							case CID_MESH_ELEMENT_MATERIAL: ar.read(pe->m_matid); break;
							}
							ar.CloseChunk();
						}
						++n;
						++pe;
					}
					else assert(false);
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
					if (nid != CID_MESH_FACE) throw ReadError("error parsing CID_MESH_FACE_SECTION (FSMesh::Load)");

					while (IArchive::IO_OK == ar.OpenChunk())
					{
						int nid = ar.GetChunkID();
						int ntype;
						switch (nid)
						{
						case CID_MESH_FACE_TYPE: 
							{
								ar.read(ntype); 
								if (ar.Version() < 0x00020000)
								{
									switch (ntype)
									{
									case FE_TRI3 : pf->SetType(FE_FACE_TRI3 ); break;
									case FE_QUAD4: pf->SetType(FE_FACE_QUAD4); break;
									case FE_TRI6 : pf->SetType(FE_FACE_TRI6 ); break;
									case FE_QUAD8: pf->SetType(FE_FACE_QUAD8); break;
									case FE_TRI7 : pf->SetType(FE_FACE_TRI7 ); break;
									case FE_QUAD9: pf->SetType(FE_FACE_QUAD9); break;
									case FE_TRI10: pf->SetType(FE_FACE_TRI10); break;
									default:
										assert(false);
									}
								}
								else
								{
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
									}
								}
							}
							break;
						case CID_MESH_FACE_GID     : ar.read(pf->m_gid); break;
						case CID_MESH_FACE_NODES   : ar.read(pf->n, pf->Nodes()); break;
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
					if (nid != CID_MESH_EDGE) throw ReadError("error parsing CID_MESH_EDGE_SECTION (FSMesh::Load)");

					int ntype;
					while (IArchive::IO_OK == ar.OpenChunk())
					{
						int nid = ar.GetChunkID();
						switch (nid)
						{
						case CID_MESH_EDGE_TYPE: 
						{ 
							ar.read(ntype); 
							if (ar.Version() < 0x00020000)
							{
								switch (ntype)
								{
								case FE_BEAM2: pe->SetType(FE_EDGE2);  break;
								case FE_BEAM3: pe->SetType(FE_EDGE3);  break;
								default:
									assert(false);
									throw ReadError("error parsing CID_MESH_EDGE_SECTION (FSMesh::Load)");
								}
							}
							else
							{
								switch (ntype)
								{
								case FE_EDGE2: pe->SetType(FE_EDGE2);  break;
								case FE_EDGE3: pe->SetType(FE_EDGE3);  break;
								case FE_EDGE4: pe->SetType(FE_EDGE4);  break;
								default:
									assert(false);
									throw ReadError("error parsing CID_MESH_EDGE_SECTION (FSMesh::Load)");
								}
							}
						} 
						break;
						case CID_MESH_EDGE_GID: ar.read(pe->m_gid); break;
						case CID_MESH_EDGE_NODES: 
							{
								int nn = pe->Nodes();
								assert(nn > 0);
								if (nn <= 0) throw ReadError("error parsing CID_MESH_EDGE_SECTION (FSMesh::Load)");
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
		case CID_MESH_PART_SECTION:
			{
				// TODO: move to GObject serialization
				FSPart* pg = 0;
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					pg = 0;
					assert(ar.GetChunkID() == CID_MESH_PART);
					pg = new FSPart(po);
					pg->Load(ar);
					po->AddFEPart(pg);

					ar.CloseChunk();
				}			
			}
			break;
		case CID_MESH_SURF_SECTION:
			{
				// TODO: move to GObject serialization
				FSSurface* pg = 0;
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					pg = 0;
					assert(ar.GetChunkID() == CID_MESH_SURFACE);
					pg = new FSSurface(po);
					pg->Load(ar);
					po->AddFESurface(pg);

					ar.CloseChunk();
				}			
			}
			break;
		case CID_MESH_NSET_SECTION:
			{
				// TODO: move to GObject serialization
				FSNodeSet* pg = 0;
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					pg = 0;
					assert(ar.GetChunkID() == CID_MESH_NODESET);
					pg = new FSNodeSet(po);
					pg->Load(ar);
					po->AddFENodeSet(pg);

					ar.CloseChunk();
				}			
			}
			break;
		case CID_MESH_DATA_SECTION:
			{
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int nid = ar.GetChunkID();
					switch (nid)
					{
					case CID_MESH_NODE_DATA:
						{
							FENodeData* pmap = AddNodeDataField("(unnamed)");
							int NN = Nodes();
							pmap->Create(NN);
							pmap->Load(ar);
						}
						break;
					case CID_MESH_SURFACE_DATA:
						{
							FESurfaceData* pmap = new FESurfaceData(this);
//							FESurfaceData* pmap = AddSurfaceDataField("(unnamed)", nullptr, FEMeshData::DATA_TYPE::DATA_SCALAR);
//							int NF = Faces();
//							pmap->Create(this, NF);
							pmap->Load(ar);
							m_meshData.push_back(pmap);
						}
						break;
					case CID_MESH_ELEM_DATA:
						{
							FEElementData* pmap = new FEElementData(this);
							pmap->Load(ar);
							m_meshData.push_back(pmap);
						}
						break;
					case CID_MESH_PART_DATA:
						{
							FEPartData* pmap = new FEPartData(this);
							pmap->Load(ar);
							m_meshData.push_back(pmap);
						}
						break;
					}

					ar.CloseChunk();
				}
			}
			break;
		}
		ar.CloseChunk();
	}

	// rebuild mesh' data
	BuildMesh();
}

//-----------------------------------------------------------------------------
// Create a shallow-copy of the mesh
void FSMesh::ShallowCopy(FSMesh* pm)
{
	m_Node = pm->m_Node;
	m_Edge = pm->m_Edge;
	m_Face = pm->m_Face;
	m_Elem = pm->m_Elem;

	m_data = pm->m_data;

	m_box = pm->m_box;
}

//-----------------------------------------------------------------------------
int FSMesh::MeshDataFields() const { return (int)m_meshData.size(); }

//-----------------------------------------------------------------------------
FEMeshData* FSMesh::GetMeshDataField(int i) { return m_meshData[i]; }

//-----------------------------------------------------------------------------
Mesh_Data& FSMesh::GetMeshData() { return m_data; }

//-----------------------------------------------------------------------------
FEMeshData* FSMesh::FindMeshDataField(const string& sz)
{
	if (m_meshData.empty()) return 0;
	for (int i = 0; i<m_meshData.size(); ++i)
	{
		const string& name = m_meshData[i]->GetName();
		if (name == sz) return m_meshData[i];
	}
	return 0;
}

//-----------------------------------------------------------------------------
void FSMesh::RemoveMeshDataField(int i)
{
	m_meshData.erase(m_meshData.begin() + i);
}

//-----------------------------------------------------------------------------
int FSMesh::GetMeshDataIndex(FEMeshData* data)
{
	for (int i = 0; i < m_meshData.size(); ++i)
		if (m_meshData[i] == data) return i;
	return -1;
}

//-----------------------------------------------------------------------------
void FSMesh::InsertMeshData(int i, FEMeshData* data)
{
	m_meshData.insert(m_meshData.begin() + i, data);
}

//-----------------------------------------------------------------------------
void FSMesh::AddMeshDataField(FEMeshData* data)
{
	assert(data);
	if (data) m_meshData.push_back(data);
}

//-----------------------------------------------------------------------------
FENodeData* FSMesh::AddNodeDataField(const string& sz, double v)
{
	FENodeData* data = new FENodeData(GetGObject());
	data->Create(v);
	data->SetName(sz);
	m_meshData.push_back(data);
	return data;
}

//-----------------------------------------------------------------------------
FENodeData* FSMesh::AddNodeDataField(const string& name, FSNodeSet* nodeset, FEMeshData::DATA_TYPE dataType)
{
	FENodeData* data = new FENodeData(GetGObject());
	data->Create(nodeset, 0.0);
	data->SetName(name);
	m_meshData.push_back(data);
	return data;
}

//-----------------------------------------------------------------------------
FESurfaceData* FSMesh::AddSurfaceDataField(const string& name, FSSurface* surface, FEMeshData::DATA_TYPE dataType)
{
	FESurfaceData* data = new FESurfaceData;
	data->Create(this, surface, dataType);
	data->SetName(name);
	m_meshData.push_back(data);
	return data;
}

//-----------------------------------------------------------------------------
FEElementData* FSMesh::AddElementDataField(const string& sz, FSPart* part, FEMeshData::DATA_TYPE dataType)
{
	FEElementData* map = new FEElementData;
	map->Create(this, part, dataType);
	map->SetName(sz);
	m_meshData.push_back(map);
	return map;
}

//-----------------------------------------------------------------------------
FSMesh* ConvertSurfaceToMesh(FSSurfaceMesh* surfaceMesh)
{
	int nodes = surfaceMesh->Nodes();
	int faces = surfaceMesh->Faces();

	FSMesh* mesh = new FSMesh;
	mesh->Create(nodes, faces);

	for (int i = 0; i<nodes; ++i)
	{
		FSNode& surfNode = surfaceMesh->Node(i);
		FSNode& meshNode = mesh->Node(i);

		meshNode = surfNode;
	}

	for (int i = 0; i<faces; ++i)
	{
		FSFace& face = surfaceMesh->Face(i);
		FSElement& el = mesh->Element(i);

		el.m_gid = face.m_gid;

		int nf = face.Nodes();
		switch (face.Nodes())
		{
		case 3: el.SetType(FE_TRI3); break;
		case 4: el.SetType(FE_QUAD4); break;
		default:
			assert(false);
			delete mesh;
			return 0;
		}

		for (int j = 0; j<nf; ++j) el.m_node[j] = face.n[j];
	}

	mesh->RebuildMesh();

	return mesh;
}

//-----------------------------------------------------------------------------
int FSMesh::CountSelectedElements() const
{
	int N = 0, NE = Elements();
	for (int i = 0; i < NE; ++i)
	{
		if (Element(i).IsSelected()) N++;
	}
	return N;
}

//-----------------------------------------------------------------------------
void FSMesh::SetUniformShellThickness(double h)
{
	for (int i = 0; i < Elements(); ++i)
	{
		FSElement& el = Element(i);
		int ne = el.Nodes();
		for (int j = 0; j < ne; ++j) el.m_h[j] = h;
	}
}
