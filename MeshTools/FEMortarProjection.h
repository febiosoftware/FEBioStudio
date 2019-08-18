#pragma once
#include <MeshLib/FEMesh.h>

class FEMortarProjection
{
public:
	FEMortarProjection();

	// calculate a surface mesh that is the mortar surface
	FEMesh* Apply(FESurface* pslave, FESurface* pmaster);
};
