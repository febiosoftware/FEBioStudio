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

#include "stdafx.h"
#include "FEMultiQuadMesh.h"
#include <MeshLib/FEMesh.h>
#include <MeshLib/FENodeNodeList.h>
#include <GeomLib/geom.h>

//-----------------------------------------------------------------------------
FEMultiQuadMesh::FEMultiQuadMesh()
{
	m_po = nullptr;
}

//-----------------------------------------------------------------------------

FEMultiQuadMesh::~FEMultiQuadMesh(void)
{
}

//-----------------------------------------------------------------------------
MBNode& FEMultiQuadMesh::AddNode(const vec3d& r)
{
	MBNode node;
	node.m_r = r;
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
	// create a new mesh
	FEMesh* pm = new FEMesh();

	// clear the node lists
	for (int i = 0; i < m_MBNode.size(); ++i) m_MBNode[i].m_fenodes.clear();
	for (int i = 0; i < m_MBEdge.size(); ++i) m_MBEdge[i].m_fenodes.clear();
	for (int i = 0; i < m_MBFace.size(); ++i) m_MBFace[i].m_fenodes.clear();

	// build the mesh
	BuildNodes(pm);
	BuildFaces(pm);
	BuildEdges(pm);

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
		if (pn->Type() != NODE_SHAPE)
		{
			AddNode(pn->LocalPosition()).SetID(pn->GetLocalID());
		}
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
		F.m_gid = pf->GetLocalID();
	}

	FindFaceNeighbours();

	return true;
}

