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

// FEShellPatch.cpp: implementation of the FEShellPatch class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FEShellPatch.h"
#include <GeomLib/GPrimitive.h>
#include <GeomLib/geom.h>
#include <MeshLib/FSMesh.h>
#include "FEMultiQuadMesh.h"

FEShellPatch::FEShellPatch()
{
	m_pobj = nullptr;

	m_t = 0.01;
	m_nx = m_ny = 10;

	AddDoubleParam(m_t, "t", "Thickness");
	AddIntParam(m_nx, "nx", "Nx");
	AddIntParam(m_ny, "ny", "Ny");
	AddChoiceParam(0, "elem_type", "Element Type")->SetEnumNames("QUAD4\0QUAD8\0QUAD9\0");
}

FSMesh* FEShellPatch::BuildMesh(GObject* po)
{
	m_pobj = dynamic_cast<GPatch*>(po);
	if (m_pobj == nullptr) return nullptr;

	// get mesh parameters
	m_nx = GetIntValue(NX);
	m_ny = GetIntValue(NY);
	int elemType = GetIntValue(ELEM_TYPE);

	BuildMultiQuad();

	// update the MB data
	switch (elemType)
	{
	case 0: SetElementType(FE_QUAD4); break;
	case 1: SetElementType(FE_QUAD8); break;
	case 2: SetElementType(FE_QUAD9); break;
	};

	// create the MB
	FSMesh* pm = FEMultiQuadMesh::BuildMQMesh();

	// assign shell thickness to section
	double t = GetFloatValue(T);
	GPart* part = m_pobj->Part(0); assert(part);
	GShellSection* shellSection = dynamic_cast<GShellSection*>(part->GetSection());
	if (shellSection) shellSection->SetShellThickness(t);
	else pm->SetUniformShellThickness(t);

	return pm;
}

bool FEShellPatch::BuildMultiQuad()
{
	ClearMQ();

	// create the MB nodes
	Build(m_pobj);
	SetFaceSizes(0, m_nx, m_ny);

	return true;
}

//////////////////////////////////////////////////////////////////////
// FECylndricalPatch
//////////////////////////////////////////////////////////////////////

FECylndricalPatch::FECylndricalPatch()
{
	m_pobj = nullptr;

	m_t = 0.01;
	m_nx = m_ny = 10;

	AddDoubleParam(m_t, "t", "Thickness");
	AddIntParam(m_nx, "nx", "Nx");
	AddIntParam(m_ny, "ny", "Ny");
	AddChoiceParam(0, "elem_type", "Element Type")->SetEnumNames("QUAD4\0QUAD8\0QUAD9\0");
}

FSMesh* FECylndricalPatch::BuildMesh(GObject* po)
{
	m_pobj = dynamic_cast<GCylindricalPatch*>(po);
	if (m_pobj == nullptr) return nullptr;

	if (BuildMultiQuad() == false) return nullptr;

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

	// assign shell thickness
	double t = GetFloatValue(T);
	pm->SetUniformShellThickness(t);

	return pm;
}

bool FECylndricalPatch::BuildMultiQuad()
{
	ClearMQ();

	// build the quad mesh data
	Build(m_pobj);

	// set sizes
	int nx = GetIntValue(NX);
	int ny = GetIntValue(NY);
	SetFaceSizes(0, nx, ny);

	return true;
}
