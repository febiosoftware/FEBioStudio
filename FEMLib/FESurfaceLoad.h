#pragma once
#include "FELoad.h"

//=============================================================================
// Base class for surface loads.
//
class FSSurfaceLoad : public FSLoad
{
public:
    FSSurfaceLoad(int ntype, FSModel* ps);
    FSSurfaceLoad(int ntype, FSModel* ps, FSItemListBuilder* pi, int nstep);
};

//-----------------------------------------------------------------------------
// Surface pressure boundary conditions
//
class FSPressureLoad : public FSSurfaceLoad
{
public:
	enum { LOAD, NTYPE };

public:
	FSPressureLoad(FSModel* ps, FSItemListBuilder* pi = 0, int nstep = 0);

	void SetLinearFlag(bool b) { SetBoolValue(NTYPE, b); }
	bool GetLinearFlag() { return GetBoolValue(NTYPE); }

	void SetLoad(double f) { SetFloatValue(LOAD, f); }
	double GetLoad() { return GetFloatValue(LOAD); }

	// used only for reading parameters for old file formats
	void LoadParam(const Param& p);
};

//-----------------------------------------------------------------------------
// Fluid flux surface boundary load
//
class FSFluidFlux : public FSSurfaceLoad
{
public:
	enum { LOAD, NTYPE, NFLUX };

public:
	FSFluidFlux(FSModel* ps, FSItemListBuilder* pi = 0, int nstep = 0);

	void SetLoad(double f) { SetFloatValue(LOAD, f); }
	double GetLoad() { return GetFloatValue(LOAD); }

	void SetLinearFlag(bool b) { SetBoolValue(NTYPE, b); }
	bool GetLinearFlag() { return GetBoolValue(NTYPE); }

	void SetMixtureFlag(bool b) { SetBoolValue(NFLUX, b); }
	bool GetMixtureFlag() { return GetBoolValue(NFLUX); }

	// used only for reading parameters for old file formats
	void LoadParam(const Param& p);
};

//-----------------------------------------------------------------------------
// Mixture Normal Traction surface boundary load
//
class FSBPNormalTraction : public FSSurfaceLoad
{
public:
	enum { LOAD, NTYPE, NTRAC };

public:
	FSBPNormalTraction(FSModel* ps, FSItemListBuilder* pi = 0, int nstep = 0);

	void SetLoad(double f) { SetFloatValue(LOAD, f); }
	double GetLoad() { return GetFloatValue(LOAD); }

	void SetLinearFlag(bool b) { SetBoolValue(NTYPE, b); }
	bool GetLinearFlag() { return GetBoolValue(NTYPE); }

	void SetMixtureFlag(bool b) { SetBoolValue(NTRAC, b); }
	bool GetMixtureFlag() { return GetBoolValue(NTRAC); }

	// used only for reading parameters for old file formats
	void LoadParam(const Param& p);
};

//-----------------------------------------------------------------------------
// Solute flux surface boundary load
//
class FSSoluteFlux : public FSSurfaceLoad
{
public:
	enum { LOAD, NTYPE, BC };

public:
	FSSoluteFlux(FSModel* ps, FSItemListBuilder* pi = 0, int nstep = 0);

	void SetLoad(double f) { SetFloatValue(LOAD, f); }
	double GetLoad() { return GetFloatValue(LOAD); }

	void SetLinearFlag(bool b) { SetBoolValue(NTYPE, b); }
	bool GetLinearFlag() { return GetBoolValue(NTYPE); }

	int GetBC() { return GetIntValue(BC); }
	void SetBC(int n) { SetIntValue(BC, n); }

	// used only for reading parameters for old file formats
	void LoadParam(const Param& p);
};

//-----------------------------------------------------------------------------
// Solute natural flux surface boundary load
//
class FSSoluteNaturalFlux : public FSSurfaceLoad
{
public:
    enum { SID, BSHL };
    
public:
    FSSoluteNaturalFlux(FSModel* ps, FSItemListBuilder* pi = 0, int nstep = 0);
    
