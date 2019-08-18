#pragma once
#include "FESurfaceModifier.h"

class FERefineSurface : public FESurfaceModifier
{
public:
	FERefineSurface();

	void SetIterations(int n);

	FESurfaceMesh* Apply(FESurfaceMesh* pm);

protected:
	FESurfaceMesh* Split(FESurfaceMesh* pm);
};
