#pragma once
#include "FEStepComponent.h"
#include "MeshTools/FEItemListBuilder.h"
#include "MeshTools/GMaterial.h"
#include <list>
//using namespace std;

using std::list;

//-----------------------------------------------------------------------------
// Base class for contact interfaces
class FSInterface : public FSStepComponent
{
public:
	FSInterface(int ntype, FSModel* ps, int nstep);
	virtual ~FSInterface();

	int Type() { return m_ntype; }

protected:
	void SaveList(FEItemListBuilder* pitem, OArchive& ar);
	FEItemListBuilder* LoadList(IArchive& ar);

protected:
	FSModel* m_ps;
	int		m_ntype;
};

//-----------------------------------------------------------------------------
//! This class is the base class for interfaces that only require one
//! surface definition (e.g. rigid interface, rigid wall interface)
class FESoloInterface : public FSInterface
{
public:
	FESoloInterface(int ntype, FSModel* ps, int nstep);
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
class FEPairedInterface : public FSInterface
{
public:
	FEPairedInterface(int ntype, FSModel* ps, int nstep);
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
	FERigidInterface(FSModel* ps, int nstep = 0);
	FERigidInterface(FSModel* ps, GMaterial* pm, FEItemListBuilder* pi, int nstep = 0);
	
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
	FERigidWallInterface(FSModel* ps, int nstep = 0);
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
	FERigidSphereInterface(FSModel* ps, int nstep = 0);
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
	FESlidingInterface(FSModel* ps, int nstep = 0);
	~FESlidingInterface() {}
};

//-----------------------------------------------------------------------------
class FESlidingWithGapsInterface : public FEPairedInterface
{
public:
	enum { LAUGON, ALTOL, PENALTY, TWOPASS, AUTOPEN, MU, EPSF, STOL, MINAUG, MAXAUG, GAPTOL, SEGUP };

public:
	FESlidingWithGapsInterface(FSModel* ps, int nstep = 0);
	~FESlidingWithGapsInterface() {}
};

//-----------------------------------------------------------------------------
class FEFacetOnFacetInterface : public FEPairedInterface
{
public:
	enum { LAUGON, ALTOL, PENALTY, TWOPASS, AUTOPEN, MU, EPSF, STOL, MINAUG, MAXAUG, GAPTOL, SEGUP, SEARCH_RADIUS };

public:
	FEFacetOnFacetInterface(FSModel* ps, int nstep = 0);
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
	FETiedInterface(FSModel* ps, int nstep = 0);
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
	FEF2FTiedInterface(FSModel* ps, int nstep = 0);
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
	FEStickyInterface(FSModel* ps, int nstep = 0);
	~FEStickyInterface(){}
};

//-----------------------------------------------------------------------------
// This class implements a periodic boundary constraint
class FEPeriodicBoundary : public FEPairedInterface
{
public:
	enum { LAUGON, ALTOL, PENALTY, TWOPASS };

public:
	FEPeriodicBoundary(FSModel* ps, int nstep = 0);
	~FEPeriodicBoundary(){}
};

//-----------------------------------------------------------------------------
// Biphasic-solid contact
class FEPoroContact : public FEPairedInterface
{
public:
	enum {LAUGON, ALTOL, PENALTY, TWOPASS, AUTOPEN, PRESSPEN, SYMMETRIC, SEARCHRAD };

public:
	FEPoroContact(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FEPoroSoluteContact : public FEPairedInterface
{
public:
	enum {LAUGON, ALTOL, PENALTY, TWOPASS, AUTOPEN, PRESSPEN, SYMMETRIC, CONCPEN, AMBPRESS, AMBCONC, SEARCHRAD };

public:
	FEPoroSoluteContact(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FEMultiphasicContact : public FEPairedInterface
{
public:
	enum {LAUGON, ALTOL, PENALTY, TWOPASS, AUTOPEN, PRESSPEN, SYMMETRIC, CONCPEN, AMBPRESS, AMBCONC, SEARCHRAD };
    
public:
	FEMultiphasicContact(FSModel* ps, int nstep = 0);
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
	FETensionCompressionInterface(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FETiedBiphasicInterface : public FEPairedInterface
{
public:
	enum {LAUGON, ALTOL, GAPTOL, PTOL, PENALTY, AUTOPEN, TWOPASS, KNMULT, SEARCHTOL, PRS_PENALTY, SYMMETRIC, SEARCHRAD};

public:
	FETiedBiphasicInterface(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FETiedMultiphasicInterface : public FEPairedInterface
{
public:
	enum { LAUGON, ALTOL, GAPTOL, PTOL, CTOL, PENALTY, AUTOPEN, TWOPASS, KNMULT, SEARCHTOL, PRS_PENALTY, SYMMETRIC, SEARCHRAD };

public:
	FETiedMultiphasicInterface(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FETiedElasticInterface : public FEPairedInterface
{
public:
	enum { LAUGON, ALTOL, GAPTOL, PTOL, PENALTY, AUTOPEN, TWOPASS, KNMULT, SEARCHTOL, PRS_PENALTY, SYMMETRIC, SEARCHRAD };

public:
	FETiedElasticInterface(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
class FEContactPotentialInterface : public FEPairedInterface
{
public:
	enum { KC, POWER, RIN, ROUT, WTOL };
public:
	FEContactPotentialInterface(FSModel* ps, int nstep = 0);
};


//-----------------------------------------------------------------------------
class FEGapHeatFluxInterface : public FEPairedInterface
{
public:
	FEGapHeatFluxInterface(FSModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
//  This class implements a rigid joint
//
class FERigidJoint : public FSInterface
{
public:
	enum {TOL, PENALTY, RJ};

public:
	FERigidJoint(FSModel* ps, int nstep = 0);
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
	FESpringTiedInterface(FSModel* ps, int nstep = 0);

	double SpringConstant() const;

	void BuildSpringList(vector<pair<int, int> >& L);
};

//-----------------------------------------------------------------------------
class FEBioInterface : public FEPairedInterface
{
public:
	FEBioInterface(FSModel* ps, int nstep = 0);
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
