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
#include <MeshLib/FEMesh.h>
#include "FEMultiQuadMesh.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FEShellPatch::FEShellPatch(GPatch* po)
{
	m_pobj = po;

	m_t = 0.01;
	m_nx = m_ny = 10;

	AddDoubleParam(m_t, "t", "Thickness");
	AddIntParam(m_nx, "nx", "Nx");
	AddIntParam(m_ny, "ny", "Ny");
	AddChoiceParam(0, "elem_type", "Element Type")->SetEnumNames("QUAD4\0QUAD8\0QUAD9\0");
}

FSMesh* FEShellPatch::BuildMesh()
{
	// get mesh parameters
	m_nx = GetIntValue(NX);
	m_ny = GetIntValue(NY);
	int elemType = GetIntValue(ELEM_TYPE);

	// create the MB nodes
	FEMultiQuadMesh MQ;
	MQ.Build(m_pobj);

	MQ.SetFaceSizes(0, m_nx, m_ny);

	// update the MB data
	switch (elemType)
	{
	case 0: MQ.SetElementType(FE_QUAD4); break;
	case 1: MQ.SetElementType(FE_QUAD8); break;
	case 2: MQ.SetElementType(FE_QUAD9); break;
	};
	
	MQ.UpdateMQ();

	// create the MB
	FSMesh* pm = MQ.BuildMesh();

	// assign shell thickness
	double t = GetFloatValue(T);
	pm->SetUniformShellThickness(t);

	return pm;
}

//////////////////////////////////////////////////////////////////////
// FECylndricalPatch
//////////////////////////////////////////////////////////////////////

FECylndricalPatch::FECylndricalPatch(GCylindricalPatch* po)
{
	m_pobj = po;

	m_t = 0.01;
	m_nx = m_ny = 10;

	AddDoubleParam(m_t, "t", "Thickness");
	AddIntParam(m_nx, "nx", "Nx");
	AddIntParam(m_ny, "ny", "Ny");
	AddChoiceParam(0, "elem_type", "Element Type")->SetEnumNames("QUAD4\0QUAD8\0QUAD9\0");
}

FSMesh* FECylndricalPatch::BuildMesh()
{
	return BuildMultiQuadMesh();
}

FSMesh* FECylndricalPatch::BuildMultiQuadMesh()
{
	// build the quad mesh data
	FEMultiQuadMesh MQ;
	MQ.Build(m_pobj);

	// set sizes
	int nx = GetIntValue(NX);
	int ny = GetIntValue(NY);
	MQ.SetFaceSizes(0, nx, ny);

	int elemType = GetIntValue(ELEM_TYPE);
	switch (elemType)
	{
	case 0: MQ.SetElementType(FE_QUAD4); break;
	case 1: MQ.SetElementType(FE_QUAD8); break;
	case 2: MQ.SetElementType(FE_QUAD9); break;
	};

	// Build the mesh
	FSMesh* pm = MQ.BuildMesh();
	if (pm == nullptr) return nullptr;

	// assign shell thickness
	double t = GetFloatValue(T);
	pm->SetUniformShellThickness(t);

	return pm;
}
