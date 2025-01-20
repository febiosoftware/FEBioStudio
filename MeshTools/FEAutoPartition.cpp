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
#include "FEAutoPartition.h"
#include <MeshLib/FSMeshBuilder.h>

//-----------------------------------------------------------------------------
FEAutoPartition::FEAutoPartition() : FEModifier("Auto Partition")
{
	AddDoubleParam(30.0, "Crease angle", "Crease angle (degrees)");
}

//-----------------------------------------------------------------------------
FSMesh* FEAutoPartition::Apply(FSMesh* pm)
{
	double w = GetFloatValue(0);
	FSMesh* newMesh = new FSMesh(*pm);
	FSMeshBuilder meshBuilder(*newMesh);
	meshBuilder.AutoPartition(w);
	return newMesh;
}

//-----------------------------------------------------------------------------
FSMesh* FEAutoPartition::Apply(FSGroup* pg)
{
	if (pg == nullptr) return nullptr;
	FSMesh* pm = pg->GetMesh();
	if (pm == nullptr) return nullptr;

	double w = GetFloatValue(0);
	FSMesh* newMesh = new FSMesh(*pm);
	FSMeshBuilder meshBuilder(*newMesh);

	if (dynamic_cast<FSEdgeSet*>(pg))
	{
		if (meshBuilder.AutoPartitionEdges(w, dynamic_cast<FSEdgeSet*>(pg)) == false)
		{
			delete newMesh;
			newMesh = nullptr;
			SetError("Cannot auto-partition this edge selection.");
		}
	}
	else if (dynamic_cast<FSSurface*>(pg))
	{
		if (meshBuilder.AutoPartitionFaces(w, dynamic_cast<FSSurface*>(pg)) == false)
		{
			delete newMesh;
			newMesh = nullptr;
			SetError("Cannot auto-partition this face selection.");
		}
	}
	else meshBuilder.AutoPartition(w);

	return newMesh;
}

//-----------------------------------------------------------------------------
FERebuildMesh::FERebuildMesh() : FEModifier("Rebuild mesh")
{
	AddBoolParam(false, "Re-partition elements");
	AddDoubleParam(30.0, "Crease angle", "Crease angle (degrees)");
}

//-----------------------------------------------------------------------------
FSMesh* FERebuildMesh::Apply(FSMesh* pm)
{
	bool repartition = GetBoolValue(0);
	double w = GetFloatValue(1);

	FSMesh* newMesh = new FSMesh(*pm);
	FSMeshBuilder meshBuilder(*newMesh);
	meshBuilder.RebuildMesh(w, repartition);
	return newMesh;
}
