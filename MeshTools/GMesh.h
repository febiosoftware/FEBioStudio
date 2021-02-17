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
#include <FSCore/box.h>
#include <FSCore/color.h>
#include <vector>
//using namespace std;

using std::vector;
using std::pair;

//-----------------------------------------------------------------------------
// The GMesh class defines a triangulated surface. GMesh classes are used to
// represent the surface of a geometry object. The pid members of the mesh
// item classes refer to the corresponding item in the parent object.
//
class GMesh
{
public:
	struct NODE
	{
		vec3d	r;		// nodal position
		vec3d	n;		// normal (but not really)
		int		tag;	// multipurpose tag
		int		pid;	// GNode parent local ID
		int		nid;	// Node index of FENode (in case a mesh object created this GMesh)
	};

	struct EDGE
	{
		int		n[2];	// nodes
		int		pid;	// GEdge parent local id
	};

	struct FACE
	{
		int		n[3];	// nodes
		int		nbr[3];	// neighbor faces
		int		pid;	// GFace parent local id
		int		sid;	// smoothing groupd ID
		int		tag;	// multipurpose tag
		vec3d	fn;		// face normal
		vec3d	nn[3];	// node normals
		GLColor	c[3];	// node colors
		bool	bext;	// external flag
	};

public:
	GMesh(void);
	virtual ~GMesh(void);

	void Create(int nodes, int faces, int edges = 0);

	virtual void Update();

	int Nodes() const { return (int) m_Node.size(); }
	int Edges() const { return (int)m_Edge.size(); }
	int Faces() const { return (int)m_Face.size(); }

	NODE& Node(int i) { return m_Node[i]; }
	EDGE& Edge(int i) { return m_Edge[i]; }
	FACE& Face(int i) { return m_Face[i]; }

	const NODE& Node(int i) const { return m_Node[i]; }
	const EDGE& Edge(int i) const { return m_Edge[i]; }
	const FACE& Face(int i) const { return m_Face[i]; }

	bool IsEmpty() const { return m_Node.empty(); }

	void UpdateNormals(int* pid, int nsize);
	void UpdateNormals();

	BOX GetBoundingBox() { return m_box; }
	void UpdateBoundingBox();

	void Attach(GMesh& m);

public:
	int	AddNode(const vec3d& r, int groupID = 0);
	int	AddNode(const vec3d& r, int nodeID, int groupID);
	void AddEdge(int* n, int nodes, int groupID = 0);
	int AddFace(int n0, int n1, int n2, int groupID = 0, int smoothID = 0, bool bext = true);
	void AddFace(int* n, int nodes, int gid = 0, int smoothID = 0, bool bext = true);
	void AddFace(vec3d* r, int gid = 0, int smoothId = 0, bool bext = true);

protected:
	void FindNeighbors();

protected:
	BOX				m_box;
	vector<NODE>	m_Node;
	vector<EDGE>	m_Edge;
	vector<FACE>	m_Face;

public:
	vector<pair<int, int> >	m_FIL;
	vector<pair<int, int> >	m_EIL;
};
