#pragma once
#include <MeshLib/FEMesh.h>
#include "FEModifier.h"

//-------------------------------------------------------------------
// Class that partitions a mesh based on connectivity and curvature
class FEAutoPartition : public FEModifier
{
public:
	FEAutoPartition();

	// set/get smoothing angle
	void SetSmoothingAngle(double w);
	double GetSmoothingAngle();

	FEMesh* Apply(FEMesh* pm);
};
