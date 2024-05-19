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
		vec3f	r;		// nodal position
		vec3f	n;		// normal (but not really)
		int		tag = 0;	// multipurpose tag
		int		pid = 0;	// GNode parent local ID
		int		nid = 0;	// Node index of FSNode (in case a mesh object created this GMesh)
	};

	struct EDGE
	{
		int		pid = 0;	// GEdge parent local id
		int		n[2];	// nodes
		vec3f	vr[2];	// nodal coordinates
	};

	struct FACE
	{
		int		pid = 0;	// GFace parent local id
		int		fid = 0;	// face ID of FSace in parent mesh (or -1 if not applicable)
		int		eid = 0;	// element ID (used by planecut algorithm)
		int		sid = 0;	// smoothing group ID
		bool	bext = true;	// external flag
		int		tag = 0;	// multipurpose tag
		int		n[3];	// nodes
		int		nbr[3];	// neighbor faces
		vec3f	fn;		// face normal
		vec3f	vn[3];	// node normals
		vec3f	vr[3];	// nodal coordinates
		GLColor	c[3];	// node colors
		float	t[3];	// texture coordinates
	};

public:
	GMesh(void);
	virtual ~GMesh(void);

	void Create(int nodes, int faces, int edges = 0);
	void Clear();

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

	BOX GetBoundingBox() { return m_box; }

	void Attach(GMesh& m, bool bupdate = true);

	void AutoSmooth(double angleDegrees);

public:
	int	AddNode(const vec3f& r, int groupID = 0);
	int	AddNode(const vec3f& r, int nodeID, int groupID);
	void AddEdge(int* n, int nodes, int groupID = 0);
	void AddEdge(vec3f* r, int nodes, int groupID = 0);
	int AddFace(int n0, int n1, int n2, int groupID = 0, int smoothID = 0, bool bext = true, int faceId = -1, int elemId = -1);
	void AddFace(const int* n, int nodes, int gid = 0, int smoothID = 0, bool bext = true, int faceId = -1, int elemId = -1);
	void AddFace(vec3f* r, int gid = 0, int smoothId = 0, bool bext = true);
	void AddFace(vec3f r[3], vec3f n[3], GLColor c);

protected:
	void FindNeighbors();

public:
	void UpdateBoundingBox();
	void UpdateNormals();

protected:
	BOX				m_box;
	vector<NODE>	m_Node;
	vector<EDGE>	m_Edge;
	vector<FACE>	m_Face;

public:
	vector<pair<int, int> >	m_FIL;
	vector<pair<int, int> >	m_EIL;
};
