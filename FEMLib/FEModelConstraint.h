#pragma once
#include "FEDomainComponent.h"

class FSModelConstraint : public FSDomainComponent
{
public:
	FSModelConstraint(int ntype, FSModel* fem, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FSSurfaceConstraint : public FSModelConstraint
{
public:
	FSSurfaceConstraint(int ntype, FSModel* fem, int nstep = 0);
};

//-----------------------------------------------------------------------------
// This class implements a volume constraint
class FSVolumeConstraint : public FSSurfaceConstraint
{
public:
	enum { LAUGON, ALTOL, PENALTY };

public:
	FSVolumeConstraint(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
// This class implements a normal fluid flow constraint
class FSNormalFlowSurface : public FSSurfaceConstraint
{
public:
	enum { LAUGON, ALTOL, PENALTY, MINAUG, MAXAUG, RHS };

public:
	FSNormalFlowSurface(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
// This class implements a symmetry plane constraint
class FSSymmetryPlane : public FSSurfaceConstraint
{
public:
	enum { LAUGON, ALTOL, PENALTY, MINAUG, MAXAUG };

public:
	FSSymmetryPlane(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FSWarpingConstraint : public FSModelConstraint
{
public:
	FSWarpingConstraint(FSModel* fem);
};

//-----------------------------------------------------------------------------
// This class implements a frictionless fluid wall constraint
class FSFrictionlessFluidWall : public FSSurfaceConstraint
{
public:
    enum { LAUGON, ALTOL, PENALTY, MINAUG, MAXAUG };

public:
    FSFrictionlessFluidWall(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FSPrestrainConstraint : public FSModelConstraint
{
public:
	FSPrestrainConstraint(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FSInSituStretchConstraint : public FSModelConstraint
{
public:
	FSInSituStretchConstraint(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FEBioNLConstraint : public FSModelConstraint
{
public:
	FEBioNLConstraint(FSModel* fem, int nstep = 0);
	void Save(OArchive& ar);
	void Load(IArchive& ar);
};
