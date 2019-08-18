#pragma once
#include "FEModifier.h"

class FEShellToSolid : public FEModifier
{
public:
	FEShellToSolid();

	FEMesh* Apply(FEMesh* pm);
};
