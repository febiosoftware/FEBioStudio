#pragma once
#include "FEStepComponent.h"
#include <MeshLib/FSItemListBuilder.h>
#include "GMaterial.h"
#include <list>
#include <MeshLib/IHasItemList.h>

//-----------------------------------------------------------------------------
// Base class for contact interfaces
class FSInterface : public FSStepComponent
{
public:
	FSInterface(int ntype, FSModel* ps, int nstep);
	virtual ~FSInterface();

	int Type() { return m_ntype; }

protected:
	void SaveList(FSItemListBuilder* pitem, OArchive& ar);
	FSItemListBuilder* LoadList(IArchive& ar);

protected:
	int		m_ntype;
};

//-----------------------------------------------------------------------------
//! This class is the base class for interfaces that only require one
//! surface definition (e.g. rigid interface, rigid wall interface)
class FSSoloInterface : public FSInterface, public FSHasOneItemList
{
public:
	FSSoloInterface(int ntype, FSModel* ps, int nstep);
	~FSSoloInterface();

	void Save(OArchive& ar);
	void Load(IArchive& ar);
};

//-----------------------------------------------------------------------------
//! This class is the base class for interfaces that require two surfaces
//!
class FSPairedInterface : public FSInterface, public FSHasTwoItemLists
{
public:
	FSPairedInterface(int ntype, FSModel* ps, int nstep);
	~FSPairedInterface();

	void SetPrimarySurface(FSItemListBuilder* pg);
	void SetSecondarySurface(FSItemListBuilder* pg);

	FSItemListBuilder*	GetPrimarySurface();
	FSItemListBuilder*	GetSecondarySurface();

	void SwapPrimarySecondary();

	void Save(OArchive& ar);
	void Load(IArchive& ar);
};

//-----------------------------------------------------------------------------
//  This class implements the rigid node and facets interface
//
class FSRigidInterface : public FSSoloInterface
{
public:
	FSRigidInterface(FSModel* ps, int nstep = 0);
	FSRigidInterface(FSModel* ps, GMaterial* pm, FSItemListBuilder* pi, int nstep = 0);
	
	GMaterial* GetRigidBody() { return m_pmat; }
	void SetRigidBody(GMaterial* pm) { m_pmat = pm; }

	void Save(OArchive& ar);
	void Load(IArchive& ar);

protected:
	GMaterial*	m_pmat;				// pointer to rigid material
};

//-----------------------------------------------------------------------------
//  This class implements the rigid wall interface
//
class FSRigidWallInterface : public FSSoloInterface
{
public:
	enum { LAUGON, ALTOL, PENALTY, PA, PB, PC, PD, OFFSET };

public:
	FSRigidWallInterface(FSModel* ps, int nstep = 0);
	~FSRigidWallInterface(){}

	void GetPlaneEquation(double a[4]);
};

//-----------------------------------------------------------------------------
//  This class implements the rigid sphere contact interface
//
class FSRigidSphereInterface : public FSSoloInterface
{
public:
	enum { LAUGON, ALTOL, PENALTY, RADIUS, CENTER, UX, UY, UZ};

public:
	FSRigidSphereInterface(FSModel* ps, int nstep = 0);
	~FSRigidSphereInterface(){}

	double Radius();
	vec3d Center();
};

//-----------------------------------------------------------------------------
//  This class implements the sliding contact interface
// NOTE: This class is now obsolete. It was deprecated since it does not map nicely to an FEBio contact interface
//       It was replaced by the FSSlidingWithGapsInterface and FSFacetOnFacetInterface.
class FSSlidingInterface : public FSPairedInterface
{
public:
	enum {LAUGON, ALTOL, PENALTY, TWOPASS, AUTOPEN, MU, EPSF, STOL, NTYPE, MINAUG, MAXAUG, GAPTOL, SEGUP };

public:
	FSSlidingInterface(FSModel* ps, int nstep = 0);
	~FSSlidingInterface() {}
};

