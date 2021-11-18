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

#include "stdafx.h"
#include "FEMultiQuadMesh.h"
#include <MeshLib/FEMesh.h>
#include <MeshLib/FENodeNodeList.h>
#include <GeomLib/geom.h>

//-----------------------------------------------------------------------------
FEMultiQuadMesh::FEMultiQuadMesh()
{
	m_po = nullptr;
	m_pm = nullptr;
	m_elemType = FE_QUAD4;
	m_bquadMesh = false;
}

//-----------------------------------------------------------------------------
FEMultiQuadMesh::~FEMultiQuadMesh(void)
{
}

//-----------------------------------------------------------------------------
// set the quad mesh flag
void FEMultiQuadMesh::SetElementType(int elemType)
{
	m_elemType = elemType;
	m_bquadMesh = (elemType != FE_QUAD4);
}

//-----------------------------------------------------------------------------
MBNode& FEMultiQuadMesh::AddNode(const vec3d& r, int ntype)
{
	MBNode node;
	node.m_r = r;
	node.m_type = ntype;
	m_MBNode.push_back(node);
	return m_MBNode[m_MBNode.size() - 1];
}

//-----------------------------------------------------------------------------
MBFace& FEMultiQuadMesh::AddFace(int n0, int n1, int n2, int n3)
{
	MBFace face;
	face.m_node[0] = n0;
	face.m_node[1] = n1;
	face.m_node[2] = n2;
	face.m_node[3] = n3;
	m_MBFace.push_back(face);
	return m_MBFace[m_MBFace.size() - 1];
}

MBFace& FEMultiQuadMesh::GetFace(int n)
{
	return m_MBFace[n];
}

//-----------------------------------------------------------------------------
// build the FE mesh
//
FEMesh* FEMultiQuadMesh::BuildMesh()
{
	if ((m_elemType != FE_QUAD4) && (m_elemType != FE_QUAD8) && (m_elemType != FE_QUAD9))
	{
		assert(false);
		return nullptr;
	}
	m_bquadMesh = (m_elemType == FE_QUAD4 ? false : true);

	// create a new mesh
	FEMesh* pm = new FEMesh();
	m_pm = pm;

	// clear the node lists
	for (int i = 0; i < m_MBNode.size(); ++i) m_MBNode[i].m_fenodes.clear();
	for (int i = 0; i < m_MBEdge.size(); ++i) m_MBEdge[i].m_fenodes.clear();
	for (int i = 0; i < m_MBFace.size(); ++i) m_MBFace[i].m_fenodes.clear();

	// build the mesh
	BuildFENodes(pm);
	BuildFEEdges(pm);
	BuildFEFaces(pm);

	assert(m_nodes == pm->Nodes());

	// update the mesh
	pm->BuildMesh();

	return pm;
}

//-----------------------------------------------------------------------------
// build the mesh from the object
bool FEMultiQuadMesh::Build(GObject* po)
{
	m_po = po;
	if (po == nullptr) return false;

	m_MBNode.clear();
	m_MBEdge.clear();
	m_MBFace.clear();
	
	// build nodes
	int NN = po->Nodes();
	for (int i = 0; i < NN; ++i)
	{
		GNode* pn = po->Node(i);
		AddNode(pn->LocalPosition(), pn->Type()).SetID(pn->GetLocalID());
	}

	// build edges
	int NE = po->Edges();
	for (int i=0; i<NE; ++i)
	{
		GEdge* pe = po->Edge(i);
		MBEdge edge;
		edge.edge = *pe;
		edge.m_gid = pe->GetLocalID();
		m_MBEdge.push_back(edge);
	}

	// build the faces
	int NF = po->Faces();
	for (int i = 0; i < NF; ++i)
	{
		GFace* pf = po->Face(i);
		assert(pf->Nodes() == 4);
		int* n = &pf->m_node[0];
		MBFace& F = AddFace(n[0], n[1], n[2], n[3]);
		F.m_edge[0] = pf->m_edge[0].nid;
		F.m_edge[1] = pf->m_edge[1].nid;
		F.m_edge[2] = pf->m_edge[2].nid;
		F.m_edge[3] = pf->m_edge[3].nid;

		F.m_edgeWinding[0] = pf->m_edge[0].nwn;
		F.m_edgeWinding[1] = pf->m_edge[1].nwn;
		F.m_edgeWinding[2] = pf->m_edge[2].nwn;
		F.m_edgeWinding[3] = pf->m_edge[3].nwn;

		F.m_gid = pf->GetLocalID();
	}

	FindFaceNeighbours();

	return true;
}

