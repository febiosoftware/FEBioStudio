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
// This is a base class for GObject. I hope to describe all geometry in terms 
// of this base class instead of GObject
class GBaseObject : public GItem_T<GBaseObject>
{
public:
	GBaseObject();
	virtual ~GBaseObject();

	virtual void Copy(const GBaseObject* po);

	// --- G E O M E T R Y ---

	// return pointer to an item using local ID
	GPart*	Part(int i) { return (i>=0 && i<(int)m_Part.size() ? m_Part[i] : 0); }
	GFace*	Face(int i) { return (i>=0 && i<(int)m_Face.size() ? m_Face[i] : 0); }
	GEdge*	Edge(int i) { return (i>=0 && i<(int)m_Edge.size() ? m_Edge[i] : 0); }
	GNode*	Node(int i) { return (i>=0 && i<(int)m_Node.size() ? m_Node[i] : 0); }

	const GPart* Part(int i) const { return (i >= 0 && i<(int)m_Part.size() ? m_Part[i] : 0); }
	const GFace* Face(int i) const { return (i >= 0 && i<(int)m_Face.size() ? m_Face[i] : 0); }
	const GEdge* Edge(int i) const { return (i >= 0 && i<(int)m_Edge.size() ? m_Edge[i] : 0); }
	const GNode* Node(int i) const { return (i >= 0 && i<(int)m_Node.size() ? m_Node[i] : 0); }

	// return item counts
	int Parts() const { return (int)m_Part.size(); }
	int Faces() const { return (int)m_Face.size(); }
	int Edges() const { return (int)m_Edge.size(); }
	int Nodes() const { return (int)m_Node.size(); }

	// return pointer to an item using global ID
	GPart* FindPart(int nid);
	GFace* FindFace(int nid);
	GEdge* FindEdge(int nid);
	GNode* FindNode(int nid);

	GPart* FindPartFromName(const char* szname);

	// --- C O N S T R U C T I O N ---

	// add a face to the object
	int AddNode(GNode* n);
	GNode* AddNode(vec3d r, int n = NODE_VERTEX, bool bdup = false);
	GEdge* AddEdge();
	int AddEdge(GEdge* e);
	int AddLine(int n1, int n2);
	int AddYArc(int n1, int n2);
	int AddZArc(int n1, int n2);
	int AddCircularArc(int n1, int n2, int n3);
	int AddArcSection(int n1, int n2, int n3);
	int AddBezierSection(const std::vector<int>& n);
	void AddFacet(const std::vector<int>& node, const std::vector<pair<int, int> >& edge, int ntype);
	void AddFacet(const std::vector<int>& edge, int ntype);
	void AddSurface(GFace* f);
	GPart* AddPart ();

	GPart* AddSolidPart();
	GPart* AddShellPart();
	GPart* AddBeamPart();

	void AddFace(GFace* f);
	GFace* AddFace();

	virtual bool DeletePart(GPart* pg);

	// update the node types
	void UpdateNodeTypes();

	void ClearAll();
	void ClearFaces();
	void ClearEdges();
	void ClearNodes();
	void ClearParts();

	void ResizeParts(int n);
	void ResizeSurfaces(int n);
	void ResizeCurves(int n);
	void ResizeNodes(int n);

public:
	void RemoveNode(int n);

public: // transformation
	Transform& GetTransform() { return m_transform; }
	const Transform& GetTransform() const { return m_transform; }

	// copy transform info
	void CopyTransform(GBaseObject* po);

protected:
	Transform	m_transform;		// The object's transform

protected:
	// --- definition of geometry ---
	std::vector<GPart*>		m_Part;	//!< parts
	std::vector<GFace*>		m_Face;	//!< surfaces
	std::vector<GEdge*>		m_Edge;	//!< edges
	std::vector<GNode*>		m_Node;	//!< nodes
};
