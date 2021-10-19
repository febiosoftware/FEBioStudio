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
#include <MathLib/math3d.h>
#include <list>
//using namespace std;

using std::list;
using std::pair;

// class that represents a triangle meshes and uses linked lists as the item containers.
// This mesh can be used for algorithms that manipulate triangle meshes
class TriMesh
{
public:
	// forward declaration of node, edge, and face classes
	class NODE;
	struct EDGE;
	struct FACE;

	// "pointers" to the items
	typedef list<NODE>::iterator NODEP;
	typedef list<EDGE>::iterator EDGEP;
	typedef list<FACE>::iterator FACEP;

	// class representing a vertex
	// only stores the vertex coordinates
	class NODE
	{
	public:
		vec3d	r;		// spatial position
		int		ntag;	// user tag
		int		gid;	// group ID
		int		eval;	// edge valences = number of edges that connect to this node (0 = isolated vertex)

	public:
		NODE() : ntag(0), gid(-1), eval(0) {}
		explicit NODE(const vec3d& v) : r(v), ntag(0), gid(-1), eval(0) {}
	};

	// An edge is defined by two nodes.
	// it also stores the two facets that share this edge
	struct EDGE
	{
		NODEP	node[2];		// pointer to nodes
		FACEP	face[2];		// pointer to faces
		int		nedge[2];		// local edge index into faces (-1 if face is not set)
		int		ntag;			// user tag
		int		gid;			// group ID

		EDGE();
		void addFace(FACEP f, int lid);
		void removeFace(FACEP p);
	};

	// A face is defined by three nodes. 
	// The edges that make up the face are also stored as well as the edge windings.
	// This class also stores a face normal
	struct FACE
	{
		NODEP	node[3];	// pointer to nodes
		EDGEP	edge[3];	// pointer to edges
		int		w[3];		// winding flags
		vec3d	normal;		// face normal
		int		ntag;		// user tag
		int		gid;
		FACE();
	};

	// class for looping over all nodes of the mesh
	class NodeIterator
	{
	public:
		NodeIterator(TriMesh& mesh) : m_mesh(mesh)
		{
			m_node = mesh.m_Node.begin();
		}

		void operator ++ () { m_node++; }


		NODEP operator -> () { return m_node; }

		bool isValid() { return (m_node != m_mesh.m_Node.end()); }

		void reset() { m_node = m_mesh.m_Node.begin(); }

	private:
		TriMesh&	m_mesh;
		NODEP			m_node;
	};
	friend class NodeIterator;

	// class for looping over all edges
	class EdgeIterator
	{
	public:
		EdgeIterator(TriMesh& mesh) : m_mesh(mesh)
		{
			m_edge = mesh.m_Edge.begin();
		}

		void operator ++ () { m_edge++; }

		void operator = (EDGEP e) { m_edge = e; }
		EDGEP operator -> () { return m_edge; }
		operator EDGEP() { return m_edge; }

		bool isValid() { return (m_edge != m_mesh.m_Edge.end()); }

		void reset() { m_edge = m_mesh.m_Edge.begin(); }

	private:
		TriMesh&	m_mesh;
		EDGEP			m_edge;
	};
	friend class EdgeIterator;

	// class for looping over all faces
	class FaceIterator
	{
	public:
		FaceIterator(TriMesh& mesh) : m_mesh(mesh)
		{
			m_face = mesh.m_Face.begin();
		}

		void operator ++ () { m_face++; }

		FACEP operator * () { return m_face; }
		FACEP operator -> () { return m_face; }

		bool isValid() { return (m_face != m_mesh.m_Face.end()); }

		void reset() { m_face = m_mesh.m_Face.begin(); }

	private:
		TriMesh&	m_mesh;
		FACEP			m_face;
	};
	friend class FaceIterator;

	NODEP opposingNode(EDGEP e, int nface);

	EDGEP nextEdge(EDGEP e, int nface);

	EDGEP prevEdge(EDGEP e, int nface);

	EDGEP opposingEdge(FACEP f, NODEP n);

	void tagAllEdges(int ntag = 0);
	void tagAllFaces(int ntag = 0);

	FACEP faceNeighbor(FACEP f, int i);

	int edgeWinding(FACEP f, EDGEP e);

//private:
public:
	list<NODE>	m_Node;	// list of nodes
	list<EDGE>	m_Edge;	// list of edges
	list<FACE>	m_Face;	// list of faces

public:
	// constructor
	TriMesh(); 

