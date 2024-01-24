#pragma once
#include "FEStepComponent.h"
#include <FSCore/Archive.h>
#include <FSCore/LoadCurve.h>

class FSModel;

#define FE_RIGID_FIXED				1
#define FE_RIGID_DISPLACEMENT		2
#define FE_RIGID_FORCE				3
#define FE_RIGID_INIT_VELOCITY		4
#define FE_RIGID_INIT_ANG_VELOCITY	5

// old rigid constraint class, retained for backward compatibility
class FSRigidConstraintOld : public FSObject
{
	enum { MATID, NAME, CONSTRAINT, BC, VAL, LC };

public:
	FSRigidConstraintOld(int ntype, int nstep);
	~FSRigidConstraintOld(void);

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
	LoadCurve	m_LC [6];	// load curves

private:
	int		m_ntype;	// constraint type
	int		m_nstep;	// step
};

// Don't instantiate this object.
// Instantiate one of the derived classes instead
class FSRigidConstraint : public FSStepComponent
{
protected:
	enum { MATID, NAME, PARAMS, STATUS };

protected:
	FSRigidConstraint(int ntype, int nstep, FSModel* fem = nullptr);

public:
	~FSRigidConstraint(void);

public:
	int Type() { return m_ntype; }

	virtual void SetMaterialID(int mid) = 0;
	virtual int GetMaterialID() const = 0;

private:
	int		m_ntype;	// constraint type
};

// Rigid constraints used to have their own Super ID, but are now grouped with
// boundary conditions. However, this causes an issue with reading older files. 
// So, we define a special super class ID, just for reading older fsm files. 
#define FERIGIDBC_ID	0x100

class FBSRigidConstraint : public FSRigidConstraint
{
public:
	FBSRigidConstraint(int ntype, int nstep, FSModel* fem = nullptr);

	void SetMaterialID(int mid) override { m_matid = mid; }
	int GetMaterialID() const override { return m_matid; }

	void Save(OArchive& ar);
	void Load(IArchive& ar);

private:
	int		m_matid;	// material ID
};


class FSRigidFixed : public FBSRigidConstraint
{
	enum { BC1, BC2, BC3, BC4, BC5, BC6 };

public:
	FSRigidFixed(FSModel* fem, int nstep = 0);

	bool GetDOF(int i) const { return GetBoolValue(BC1+i); }
	void SetDOF(int i, bool b) { SetBoolValue(BC1+i, b); }
};

class FSRigidPrescribed : public FBSRigidConstraint
{
public:
	enum { DOF, VALUE };

public:
	FSRigidPrescribed(int ntype, int nstep, FSModel* fem = nullptr);

	int GetDOF() const { return GetIntValue(DOF); }
	void SetDOF(int n) { SetIntValue(DOF, n); }

	double GetValue() const { return GetFloatValue(VALUE); }
	void SetValue(double v) { SetFloatValue(VALUE, v); }
};

class FSRigidDisplacement : public FSRigidPrescribed
{
public:
	enum { VAR, VALUE, BC_RELATIVE };

public:
	FSRigidDisplacement(FSModel* fem, int nstep = 0);
	FSRigidDisplacement(int bc, int matid, double v, int nstep);

	bool GetRelativeFlag() const;
};

class FSRigidForce : public FSRigidPrescribed
{
public:
	FSRigidForce(FSModel* fem, int nstep = 0);
	FSRigidForce(int bc, int matid, double v, int nstep);

	int GetForceType() const;
	void SetForceType(int n);

	bool IsRelative() const;
};

class FSRigidVelocity : public FBSRigidConstraint
{
	enum { VEL };

public:
	FSRigidVelocity(FSModel* fem, int nstep = 0);

	void SetVelocity(const vec3d& v) { SetVecValue(VEL, v); }
	vec3d GetVelocity() const { return GetVecValue(VEL); }
};

class FSRigidAngularVelocity : public FBSRigidConstraint
{
	enum { VEL };

public:
	FSRigidAngularVelocity(FSModel* fem, int nstep = 0);

	void SetVelocity(const vec3d& v) { SetVecValue(VEL, v); }
	vec3d GetVelocity() const { return GetVecValue(VEL); }
};

std::vector<FSRigidConstraint*> convertOldToNewRigidConstraint(FSModel* fem, FSRigidConstraintOld* rc);
