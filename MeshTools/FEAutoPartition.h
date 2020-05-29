#pragma once
#include <MeshLib/FEMesh.h>
#include "FEModifier.h"

//-------------------------------------------------------------------
// Class that partitions a mesh based on connectivity and curvature
class FEAutoPartition : public FEModifier
{
public:
	FEAutoPartition();
	FEMesh* Apply(FEGroup* pg);
	FEMesh* Apply(FEMesh* pm);
};

class FERebuildMesh : public FEModifier
{
public:
	FERebuildMesh();
	FEMesh* Apply(FEMesh* pm);
};
