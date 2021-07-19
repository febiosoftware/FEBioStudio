#pragma once
#include "FEStepComponent.h"
#include "MeshTools/FEItemListBuilder.h"
#include "MeshTools/GMaterial.h"
#include <list>
//using namespace std;

//-----------------------------------------------------------------------------
// Base class for contact interfaces
class FERigidConnector : public FEStepComponent
{
public:
	FERigidConnector(int ntype, FEModel* ps, int nstep);
	virtual ~FERigidConnector();

	int Type();

	virtual void SetPosition(const vec3d& r);

	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

public:
    void SetRigidBody1(int rb) { m_rbA = rb; }
    void SetRigidBody2(int rb) { m_rbB = rb; }

    int GetRigidBody1() const { return m_rbA; }
    int GetRigidBody2() const { return m_rbB; }

protected:
	void SaveList(FEItemListBuilder* pitem, OArchive& ar);
	FEItemListBuilder* LoadList(IArchive& ar);

protected:
	FEModel* m_ps;
	int		m_ntype;
	int	m_rbA;
	int	m_rbB;
};

//-----------------------------------------------------------------------------
//  This class implements a rigid spherical joint
//
class FERigidSphericalJoint : public FERigidConnector
{
public:
    enum {TOL, GTOL, ATOL, F_PENALTY, M_PENALTY, AUTOPEN, J_ORIG,
        MIN_AUG, MAX_AUG, B_ROT, ROT_X, ROT_Y, ROT_Z, MX, MY, MZ };
    
public:
    FERigidSphericalJoint(FEModel* ps, int nstep = 0);
	void SetPosition(const vec3d& r) override;
};

//-----------------------------------------------------------------------------
//  This class implements a rigid revolute joint
//
class FERigidRevoluteJoint : public FERigidConnector
{
public:
    enum {TOL, GTOL, ATOL, F_PENALTY, M_PENALTY, AUTOPEN, J_ORIG, J_AXIS, T_AXIS,
        MIN_AUG, MAX_AUG, B_ROT, ROT, MOMENT };
    
public:
    FERigidRevoluteJoint(FEModel* ps, int nstep = 0);
	void SetPosition(const vec3d& r) override;
};

//-----------------------------------------------------------------------------
//  This class implements a rigid prismatic joint
//
class FERigidPrismaticJoint : public FERigidConnector
{
public:
    enum {TOL, GTOL, ATOL, F_PENALTY, M_PENALTY, AUTOPEN, J_ORIG, J_AXIS, T_AXIS,
        MIN_AUG, MAX_AUG, B_TRANS, TRANS, FORCE };
    
public:
    FERigidPrismaticJoint(FEModel* ps, int nstep = 0);
	void SetPosition(const vec3d& r) override;
};

//-----------------------------------------------------------------------------
//  This class implements a rigid cylindrical joint
//
class FERigidCylindricalJoint : public FERigidConnector
{
public:
    enum {TOL, GTOL, ATOL, F_PENALTY, M_PENALTY, AUTOPEN, J_ORIG, J_AXIS, T_AXIS,
        MIN_AUG, MAX_AUG, B_TRANS, TRANS, FORCE, B_ROT, ROT, MOMENT };
    
public:
    FERigidCylindricalJoint(FEModel* ps, int nstep = 0);
	void SetPosition(const vec3d& r) override;
};

//-----------------------------------------------------------------------------
//  This class implements a rigid planar joint
//
class FERigidPlanarJoint : public FERigidConnector
{
public:
    enum {TOL, GTOL, ATOL, F_PENALTY, M_PENALTY, AUTOPEN, J_ORIG, J_AXIS, T_AXIS,
        MIN_AUG, MAX_AUG, B_TRANS1, TRANS1, B_TRANS2, TRANS2, B_ROT, ROT };
    
public:
    FERigidPlanarJoint(FEModel* ps, int nstep = 0);
	void SetPosition(const vec3d& r) override;
};

//-----------------------------------------------------------------------------
//  This class implements a rigid lock joint
//
class FERigidLock : public FERigidConnector
{
public:
    enum {TOL, GTOL, ATOL, F_PENALTY, M_PENALTY, AUTOPEN, J_ORIG, J_AXIS, T_AXIS,
        MIN_AUG, MAX_AUG };
    
public:
    FERigidLock(FEModel* ps, int nstep = 0);
    void SetPosition(const vec3d& r) override;
};

//-----------------------------------------------------------------------------
//  This class implements a rigid spring
//
class FERigidSpring : public FERigidConnector
{
public:
    enum { K, XA, XB };
    
public:
    FERigidSpring(FEModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
//  This class implements a rigid damper
//
class FERigidDamper : public FERigidConnector
{
public:
    enum { C, XA, XB };
    
public:
    FERigidDamper(FEModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
//  This class implements a rigid angular damper
//
class FERigidAngularDamper : public FERigidConnector
{
public:
    enum { C };
    
public:
    FERigidAngularDamper(FEModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
//  This class implements a rigid contractile force
//
class FERigidContractileForce : public FERigidConnector
{
public:
    enum { F, XA, XB };
    
public:
    FERigidContractileForce(FEModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FEGenericRigidJoint : public FERigidConnector
{
public:
	FEGenericRigidJoint(FEModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FEBioRigidConnector : public FERigidConnector
{
public:
    FEBioRigidConnector(FEModel* ps, int nstep = 0);
    void Save(OArchive& ar);
    void Load(IArchive& ar);
};
