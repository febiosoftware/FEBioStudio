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

#pragma once
#include "FEMesher.h"
#include "FEModifier.h"
#include <GeomLib/GObject.h>
#include <vector>

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
	MBItem() { m_ntag = 0; m_gid = -1; }

	MBItem(const MBItem& it)
	{
		m_ntag = it.m_ntag;
		m_gid = it.m_gid;
		m_fenodes = it.m_fenodes;
	}

	void operator = (const MBItem& it)
	{
		m_ntag = it.m_ntag;
		m_gid = it.m_gid;
		m_fenodes = it.m_fenodes;
	}

	void SetID(int n) { m_gid = n; }

public:
	int		m_ntag;	// tag
	int		m_gid;	// group ID

	std::vector<int>	m_fenodes;
};

class MBNode : public MBItem
{
public:
	vec3d	m_r;	// position of the node
	int		m_type;

	MBNode()
	{
		m_type = NODE_VERTEX;
	}
};

class MBEdge : public MBItem
{
public:
	int		m_node[2];	// indices of GNodes
	int		m_cnode;	// center node for arcs
	int		m_orient;	// orientation for arcs
	int		m_ntype;	// type identifier

	int		m_nx;		// tesselation
	double	m_gx;	// zoning
	bool	m_bx;	// single or double zoning
	
	int		m_mx;	// size of fenodes (don't set this manually!)

	int Node(int i) const { return m_node[i]; }

public:
	MBEdge() 
	{
		m_nx = 1;
		m_gx = 1;
		m_bx = false;
		m_orient = 1;
		m_cnode = -1;
	}
	MBEdge(int n0, int n1) { m_node[0] = n0; m_node[1] = n1; m_ntype = EDGE_LINE; m_orient = 1; m_cnode = -1; }
	bool operator == (const MBEdge& e) const
	{
		const int* n1 = m_node;
		const int* n2 = e.m_node;
		if ((n1[0] != n2[0]) && (n1[0] != n2[1])) return false;
		if ((n1[1] != n2[0]) && (n1[1] != n2[1])) return false;
		return true;
	}

	MBEdge& SetWinding(int w) { m_orient = w; return *this; }
	MBEdge& SetType(int ntype) { m_ntype = ntype; return *this; }

	MBEdge& SetEdge(int ntype, int nwinding, int cnode = -1)
	{
		m_orient = nwinding;
		m_ntype = ntype;
		m_cnode = cnode;
		return *this;
	}
};

class MBFace : public MBItem
{
public:
	int	m_node[4];	// face nodes
	int	m_edge[4];	// edge indices
	int	m_edgeWinding[4];	// edge winding 1 = positive, -1 is negative
	int	m_block[2];	// owning blocks
	int	m_nx, m_ny;	// face tesselation
	double m_gx, m_gy;	// zoning
	bool m_bx, m_by;	// single or double zoning
	int	m_sid;		// surface smoothgin ID

	// data for sphere sections
	bool	m_isSphere;
	vec3d	m_sphereCenter;
	double	m_sphereRadius;

	// data for revolve sections
	bool	m_isRevolve;
	int		m_nrevolveEdge;

