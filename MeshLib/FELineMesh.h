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
#include "FENode.h"
#include "FEEdge.h"
#include <FSCore/box.h>
#include <vector>

class GObject;

// class that manages a list of nodes and edges
// This serves as a base class for most meshes
class FSLineMesh
{
public:
	FSLineMesh();

	// Should be called when mesh needs to be updated (but not reconstructred)
	// E.g. for surface meshes, this will update face normals, etc.
	virtual void UpdateMesh() = 0;

	// Should be called when mesh needs to be reconstructed
	virtual void BuildMesh() = 0;

	bool IsEditable() const;

public: // node interface

	// access node data
	int Nodes() const { return (int) m_Node.size(); }
	FSNode& Node(int i) { return m_Node[i]; }
	const FSNode& Node(int i) const { return m_Node[i]; }
	FSNode* NodePtr(int n = 0) { return ((n >= 0) && (n<(int)m_Node.size()) ? &m_Node[n] : 0);; };
	const FSNode* NodePtr(int n = 0) const { return ((n >= 0) && (n<(int)m_Node.size()) ? &m_Node[n] : 0); }

	void TagAllNodes(int ntag);

public: // edge interface

	// access edge data
	int Edges() const { return (int) m_Edge.size(); }
	FSEdge& Edge(int i) { return m_Edge[i]; }
	const FSEdge& Edge(int i) const { return m_Edge[i]; }
	FSEdge* EdgePtr(int n = 0) { return ((n >= 0) && (n<(int)m_Edge.size()) ? &m_Edge[n] : 0); }
	const FSEdge* EdgePtr(int n = 0) const { return ((n >= 0) && (n<(int)m_Edge.size()) ? &m_Edge[n] : 0); }

	void TagAllEdges(int ntag);

public:
	// set/get parent object
	void SetGObject(GObject* po);
	GObject* GetGObject();
	const GObject* GetGObject() const;

	// convert a local point to global coordinates
	// (This uses the parent object's transform)
	vec3d LocalToGlobal(const vec3d& r) const;

	vec3d GlobalToLocal(const vec3d& r) const;

	// get the global node position
	vec3d NodePosition(int i) const;

	// get the local node position
	vec3d NodeLocalPosition(int i) const;

	// get the edge center
	vec3d EdgeCenter(FSEdge& e) const;

	// update the bounding box
	void UpdateBoundingBox();

protected:
	GObject*	m_pobj;		//!< owning object
	BOX			m_box;		//!< bounding box

	std::vector<FSNode>	m_Node;		//!< Node list
	std::vector<FSEdge>	m_Edge;		//!< Edge list
};