//-----------------------------------------------------------------------------
vec3d FEMultiQuadMesh::EdgePosition(MBEdge& e, const MQPoint& q)
{
	double r = q.m_r;

	vec3d r1 = m_MBNode[e.Node(0)].m_r;
	vec3d r2 = m_MBNode[e.Node(1)].m_r;

	vec3d p(0, 0, 0);
	switch (e.edge.Type())
	{
	case EDGE_LINE:
	{
		p = r1 * (1.0 - r) + r2 * r;
	}
	break;
	case EDGE_ZARC:
	{
		vec2d c(0, 0);
		vec2d a(r1.x, r1.y);
		vec2d b(r2.x, r2.y);

		// create an arc object
		GM_CIRCLE_ARC ca(c, a, b, e.m_winding);

		vec2d q = ca.Point(r);
		p = vec3d(q.x(), q.y(), r1.z);
	}
	break;
	case EDGE_3P_CIRC_ARC:
	{
		vec3d r0 = m_MBNode[e.edge.m_cnode].m_r;
		vec3d r1 = m_MBNode[e.edge.m_node[0]].m_r;
		vec3d r2 = m_MBNode[e.edge.m_node[1]].m_r;
		GM_CIRCLE_3P_ARC c(r0, r1, r2, e.m_winding);
		p = c.Point(r);
	}
	break;
	default:
		assert(false);
	}

	return p;
}

//-----------------------------------------------------------------------------
vec3d FEMultiQuadMesh::FacePosition(MBFace& f, const FEMultiQuadMesh::MQPoint& q)
{
	double r = q.m_r;
	double s = q.m_s;

	double N1 = (1 - r) * (1 - s);
	double N2 = r * (1 - s);
	double N3 = r * s;
	double N4 = (1 - r) * s;

	vec3d r1 = m_MBNode[f.m_node[0]].m_r;
	vec3d r2 = m_MBNode[f.m_node[1]].m_r;
	vec3d r3 = m_MBNode[f.m_node[2]].m_r;
	vec3d r4 = m_MBNode[f.m_node[3]].m_r;

	// get edge points
	int i = q.m_i;
	int j = q.m_j;

	int nx = (m_bquadMesh ? (2 * f.m_nx + 1) : (f.m_nx + 1));
	int ny = (m_bquadMesh ? (2 * f.m_ny + 1) : (f.m_ny + 1));

	vec3d e1 = m_pm->Node(GetFaceEdgeNodeIndex(f, 0, i)).r;
	vec3d e2 = m_pm->Node(GetFaceEdgeNodeIndex(f, 1, j)).r;
	vec3d e3 = m_pm->Node(GetFaceEdgeNodeIndex(f, 2, nx - i - 1)).r;
	vec3d e4 = m_pm->Node(GetFaceEdgeNodeIndex(f, 3, ny - j - 1)).r;

	vec3d p = e1 * (1 - s) + e2 * r + e3 * s + e4 * (1 - r) \
		- (r1 * N1 + r2 * N2 + r3 * N3 + r4 * N4);

	return p;
}

