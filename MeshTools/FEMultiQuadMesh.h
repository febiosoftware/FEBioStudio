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
	FEMultiQuadMesh(GObject& o);

	// destructor
	virtual ~FEMultiQuadMesh();

	// build the mesh
	FSMesh* BuildMQMesh();

	// set the quad mesh flag
	void SetElementType(int elemType);

	// build the mesh from the object
	bool Build();

	// build the multi-quad data
	virtual bool BuildMultiQuad();

	// clear the MQ data
	void ClearMQ();

public:
	int Nodes() const;
	MBNode& AddNode(const vec3d& r, int ntype = NODE_VERTEX);
	MBNode& GetMBNode(int i) { return m_MBNode[i]; }

	int Edges() const;
	MBEdge& GetEdge(int i);
	MBEdge& AddEdge();

	int Faces() const;
	MBFace& AddFace(int n0, int n1, int n2, int n3);
	MBFace& AddFace();
	MBFace& GetFace(int n);

	void SetFaceEdgeIDs(int nface, int n0, int n1, int n2, int n3);

	MBEdge& GetFaceEdge(int nface, int nedge);

	void SetFaceSizes(int nface, int nx, int ny);

	void ClearMeshSettings();

	bool SetEdgeDivisions(int iedge, int nd);
	bool SetDefaultDivisions(int nd);
	bool SetNodeWeights(std::vector<double>& w);

protected:
	void BuildMBEdges();

	// build the mesh items
	void BuildFENodes(FSMesh* pm);
	void BuildFEFaces(FSMesh* pm);
	void BuildFEEdges(FSMesh* pm);

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
	vector<MBFace>	m_MBFace;
	vector<MBEdge>	m_MBEdge;
	vector<MBNode>	m_MBNode;

	FSMesh* m_pm;
	FSNode* m_currentNode;
	int		m_nodes;
};

class GMultiPatch;

class FEMultiQuadMesher : public FEMultiQuadMesh
{
	enum { DIVS, ELEM_TYPE };

public:
	FEMultiQuadMesher(GObject& o);

	// build the mesh
	FSMesh* BuildMesh() override;

	bool BuildMultiQuad() override;

private:
	GMultiPatch* m_po;
};
