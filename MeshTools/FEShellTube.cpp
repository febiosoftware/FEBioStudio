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

// FEShellTube.cpp: implementation of the FEShellTube class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FEShellTube.h"
#include <GeomLib/GPrimitive.h>
#include <MeshLib/FEMesh.h>
#include "FEMultiQuadMesh.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FEShellTube::FEShellTube(GThinTube* po)
{
	m_pobj = po;

	m_t = 0.01;
	m_nd = 16;
	m_nz = 16;

	AddDoubleParam(m_t, "t", "Thickness");
	AddIntParam(m_nd, "nd", "Divisions");
	AddIntParam(m_nz, "nz", "Stacks");
	AddChoiceParam(0, "elem_type", "Element Type")->SetEnumNames("QUAD4\0QUAD8\0QUAD9\0");
}

FEMesh* FEShellTube::BuildMesh()
{
	// get discretization parameters
	int nd = GetIntValue(NDIV);
	int nz = GetIntValue(NSTACK);

	// check parameters
	if (nd < 1) nd = 1;
	if (nz < 1) nz = 1;

	FEMultiQuadMesh MQ;
	MQ.Build(m_pobj);

	// set element type
	int elemType = GetIntValue(ELEM_TYPE);
	switch (elemType)
	{
	case 0: MQ.SetElementType(FE_QUAD4); break;
	case 1: MQ.SetElementType(FE_QUAD8); break;
	case 2: MQ.SetElementType(FE_QUAD9); break;
	};

	// set tesselation
	MQ.SetFaceSizes(0, nd, nz);
	MQ.SetFaceSizes(1, nd, nz);
	MQ.SetFaceSizes(2, nd, nz);
	MQ.SetFaceSizes(3, nd, nz);

	// Build the mesh
	FEMesh* pm = MQ.BuildMesh();
	if (pm == nullptr) return nullptr;

	// assign shell thickness
	double t = GetFloatValue(T);
	pm->SetUniformShellThickness(t);

	return pm;
}
