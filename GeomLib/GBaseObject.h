#pragma once
#include "GItem.h"
#include <MathLib/Transform.h>

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

	// --- C O N S T R U C T I O N ---

	// add a face to the object
	int AddNode(GNode* n);
	int AddNode(vec3d r, int n = NODE_VERTEX, bool bdup = false);
	int AddEdge(GEdge* e);
	int AddLine(int n1, int n2);
	int AddYArc(int n1, int n2);
	int AddZArc(int n1, int n2);
	int AddCircularArc(int n1, int n2, int n3);
	int AddArcSection(int n1, int n2, int n3);
	void AddFacet(const vector<int>& node, const vector<pair<int, int> >& edge, int ntype);
	void AddFacet(const vector<int>& edge, int ntype);
	void AddSurface(GFace* f);
	void AddPart ();
	void AddFace(GFace* f);

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

public: // transformation
	Transform& GetTransform() { return m_transform; }
	const Transform& GetTransform() const { return m_transform; }

	// copy transform info
	void CopyTransform(GBaseObject* po);

protected:
	Transform	m_transform;		// The object's transform

protected:
	// --- definition of geometry ---
	vector<GPart*>		m_Part;	//!< parts
	vector<GFace*>		m_Face;	//!< surfaces
	vector<GEdge*>		m_Edge;	//!< edges
	vector<GNode*>		m_Node;	//!< nodes
};
