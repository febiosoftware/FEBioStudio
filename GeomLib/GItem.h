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
#include <FSCore/FSObject.h>
#include <FSCore/box.h>
#include "GPartSection.h"
class FSEdgeSet;

//-----------------------------------------------------------------------------
// State values for GItem state
enum {
	GEO_VISIBLE  = 1, 
	GEO_SELECTED = 2,
	GEO_REQUIRED = 4
};

//-----------------------------------------------------------------------------
// Node types
#define NODE_UNKNOWN	0
#define NODE_VERTEX		1		// vertex defines a node that is part of the geometry
#define NODE_SHAPE		2		// a shape node is only used to define the geometry but is not considered part of it.

//-----------------------------------------------------------------------------
// Edge types
#define EDGE_UNKNOWN		0
#define EDGE_LINE			1		// straight line between two nodes
#define EDGE_3P_CIRC_ARC	2		// 3-point circular arc. 
#define EDGE_YARC			3		// circular arc defined by two points and the y-axis
#define EDGE_ZARC			4		// circular arc defined by two points and the z-axis
#define EDGE_3P_ARC			5		// coordinate alligned elliptical arc section defined by three points
#define EDGE_MESH			6		// edge is determined by a mesh (used by GCurveMeshObject)

//-----------------------------------------------------------------------------
// Face types
#define FACE_UNKNOWN		0	
#define FACE_POLYGON		1		// polygonal area; edges don't have to be linear
#define FACE_EXTRUDE		2		// face is extruded
#define FACE_QUAD			3		// face is bilinear quadrialter
#define FACE_REVOLVE		4		// face created by revolving an edge
#define FACE_REVOLVE_WEDGE	5		// 3-point face create by revolving edge that has one node on the axis

//-----------------------------------------------------------------------------
// Part types
#define PART_UNKNOWN	0
#define PART_COMPLEX	1		// general domain definition; surfaces can be of any type

//-----------------------------------------------------------------------------
namespace GO {
	enum Orientation:int {
		CW = -1,
		CCW = 1
	};
}

//-----------------------------------------------------------------------------
// forward declarations
class GBaseObject;

//-----------------------------------------------------------------------------
// Base class for items of geometry objects.
// GItem's have two ID's: 
//  - m_gid: the global ID number identifying the item number in the model (one-based)
//  - m_lid: the local ID number which is the index in the object's item list (zero-based)
//
class GItem : public FSObject
{
public:
	// Constructor. Takes parent object as parameter
	GItem(GBaseObject* po = 0) { m_state = GEO_VISIBLE; m_gid = 0; m_lid = -1; m_po = po; m_weight = 0.0; }
	virtual ~GItem() { m_po = 0; }

	// get/set global ID
	int GetID() const { return m_gid; }
	virtual void SetID(int nid) = 0;

	// get/set local ID
	void SetLocalID(int n) { m_lid = n; }
	int GetLocalID() const { return m_lid; }

	// get the parent object
	GBaseObject* Object() { return m_po; }
	const GBaseObject* Object() const { return m_po; }

	// check visibility state (only used by GBaseObject's)
	bool IsVisible () const { return ((m_state & GEO_VISIBLE ) != 0); }
	void ShowItem() { m_state = m_state | GEO_VISIBLE;  }
	void HideItem() { m_state = 0; }

	// check selection state
	bool IsSelected() const { return ((m_state & GEO_SELECTED) != 0); }
	void Select  () { m_state = m_state | GEO_SELECTED; }
	void UnSelect() { m_state = m_state & ~GEO_SELECTED; }

	// set required state
	bool IsRequired() const { return ((m_state & GEO_REQUIRED) != 0); }
	void SetRequired(bool b) { if (b) m_state = m_state | GEO_REQUIRED;	else m_state = m_state & ~GEO_REQUIRED; }

	// get/set state
	unsigned int GetState() const { return m_state; }
	void SetState(unsigned int state) { m_state = state; }

	void SetMeshWeight(double w) { m_weight = w; }
	double GetMeshWeight() const { return m_weight; }

public:
	int		m_ntag;	// multi-purpose tag

protected:
	unsigned int	m_state;	// state variable
	int				m_gid;		// global ID (one-based)
	int				m_lid;		// local ID (zero-based)

	double	m_weight;	// weight used for meshing

	GBaseObject*	m_po;	// pointer to object this item belongs to
};

