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

#pragma once
#include "FEMesher.h"
#include <vector>
using namespace std;

//-----------------------------------------------------------------------------
class FEShapeModifier
{
public:
	FEShapeModifier();
	virtual ~FEShapeModifier();

	virtual vec3d Apply(const vec3d& r) = 0;
};

//-----------------------------------------------------------------------------
// class MBItem: base class for all Multi Block items.
// members:
//		m_ntag	- used for tagging
//		m_bext	- exterior flag
//		m_gid	- groupd id
//
// The group ID is default to -1 indicating that the item cannot be assigned
// to a group. For instance, most of the FE nodes will not be assigned a gid.
//

class MBItem
{
public:
	MBItem() { m_ntag = 0; m_gid = -1; m_bext = false; m_mod = nullptr; }

	MBItem(const MBItem& it)
	{
		m_ntag = it.m_ntag;
		m_bext = it.m_bext;
		m_gid = it.m_gid;
		m_mod = it.m_mod;
		m_fenodes = it.m_fenodes;
	}

	void operator = (const MBItem& it)
	{
		m_ntag = it.m_ntag;
		m_bext = it.m_bext;
		m_gid = it.m_gid;
		m_mod = it.m_mod;
		m_fenodes = it.m_fenodes;
	}

	void SetID(int n) { m_gid = n; }

public:
	int		m_ntag;	// tag
	bool	m_bext;	// external item flag
	int		m_gid;	// group ID
	FEShapeModifier*	m_mod;

	vector<int>	m_fenodes;
};

class MBNode : public MBItem
{
public:
	vec3d	m_r;	// position of the node
};

class MBEdge : public MBItem
{
public:
	int	m_node[2];	// edge nodes
	int	m_face[2];	// external faces
	int	m_nx;		// tesselation
	double	m_gx;	// zoning
	bool	m_bx;	// single or double zoning

public:
	bool operator == (const MBEdge& e) const
	{
		const int* n1 = m_node;
		const int* n2 = e.m_node;
		if ((n1[0] != n2[0]) && (n1[0] != n2[1])) return false;
		if ((n1[1] != n2[0]) && (n1[1] != n2[1])) return false;
		return true;
	}
};

class MBFace : public MBItem
{
public:
	int	m_node[4];	// face nodes
	int	m_edge[4];	// edge indices
	int	m_block[2];	// owning blocks
	int	m_nx, m_ny;	// face tesselation
	double m_gx, m_gy;	// zoning
	bool m_bx, m_by;	// single or double zoning
	int	m_nbr[4];	// the neighbour faces

public:
	MBFace()
	{
		m_node[0] = m_node[1] = m_node[2] = m_node[3] = -1;
		m_edge[0] = m_edge[1] = m_edge[2] = m_edge[3] = -1;
		m_block[0] = m_block[1] = -1;
		m_nx = m_ny = 1;
		m_gx = m_gy = 1.0;
		m_bx = m_by = false;
		m_nbr[0] = m_nbr[1] = m_nbr[2] = m_nbr[3] = -1;
	}

	bool operator == (const MBFace& f) const
	{
		const int* n1 = m_node;
		const int* n2 = f.m_node;
		if ((n1[0] != n2[0]) && (n1[0] != n2[1]) && (n1[0] != n2[2]) && (n1[0] != n2[3])) return false;
		if ((n1[1] != n2[0]) && (n1[1] != n2[1]) && (n1[1] != n2[2]) && (n1[1] != n2[3])) return false;
		if ((n1[2] != n2[0]) && (n1[2] != n2[1]) && (n1[2] != n2[2]) && (n1[2] != n2[3])) return false;
		if ((n1[3] != n2[0]) && (n1[3] != n2[1]) && (n1[3] != n2[2]) && (n1[3] != n2[3])) return false;
		return true;
	}

	bool IsExternal() { return m_block[1] == -1; }

	void SetSizes(int nx, int ny) { m_nx = nx; m_ny = ny; }
};

class MBBlock : public MBItem
{
public:
	MBBlock()
	{
		m_nx = m_ny = m_nz = -1;
		m_gx = m_gy = m_gz = 1;
		m_bx = m_by = m_bz = false;
	}

	void SetNodes(int n1,int n2,int n3,int n4,int n5,int n6,int n7,int n8);
	void SetSizes(int nx, int ny, int nz) { m_nx = nx; m_ny = ny; m_nz = nz; }
	void SetZoning(double gx, double gy, double gz, bool bx, bool by, bool bz);

public:
	int	m_node[8];			// the eight nodes of the block
	int	m_nx, m_ny, m_nz;	// tesselation in x,y,z
	double	m_gx, m_gy, m_gz;	// zoning
	bool	m_bx, m_by, m_bz;	// single or double zoning

	int m_Nbr[6];	// indices to neighbouring blocks
	int	m_face[6];	// indices to faces
};

//-----------------------------------------------------------------------------
// The multi-block mesh builds an fe mesh from an MB geometry
//
class FEMultiBlockMesh : public FEMesher
{
public:
	// constructor
	FEMultiBlockMesh();

	// destructor
	~FEMultiBlockMesh();

	// build the mesh
	FEMesh* BuildMesh();

protected:
	// update the Multi-Block data
	void UpdateMB();

	MBNode& GetMBNode(int i) { return m_MBNode[i]; }
	MBFace& GetBlockFace(int nb, int nf);
	MBEdge& GetFaceEdge(MBFace& f, int n);

protected:
	void FindBlockNeighbours();
	void FindFaceNeighbours();
	void BuildMBFaces();
	void BuildMBEdges();

	// build the mesh items
	void BuildNodes   (FEMesh* pm);
	void BuildElements(FEMesh* pm);
	void BuildFaces   (FEMesh* pm);
	void BuildEdges   (FEMesh* pm);

	void BuildNodeBlockTable(vector< vector<int> >& NBT);
	void BuildNodeFaceTable(vector< vector<int> >& NFT);
	MBFace BuildBlockFace(MBBlock& B, int j);
	int FindFaceIndex(MBBlock& B, MBFace& face);
	int FindEdgeIndex(MBFace& F, int n1, int n2);
	int FindEdge(int n1, int n2);

	int GetBlockNodeIndex(MBBlock& b, int i, int j, int k);
	int GetFaceNodeIndex(MBFace& f, int i, int j);
	int GetEdgeNodeIndex(MBEdge& e, int i);
	void SetBlockFaceID(MBBlock& b, int n0, int n1, int n2, int n3, int n4, int n5);
	void SetFaceEdgeID(MBFace& f, int n0, int n1, int n2, int n3);

	int GetBlockFaceNodeIndex(MBBlock& b, int nf, int i, int j);
	int GetFaceEdgeNodeIndex(MBFace& f, int ne, int i);

public:
	void SetGlobalShapeModifier(FEShapeModifier* shapeMod);
	void SetShapeModifier(MBBlock& b, FEShapeModifier* mod);
	void SetShapeModifier(MBFace& f, FEShapeModifier* mod);
	void SetShapeModifier(MBEdge& e, FEShapeModifier* mod);
	void SetShapeModifier(MBNode& n, FEShapeModifier* mod);

protected:
	void ApplyMeshModifiers(FEMesh* pm);

	int GetFENode(MBNode& node);
	vector<int> GetFENodeList(MBEdge& node);
	vector<int> GetFENodeList(MBFace& node);
	vector<int> GetFENodeList(MBBlock& node);

protected:
	vector<MBBlock>	m_MBlock;
	vector<MBFace>	m_MBFace;
	vector<MBEdge>	m_MBEdge;
	vector<MBNode>	m_MBNode;
};
