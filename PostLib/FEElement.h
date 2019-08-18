#pragma once
#include "math3d.h"
#include "FEItem.h"
#include "FEFace.h"
#include <assert.h>

namespace Post {

//-----------------------------------------------------------------------------
// Different element types
// Add new elements to the bottom of the list. Do not change the order of these numbers
// as they are used as indices into the FEElementLibrary element list.
enum FEElemType {
	FE_LINE2,
	FE_LINE3,
	FE_TRI3,
    FE_TRI6,
	FE_QUAD4,
	FE_QUAD8,
    FE_QUAD9,
	FE_TET4,
	FE_TET10,
	FE_TET15,
	FE_TET20,
	FE_PENTA6,
    FE_PENTA15,
    FE_HEX8,
	FE_HEX20,
	FE_HEX27,
	FE_PYRA5,
	FE_TET5
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
// Forward declaration of the element class
class FEElement;

//-----------------------------------------------------------------------------
// Element traits
struct ElemTraits
{
	int		ntype;	// type of element
	int		nshape;	// shape of element
	int		nclass; // element class
	int		nodes;	// number of nodes
	int		faces;	// number of faces (only for solid elements)
	int		edges;	// number of edges (only for shell elements)
};

//-----------------------------------------------------------------------------
// Class that describes an element of the mesh
class FEElement : public FEItem
{
public:
	FEElement();
		
	bool HasNode(int node) const;

public:
	FEFace GetFace(int i) const;
	void GetFace(int i, FEFace& face) const;
	FEEdge GetEdge(int i) const;

	bool operator != (FEElement& el);

	// evaluate shape function at iso-parameteric point (r,s,t)
	void shape(double* H, double r, double s, double t);

	// evaluate a vector expression at iso-points (r,s)
	double eval(double* d, double r, double s, double t);

	// evaluate a vector expression at iso-points (r,s)
	float eval(float* d, double r, double s, double t);

	// evaluate a vector expression at iso-points (r,s)
	vec3f eval(vec3f* v, double r, double s, double t);

	// shape function derivatives
	void shape_deriv(double* Hr, double* Hs, double* Ht, double r, double s, double t);

	// get iso-param coordinates of the nodes
	void iso_coord(int n, double q[3]);

	bool IsExterior() const;

public:
	void SetType(FEElemType type);
	int Type() const { return m_traits->ntype; }
	int Shape() const { return m_traits->nshape; }
	int Class() const { return m_traits->nclass; }

	bool IsSolid() const { return (m_traits->nclass == ELEM_SOLID); }
	bool IsShell() const { return (m_traits->nclass == ELEM_SHELL); }
	bool IsBeam () const { return (m_traits->nclass == ELEM_BEAM ); }

	int Nodes() const { return m_traits->nodes; }
	int Faces() const { return m_traits->faces; }
	int Edges() const { return m_traits->edges; }

public:
	int			m_lid;		// local ID (zero-based index into element array)
	int			m_MatID;	// material id
	int*		m_node;		// array of nodes ID
	float		m_tex;		// element texture coordinate

