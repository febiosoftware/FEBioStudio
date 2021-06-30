#pragma once
#include "FEStepComponent.h"
#include "MeshTools/FEItemListBuilder.h"
#include "MeshTools/GMaterial.h"
#include <list>
//using namespace std;

using std::list;

//-----------------------------------------------------------------------------
// Base class for contact interfaces
class FEInterface : public FEStepComponent
{
public:
	FEInterface(int ntype, FEModel* ps, int nstep);
	virtual ~FEInterface();

	int Type() { return m_ntype; }

protected:
	void SaveList(FEItemListBuilder* pitem, OArchive& ar);
	FEItemListBuilder* LoadList(IArchive& ar);

protected:
	FEModel* m_ps;
	int		m_ntype;
};

//-----------------------------------------------------------------------------
//! This class is the base class for interfaces that only require one
//! surface definition (e.g. rigid interface, rigid wall interface)
class FESoloInterface : public FEInterface
{
public:
	FESoloInterface(int ntype, FEModel* ps, int nstep);
	~FESoloInterface();

	FEItemListBuilder* GetItemList() { return m_pItem; }
	void SetItemList(FEItemListBuilder* pi) { m_pItem = pi; }

	void Save(OArchive& ar);
	void Load(IArchive& ar);

protected:
	FEItemListBuilder*	m_pItem;	// list of items that define interface
};

//-----------------------------------------------------------------------------
//! This class is the base class for interfaces that require two surfaces
//!
class FEPairedInterface : public FEInterface
{
public:
	FEPairedInterface(int ntype, FEModel* ps, int nstep);
	~FEPairedInterface();

	void SetPrimarySurface(FEItemListBuilder* pg) { m_surf1 = pg; }
	void SetSecondarySurface(FEItemListBuilder* pg) { m_surf2 = pg; }

	FEItemListBuilder*	GetPrimarySurface() { return m_surf1; }
	FEItemListBuilder*	GetSecondarySurface() { return m_surf2;  }

	FEItemListBuilder* GetItemList(int index) { return (index == 0 ? GetPrimarySurface() : GetSecondarySurface()); }
	void SetItemList(int index, FEItemListBuilder* itemList) { (index == 0 ? SetPrimarySurface(itemList) : SetSecondarySurface(itemList)); }

	void SwapPrimarySecondary();

	void Save(OArchive& ar);
	void Load(IArchive& ar);

public:
	FEItemListBuilder*	m_surf1;	// primary surface item list
	FEItemListBuilder*	m_surf2;	// secondary syurface item list
};

//-----------------------------------------------------------------------------
//  This class implements the rigid node and facets interface
//
class FERigidInterface : public FESoloInterface
{
public:
	FERigidInterface(FEModel* ps, int nstep = 0);
	FERigidInterface(FEModel* ps, GMaterial* pm, FEItemListBuilder* pi, int nstep = 0);
	
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
class FERigidWallInterface : public FESoloInterface
{
public:
	enum { LAUGON, ALTOL, PENALTY, PA, PB, PC, PD, OFFSET };

public:
	FERigidWallInterface(FEModel* ps, int nstep = 0);
	~FERigidWallInterface(){}

	FELoadCurve* GetLoadCurve() { return GetParamLC(OFFSET); }

	void GetPlaneEquation(double a[4]);
};

//-----------------------------------------------------------------------------
//  This class implements the rigid sphere contact interface
//
class FERigidSphereInterface : public FESoloInterface
{
public:
	enum { LAUGON, ALTOL, PENALTY, RADIUS, CENTER, UX, UY, UZ};

public:
	FERigidSphereInterface(FEModel* ps, int nstep = 0);
	~FERigidSphereInterface(){}

	FELoadCurve* GetLoadCurve(int i);