//-----------------------------------------------------------------------------
// Intermediate base class defining a counter for each derived item class that
// can be used to determine a unique global ID number
template <class T> class GItem_T : public GItem
{
public:
	GItem_T(GBaseObject* po = 0) : GItem(po) {}
	void SetID(int nid) { m_gid = nid; if (nid > m_ncount) m_ncount = nid; }
	static int CreateUniqueID() { return ++m_ncount; }
	static void ResetCounter() { m_ncount = 0; }

	static void IncreaseCounter() { m_ncount++; }
	static void DecreaseCounter() { m_ncount--; }

	static void SetCounter(int n) { m_ncount = n; }
	static int GetCounter() { return m_ncount; }
	
private:
	static int	m_ncount;
};

//-----------------------------------------------------------------------------
// Defines a part of the object
class GPart : public GItem_T<GPart>
{
public:
	GPart();
	~GPart();

	GPart(GBaseObject* po);

	GPart(const GPart& p);
	void operator = (const GPart& p);

	bool IsSolid() const;
	bool IsShell() const;
	bool IsBeam () const;

public:
	int GetMaterialID() const { return m_matid; }
	void SetMaterialID(int mid) { m_matid = mid; }

	void SetSection(GPartSection* section);
	GPartSection* GetSection() const;

	BOX GetLocalBox() const;
	BOX GetGlobalBox() const;
	void UpdateBoundingBox();
	bool Update(bool b) override;
    
public:
	int Nodes() const { return (int)m_node.size(); }
	int Edges() const { return (int)m_edge.size(); }
	int Faces() const { return (int)m_face.size(); }

protected:
	int		m_matid;
	GPartSection* m_section;
	BOX		m_box;	// bounding box in local coordinate

public:
	std::vector<int>	m_node;
	std::vector<int>	m_face;
	std::vector<int>	m_edge;
};

//-----------------------------------------------------------------------------
// Defines a face of the object
class GFace : public GItem_T<GFace>
{
public:
	struct EDGE
	{
		int		nid;	// local ID of edge
		int		nwn;	// winding (+1 or -1)
	};

public:
	GFace();
	GFace(GBaseObject* po);

	GFace(const GFace& f);
	void operator = (const GFace& f);

	bool IsExternal() { return (m_nPID[1] == -1); }


	int Nodes() { return (int) m_node.size(); }
	int Edges() { return (int) m_edge.size(); }

	bool HasEdge(int i);

	void Invert();

public:
	int				m_ntype;	// face type
	int				m_nPID[3];	// part ID's
	std::vector<int>	m_node;		// node ID's
	std::vector<EDGE>	m_edge;		// edges defining face
};

class GNode;

// Defines the edge of the object
class GEdge : public GItem_T<GEdge>
{
public:
	GEdge() : GItem_T<GEdge>(0) { m_node[0] = m_node[1] = -1; m_ntype = EDGE_UNKNOWN; m_orient = GO::CCW;  }
	GEdge(GBaseObject* po) : GItem_T<GEdge>(po) { m_node[0] = m_node[1] = -1; m_ntype = EDGE_UNKNOWN; m_orient = GO::CCW;}

	GEdge(const GEdge& e);
	void operator = (const GEdge& e);

	bool operator == (const GEdge& e);

	GNode* Node(int i);

public:
	double Length();

	vec2d Tangent(double l);

	vec3d Point(double l);

	bool HasNode(int n) { return ((m_node[0]==n)||(m_node[1]==n)); }

	int Type() const { return m_ntype; }

	FSEdgeSet* GetFEEdgeSet() const;

public:
	int		m_node[2];	// indices of GNodes
	int		m_cnode;	// center node for arcs
	int		m_orient;	// orientation for arcs
	int		m_ntype;	// type identifier
};

//-----------------------------------------------------------------------------
//! Defines the nodes of the object. A node is defined by its position in space.
//! There are two types of nodes: nodes that define the end-points of edges, and
//! those that do not. The latter types of nodes are only used to define the 
//! geometry, but should not be considered actually part of the object. The m_ntype
//! identifier defines the node type. Also note that only vertices should have
//! global ID's. All nodes should have local ID's though.
class GNode : public GItem_T<GNode>
{
public:
	GNode();
	GNode(GBaseObject* po);

	GNode(const GNode& n);
	void operator = (const GNode& n);

	// get the edge type
	int Type() const { return m_ntype; }

	// set the type
	void SetType(int ntype) { m_ntype = ntype; }

	// get the local position of this node
	vec3d& LocalPosition() { return m_r; }
	const vec3d& LocalPosition() const { return m_r; }

	// get the global position of the node
	vec3d Position() const;

	void MakeRequired();

	void SetNodeIndex(int n) { m_node = n; }
	int GetNodeIndex() const { return m_node; }

private:
	vec3d		m_r;		// node position (in local coordinates)
	int			m_ntype;	// node type
	int			m_node;		// index of node in mesh
};
