#pragma once
#include "FEModifier.h"

//-----------------------------------------------------------------------------
class FERevolveFaces : public FEModifier
{
public:
	FERevolveFaces();
	FEMesh* Apply(FEMesh* pm);

protected:	
	FEMesh* RevolveSolidMesh(FEMesh* pm);
	FEMesh* RevolveShellMesh(FEMesh* pm);
};
