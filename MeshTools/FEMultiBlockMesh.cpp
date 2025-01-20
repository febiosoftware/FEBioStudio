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
#include "FEMultiBlockMesh.h"
#include <MeshLib/FEMesh.h>
#include <MeshLib/FENodeNodeList.h>
#include <GeomLib/geom.h>
#include <GeomLib/GMultiBox.h>
#include <GeomLib/GMultiPatch.h>
#include <algorithm>
#include "FESelection.h"
#include <GeomLib/GGroup.h>
using namespace std;

void MBBlock::SetNodes(int n1,int n2,int n3,int n4,int n5,int n6,int n7,int n8)
{
	m_node[0] = n1;
	m_node[1] = n2;
	m_node[2] = n3;
	m_node[3] = n4;
	m_node[4] = n5;
	m_node[5] = n6;
	m_node[6] = n7;
	m_node[7] = n8;
}

MBBlock& MBBlock::SetZoning(double gx, double gy, double gz, bool bx, bool by, bool bz)
{
	m_gx = gx;
	m_gy = gy;
	m_gz = gz;
	m_bx = bx;
	m_by = by;
	m_bz = bz;
	return *this;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

FEMultiBlockMesh::FEMultiBlockMesh()
{
	m_pm = nullptr;
	m_elemType = FE_HEX8;
}

//-----------------------------------------------------------------------------
FEMultiBlockMesh::FEMultiBlockMesh(const FEMultiBlockMesh& mb)
{
	CopyFrom(mb);
}

//-----------------------------------------------------------------------------
void FEMultiBlockMesh::operator = (const FEMultiBlockMesh& mb)
{
	CopyFrom(mb);
}

//-----------------------------------------------------------------------------
void FEMultiBlockMesh::CopyFrom(const FEMultiBlockMesh& mb)
{
	ClearMB();
	m_MBNode = mb.m_MBNode;
	m_MBEdge = mb.m_MBEdge;
	m_MBFace = mb.m_MBFace;
	m_MBlock = mb.m_MBlock;

	m_elemType = mb.m_elemType;
	m_quadMesh = mb.m_quadMesh;
}

//-----------------------------------------------------------------------------

FEMultiBlockMesh::~FEMultiBlockMesh(void)
{
}

//-----------------------------------------------------------------------------
void FEMultiBlockMesh::SetElementType(int elemType)
{
	assert((m_elemType == FE_HEX8) || (m_elemType == FE_HEX20) || (m_elemType == FE_HEX27));
	m_elemType = elemType;
	m_quadMesh = (elemType != FE_HEX8);
}

void FEMultiBlockMesh::ClearMB()
{
	m_MBlock.clear();
	m_MBFace.clear();
	m_MBEdge.clear();
	m_MBNode.clear();
	m_pm = nullptr;
	m_currentNode = nullptr;
}

bool FEMultiBlockMesh::BuildMultiBlock()
{
	return false;
}

//-----------------------------------------------------------------------------
// build the FE mesh
//
FSMesh* FEMultiBlockMesh::BuildMesh()
{
	if ((m_elemType != FE_HEX8) && (m_elemType != FE_HEX20) && (m_elemType != FE_HEX27))
	{
		assert(false);
		return nullptr;
	}
	m_quadMesh = (m_elemType != FE_HEX8);

	// update MB data structures
	UpdateMB();

	// create a new mesh
	FSMesh* pm = new FSMesh();
	m_pm = pm;

	// clear the node lists
	for (int i = 0; i < m_MBNode.size(); ++i) m_MBNode[i].m_fenodes.clear();
	for (int i = 0; i < m_MBEdge.size(); ++i) m_MBEdge[i].m_fenodes.clear();
	for (int i = 0; i < m_MBFace.size(); ++i) m_MBFace[i].m_fenodes.clear();
	for (int i = 0; i < m_MBlock.size(); ++i) m_MBlock[i].m_fenodes.clear();

	// build the mesh
	BuildFENodes(pm);
	BuildFEEdges(pm);
	BuildFEFaces(pm);
	BuildFEElements(pm);

	assert(pm->Nodes() == m_nodes);

	// update the mesh
	pm->BuildMesh();

	return pm;
}

//-----------------------------------------------------------------------------
vec3d FEMultiBlockMesh::EdgePosition(MBEdge& e, const MQPoint& q)
{
	double r = q.m_r;

	vec3d r1 = m_MBNode[e.Node(0)].m_r;
	vec3d r2 = m_MBNode[e.Node(1)].m_r;

	vec3d p;
	switch (e.m_ntype)
	{
	case EDGE_LINE:
		p = r1 * (1 - r) + r2 * r;
		break;
	case EDGE_ZARC:
	{
		vec2d c(0, 0);
		vec2d a(r1.x, r1.y);
		vec2d b(r2.x, r2.y);

		// create an arc object
		GM_CIRCLE_ARC ca(c, a, b, e.m_orient);
		vec2d q = ca.Point(r);
		p = vec3d(q.x(), q.y(), r1.z);
	}
	break;
	case EDGE_3P_CIRC_ARC:
	{
		vec3d r0 = m_MBNode[e.m_cnode].m_r;
		vec3d r1 = m_MBNode[e.m_node[0]].m_r;
		vec3d r2 = m_MBNode[e.m_node[1]].m_r;
		GM_CIRCLE_3P_ARC c(r0, r1, r2, e.m_orient);
		p = c.Point(r);
	}
	break;
	default:
		assert(false);
	}

	return p;
}

//-----------------------------------------------------------------------------
vec3d FEMultiBlockMesh::FacePosition(MBFace& F, const MQPoint& q)
{
	vec3d r1 = m_MBNode[F.m_node[0]].m_r;
	vec3d r2 = m_MBNode[F.m_node[1]].m_r;
	vec3d r3 = m_MBNode[F.m_node[2]].m_r;
	vec3d r4 = m_MBNode[F.m_node[3]].m_r;

/*
	// linear interpolation
	N1 = (1-r)*(1-s);
	N2 = r*(1-s);
	N3 = r*s;
	N4 = (1-r)*s;
	pn->r = r1*N1 + r2*N2 + r3*N3 + r4*N4;
*/

	// transfinite interpolation
	double r = q.m_r;
	double s = q.m_s;

	double N1 = (1 - r) * (1 - s);
	double N2 = r * (1 - s);
	double N3 = r * s;
	double N4 = (1 - r) * s;

	int mx = F.m_mx;
	int my = F.m_my;

	int i = q.m_i;
	int j = q.m_j;

	// get edge points
	vec3d e[4];
	e[0] = m_pm->Node(GetFaceEdgeNodeIndex(F, 0, i)).r;
	e[1] = m_pm->Node(GetFaceEdgeNodeIndex(F, 1, j)).r;
	e[2] = m_pm->Node(GetFaceEdgeNodeIndex(F, 2, mx - i - 1)).r;
	e[3] = m_pm->Node(GetFaceEdgeNodeIndex(F, 3, my - j - 1)).r;

	vec3d p = e[0] * (1 - s) + e[1] * r + e[2] * s + e[3] * (1 - r) \
		- (r1 * N1 + r2 * N2 + r3 * N3 + r4 * N4);

	// if this point should be on a sphere, project it to the sphere
	if (F.m_isSphere)
	{
		vec3d t = p - F.m_sphereCenter; t.Normalize();
		p = F.m_sphereCenter + t * F.m_sphereRadius;
	}

	if (F.m_isRevolve)
	{
		vec2d c(e[F.m_nrevolveEdge].x, e[F.m_nrevolveEdge].y);
		double R = c.norm();

		double z = p.z;
		p.z = 0;
		p.Normalize();
		p.x *= R;
		p.y *= R;
		p.z = z;
	}

	return p;
}

//-----------------------------------------------------------------------------
vec3d FEMultiBlockMesh::BlockPosition(MBBlock& B, const MQPoint& q)
{
	double r = q.m_r;
	double s = q.m_s;
	double t = q.m_t;

	double N1 = (1 - r) * (1 - s) * (1 - t);
	double N2 = r * (1 - s) * (1 - t);
	double N3 = r * s * (1 - t);
	double N4 = (1 - r) * s * (1 - t);
	double N5 = (1 - r) * (1 - s) * t;
	double N6 = r * (1 - s) * t;
	double N7 = r * s * t;
	double N8 = (1 - r) * s * t;

	vec3d r1 = m_MBNode[B.m_node[0]].m_r;
	vec3d r2 = m_MBNode[B.m_node[1]].m_r;
	vec3d r3 = m_MBNode[B.m_node[2]].m_r;
	vec3d r4 = m_MBNode[B.m_node[3]].m_r;
	vec3d r5 = m_MBNode[B.m_node[4]].m_r;
	vec3d r6 = m_MBNode[B.m_node[5]].m_r;
	vec3d r7 = m_MBNode[B.m_node[6]].m_r;
	vec3d r8 = m_MBNode[B.m_node[7]].m_r;

	// tri-linear interpolation
//	pn->r = r1*N1 + r2*N2 + r3*N3 + r4*N4 + r5*N5 + r6*N6 + r7*N7 + r8*N8;

	int mx = B.m_mx;
	int my = B.m_my;
	int mz = B.m_mz;

	int i = q.m_i;
	int j = q.m_j;
	int k = q.m_k;

	// transfinite interpolation
	vec3d f1 = m_pm->Node(GetBlockFaceNodeIndex(B, 4, i, my - j - 1)).r;
	vec3d f2 = m_pm->Node(GetBlockFaceNodeIndex(B, 0, i, k)).r;
	vec3d f3 = m_pm->Node(GetBlockFaceNodeIndex(B, 3, my - j - 1, k)).r;
	vec3d f4 = m_pm->Node(GetBlockFaceNodeIndex(B, 5, i, j)).r;
	vec3d f5 = m_pm->Node(GetBlockFaceNodeIndex(B, 2, mx - i - 1, k)).r;
	vec3d f6 = m_pm->Node(GetBlockFaceNodeIndex(B, 1, j, k)).r;

	vec3d p = (f1 * (1 - t) + f2 * (1 - s) + f3 * (1 - r) + f4 * t + f5 * s + f6 * r \
		- (r1 * N1 + r2 * N2 + r3 * N3 + r4 * N4 + r5 * N5 + r6 * N6 + r7 * N7 + r8 * N8)) * 0.5;

	return p;
}

//-----------------------------------------------------------------------------
int FEMultiBlockMesh::AddFENode(const vec3d& r, int gid)
{
	m_currentNode->r = r;
	m_currentNode->m_gid = gid;
	m_currentNode++;
	m_nodes++;
	return m_nodes - 1;
}

//-----------------------------------------------------------------------------
// build the FE nodes
//
void FEMultiBlockMesh::BuildFENodes(FSMesh *pm)
{
	int NB = m_MBlock.size();
	int NF = m_MBFace.size();
	int NE = m_MBEdge.size();
	int NN = m_MBNode.size();

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
		nodes += (m_quadMesh ? 2*E.m_nx - 1 : E.m_nx -1 );
	}
	for (int i = 0; i < NF; ++i)
	{
		MBFace& F = m_MBFace[i];
		switch (m_elemType)
		{
		case FE_HEX8 : nodes += (F.m_nx - 1) * (F.m_ny - 1); break;
		case FE_HEX20: nodes += (F.m_nx - 1) * (F.m_ny - 1) + (F.m_nx - 1)*F.m_ny + (F.m_ny - 1)*F.m_nx; break;
		case FE_HEX27: nodes += (2*F.m_nx - 1) * (2*F.m_ny - 1); break;
		}
	}
	for (int i=0; i<NB; ++i)
	{
		MBBlock& B = m_MBlock[i];
		switch (m_elemType)
		{
		case FE_HEX8 : nodes += (B.m_nx - 1) * (B.m_ny - 1) * (B.m_nz - 1); break;
		case FE_HEX20:
			nodes += (B.m_nx - 1) * (B.m_ny - 1) * (B.m_nz - 1);
			nodes += (B.m_nz - 1) * (B.m_nx - 1) * B.m_ny;
			nodes += (B.m_nz - 1) * (B.m_ny - 1) * B.m_nx;
			nodes += B.m_nz * (B.m_nx - 1) * (B.m_ny - 1);
			break;
		case FE_HEX27: nodes += (2*B.m_nx - 1) * (2*B.m_ny - 1) * (2*B.m_nz - 1); break;
		}
	}

	// create storage
	pm->Create(nodes, 0);
	m_currentNode = pm->NodePtr();
	m_nodes = 0;

	// A. create the nodes
	// A.1. add all MB nodes
	for (int i=0; i<NN; ++i)
	{
		MBNode& node = m_MBNode[i];
		if (node.m_type != NODE_SHAPE)
		{
			node.m_ntag = AddFENode(node.m_r, node.m_gid);
			node.m_fenodes.push_back(node.m_ntag);
		}
		else node.m_ntag = -1;
	}
}

