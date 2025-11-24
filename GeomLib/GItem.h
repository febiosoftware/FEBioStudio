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
//! State values for GItem state
enum {
	GEO_VISIBLE  = 1, 
	GEO_SELECTED = 2,
	GEO_REQUIRED = 4,
	GEO_ACTIVE   = 8
};

//-----------------------------------------------------------------------------
//! Node types
enum NodeType {
	NODE_UNKNOWN = 0,
	NODE_VERTEX	 = 1,		// vertex defines a node that is part of the geometry
	NODE_SHAPE	 = 2		// a shape node is only used to define the geometry but is not considered part of it.
};

//-----------------------------------------------------------------------------
//! Edge types
enum EdgeType {
	EDGE_UNKNOWN,
	EDGE_LINE,			// straight line between two nodes
	EDGE_3P_CIRC_ARC,	// 3-point circular arc. 
	EDGE_YARC,			// circular arc defined by two points and the y-axis
	EDGE_ZARC,			// circular arc defined by two points and the z-axis
	EDGE_3P_ARC,		// coordinate alligned elliptical arc section defined by three points
	EDGE_MESH,			// edge is determined by a mesh (used by GCurveMeshObject)
	EDGE_BEZIER			// Bezier-curve
};

//-----------------------------------------------------------------------------
//! Face types
enum FaceType {
	FACE_UNKNOWN,
	FACE_POLYGON,	// polygonal area; edges don't have to be linear
	FACE_EXTRUDE,	// face is extruded
	FACE_QUAD,		// face is bilinear quadrilateral
	FACE_REVOLVE,	// face created by revolving an edge
	FACE_REVOLVE_WEDGE,	// 3-point face create by revolving edge that has one node on the axis
};

//-----------------------------------------------------------------------------
//! Part types
#define PART_UNKNOWN	0
#define PART_COMPLEX	1		// general domain definition; surfaces can be of any type

//-----------------------------------------------------------------------------
//! Namespace for geometry orientation
namespace GO {
	enum Orientation:int {
		CW = -1,
		CCW = 1
	};
}

//-----------------------------------------------------------------------------
//! Forward declarations
class GBaseObject;

//-----------------------------------------------------------------------------
//! Base class for items of geometry objects.
//! GItem's have two ID's: 
//!  - m_gid: the global ID number identifying the item number in the model (one-based)
//!  - m_lid: the local ID number which is the index in the object's item list (zero-based)
//!
class GItem : public FSObject
{
public:
	//! Constructor. Takes parent object as parameter
	GItem(GBaseObject* po = 0) { m_state = GEO_VISIBLE | GEO_ACTIVE; m_gid = 0; m_lid = -1; m_po = po; m_weight = 0.0; m_ntag = 0; }
	
	//! Destructor
	virtual ~GItem() { m_po = 0; }

	//! Get global ID
	int GetID() const { return m_gid; }
	
	//! Set global ID (pure virtual)
	virtual void SetID(int nid) = 0;

	//! Set local ID
	void SetLocalID(int n) { m_lid = n; }
	
	//! Get local ID
	int GetLocalID() const { return m_lid; }

	//! Get the parent object (non-const version)
	GBaseObject* Object() { return m_po; }
	
	//! Get the parent object (const version)
	const GBaseObject* Object() const { return m_po; }

	//! Check visibility state (only used by GBaseObject's)
	bool IsVisible () const { return ((m_state & GEO_VISIBLE ) != 0); }
	
	//! Show this item
	void ShowItem() { m_state = m_state | GEO_VISIBLE;  }
	
	//! Hide this item
	void HideItem() { m_state = m_state & ~(GEO_VISIBLE | GEO_SELECTED); }

	//! Check selection state
	bool IsSelected() const { return ((m_state & GEO_SELECTED) != 0); }
	
	//! Select this item
	void Select  () { m_state = m_state | GEO_SELECTED; }
	
	//! Unselect this item
	void UnSelect() { m_state = m_state & ~GEO_SELECTED; }

