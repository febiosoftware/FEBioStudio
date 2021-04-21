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
#include "FEMultiBlockMesh.h"
#include <vector>
using namespace std;

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

public:
	MBNode& AddNode(const vec3d& r);
	MBFace& AddFace(int n0, int n1, int n2, int n3);

	// update the Multii-Block data
	void UpdateMQ();

	void SetFaceEdgeIDs(int nface, int n0, int n1, int n2, int n3);

	MBNode& GetMBNode(int i) { return m_MBNode[i]; }
	MBEdge& GetFaceEdge(int nface, int nedge);

protected:
	void FindFaceNeighbours();
	void BuildMBEdges();

	// build the mesh items
	void BuildNodes(FEMesh* pm);
	void BuildFaces(FEMesh* pm);
	void BuildEdges(FEMesh* pm);

	void BuildNodeFaceTable(vector< vector<int> >& NFT);
	int FindEdgeIndex(MBFace& F, int n1, int n2);
	int FindEdge(int n1, int n2);

	int GetFaceNodeIndex(MBFace& f, int i, int j);
	int GetEdgeNodeIndex(MBEdge& e, int i);

	int GetFaceEdgeNodeIndex(MBFace& f, int ne, int i);

public:
	void SetShapeModifier(MBFace& f, FEShapeModifier* mod);
	void SetShapeModifier(MBEdge& e, FEShapeModifier* mod);
	void SetShapeModifier(MBNode& n, FEShapeModifier* mod);

protected:
	void ApplyMeshModifiers(FEMesh* pm);

	int GetFENode(MBNode& node);
	vector<int> GetFENodeList(MBEdge& node);
	vector<int> GetFENodeList(MBFace& node);

protected:
	vector<MBFace>	m_MBFace;
	vector<MBEdge>	m_MBEdge;
	vector<MBNode>	m_MBNode;
};