//-----------------------------------------------------------------------------
int FEMultiBlockMesh::AddFEEdgeNode(MBEdge& E, const MQPoint& q)
{
	int i = q.m_i;
	int n = E.m_fenodes[i];
	if (i == 0) n = m_MBNode[E.Node(0)].m_ntag;
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
// Build the FE edges
//
void FEMultiBlockMesh::BuildFEEdges(FSMesh* pm)
{
	// count edges
	int edges = 0;
	for (int i = 0; i < (int)m_MBEdge.size(); ++i)
	{
		MBEdge& e = m_MBEdge[i];
		if (e.m_gid >= 0)
			edges += e.m_nx;
	}

	// allocate faces
	pm->Create(0, 0, 0, edges);

	// build the edges
	FSEdge* pe = pm->EdgePtr();
	for (int k = 0; k < (int)m_MBEdge.size(); ++k)
	{
		MBEdge& e = m_MBEdge[k];
		e.m_ntag = edges;

		// allocate fenodes array
		e.m_mx = (m_quadMesh ? 2 * e.m_nx + 1 : e.m_nx + 1);
		e.m_fenodes.assign(e.m_mx, -1);

		// discretize edge
		Sampler1D dx(e.m_nx, e.m_gx, e.m_bx);
		int n = 0;
		int nn = (m_quadMesh ? 2 : 1);
		int en[3];
		for (int i = 0; i < e.m_nx; ++i)
		{
			double r = dx.value();
			double dr = dx.increment();

			en[0] = AddFEEdgeNode(e, MQPoint(n, r));
			if (m_quadMesh)
			{
				// Note that we insert the middle node before the right edge node!
				en[2] = AddFEEdgeNode(e, MQPoint(n + 1, r + 0.5 * dr));
			}
			else en[2] = -1;
			en[1] = AddFEEdgeNode(e, MQPoint(n + nn, r + dr));

			if (e.m_gid >= 0)
			{
				pe->m_gid = e.m_gid;
				pe->SetType(m_quadMesh ? FE_EDGE3 : FE_EDGE2);
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
int FEMultiBlockMesh::AddFEFaceNode(MBFace& F, const MQPoint& q)
{
	int i = q.m_i;
	int j = q.m_j;

	int mx = F.m_mx;
	int my = F.m_my;

	if      (j == 0     ) return GetFaceEdgeNodeIndex(F, 0, i);
	else if (i == mx - 1) return GetFaceEdgeNodeIndex(F, 1, j);
	else if (j == my - 1) return GetFaceEdgeNodeIndex(F, 2, mx - i - 1);
	else if (i == 0     ) return GetFaceEdgeNodeIndex(F, 3, my - j - 1);
	else
	{
		int n = j * mx + i;
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
// Build the FE faces
//
void FEMultiBlockMesh::BuildFEFaces(FSMesh* pm)
{
	// count faces
	int faces = 0;
	for (int i = 0; i < (int)m_MBFace.size(); ++i)
	{
		MBFace& f = m_MBFace[i];
		if (f.m_gid >= 0)
		{
			faces += f.m_nx * f.m_ny;
		}
	}

	// allocate faces
	pm->Create(0, 0, faces);

	// A.3. add all face nodes
	int fn[9] = { -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	faces = 0;
	for (int n = 0; n < (int)m_MBFace.size(); ++n)
	{
		MBFace& F = m_MBFace[n];
		F.m_ntag = (F.m_gid >= 0 ? faces : -1);

		F.m_mx = (m_quadMesh ? (2 * F.m_nx + 1) : F.m_nx + 1);
		F.m_my = (m_quadMesh ? (2 * F.m_ny + 1) : F.m_ny + 1);

		int nf = F.m_mx*F.m_my;
		F.m_fenodes.assign(nf, -1);

		int nx = F.m_nx;
		int ny = F.m_ny;

		Sampler1D dx(nx, F.m_gx, F.m_bx);
		Sampler1D dy(ny, F.m_gy, F.m_by);

		int nn = (m_quadMesh ? 2 : 1);
		int nj = 0;
		for (int j = 0; j < ny; ++j)
		{
			dx.reset();
			double r = dx.value();
			double dr = dx.increment();
			double s = dy.value();
			double ds = dy.increment();

			int ni = 0;
			for (int i = 0; i < nx; ++i)
			{
				r = dx.value();
				dr = dx.increment();

				fn[0] = AddFEFaceNode(F, MQPoint(ni, nj, r, s));
				fn[1] = AddFEFaceNode(F, MQPoint(ni + nn, nj, r + dr, s));
				fn[2] = AddFEFaceNode(F, MQPoint(ni + nn, nj + nn, r + dr, s + ds));
				fn[3] = AddFEFaceNode(F, MQPoint(ni, nj + nn, r, s + ds));

				if (m_quadMesh)
				{
					fn[4] = AddFEFaceNode(F, MQPoint(ni + 1, nj, r + 0.5 * dr, s));
					fn[5] = AddFEFaceNode(F, MQPoint(ni + 2, nj + 1, r + dr, s + 0.5 * ds));
					fn[6] = AddFEFaceNode(F, MQPoint(ni + 1, nj + 2, r + 0.5 * dr, s + ds));
					fn[7] = AddFEFaceNode(F, MQPoint(ni, nj + 1, r, s + 0.5 * ds));

					if (m_elemType == FE_HEX27)
						fn[8] = AddFEFaceNode(F, MQPoint(ni + 1, nj + 1, r + 0.5 * dr, s + 0.5 * ds));
				}

				if (F.m_gid >= 0)
				{
					FSFace* pf = pm->FacePtr(faces++);
					pf->m_gid = F.m_gid;
					pf->m_sid = (F.m_sid < 0 ? F.m_gid : F.m_sid);
					switch (m_elemType)
					{
					case FE_HEX8 : pf->SetType(FE_FACE_QUAD4); break;
					case FE_HEX20: pf->SetType(FE_FACE_QUAD8); break;
					case FE_HEX27: pf->SetType(FE_FACE_QUAD9); break;
					};
					for (int k = 0; k < 9; k++) pf->n[k] = fn[k];
				}

				dx.advance();
				ni += nn;
			}

			dy.advance();
			nj += nn;
		}
	}
}

//-----------------------------------------------------------------------------
int FEMultiBlockMesh::AddFEElemNode(MBBlock& B, const MQPoint& q)
{
	int mx = B.m_mx, my = B.m_my, mz = B.m_mz;
	int i = q.m_i, j = q.m_j, k = q.m_k;
	if (i == 0)
	{
		return GetBlockFaceNodeIndex(B, 3, my - j - 1, k);
	}
	else if (i == mx-1)
	{
		return GetBlockFaceNodeIndex(B, 1, j, k);
	}
	else if (j == 0)
	{
		return GetBlockFaceNodeIndex(B, 0, i, k);
	}
	else if (j == my-1)
	{
		return GetBlockFaceNodeIndex(B, 2, mx - i - 1, k);
	}
	else if (k == 0)
	{
		return GetBlockFaceNodeIndex(B, 4, i, my - j - 1);
	}
	else if (k == mz-1)
	{
		return GetBlockFaceNodeIndex(B, 5, i, j);
	}
	else
	{
		int n = k*mx*my + j * mx + i;
		if (B.m_fenodes[n] == -1)
		{
			vec3d p = BlockPosition(B, q);
			B.m_fenodes[n] = AddFENode(p);
		}
		return B.m_fenodes[n];
	}
}

//-----------------------------------------------------------------------------
// build the FE elements
//
void FEMultiBlockMesh::BuildFEElements(FSMesh* pm)
{
	int NB = m_MBlock.size();

	// figure out how many elements we have
	int elems = 0;
	for (int i=0; i<NB; ++i)
	{
		MBBlock& B = m_MBlock[i];
		elems += B.m_nx*B.m_ny*B.m_nz;
	}

	// allocate elements
	pm->Create(0, elems);

	// create the elements
	int eid = 0;
	for (int l=0; l<NB; ++l)
	{
		MBBlock& b = m_MBlock[l];
		b.m_ntag = m_nodes;

		int nx = b.m_nx;
		int ny = b.m_ny;
		int nz = b.m_nz;

		b.m_mx = (m_quadMesh ? (2 * b.m_nx + 1) : b.m_nx + 1);
		b.m_my = (m_quadMesh ? (2 * b.m_ny + 1) : b.m_ny + 1);
		b.m_mz = (m_quadMesh ? (2 * b.m_nz + 1) : b.m_nz + 1);
		int nb = b.m_mx * b.m_my * b.m_mz;
		b.m_fenodes.assign(nb, -1);

		Sampler1D dx(nx, b.m_gx, b.m_bx);
		Sampler1D dy(ny, b.m_gy, b.m_by);
		Sampler1D dz(ny, b.m_gz, b.m_bz);

		int nn = (m_quadMesh ? 2 : 1);
		int nk = 0;

		for (int k=0; k<nz; ++k)
		{
			dx.reset();
			dy.reset();
			double r = dx.value(); double dr = dx.increment();
			double s = dy.value(); double ds = dy.increment();
			double t = dz.value(); double dt = dz.increment();

			int nj = 0;
			for (int j=0; j<ny; ++j)
			{
				dx.reset();
				double r = dx.value(); double dr = dx.increment();
				double s = dy.value(); double ds = dy.increment();

				int ni = 0;
				for (int i=0; i<nx; ++i)
				{
					FEElement_* pe = pm->ElementPtr(eid++);
					pe->m_gid = b.m_gid;
					pe->SetType(m_elemType);

					double r = dx.value();
					double dr = dx.increment();

					pe->m_node[0] = AddFEElemNode(b, MQPoint(ni     , nj     , nk     , r     , s     , t     ));
					pe->m_node[1] = AddFEElemNode(b, MQPoint(ni + nn, nj     , nk     , r + dr, s     , t     ));
					pe->m_node[2] = AddFEElemNode(b, MQPoint(ni + nn, nj + nn, nk     , r + dr, s + ds, t     ));
					pe->m_node[3] = AddFEElemNode(b, MQPoint(ni     , nj + nn, nk     , r     , s + ds, t    ));
					pe->m_node[4] = AddFEElemNode(b, MQPoint(ni     , nj     , nk + nn, r     , s     , t + dt));
					pe->m_node[5] = AddFEElemNode(b, MQPoint(ni + nn, nj     , nk + nn, r + dr, s     , t + dt));
					pe->m_node[6] = AddFEElemNode(b, MQPoint(ni + nn, nj + nn, nk + nn, r + dr, s + ds, t + dt));
					pe->m_node[7] = AddFEElemNode(b, MQPoint(ni     , nj + nn, nk + nn, r     , s + ds, t + dt));

					if (m_elemType != FE_HEX8)
					{
						pe->m_node[ 8] = AddFEElemNode(b, MQPoint(ni + 1, nj    , nk    , r + 0.5*dr, s         , t         ));
						pe->m_node[ 9] = AddFEElemNode(b, MQPoint(ni + 2, nj + 1, nk    , r +     dr, s + 0.5*ds, t         ));
						pe->m_node[10] = AddFEElemNode(b, MQPoint(ni + 1, nj + 2, nk    , r + 0.5*dr, s +     ds, t         ));
						pe->m_node[11] = AddFEElemNode(b, MQPoint(ni    , nj + 1, nk    , r         , s + 0.5*ds, t         ));
						pe->m_node[12] = AddFEElemNode(b, MQPoint(ni + 1, nj    , nk + 2, r + 0.5*dr, s         , t + dt    ));
						pe->m_node[13] = AddFEElemNode(b, MQPoint(ni + 2, nj + 1, nk + 2, r +     dr, s + 0.5*ds, t + dt    ));
						pe->m_node[14] = AddFEElemNode(b, MQPoint(ni + 1, nj + 2, nk + 2, r + 0.5*dr, s +     ds, t + dt    ));
						pe->m_node[15] = AddFEElemNode(b, MQPoint(ni    , nj + 1, nk + 2, r         , s + 0.5*ds, t + dt    ));
						pe->m_node[16] = AddFEElemNode(b, MQPoint(ni    , nj    , nk + 1, r         , s         , t + 0.5*dt));
						pe->m_node[17] = AddFEElemNode(b, MQPoint(ni + 2, nj    , nk + 1, r  +    dr, s         , t + 0.5*dt));
						pe->m_node[18] = AddFEElemNode(b, MQPoint(ni + 2, nj + 2, nk + 1, r  +    dr, s  +    ds, t + 0.5*dt));
						pe->m_node[19] = AddFEElemNode(b, MQPoint(ni    , nj + 2, nk + 1, r         , s  +    ds, t + 0.5*dt));

						if (m_elemType == FE_HEX27)
						{
							pe->m_node[20] = AddFEElemNode(b, MQPoint(ni + 1, nj    , nk + 1, r + 0.5*dr, s         , t + 0.5*dt));
							pe->m_node[21] = AddFEElemNode(b, MQPoint(ni + 2, nj + 1, nk + 1, r +     dr, s + 0.5*ds, t + 0.5*dt));
							pe->m_node[22] = AddFEElemNode(b, MQPoint(ni + 1, nj + 2, nk + 1, r + 0.5*dr, s +     ds, t + 0.5*dt));
							pe->m_node[23] = AddFEElemNode(b, MQPoint(ni    , nj + 1, nk + 1, r         , s + 0.5*ds, t + 0.5*dt));
							pe->m_node[24] = AddFEElemNode(b, MQPoint(ni + 1, nj + 1, nk    , r + 0.5*dr, s + 0.5*ds, t         ));
							pe->m_node[25] = AddFEElemNode(b, MQPoint(ni + 1, nj + 1, nk + 2, r + 0.5*dr, s + 0.5*ds, t +     dt));
							pe->m_node[26] = AddFEElemNode(b, MQPoint(ni + 1, nj + 1, nk + 1, r + 0.5*dr, s + 0.5*ds, t + 0.5*dt));
						}
					}

					dx.advance();
					ni += nn;
				}
				dy.advance();
				nj += nn;
			}
			dz.advance();
			nk += nn;
		}
	}
}

//-----------------------------------------------------------------------------
void FEMultiBlockMesh::BuildMBEdges()
{
	int i, j, n, n1, n2;
	int NF = m_MBFace.size();
	m_MBEdge.clear();
	m_MBEdge.reserve(4*NF);
	int NE = 0;
	for (i=0; i<NF; ++i)
	{
		MBFace& f = m_MBFace[i];
		for (j=0; j<4; ++j)
		{
			n1 = f.m_node[j];
			n2 = f.m_node[(j+1)%4];
			n = FindEdge(n1, n2);

			if (n >= 0)
			{
				MBEdge& e = m_MBEdge[n];
				f.m_edge[j] = n;
			}
			else
			{
				MBEdge e(n1, n2);
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
					e.m_gx = (f.m_bx?f.m_gx:1/f.m_gx);
					e.m_bx = f.m_bx;
					break;
				case 3:
					e.m_nx = f.m_ny;
					e.m_gx = (f.m_by?f.m_gy:1/f.m_gy);
					e.m_bx = f.m_by;
					break;
				}
				m_MBEdge.push_back(e);
				f.m_edge[j] = NE++;
			}
		}
	}

	NE = m_MBEdge.size();

	// find the block edges
	int NN = m_MBNode.size();
	vector< vector<int> > ET(NN);
	for (int i = 0; i < NE; ++i)
	{
		MBEdge& ei = m_MBEdge[i];
		ET[ei.m_node[0]].push_back(i);
		ET[ei.m_node[1]].push_back(i);
	}

	const int EL[12][2] = { {0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7} };
	int NB = m_MBlock.size();
	for (int i = 0; i < NB; ++i)
	{
		MBBlock& b = m_MBlock[i];
		for (int j = 0; j < 12; ++j)
		{
			b.m_edge[j] = -1;

			int n0 = b.m_node[EL[j][0]];
			int n1 = b.m_node[EL[j][1]];

			int ne = ET[n0].size();
			for (int k = 0; k < ne; ++k)
			{
				MBEdge& ek = m_MBEdge[ET[n0][k]];
				if (((ek.Node(0) == n0) && (ek.Node(1) == n1))||
					((ek.Node(0) == n1) && (ek.Node(1) == n0)))
				{
					b.m_edge[j] = ET[n0][k];
					break;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// This function builds all the faces of the multi-block
void FEMultiBlockMesh::BuildMBFaces()
{
	// clear all faces
	m_MBFace.clear();

	// get the number of blocks
	int NB = m_MBlock.size();

	// reset all block tags
	for (int i=0; i<NB; ++i) m_MBlock[i].m_ntag = 0;

	// count faces
	int faces = 0;
	for (int i=0; i<NB; ++i)
	{
		MBBlock& B = m_MBlock[i];
		for (int j=0; j<6; ++j)
		{
			if (B.m_Nbr[j] == -1) ++faces;
			else 
			{
				MBBlock& B2 = m_MBlock[ B.m_Nbr[j] ];
				if (B2.m_ntag == 0) ++faces;
			}
		}
		B.m_ntag = 1;
	}

	// create face array
	m_MBFace.resize(faces);

	// reset all block tags
	for (int i=0; i<NB; ++i) m_MBlock[i].m_ntag = 0;

	// fill face array
	faces = 0;
	MBFace f;
	int n1, n2;
	for (int i=0; i<NB; ++i)
	{
		MBBlock& B = m_MBlock[i];
		n1 = i;
		for (int j=0; j<6; ++j)
		{
			bool badd = false;
			n2 = -1;
			if (B.m_Nbr[j] == -1) badd = true;
			else 
			{
				MBBlock& B2 = m_MBlock[ B.m_Nbr[j] ];
				if (B2.m_ntag == 0) { badd = true; n2 = B.m_Nbr[j]; }
				else
				{
					for (int k=0; k<6; ++k) 
					{
						if (B2.m_Nbr[k] == i)
						{
							B.m_face[j] = B2.m_face[k];
							break;
						}
					}
				}
			}

			if (badd)
			{
				f = BuildBlockFace(B, j);
				B.m_face[j] = faces;
				f.m_block[0] = n1;
				f.m_block[1] = n2;
				m_MBFace[faces++] = f;
			}
		}
		B.m_ntag = 1;
	}
}

//-----------------------------------------------------------------------------
// This function finds the neighbors for all blocks.
// It is called from BuildMB().
// At this point it is assumed that the nodes and the blocks are defined.
void FEMultiBlockMesh::FindBlockNeighbours()
{
	// build the node-block table
	// This table stores for each ndoe the list of blocks that the node connects to
	vector< vector<int> > NBT;
	BuildNodeBlockTable(NBT);

	// reset all block's neighbours
	int NB = m_MBlock.size();
	for (int i=0; i<NB; ++i)
	{
		MBBlock& B = m_MBlock[i];
		for (int j=0; j<6; ++j) B.m_Nbr[j] = -1;
	}

	// Now we have to find all the block's neighbours.
	// We do this by looping over all the faces for each block
	// and finding the block that has the same face.
	for (int i=0; i<NB; ++i)
	{
		MBBlock& B = m_MBlock[i];
		for (int j=0; j<6; ++j)
		{
			if (B.m_Nbr[j] == -1)
			{
				// get the block's face
				MBFace face = BuildBlockFace(B, j);

				// pick a node on this face
				int inode = face.m_node[0];

				// loop over all the blocks that connect to this node
				for (int k=0; k<(int) NBT[inode].size(); ++k)
				{
					// get the next block index
					int nb = NBT[inode][k];

					// make sure it is not this block
					if (nb != i)
					{
						// get the other block
						MBBlock& B2 = m_MBlock[nb];

						// find the face index of the matching face
						int l = FindFaceIndex(m_MBlock[nb], face);

						// if we find it, assign the neighbors
						if (l != -1)
						{
							B.m_Nbr[j] = nb;
							B2.m_Nbr[l] = i;
							break;
						}
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------

MBFace FEMultiBlockMesh::BuildBlockFace(MBBlock& B, int j)
{
	MBFace f;
	int* n = f.m_node;
	int* bn = B.m_node;
	switch (j)
	{
	case 0: 
		n[0] = bn[0]; n[1] = bn[1]; n[2] = bn[5]; n[3] = bn[4]; 
		f.m_nx = B.m_nx; f.m_ny = B.m_nz;
		f.m_mx = B.m_mx; f.m_my = B.m_mz;
		f.m_gx = B.m_gx; f.m_gy = B.m_gz;
		f.m_bx = B.m_bx; f.m_by = B.m_bz;
		break;
	case 1: 
		n[0] = bn[1]; n[1] = bn[2]; n[2] = bn[6]; n[3] = bn[5]; 
		f.m_nx = B.m_ny; f.m_ny = B.m_nz;
		f.m_mx = B.m_my; f.m_my = B.m_mz;
		f.m_gx = B.m_gy; f.m_gy = B.m_gz;
		f.m_bx = B.m_by; f.m_by = B.m_bz;
		break;
	case 2: 
		n[0] = bn[2]; n[1] = bn[3]; n[2] = bn[7]; n[3] = bn[6]; 
		f.m_nx = B.m_nx; f.m_ny = B.m_nz;
		f.m_mx = B.m_mx; f.m_my = B.m_mz;
		f.m_gx = (B.m_bx?B.m_gx:1/B.m_gx); f.m_gy = B.m_gz;
		f.m_bx = B.m_bx; f.m_by = B.m_bz;
		break;
	case 3: 
		n[0] = bn[3]; n[1] = bn[0]; n[2] = bn[4]; n[3] = bn[7]; 
		f.m_nx = B.m_ny; f.m_ny = B.m_nz;
		f.m_mx = B.m_my; f.m_my = B.m_mz;
		f.m_gx = (B.m_by?B.m_gy:1/B.m_gy); f.m_gy = B.m_gz;
		f.m_bx = B.m_by; f.m_by = B.m_bz;
		break;
	case 4: 
		n[0] = bn[3]; n[1] = bn[2]; n[2] = bn[1]; n[3] = bn[0]; 
		f.m_nx = B.m_nx; f.m_ny = B.m_ny;
		f.m_mx = B.m_mx; f.m_my = B.m_my;
		f.m_gx = B.m_gx; f.m_gy = (B.m_by?B.m_gy:1/B.m_gy);
		f.m_bx = B.m_bx; f.m_by = B.m_by;
		break;
	case 5: 
		n[0] = bn[4]; n[1] = bn[5]; n[2] = bn[6]; n[3] = bn[7]; 
		f.m_nx = B.m_nx; f.m_ny = B.m_ny;
		f.m_mx = B.m_mx; f.m_my = B.m_my;
		f.m_gx = B.m_gx; f.m_gy = B.m_gy;
		f.m_bx = B.m_bx; f.m_by = B.m_by;
		break;
	default:
		assert(false);
	}
	return f;
}

//-----------------------------------------------------------------------------

int FEMultiBlockMesh::FindEdgeIndex(MBFace& F, int n1, int n2)
{
	int m1, m2;
	for (int j=0; j<4; ++j)
	{
		m1 = F.m_node[j];
		m2 = F.m_node[(j+1)%4];
		if (((n1==m1) && (n2==m2))||((n1==m2)&& (n2==m1))) return j;
	}
	return -1;
}

//-----------------------------------------------------------------------------

int FEMultiBlockMesh::FindFaceIndex(MBBlock &B, MBFace &face)
{
	int i;
	MBFace f2;
	for (i=0; i<6; ++i)
	{
		f2 = BuildBlockFace(B, i);
		if (f2 == face) return i;
	}
	return -1;
}

//-----------------------------------------------------------------------------

void FEMultiBlockMesh::BuildNodeFaceTable(vector< vector<int> >& NFT)
{
	int i, j, n;

	int NN = m_MBNode.size();
	int NF = m_MBFace.size();

	// calculate node valences
	for (i=0; i<NN; ++i) m_MBNode[i].m_ntag = 0;

	for (i=0; i<NF; ++i)
	{
		MBFace& F = m_MBFace[i];
		for (j=0; j<4; ++j) m_MBNode[ F.m_node[j] ].m_ntag++;
	}

	// create the node-block array
	NFT.resize(NN);
	for (i=0; i<NN; ++i) 
	{
		if (m_MBNode[i].m_ntag > 0)
		{
			NFT[i].resize(m_MBNode[i].m_ntag);
			m_MBNode[i].m_ntag = 0;
		}
	}

	// fill the node-block array
	for (i=0; i<NF; ++i)
	{
		MBFace& F = m_MBFace[i];
		for (j=0; j<4; ++j)
		{
			n = F.m_node[j];
			MBNode& node = m_MBNode[n];
			NFT[n][node.m_ntag++] = i;
		}
	}
}

//-----------------------------------------------------------------------------

void FEMultiBlockMesh::BuildNodeBlockTable(vector<vector<int> > &NBT)
{
	int i, j, n;

	int NN = m_MBNode.size();
	int NB = m_MBlock.size();

	// calculate node valences
	for (i=0; i<NN; ++i) m_MBNode[i].m_ntag = 0;

	for (i=0; i<NB; ++i)
	{
		MBBlock& B = m_MBlock[i];
		for (j=0; j<8; ++j) m_MBNode[ B.m_node[j] ].m_ntag++;
	}

	// create the node-block array
	NBT.resize(NN);
	for (i=0; i<NN; ++i) 
	{
		if (m_MBNode[i].m_ntag > 0)
		{
			NBT[i].resize(m_MBNode[i].m_ntag);
			m_MBNode[i].m_ntag = 0;
		}
	}

	// fill the node-block array
	for (i=0; i<NB; ++i)
	{
		MBBlock& B = m_MBlock[i];
		for (j=0; j<8; ++j)
		{
			n = B.m_node[j];
			MBNode& node = m_MBNode[n];
			NBT[n][node.m_ntag++] = i;
		}
	}
}

//-----------------------------------------------------------------------------

int FEMultiBlockMesh::FindEdge(int n1, int n2)
{
	MBEdge e(n1, n2);

	int NE = m_MBEdge.size();

	for (int i=0; i<NE; ++i)
	{
		MBEdge& e2 = m_MBEdge[i];
		if (e2 == e) return i;
	}

	return -1;
}

//-----------------------------------------------------------------------------

int FEMultiBlockMesh::GetBlockNodeIndex(MBBlock &b, int i, int j, int k)
{
	if (i==0) 
	{
		return GetBlockFaceNodeIndex(b, 3, b.m_ny-j, k);
	}
	else if (i==b.m_nx)
	{
		return GetBlockFaceNodeIndex(b, 1, j, k);
	}
	else if (j==0)
	{
		return GetBlockFaceNodeIndex(b, 0, i, k);
	}
	else if (j==b.m_ny)
	{
		return GetBlockFaceNodeIndex(b, 2, b.m_nx-i, k);
	}
	else if (k==0)
	{
		return GetBlockFaceNodeIndex(b, 4, i, b.m_ny-j);
	}
	else if (k==b.m_nz)
	{
		return GetBlockFaceNodeIndex(b, 5, i, j);
	}
	else
	{
		return b.m_ntag + (i-1) + (j-1)*(b.m_nx-1) + (k-1)*(b.m_nx-1)*(b.m_ny-1);
	}
}

//-----------------------------------------------------------------------------

int FEMultiBlockMesh::GetBlockFaceNodeIndex(MBBlock &b, int nf, int i, int j)
{
	// get the face
	MBFace& f = m_MBFace[b.m_face[nf]];

	// find the transformation
	MBFace bf = BuildBlockFace(b, nf);

	int l = -1;
	if      (f.m_node[0] == bf.m_node[0]) l = 0;
	else if (f.m_node[0] == bf.m_node[1]) l = 1;
	else if (f.m_node[0] == bf.m_node[2]) l = 2;
	else if (f.m_node[0] == bf.m_node[3]) l = 3;

	int m = 0;
	if (f.m_node[1] == bf.m_node[(l+1)%4]) m = 1;
	else if (f.m_node[1] == bf.m_node[(l+3)%4]) m = -1;

	assert((l!=-1) && (m!=0));

	// get the face node index
	switch (l)
	{
	case 0:
		if (m==1) return GetFaceNodeIndex(f, i, j);
		else return GetFaceNodeIndex(f, j, i);
		break;
	case 1:
		if (m==1) return GetFaceNodeIndex(f, j, bf.m_mx-i-1);
		else return GetFaceNodeIndex(f, bf.m_mx-i-1, j);
		break;
	case 2:
		if (m==1) return GetFaceNodeIndex(f, bf.m_mx-i-1, bf.m_my-j-1);
		else return GetFaceNodeIndex(f, bf.m_my-j-1, bf.m_mx-i-1);
		break;
	case 3:
		if (m==1) return GetFaceNodeIndex(f, bf.m_my-j-1, i);
		else return GetFaceNodeIndex(f, i, bf.m_my-j-1);
		break;
	}

	return 0;
}

//-----------------------------------------------------------------------------

int FEMultiBlockMesh::GetFaceNodeIndex(MBFace& f, int i, int j)
{
	if (i==0) 
	{
		return GetFaceEdgeNodeIndex(f, 3, f.m_my-j-1);
	}
	else if (i==f.m_mx-1)
	{
		return GetFaceEdgeNodeIndex(f, 1, j);
	}
	else if (j==0)
	{
		return GetFaceEdgeNodeIndex(f, 0, i);
	}
	else if (j==f.m_my-1)
	{
		return GetFaceEdgeNodeIndex(f, 2, f.m_mx-i-1);
	}
	else
	{
		int n = j * f.m_mx + i;
		return f.m_fenodes[n];
	}
}

//-----------------------------------------------------------------------------
int FEMultiBlockMesh::GetFaceEdgeNodeIndex(MBFace& f, int ne, int i)
{
	// get the edge
	MBEdge& e = m_MBEdge[ f.m_edge[ne] ];

	// next, we need to see if we need to flip the edge or not
	if (e.Node(0) == f.m_node[ne])
	{
		// don't flip the edge
		return GetEdgeNodeIndex(e, i);
	}
	else if (e.Node(1) == f.m_node[ne])
	{
		// do flip the edge
		return GetEdgeNodeIndex(e, e.m_mx-i-1);
	}
	assert(false);
	return -1;
}

//-----------------------------------------------------------------------------

void FEMultiBlockMesh::SetBlockFaceID(MBBlock& b, int n0, int n1, int n2, int n3, int n4, int n5)
{
	m_MBFace[b.m_face[0]].m_gid = n0;
	m_MBFace[b.m_face[1]].m_gid = n1;
	m_MBFace[b.m_face[2]].m_gid = n2;
	m_MBFace[b.m_face[3]].m_gid = n3;
	m_MBFace[b.m_face[4]].m_gid = n4;
	m_MBFace[b.m_face[5]].m_gid = n5;
}

//-----------------------------------------------------------------------------

void FEMultiBlockMesh::SetFaceEdgeID(MBFace& f, int n0, int n1, int n2, int n3)
{
	m_MBEdge[f.m_edge[0]].m_gid = n0;
	m_MBEdge[f.m_edge[1]].m_gid = n1;
	m_MBEdge[f.m_edge[2]].m_gid = n2;
	m_MBEdge[f.m_edge[3]].m_gid = n3;
}

//-----------------------------------------------------------------------------

int FEMultiBlockMesh::GetEdgeNodeIndex(MBEdge& e, int i)
{
	if (i==0) 
	{
		return m_MBNode[e.Node(0)].m_ntag;
	}
	else if (i==e.m_mx-1)
	{
		return m_MBNode[e.Node(1)].m_ntag;
	}
	else
	{
		return e.m_fenodes[i];
	}
}

//-----------------------------------------------------------------------------

MBFace& FEMultiBlockMesh::GetBlockFace(int nb, int nf)
{
	return m_MBFace[ m_MBlock[nb].m_face[nf] ];
}

//-----------------------------------------------------------------------------
MBEdge& FEMultiBlockMesh::GetBlockEdge(int nblock, int nedge)
{
	MBBlock& b = m_MBlock[nblock];
	int eid = b.m_edge[nedge];
	assert(eid >= 0);
	return m_MBEdge[eid];
}

//-----------------------------------------------------------------------------
MBNode& FEMultiBlockMesh::AddNode(const vec3d& r, int nodeType)
{
	MBNode node;
	node.m_r = r;
	node.m_type = nodeType;
	m_MBNode.push_back(node);
	return m_MBNode[m_MBNode.size() - 1];
}

//-----------------------------------------------------------------------------
MBBlock& FEMultiBlockMesh::AddBlock(int n0, int n1, int n2, int n3, int n4, int n5, int n6, int n7)
{
	MBBlock b;
	b.SetNodes(n0, n1, n2, n3, n4, n5, n6, n7);
	m_MBlock.push_back(b);
	return m_MBlock[m_MBlock.size() - 1];
}

//-----------------------------------------------------------------------------
MBBlock& FEMultiBlockMesh::AddBlock()
{
	MBBlock b;
	m_MBlock.push_back(b);
	return m_MBlock[m_MBlock.size() - 1];
}

//-----------------------------------------------------------------------------
MBEdge& FEMultiBlockMesh::GetEdge(int nedge)
{
	return m_MBEdge[nedge];
}

//-----------------------------------------------------------------------------

MBEdge& FEMultiBlockMesh::GetFaceEdge(MBFace& f, int n)
{
	return m_MBEdge[ f.m_edge[n] ];
}

//-----------------------------------------------------------------------------
MBFace& FEMultiBlockMesh::AddFace()
{
	m_MBFace.push_back(MBFace());
	return m_MBFace.back();
}

//-----------------------------------------------------------------------------
MBEdge& FEMultiBlockMesh::AddEdge()
{
	m_MBEdge.push_back(MBEdge());
	return m_MBEdge.back();
}

//-----------------------------------------------------------------------------
void FEMultiBlockMesh::BuildMB()
{
	FindBlockNeighbours();
	BuildMBFaces();
	BuildMBEdges();
}

//-----------------------------------------------------------------------------
void FEMultiBlockMesh::UpdateMB()
{
	// update face geometry data
	for (int i = 0; i < m_MBFace.size(); ++i)
	{
		MBFace& f = m_MBFace[i];

		// figure out edge winding
		for (int j = 0; j < 4; ++j)
		{
			int n0 = f.m_node[j];
			int n1 = f.m_node[(j+1)%4];
			MBEdge& ej = m_MBEdge[f.m_edge[j]];
			if      ((n0 == ej.Node(0)) && (n1 == ej.Node(1))) f.m_edgeWinding[j] =  1;
			else if ((n0 == ej.Node(1)) && (n1 == ej.Node(0))) f.m_edgeWinding[j] = -1;
			else { assert(false); }
		}

		// see if this face is a sphere
		// it is assumed a sphere if all edges are 3P arcs with the same center node
		f.m_isSphere = true;
		f.m_sphereRadius = 0;
		f.m_sphereCenter = vec3d(0, 0, 0);
		int c0 = m_MBEdge[f.m_edge[0]].m_cnode;
		for (int j = 0; j < 4; ++j)
		{
			MBEdge& edgej = m_MBEdge[f.m_edge[j]];
			if ((edgej.m_ntype != EDGE_3P_CIRC_ARC) || (edgej.m_cnode != c0))
			{
				f.m_isSphere = false;
				break;
			}
		}
		if (f.m_isSphere)
		{
			// we assume that the corner nodes are already on the sphere
			vec3d r0 = m_MBNode[f.m_node[0]].m_r;
			f.m_sphereCenter = m_MBNode[c0].m_r;
			f.m_sphereRadius = (r0 - f.m_sphereCenter).Length();
		}

		// see if it is a revolved surface
		f.m_isRevolve = false;
		f.m_nrevolveEdge = -1;
		if ((m_MBEdge[f.m_edge[0]].m_ntype == EDGE_ZARC) &&
			(m_MBEdge[f.m_edge[2]].m_ntype == EDGE_ZARC))
		{
			f.m_isRevolve = true;
			f.m_nrevolveEdge = 1;
		}
		if ((m_MBEdge[f.m_edge[1]].m_ntype == EDGE_ZARC) &&
			(m_MBEdge[f.m_edge[3]].m_ntype == EDGE_ZARC))
		{
			f.m_isRevolve = true;
			f.m_nrevolveEdge = 0;
		}
	}
}

//-----------------------------------------------------------------------------
int FEMultiBlockMesh::GetFENode(MBNode& node)
{
	return node.m_ntag;
}

vector<int> FEMultiBlockMesh::GetFENodeList(MBEdge& edge)
{
	vector<int> nodeList;
	nodeList.push_back(m_MBNode[edge.Node(0)].m_fenodes[0]);
	nodeList.insert(nodeList.end(), edge.m_fenodes.begin(), edge.m_fenodes.end());
	nodeList.push_back(m_MBNode[edge.Node(1)].m_fenodes[0]);
	return nodeList;
}

vector<int> FEMultiBlockMesh::GetFENodeList(MBFace& face)
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

vector<int> FEMultiBlockMesh::GetFENodeList(MBBlock& block)
{
	vector<int> f1 = GetFENodeList(m_MBFace[block.m_face[0]]);
	vector<int> f2 = GetFENodeList(m_MBFace[block.m_face[1]]);
	vector<int> f3 = GetFENodeList(m_MBFace[block.m_face[2]]);
	vector<int> f4 = GetFENodeList(m_MBFace[block.m_face[3]]);
	vector<int> f5 = GetFENodeList(m_MBFace[block.m_face[4]]);
	vector<int> f6 = GetFENodeList(m_MBFace[block.m_face[5]]);

	vector<int> nodeList = block.m_fenodes;
	nodeList.insert(nodeList.end(), f1.begin(), f1.end());
	nodeList.insert(nodeList.end(), f2.begin(), f2.end());
	nodeList.insert(nodeList.end(), f3.begin(), f3.end());
	nodeList.insert(nodeList.end(), f4.begin(), f4.end());
	nodeList.insert(nodeList.end(), f5.begin(), f5.end());
	nodeList.insert(nodeList.end(), f6.begin(), f6.end());

	return nodeList;
}

void FEMultiBlockMesh::ClearMeshSettings()
{
	for (MBEdge& e : m_MBEdge)
	{
		e.m_nx = 0;
		e.m_gx = 1.0;
		e.m_bx = false;
	}
	for (MBFace& f : m_MBFace)
	{
		f.m_nx = f.m_ny = 0;
		f.m_gx = f.m_gy = 1.0;
		f.m_bx = f.m_by = false;
	}
	for (MBBlock& b : m_MBlock)
	{
		b.m_nx = b.m_ny = b.m_nz = 0;
		b.m_gx = b.m_gy = b.m_gz = 1.0;
		b.m_bx = b.m_by = b.m_bz = false;
	}
}

bool FEMultiBlockMesh::SetEdgeDivisions(int iedge, int nd)
{
	MBEdge& edge = m_MBEdge[iedge];
	if (edge.m_nx == nd) return true;	// edge divisions unchanged
//	if (edge.m_nx != 0) return false; // edge divisions already set. Cannot override.

	for (MBEdge& e : m_MBEdge) e.m_ntag = 0;
	edge.m_ntag = nd;

	// propagate new edge size
	const int ET[3][4] = {
		{ 0, 2,  4,  6 },
		{ 1, 3,  5,  7 },
		{ 8, 9, 10, 11 }
	};

	const int EC[12] = {0, 1, 0, 1, 0, 1, 0, 1, 2, 2, 2, 2};

	bool done = false;
	while (!done)
	{
		done = true;
		for (MBBlock& b : m_MBlock)
		{
			for (int i = 0; i < 12; ++i)
			{
				MBEdge& ei = m_MBEdge[b.m_edge[i]];
				if (ei.m_ntag == nd)
				{
					int ec = EC[i];
					const int* et = ET[ec];
					for (int j = 0; j < 4; ++j)
					{
						MBEdge& ej = m_MBEdge[b.m_edge[et[j]]];
						if (ej.m_ntag == 0) {
							ej.m_ntag = nd; 
							done = false;
						}
					}
				}
			}
		}
	}

	// okay, everything looks good, so let's keep the new edge divisions
	for (MBEdge& e : m_MBEdge) {
		if (e.m_ntag == nd) e.m_nx = nd;
	}

	return true;
}

bool FEMultiBlockMesh::SetDefaultDivisions(int nd)
{
	for (MBEdge& e : m_MBEdge)
	{
		if (e.m_nx == 0) e.m_nx = nd;
	}

	// set face meshing stats
	for (MBFace& f : m_MBFace)
	{
		MBEdge& e0 = m_MBEdge[f.m_edge[0]];
		MBEdge& e1 = m_MBEdge[f.m_edge[1]];

		f.m_nx = e0.m_nx; assert(f.m_nx > 0);
		f.m_ny = e1.m_nx; assert(f.m_ny > 0);

		if (f.m_edgeWinding[0] == 1) f.m_gx = e0.m_gx; else f.m_gx = 1.0 / e0.m_gx;
		if (f.m_edgeWinding[1] == 1) f.m_gy = e1.m_gx; else f.m_gy = 1.0 / e1.m_gx;

		if (m_MBEdge[f.m_edge[2]].m_nx != f.m_nx) return false;
		if (m_MBEdge[f.m_edge[3]].m_nx != f.m_ny) return false;
	}

	// set block meshing stats
//	const int EL[12][2] = { {0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7} };
	for (MBBlock& b : m_MBlock)
	{
		MBEdge& e0 = m_MBEdge[b.m_edge[0]];
		MBEdge& e1 = m_MBEdge[b.m_edge[1]];
		MBEdge& e2 = m_MBEdge[b.m_edge[8]];

		b.m_nx = e0.m_nx; assert(b.m_nx > 0);
		b.m_ny = e1.m_nx; assert(b.m_ny > 0);
		b.m_nz = e2.m_nx; assert(b.m_nz > 0);

		if      ((e0.m_node[0] == b.m_node[0]) && (e0.m_node[1] == b.m_node[1])) b.m_gx = e0.m_gx;
		else if ((e0.m_node[0] == b.m_node[1]) && (e0.m_node[1] == b.m_node[0])) b.m_gx = 1.0 / e0.m_gx;
		else assert(false);

		if      ((e1.m_node[0] == b.m_node[1]) && (e1.m_node[1] == b.m_node[2])) b.m_gy = e1.m_gx;
		else if ((e1.m_node[0] == b.m_node[2]) && (e1.m_node[1] == b.m_node[1])) b.m_gy = 1.0 / e1.m_gx;
		else assert(false);

		if      ((e2.m_node[0] == b.m_node[0]) && (e2.m_node[1] == b.m_node[4])) b.m_gz = e2.m_gx;
		else if ((e2.m_node[0] == b.m_node[4]) && (e2.m_node[1] == b.m_node[0])) b.m_gz = 1.0 / e2.m_gx;
		else assert(false);

		if (m_MBEdge[b.m_edge[2]].m_nx != b.m_nx) return false;
		if (m_MBEdge[b.m_edge[4]].m_nx != b.m_nx) return false;
		if (m_MBEdge[b.m_edge[6]].m_nx != b.m_nx) return false;

		if (m_MBEdge[b.m_edge[3]].m_nx != b.m_ny) return false;
		if (m_MBEdge[b.m_edge[5]].m_nx != b.m_ny) return false;
		if (m_MBEdge[b.m_edge[7]].m_nx != b.m_ny) return false;

		if (m_MBEdge[b.m_edge[ 9]].m_nx != b.m_nz) return false;
		if (m_MBEdge[b.m_edge[10]].m_nx != b.m_nz) return false;
		if (m_MBEdge[b.m_edge[11]].m_nx != b.m_nz) return false;
	}

	return true;
}

bool FEMultiBlockMesh::SetNodeWeights(std::vector<double>& w)
{
	if (w.size() != m_MBNode.size()) return false;

	for (MBEdge& e : m_MBEdge)
	{
		int n0 = e.m_node[0];
		int n1 = e.m_node[1];
		double w0 = w[n0]; if (w0 == 0.0) w0 = 1.0;
		double w1 = w[n1]; if (w1 == 0.0) w1 = 1.0;

		if ((w0 != 1.0) && (w1 == 1.0))
		{
			e.m_gx = w0;
		}
		else if ((w1 != 1.0) && (w0 == 1.0))
		{
			e.m_gx = 1.0 / w1;
		}
		else if ((e.m_nx != 0) && (w0 == w1) && (w0 != 1.0))
		{
			e.m_gx = w0;
			e.m_bx = true;
		}
		else if (w0 != w1) { assert(false); }
	}
    return true;
}

//==============================================================
FEMultiBlockMesher::FEMultiBlockMesher(GMultiBox* po) : m_po(po)
{
	AddDoubleParam(0.1, "h", "Element size");
	AddIntParam(0, "elem", "Element Type")->SetEnumNames("Hex8\0Hex20\0Hex27\0");
}

bool FEMultiBlockMesher::BuildMultiBlock()
{
	if (m_po == nullptr) return false;

	ClearMB();

	GMultiBox& o = *m_po;
	for (int i=0; i<o.Nodes(); ++i)
	{
		GNode* n = o.Node(i);
		MBNode& mbNode = AddNode(n->LocalPosition(), n->Type());
		mbNode.SetID(n->GetLocalID());
	}

	for (int i=0; i<o.Edges(); ++i)
	{
		GEdge* e = o.Edge(i);
		MBEdge& mbEdge = AddEdge();
		mbEdge.m_ntype = e->m_ntype;
		mbEdge.m_node[0] = e->m_node[0];
		mbEdge.m_node[1] = e->m_node[1];
		mbEdge.m_cnode = (e->m_cnode.empty() ? -1 : e->m_cnode[0]);
		mbEdge.m_orient = e->m_orient;
		mbEdge.SetID(e->GetLocalID());
	}

	for (int i=0; i<o.Faces(); ++i)
	{
		GFace* f = o.Face(i);
		if (f->Nodes() != 4) return false;
		MBFace& mbFace = AddFace();
		mbFace.m_gid = f->GetLocalID();
		mbFace.m_block[0] = f->m_nPID[0];
		mbFace.m_block[1] = f->m_nPID[1];
		for (int j = 0; j < 4; ++j) mbFace.m_node[j] = f->m_node[j];
		for (int j = 0; j < 4; ++j) {
			mbFace.m_edge[j] = f->m_edge[j].nid;
			mbFace.m_edgeWinding[j] = f->m_edge[j].nwn;
		}
	}

	for (int i=0; i<o.Parts(); ++i)
	{
		GPart* p = o.Part(i);
		if (p->Nodes() != 8) return false;
		if (p->Edges() != 12) return false;
		if (p->Faces() != 6) return false;
		MBBlock& mbBlock = AddBlock();
		mbBlock.m_gid = p->GetLocalID();
		for (int j = 0; j <  8; ++j) mbBlock.m_node[j] = p->m_node[j];
		for (int j = 0; j < 12; ++j) mbBlock.m_edge[j] = p->m_edge[j];
		for (int j = 0; j <  6; ++j) mbBlock.m_face[j] = p->m_face[j];
	}

	// update block neighbors
	FindBlockNeighbours();

	return true;
}

FSMesh* FEMultiBlockMesher::BuildMesh()
{
	if (m_po == nullptr) return nullptr;
	GMultiBox& o = *m_po;

	// rebuild the multiblock data
	BuildMultiBlock();

	FEMultiBlockMesh& mb = *this;

	// clear all mesh settings
	mb.ClearMeshSettings();

	// assign meshing parameters
	double h = GetFloatValue(ELEM_SIZE);
	if (h < 0) return nullptr;

	// first set the edge divisions
	for (int i = 0; i < o.Edges(); ++i)
	{
		GEdge* edge = o.Edge(i);
		double w = edge->GetMeshWeight();
		if (w <= 0) w = 1;
		double L = edge->Length();
		int nd = (int)(L / h);
		if (nd < 1) nd = 1;
		int nx = (int)(nd * w);
		if (nx < 1) nx = 1;
		mb.SetEdgeDivisions(i, nx);
	}

	bool allConstraintsSatisfied = false;
	while (!allConstraintsSatisfied)
	{
		allConstraintsSatisfied = true;

		// set all face divisions
		for (int i = 0; i < mb.Faces(); ++i)
		{
			MBFace& face = mb.GetFace(i);
			MBEdge& e0 = mb.GetEdge(face.m_edge[0]);
			MBEdge& e1 = mb.GetEdge(face.m_edge[1]);
			MBEdge& e2 = mb.GetEdge(face.m_edge[2]);
			MBEdge& e3 = mb.GetEdge(face.m_edge[3]);
			if (e0.m_nx != e2.m_nx)
			{
				int n = (e0.m_nx > e2.m_nx ? e0.m_nx : e2.m_nx);
				e0.m_nx = e2.m_nx = n;
				allConstraintsSatisfied = false;
			}
			if (e1.m_nx != e3.m_nx)
			{
				int n = (e1.m_nx > e3.m_nx ? e1.m_nx : e3.m_nx);
				e1.m_nx = e3.m_nx = n;
				allConstraintsSatisfied = false;
			}
			face.m_nx = e0.m_nx;
			face.m_ny = e1.m_nx;
		}

		// set all block divisions
//		const int EL[12][2] = { {0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7} };
		const int ELT[3][4] = { {0,2,4,6},{1,3,5,7},{8,9,10,11} };
		for (int i = 0; i < mb.Blocks(); ++i)
		{
			MBBlock& b = mb.GetBlock(i);
			for (int j = 0; j < 3; ++j)
			{
				int m[4];
				for (int k = 0; k < 4; ++k)
				{
					m[k] = mb.GetEdge(b.m_edge[ELT[j][k]]).m_nx;
				}
				if ((m[0] != m[1]) || (m[0] != m[1]) || (m[0] != m[3]))
				{
					allConstraintsSatisfied = false;
					int mmax = m[0];
					if (m[1] > mmax) mmax = m[1];
					if (m[2] > mmax) mmax = m[2];
					if (m[3] > mmax) mmax = m[3];
					for (int k = 0; k < 4; ++k)
					{
						mb.GetEdge(b.m_edge[ELT[j][k]]).m_nx = mmax;
					}
				}
			}
			b.m_nx = mb.GetEdge(b.m_edge[0]).m_nx;
			b.m_ny = mb.GetEdge(b.m_edge[1]).m_nx;
			b.m_nz = mb.GetEdge(b.m_edge[8]).m_nx;
		}
	}

	// set the node weights (which also adjusts the edge zoning)
	vector<double> w(o.Nodes(), 0.0);
	for (int i = 0; i < o.Nodes(); ++i)
	{
		GNode* node = o.Node(i);
		w[i] = node->GetMeshWeight();
	}
	mb.SetNodeWeights(w);

	// set the divisions of the remaining items
//	mb.SetDefaultDivisions(nd);

	int elemType = GetIntValue(ELEM_TYPE);
	switch (elemType)
	{
	case 0: mb.SetElementType(FE_HEX8 ); break;
	case 1: mb.SetElementType(FE_HEX20); break;
	case 2: mb.SetElementType(FE_HEX27); break;
	default:
		assert(false);
	}

	return FEMultiBlockMesh::BuildMesh();
}

void FEMultiBlockMesher::RebuildMB()
{
	// Update edge list of faces
	for (MBFace& f : m_MBFace)
	{
		for (int i = 0; i < 4; ++i)
		{
			int n0 = f.m_node[i];
			int n1 = f.m_node[(i + 1) % 4];
			int ne = FindEdge(n0, n1); assert(ne >= 0);

			f.m_edge[i] = ne;

			MBEdge& ei = m_MBEdge[ne];
			if      ((ei.Node(0) == n0) && (ei.Node(1) == n1)) f.m_edgeWinding[i] =  1;
			else if ((ei.Node(0) == n1) && (ei.Node(1) == n0)) f.m_edgeWinding[i] = -1;
			else assert(false);
		}
	}

	// update block data
	for (MBBlock& b : m_MBlock)
	{
		// update face data
		for (int i = 0; i < 6; ++i)
		{
			b.m_face[i] = -1;
			MBFace fi = BuildBlockFace(b, i);
			
			// TODO: Find a better way
			for (int j = 0; j < m_MBFace.size(); ++j)
			{
				MBFace& fj = m_MBFace[j];
				if (IsSameFace(fi.m_node, fj.m_node))
				{
					b.m_face[i] = j;
					break;
				}
			}
			assert(b.m_face[i] != -1);
		}

		// update edge data
		const int EL[12][2] = { {0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7} };
		for (int i = 0; i < 12; ++i)
		{
			int n0 = b.m_node[EL[i][0]];
			int n1 = b.m_node[EL[i][1]];

			int ne = FindEdge(n0, n1); assert(ne >= 0);
			b.m_edge[i] = ne;
		}

		// update block neighbors
		FindBlockNeighbours();
	}
}

//===============================================================================
FESetMBWeight::FESetMBWeight() : FEModifier("Set MB Weight")
{
	AddDoubleParam(0.0, "weight");
}

FSMesh* FESetMBWeight::Apply(GObject* po, FESelection* sel)
{
	if (sel == nullptr) return nullptr;

	GMultiBox* mb = dynamic_cast<GMultiBox*>(po);
	if (mb)
	{
		FEItemListBuilder* list = sel->CreateItemList();

		double w = GetFloatValue(0);

		GEdgeList* edgeList = dynamic_cast<GEdgeList*>(list);
		if (edgeList)
		{
			vector<GEdge*> edges = edgeList->GetEdgeList();
			for (GEdge* edge : edges)
			{
				edge->SetMeshWeight(w);
			}
		}

		GFaceList* faceList = dynamic_cast<GFaceList*>(list);
		if (faceList)
		{
			vector<GFace*> faces = faceList->GetFaceList();
			for (GFace* face : faces)
			{
				for (int j = 0; j < face->Nodes(); ++j)
				{
					GNode* n = mb->Node(face->m_node[j]);
					n->SetMeshWeight(w);
				}
			}
		}

		// don't call po->BuildMesh() since that deletes the current mesh
		return po->GetFEMesher()->BuildMesh();
	}
	else if (dynamic_cast<GMultiPatch*>(po))
	{
		GMultiPatch* mp = dynamic_cast<GMultiPatch*>(po);

		FEItemListBuilder* list = sel->CreateItemList();

		double w = GetFloatValue(0);

		GEdgeList* edgeList = dynamic_cast<GEdgeList*>(list);
		if (edgeList)
		{
			vector<GEdge*> edges = edgeList->GetEdgeList();
			for (GEdge* edge : edges)
			{
				edge->SetMeshWeight(w);
			}
		}

		GNodeList* nodeList = dynamic_cast<GNodeList*>(list);
		if (nodeList)
		{
			vector<GNode*> nodes = nodeList->GetNodeList();
			for (GNode* node : nodes)
			{
				node->SetMeshWeight(w);
			}
		}

		// don't call po->BuildMesh() since that deletes the current mesh
		return po->GetFEMesher()->BuildMesh();
	}

	return nullptr;
}
