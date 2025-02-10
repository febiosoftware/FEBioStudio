#pragma once
#include "FEStepComponent.h"
#include <MeshLib/FSItemListBuilder.h>

//-----------------------------------------------------------------------------
// Base class for contact interfaces
class FSRigidConnector : public FSStepComponent
{
public:
	FSRigidConnector(int ntype, FSModel* ps, int nstep);
	virtual ~FSRigidConnector();

	int Type();

	virtual void SetPosition(const vec3d& r);

public:
    virtual void SetRigidBody1(int rb) = 0;
    virtual void SetRigidBody2(int rb) = 0;

    virtual int GetRigidBody1() const = 0;
    virtual int GetRigidBody2() const = 0;

protected:
	void SaveList(FSItemListBuilder* pitem, OArchive& ar);
	FSItemListBuilder* LoadList(IArchive& ar);

protected:
	int		m_ntype;
};

//-----------------------------------------------------------------------------
class FBSRigidConnector : public FSRigidConnector
{
public:
    FBSRigidConnector(int ntype, FSModel* ps, int nstep);

public:
    void SetRigidBody1(int rb) override { m_rbA = rb; }
    void SetRigidBody2(int rb) override { m_rbB = rb; }

    int GetRigidBody1() const override { return m_rbA; }
    int GetRigidBody2() const override { return m_rbB; }

    void Save(OArchive& ar) override;
    void Load(IArchive& ar) override;

private:
    int	m_rbA;
    int	m_rbB;
};

//-----------------------------------------------------------------------------
//  This class implements a rigid spherical joint
//
class FSRigidSphericalJoint : public FBSRigidConnector
{
public:
    enum {TOL, GTOL, ATOL, F_PENALTY, M_PENALTY, AUTOPEN, J_ORIG,
        MIN_AUG, MAX_AUG, B_ROT, ROT_X, ROT_Y, ROT_Z, MX, MY, MZ };
    
public:
    FSRigidSphericalJoint(FSModel* ps, int nstep = 0);
	void SetPosition(const vec3d& r) override;
};

//-----------------------------------------------------------------------------
//  This class implements a rigid revolute joint
//
class FSRigidRevoluteJoint : public FBSRigidConnector
{
public:
    enum {TOL, GTOL, ATOL, F_PENALTY, M_PENALTY, AUTOPEN, J_ORIG, J_AXIS, T_AXIS,
        MIN_AUG, MAX_AUG, B_ROT, ROT, MOMENT };
    
public:
    FSRigidRevoluteJoint(FSModel* ps, int nstep = 0);
	void SetPosition(const vec3d& r) override;
};

//-----------------------------------------------------------------------------
//  This class implements a rigid prismatic joint
//
class FSRigidPrismaticJoint : public FBSRigidConnector
{
public:
    enum {TOL, GTOL, ATOL, F_PENALTY, M_PENALTY, AUTOPEN, J_ORIG, J_AXIS, T_AXIS,
        MIN_AUG, MAX_AUG, B_TRANS, TRANS, FORCE };
    
public:
    FSRigidPrismaticJoint(FSModel* ps, int nstep = 0);
	void SetPosition(const vec3d& r) override;
};

//-----------------------------------------------------------------------------
//  This class implements a rigid cylindrical joint
//
class FSRigidCylindricalJoint : public FBSRigidConnector
{
public:
    enum {TOL, GTOL, ATOL, F_PENALTY, M_PENALTY, AUTOPEN, J_ORIG, J_AXIS, T_AXIS,
        MIN_AUG, MAX_AUG, B_TRANS, TRANS, FORCE, B_ROT, ROT, MOMENT };
    
public:
    FSRigidCylindricalJoint(FSModel* ps, int nstep = 0);
	void SetPosition(const vec3d& r) override;
};

//-----------------------------------------------------------------------------
//  This class implements a rigid planar joint
//
class FSRigidPlanarJoint : public FBSRigidConnector
{
public:
    enum {TOL, GTOL, ATOL, F_PENALTY, M_PENALTY, AUTOPEN, J_ORIG, J_AXIS, T_AXIS,
        MIN_AUG, MAX_AUG, B_TRANS1, TRANS1, B_TRANS2, TRANS2, B_ROT, ROT };
    
public:
    FSRigidPlanarJoint(FSModel* ps, int nstep = 0);
	void SetPosition(const vec3d& r) override;
};

//-----------------------------------------------------------------------------
//  This class implements a rigid lock joint
//
class FSRigidLock : public FBSRigidConnector
{
public:
    enum {TOL, GTOL, ATOL, F_PENALTY, M_PENALTY, AUTOPEN, J_ORIG, J_AXIS, T_AXIS,
        MIN_AUG, MAX_AUG };
    
public:
    FSRigidLock(FSModel* ps, int nstep = 0);
    void SetPosition(const vec3d& r) override;
};

//-----------------------------------------------------------------------------
//  This class implements a rigid spring
//
class FSRigidSpring : public FBSRigidConnector
{
public:
    enum { K, XA, XB };
    
public:
    FSRigidSpring(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
//  This class implements a rigid damper
//
class FSRigidDamper : public FBSRigidConnector
{
public:
    enum { C, XA, XB };
    
public:
    FSRigidDamper(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
//  This class implements a rigid angular damper
//
class FSRigidAngularDamper : public FBSRigidConnector
{
public:
    enum { C };
    
public:
    FSRigidAngularDamper(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
//  This class implements a rigid contractile force
//
class FSRigidContractileForce : public FBSRigidConnector
{
public:
    enum { F, XA, XB };
    
public:
    FSRigidContractileForce(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FSGenericRigidJoint : public FBSRigidConnector
{
public:
	FSGenericRigidJoint(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FEBioRigidConnector : public FSRigidConnector
{
public:
    FEBioRigidConnector(FSModel* ps, int nstep = 0);
    void Save(OArchive& ar);
    void Load(IArchive& ar);

    void SetRigidBody1(int rb) override;
    void SetRigidBody2(int rb) override;

    int GetRigidBody1() const override;
    int GetRigidBody2() const override;
};