//-----------------------------------------------------------------------------
// build the FE nodes
//
void FEMultiQuadMesh::BuildFENodes(FEMesh *pm)
{
	int NF = (int) m_MBFace.size();
	int NE = (int) m_MBEdge.size();
	int NN = (int) m_MBNode.size();

	// now, let's count the nodes
	int nodes = 0;
	for (int i = 0; i < NN; ++i)
	{
		MBNode& N = m_MBNode[i];
		if (N.m_type != NODE_SHAPE)
			nodes += 1;
	}
	for (int i = 0; i < NE; ++i)
	{
		MBEdge& E = m_MBEdge[i];
		nodes += E.m_nx - 1;
	}
	for (int i = 0; i < NF; ++i)
	{
		MBFace& F = m_MBFace[i];
		nodes += (F.m_nx - 1) * (F.m_ny - 1);
	}

	// add nodes for 2nd order mesh
	if (m_elemType == FE_QUAD8)
	{
		for (int i = 0; i < NE; ++i)
		{
			MBEdge& E = m_MBEdge[i];
			nodes += E.m_nx;
		}
		for (int i = 0; i < NF; ++i)
		{
			MBFace& F = m_MBFace[i];
			nodes += F.m_nx*(F.m_ny - 1) + (F.m_nx - 1)*F.m_ny;
		}
	}

	if (m_elemType == FE_QUAD9)
	{
		for (int i = 0; i < NE; ++i)
		{
			MBEdge& E = m_MBEdge[i];
			nodes += E.m_nx;
		}
		for (int i = 0; i < NF; ++i)
		{
			MBFace& F = m_MBFace[i];
			nodes += (F.m_nx + F.m_nx - 1)*(F.m_ny + F.m_ny - 1) - (F.m_nx - 1) * (F.m_ny - 1);
		}
	}

	// create storage
	pm->Create(nodes, 0);
	m_nodes = 0;
	m_currentNode = pm->NodePtr();

	// create the vertices
	for (int i = 0; i < NN; ++i)
	{
		MBNode& node = m_MBNode[i];
		node.m_ntag = -1;
		if (node.m_type != NODE_SHAPE)
		{
			node.m_ntag = AddFENode(node);
		}
		else node.m_ntag = -1;
	}
}

//-----------------------------------------------------------------------------
int FEMultiQuadMesh::AddFENode(const vec3d& r, int gid)
{
	m_currentNode->r = r;
	m_currentNode->m_gid = gid;
	m_currentNode++;
	m_nodes++;
	return m_nodes - 1;
}

//-----------------------------------------------------------------------------
int FEMultiQuadMesh::AddFENode(MBNode& N)
{
	assert(N.m_type != NODE_SHAPE);
	if (N.m_ntag == -1)
	{
		return AddFENode(N.m_r, N.m_gid);
	}
	return N.m_ntag;
}

//-----------------------------------------------------------------------------
int FEMultiQuadMesh::AddFEEdgeNode(MBEdge& E, const MQPoint& q)
{
	int i = q.m_i;
	int n = E.m_fenodes[i];
	if      (i == 0                     ) n = m_MBNode[E.Node(0)].m_ntag;
	else if (i == E.m_fenodes.size() - 1) n = m_MBNode[E.Node(1)].m_ntag;
	else if (E.m_fenodes[i] == -1)
	{
		vec3d p = EdgePosition(E, q);
		n = AddFENode(p);
	}
	if (E.m_fenodes[i] == -1) E.m_fenodes[i] = n;
	assert(E.m_fenodes[i] == n);
	return n;
}

//-----------------------------------------------------------------------------
int FEMultiQuadMesh::AddFEFaceNode(MBFace& F, const MQPoint& q)
{
	int i = q.m_i;
	int j = q.m_j;

	int nx = (m_bquadMesh ? (2 * F.m_nx + 1) : F.m_nx + 1);
	int ny = (m_bquadMesh ? (2 * F.m_ny + 1) : F.m_ny + 1);

	if      (j == 0     ) return GetFaceEdgeNodeIndex(F, 0, i);
	else if (i == nx - 1) return GetFaceEdgeNodeIndex(F, 1, j);
	else if (j == ny - 1) return GetFaceEdgeNodeIndex(F, 2, nx - i - 1);
	else if (i == 0     ) return GetFaceEdgeNodeIndex(F, 3, ny - j - 1);
	else
	{
		int n = j * nx + i;
		if (F.m_fenodes[n] == -1)
		{
			vec3d p = FacePosition(F, q);
			F.m_fenodes[n] = AddFENode(p);
		}
		return F.m_fenodes[n];
	}

	assert(false);
	return -1;
}

