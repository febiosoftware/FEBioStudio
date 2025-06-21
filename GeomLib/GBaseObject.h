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
#include "GItem.h"
#include <FECore/FETransform.h>

//-----------------------------------------------------------------------------
//! This is a base class for GObject. I hope to describe all geometry in terms 
//! of this base class instead of GObject
class GBaseObject : public GItem_T<GBaseObject>
{
public:
	//! Default constructor
	GBaseObject();
	//! Virtual destructor
	virtual ~GBaseObject();

	//! Copy constructor from another GBaseObject
	virtual void Copy(const GBaseObject* po);

	// --- G E O M E T R Y ---

	//! Return pointer to a part using local ID
	GPart*	Part(int i) { return (i>=0 && i<(int)m_Part.size() ? m_Part[i] : 0); }
	//! Return pointer to a face using local ID
	GFace*	Face(int i) { return (i>=0 && i<(int)m_Face.size() ? m_Face[i] : 0); }
	//! Return pointer to an edge using local ID
	GEdge*	Edge(int i) { return (i>=0 && i<(int)m_Edge.size() ? m_Edge[i] : 0); }
	//! Return pointer to a node using local ID
	GNode*	Node(int i) { return (i>=0 && i<(int)m_Node.size() ? m_Node[i] : 0); }

	//! Return const pointer to a part using local ID
	const GPart* Part(int i) const { return (i >= 0 && i<(int)m_Part.size() ? m_Part[i] : 0); }
	//! Return const pointer to a face using local ID
	const GFace* Face(int i) const { return (i >= 0 && i<(int)m_Face.size() ? m_Face[i] : 0); }
	//! Return const pointer to an edge using local ID
	const GEdge* Edge(int i) const { return (i >= 0 && i<(int)m_Edge.size() ? m_Edge[i] : 0); }
	//! Return const pointer to a node using local ID
	const GNode* Node(int i) const { return (i >= 0 && i<(int)m_Node.size() ? m_Node[i] : 0); }

	//! Return number of parts
	int Parts() const { return (int)m_Part.size(); }
	//! Return number of faces
	int Faces() const { return (int)m_Face.size(); }
	//! Return number of edges
	int Edges() const { return (int)m_Edge.size(); }
	//! Return number of nodes
	int Nodes() const { return (int)m_Node.size(); }

	//! Return pointer to a part using global ID
	GPart* FindPart(int nid);
	//! Return pointer to a face using global ID
	GFace* FindFace(int nid);
	//! Return pointer to an edge using global ID
	GEdge* FindEdge(int nid);
	//! Return pointer to a node using global ID
	GNode* FindNode(int nid);

	//! Find a part by name
	GPart* FindPartFromName(const char* szname);

	// --- C O N S T R U C T I O N ---

	//! Add a node to the object
	int AddNode(GNode* n);
	//! Add a node at specified position with given type
	GNode* AddNode(vec3d r, int n = NODE_VERTEX, bool bdup = false);
	//! Add an edge to the object
	GEdge* AddEdge();
	//! Add an edge to the object
	int AddEdge(GEdge* e);
	//! Add a line between two nodes
	int AddLine(int n1, int n2);
	//! Add a Y-axis arc between two nodes
	int AddYArc(int n1, int n2);
	//! Add a Z-axis arc between two nodes
	int AddZArc(int n1, int n2);
	//! Add a circular arc through three nodes
	int AddCircularArc(int n1, int n2, int n3);
	//! Add an arc section through three nodes
	int AddArcSection(int n1, int n2, int n3);
	//! Add a Bezier section using control points
	int AddBezierSection(const std::vector<int>& n);
	//! Add a facet with specified nodes and edges
	void AddFacet(const std::vector<int>& node, const std::vector<std::pair<int, int> >& edge, int ntype);
	//! Add a facet with specified edges
	void AddFacet(const std::vector<int>& edge, int ntype);
	//! Add a surface to the object
	void AddSurface(GFace* f);
	//! Add a part to the object
	GPart* AddPart ();

	//! Add a solid part to the object
	GPart* AddSolidPart();
	//! Add a shell part to the object
	GPart* AddShellPart();
	//! Add a beam part to the object
	GPart* AddBeamPart();

	//! Add a face to the object
	void AddFace(GFace* f);
	//! Add a face to the object
	GFace* AddFace();

	//! Delete a part from the object
	virtual bool DeletePart(GPart* pg);

	//! Update the node types
	void UpdateNodeTypes();

	//! Clear all geometry data
	void ClearAll();
	//! Clear all faces
	void ClearFaces();
	//! Clear all edges
	void ClearEdges();
	//! Clear all nodes
	void ClearNodes();
	//! Clear all parts
	void ClearParts();

	//! Resize the parts array
	void ResizeParts(int n);
	//! Resize the surfaces array
	void ResizeSurfaces(int n);
	//! Resize the curves array
	void ResizeCurves(int n);
	//! Resize the nodes array
	void ResizeNodes(int n);

public:
	//! Remove a node by index
	void RemoveNode(int n);

public: // transformation
	//! Get the object's transform
	Transform& GetTransform() { return m_transform; }
	//! Get the object's transform (const version)
	const Transform& GetTransform() const { return m_transform; }

	//! Set the object's position
	void SetPosition(const vec3d& position);
	//! Get the object's position
	vec3d GetPosition() const;

	//! Copy transform info from another object
	void CopyTransform(GBaseObject* po);

	//! Get the object's render transform
	Transform& GetRenderTransform() { return m_renderTransform; }
	//! Get the object's render transform (const version)
	const Transform& GetRenderTransform() const { return m_renderTransform; }
	//! Set the object's render transform
	void SetRenderTransform(const Transform& T) { m_renderTransform = T; }

protected:
	//! The object's transform
	Transform	m_transform;
	//! The objects' transform for rendering
	Transform	m_renderTransform;

protected:
	// --- definition of geometry ---
	//! Parts vector
	std::vector<GPart*>		m_Part;
	//! Surfaces vector
	std::vector<GFace*>		m_Face;
	//! Edges vector
	std::vector<GEdge*>		m_Edge;
	//! Nodes vector
	std::vector<GNode*>		m_Node;
};