	double Radius();
	vec3d Center();
};

//-----------------------------------------------------------------------------
//  This class implements the sliding contact interface
// NOTE: This class is now obsolete. It was deprecated since it does not map nicely to an FEBio contact interface
//       It was replaced by the FESlidingWithGapsInterface and FEFacetOnFacetInterface.
class FESlidingInterface : public FEPairedInterface
{
public:
	enum {LAUGON, ALTOL, PENALTY, TWOPASS, AUTOPEN, MU, EPSF, STOL, NTYPE, MINAUG, MAXAUG, GAPTOL, SEGUP };

public:
	FESlidingInterface(FEModel* ps, int nstep = 0);
	~FESlidingInterface() {}
};

//-----------------------------------------------------------------------------
class FESlidingWithGapsInterface : public FEPairedInterface
{
public:
	enum { LAUGON, ALTOL, PENALTY, TWOPASS, AUTOPEN, MU, EPSF, STOL, MINAUG, MAXAUG, GAPTOL, SEGUP };

public:
	FESlidingWithGapsInterface(FEModel* ps, int nstep = 0);
	~FESlidingWithGapsInterface() {}
};

//-----------------------------------------------------------------------------
class FEFacetOnFacetInterface : public FEPairedInterface
{
public:
	enum { LAUGON, ALTOL, PENALTY, TWOPASS, AUTOPEN, MU, EPSF, STOL, MINAUG, MAXAUG, GAPTOL, SEGUP, SEARCH_RADIUS };

public:
	FEFacetOnFacetInterface(FEModel* ps, int nstep = 0);
	~FEFacetOnFacetInterface() {}
};

//-----------------------------------------------------------------------------
//  This class implements the tied contact interface
//
class FETiedInterface : public FEPairedInterface
{
public:
	enum { LAUGON, ALTOL, PENALTY, MINAUG, MAXAUG };
public:
	FETiedInterface(FEModel* ps, int nstep = 0);
	~FETiedInterface(){}
};

//-----------------------------------------------------------------------------
//  This class implements the facet-on-facet tied contact interface
//
class FEF2FTiedInterface : public FEPairedInterface
{
public:
	enum { LAUGON, ALTOL, PENALTY, MINAUG, MAXAUG };
public:
	FEF2FTiedInterface(FEModel* ps, int nstep = 0);
	~FEF2FTiedInterface(){}
};

//-----------------------------------------------------------------------------
//  This class implements the sticky contact interface
//
class FEStickyInterface : public FEPairedInterface
{
public:
	enum { LAUGON, ALTOL, PENALTY, MINAUG, MAXAUG };
public:
	FEStickyInterface(FEModel* ps, int nstep = 0);
	~FEStickyInterface(){}
};

//-----------------------------------------------------------------------------
// This class implements a periodic boundary constraint
class FEPeriodicBoundary : public FEPairedInterface
{
public:
	enum { LAUGON, ALTOL, PENALTY, TWOPASS };

public:
	FEPeriodicBoundary(FEModel* ps, int nstep = 0);
	~FEPeriodicBoundary(){}
};

//-----------------------------------------------------------------------------
// Biphasic-solid contact
class FEPoroContact : public FEPairedInterface
{
public:
	enum {LAUGON, ALTOL, PENALTY, TWOPASS, AUTOPEN, PRESSPEN, SYMMETRIC, SEARCHRAD };

public:
	FEPoroContact(FEModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FEPoroSoluteContact : public FEPairedInterface
{
public:
	enum {LAUGON, ALTOL, PENALTY, TWOPASS, AUTOPEN, PRESSPEN, SYMMETRIC, CONCPEN, AMBPRESS, AMBCONC, SEARCHRAD };

public:
	FEPoroSoluteContact(FEModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FEMultiphasicContact : public FEPairedInterface
{
public:
	enum {LAUGON, ALTOL, PENALTY, TWOPASS, AUTOPEN, PRESSPEN, SYMMETRIC, CONCPEN, AMBPRESS, AMBCONC, SEARCHRAD };
    
public:
	FEMultiphasicContact(FEModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FETensionCompressionInterface : public FEPairedInterface
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
	FETensionCompressionInterface(FEModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FETiedBiphasicInterface : public FEPairedInterface
{
public:
	enum {LAUGON, ALTOL, GAPTOL, PTOL, PENALTY, AUTOPEN, TWOPASS, KNMULT, SEARCHTOL, PRS_PENALTY, SYMMETRIC, SEARCHRAD};

public:
	FETiedBiphasicInterface(FEModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FETiedMultiphasicInterface : public FEPairedInterface
{
public:
	enum { LAUGON, ALTOL, GAPTOL, PTOL, CTOL, PENALTY, AUTOPEN, TWOPASS, KNMULT, SEARCHTOL, PRS_PENALTY, SYMMETRIC, SEARCHRAD };

public:
	FETiedMultiphasicInterface(FEModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FETiedElasticInterface : public FEPairedInterface
{
public:
	enum { LAUGON, ALTOL, GAPTOL, PTOL, PENALTY, AUTOPEN, TWOPASS, KNMULT, SEARCHTOL, PRS_PENALTY, SYMMETRIC, SEARCHRAD };

public:
	FETiedElasticInterface(FEModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FEGapHeatFluxInterface : public FEPairedInterface
{
public:
	FEGapHeatFluxInterface(FEModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
//  This class implements a rigid joint
//
class FERigidJoint : public FEInterface
{
public:
	enum {TOL, PENALTY, RJ};

public:
	FERigidJoint(FEModel* ps, int nstep = 0);
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
class FESpringTiedInterface : public FEPairedInterface
{
public:
	enum { ECONST };

public:
	FESpringTiedInterface(FEModel* ps, int nstep = 0);

	double SpringConstant() const;

	void BuildSpringList(vector<pair<int, int> >& L);
};

//-----------------------------------------------------------------------------
class FEBioInterface : public FEPairedInterface
{
public:
	FEBioInterface(FEModel* ps, int nstep = 0);
	void Save(OArchive& ar);
	void Load(IArchive& ar);
};

//-----------------------------------------------------------------------------
// This class implements a linear constraint
// TODO: Figure out a way to integrate this
class FELinearConstraintSet : public FSObject
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
		vector<DOF>	m_dof;
	};

public:
	FELinearConstraintSet();
	FELinearConstraintSet(const FELinearConstraintSet& lcs);
	FELinearConstraintSet& operator = (const FELinearConstraintSet& lcs);

public:
	double	m_atol;
	double	m_penalty;
	int		m_nmaxaug;

public:
	vector<LinearConstraint>	m_set;
};
