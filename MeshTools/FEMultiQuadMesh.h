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
#include "FEMultiBlockMesh.h"
#include <vector>

using std::vector;

//-----------------------------------------------------------------------------
// The multi-quad mesh builds an fe mesh from an MB geometry
//
class FEMultiQuadMesh : public FEMesher
{
public:
	// constructor
	FEMultiQuadMesh();

	// destructor
	~FEMultiQuadMesh();

	// build the mesh
	FEMesh* BuildMesh();

	// set the quad mesh flag
	void SetElementType(int elemType);

	// build the mesh from the object
	bool Build(GObject* po);

public:
	MBNode& AddNode(const vec3d& r, int ntype = NODE_VERTEX);
	MBFace& AddFace(int n0, int n1, int n2, int n3);

	MBFace& GetFace(int n);

	// update the Multii-Block data
	void UpdateMQ();

	void SetFaceEdgeIDs(int nface, int n0, int n1, int n2, int n3);

	MBNode& GetMBNode(int i) { return m_MBNode[i]; }
	MBEdge& GetFaceEdge(int nface, int nedge);

	void SetFaceSizes(int nface, int nx, int ny);

protected:
	void FindFaceNeighbours();
	void BuildMBEdges();

	// build the mesh items
	void BuildFENodes(FEMesh* pm);
	void BuildFEFaces(FEMesh* pm);
	void BuildFEEdges(FEMesh* pm);

	void BuildNodeFaceTable(vector< vector<int> >& NFT);
	int FindEdgeIndex(MBFace& F, int n1, int n2);
	int FindEdge(int n1, int n2);

	int GetEdgeNodeIndex(MBEdge& e, int i);

	int GetFaceEdgeNodeIndex(MBFace& f, int ne, int i);

protected:
	class MQPoint
	{
	public:
		int	m_i, m_j;
		double	m_r, m_s;

		MQPoint() { m_i = m_j = -1; m_r = m_s = 0.0; }
		MQPoint(int i, double r) { m_i = i; m_j = -1; m_r = r; m_s = 0.0; }
		MQPoint(int i, int j, double r, double s) { m_i = i; m_j = j; m_r = r; m_s = s; }
	};

protected:
	int GetFENode(MBNode& node);

	vec3d EdgePosition(MBEdge& edge, const MQPoint& q);
	vec3d FacePosition(MBFace& face, const MQPoint& q);

	int AddFENode(const vec3d& r, int gid = -1);

protected:
	int AddFENode(MBNode& N);
	int AddFEEdgeNode(MBEdge& E, const MQPoint& q);
	int AddFEFaceNode(MBFace& F, const MQPoint& q);

private:
	int		m_elemType;	// element type to generate
	bool	m_bquadMesh;

protected:
	GObject*		m_po;
	vector<MBFace>	m_MBFace;
	vector<MBEdge>	m_MBEdge;
	vector<MBNode>	m_MBNode;

	FEMesh* m_pm;
	FENode* m_currentNode;
	int		m_nodes;
};