//-----------------------------------------------------------------------------
// build the FE nodes
//
void FEMultiQuadMesh::BuildNodes(FEMesh *pm)
{
	int NF = m_MBFace.size();
	int NE = m_MBEdge.size();
	int NN = m_MBNode.size();

	// now, let's count the nodes
	int nodes = 0;
	for (int i = 0; i < NF; ++i)
	{
		MBFace& F = m_MBFace[i];
		nodes += (F.m_nx-1)*(F.m_ny-1);
	}
	for (int i = 0; i < NE; ++i)
	{
		MBEdge& E = m_MBEdge[i];
		nodes += E.m_nx - 1;
	}
	for (int i = 0; i < NN; ++i)
	{
		MBNode& N = m_MBNode[i];
		nodes += 1;
	}

	// create storage
	pm->Create(nodes, 0);

	// A. create the nodes
	// A.1. add all MB nodes
	nodes = 0;
	FENode* pn = pm->NodePtr();
	for (int i = 0; i < NN; ++i, ++pn)
	{
		pn->r = m_MBNode[i].m_r;
		m_MBNode[i].m_ntag = nodes;
		m_MBNode[i].m_fenodes.push_back(nodes++);
		pn->m_gid = m_MBNode[i].m_gid;
	}

	// A.2. add all edge nodes
	vec3d r1, r2, r3, r4, r5, r6, r7, r8;
	double r, s, t, gr, gs, gt, fr, fs, ft, dr, ds, dt;
	double N1, N2, N3, N4, N5, N6, N7, N8;
	for (int i = 0; i < NE; ++i)
	{
		MBEdge& e = m_MBEdge[i];
		r1 = m_MBNode[e.Node(0)].m_r;
		r2 = m_MBNode[e.Node(1)].m_r;
		e.m_ntag = nodes;

		fr = e.m_gx;
		gr = 1;
		if (e.m_bx)
		{
			gr = 2; if (e.m_nx % 2) gr += fr;
			for (int j = 0; j < e.m_nx / 2 - 1; ++j) gr = fr * gr + 2;
			gr = 1 / gr;
		}
		else
		{
			for (int j = 0; j < e.m_nx - 1; ++j) gr = fr * gr + 1;
			gr = 1 / gr;
		}

		dr = gr;
		r = 0;
		for (int j = 0; j < e.m_nx; ++j)
		{
			if (j > 0)
			{
				switch (e.edge.Type())
				{
				case EDGE_LINE:
				{
					pn->r = r1 * (1.0 - r) + r2 * r;
				}
				break;
				case EDGE_ZARC:
				{
					vec2d c(0, 0);
					vec2d a(r1.x, r1.y);
					vec2d b(r2.x, r2.y);

					// create an arc object
					GM_CIRCLE_ARC ca(c, a, b, e.m_winding);

					vec2d p = ca.Point(r);
					pn->r = vec3d(p.x, p.y, r1.z);
				}
				break;
				case EDGE_3P_CIRC_ARC:
				{
					assert(m_po);
					vec3d r0 = m_po->Node(e.edge.m_cnode)->LocalPosition();
					vec3d r1 = m_po->Node(e.edge.m_node[0])->LocalPosition() - r0;
					vec3d r2 = m_po->Node(e.edge.m_node[1])->LocalPosition() - r0;
					vec3d n = r1 ^ r2; n.Normalize();
					quatd q(n, vec3d(0, 0, 1)), qi = q.Inverse();
					q.RotateVector(r1);
					q.RotateVector(r2);
					GM_CIRCLE_ARC c(vec2d(0, 0), vec2d(r1.x, r1.y), vec2d(r2.x, r2.y));
					vec2d a = c.Point(r);
					vec3d p(a.x, a.y, 0);
					qi.RotateVector(p);
					pn->r = p + r0;
				}
				break;
				default:
					assert(false);
				}

				e.m_fenodes.push_back(nodes);
				++pn;
				++nodes;
			}

			r += dr;
			dr *= fr;
			if (e.m_bx && (j == e.m_nx / 2 - 1))
			{
				if (e.m_nx % 2 == 0) dr /= fr;
				fr = 1.0 / fr;
			}
		}
	}

	// A.3. add all face nodes
	for (int i = 0; i < NF; ++i)
	{
		MBFace& f = m_MBFace[i];
		r1 = m_MBNode[f.m_node[0]].m_r;
		r2 = m_MBNode[f.m_node[1]].m_r;
		r3 = m_MBNode[f.m_node[2]].m_r;
		r4 = m_MBNode[f.m_node[3]].m_r;
		f.m_ntag = nodes;

		int nx = f.m_nx;
		int ny = f.m_ny;

		fr = f.m_gx;
		gr = 1;
		if (f.m_bx)
		{
			gr = 2; if (f.m_nx % 2) gr += fr;
			for (int j = 0; j < f.m_nx / 2 - 1; ++j) gr = fr * gr + 2;
			gr = 1 / gr;
		}
		else
		{
			for (int j = 0; j < f.m_nx - 1; ++j) gr = fr * gr + 1;
			gr = 1 / gr;
		}

		fs = f.m_gy;
		gs = 1;
		if (f.m_by)
		{
			gs = 2; if (f.m_ny % 2) gs += fs;
			for (int j = 0; j < f.m_ny / 2 - 1; ++j) gs = fs * gs + 2;
			gs = 1 / gs;
		}
		else
		{
			for (int j = 0; j < f.m_ny - 1; ++j) gs = fs * gs + 1;
			gs = 1 / gs;
		}

		ds = gs;
		s = 0;
		for (int j = 0; j < ny; ++j)
		{
			if (j > 0)
			{
				dr = gr;
				r = 0;
				for (int k = 0; k < nx; ++k)
				{
					if (k > 0)
					{
						N1 = (1 - r)*(1 - s);
						N2 = r * (1 - s);
						N3 = r * s;
						N4 = (1 - r)*s;
					
						// get edge points
						vec3d e1 = pm->Node(GetFaceEdgeNodeIndex(f, 0, k)).r;
						vec3d e2 = pm->Node(GetFaceEdgeNodeIndex(f, 1, j)).r;
						vec3d e3 = pm->Node(GetFaceEdgeNodeIndex(f, 2, nx - k)).r;
						vec3d e4 = pm->Node(GetFaceEdgeNodeIndex(f, 3, ny - j)).r;

						vec3d p = e1 * (1 - s) + e2 * r + e3 * s + e4 * (1 - r) \
							- (r1*N1 + r2*N2 + r3*N3 + r4*N4);
						
						pn->r = p;

						f.m_fenodes.push_back(nodes);
						++pn;
						++nodes;
					}
					r += dr;
					dr *= fr;
					if (f.m_bx && (k == f.m_nx / 2 - 1))
					{
						if (f.m_nx % 2 == 0) dr /= fr;
						fr = 1.0 / fr;
					}
				}
				if (f.m_bx) fr = 1.0 / fr;
			}

			s += ds;
			ds *= fs;
			if (f.m_by && (j == f.m_ny / 2 - 1))
			{
				if (f.m_ny % 2 == 0) ds /= fs;
				fs = 1.0 / fs;
			}
		}
	}

	// make sure we have the right number of nodes
	assert(nodes == pm->Nodes());
}

