#include "stdafx.h"
#include "FEAutoPartition.h"

//-----------------------------------------------------------------------------
FEAutoPartition::FEAutoPartition() : FEModifier("Auto Partition")
{
	AddBoolParam(false, "Rebuild", "Rebuild mesh");
	AddBoolParam(false, "Re-partition elements");
	AddDoubleParam(30.0, "Crease angle:", "Crease angle (degrees):");
}

//-----------------------------------------------------------------------------
// set/get smoothing angle
void FEAutoPartition::SetSmoothingAngle(double w)
{
	SetFloatValue(2, w);
}

double FEAutoPartition::GetSmoothingAngle()
{
	return GetFloatValue(2);
}

//-----------------------------------------------------------------------------
FEMesh* FEAutoPartition::Apply(FEMesh* pm)
{
	bool rebuildMesh = GetBoolValue(0);
	bool repartition = GetBoolValue(1);
	double w = GetFloatValue(2);

	FEMesh* newMesh = new FEMesh(*pm);

	if (repartition)
		newMesh->AutoPartitionElements();

	if (rebuildMesh)
		newMesh->RebuildMesh(w);
	else
		newMesh->AutoPartition(w);

	return newMesh;
}
