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

// FEShellDisc.cpp: implementation of the FEShellDisc class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FEShellDisc.h"
#include <GeomLib/GPrimitive.h>
#include <MeshLib/FEMesh.h>
#include "FEMultiQuadMesh.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FEShellDisc::FEShellDisc()
{
	m_pobj = nullptr;

	m_r = 0.5;
	m_t = 0.01;
	m_nd = m_nr = 6;

	AddDoubleParam(m_r, "r", "Ratio");
	AddDoubleParam(m_t, "t", "Thickness");
	AddIntParam(m_nd, "nd", "Divisions");
	AddIntParam(m_nr, "nr", "Segments");
	AddChoiceParam(0, "elem_type", "Element Type")->SetEnumNames("QUAD4\0QUAD8\0QUAD9\0");
}

FSMesh* FEShellDisc::BuildMesh(GObject* po)
{
	m_pobj = dynamic_cast<GDisc*>(po);
	if (m_pobj == nullptr) return nullptr;

	if (BuildMultiQuad() == false) return nullptr;

	// set element type
	int elemType = GetIntValue(ELEM_TYPE);
	switch (elemType)
	{
	case 0: SetElementType(FE_QUAD4); break;
	case 1: SetElementType(FE_QUAD8); break;
	case 2: SetElementType(FE_QUAD9); break;
	};

	// Build the mesh
	FSMesh* pm = FEMultiQuadMesh::BuildMQMesh();
	if (pm == nullptr) return nullptr;

	pm->BuildMesh();

	// assign shell thickness to section
	double h = GetFloatValue(T);
	GPart* part = m_pobj->Part(0); assert(part);
	GShellSection* shellSection = dynamic_cast<GShellSection*>(part->GetSection());
	if (shellSection) shellSection->SetShellThickness(h);
	else pm->SetUniformShellThickness(h);

	return pm;
}

bool FEShellDisc::BuildMultiQuad()
{
	ClearMQ();

	double r = m_pobj->GetFloatValue(GDisc::RADIUS);
	double f = GetFloatValue(RATIO);
	int nd = GetIntValue(NDIV);
	int ns = GetIntValue(NSEG);

	// position the nodes
	const double a = f*r*sqrt(2.0) / 2.0;
	const double b = r * sqrt(2.0) / 2.0;
	MBNode& N1  = AddNode(vec3d( 0,  0,  0)); N1.SetID(4);
	MBNode& N2  = AddNode(vec3d( a,  0,  0));
	MBNode& N3  = AddNode(vec3d( a,  a,  0));
	MBNode& N4  = AddNode(vec3d( 0,  a,  0));
	MBNode& N5  = AddNode(vec3d(-a,  a,  0));
	MBNode& N6  = AddNode(vec3d(-a,  0,  0));
	MBNode& N7  = AddNode(vec3d(-a, -a,  0));
	MBNode& N8  = AddNode(vec3d( 0, -a,  0));
	MBNode& N9  = AddNode(vec3d( a, -a,  0));
	MBNode& N10 = AddNode(vec3d( r,  0,  0)); N10.SetID(0);
	MBNode& N11 = AddNode(vec3d( b,  b,  0));
	MBNode& N12 = AddNode(vec3d( 0,  r,  0)); N12.SetID(1);
	MBNode& N13 = AddNode(vec3d(-b,  b,  0));
	MBNode& N14 = AddNode(vec3d(-r,  0,  0)); N14.SetID(2);
	MBNode& N15 = AddNode(vec3d(-b, -b,  0));
	MBNode& N16 = AddNode(vec3d( 0, -r,  0)); N16.SetID(3);
	MBNode& N17 = AddNode(vec3d( b, -b,  0));

	// build the faces
	MBFace& F1  = AddFace(0,  1,  2, 3); F1.SetID(0); F1.SetSizes(nd, nd);
	MBFace& F2  = AddFace(0,  3,  4, 5); F2.SetID(1); F2.SetSizes(nd, nd);
	MBFace& F3  = AddFace(0,  5,  6, 7); F3.SetID(2); F3.SetSizes(nd, nd);
	MBFace& F4  = AddFace(0,  7,  8, 1); F4.SetID(3); F4.SetSizes(nd, nd);
	MBFace& F5  = AddFace(1,  9, 10, 2); F5.SetID(0); F5.SetSizes(ns, nd);
	MBFace& F6  = AddFace(2, 10, 11, 3); F6.SetID(0); F6.SetSizes(ns, nd);
	MBFace& F7  = AddFace(3, 11, 12, 4); F7.SetID(1); F7.SetSizes(ns, nd);
	MBFace& F8  = AddFace(4, 12, 13, 5); F8.SetID(1); F8.SetSizes(ns, nd);
	MBFace& F9  = AddFace(5, 13, 14, 6); F9.SetID(2); F9.SetSizes(ns, nd);
	MBFace& F10 = AddFace(6, 14, 15, 7); F10.SetID(2); F10.SetSizes(ns, nd);
	MBFace& F11 = AddFace(7, 15, 16, 8); F11.SetID(3); F11.SetSizes(ns, nd);
	MBFace& F12 = AddFace(8, 16,  9, 1); F12.SetID(3); F12.SetSizes(ns, nd);

	// build the MQ edges
	BuildMBEdges();

	// assign edge IDs
	SetFaceEdgeIDs( 0,  4, -1, -1,  5);
	SetFaceEdgeIDs( 1,  5, -1, -1,  6);
	SetFaceEdgeIDs( 2,  6, -1, -1,  7);
	SetFaceEdgeIDs( 3,  7, -1, -1,  4);
	SetFaceEdgeIDs( 4,  4,  0, -1, -1);
	SetFaceEdgeIDs( 5, -1,  0,  5, -1);
	SetFaceEdgeIDs( 6,  5,  1, -1, -1);
	SetFaceEdgeIDs( 7, -1,  1,  6, -1);
	SetFaceEdgeIDs( 8,  6,  2, -1, -1);
	SetFaceEdgeIDs( 9, -1,  2,  7, -1);
	SetFaceEdgeIDs(10,  7,  3, -1, -1);
	SetFaceEdgeIDs(11, -1,  3,  4, -1);

	// set edge types (default is line, so we only set the arcs)
	GetFaceEdge( 4, 1).m_ntype = EDGE_ZARC;
	GetFaceEdge( 5, 1).m_ntype = EDGE_ZARC;
	GetFaceEdge( 6, 1).m_ntype = EDGE_ZARC;
	GetFaceEdge( 7, 1).m_ntype = EDGE_ZARC;
	GetFaceEdge( 8, 1).m_ntype = EDGE_ZARC;
	GetFaceEdge( 9, 1).m_ntype = EDGE_ZARC;
	GetFaceEdge(10, 1).m_ntype = EDGE_ZARC;
	GetFaceEdge(11, 1).m_ntype = EDGE_ZARC;

	return true;
}