//-----------------------------------------------------------------------------
class FSSlidingWithGapsInterface : public FSPairedInterface
{
public:
	enum { LAUGON, ALTOL, PENALTY, TWOPASS, AUTOPEN, MU, EPSF, STOL, MINAUG, MAXAUG, GAPTOL, SEGUP };

public:
	FSSlidingWithGapsInterface(FSModel* ps, int nstep = 0);
	~FSSlidingWithGapsInterface() {}
};

//-----------------------------------------------------------------------------
class FSFacetOnFacetInterface : public FSPairedInterface
{
public:
	enum { LAUGON, ALTOL, PENALTY, TWOPASS, AUTOPEN, MU, EPSF, STOL, MINAUG, MAXAUG, GAPTOL, SEGUP, SEARCH_RADIUS };

public:
	FSFacetOnFacetInterface(FSModel* ps, int nstep = 0);
	~FSFacetOnFacetInterface() {}
};

//-----------------------------------------------------------------------------
//  This class implements the tied contact interface
//
class FSTiedInterface : public FSPairedInterface
{
public:
	enum { LAUGON, ALTOL, PENALTY, MINAUG, MAXAUG };
public:
	FSTiedInterface(FSModel* ps, int nstep = 0);
	~FSTiedInterface(){}
};

//-----------------------------------------------------------------------------
//  This class implements the facet-on-facet tied contact interface
//
class FSF2FTiedInterface : public FSPairedInterface
{
public:
	enum { LAUGON, ALTOL, PENALTY, MINAUG, MAXAUG };
public:
	FSF2FTiedInterface(FSModel* ps, int nstep = 0);
	~FSF2FTiedInterface(){}
};

//-----------------------------------------------------------------------------
//  This class implements the sticky contact interface
//
class FSStickyInterface : public FSPairedInterface
{
public:
	enum { LAUGON, ALTOL, PENALTY, MINAUG, MAXAUG };
public:
	FSStickyInterface(FSModel* ps, int nstep = 0);
	~FSStickyInterface(){}
};

//-----------------------------------------------------------------------------
// This class implements a periodic boundary constraint
class FSPeriodicBoundary : public FSPairedInterface
{
public:
	enum { LAUGON, ALTOL, PENALTY, TWOPASS };

public:
	FSPeriodicBoundary(FSModel* ps, int nstep = 0);
	~FSPeriodicBoundary(){}
};

