#pragma once
#include "FESurfaceModifier.h"

//-----------------------------------------------------------------------------
class FESmoothSurfaceMesh : public FESurfaceModifier
{
public:
	FESmoothSurfaceMesh();
	FESurfaceMesh* Apply(FESurfaceMesh* pm);

public:
	void ShapeSmoothMesh(FESurfaceMesh& mesh, const FESurfaceMesh& backMesh, bool preserveShape = false, bool preserveEdges = false);
};
