#pragma once
#include "FENode.h"
#include "FEEdge.h"
#include <FSCore/box.h>

class GObject;

// class that manages a list of nodes and edges
// This serves as a base class for most meshes
class FELineMesh
{
public:
	FELineMesh();

	// Should be called when mesh needs to be updated (but not reconstructred)
	// E.g. for surface meshes, this will update face normals, etc.
	virtual void UpdateMesh() = 0;

	// Should be called when mesh needs to be reconstructed
	virtual void BuildMesh() = 0;

	// updates selection state (just call's owning object's UpdateSelection())
	virtual void UpdateSelection();

public: // node interface

	// access node data
	int Nodes() const { return (int) m_Node.size(); }
	FENode& Node(int i) { return m_Node[i]; }
	const FENode& Node(int i) const { return m_Node[i]; }
	FENode* NodePtr(int n = 0) { return ((n >= 0) && (n<(int)m_Node.size()) ? &m_Node[n] : 0);; };
	const FENode* NodePtr(int n = 0) const { return ((n >= 0) && (n<(int)m_Node.size()) ? &m_Node[n] : 0); }

	void TagAllNodes(int ntag);

public: // edge interface

	// access edge data
	int Edges() const { return (int) m_Edge.size(); }
	FEEdge& Edge(int i) { return m_Edge[i]; }
	const FEEdge& Edge(int i) const { return m_Edge[i]; }
	FEEdge* EdgePtr(int n = 0) { return ((n >= 0) && (n<(int)m_Edge.size()) ? &m_Edge[n] : 0); }
	const FEEdge* EdgePtr(int n = 0) const { return ((n >= 0) && (n<(int)m_Edge.size()) ? &m_Edge[n] : 0); }

	void TagAllEdges(int ntag);

public:
	// set/get parent object
	void SetGObject(GObject* po);
	GObject* GetGObject();

	// convert a local point to global coordinates
	// (This uses the parent object's transform)
	vec3d LocalToGlobal(const vec3d& r) const;

	vec3d GlobalToLocal(const vec3d& r) const;

	// get the global node position
	vec3d NodePosition(int i) const;

	// get the local node position
	vec3d NodeLocalPosition(int i) const;

	// update the bounding box
	void UpdateBoundingBox();

protected:
	GObject*	m_pobj;		//!< owning object
	BOX			m_box;		//!< bounding box

	vector<FENode>	m_Node;		//!< Node list
	vector<FEEdge>	m_Edge;		//!< Edge list
};
