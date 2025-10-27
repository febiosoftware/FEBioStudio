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

using std::vector;
using std::pair;

// The GLMesh class defines a triangulated surface. GLMesh classes are used to
// represent the surface of a geometry object. The pid members of the mesh
// item classes refer to the corresponding item in the parent object.
//
class GLMesh
{
public:
	struct NODE
	{
		vec3f	r;		// nodal position
		vec3f	n;		// normal (but not really)
		GLColor c;
		int		tag = 0;	// multipurpose tag
		int		pid = 0;	// GNode parent local ID
		int		nid = 0;	// Node index of FSNode (in case a mesh object created this GLMesh)
	};

	struct EDGE
	{
		int		pid = 0;	// GEdge parent local id
		int		n[2];	// nodes
		vec3f	vr[2];	// nodal coordinates
		GLColor	c[2];	// node colors
	};

	struct FACE
	{
		int		pid = 0;	// GFace parent local id
		int		fid = 0;	// face ID of FSace in parent mesh (or -1 if not applicable)
		int		eid = 0;	// element ID (used by planecut algorithm)
		int		sid = 0;	// smoothing group ID
		int		mid = 0;	// material ID
		bool	bext = true;	// external flag
		int		tag = 0;	// multipurpose tag
		int		n[3];	// nodes
		int		nbr[3];	// neighbor faces
		vec3f	fn;		// face normal
		vec3f	vn[3];	// node normals
		vec3f	vr[3];	// nodal coordinates
		GLColor	c[3];	// node colors
		vec3f	t[3];	// texture coordinates
	};

	struct PARTITION
	{
		int n0 = 0; // start index into face list
		int nf = 0; // nr of faces in partition
		int tag = 0; // used to define partition attributes such as whether it's internal/external
	};

public:
	GLMesh(void);
	virtual ~GLMesh(void);

	void Create(int nodes, int faces, int edges = 0);
	void Clear();

	void NewPartition(int tag = 0);

	void AutoPartition();

	void Update(bool updateNormals = true);

	bool IsModified() const { return m_isModified; }

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

	void Attach(GLMesh& m, bool bupdate = true);

	void AutoSmooth(double angleDegrees);

public:
	size_t Partitions() const { return m_FIL.size(); }
	const PARTITION& Partition(size_t n) const { return m_FIL[n]; }

	size_t EILs() const { return m_EIL.size(); }
	const std::pair<int, int>& EIL(size_t n) const { return m_EIL[n]; }

public:
	int	AddNode(const vec3f& r, int groupID = 0);
	int	AddNode(const vec3f& r, int nodeID, int groupID);

	int	AddNode(const vec3f& r, GLColor c);

public:
	void AddEdge(int* n, int nodes, int groupID = 0);
	void AddEdge(vec3f* r, int nodes, int groupID = 0);

	void AddEdge(vec3f r[2], GLColor c);
	void AddEdge(vec3f r[2], GLColor c[2]);

	void AddEdge(const vec3f& a, const vec3f& b);

public:
	int AddFace(int n0, int n1, int n2, int groupID = 0, int smoothID = 0, bool bext = true, int faceId = -1, int elemId = -1, int mat = 0);
	void AddFace(const int* n, int nodes, int gid = 0, int smoothID = 0, bool bext = true, int faceId = -1, int elemId = -1, int mat = 0);
	void AddFace(vec3f* r, int gid = 0, int smoothId = 0, bool bext = true);
	
	void AddFace(vec3f r[3], GLColor c);
	void AddFace(vec3f r[3], vec3f n[3], GLColor c);
	void AddFace(vec3f r[3], vec3f n[3], float tex, GLColor c);
	void AddFace(vec3f r[3], vec3f n[3], float tex[3], GLColor c);
	void AddFace(vec3f r[3], GLColor c[3]);
	void AddFace(vec3f r[3], float t[3]);
	void AddFace(vec3f r[3], vec3f t[3]);
	void AddFace(vec3f r[3], float t[3], GLColor c[3]);

	int SetFaceTex(int f0, float* t, int n);

protected:
	void FindNeighbors();

public:
	void UpdateBoundingBox();
	void UpdateNormals();

	void setModified(bool b) { m_isModified = b; }

private:
	int AddFace(const FACE& face);

protected:
	BOX				m_box;
	vector<NODE>	m_Node;
	vector<EDGE>	m_Edge;
	vector<FACE>	m_Face;

private:
	vector<PARTITION>		m_FIL;
	vector<pair<int, int> >	m_EIL;

	bool m_isModified = false;
	bool m_hasNeighborList = false;
};