	FEElement*	m_pElem[6];		// array of pointers to neighbour elements

protected:
	const ElemTraits* m_traits;	// element traits
};

//=============================================================================
// Element traits classes
template <int Type> class FEElementTraits {};
// Each of these classes defines:
// Nodes: number of nodes
// Faces: number of faces. Only solid elements define faces
// Edges: number of edges. Only surface elements define edges

template <> class FEElementTraits<FE_LINE2  >{ public: enum {Nodes =  2}; enum {Faces = 0}; enum {Edges = 0}; static FEElemType Type() { return FE_LINE2 ; }};
template <> class FEElementTraits<FE_LINE3  >{ public: enum {Nodes =  3}; enum {Faces = 0}; enum {Edges = 0}; static FEElemType Type() { return FE_LINE3 ; }};
template <> class FEElementTraits<FE_TRI3   >{ public: enum {Nodes =  3}; enum {Faces = 0}; enum {Edges = 3}; static FEElemType Type() { return FE_TRI3  ; }};
template <> class FEElementTraits<FE_TRI6   >{ public: enum {Nodes =  6}; enum {Faces = 0}; enum {Edges = 3}; static FEElemType Type() { return FE_TRI6  ; }};
template <> class FEElementTraits<FE_QUAD4  >{ public: enum {Nodes =  4}; enum {Faces = 0}; enum {Edges = 4}; static FEElemType Type() { return FE_QUAD4 ; }};
template <> class FEElementTraits<FE_QUAD8  >{ public: enum {Nodes =  8}; enum {Faces = 0}; enum {Edges = 4}; static FEElemType Type() { return FE_QUAD8 ; }};
template <> class FEElementTraits<FE_QUAD9  >{ public: enum {Nodes =  9}; enum {Faces = 0}; enum {Edges = 4}; static FEElemType Type() { return FE_QUAD9 ; }};
template <> class FEElementTraits<FE_TET4   >{ public: enum {Nodes =  4}; enum {Faces = 4}; enum {Edges = 0}; static FEElemType Type() { return FE_TET4  ; }};
template <> class FEElementTraits<FE_TET10  >{ public: enum {Nodes = 10}; enum {Faces = 4}; enum {Edges = 0}; static FEElemType Type() { return FE_TET10 ; }};
template <> class FEElementTraits<FE_TET15  >{ public: enum {Nodes = 15}; enum {Faces = 4}; enum {Edges = 0}; static FEElemType Type() { return FE_TET15 ; }};
template <> class FEElementTraits<FE_TET20  >{ public: enum {Nodes = 20}; enum {Faces = 4}; enum {Edges = 0}; static FEElemType Type() { return FE_TET20 ; }};
template <> class FEElementTraits<FE_PENTA6 >{ public: enum {Nodes =  6}; enum {Faces = 5}; enum {Edges = 0}; static FEElemType Type() { return FE_PENTA6; }};
template <> class FEElementTraits<FE_PENTA15>{ public: enum {Nodes = 15}; enum {Faces = 5}; enum {Edges = 0}; static FEElemType Type() { return FE_PENTA15;}};
template <> class FEElementTraits<FE_HEX8   >{ public: enum {Nodes =  8}; enum {Faces = 6}; enum {Edges = 0}; static FEElemType Type() { return FE_HEX8  ; }};
template <> class FEElementTraits<FE_HEX20  >{ public: enum {Nodes = 20}; enum {Faces = 6}; enum {Edges = 0}; static FEElemType Type() { return FE_HEX20 ; }};
template <> class FEElementTraits<FE_HEX27  >{ public: enum {Nodes = 27}; enum {Faces = 6}; enum {Edges = 0}; static FEElemType Type() { return FE_HEX27 ; }};
template <> class FEElementTraits<FE_PYRA5  >{ public: enum {Nodes =  5}; enum {Faces = 5}; enum {Edges = 0}; static FEElemType Type() { return FE_PYRA5 ; }};
template <> class FEElementTraits<FE_TET5   >{ public: enum {Nodes =  5}; enum {Faces = 4}; enum {Edges = 0}; static FEElemType Type() { return FE_TET5  ; }};

template <class T> class FEElementBase : public FEElement
{
public:
	FEElementBase()
	{
		SetType(T::Type());
		m_node = _node;
		for (int i=0; i<T::Nodes; ++i) m_node[i] = -1;
	}

	FEElementBase(const FEElementBase& el) : FEElement(el)
	{
		m_node = _node;
		for (int i=0; i<T::Nodes; ++i) m_node[i] = el.m_node[i];
	}

	void operator = (const FEElementBase& el)
	{
		FEElement::operator = (el);
		for (int i=0; i<T::Nodes; ++i) m_node[i] = el.m_node[i];
	}

public:
	int	_node[T::Nodes];
};

typedef FEElementBase< FEElementTraits<FE_LINE2  > > FELine2;
typedef FEElementBase< FEElementTraits<FE_LINE3  > > FELine3;
typedef FEElementBase< FEElementTraits<FE_TRI3   > > FETri3;
typedef FEElementBase< FEElementTraits<FE_TRI6   > > FETri6;
typedef FEElementBase< FEElementTraits<FE_QUAD4  > > FEQuad4;
typedef FEElementBase< FEElementTraits<FE_QUAD8  > > FEQuad8;
typedef FEElementBase< FEElementTraits<FE_QUAD9  > > FEQuad9;
typedef FEElementBase< FEElementTraits<FE_TET4   > > FETet4;
typedef FEElementBase< FEElementTraits<FE_TET10  > > FETet10;
typedef FEElementBase< FEElementTraits<FE_TET15  > > FETet15;
typedef FEElementBase< FEElementTraits<FE_TET20  > > FETet20;
typedef FEElementBase< FEElementTraits<FE_PENTA6 > > FEPenta6;
typedef FEElementBase< FEElementTraits<FE_PENTA15> > FEPenta15;
typedef FEElementBase< FEElementTraits<FE_HEX8   > > FEHex8;
typedef FEElementBase< FEElementTraits<FE_HEX20  > > FEHex20;
typedef FEElementBase< FEElementTraits<FE_HEX27  > > FEHex27;
typedef FEElementBase< FEElementTraits<FE_PYRA5  > > FEPyra5;
typedef FEElementBase< FEElementTraits<FE_TET5   > > FETet5;

//-----------------------------------------------------------------------------
// Generice element class that can represent any of the supported element classes
class FEGenericElement : public FEElement
{
public:
	enum {MAX_NODES = 27};

public:
	FEGenericElement();
	FEGenericElement(const FEGenericElement& e);
	void operator = (const FEGenericElement& e);

public:
	int		_node[MAX_NODES];	// array of nodes ID
};

//-----------------------------------------------------------------------------
// This element class can represent any of the linear elements.
class FELinearElement : public FEElement
{
public:
	enum {MAX_NODES = 8};

public:
	FELinearElement();
	FELinearElement(const FELinearElement& e);
	void operator = (const FELinearElement& e);

public:
	int		_node[MAX_NODES];	// array of nodes ID
};

//=============================================================================
class FEElementLibrary
{
public:
	static void InitLibrary();

	static const ElemTraits* GetTraits(FEElemType type);

private:
	FEElementLibrary(){}
	FEElementLibrary(const FEElementLibrary&){}

	static void addElement(int ntype, int nshape, int nclass, int nodes, int faces, int edges);

private:
	static vector<ElemTraits>	m_lib;
};
}
