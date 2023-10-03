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
#include "FESurfaceModifier.h"
#include <MeshLib/FENodeEdgeList.h>

//-----------------------------------------------------------------------------
// This class implements a modifier that fills a hole in a quad mesh. 
class FEFillQuadHole : public FESurfaceModifier
{
public:
	// helper class for defining a closed loop of edges
	class EdgeRing
	{
	public:
		EdgeRing() { m_winding = 1; }
		EdgeRing(const EdgeRing& ring) { m_node = ring.m_node; m_r = ring.m_r; m_winding = ring.m_winding; m_normal = ring.m_normal; }
		void operator = (const EdgeRing& ring) { m_node = ring.m_node; m_r = ring.m_r; m_winding = ring.m_winding;  m_normal = ring.m_normal; }

		bool empty() { return m_node.empty(); }

		void add(int n, const vec3d& r, const vec3d& nn) { m_node.push_back(n); m_r.push_back(r); m_normal.push_back(nn); }

		void clear() { m_node.clear(); m_r.clear(); m_normal.clear(); }

		int size() { return (int)m_node.size(); }

		int operator [] (int i) { return m_node[i]; }

		void GetLeftEar(int n0, int n1, EdgeRing& ear);
		void GetRightEar(int n0, int n1, EdgeRing& ear);

		bool contains(int inode);

	public:
		std::vector<vec3d>	m_r;		// nodal positions
		std::vector<int>	m_node;		// sorted list of nodes defining the closed loop
		int					m_winding;	// +1 or -1 depending on the winding
		std::vector<vec3d>	m_normal;	//node normals
	};

	// helper class for representing a new face
	struct QUAD
	{
		vec3d	r[4];	// nodal positions
		int		n[4];	// the three nodes of the face
	};

public:
	FEFillQuadHole();

	FSSurfaceMesh* Apply(FSSurfaceMesh* pm);

	// fill all holes
	void FillAllHoles(FSSurfaceMesh* pm);

private:
	// build the nodal normals
	void BuildNodalNormals(FSSurfaceMesh& mesh);

	// Find the ring based on a node (TODO: should I move this to the FSMesh class?)
	bool FindEdgeRing(FSSurfaceMesh& mesh, int node, EdgeRing& ring);

	// divide a ring
	bool DivideRing(EdgeRing& ring, std::vector<QUAD>& tri_list);

	void UpdateMesh(FSSurfaceMesh& mesh, const std::vector<QUAD>& quad_list);

	vec3d edgeVector(FSEdge& e, FSSurfaceMesh& mesh);

private:
	std::vector<vec3d>	m_node_normals;
	FSNodeEdgeList		m_NEL;
};