//-----------------------------------------------------------------------------
// Build the FE edges
//
void FEMultiQuadMesh::BuildFEEdges(FEMesh* pm)
{
	// count edges
	int edges = 0;
	for (int i = 0; i < (int)m_MBEdge.size(); ++i)
	{
		MBEdge& e = m_MBEdge[i];
		if (e.m_gid >= 0)
			edges += e.m_nx;
	}

	// allocate edges
	pm->Create(0, 0, 0, edges);

	// build the edges
	FEEdge* pe = pm->EdgePtr();
	edges = 0;
	for (int k = 0; k < (int)m_MBEdge.size(); ++k)
	{
		MBEdge& e = m_MBEdge[k];
		e.m_ntag = edges;

		int ne = (m_bquadMesh ?  2*e.m_nx + 1 : e.m_nx + 1);
		e.m_fenodes.assign(ne, -1);

		// discretize edge
		Sampler1D dx(e.m_nx, e.m_gx, e.m_bx);
		int n = 0;
		int nn = (m_bquadMesh ? 2 : 1);
		int en[3];
		for (int i = 0; i < e.m_nx; ++i)
		{
			double r = dx.value();
			double dr = dx.increment();

			en[0] = AddFEEdgeNode(e, MQPoint(n, r));
			if (m_bquadMesh)
			{
				// Note that we insert the middle node before the right edge node!
				en[2] = AddFEEdgeNode(e, MQPoint(n + 1, r + 0.5 * dr));
			}
			else en[2] = -1;
			en[1] = AddFEEdgeNode(e, MQPoint(n + nn, r + dr));

			if (e.m_gid >= 0)
			{
				pe->m_gid = e.m_gid;
				pe->SetType(m_bquadMesh ? FE_EDGE3 : FE_EDGE2);
				pe->n[0] = en[0];
				pe->n[1] = en[1];
				pe->n[2] = en[2];
				pe++;
				edges++;
			}

			dx.advance();
			n += nn;
		}
	}
}

