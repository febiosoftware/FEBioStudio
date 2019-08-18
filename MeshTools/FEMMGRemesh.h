#pragma once
#include "FEModifier.h"

//-----------------------------------------------------------------------------
//! This modifier refines a tet mesh using MMG.
class FEMMGRemesh : public FEModifier
{
	enum {
		ELEM_SIZE,
		HMIN,
		HAUSDORFF,
		HGRAD
	};

public:
	FEMMGRemesh();
	FEMesh* Apply(FEMesh* pm);
};
