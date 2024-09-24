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

#include <assert.h>
#include <FSCore/math3d.h>
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
	FE_TET5,
	FE_PYRA13
};

//-----------------------------------------------------------------------------
// Element shapes
enum FEElemShape {
	ELEM_LINE,
	ELEM_TRI,
	ELEM_QUAD,
	ELEM_TET,
	ELEM_PENTA,
	ELEM_HEX,
	ELEM_PYRA
};

//-----------------------------------------------------------------------------
// Element class
enum FEElemClass
{
	ELEM_BEAM,
	ELEM_SHELL,
	ELEM_SOLID
};

//-----------------------------------------------------------------------------
// Element traits
struct FSElemTraits
{
	int	ntype;	// type of element
	int	nshape;	// shape of element
	int	nclass; // element class
	int	nodes;	// number of nodes
	int	faces;	// number of faces (only for solid elements)
	int	edges;	// number of edges (only for shell elements)
};

//-----------------------------------------------------------------------------
// The FEElement_ class defines the data interface to the element data. 
// Specialized element classes are then defined by deriving from this base class.
// A note on shells:
//  - shells require a thickness, which is stored in _h. 
//  - shells can lie on top of solids or sandwhiched between solids
//    For such shells, the _nbr[4] and _nbr[5] are used to identify the elements
//    on top of which they sit.
class FEElement_ : public MeshItem
{
public:
	//! constructor
	FEElement_();

public:
	//! Set the element type
	void SetType(int ntype);

	int Type () const { return m_traits->ntype; }
	int Shape() const { return m_traits->nshape; }
	int Class() const { return m_traits->nclass; }

	bool IsSolid() const { return (m_traits->nclass == ELEM_SOLID); }
	bool IsShell() const { return (m_traits->nclass == ELEM_SHELL); }
	bool IsBeam () const { return (m_traits->nclass == ELEM_BEAM ); }

	int Nodes() const { return m_traits->nodes; }
	int Faces() const { return m_traits->faces; }
	int Edges() const { return m_traits->edges; }

public:
	// comparison operator
	bool is_equal(FEElement_& e);

	bool operator != (FEElement_& el);

	bool HasNode(int node) const;

	//! Is the element of this type
	bool IsType(int ntype) const { return (Type() == ntype); }

	//! get the index into the element's node array (or -1 of the element does not have this node)
	int FindNodeIndex(int nid);

public: // for solid elements only

	//! Get the face i
	FSFace GetFace(int i) const;
	void GetFace(int i, FSFace& face) const;
	int GetLocalFaceIndices(int i, int* n) const;

	// Find the face. Returns local index in face array
	int FindFace(const FSFace& f);

public: // for shell elements only

	//! Get the face of a shell
	void GetShellFace(FSFace& f) const;

	//! Get the edge
	FSEdge GetEdge(int i) const;

	//! Find the edge index of a shell
	int FindEdge(const FSEdge& edge) const;

public:
	// evaluate shape function at iso-parameteric point (r,s,t)
	void shape(double* H, double r, double s, double t);

	// evaluate a vector expression at iso-points (r,s,t)
	double eval(double* d, double r, double s, double t);

	// evaluate a vector expression at iso-points (r,s,t)
	float eval(float* d, double r, double s, double t);

	// evaluate a vector expression at iso-points (r,s,t)
	vec3f eval(vec3f* v, double r, double s, double t);

	// evaluate a vector expression at iso-points (r,s)
	vec3f eval(vec3f* v, double r, double s);

	// shape function derivatives
	void shape_deriv(double* Hr, double* Hs, double* Ht, double r, double s, double t);

	// get iso-param coordinates of the nodes
	void iso_coord(int n, double q[3]);

	// get iso-param coordinates of the nodes
	void iso_coord_2d(int n, double q[2]);

	// set the material axis
	void setAxes(const vec3d& a, const vec3d& d);

public:
	// evaluate shape function at iso-parameteric point (r,s) (for 2D elements only!)
	void shape_2d(double* H, double r, double s);

