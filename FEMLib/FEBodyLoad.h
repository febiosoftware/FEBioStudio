#pragma once
#include "FELoad.h"

//=============================================================================
// Body loads
//=============================================================================
// Base class for all volumetric body loads
class FSBodyLoad : public FSLoad
{
public:
	FSBodyLoad(int ntype, FSModel* ps, int nstep);
};

//-----------------------------------------------------------------------------
// Constant Body Force (Obsolete)
class FSConstBodyForce : public FSBodyLoad
{
public:
	enum { FORCE_X, FORCE_Y, FORCE_Z };

	FELoadCurve* GetLoadCurve(int n);

	double GetLoad(int n);

	void SetLoad(int n, double v);

public:
	FSConstBodyForce(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
// Non-constant Body Force (Obsolete)
class FENonConstBodyForce : public FSBodyLoad
{
public:
	enum { FORCE_X, FORCE_Y, FORCE_Z };

	FELoadCurve* GetLoadCurve(int n);

public:
	FENonConstBodyForce(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
// Heat Source
class FEHeatSource : public FSBodyLoad
{
public:
	enum { LOAD };

public:
	FEHeatSource(FSModel* ps, int nstep = 0);

	FELoadCurve* GetLoadCurve() { return GetParamLC(LOAD); }

	double GetLoad() { return GetFloatValue(LOAD); }
	void SetLoad(double v) { SetFloatValue(LOAD, v); }
};

//-----------------------------------------------------------------------------
// SBM source (experimental feature)
class FESBMPointSource : public FSBodyLoad
{
public:
	enum { SBM, VALUE, POS_X, POS_Y, POS_Z };

public:
	FESBMPointSource(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
// Centrifugal body force
class FECentrifugalBodyForce : public FSBodyLoad
{
public:
    enum { ANGSPD, ROT_AXIS, ROT_CNTR };
    
public:
    FECentrifugalBodyForce(FSModel* ps, int nstep = 0);
    
    FELoadCurve* GetLoadCurve() { return GetParamLC(ANGSPD); }
    
    double GetLoad() { return GetFloatValue(ANGSPD); }
    void SetLoad(double v) { SetFloatValue(ANGSPD, v); }
};

//-----------------------------------------------------------------------------
// mass damping "load"
class FEMassDamping : public FSBodyLoad
{
public:
	enum { C };

public:
	FEMassDamping(FSModel* ps, int nstep = 0);

	FELoadCurve* GetLoadCurve() { return GetParamLC(C); }

	double GetLoad() { return GetFloatValue(C); }
	void SetLoad(double v) { SetFloatValue(C, v); }
};
//-------------------------------------------------------------------------------
class FEBioBodyLoad : public FSBodyLoad
{
public:
	FEBioBodyLoad(FSModel* ps, int nstep = 0);
	void Save(OArchive& ar);
	void Load(IArchive& ar);
};
