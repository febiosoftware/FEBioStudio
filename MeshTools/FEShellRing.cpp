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

// FEShellRing.cpp: implementation of the FEShellRing class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FEShellRing.h"
#include <GeomLib/GPrimitive.h>
#include <MeshLib/FSMesh.h>
#include "FEMultiQuadMesh.h"

FEShellRing::FEShellRing()
{
	m_pobj = nullptr;

	m_t = 0.01;
	m_ns = 16;
	m_nr = 8;

	AddDoubleParam(m_t, "t", "Thickness");
	AddIntParam(m_ns, "ns", "Slices");
	AddIntParam(m_nr, "nr", "Division");
	AddChoiceParam(0, "elem_type", "Element Type")->SetEnumNames("QUAD4\0QUAD8\0QUAD9\0");
}

FSMesh* FEShellRing::BuildMesh(GObject* po)
{
	m_pobj = dynamic_cast<GRing*>(po);
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

	// assign shell thickness to section
	double t = GetFloatValue(T);
	GPart* part = m_pobj->Part(0); assert(part);
	GShellSection* shellSection = dynamic_cast<GShellSection*>(part->GetSection());
	if (shellSection) shellSection->SetShellThickness(t);
	else pm->SetUniformShellThickness(t);

	return pm;
}

bool FEShellRing::BuildMultiQuad()
{
	ClearMQ();

	// build the mesh data structures
	Build(m_pobj);

	// set discretization
	int nd = GetIntValue(NDIV); if (nd < 1) nd = 1;
	int ns = GetIntValue(NSLICE); if (ns < 1) ns = 1;
	SetFaceSizes(0, ns, nd);
	SetFaceSizes(1, ns, nd);
	SetFaceSizes(2, ns, nd);
	SetFaceSizes(3, ns, nd);

	return true;
}