    int GetBC() { return GetIntValue(SID); }
    void SetBC(int n) { SetIntValue(SID, n); }
    

    void SetShellBottomFlag(bool b) { SetBoolValue(BSHL, b); }
    bool GetShellBottomFlag() { return GetBoolValue(BSHL); }
};

//-----------------------------------------------------------------------------
// Matching osmotic coefficient surface boundary load
//
class FSMatchingOsmoticCoefficient : public FSSurfaceLoad
{
public:
    enum { AMBP, AMBC, BSHL };

public:
    FSMatchingOsmoticCoefficient(FSModel* ps, FSItemListBuilder* pi = 0, int nstep = 0);

    void SetLoadP(double f) { SetFloatValue(AMBP, f); }
    double GetLoadP() { return GetFloatValue(AMBP); }

    void SetLoadC(double f) { SetFloatValue(AMBC, f); }
    double GetLoadC() { return GetFloatValue(AMBC); }

    void SetShellBottomFlag(bool b) { SetBoolValue(BSHL, b); }
    bool GetShellBottomFlag() { return GetBoolValue(BSHL); }

};

//-----------------------------------------------------------------------------
// Heat flux surface boundary load
//
class FSHeatFlux : public FSSurfaceLoad
{
public:
	enum { FLUX };

public:
	FSHeatFlux(FSModel* ps, FSItemListBuilder* pi = 0, int nstep = 0);

	void SetLoad(double f) { SetFloatValue(FLUX, f); }
	double GetLoad() { return GetFloatValue(FLUX); }

	// used only for reading parameters for old file formats
	void LoadParam(const Param& p);
};

//-----------------------------------------------------------------------------
// Convective Heat flux surface boundary load
//
class FSConvectiveHeatFlux : public FSSurfaceLoad
{
public:
	enum { HC, TREF };

public:
	FSConvectiveHeatFlux(FSModel* ps, FSItemListBuilder* pi = 0, int nstep = 0);

	double GetCoefficient() { return GetFloatValue(HC); }
	double GetTemperature() { return GetFloatValue(TREF); }

	void SetCoefficient(double hc) { SetFloatValue(HC, hc); }
	void SetTemperature(double Ta) { SetFloatValue(TREF, Ta); }

	// used only for reading parameters for old file formats
	void LoadParam(const Param& p);
};

//-----------------------------------------------------------------------------

class FSSurfaceTraction : public FSSurfaceLoad
{
public:
	enum { LOAD, TRACTION };

public:
	FSSurfaceTraction(FSModel* ps, FSItemListBuilder* pi = 0, int nstep = 0);

	void SetScale(double f) { SetFloatValue(LOAD, f); }
	double GetScale() { return GetFloatValue(LOAD); }

	void SetTraction(const vec3d& t) { SetVecValue(TRACTION, t); }
	vec3d GetTraction() { return GetVecValue(TRACTION); }

	// used only for reading parameters for old file formats
	void LoadParam(const Param& p);
};

//-----------------------------------------------------------------------------

class FSSurfaceForceUniform : public FSSurfaceLoad
{
public:
    enum { SCALE, FORCE };
    
public:
    FSSurfaceForceUniform(FSModel* ps, FSItemListBuilder* pi = 0, int nstep = 0);
    
    void SetScale(double f) { SetFloatValue(SCALE, f); }
    double GetScale() { return GetFloatValue(SCALE); }
    
    void SetForce(const vec3d& t) { SetVecValue(FORCE, t); }
    vec3d GetForce() { return GetVecValue(FORCE); }
};

//-----------------------------------------------------------------------------
class FSBearingLoad: public FSSurfaceLoad
{
public:
    enum { SCALE, FORCE, PROFILE, NTYPE };
    
public:
    FSBearingLoad(FSModel* ps, FSItemListBuilder* pi = 0, int nstep = 0);
};

