#pragma once
#include "FEModifier.h"

//-----------------------------------------------------------------------------
// Convert a linear mesh to a quadratic mesh
class FELinearToQuadratic : public FEModifier
{
public:
	FELinearToQuadratic();
	FEMesh* Apply(FEMesh* pm);
	void SetSmoothing(bool b) { m_bsmooth = b; }

protected:
	bool m_bsmooth;
};

//-----------------------------------------------------------------------------
// Convert a quadratic solid mesh to a linear solid mesh
class FEQuadraticToLinear : public FEModifier
{
public:
	FEQuadraticToLinear() : FEModifier("Quadratic-to-Linear"){}
	FEMesh* Apply(FEMesh* pm);
};

//-----------------------------------------------------------------------------
// helper class for smoothing a solid quadratic mesh
class FESolidSmooth
{
public:
	void Apply(FEMesh* pm);
};
