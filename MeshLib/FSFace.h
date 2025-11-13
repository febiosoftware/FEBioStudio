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
#include "FSMeshItem.h"
#include "FSEdge.h"
#include <FSCore/math3d.h>

//-----------------------------------------------------------------------------
//! Face shapes
enum FEFaceShape
{
	FE_FACE_INVALID_SHAPE,
	FE_FACE_TRI,		// triangle
	FE_FACE_QUAD		// quadrilateral
};

//-----------------------------------------------------------------------------
//! Different face types
//! (NOTE: Do not change the order of these values)
enum FEFaceType
{
	FE_FACE_INVALID_TYPE,
	FE_FACE_TRI3,
	FE_FACE_QUAD4,
	FE_FACE_TRI6,
	FE_FACE_TRI7,
	FE_FACE_QUAD8,
	FE_FACE_QUAD9,
	FE_FACE_TRI10
};

//-----------------------------------------------------------------------------
//! FSFace class stores face data. 
//! A face can either have 3, 4, 6, 7, 8, 9 or 10 nodes. 
//!  - 3  : linear triangle
//!  - 6,7: quadratic triangle
//!  - 10 : cubic triangle
//!  - 4  : linear quad
//!  - 8,9: quadratic quad
//!
//!   4       7       3      3            3
//!   +-------o-------+      +            +
//!   |               |      |\           | \
//!   |               |      | \         9o    o7
//!  8o       x9      o6    6o  o5        | x10 \
//!   |               |      | x7 \       8o     o6 
//!   |               |      |     \      |       \
//!   +-------o-------+      +--o--+      +--o--o--+
//!   1       5       2      1  4  2      1  4  5  2
//!
//! Note that for the first three nodes for a triangle and the first four nodes
//! of a quad are always the corner nodes.
//!
class FSFace : public FSMeshItem
{
public:
	enum { MAX_NODES = 10 };

	//! Element reference structure
	struct ELEM_REF
	{
		int		eid;	// element ID
		int		lid;	// local face index into element's face array
	};

public:
	//! constructor
	FSFace();

	//! comparison operator
	bool operator == (const FSFace& f) const;

	//! get the type
	int Type() const { return m_type; }

	//! set the type
	void SetType(FEFaceType type);

	//! get the shape
	int Shape() const;

	//! return number of nodes
	int Nodes() const;

	//! return number of edges
	int Edges() const;

	//! get the edge node numbers
	int GetEdgeNodes(int i, int* n) const;

	//! return an edge
	FSEdge GetEdge(int i) const;

	//! See if this face has an edge
	bool HasEdge(int n1, int n2);

	//! See if this face has node with ID i
	bool HasNode(int i);

	//! Find the array index of node with ID i
	int FindNode(int i);

	//! Is this face internal or external
	bool IsExternal() { return (m_elem[1].eid == -1); }

	//! See if a node list is an edge
	int FindEdge(const FSEdge& edge);

public:
	//! evaluate shape function at iso-parameteric point (r,s)
	void shape(double* H, double r, double s);

    //! evaluate derivatives of shape function at iso-parameteric point (r,s)
    void shape_deriv(double* Hr, double* Hs, double r, double s);

	//! evaluate a scalar expression at iso-points (r,s)
	double eval(double* d, double r, double s);

	//! evaluate a vector expression at iso-points (r,s)
	vec3f eval(vec3f* v, double r, double s);
	//! evaluate a vector expression at iso-points (r,s)
	vec3d eval(vec3d* v, double r, double s);

    //! evaluate the derivative of a scalar expression at iso-points (r,s)
    double eval_deriv1(double* d, double r, double s);
    //! evaluate the derivative of a scalar expression at iso-points (r,s)
    double eval_deriv2(double* d, double r, double s);

    //! evaluate the derivative of a vector expression at iso-points (r,s)
    vec3d eval_deriv1(vec3d* v, double r, double s);
    //! evaluate the derivative of a vector expression at iso-points (r,s)
    vec3d eval_deriv2(vec3d* v, double r, double s);
    
    //! evaluate gauss integration points and weights
    int gauss(double* gr, double* gs, double* gw);
    
public:
	//! face type
	int	m_type;
	//! nodal ID's
	int	n[MAX_NODES];

	//! neighbour faces
	int		m_nbr[4];

	// TODO: move texture coordinates elsewhere
	//! nodal 1D-texture coordinates
	float	m_tex[MAX_NODES];
	//! element texture coordinate
	float	m_texe;

	//! the elements to which this face belongs
	ELEM_REF	m_elem[3];
	//! the edges (interior faces don't have edges!)
	int			m_edge[4];
};
