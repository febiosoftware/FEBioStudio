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
#include "FSNode.h"
#include "FSEdge.h"
#include <FSCore/box.h>
#include "FSNodeEdgeList.h"
#include <vector>

class GObject;

//! Class that manages a list of nodes and edges
//! This serves as a base class for most meshes
class FSLineMesh
{
public:
	//! Default constructor
	FSLineMesh();

	//! Should be called when mesh needs to be updated (but not reconstructred)
	//! E.g. for surface meshes, this will update face normals, etc.
	virtual void UpdateMesh() = 0;

	//! Should be called when mesh needs to be reconstructed
	virtual void BuildMesh() = 0;

	//! Check if the mesh is editable
	bool IsEditable() const;

public: // node interface

	//! Get the number of nodes in the mesh
	int Nodes() const { return (int) m_Node.size(); }
	//! Get a reference to a node by index
	FSNode& Node(int i) { return m_Node[i]; }
	//! Get a const reference to a node by index
	const FSNode& Node(int i) const { return m_Node[i]; }
	//! Get a pointer to a node by index
	FSNode* NodePtr(int n = 0) { return ((n >= 0) && (n<(int)m_Node.size()) ? &m_Node[n] : 0);; };
	//! Get a const pointer to a node by index
	const FSNode* NodePtr(int n = 0) const { return ((n >= 0) && (n<(int)m_Node.size()) ? &m_Node[n] : 0); }

	//! Tag all nodes with the specified tag value
	void TagAllNodes(int ntag);

public: // edge interface

	//! Get the number of edges in the mesh
	int Edges() const { return (int) m_Edge.size(); }
	//! Get a reference to an edge by index
	FSEdge& Edge(int i) { return m_Edge[i]; }
	//! Get a const reference to an edge by index
	const FSEdge& Edge(int i) const { return m_Edge[i]; }
	//! Get a pointer to an edge by index
	FSEdge* EdgePtr(int n = 0) { return ((n >= 0) && (n<(int)m_Edge.size()) ? &m_Edge[n] : 0); }
	//! Get a const pointer to an edge by index
	const FSEdge* EdgePtr(int n = 0) const { return ((n >= 0) && (n<(int)m_Edge.size()) ? &m_Edge[n] : 0); }

	//! Tag all edges with the specified tag value
	void TagAllEdges(int ntag);

public:
	//! Set the parent object
	void SetGObject(GObject* po);
	//! Get the parent object
	GObject* GetGObject();
	//! Get the parent object (const version)
	const GObject* GetGObject() const;

	//! Convert a local point to global coordinates
	//! (This uses the parent object's transform)
	vec3d LocalToGlobal(const vec3d& r) const;

	//! Convert a global point to local coordinates
	vec3d GlobalToLocal(const vec3d& r) const;

	//! Get the global node position
	vec3d NodePosition(int i) const;

	//! Get the local node position
	vec3d NodeLocalPosition(int i) const;

	//! Get the edge center
	vec3d EdgeCenter(FSEdge& e) const;

	//! Update the bounding box
	void UpdateBoundingBox();

	//! Get the node-edge list for a specific node
	const std::vector<NodeEdgeRef>& NodeEdgeList(int node) const;

protected:
	//! Owning object
	GObject*	m_pobj;
	//! Bounding box
	BOX			m_box;

	//! Node list
	std::vector<FSNode>	m_Node;
	//! Edge list
	std::vector<FSEdge>	m_Edge;
	//! Node-edge connectivity list
	FSNodeEdgeList		m_NLL;
};

namespace MeshTools {
	//! Get connected edges on a line mesh starting from a given edge
	std::vector<int> GetConnectedEdgesOnLineMesh(FSLineMesh* pm, int startEdge, double angDeg, bool bmax);
}