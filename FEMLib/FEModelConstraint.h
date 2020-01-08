#pragma once
#include "FEModelComponent.h"

class FEModelConstraint : public FEModelComponent
{
public:
	FEModelConstraint(int ntype, FEModel* fem, int nstep = 0);
};

//-----------------------------------------------------------------------------
// This class implements a volume constraint
class FEVolumeConstraint : public FEModelConstraint
{
public:
	enum { LAUGON, ALTOL, PENALTY };

public:
	FEVolumeConstraint(FEModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
// This class implements a normal fluid flow constraint
class FENormalFlowSurface : public FEModelConstraint
{
public:
	enum { LAUGON, ALTOL, PENALTY, MINAUG, MAXAUG, RHS };

public:
	FENormalFlowSurface(FEModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
// This class implements a symmetry plane constraint
class FESymmetryPlane : public FEModelConstraint
{
public:
	enum { LAUGON, ALTOL, PENALTY, MINAUG, MAXAUG };

public:
	FESymmetryPlane(FEModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FEWarpingConstraint : public FEModelConstraint
{
public:
	FEWarpingConstraint(FEModel* fem);
};