	int		m_mx, m_my;

public:
	MBFace()
	{
		m_node[0] = m_node[1] = m_node[2] = m_node[3] = -1;
		m_edge[0] = m_edge[1] = m_edge[2] = m_edge[3] = -1;
		m_block[0] = m_block[1] = -1;
		m_nx = m_ny = 1;
		m_gx = m_gy = 1.0;
		m_bx = m_by = false;
		m_edgeWinding[0] = m_edgeWinding[1] = m_edgeWinding[2] = m_edgeWinding[3] = 0;
		m_sid = -1; // -1 = use GID instead

		m_isSphere  = false;
		m_isRevolve = false;
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

	MBFace& SetSizes(int nx, int ny) { m_nx = nx; m_ny = ny; return *this; }
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
	MBBlock& SetSizes(int nx, int ny, int nz) { m_nx = nx; m_ny = ny; m_nz = nz; return *this; }
	MBBlock& SetZoning(double gx, double gy, double gz, bool bx, bool by, bool bz);

public:
	int	m_node[8];			// the eight nodes of the block
	int	m_nx, m_ny, m_nz;	// tesselation in x,y,z
	double	m_gx, m_gy, m_gz;	// zoning
	bool	m_bx, m_by, m_bz;	// single or double zoning

	int		m_mx, m_my, m_mz;

	int m_Nbr[6];	// indices to neighbouring blocks
	int	m_face[6];	// indices to faces
	int m_edge[12];	// indices to edges
};

//-----------------------------------------------------------------------------
// The multi-block mesh builds an fe mesh from an MB geometry
//
class FEMultiBlockMesh : public FEMesher
{
public:
	// constructor
	FEMultiBlockMesh(GObject& o);

	void CopyFrom(const FEMultiBlockMesh& mb);

	// destructor
	~FEMultiBlockMesh();

	void SetElementType(int elemType);

	virtual bool BuildMultiBlock();

	void ClearMB();

	FSMesh* BuildMBMesh();

	MBNode& AddNode(const vec3d& r, int nodeType = NODE_VERTEX);

	MBBlock& AddBlock(int n0, int n1, int n2, int n3, int n4, int n5, int n6, int n7);

	MBBlock& AddBlock();

	void ClearMeshSettings();

	bool SetEdgeDivisions(int iedge, int nd);
	bool SetDefaultDivisions(int nd);
	bool SetNodeWeights(std::vector<double>& w);

	// update the Multi-Block data
	void UpdateMB();

	void BuildMB();

	int Nodes() const { return (int)m_MBNode.size(); }

	MBNode& GetMBNode(int i) { return m_MBNode[i]; }
	const MBNode& GetMBNode(int i) const { return m_MBNode[i]; }
	MBFace& GetBlockFace(int nb, int nf);
	MBEdge& GetFaceEdge(MBFace& f, int n);
	MBFace& AddFace();

	int Edges() const { return (int)m_MBEdge.size(); }
	MBEdge& GetEdge(int nedge);
	MBEdge& AddEdge();

	int Blocks() const { return (int)m_MBlock.size(); }
	MBBlock& GetBlock(int i) { return m_MBlock[i]; }

	MBEdge& GetBlockEdge(int nblock, int nedge);

	void SetBlockFaceID(MBBlock& b, int n0, int n1, int n2, int n3, int n4, int n5);
	void SetFaceEdgeID(MBFace& f, int n0, int n1, int n2, int n3);

	int Faces() const { return (int)m_MBFace.size(); }
	MBFace& GetFace(int i) { return m_MBFace[i]; }

protected:
	void FindBlockNeighbours();
	void BuildMBFaces();
	void BuildMBEdges();

	// build the mesh items
	void BuildFENodes   (FSMesh* pm);
	void BuildFEElements(FSMesh* pm);
	void BuildFEFaces   (FSMesh* pm);
	void BuildFEEdges   (FSMesh* pm);

	void BuildNodeBlockTable(std::vector< std::vector<int> >& NBT);
	void BuildNodeFaceTable(std::vector< std::vector<int> >& NFT);
	MBFace BuildBlockFace(MBBlock& B, int j);
	int FindFaceIndex(MBBlock& B, MBFace& face);
	int FindEdgeIndex(MBFace& F, int n1, int n2);
	int FindEdge(int n1, int n2);

	int GetBlockNodeIndex(MBBlock& b, int i, int j, int k);
	int GetFaceNodeIndex(MBFace& f, int i, int j);
	int GetEdgeNodeIndex(MBEdge& e, int i);