	int Nodes() const { return (int)m_Node.size(); }
	int Edges() const { return (int)m_Edge.size(); }
	int Faces() const { return (int)m_Face.size(); }

public: // add items to mesh

	// add a node 
	// This node will not be attached to any edge of face
	NODEP addNode(const vec3d& r, int ntag = 0);

	// Add a new edge
	// This edge will not be connected to a face
	EDGEP addEdge(NODEP n0, NODEP n1, int ntag = 0);

	// Add a face
	// It is assumed that the nodes and edges are defined for the face
	FACEP addFace(const FACE& f);

public: // remove items from mesh

	// Remove a face
	void removeFace(FACEP f, bool removeEdges = false);

	// Remove an edge
	// This will also remove the shared faces
	void removeEdge(EDGEP p);

public: // mesh operations

	// split a face at a node
	void splitFace(FACEP f, NODEP p);

	// split an edge at a node
	void splitEdge(EDGEP e, NODEP p);

	// flip an edge
	// returns false if the edge cannot be flipped (e.g. it's an outside edge)
	bool flipEdge(EDGEP e);

	// find an edge
	EDGEP findEdge(NODEP n0, NODEP n1);

public:
	void PartitionSurface();

//private:
public:
	// helper function for updating face data after inserting new face
	void updateFace(FACEP fp);
};

//-----------------------------------------------------------------------------
// used by some of the algorihtms below
typedef pair<TriMesh::NODEP, TriMesh::NODEP> NodePair;

//-----------------------------------------------------------------------------
// See if point p projects inside the face
// returns:
//    -1: point does not lie in face
// 0,1,2: point coincide with node i (within tolerance)
// 3,4,5: point falls on edge (i-3)
//     6: point lies in face
int projectToFace(TriMesh::FACE& f, const vec3d& p, vec3d& q, const double eps = 1e-2);

//-----------------------------------------------------------------------------
// project r on edge.
// returns: q, projection onto edge
//          return value is edge length from edge point 0
double projectToEdge(TriMesh::EDGE& edge, const vec3d& r, vec3d& q);

//-----------------------------------------------------------------------------
// See if two edges intersect in plane (n0, N)
// returns:
// 0 = no intersection
// 1 = intersects at node n0
// 2 = intersects at node n1
// 3 = proper intersection
// 4 = edges coincide at points
// 5 = edges are identical
int edgeIntersect(TriMesh::NODEP n0, TriMesh::NODEP n1, TriMesh::NODEP n2, TriMesh::NODEP n3, vec3d N, vec3d& q, double& L, const double eps = 1e-2);

//-----------------------------------------------------------------------------
// Insert a point in the mesh.
// If the point r coincides with a node of the mesh, no new node is inserted and the corresponding node is returned.
// If the point r falls on an edge, the edge is split and a new node is inserted. 
// If the point r falls inside a face, the face is split and a new node is inserted.
TriMesh::NODEP insertPoint(TriMesh& mesh, const vec3d& r, const double eps = 1e-2);

//-----------------------------------------------------------------------------
// Class that describes the "star" of a node, i.e. the edges and faces that connect to this node.
class TriStar
{
public:
	TriStar(){}
	TriStar(const TriStar& star)
	{
		m_node = star.m_node;
		m_edge = star.m_edge;
		m_face = star.m_face;
	}

public:
	TriMesh::NODEP				m_node;
	std::vector<TriMesh::EDGEP>	m_edge;
	std::vector<TriMesh::FACEP> m_face;
};


//-----------------------------------------------------------------------------
// Inserts a point and remeshes the neighborhood if necessary to retain a quality mesh.
// R is the value of the local size field. If zero, a value will be calculated based on local element size.

TriMesh::NODEP insertDelaunyPoint(TriMesh& mesh, const vec3d& r, bool remesh = true, const double eps = 1e-12);

TriMesh::EDGEP insertDelaunyEdge(TriMesh& mesh, TriMesh::NODEP n0, TriMesh::NODEP n1);

//-----------------------------------------------------------------------------
void insertEdge(TriMesh& mesh, TriMesh::NODEP n0, TriMesh::NODEP n1, vector<TriMesh::NODEP>& nodeList, int tag, const double eps = 1e-2);