//-----------------------------------------------------------------------------
// Biphasic-solid contact
class FSPoroContact : public FSPairedInterface
{
public:
	enum {LAUGON, ALTOL, PENALTY, TWOPASS, AUTOPEN, PRESSPEN, SYMMETRIC, SEARCHRAD };

public:
	FSPoroContact(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FSPoroSoluteContact : public FSPairedInterface
{
public:
	enum {LAUGON, ALTOL, PENALTY, TWOPASS, AUTOPEN, PRESSPEN, SYMMETRIC, CONCPEN, AMBPRESS, AMBCONC, SEARCHRAD };

public:
	FSPoroSoluteContact(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FSMultiphasicContact : public FSPairedInterface
{
public:
	enum {LAUGON, ALTOL, PENALTY, TWOPASS, AUTOPEN, PRESSPEN, SYMMETRIC, CONCPEN, AMBPRESS, AMBCONC, SEARCHRAD };
    
public:
	FSMultiphasicContact(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FSTensionCompressionInterface : public FSPairedInterface
{
public:
	enum {
		LAUGON, 
		ALTOL, 
		GAPTOL, 
		PENALTY, 
		AUTOPEN, 
		TWOPASS, 
		SEARCHTOL, 
		SYMMETRIC, 
		SEARCHRAD, 
		NSEGUP, 
		BTENSION, 
		MINAUG,
		MAXAUG,
		FRICCOEFF,
		AUGSMOOTH,
		BNODERELOC,
		BFLIPMASTER, 
		BFLIPSLAVE, 
		KNMULT
	};

public:
	FSTensionCompressionInterface(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FSTiedBiphasicInterface : public FSPairedInterface
{
public:
	enum {LAUGON, ALTOL, GAPTOL, PTOL, PENALTY, AUTOPEN, TWOPASS, KNMULT, SEARCHTOL, PRS_PENALTY, SYMMETRIC, SEARCHRAD};

public:
	FSTiedBiphasicInterface(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FSTiedMultiphasicInterface : public FSPairedInterface
{
public:
	enum { LAUGON, ALTOL, GAPTOL, PTOL, CTOL, PENALTY, AUTOPEN, TWOPASS, KNMULT, SEARCHTOL, PRS_PENALTY, SYMMETRIC, SEARCHRAD };

public:
	FSTiedMultiphasicInterface(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FSTiedElasticInterface : public FSPairedInterface
{
public:
	enum { LAUGON, ALTOL, GAPTOL, PTOL, PENALTY, AUTOPEN, TWOPASS, KNMULT, SEARCHTOL, PRS_PENALTY, SYMMETRIC, SEARCHRAD };

public:
	FSTiedElasticInterface(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FSContactPotentialInterface : public FSPairedInterface
{
public:
	enum { KC, POWER, RIN, ROUT, WTOL };
public:
	FSContactPotentialInterface(FSModel* ps, int nstep = 0);
};


//-----------------------------------------------------------------------------
class FSGapHeatFluxInterface : public FSPairedInterface
{
public:
	FSGapHeatFluxInterface(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
//  This class implements a rigid joint
//
class FSRigidJoint : public FSInterface
{
public:
	enum {TOL, PENALTY, RJ};

public:
	FSRigidJoint(FSModel* ps, int nstep = 0);
	void Save(OArchive& ar);
	void Load(IArchive& ar);

public:
	GMaterial*	m_pbodyA;	// rigid body a
	GMaterial*	m_pbodyB;	// rigid body b
};

//-----------------------------------------------------------------------------
// This class implements a spring-tied interface, that is, an interface
// where the nodes of one surface are connected with springs to the other
// surface.
class FSSpringTiedInterface : public FSPairedInterface
{
public:
	enum { ECONST };

public:
	FSSpringTiedInterface(FSModel* ps, int nstep = 0);

	double SpringConstant() const;

	void BuildSpringList(std::vector<std::pair<int, int> >& L);
};

//-----------------------------------------------------------------------------
class FEBioInterface : public FSPairedInterface
{
public:
	FEBioInterface(FSModel* ps, int nstep = 0);
	void Save(OArchive& ar);
	void Load(IArchive& ar);
};

//-----------------------------------------------------------------------------
// This class implements a linear constraint
// TODO: Figure out a way to integrate this
class FSLinearConstraintSet : public FSObject
{
public:
	// a linear constraint defined via LCDOFs
	class LinearConstraint
	{
	public:
		// linear constraint dof
		class DOF
		{
		public:
			DOF() { node = -1; bc = 0; s = 0.0; }

		public:
			int		node;
			int		bc;
			double	s;
		};

	public:
		LinearConstraint(){}
		LinearConstraint(const LinearConstraint& LC) { m_dof = LC.m_dof; }
		LinearConstraint& operator = (const LinearConstraint& LC) { m_dof = LC.m_dof; return (*this); }

	public:
		std::vector<DOF>	m_dof;
	};

public:
	FSLinearConstraintSet();
	FSLinearConstraintSet(const FSLinearConstraintSet& lcs);
	FSLinearConstraintSet& operator = (const FSLinearConstraintSet& lcs);

public:
	double	m_atol;
	double	m_penalty;
	int		m_nmaxaug;

public:
	std::vector<LinearConstraint>	m_set;
};
