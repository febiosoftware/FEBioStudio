#pragma once
#include "FEDomainComponent.h"

class FEModelConstraint : public FSDomainComponent
{
public:
	FEModelConstraint(int ntype, FSModel* fem, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FESurfaceConstraint : public FEModelConstraint
{
public:
	FESurfaceConstraint(int ntype, FSModel* fem, int nstep = 0);
};

//-----------------------------------------------------------------------------
// This class implements a volume constraint
class FEVolumeConstraint : public FESurfaceConstraint
{
public:
	enum { LAUGON, ALTOL, PENALTY };

public:
	FEVolumeConstraint(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
// This class implements a normal fluid flow constraint
class FENormalFlowSurface : public FESurfaceConstraint
{
public:
	enum { LAUGON, ALTOL, PENALTY, MINAUG, MAXAUG, RHS };

public:
	FENormalFlowSurface(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
// This class implements a symmetry plane constraint
class FESymmetryPlane : public FESurfaceConstraint
{
public:
	enum { LAUGON, ALTOL, PENALTY, MINAUG, MAXAUG };

public:
	FESymmetryPlane(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FEWarpingConstraint : public FEModelConstraint
{
public:
	FEWarpingConstraint(FSModel* fem);
};

//-----------------------------------------------------------------------------
// This class implements a frictionless fluid wall constraint
class FEFrictionlessFluidWall : public FESurfaceConstraint
{
public:
    enum { LAUGON, ALTOL, PENALTY, MINAUG, MAXAUG };

public:
    FEFrictionlessFluidWall(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FEPrestrainConstraint : public FEModelConstraint
{
public:
	FEPrestrainConstraint(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FEInSituStretchConstraint : public FEModelConstraint
{
public:
	FEInSituStretchConstraint(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FEBioNLConstraint : public FEModelConstraint
{
public:
	FEBioNLConstraint(FSModel* fem, int nstep = 0);
	void Save(OArchive& ar);
	void Load(IArchive& ar);
};
