#pragma once
#include "FEDomainComponent.h"

class FSModelConstraint : public FSDomainComponent
{
public:
	FSModelConstraint(int ntype, FSModel* fem, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FESurfaceConstraint : public FSModelConstraint
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
class FEWarpingConstraint : public FSModelConstraint
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
class FEPrestrainConstraint : public FSModelConstraint
{
public:
	FEPrestrainConstraint(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FEInSituStretchConstraint : public FSModelConstraint
{
public:
	FEInSituStretchConstraint(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FEBioNLConstraint : public FSModelConstraint
{
public:
	FEBioNLConstraint(FSModel* fem, int nstep = 0);
	void Save(OArchive& ar);
	void Load(IArchive& ar);
};
