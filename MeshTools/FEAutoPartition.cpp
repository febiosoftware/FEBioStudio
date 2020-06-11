/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
#include <MeshLib/FEMeshBuilder.h>

//-----------------------------------------------------------------------------
FEAutoPartition::FEAutoPartition() : FEModifier("Auto Partition")
{
	AddDoubleParam(30.0, "Crease angle", "Crease angle (degrees)");
}

//-----------------------------------------------------------------------------
FEMesh* FEAutoPartition::Apply(FEMesh* pm)
{
	double w = GetFloatValue(0);
	FEMesh* newMesh = new FEMesh(*pm);
	FEMeshBuilder meshBuilder(*newMesh);
	meshBuilder.AutoPartition(w);
	return newMesh;
}

//-----------------------------------------------------------------------------
FEMesh* FEAutoPartition::Apply(FEGroup* pg)
{
	if (pg == nullptr) return nullptr;
	FEMesh* pm = pg->GetMesh();
	if (pm == nullptr) return nullptr;

	double w = GetFloatValue(0);
	FEMesh* newMesh = new FEMesh(*pm);
	FEMeshBuilder meshBuilder(*newMesh);

	if (dynamic_cast<FEEdgeSet*>(pg))
	{
		if (meshBuilder.AutoPartitionEdges(w, dynamic_cast<FEEdgeSet*>(pg)) == false)
		{
			delete newMesh;
			newMesh = nullptr;
			SetError("Cannot auto-partition this edge selection.");
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
FEMesh* FERebuildMesh::Apply(FEMesh* pm)
{
	bool repartition = GetBoolValue(0);
	double w = GetFloatValue(1);

	FEMesh* newMesh = new FEMesh(*pm);
	FEMeshBuilder meshBuilder(*newMesh);
	meshBuilder.RebuildMesh(w, repartition);
	return newMesh;
}