	int GetBlockFaceNodeIndex(MBBlock& b, int nf, int i, int j);
	int GetFaceEdgeNodeIndex(MBFace& f, int ne, int i);

protected:
	class MQPoint
	{
	public:
		int	m_i, m_j, m_k;
		double	m_r, m_s, m_t;

		MQPoint() { m_i = m_j = m_k = -1; m_r = m_s = m_t = 0.0; }
		MQPoint(int i, double r) { m_i = i; m_j = m_k = -1; m_r = r; m_s = m_t = 0.0; }
		MQPoint(int i, int j, double r, double s) { m_i = i; m_j = j; m_k = -1;  m_r = r; m_s = s; m_t = 0.0; }
		MQPoint(int i, int j, int k, double r, double s, double t) { m_i = i; m_j = j; m_k = k;  m_r = r; m_s = s; m_t = t; }
	};

	vec3d EdgePosition (MBEdge& E, const MQPoint& q);
	vec3d FacePosition (MBFace& F, const MQPoint& q);
	vec3d BlockPosition(MBBlock& B, const MQPoint& q);

protected:
	int GetFENode(MBNode& node);
	std::vector<int> GetFENodeList(MBEdge& node);
	std::vector<int> GetFENodeList(MBFace& node);
	std::vector<int> GetFENodeList(MBBlock& node);

	int AddFENode(const vec3d& r, int gid = -1);
	int AddFEEdgeNode(MBEdge& E, const MQPoint& q);
	int AddFEFaceNode(MBFace& F, const MQPoint& q);
	int AddFEElemNode(MBBlock& B, const MQPoint& q);

protected:
	std::vector<MBBlock>	m_MBlock;
	std::vector<MBFace>	m_MBFace;
	std::vector<MBEdge>	m_MBEdge;
	std::vector<MBNode>	m_MBNode;

	int		m_elemType;
	bool	m_quadMesh;

	FSMesh* m_pm;
	FSNode* m_currentNode;
	int		m_nodes;
};

class Sampler1D
{
public:
	Sampler1D(int n, double bias, bool symm) : m_steps(n), m_bias(bias), m_symm(symm)
	{
		m_fr = bias;
		m_gr = 1;
		if (symm)
		{
			m_gr = 2; if (n % 2) m_gr += bias;
			for (int j = 0; j < n / 2 - 1; ++j) m_gr = bias * m_gr + 2;
			m_gr = 1 / m_gr;
		}
		else
		{
			for (int j = 0; j < n - 1; ++j) m_gr = bias * m_gr + 1;
			m_gr = 1 / m_gr;
		}

		m_n = 0;
		m_r = 0;
		m_dr = m_gr;
	}

	double value() const { return m_r; }

	double increment() const { return m_dr; }

	void advance()
	{
		m_r += m_dr;
		m_dr *= m_fr;
		if (m_symm && (m_n == m_steps / 2 - 1))
		{
			if (m_steps % 2 == 0) m_dr /= m_fr;
			m_fr = 1.0 / m_fr;
		}
		m_n++;
	}

	void reset()
	{
		m_n = 0;
		m_r = 0;
		m_dr = m_gr;
		m_fr = m_bias;
	}

private:
	int		m_n;
	int		m_steps;
	double	m_bias;
	bool	m_symm;

	double	m_r;
	double	m_dr;
	double	m_gr;
	double	m_fr;
};

class GMultiBox;

class FEMultiBlockMesher : public FEMultiBlockMesh
{
	enum { ELEM_SIZE, ELEM_TYPE };

public:
	FEMultiBlockMesher(GObject& o);

	// build the mesh
	FSMesh* BuildMesh() override;

	bool BuildMultiBlock() override;

private:
	// rebuild MB after loading
	void RebuildMB();
};

class FESetMBWeight : public FEModifier
{
public:
	FESetMBWeight();

	//! Apply the weld modifier
	FSMesh* Apply(GObject* po, FESelection* sel);
};