	//! Check if item is required
	bool IsRequired() const { return ((m_state & GEO_REQUIRED) != 0); }
	
	//! Set required state
	void SetRequired(bool b) { if (b) m_state = m_state | GEO_REQUIRED;	else m_state = m_state & ~GEO_REQUIRED; }

	//! Check active status
	bool IsActive() const { { return ((m_state & GEO_ACTIVE) != 0); } }
	
	//! Set active state
	void SetActive(bool b) { if (b) m_state = m_state | GEO_ACTIVE;	else m_state = m_state & ~GEO_ACTIVE; }

	//! Get state flags
	unsigned int GetState() const { return m_state; }
	
	//! Set state flags
	void SetState(unsigned int state) { m_state = state; }

	//! Set mesh weight for this item
	void SetMeshWeight(double w) { m_weight = w; }
	
	//! Get mesh weight for this item
	double GetMeshWeight() const { return m_weight; }

public:
	//! Multi-purpose tag
	int		m_ntag;

protected:
	//! State variable
	unsigned int	m_state;
	
	//! Global ID (one-based)
	int				m_gid;
	
	//! Local ID (zero-based)
	int				m_lid;

	//! Weight used for meshing
	double	m_weight;

	//! Pointer to object this item belongs to
	GBaseObject*	m_po;
};

//-----------------------------------------------------------------------------
//! Intermediate base class defining a counter for each derived item class that
//! can be used to determine a unique global ID number
template <class T> class GItem_T : public GItem
{
public:
	//! Constructor
	GItem_T(GBaseObject* po = 0) : GItem(po) {}
	
	//! Set ID and update counter
	void SetID(int nid) { m_gid = nid; if (nid > m_ncount) m_ncount = nid; }
	
	//! Create a unique ID
	static int CreateUniqueID() { return ++m_ncount; }
	
	//! Reset the counter
	static void ResetCounter() { m_ncount = 0; }

	//! Increase the counter
	static void IncreaseCounter() { m_ncount++; }
	
	//! Decrease the counter
	static void DecreaseCounter() { m_ncount--; }

	//! Set the counter value
	static void SetCounter(int n) { m_ncount = n; }
	
	//! Get the counter value
	static int GetCounter() { return m_ncount; }
	
private:
	//! Static counter for unique IDs
	static int	m_ncount;
};

//-----------------------------------------------------------------------------
//! Defines a part of the object
class GPart : public GItem_T<GPart>
{
public:
	//! Default constructor
	GPart();
	
	//! Destructor
	~GPart();

	//! Constructor with parent object
	GPart(GBaseObject* po);

	//! Copy constructor
	GPart(const GPart& p);
	
	//! Assignment operator
	void operator = (const GPart& p);

public:
	//! Set the part section
	void SetSection(GPartSection* section);
	
	//! Get the part section
	GPartSection* GetSection() const;

	//! Check if part is solid
	bool IsSolid() const;
	
	//! Check if part is shell
	bool IsShell() const;
	
	//! Check if part is beam
	bool IsBeam () const;

	//! Get material ID
	int GetMaterialID() const { return m_matid; }
	
	//! Set material ID
	void SetMaterialID(int mid) { m_matid = mid; }

public:
	//! Save to archive
	void Save(OArchive& ar) override;
	
	//! Load from archive
	void Load(IArchive& ar) override;

	//! Update the part
	bool Update(bool b) override;

public:
	//! Get number of nodes
	int Nodes() const { return (int)m_node.size(); }
	
	//! Get number of edges
	int Edges() const { return (int)m_edge.size(); }
	
	//! Get number of faces
	int Faces() const { return (int)m_face.size(); }

protected:
	//! Material ID
	int		m_matid;
	
	//! Part section
	GPartSection* m_section;
	
public:
	//! Node indices
	std::vector<int>	m_node;
	
	//! Face indices
	std::vector<int>	m_face;
	
	//! Edge indices
	std::vector<int>	m_edge;
};

//-----------------------------------------------------------------------------
//! Defines a face of the object
class GFace : public GItem_T<GFace>
{
public:
	//! Edge structure containing ID and winding
	struct EDGE
	{
		//! Local ID of edge
		int		nid;
		
