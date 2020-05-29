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