//-----------------------------------------------------------------------------
// build the FE elements
//
void FEMultiQuadMesh::BuildFaces(FEMesh* pm)
{
	// figure out how many elements we have
	int faces = 0;
	int NF = m_MBFace.size();
	for (int i = 0; i < NF; ++i)
	{
		MBFace& F = m_MBFace[i];
		faces += F.m_nx*F.m_ny;
	}

	// allocate faces and elements
	pm->Create(0, faces, faces);

	// create the faces
	int fid = 0;
	for (int i = 0; i < NF; ++i)
	{
		MBFace& F = m_MBFace[i];
		for (int k = 0; k < F.m_ny; ++k)
		{
			for (int l = 0; l < F.m_nx; ++l)
			{
				FEFace* pf = pm->FacePtr(fid++);

				pf->n[0] = GetFaceNodeIndex(F, l, k);
				pf->n[1] = GetFaceNodeIndex(F, l + 1, k);
				pf->n[2] = GetFaceNodeIndex(F, l + 1, k + 1);
				pf->n[3] = GetFaceNodeIndex(F, l, k + 1);
				pf->SetType(FE_FACE_QUAD4);
				pf->m_gid = F.m_gid;
			}
		}
	}

	// also build the elements
	for (int i = 0; i < faces; ++i)
	{
		FEFace& f = pm->Face(i);
		FEElement& el = pm->Element(i);
		el.SetType(FE_QUAD4);
		el.m_node[0] = f.n[0];
		el.m_node[1] = f.n[1];
		el.m_node[2] = f.n[2];
		el.m_node[3] = f.n[3];
		el.m_gid = 0;
	}
}

//-----------------------------------------------------------------------------
// Build the FE edges
//
void FEMultiQuadMesh::BuildEdges(FEMesh* pm)
{
	// count edges
	int edges = 0;
	for (int i = 0; i < (int)m_MBEdge.size(); ++i)
	{
		MBEdge& e = m_MBEdge[i];
		if (e.m_gid >= 0) edges += e.m_nx;
	}

	// allocate faces
	pm->Create(0, 0, 0, edges);

	// build the edges
	FEEdge* pe = pm->EdgePtr();
	for (int k = 0; k < (int)m_MBEdge.size(); ++k)
	{
		MBEdge& e = m_MBEdge[k];
		if (e.m_gid >= 0)
		{
			for (int i = 0; i < e.m_nx; ++i, ++pe)
			{
				pe->m_gid = e.m_gid;
				pe->SetType(FE_EDGE2);
				pe->n[0] = GetEdgeNodeIndex(e, i);
				pe->n[1] = GetEdgeNodeIndex(e, i + 1);
			}
		}
	}
}

//-----------------------------------------------------------------------------

void FEMultiQuadMesh::BuildMBEdges()
{
	int NF = m_MBFace.size();
	m_MBEdge.clear();
	m_MBEdge.reserve(4 * NF);
	int NE = 0;
	for (int i = 0; i < NF; ++i)
	{
		MBFace& f = m_MBFace[i];
		for (int j = 0; j < 4; ++j)
		{
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
			}
		}
	}

	NE = m_MBEdge.size();

	// set the external flag of the edges
	for (int i = 0; i < NE; ++i) m_MBEdge[i].m_bext = false;
	for (int i = 0; i < NF; ++i)
	{
		MBFace& f = m_MBFace[i];
		for (int j = 0; j < 4; ++j)
		{
			if (f.m_nbr[j] == -1)
			{
				m_MBEdge[f.m_edge[j]].m_bext = true;
			}
		}
	}
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
	int NN = m_MBNode.size();
	int NF = m_MBFace.size();

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
		assert(m_MBNode[i].m_ntag);
		NFT[i].resize(m_MBNode[i].m_ntag);
		m_MBNode[i].m_ntag = 0;
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

	int NE = m_MBEdge.size();

	for (int i = 0; i < NE; ++i)
	{
		MBEdge& e2 = m_MBEdge[i];
		if (e2 == e) return i;
	}

	return -1;
}

//-----------------------------------------------------------------------------
int FEMultiQuadMesh::GetFaceNodeIndex(MBFace& f, int i, int j)
{
	if (i == 0)
	{
		return GetFaceEdgeNodeIndex(f, 3, f.m_ny - j);
	}
	else if (i == f.m_nx)
	{
		return GetFaceEdgeNodeIndex(f, 1, j);
	}
	else if (j == 0)
	{
		return GetFaceEdgeNodeIndex(f, 0, i);
	}
	else if (j == f.m_ny)
	{
		return GetFaceEdgeNodeIndex(f, 2, f.m_nx - i);
	}
	else
	{
		return f.m_ntag + (i - 1) + (j - 1)*(f.m_nx - 1);
	}
}

//-----------------------------------------------------------------------------


