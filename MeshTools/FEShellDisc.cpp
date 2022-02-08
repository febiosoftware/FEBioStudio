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

FEShellDisc::FEShellDisc(GDisc* po)
{
	m_pobj = po;

	m_r = 0.5;
	m_t = 0.01;
	m_nd = m_nr = 6;

	AddDoubleParam(m_r, "r", "Ratio");
	AddDoubleParam(m_t, "t", "Thickness");
	AddIntParam(m_nd, "nd", "Divisions");
	AddIntParam(m_nr, "nr", "Segments");
	AddChoiceParam(0, "elem_type", "Element Type")->SetEnumNames("QUAD4\0QUAD8\0QUAD9\0");
}

FEMesh* FEShellDisc::BuildMesh()
{
	FEMultiQuadMesh MQ;

	double r = m_pobj->GetFloatValue(GDisc::RADIUS);
	double f = GetFloatValue(RATIO);
	double h = GetFloatValue(T);
	int nd = GetIntValue(NDIV);
	int ns = GetIntValue(NSEG);

	// position the nodes
	const double a = f*r*sqrt(2.0) / 2.0;
	const double b = r * sqrt(2.0) / 2.0;
	MBNode& N1  = MQ.AddNode(vec3d( 0,  0,  0)); N1.SetID(4);
	MBNode& N2  = MQ.AddNode(vec3d( a,  0,  0));
	MBNode& N3  = MQ.AddNode(vec3d( a,  a,  0));
	MBNode& N4  = MQ.AddNode(vec3d( 0,  a,  0));
	MBNode& N5  = MQ.AddNode(vec3d(-a,  a,  0));
	MBNode& N6  = MQ.AddNode(vec3d(-a,  0,  0));
	MBNode& N7  = MQ.AddNode(vec3d(-a, -a,  0));
	MBNode& N8  = MQ.AddNode(vec3d( 0, -a,  0));
	MBNode& N9  = MQ.AddNode(vec3d( a, -a,  0));
	MBNode& N10 = MQ.AddNode(vec3d( r,  0,  0)); N10.SetID(0);
	MBNode& N11 = MQ.AddNode(vec3d( b,  b,  0));
	MBNode& N12 = MQ.AddNode(vec3d( 0,  r,  0)); N12.SetID(1);
	MBNode& N13 = MQ.AddNode(vec3d(-b,  b,  0));
	MBNode& N14 = MQ.AddNode(vec3d(-r,  0,  0)); N14.SetID(2);
	MBNode& N15 = MQ.AddNode(vec3d(-b, -b,  0));
	MBNode& N16 = MQ.AddNode(vec3d( 0, -r,  0)); N16.SetID(3);
	MBNode& N17 = MQ.AddNode(vec3d( b, -b,  0));

	// build the faces
	MBFace& F1  = MQ.AddFace(0,  1,  2, 3); F1.SetID(0); F1.SetSizes(nd, nd);
	MBFace& F2  = MQ.AddFace(0,  3,  4, 5); F2.SetID(1); F2.SetSizes(nd, nd);
	MBFace& F3  = MQ.AddFace(0,  5,  6, 7); F3.SetID(2); F3.SetSizes(nd, nd);
	MBFace& F4  = MQ.AddFace(0,  7,  8, 1); F4.SetID(3); F4.SetSizes(nd, nd);
	MBFace& F5  = MQ.AddFace(1,  9, 10, 2); F5.SetID(0); F5.SetSizes(ns, nd);
	MBFace& F6  = MQ.AddFace(2, 10, 11, 3); F6.SetID(0); F6.SetSizes(ns, nd);
	MBFace& F7  = MQ.AddFace(3, 11, 12, 4); F7.SetID(1); F7.SetSizes(ns, nd);
	MBFace& F8  = MQ.AddFace(4, 12, 13, 5); F8.SetID(1); F8.SetSizes(ns, nd);
	MBFace& F9  = MQ.AddFace(5, 13, 14, 6); F9.SetID(2); F9.SetSizes(ns, nd);
	MBFace& F10 = MQ.AddFace(6, 14, 15, 7); F10.SetID(2); F10.SetSizes(ns, nd);
	MBFace& F11 = MQ.AddFace(7, 15, 16, 8); F11.SetID(3); F11.SetSizes(ns, nd);
	MBFace& F12 = MQ.AddFace(8, 16,  9, 1); F12.SetID(3); F12.SetSizes(ns, nd);

	// build the MQ structure
	MQ.UpdateMQ();

	// assign edge IDs
	MQ.SetFaceEdgeIDs( 0,  4, -1, -1,  5);
	MQ.SetFaceEdgeIDs( 1,  5, -1, -1,  6);
	MQ.SetFaceEdgeIDs( 2,  6, -1, -1,  7);
	MQ.SetFaceEdgeIDs( 3,  7, -1, -1,  4);
	MQ.SetFaceEdgeIDs( 4,  4,  0, -1, -1);
	MQ.SetFaceEdgeIDs( 5, -1,  0,  5, -1);
	MQ.SetFaceEdgeIDs( 6,  5,  1, -1, -1);
	MQ.SetFaceEdgeIDs( 7, -1,  1,  6, -1);
	MQ.SetFaceEdgeIDs( 8,  6,  2, -1, -1);
	MQ.SetFaceEdgeIDs( 9, -1,  2,  7, -1);
	MQ.SetFaceEdgeIDs(10,  7,  3, -1, -1);
	MQ.SetFaceEdgeIDs(11, -1,  3,  4, -1);

	// set edge types (default is line, so we only set the arcs)
	MQ.GetFaceEdge( 4, 1).m_ntype = EDGE_ZARC;
	MQ.GetFaceEdge( 5, 1).m_ntype = EDGE_ZARC;
	MQ.GetFaceEdge( 6, 1).m_ntype = EDGE_ZARC;
	MQ.GetFaceEdge( 7, 1).m_ntype = EDGE_ZARC;
	MQ.GetFaceEdge( 8, 1).m_ntype = EDGE_ZARC;
	MQ.GetFaceEdge( 9, 1).m_ntype = EDGE_ZARC;
	MQ.GetFaceEdge(10, 1).m_ntype = EDGE_ZARC;
	MQ.GetFaceEdge(11, 1).m_ntype = EDGE_ZARC;

	// set element type
	int elemType = GetIntValue(ELEM_TYPE);
	switch (elemType)
	{
	case 0: MQ.SetElementType(FE_QUAD4); break;
	case 1: MQ.SetElementType(FE_QUAD8); break;
	case 2: MQ.SetElementType(FE_QUAD9); break;
	};

	// Build the mesh
	FEMesh* pm = MQ.BuildMesh();
	if (pm == nullptr) return nullptr;

	pm->BuildMesh();

	// assign shell thickness
	pm->SetUniformShellThickness(h);

	return pm;
}
