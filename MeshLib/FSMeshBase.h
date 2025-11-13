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
#include <vector>
#include <set>
#include <FSCore/box.h>
#include "FSNode.h"
#include "FSEdge.h"
#include "FSFace.h"
#include "FSLineMesh.h"
#include "FSNodeFaceList.h"

//-------------------------------------------------------------------
//! Base class for mesh classes.
//! Essentially manages the nodes, edges, and faces
class FSMeshBase : public FSLineMesh
{
public:
	//! Default constructor
	FSMeshBase();
	//! Virtual destructor
	virtual ~FSMeshBase();

	//! Get the bounding box of the mesh
	BOX GetBoundingBox() const { return m_box; }

	//! Update the mesh structure (override from FSLineMesh)
	void UpdateMesh() override;

public:
	//! Get the local positions of a face
	void FaceNodeLocalPositions(const FSFace& f, vec3d* r) const;

public:
	//! Update the normals
	void UpdateNormals();

	//! Update item visibility
	virtual void UpdateItemVisibility() {}

	//! Calculate the center of a face
	vec3d FaceCenter(FSFace& f) const;

	//! Calculate the area of a face
	double FaceArea(FSFace& f);
	//! Calculate the area of a face given vertices and face type
	double FaceArea(const std::vector<vec3d>& f, int faceType);

	//! Get the world positions of face nodes
	void FaceNodePosition(const FSFace& f, vec3d* r) const;
	//! Get the normals of face nodes
	void FaceNodeNormals(const FSFace& f, vec3f* n) const;
	//! Get the texture coordinates of face nodes
	void FaceNodeTexCoords(const FSFace& f, float* t) const;

	//! Clear the selection of all faces
	void ClearFaceSelection();

	//! Get node neighbors within specified levels
	void GetNodeNeighbors(int inode, int levels, std::set<int>& nl1);

public: // interface for accessing mesh items
	//! Get the number of faces
	int Faces() const { return (int)m_Face.size(); }
	//! Get a reference to a face by index
	FSFace& Face(int n) { return m_Face[n]; }
	//! Get a const reference to a face by index
	const FSFace& Face(int n) const { return m_Face[n]; }
	//! Get a pointer to a face by index
	FSFace* FacePtr(int n = 0) { return ((n >= 0) && (n<(int)m_Face.size()) ? &m_Face[n] : 0); }
	//! Get a const pointer to a face by index
	const FSFace* FacePtr(int n = 0) const { return ((n >= 0) && (n<(int)m_Face.size()) ? &m_Face[n] : 0); }

	//! Delete all faces
	void DeleteFaces() { if (!m_Face.empty()) m_Face.clear(); m_NFL.Clear(); }
	//! Delete all edges
	void DeleteEdges() { if (!m_Edge.empty()) m_Edge.clear(); m_NFL.Clear(); }
	//! Delete all nodes
	void DeleteNodes() { if (!m_Node.empty()) m_Node.clear(); m_NFL.Clear(); }

public:
	//! Tag all faces with a specific tag value
	void TagAllFaces(int ntag);

	//! Count the number of selected nodes
	int CountSelectedNodes() const;
	//! Count the number of selected edges
	int CountSelectedEdges() const;
	//! Count the number of selected faces
	int CountSelectedFaces() const;

	//! Check if two nodes form an edge
	bool IsEdge(int n0, int n1);

	//! Find an edge if it exists (or null otherwise)
	FSEdge* FindEdge(int n0, int n1);

	//! Check if an edge is a crease edge
	bool IsCreaseEdge(int n0, int n1);

	//! Get the node-face list
	FSNodeFaceList& NodeFaceList();

protected:
	//! Remove edges with specified tag
	void RemoveEdges(int ntag);
	//! Remove faces with specified tag
	void RemoveFaces(int ntag);

protected:
	//! Vector of FE faces
	std::vector<FSFace>		m_Face;

	//! Node-face list for efficient lookups
	FSNodeFaceList		m_NFL;
};

//-------------------------------------------------------------------
//! Namespace containing mesh utility functions
namespace MeshTools {
	//! Get a list of face indices, connected to a face 
	//! INPUT:
	//! pm               : the mesh
	//! nface            : the index of the start face
	//! tolAngleDeg      : angle of selection tolerance (degrees). Set to zero to turn off.
	//! respectPartitions: do not cross surface partitions if true
	std::vector<int> GetConnectedFaces(FSMeshBase* pm, int nface, double tolAngleDeg, bool respectPartitions);

	//! Get connected faces by path between start and end face
	std::vector<int> GetConnectedFacesByPath(FSMeshBase* pm, int startFace, int endFace);

	//! Get connected nodes starting from a node
	std::vector<int> GetConnectedNodes(FSMeshBase* pm, int startNode, double tolAngleDeg, bool bmax, bool respectPartitions);

	//! Get connected nodes by path between start and end node
	std::vector<int> GetConnectedNodesByPath(FSMeshBase* pm, int startNode, int endNode);

	//! Get connected edges by path between start and end edge
	std::vector<int> GetConnectedEdgesByPath(FSMeshBase* pm, int startEdge, int endEdge);

	//! Get connected edges starting from an edge
	std::vector<int> GetConnectedEdges(FSMeshBase* pm, int startEdge, double tolAngleDeg, bool bmax);

	//! Tag connected nodes starting from a node
	void TagConnectedNodes(FSMeshBase* pm, int node, double tolAngleDeg, bool bmax, bool respectPartitions, int tag);

	//! Tag nodes by shortest path between two nodes
	void TagNodesByShortestPath(FSMeshBase* pm, int n0, int n1, int tag);
}