#pragma once
#include "FEModifier.h"

class FEDiscardMesh : public FEModifier
{
public:
	FEDiscardMesh();

	FEMesh* Apply(FEMesh* pm);
};