//-----------------------------------------------------------------------------

class FSFluidTraction : public FSSurfaceLoad
{
public:
	enum { LOAD, TRACTION };

public:
	FSFluidTraction(FSModel* ps, FSItemListBuilder* pi = 0, int nstep = 0);

	void SetScale(double s);
	double GetScale();

	void SetTraction(const vec3d& t);
	vec3d GetTraction();

	// used only for reading parameters for old file formats
	void LoadParam(const Param& p);
};

//-----------------------------------------------------------------------------
// fluid pressure boundary conditions
//
class FSFluidPressureLoad : public FSSurfaceLoad
{
public:
    enum { LOAD };
    
public:
    FSFluidPressureLoad(FSModel* ps, FSItemListBuilder* pi = 0, int nstep = 0);
    
    void SetLoad(double f) { SetFloatValue(LOAD, f); }
    double GetLoad() { return GetFloatValue(LOAD); }
    
    // used only for reading parameters for old file formats
    void LoadParam(const Param& p);
};

//-----------------------------------------------------------------------------

class FSFluidVelocity : public FSSurfaceLoad
{
public:
    enum { LOAD };
    
public:
    FSFluidVelocity(FSModel* ps);
    FSFluidVelocity(FSModel* ps, FSItemListBuilder* pi, vec3d t, int nstep = 0);
    
    void SetLoad(vec3d t) { SetVecValue(LOAD, t); }
    vec3d GetLoad() { return GetVecValue(LOAD); }
};

//-----------------------------------------------------------------------------

class FSFluidNormalVelocity : public FSSurfaceLoad
{
public:
    enum {LOAD, BP, BPARAB, BRIMP};
    
public:
    FSFluidNormalVelocity(FSModel* ps);
    FSFluidNormalVelocity(FSModel* ps, FSItemListBuilder* pi, double vn, bool bp, bool bparab, bool brimp, int nstep = 0);
    
    void SetLoad(double f) { SetFloatValue(LOAD, f); }
    double GetLoad() { return GetFloatValue(LOAD); }
    
    void SetBP(bool b) { SetBoolValue(BP, b); }
    bool GetBP() { return GetBoolValue(BP); }
    
    void SetBParab(bool b) { SetBoolValue(BPARAB, b); }
    bool GetBParab() { return GetBoolValue(BPARAB); }
    
    void SetBRimP(bool b) { SetBoolValue(BRIMP, b); }
    bool GetBRimP() { return GetBoolValue(BRIMP); }
};

//-----------------------------------------------------------------------------

class FSFluidRotationalVelocity : public FSSurfaceLoad
{
public:
    enum {LOAD, AXIS, ORIGIN };
    
public:
    FSFluidRotationalVelocity(FSModel* ps);
    FSFluidRotationalVelocity(FSModel* ps, FSItemListBuilder* pi, double w, vec3d n, vec3d p, int nstep = 0);
    
    void SetLoad(double w) { SetFloatValue(LOAD, w); }
    double GetLoad() { return GetFloatValue(LOAD); }
    
    void SetAxis(vec3d n) { SetVecValue(AXIS, n); }
    vec3d GetAxis() { return GetVecValue(AXIS); }
    
    void SetOrigin(vec3d p) { SetVecValue(ORIGIN, p); }
    vec3d GetOrigin() { return GetVecValue(ORIGIN); }
};

//-----------------------------------------------------------------------------

class FSFluidFlowResistance : public FSSurfaceLoad
{
public:
    enum {LOAD, PO };
    
public:
    FSFluidFlowResistance(FSModel* ps);
    FSFluidFlowResistance(FSModel* ps, FSItemListBuilder* pi, double b, double p, int nstep = 0);
    