	// shape function derivatives (for 2D elements only)
	void shape_deriv_2d(double* Hr, double* Hs, double r, double s);

protected:
	// help class for copy-ing element data
	void copy(const FEElement_& el);

public:
	int*		m_node;		//!< pointer to node data
	int*		m_nbr;		//!< neighbour elements
	int*		m_face;		//!< faces (-1 for interior faces)
	double* 	m_h;		//!< element thickness (only used by shells)

public:
	int			m_lid;		// local ID (zero-based index into element array)
	int			m_MatID;	// material id
	float		m_tex;		// element texture coordinate

public:
	vec3d	m_fiber;	//!< fiber orientation \todo maybe I can add an element attribute section
	mat3d	m_Q;		//!< local material orientation
	bool	m_Qactive;	//!< active local material orientation
	double	m_a0;		//!< cross-sectional area (only used by truss elements)
	
protected:
	const FSElemTraits* m_traits;	// element traits
};

//-----------------------------------------------------------------------------
// The FSElement class can be used to represent a general purpose element. 
// This class can represent an element of all different types. 
class FSElement : public FEElement_
{
public:
	enum { MAX_NODES = 27 };

public:
	//! constructor
	FSElement();

	//! copy constructor
	FSElement(const FSElement& el);

	//! assignment operator
	FSElement& operator = (const FSElement& el);

private:
	int		_node[MAX_NODES];	//!< nodal id's
	int		_nbr[6];			//!< neighbour elements
	int		_face[6];			//!< faces (-1 for interior faces)
	double 	_h[9];				//!< element thickness (only used by shells)
};

//=============================================================================
// Element traits classes
template <int Type> class FSElementTraits {};
// Each of these classes defines:
// Nodes: number of nodes
// Faces: number of faces. Only solid elements define faces
// Edges: number of edges. Only surface elements define edges

template <> class FSElementTraits<FE_BEAM2  > { public: enum { Nodes = 2 }; enum { Faces = 0 }; enum { Edges = 0 }; static FEElementType Type() { return FE_BEAM2; } };
template <> class FSElementTraits<FE_BEAM3  > { public: enum { Nodes = 3 }; enum { Faces = 0 }; enum { Edges = 0 }; static FEElementType Type() { return FE_BEAM3; } };
template <> class FSElementTraits<FE_TRI3   > { public: enum { Nodes = 3 }; enum { Faces = 0 }; enum { Edges = 3 }; static FEElementType Type() { return FE_TRI3; } };
template <> class FSElementTraits<FE_TRI6   > { public: enum { Nodes = 6 }; enum { Faces = 0 }; enum { Edges = 3 }; static FEElementType Type() { return FE_TRI6; } };
template <> class FSElementTraits<FE_QUAD4  > { public: enum { Nodes = 4 }; enum { Faces = 0 }; enum { Edges = 4 }; static FEElementType Type() { return FE_QUAD4; } };
template <> class FSElementTraits<FE_QUAD8  > { public: enum { Nodes = 8 }; enum { Faces = 0 }; enum { Edges = 4 }; static FEElementType Type() { return FE_QUAD8; } };
template <> class FSElementTraits<FE_QUAD9  > { public: enum { Nodes = 9 }; enum { Faces = 0 }; enum { Edges = 4 }; static FEElementType Type() { return FE_QUAD9; } };
template <> class FSElementTraits<FE_TET4   > { public: enum { Nodes = 4 }; enum { Faces = 4 }; enum { Edges = 0 }; static FEElementType Type() { return FE_TET4; } };
template <> class FSElementTraits<FE_TET10  > { public: enum { Nodes = 10 }; enum { Faces = 4 }; enum { Edges = 0 }; static FEElementType Type() { return FE_TET10; } };
template <> class FSElementTraits<FE_TET15  > { public: enum { Nodes = 15 }; enum { Faces = 4 }; enum { Edges = 0 }; static FEElementType Type() { return FE_TET15; } };
template <> class FSElementTraits<FE_TET20  > { public: enum { Nodes = 20 }; enum { Faces = 4 }; enum { Edges = 0 }; static FEElementType Type() { return FE_TET20; } };
template <> class FSElementTraits<FE_PENTA6 > { public: enum { Nodes = 6 }; enum { Faces = 5 }; enum { Edges = 0 }; static FEElementType Type() { return FE_PENTA6; } };
template <> class FSElementTraits<FE_PENTA15> { public: enum { Nodes = 15 }; enum { Faces = 5 }; enum { Edges = 0 }; static FEElementType Type() { return FE_PENTA15; } };
template <> class FSElementTraits<FE_HEX8   > { public: enum { Nodes = 8 }; enum { Faces = 6 }; enum { Edges = 0 }; static FEElementType Type() { return FE_HEX8; } };
template <> class FSElementTraits<FE_HEX20  > { public: enum { Nodes = 20 }; enum { Faces = 6 }; enum { Edges = 0 }; static FEElementType Type() { return FE_HEX20; } };
template <> class FSElementTraits<FE_HEX27  > { public: enum { Nodes = 27 }; enum { Faces = 6 }; enum { Edges = 0 }; static FEElementType Type() { return FE_HEX27; } };
template <> class FSElementTraits<FE_PYRA5  > { public: enum { Nodes = 5 }; enum { Faces = 5 }; enum { Edges = 0 }; static FEElementType Type() { return FE_PYRA5; } };
template <> class FSElementTraits<FE_TET5   > { public: enum { Nodes = 5 }; enum { Faces = 4 }; enum { Edges = 0 }; static FEElementType Type() { return FE_TET5; } };
template <> class FSElementTraits<FE_PYRA13 > { public: enum { Nodes = 13 }; enum { Faces = 5 }; enum { Edges = 0 }; static FEElementType Type() { return FE_PYRA13; } };

