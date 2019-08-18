#pragma once

#include <assert.h>
#include "MathLib/mat3d.h"
#include "FEItem.h"
#include "FEEdge.h"
#include "FEFace.h"

//-----------------------------------------------------------------------------
// element types
// (NOTE: do not change the order or values of these macros.)
enum FEElementType {
	FE_INVALID_ELEMENT_TYPE,
	FE_HEX8,
	FE_TET4,
	FE_PENTA6,
	FE_QUAD4,
	FE_TRI3,
	FE_BEAM2,
	FE_HEX20,
	FE_QUAD8,
	FE_BEAM3,
	FE_TET10,
	FE_TRI6,
	FE_TET15,
	FE_HEX27,
	FE_TRI7,
	FE_QUAD9,
	FE_PENTA15,
	FE_PYRA5,
	FE_TET20,
	FE_TRI10,
	FE_TET5
};

//-----------------------------------------------------------------------------
// The FEElement_ class defines the data interface to the element data. 
// Specialized element classes are then defined by deriving from this base class.
// A note on shells:
//  - shells require a thickness, which is stored in _h. 
//  - shells can lie on top of solids or sandwhiched between solids
//    For such shells, the _nbr[4] and _nbr[5] are used to identify the elements
//    on top of which they sit.
class FEElement_ : public FEItem
{
public:
	//! constructor
	FEElement_();

	// comparison operator
	bool is_equal(FEElement_& e);

	//! return the element type
	int GetType() const { return m_ntype; }

	//! Is the element of this type
	bool IsType(int ntype) const { return m_ntype == ntype; }

	//! number of nodes
	int Nodes() const { return m_nodes; }

	//! Number of faces (shells have no faces)
	int Faces() const { return m_nfaces; }

	//! Number of edges (solids have no edges)
	int Edges() const { return m_nedges; }

	//! Get the face i (only solids have faces)
	FEFace GetFace(int i) const;
	void GetFace(FEFace& face, int i) const;

	//! Get the edge
	FEEdge GetEdge(int i);

	//! Is this an exterior element
	bool IsExterior();

	//! get the index into the element's node array (or -1 of the element does not have this node)
	int FindNodeIndex(int nid);

	// Find the face. Returns local index in face array
	int FindFace(const FEFace& f);

public: // for shells

	//! Get the face of a shell
	void GetShellFace(FEFace& f) const;

protected:
	// help class for copy-ing element data
	void copy(const FEElement_& el);

public:
	// Check the element class
	bool IsSolid() const { return (m_ntype == FE_HEX8) || (m_ntype == FE_HEX20) || (m_ntype == FE_HEX27) || (m_ntype == FE_PENTA6) || (m_ntype == FE_TET4) || (m_ntype == FE_TET10) || (m_ntype == FE_TET15) || (m_ntype == FE_TET20) || (m_ntype == FE_PYRA5) || (m_ntype == FE_PENTA15) || (m_ntype == FE_TET5); }
	bool IsShell() const { return (m_ntype == FE_TRI3) || (m_ntype == FE_QUAD4) || (m_ntype == FE_QUAD8) || (m_ntype == FE_QUAD9) || (m_ntype == FE_TRI6) || (m_ntype == FE_TRI7); }
	bool IsBeam () const { return (m_ntype == FE_BEAM2); }

public:
	int*		m_node;		//!< pointer to node data
	int*		m_nbr;		//!< neighbour elements
	int*		m_face;		//!< faces (-1 for interior faces)
	double* 	m_h;		//!< element thickness (only used by shells)
	
public:
	vec3d	m_fiber;	//!< fiber orientation \todo maybe I can add an element attribute section
	mat3d	m_Q;		//!< local material orientation
	bool	m_Qactive;	//!< active local material orientation
	double	m_a0;		//!< cross-sectional area (only used by truss elements)
	
protected:
	int		m_ntype; 	//!< type of element
	int		m_nodes;	//!< nr of nodes
	int		m_nfaces;	//!< nr of faces	( 0 for shells)
	int		m_nedges;	//!< nr of edges	( 0 for solids)
};

//-----------------------------------------------------------------------------
// The FEElement class can be used to represent a general purpose element. 
// This class can represent an element of all different types. 
class FEElement : public FEElement_
{
public:
	enum { MAX_NODES = 27 };

public:
	//! constructor
	FEElement();

	//! copy constructor
	FEElement(const FEElement& el);

	//! assignment operator
	FEElement& operator = (const FEElement& el);

	//! Set the element type
	void SetType(int ntype);

private:
	int		_node[MAX_NODES];	//!< nodal id's
	int		_nbr[6];			//!< neighbour elements
	int		_face[6];			//!< faces (-1 for interior faces)
	double 	_h[9];				//!< element thickness (only used by shells)
};