int FEMultiQuadMesh::GetFaceEdgeNodeIndex(MBFace& f, int ne, int i)
{
	// get the edge
	MBEdge& e = m_MBEdge[f.m_edge[ne]];

	// next, we need to see if we need to flip the edge or not
	if (e.Node(0) == f.m_node[ne])
	{
		// don't flip the edge
		return GetEdgeNodeIndex(e, i);
	}
	else if (e.Node(1) == f.m_node[ne])
	{
		// do flip the edge
		return GetEdgeNodeIndex(e, e.m_nx - i);
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
	if (i == 0)
	{
		return m_MBNode[e.Node(0)].m_ntag;
	}
	else if (i == e.m_nx)
	{
		return m_MBNode[e.Node(1)].m_ntag;
	}
	else
	{
		return e.m_ntag + (i - 1);
	}
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
void FEMultiQuadMesh::SetShapeModifier(MBFace& f, FEShapeModifier* mod)
{
	f.m_mod = mod;
}

void FEMultiQuadMesh::SetShapeModifier(MBEdge& e, FEShapeModifier* mod)
{
	e.m_mod = mod;
}

void FEMultiQuadMesh::SetShapeModifier(MBNode& n, FEShapeModifier* mod)
{
	n.m_mod = mod;
}

//-----------------------------------------------------------------------------
int FEMultiQuadMesh::GetFENode(MBNode& node)
{
	return node.m_ntag;
}

vector<int> FEMultiQuadMesh::GetFENodeList(MBEdge& edge)
{
	vector<int> nodeList;
	nodeList.push_back(m_MBNode[edge.Node(0)].m_fenodes[0]);
	nodeList.insert(nodeList.end(), edge.m_fenodes.begin(), edge.m_fenodes.end());
	nodeList.push_back(m_MBNode[edge.Node(1)].m_fenodes[0]);
	return nodeList;
}

vector<int> FEMultiQuadMesh::GetFENodeList(MBFace& face)
{
	vector<int> e1 = GetFENodeList(m_MBEdge[face.m_edge[0]]);
	vector<int> e2 = GetFENodeList(m_MBEdge[face.m_edge[1]]);
	vector<int> e3 = GetFENodeList(m_MBEdge[face.m_edge[2]]);
	vector<int> e4 = GetFENodeList(m_MBEdge[face.m_edge[3]]);

	vector<int> nodeList = face.m_fenodes;
	nodeList.insert(nodeList.end(), e1.begin(), e1.end());
	nodeList.insert(nodeList.end(), e2.begin(), e2.end());
	nodeList.insert(nodeList.end(), e3.begin(), e3.end());
	nodeList.insert(nodeList.end(), e4.begin(), e4.end());

	return nodeList;
}

//-----------------------------------------------------------------------------
void FEMultiQuadMesh::ApplyMeshModifiers(FEMesh* pm)
{
	pm->TagAllNodes(0);

	// apply node modifiers first
	for (int i = 0; i < m_MBNode.size(); ++i)
	{
		MBNode& node = m_MBNode[i];
		if (node.m_mod)
		{
			int n = GetFENode(node); assert(n >= 0);
			FENode& fen = pm->Node(n);
			assert(fen.m_ntag == 0);
			vec3d r0 = fen.pos();
			vec3d rn = node.m_mod->Apply(r0);
			fen.pos(rn);
			fen.m_ntag = 1;
		}
	}

	// apply edge modifiers
	for (int i = 0; i < m_MBEdge.size(); ++i)
	{
		MBEdge& edge = m_MBEdge[i];
		if (edge.m_mod)
		{
			vector<int> nodeList = GetFENodeList(edge);
			for (int j = 0; j < nodeList.size(); ++j)
			{
				FENode& fen = pm->Node(nodeList[j]);
				if (fen.m_ntag == 0)
				{
					vec3d r0 = fen.pos();
					vec3d rn = edge.m_mod->Apply(r0);
					fen.pos(rn);
					fen.m_ntag = 2;
				}
			}
		}
	}

	// apply face modifiers
	for (int i = 0; i < m_MBFace.size(); ++i)
	{
		MBFace& face = m_MBFace[i];
		if (face.m_mod)
		{
			vector<int> nodeList = GetFENodeList(face);
			for (int j = 0; j < nodeList.size(); ++j)
			{
				FENode& fen = pm->Node(nodeList[j]);
				if (fen.m_ntag == 0)
				{
					vec3d r0 = fen.pos();
					vec3d rn = face.m_mod->Apply(r0);
					fen.pos(rn);
					fen.m_ntag = 3;
				}
			}
		}
	}

	// fix all corner nodes
	for (int i = 0; i < m_MBNode.size(); ++i)
	{
		//		pm->Node(m_MBNode[i].m_fenodes[0]).m_ntag = 4;
	}

	// solve for the remaining nodal positions
	FENodeNodeList NNL(pm);
	double err = 0, tol = 1e-5;
	int niter = 0, maxIters = 100;
	do
	{
		err = 0.0;
		for (int i = 0; i < pm->Nodes(); ++i)
		{
			FENode& node = pm->Node(i);
			if (node.m_ntag == 0)
			{
				int nn = NNL.Valence(i);
				vec3d r(0, 0, 0);
				for (int j = 0; j < nn; ++j)
				{
					r += pm->Node(NNL.Node(i, j)).pos();
				}
				r /= (double)nn;

				vec3d dr = r - node.pos();
				err += dr.SqrLength();
				node.pos(r);
			}
		}
		err = sqrt(err);
		niter++;
	} while ((err > tol) && (niter < maxIters));
}
