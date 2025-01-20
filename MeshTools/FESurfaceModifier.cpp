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

#include "stdafx.h"
#include "FESurfaceModifier.h"
#include <MeshLib/FSSurfaceMesh.h>
#include <GeomLib/FSGroup.h>
#include <stdarg.h>

FESurfaceModifier::FESurfaceModifier(const std::string& name)
{ 
	SetName(name);
}

FESurfaceModifier::~FESurfaceModifier() {}

bool FESurfaceModifier::SetError(const char* szerr, ...)
{
	// get a pointer to the argument list
	va_list	args;

	// copy to string
	char sz[256] = { 0 };
	va_start(args, szerr);
	vsprintf(sz, szerr, args);
	va_end(args);

	if (m_error.empty() == false) m_error += "\n";
	m_error += std::string(sz);
	return false;
}

void FESurfaceModifier::ClearError()
{
	m_error = "";
}

std::string FESurfaceModifier::GetErrorString()
{
	return m_error;
}

//=============================================================================

FESurfacePartitionSelection::FESurfacePartitionSelection()
{
	m_partition = -1;
}

void FESurfacePartitionSelection::assignToPartition(int n)
{
	m_partition = n;
}


FSSurfaceMesh* FESurfacePartitionSelection::Apply(FSSurfaceMesh* pm, FSGroup* pg)
{
	if (dynamic_cast<FSEdgeSet*>(pg))
	{
		FSSurfaceMesh* newMesh = new FSSurfaceMesh(*pm);
		newMesh->PartitionEdgeSelection(m_partition);
		return newMesh;
	}
	else if (dynamic_cast<FSNodeSet*>(pg))
	{
		FSSurfaceMesh* newMesh = new FSSurfaceMesh(*pm);
		newMesh->PartitionNodeSelection();
		return newMesh;
	}
	else
	{
		int n = pm->CountSelectedFaces();
		if (n > 0)
		{
			FSSurfaceMesh* newMesh = new FSSurfaceMesh(*pm);
			PartitionSelectedFaces(newMesh);
			return newMesh;
		}
	}

	return 0;
}

void FESurfacePartitionSelection::PartitionSelectedFaces(FSSurfaceMesh* mesh)
{
	int NF = mesh->Faces();
	int ng = mesh->CountFacePartitions();
	for (int i = 0; i < NF; ++i)
	{
		FSFace& f = mesh->Face(i);
		if (f.IsSelected())
		{
			f.m_gid = ng;
		}
	}
	mesh->BuildMesh();
}

//=============================================================================
FESurfaceAutoPartition::FESurfaceAutoPartition() : FESurfaceModifier("Auto Partition")
{
	AddDoubleParam(30.0, "Crease angle", "Crease angle (degrees)");
}

//-----------------------------------------------------------------------------
FSSurfaceMesh* FESurfaceAutoPartition::Apply(FSSurfaceMesh* pm)
{
	double w = GetFloatValue(0);
	FSSurfaceMesh* newMesh = new FSSurfaceMesh(*pm);
	newMesh->AutoPartition(w);
	return newMesh;
}