template <class T> class FEElementBase : public FEElement_
{
public:
	FEElementBase()
	{
		SetType(T::Type());
		m_node = _node;
		m_nbr = _nbr;
		m_face = _face;
		m_h = _h;
		for (int i = 0; i<T::Nodes; ++i) m_node[i] = -1;
	}

	FEElementBase(const FEElementBase& el) : FEElement_(el)
	{
		m_node = _node;
		for (int i = 0; i<T::Nodes; ++i) m_node[i] = el.m_node[i];
		m_nbr = _nbr;
	}

	void operator = (const FEElementBase& el)
	{
		FEElement_::operator = (el);
		for (int i = 0; i<T::Nodes; ++i) m_node[i] = el.m_node[i];
	}

public:
	int		_node[T::Nodes];
	int		_nbr[6];
	int		_face[6];			//!< faces (-1 for interior faces)
	double 	_h[9];				//!< element thickness (only used by shells)
};

typedef FEElementBase< FSElementTraits<FE_BEAM2  > > FELine2;
typedef FEElementBase< FSElementTraits<FE_BEAM3  > > FELine3;
typedef FEElementBase< FSElementTraits<FE_TRI3   > > FETri3;
typedef FEElementBase< FSElementTraits<FE_TRI6   > > FETri6;
typedef FEElementBase< FSElementTraits<FE_QUAD4  > > FEQuad4;
typedef FEElementBase< FSElementTraits<FE_QUAD8  > > FEQuad8;
typedef FEElementBase< FSElementTraits<FE_QUAD9  > > FEQuad9;
typedef FEElementBase< FSElementTraits<FE_TET4   > > FETet4;
typedef FEElementBase< FSElementTraits<FE_TET10  > > FETet10;
typedef FEElementBase< FSElementTraits<FE_TET15  > > FETet15;
typedef FEElementBase< FSElementTraits<FE_TET20  > > FETet20;
typedef FEElementBase< FSElementTraits<FE_PENTA6 > > FEPenta6;
typedef FEElementBase< FSElementTraits<FE_PENTA15> > FEPenta15;
typedef FEElementBase< FSElementTraits<FE_HEX8   > > FEHex8;
typedef FEElementBase< FSElementTraits<FE_HEX20  > > FEHex20;
typedef FEElementBase< FSElementTraits<FE_HEX27  > > FEHex27;
typedef FEElementBase< FSElementTraits<FE_PYRA5  > > FEPyra5;
typedef FEElementBase< FSElementTraits<FE_TET5   > > FETet5;
typedef FEElementBase< FSElementTraits<FE_PYRA13 > > FEPyra13;

//-----------------------------------------------------------------------------
// This element class can represent any of the linear elements.
class FELinearElement : public FEElement_
{
public:
	enum { MAX_NODES = 8 };

public:
	FELinearElement();
	FELinearElement(const FELinearElement& e);
	void operator = (const FELinearElement& e);

public:
	int		_node[MAX_NODES];	// array of nodes ID
	int		_nbr[6];
	int		_face[6];			//!< faces (-1 for interior faces)
	double 	_h[9];				//!< element thickness (only used by shells)
};