//-----------------------------------------------------------------------------
// build the FE elements
//
void FEMultiQuadMesh::BuildFEFaces(FEMesh* pm)
{
	// figure out how many faces we have
	int faces = 0;
	int NF = (int)m_MBFace.size();
	for (int i = 0; i < NF; ++i)
	{
		MBFace& F = m_MBFace[i];
		faces += F.m_nx*F.m_ny;
	}

	// allocate faces and elements
	pm->Create(0, faces, faces);

	// A.3. add all face nodes
	int fn[9] = { -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	faces = 0;
	for (int n = 0; n < NF; ++n)
	{
		MBFace& F = m_MBFace[n];
		F.m_ntag = faces;

		int nf = 0;
		if (m_bquadMesh) nf = (2*F.m_nx + 1) * (2*F.m_ny + 1);
		else nf = (F.m_nx + 1) * (F.m_ny + 1);
		F.m_fenodes.assign(nf, -1);

		int nx = F.m_nx;
		int ny = F.m_ny;

		Sampler1D dx(nx, F.m_gx, F.m_bx);
		Sampler1D dy(ny, F.m_gy, F.m_by);

		int nn = (m_bquadMesh ? 2 : 1);
		int nj = 0;
		for (int j = 0; j < ny; ++j)
		{
			dx.reset();
			double r = dx.value();
			double dr = dx.increment();
			double s = dy.value();
			double ds = dy.increment();

			int ni = 0;
			for (int i = 0; i < nx; ++i, faces++)
			{
				FEFace* pf = pm->FacePtr(faces);
				pf->m_gid = F.m_gid;
				switch (m_elemType)
				{
				case FE_QUAD4: pf->SetType(FE_FACE_QUAD4); break;
				case FE_QUAD8: pf->SetType(FE_FACE_QUAD8); break;
				case FE_QUAD9: pf->SetType(FE_FACE_QUAD9); break;
				};

				r = dx.value();
				dr = dx.increment();

				fn[0] = AddFEFaceNode(F, MQPoint(ni     , nj     , r     , s     ));
				fn[1] = AddFEFaceNode(F, MQPoint(ni + nn, nj     , r + dr, s     ));
				fn[2] = AddFEFaceNode(F, MQPoint(ni + nn, nj + nn, r + dr, s + ds));
				fn[3] = AddFEFaceNode(F, MQPoint(ni     , nj + nn, r     , s + ds));

				if (m_bquadMesh)
				{
					fn[4] = AddFEFaceNode(F, MQPoint(ni + 1, nj    , r + 0.5*dr, s         ));
					fn[5] = AddFEFaceNode(F, MQPoint(ni + 2, nj + 1, r +     dr, s + 0.5*ds));
					fn[6] = AddFEFaceNode(F, MQPoint(ni + 1, nj + 2, r + 0.5*dr, s +     ds));
					fn[7] = AddFEFaceNode(F, MQPoint(ni    , nj + 1, r         , s + 0.5*ds));

					if (m_elemType == FE_QUAD9)
						fn[8] = AddFEFaceNode(F, MQPoint(ni + 1, nj + 1, r + 0.5*dr, s + 0.5*ds));
				}

				for (int k = 0; k < 9; k++) pf->n[k] = fn[k];

				dx.advance();
				ni += nn;
			}

			dy.advance();
			nj += nn;
		}
	}

	// also build the elements
	for (int i = 0; i < faces; ++i)
	{
		FEFace& f = pm->Face(i);
		FEElement& el = pm->Element(i);
		el.m_gid = 0;
		el.SetType(m_elemType);

		int ne = el.Nodes();
		for (int k=0; k<ne; ++k) el.m_node[k] = f.n[k];
	}
}

//-----------------------------------------------------------------------------
void FEMultiQuadMesh::BuildMBEdges()
{
	int NF = (int) m_MBFace.size();
	m_MBEdge.clear();
	m_MBEdge.reserve(4 * NF);
	int NE = 0;
	for (int i = 0; i < NF; ++i)
	{
		MBFace& f = m_MBFace[i];
		for (int j = 0; j < 4; ++j)
		{
			f.m_edgeWinding[j] = 0;

			// get the two nodes defining an edge
			int n1 = f.m_node[j];
			int n2 = f.m_node[(j + 1) % 4];

			// see if this edge already exists
			int n = FindEdge(n1, n2);

			// if it does
			if (n >= 0)
			{
				MBEdge& e = m_MBEdge[n];
				f.m_edge[j] = n;
				if (e.m_face[0] == -1) e.m_face[0] = i;
				else e.m_face[1] = i;

				if ((e.Node(0) == n1) && (e.Node(1) == n2)) f.m_edgeWinding[j] =  1;
				if ((e.Node(0) == n2) && (e.Node(1) == n1)) f.m_edgeWinding[j] = -1;
				assert(f.m_edgeWinding[j] != 0);
			}
			else
			{
				MBEdge e(n1, n2);
				e.m_face[0] = e.m_face[1] = -1;

				e.m_face[0] = i;

				switch (j)
				{
				case 0:
					e.m_nx = f.m_nx;
					e.m_gx = f.m_gx;
					e.m_bx = f.m_bx;
					break;
				case 1:
					e.m_nx = f.m_ny;
					e.m_gx = f.m_gy;
					e.m_bx = f.m_by;
					break;
				case 2:
					e.m_nx = f.m_nx;
					e.m_gx = (f.m_bx ? f.m_gx : 1 / f.m_gx);
					e.m_bx = f.m_bx;
					break;
				case 3:
					e.m_nx = f.m_ny;
					e.m_gx = (f.m_by ? f.m_gy : 1 / f.m_gy);
					e.m_bx = f.m_by;
					break;
				}
				m_MBEdge.push_back(e);
				f.m_edge[j] = NE++;
				f.m_edgeWinding[j] = 1;
			}
		}
	}

	NE = (int)m_MBEdge.size();
}

//-----------------------------------------------------------------------------
void FEMultiQuadMesh::FindFaceNeighbours()
{
	// build the node-face table
	vector< vector<int> > NFT;
	BuildNodeFaceTable(NFT);

	// reset all face neighbours
	int NF = (int)m_MBFace.size();
	for (int i = 0; i < NF; ++i)
	{
		MBFace& F = m_MBFace[i];
		for (int j = 0; j < 4; ++j) F.m_nbr[j] = -1;
	}

	// find the face's neighbours
	for (int i = 0; i < NF; ++i)
	{
		MBFace& F = m_MBFace[i];

		for (int j = 0; j < 4; ++j)
		{
			if (F.m_nbr[j] == -1)
			{
				// pick a node
				int n1 = F.m_node[j];
				int n2 = F.m_node[(j + 1) % 4];
				for (int k = 0; k < (int)NFT[n1].size(); ++k)
				{
					int nf = NFT[n1][k];
					if (nf != i)
					{
						MBFace& F2 = m_MBFace[nf];

						int l = FindEdgeIndex(F2, n1, n2);
						if (l != -1)
						{
							F.m_nbr[j] = nf;
							F2.m_nbr[l] = i;
							break;
						}
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
int FEMultiQuadMesh::FindEdgeIndex(MBFace& F, int n1, int n2)
{
	int m1, m2;
	for (int j = 0; j < 4; ++j)
	{
		m1 = F.m_node[j];
		m2 = F.m_node[(j + 1) % 4];
		if (((n1 == m1) && (n2 == m2)) || ((n1 == m2) && (n2 == m1))) return j;
	}
	return -1;
}

//-----------------------------------------------------------------------------
void FEMultiQuadMesh::BuildNodeFaceTable(vector< vector<int> >& NFT)
{
	int NN = (int) m_MBNode.size();
	int NF = (int) m_MBFace.size();

	// calculate node valences
	for (int i = 0; i < NN; ++i) m_MBNode[i].m_ntag = 0;

	for (int i = 0; i < NF; ++i)
	{
		MBFace& F = m_MBFace[i];
		for (int j = 0; j < 4; ++j) m_MBNode[F.m_node[j]].m_ntag++;
	}

	// create the node-block array
	NFT.resize(NN);
	for (int i = 0; i < NN; ++i)
	{
		if (m_MBNode[i].m_ntag > 0)
		{
			NFT[i].resize(m_MBNode[i].m_ntag);
			m_MBNode[i].m_ntag = 0;
		}
	}

	// fill the node-block array
	for (int i = 0; i < NF; ++i)
	{
		MBFace& F = m_MBFace[i];
		for (int j = 0; j < 4; ++j)
		{
			int n = F.m_node[j];
			MBNode& node = m_MBNode[n];
			NFT[n][node.m_ntag++] = i;
		}
	}
}

//-----------------------------------------------------------------------------
int FEMultiQuadMesh::FindEdge(int n1, int n2)
{
	MBEdge e(n1, n2);

	int NE = (int)m_MBEdge.size();

	for (int i = 0; i < NE; ++i)
	{
		MBEdge& e2 = m_MBEdge[i];
		if (e2 == e) return i;
	}

	return -1;
}

//-----------------------------------------------------------------------------
int FEMultiQuadMesh::GetFaceEdgeNodeIndex(MBFace& f, int ne, int i)
{
	// get the edge
	MBEdge& e = m_MBEdge[f.m_edge[ne]];

	// next, we need to see if we need to flip the edge or not
	if (f.m_edgeWinding[ne] == 1)
	{
		// don't flip the edge
		return GetEdgeNodeIndex(e, i);
	}
	else if (f.m_edgeWinding[ne] == -1)
	{
		// do flip the edge
		return GetEdgeNodeIndex(e, e.m_fenodes.size() - i - 1);
	}
	assert(false);
	return -1;
}

//-----------------------------------------------------------------------------
void FEMultiQuadMesh::SetFaceEdgeIDs(int nface, int n0, int n1, int n2, int n3)
{
	MBFace& face = m_MBFace[nface];
	m_MBEdge[face.m_edge[0]].m_gid = n0;
	m_MBEdge[face.m_edge[1]].m_gid = n1;
	m_MBEdge[face.m_edge[2]].m_gid = n2;
	m_MBEdge[face.m_edge[3]].m_gid = n3;
}

//-----------------------------------------------------------------------------
int FEMultiQuadMesh::GetEdgeNodeIndex(MBEdge& e, int i)
{
	return e.m_fenodes[i];
}

//-----------------------------------------------------------------------------
MBEdge& FEMultiQuadMesh::GetFaceEdge(int nface, int nedge)
{
	return m_MBEdge[m_MBFace[nface].m_edge[nedge]];
}

//-----------------------------------------------------------------------------
void FEMultiQuadMesh::SetFaceSizes(int nface, int nx, int ny)
{
	MBFace& F = m_MBFace[nface];
	F.SetSizes(nx, ny);
	m_MBEdge[F.m_edge[0]].m_nx = nx;
	m_MBEdge[F.m_edge[1]].m_nx = ny;
	m_MBEdge[F.m_edge[2]].m_nx = nx;
	m_MBEdge[F.m_edge[3]].m_nx = ny;
}

//-----------------------------------------------------------------------------
void FEMultiQuadMesh::UpdateMQ()
{
	FindFaceNeighbours();
	BuildMBEdges();
}

//-----------------------------------------------------------------------------
int FEMultiQuadMesh::GetFENode(MBNode& node)
{
	return node.m_ntag;
}
