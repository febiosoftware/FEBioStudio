#pragma once
#include <FEMLib/FEStepComponent.h>
#include <FSCore/Archive.h>
#include <FSCore/LoadCurve.h>

class FEModel;

#define FE_RIGID_FIXED				1
#define FE_RIGID_DISPLACEMENT		2
#define FE_RIGID_FORCE				3
#define FE_RIGID_INIT_VELOCITY		4
#define FE_RIGID_INIT_ANG_VELOCITY	5

// old rigid constraint class, retained for backward compatibility
class FERigidConstraintOld : public FSObject
{
	enum { MATID, NAME, CONSTRAINT, BC, VAL, LC };

public:
	FERigidConstraintOld(int ntype, int nstep);
	~FERigidConstraintOld(void);

	int Type() { return m_ntype; }
	int GetStep() { return m_nstep; }
	void SetStep(int n) { m_nstep = n; }

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	void SetMaterialID(int mid) { m_mid = mid; }
	int GetMaterialID() const { return m_mid; }

public:
	int			m_mid;		// material ID
	int			m_BC[6];	// BCs
	double		m_val[6];	// scale values
	FELoadCurve	m_LC [6];	// load curves

private:
	int		m_ntype;	// constraint type
	int		m_nstep;	// step
};

// Don't instantiate this object.
// Instantiate one of the derived classes instead
class FERigidConstraint : public FEStepComponent
{
	enum { MATID, NAME, PARAMS };

protected:
	FERigidConstraint(int ntype, int nstep);

public:
	~FERigidConstraint(void);

	void Save(OArchive& ar);
	void Load(IArchive& ar);

public:
	int Type() { return m_ntype; }

	void SetMaterialID(int mid) { m_matid = mid; }
	int GetMaterialID() const { return m_matid; }

private:
	int		m_ntype;	// constraint type
	int		m_matid;	// material ID
};


class FERigidFixed : public FERigidConstraint
{
	enum { BC1, BC2, BC3, BC4, BC5, BC6 };

public:
	FERigidFixed(FEModel* fem, int nstep = 0);

	bool GetDOF(int i) const { return GetBoolValue(BC1+i); }
	void SetDOF(int i, bool b) { SetBoolValue(BC1+i, b); }
};

class FERigidPrescribed : public FERigidConstraint
{
	enum { DOF, VALUE };

public:
	FERigidPrescribed(int ntype, int nstep);

	int GetDOF() const { return GetIntValue(DOF); }
	void SetDOF(int n) { SetIntValue(DOF, n); }

	double GetValue() const { return GetFloatValue(VALUE); }
	void SetValue(double v) { SetFloatValue(VALUE, v); }

	FELoadCurve* GetLoadCurve() { return GetParamLC(VALUE); }
	void SetLoadCurve(const FELoadCurve& lc) { GetParam(VALUE).SetLoadCurve(lc); }
	void RemoveLoadcurve() { GetParam(VALUE).DeleteLoadCurve(); }
};

class FERigidDisplacement : public FERigidPrescribed
{
public:
	FERigidDisplacement(FEModel* fem, int nstep = 0);
	FERigidDisplacement(int bc, int matid, double v, int nstep);

	bool GetRelativeFlag() const;
};

class FERigidForce : public FERigidPrescribed
{
public:
	FERigidForce(FEModel* fem, int nstep = 0);
	FERigidForce(int bc, int matid, double v, int nstep);

	int GetForceType() const;
	void SetForceType(int n);

	bool IsRelative() const;
};

class FERigidVelocity : public FERigidConstraint
{
	enum { VEL };

public:
	FERigidVelocity(FEModel* fem, int nstep = 0);

	void SetVelocity(const vec3d& v) { SetVecValue(VEL, v); }
	vec3d GetVelocity() const { return GetVecValue(VEL); }
};

class FERigidAngularVelocity : public FERigidConstraint
{
	enum { VEL };

public:
	FERigidAngularVelocity(FEModel* fem, int nstep = 0);

	void SetVelocity(const vec3d& v) { SetVecValue(VEL, v); }
	vec3d GetVelocity() const { return GetVecValue(VEL); }
};


vector<FERigidConstraint*> convertOldToNewRigidConstraint(FEModel* fem, FERigidConstraintOld* rc);