		//! Winding (+1 or -1)
		int		nwn;
	};

public:
	//! Default constructor
	GFace();
	
	//! Constructor with parent object
	GFace(GBaseObject* po);

	//! Copy constructor
	GFace(const GFace& f);
	
	//! Assignment operator
	void operator = (const GFace& f);

	//! Check if face is external
	bool IsExternal() { return (m_nPID[1] == -1); }

	//! Get number of nodes
	int Nodes() { return (int) m_node.size(); }
	
	//! Get number of edges
	int Edges() { return (int) m_edge.size(); }

	//! Check if face has a specific edge
	bool HasEdge(int i);

	//! Invert the face orientation
	void Invert();

public:
	//! Save to archive
	void Save(OArchive& ar) override;
	
	//! Load from archive
	void Load(IArchive& ar) override;

public:
	//! Face type
	int				m_ntype;
	
	//! Part ID's
	int				m_nPID[3];
	
	//! Node ID's
	std::vector<int>	m_node;
	
	//! Edges defining face
	std::vector<EDGE>	m_edge;
};

//! Forward declaration
class GNode;

//! Defines the edge of the object
class GEdge : public GItem_T<GEdge>
{
public:
	//! Default constructor
	GEdge() : GItem_T<GEdge>(0) { m_node[0] = m_node[1] = -1; m_ntype = EDGE_UNKNOWN; m_orient = GO::CCW; }
	
	//! Constructor with parent object
	GEdge(GBaseObject* po) : GItem_T<GEdge>(po) { m_node[0] = m_node[1] = -1; m_ntype = EDGE_UNKNOWN; m_orient = GO::CCW;}

	//! Copy constructor
	GEdge(const GEdge& e);
	
	//! Assignment operator
	void operator = (const GEdge& e);

	//! Equality operator
	bool operator == (const GEdge& e);

	//! Get node at index
	GNode* Node(int i);

public:
	//! Save to archive
	void Save(OArchive& ar) override;
	
	//! Load from archive
	void Load(IArchive& ar) override;

public:
	//! Calculate edge length
	double Length();

	//! Get tangent vector at parameter l
	vec2d Tangent(double l);

	//! Get point at parameter l
	vec3d Point(double l);

	//! Check if edge has a specific node
	bool HasNode(int n) { return ((m_node[0]==n)||(m_node[1]==n)); }

	//! Get edge type
	int Type() const { return m_ntype; }

	//! Get FE edge set
	FSEdgeSet* GetFEEdgeSet() const;

public:
	//! Indices of start and end nodes
	int		m_node[2];
	
	//! Additional shape nodes
	std::vector<int>		m_cnode;
	
	//! Orientation for arcs
	int		m_orient;
	
	//! Type identifier
	int		m_ntype;
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
	//! Default constructor
	GNode();
	
	//! Constructor with parent object
	GNode(GBaseObject* po);

	//! Copy constructor
	GNode(const GNode& n);
	
	//! Assignment operator
	void operator = (const GNode& n);

	//! Get the node type
	int Type() const { return m_ntype; }

	//! Set the node type
	void SetType(int ntype) { m_ntype = ntype; }

	//! Get the local position of this node (non-const version)
	vec3d& LocalPosition() { return m_r; }
	
	//! Get the local position of this node (const version)
	const vec3d& LocalPosition() const { return m_r; }

	//! Get the global position of the node
	vec3d Position() const;

	//! Make this node required
	void MakeRequired();

	//! Set the node index in the mesh
	void SetNodeIndex(int n) { m_node = n; }
	
	//! Get the node index in the mesh
	int GetNodeIndex() const { return m_node; }

public:
	//! Save to archive
	void Save(OArchive& ar) override;
	
	//! Load from archive
	void Load(IArchive& ar) override;

private:
	//! Node position (in local coordinates)
	vec3d		m_r;
	
	//! Node type
	int			m_ntype;
	
	//! Index of node in mesh
	int			m_node;
};