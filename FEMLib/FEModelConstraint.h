#pragma once
#include "FEModelComponent.h"

class FEModelConstraint : public FEModelComponent
{
public:
	FEModelConstraint(int ntype, FEModel* fem, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FESurfaceConstraint : public FEModelConstraint
{
public:
	FESurfaceConstraint(int ntype, FEModel* fem, int nstep = 0);
};

//-----------------------------------------------------------------------------
// This class implements a volume constraint
class FEVolumeConstraint : public FESurfaceConstraint
{
public:
	enum { LAUGON, ALTOL, PENALTY };

public:
	FEVolumeConstraint(FEModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
// This class implements a normal fluid flow constraint
class FENormalFlowSurface : public FESurfaceConstraint
{
public:
	enum { LAUGON, ALTOL, PENALTY, MINAUG, MAXAUG, RHS };

public:
	FENormalFlowSurface(FEModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
// This class implements a symmetry plane constraint
class FESymmetryPlane : public FESurfaceConstraint
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

//-----------------------------------------------------------------------------
// This class implements a frictionless fluid wall constraint
class FEFrictionlessFluidWall : public FESurfaceConstraint
{
public:
    enum { LAUGON, ALTOL, PENALTY, MINAUG, MAXAUG };

public:
    FEFrictionlessFluidWall(FEModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FEPrestrainConstraint : public FEModelConstraint
{
public:
	FEPrestrainConstraint(FEModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FEInSituStretchConstraint : public FEModelConstraint
{
public:
	FEInSituStretchConstraint(FEModel* ps, int nstep = 0);
};