    void SetLoad(double f) { SetFloatValue(LOAD, f); }
    double GetLoad() { return GetFloatValue(LOAD); }
    
    void SetPO(double f) { SetFloatValue(PO, f); }
    double GetPO() { return GetFloatValue(PO); }
    
};

//-----------------------------------------------------------------------------

class FSFluidFlowRCR : public FSSurfaceLoad
{
public:
    enum {LOAD, RD, CO, PO, IP };
    
public:
    FSFluidFlowRCR(FSModel* ps);
    FSFluidFlowRCR(FSModel* ps, FSItemListBuilder* pi, double rp, double rd, double co, double po, double ip, bool be, int nstep = 0);
    
    void SetLoad(double f) { SetFloatValue(LOAD, f); }
    double GetLoad() { return GetFloatValue(LOAD); }
    
    void SetRD(double f) { SetFloatValue(RD, f); }
    double GetRD() { return GetFloatValue(RD); }
    
    void SetCO(double f) { SetFloatValue(CO, f); }
    double GetCO() { return GetFloatValue(CO); }
    
    void SetPO(double f) { SetFloatValue(PO, f); }
    double GetPO() { return GetFloatValue(PO); }

    void SetIP(double f) { SetFloatValue(IP, f); }
    double GetIP() { return GetFloatValue(IP); }
    
};

//-----------------------------------------------------------------------------

class FSFluidBackflowStabilization : public FSSurfaceLoad
{
public:
    enum { LOAD };
    
public:
    FSFluidBackflowStabilization(FSModel* ps);
    FSFluidBackflowStabilization(FSModel* ps, FSItemListBuilder* pi, double b, int nstep = 0);
    
    void SetLoad(double f) { SetFloatValue(LOAD, f); }
    double GetLoad() { return GetFloatValue(LOAD); }
    
};

//-----------------------------------------------------------------------------

class FSFluidTangentialStabilization : public FSSurfaceLoad
{
public:
    enum { LOAD };
    
public:
    FSFluidTangentialStabilization(FSModel* ps);
    FSFluidTangentialStabilization(FSModel* ps, FSItemListBuilder* pi, double b, int nstep = 0);
    
    void SetLoad(double f) { SetFloatValue(LOAD, f); }
    double GetLoad() { return GetFloatValue(LOAD); }
    
};

//-----------------------------------------------------------------------------

class FSFSITraction : public FSSurfaceLoad
{
public:
    FSFSITraction(FSModel* ps);
    FSFSITraction(FSModel* ps, FSItemListBuilder* pi, int nstep = 0);
};

//-----------------------------------------------------------------------------

class FSBFSITraction : public FSSurfaceLoad
{
public:
    FSBFSITraction(FSModel* ps);
    FSBFSITraction(FSModel* ps, FSItemListBuilder* pi, int nstep = 0);
};

//-----------------------------------------------------------------------------
// concentration flux for reaction-diffusion problems
class FSConcentrationFlux : public FSSurfaceLoad
{
public:
	enum { SOL_ID, FLUX };

public:
	FSConcentrationFlux(FSModel* ps);
	
	void SetFlux(double f);
	double GetFlux();

	int GetSoluteID();
	void SetSoluteID(int n);
};

//-----------------------------------------------------------------------------
class FEBioSurfaceLoad : public FSSurfaceLoad
{
public:
    FEBioSurfaceLoad(FSModel* ps);
    void Save(OArchive& ar);
    void Load(IArchive& ar);
};

//-----------------------------------------------------------------------------
// fluid-solutes natural flux surface boundary load
//
class FSFluidSolutesNaturalFlux : public FSSurfaceLoad
{
public:
    enum { SID };
    
public:
    FSFluidSolutesNaturalFlux(FSModel* ps, FSItemListBuilder* pi = 0, int nstep = 0);
    
    int GetBC() { return GetIntValue(SID); }
    void SetBC(int n) { SetIntValue(SID, n); }
    
};

