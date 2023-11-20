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
// This class implements a modifier that fills a hole in a mesh. 
class FEFillHole : public FESurfaceModifier
{
public:
	// helper class for defining a closed loop of edges
	class EdgeRing
	{
	public:
		EdgeRing(){ m_winding = 1; }
		EdgeRing(const EdgeRing& ring) { m_node = ring.m_node; m_r = ring.m_r; m_winding = ring.m_winding; m_normal = ring.m_normal;}
		void operator = (const EdgeRing& ring) { m_node = ring.m_node; m_r = ring.m_r; m_winding = ring.m_winding;  m_normal = ring.m_normal;}

		bool empty() { return m_node.empty(); }

		void add(int n, const vec3d& r, const vec3d& nn) { m_node.push_back(n); m_r.push_back(r); m_normal.push_back(nn);}

		void clear() { m_node.clear(); m_r.clear(); m_normal.clear();}

		int size() { return (int) m_node.size(); }

		int operator [] (int i) { return m_node[i]; }

		void GetLeftEar(int n0, int n1, EdgeRing& ear);
		void GetRightEar(int n0, int n1, EdgeRing & ear);

		bool contains(int inode);

	public:
		std::vector<vec3d>	m_r;		// nodal positions
		std::vector<int>	m_node;		// sorted list of nodes defining the closed loop
		int					m_winding;	// +1 or -1 depending on the winding
		std::vector<vec3d>	m_normal;	//node normals
	};

	// helper class for representing a new face
	struct FACE
	{
		vec3d	r[3];	// nodal positions
		int		n[3];	// the three nodes of the face
	};

public:
	FEFillHole();

	FSSurfaceMesh* Apply(FSSurfaceMesh* pm);

	//AFM
	bool AFM(FSSurfaceMesh& mesh, EdgeRing& ring, std::vector<FACE>& tri_list, std::vector<vec3d> &node_list);
	//Addd node
	vec3d newNode(vec3d current_node, vec3d next_node, vec3d prev_node,vec3d node_normal, double scale, bool concave);
	// divide a ring
	bool DivideRing(EdgeRing& ring, std::vector<FACE>& tri_list);

	// divide a ring
	bool DivideRing1(EdgeRing& ring, std::vector<FACE>& tri_list);
	bool DivideRing2(EdgeRing& ring, std::vector<FACE>& tri_list);

	// fill all holes
	void FillAllHoles(FSSurfaceMesh* pm);


private:
	// Find the ring based on a node (TODO: should I move this to the FSMesh class?)
	bool FindEdgeRing(FSSurfaceMesh& mesh, int node, EdgeRing& ring);
	
	// Find the approximate normal of a ring
	vec3d RingNormal(EdgeRing& ring);

	// check if the split is valid
	bool IsValidSplit(EdgeRing& left, EdgeRing& right, const vec3d& p, const vec3d& t);

	// get location sign (+1 on positive side, -1 on negative side, 0 = on both sides)
	int GetPlaneOrientation(EdgeRing& ring, const vec3d& p, const vec3d& t);

	// get the area of the smallest triangle
	double min_tri_area(std::vector<FACE>& tri);

	// get the quality of the worst triangle
	double min_tri_quality(std::vector<FACE>& tri);

private:
	std::vector<vec3d>	m_node_normals;
	FSNodeEdgeList		m_NEL;

	bool m_optimize;
	bool m_insertNodes;
};
