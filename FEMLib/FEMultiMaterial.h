#pragma once
#include <FEMLib/FEMaterial.h>
#include <vector>
using namespace std;

class FEMultiphasicMaterial;
class FEModel;

//-----------------------------------------------------------------------------
// visco-elastic
class FEViscoElastic : public FEMaterial
{
public:
	// max nr of Prony terms
	enum { MAX_TERMS = 6 };

	// material parameters
	enum { MP_DENSITY, 
		MP_G1, MP_G2, MP_G3, MP_G4, MP_G5, MP_G6, 
		MP_T1, MP_T2, MP_T3, MP_T4, MP_T5, MP_T6
	};

public:
	// constructor
	FEViscoElastic();

	// set the elastic component of the material
	void SetElasticMaterial(FEMaterial* pm) { ReplaceProperty(0, pm); }
	FEMaterial* GetElasticMaterial() { return GetProperty(0).GetMaterial(); }

	DECLARE_REGISTERED(FEViscoElastic);
};


//-----------------------------------------------------------------------------
// uncoupled visco-elastic
class FEUncoupledViscoElastic : public FEMaterial
{
public:
	// max nr of Prony terms
	enum { MAX_TERMS = 6 };

	// material parameters
	enum { MP_DENSITY,
		MP_G1, MP_G2, MP_G3, MP_G4, MP_G5, MP_G6, 
		MP_T1, MP_T2, MP_T3, MP_T4, MP_T5, MP_T6,
        MP_K
	};

public:
	// constructor
	FEUncoupledViscoElastic();

	// set the elastic component of the material
	void SetElasticMaterial(FEMaterial* pm) { ReplaceProperty(0, pm); }
	FEMaterial* GetElasticMaterial() { return GetProperty(0).GetMaterial(); }

	DECLARE_REGISTERED(FEUncoupledViscoElastic);
};

//-----------------------------------------------------------------------------
// The FEMultiMaterial class is used as a base class for materials that define
// material properties for multi-physics problems. 
class FEMultiMaterial : public FEMaterial
{
public:
	FEMultiMaterial(int ntype);
};

//-----------------------------------------------------------------------------
// biphasic
class FEBiphasic : public FEMultiMaterial
{
public:
	// material parameters
	enum { MP_PHI0, MP_RHOTW };

public:
	// constructor
	FEBiphasic();

	// set/get solid component 
	void SetSolidMaterial(FEMaterial* pm) { ReplaceProperty(0, pm); }
	FEMaterial* GetSolidMaterial() { return GetProperty(0).GetMaterial(); }

	// set/get permeability
	void SetPermeability(FEMaterial* pm) { ReplaceProperty(1, pm); }
	FEMaterial* GetPermeability() { return GetProperty(1).GetMaterial(); }

	DECLARE_REGISTERED(FEBiphasic);
};


//-----------------------------------------------------------------------------
// The FESoluteMaterial is used as a component of a multiphasic material
class FESoluteMaterial : public FEMaterial
{
public:
	enum { MP_NSOL };

public:
	FESoluteMaterial();

	// set/get diffusivity
	void SetDiffusivity(FEMaterial* pm) { ReplaceProperty(0, pm); }
	FEMaterial* GetDiffusivity() { return GetProperty(0).GetMaterial(); }

	// set/get solubility
	void SetSolubility(FEMaterial* pm) { ReplaceProperty(1, pm); }
	FEMaterial* GetSolubility() { return GetProperty(1).GetMaterial(); }

	// get/set solute index
	void SetSoluteIndex(int i) { SetIntValue(MP_NSOL, i); }
	int GetSoluteIndex() { return GetIntValue(MP_NSOL); }

protected:
	DECLARE_REGISTERED(FESoluteMaterial);
};

//-----------------------------------------------------------------------------
// The FESBMMaterial is used as a component of a multiphasic material
class FESBMMaterial : public FEMaterial
{
public:
	enum { MP_NSBM , MP_RHO0 , MP_RMIN , MP_RMAX };
    
public:
	FESBMMaterial();
    
	// get/set solid-bound molecule index
	void SetSBMIndex(int i) { SetIntValue(MP_NSBM, i); }
	int GetSBMIndex() { return GetIntValue(MP_NSBM); }

protected:
	DECLARE_REGISTERED(FESBMMaterial);
};

//-----------------------------------------------------------------------------
// biphasic-solute
class FEBiphasicSolute : public FEMultiMaterial
{
public:
	// material parameters
	enum {
		MP_PHI0
	};

public:
	FEBiphasicSolute();

	// set/get elastic component 
	void SetSolidMaterial(FEMaterial* pm) { ReplaceProperty(0, pm); }
	FEMaterial* GetSolidMaterial() { return GetProperty(0).GetMaterial(); }

	// set/get permeability
	void SetPermeability(FEMaterial* pm) { ReplaceProperty(1, pm); }
	FEMaterial* GetPermeability() { return GetProperty(1).GetMaterial(); }

	// set/get osmotic coefficient
	void SetOsmoticCoefficient(FEMaterial* pm) { ReplaceProperty(2, pm); }
	FEMaterial* GetOsmoticCoefficient() { return GetProperty(2).GetMaterial(); }

	// set/get solute
	void SetSoluteMaterial(FESoluteMaterial* pm) { ReplaceProperty(3, pm); }
	FEMaterial* GetSoluteMaterial() { return GetProperty(3).GetMaterial(); }

	DECLARE_REGISTERED(FEBiphasicSolute);
};

//-----------------------------------------------------------------------------
// triphasic
class FETriphasicMaterial : public FEMultiMaterial
{
public:
	// material parameters
	enum { MP_PHI0, MP_CHARGE };

public:
	FETriphasicMaterial();

	// set/get elastic component 
	void SetSolidMaterial(FEMaterial* pm) { ReplaceProperty(0, pm); }
	FEMaterial* GetSolidMaterial() { return GetProperty(0).GetMaterial(); }

	// set/get permeability
	void SetPermeability(FEMaterial* pm) { ReplaceProperty(1, pm); }
	FEMaterial* GetPermeability() { return GetProperty(1).GetMaterial(); }

	// set/get osmotic coefficient
	void SetOsmoticCoefficient(FEMaterial* pm) { ReplaceProperty(2, pm); }
	FEMaterial* GetOsmoticCoefficient() { return GetProperty(2).GetMaterial(); }

	// set/get solute i
	void SetSoluteMaterial(FESoluteMaterial* pm, int i);
	FEMaterial* GetSoluteMaterial(int i);

	DECLARE_REGISTERED(FETriphasicMaterial);
};

//-----------------------------------------------------------------------------
// solid mixture class
class FESolidMixture : public FEMaterial
{
public:
	FESolidMixture();

	DECLARE_REGISTERED(FESolidMixture);
};

//-----------------------------------------------------------------------------
// uncoupled solid mixture class
class FEUncoupledSolidMixture : public FEMaterial
{
public:
	FEUncoupledSolidMixture();

	DECLARE_REGISTERED(FEUncoupledSolidMixture);
};

//-----------------------------------------------------------------------------
// continuous fiber distribution
class FECFDMaterial : public FEMaterial
{
public:
    // constructor
    FECFDMaterial();
    
    // set/get fiber material
    void SetFiberMaterial(FEMaterial* pm) { ReplaceProperty(0, pm); }
    FEMaterial* GetFiberMaterial() { return GetProperty(0).GetMaterial(); }
    
    // set/get distribution
    void SetDistribution(FEMaterial* pm) { ReplaceProperty(1, pm); }
    FEMaterial* GetDistribution() { return GetProperty(1).GetMaterial(); }
    
    // set/get scheme
    void SetScheme(FEMaterial* pm) { ReplaceProperty(2, pm); }
    FEMaterial* GetScheme() { return GetProperty(2).GetMaterial(); }
    
public:
    DECLARE_REGISTERED(FECFDMaterial);
};

//-----------------------------------------------------------------------------
// continuous fiber distribution uncoupled
class FECFDUCMaterial : public FEMaterial
{
public:
    // constructor
    FECFDUCMaterial();
    
    // set/get fiber material
    void SetFiberMaterial(FEMaterial* pm) { ReplaceProperty(0, pm); }
    FEMaterial* GetFiberMaterial() { return GetProperty(0).GetMaterial(); }
    
    // set/get distribution
    void SetDistribution(FEMaterial* pm) { ReplaceProperty(1, pm); }
    FEMaterial* GetDistribution() { return GetProperty(1).GetMaterial(); }
    
    // set/get scheme
    void SetScheme(FEMaterial* pm) { ReplaceProperty(2, pm); }
    FEMaterial* GetScheme() { return GetProperty(2).GetMaterial(); }
    
public:
    DECLARE_REGISTERED(FECFDUCMaterial);
};

//-----------------------------------------------------------------------------
// elastic damage material
class FEElasticDamageMaterial : public FEMaterial
{
public:
    // constructor
    FEElasticDamageMaterial();
    
    // set/get elastic material
    void SetElasticMaterial(FEMaterial* pm) { ReplaceProperty(0, pm); }
    FEMaterial* GetElasticMaterial() { return GetProperty(0).GetMaterial(); }
    
    // set/get damage material
    void SetDamageMaterial(FEMaterial* pm) { ReplaceProperty(1, pm); }
    FEMaterial* GetDamageMaterial() { return GetProperty(1).GetMaterial(); }
    
    // set/get criterion
    void SetCriterion(FEMaterial* pm) { ReplaceProperty(2, pm); }
    FEMaterial* GetCriterion() { return GetProperty(2).GetMaterial(); }
    
public:
    DECLARE_REGISTERED(FEElasticDamageMaterial);
};

//-----------------------------------------------------------------------------
// uncoupled elastic damage material
class FEElasticDamageMaterialUC : public FEMaterial
{
public:
    // constructor
    FEElasticDamageMaterialUC();
    
    // set/get elastic material
    void SetElasticMaterial(FEMaterial* pm) { ReplaceProperty(0, pm); }
    FEMaterial* GetElasticMaterial() { return GetProperty(0).GetMaterial(); }
    
    // set/get damage material
    void SetDamageMaterial(FEMaterial* pm) { ReplaceProperty(1, pm); }
    FEMaterial* GetDamageMaterial() { return GetProperty(1).GetMaterial(); }
    
    // set/get criterion
    void SetCriterion(FEMaterial* pm) { ReplaceProperty(2, pm); }
    FEMaterial* GetCriterion() { return GetProperty(2).GetMaterial(); }
    
public:
    DECLARE_REGISTERED(FEElasticDamageMaterialUC);
};

//-----------------------------------------------------------------------------
// reactive viscoelastic material
class FEReactiveViscoelasticMaterial : public FEMaterial
{
public:
    // material parameters
    enum { MP_KNTCS, MP_TRGGR };
    
public:
    // constructor
    FEReactiveViscoelasticMaterial();
    
    // set/get elastic material
    void SetElasticMaterial(FEMaterial* pm) { ReplaceProperty(0, pm); }
    FEMaterial* GetElasticMaterial() { return GetProperty(0).GetMaterial(); }
    
    // set/get bond material
    void SetBondMaterial(FEMaterial* pm) { ReplaceProperty(1, pm); }
    FEMaterial* GetBondMaterial() { return GetProperty(1).GetMaterial(); }
    
    // set/get relaxation
    void SetRelaxation(FEMaterial* pm) { ReplaceProperty(2, pm); }
    FEMaterial* GetRelaxation() { return GetProperty(2).GetMaterial(); }
    
public:
    DECLARE_REGISTERED(FEReactiveViscoelasticMaterial);
};

//-----------------------------------------------------------------------------
// uncoupled reactive viscoelastic material
class FEReactiveViscoelasticMaterialUC : public FEMaterial
{
public:
    // material parameters
    enum { MP_KNTCS, MP_TRGGR };
    
public:
    // constructor
    FEReactiveViscoelasticMaterialUC();
    
    // set/get elastic material
    void SetElasticMaterial(FEMaterial* pm) { ReplaceProperty(0, pm); }
    FEMaterial* GetElasticMaterial() { return GetProperty(0).GetMaterial(); }
    
    // set/get bond material
    void SetBondMaterial(FEMaterial* pm) { ReplaceProperty(1, pm); }
    FEMaterial* GetBondMaterial() { return GetProperty(1).GetMaterial(); }
    
    // set/get relaxation
    void SetRelaxation(FEMaterial* pm) { ReplaceProperty(2, pm); }
    FEMaterial* GetRelaxation() { return GetProperty(2).GetMaterial(); }
    
public:
    DECLARE_REGISTERED(FEReactiveViscoelasticMaterialUC);
};

//-----------------------------------------------------------------------------
// The FEReactantMaterial is used as a component of a chemical reaction material
class FEReactantMaterial : public FEMaterial
{
public:
	// material parameters
	enum { MP_NU , MP_TYPE , MP_ID };
    
public:
	FEReactantMaterial();
    
	// get/set type (solute or sbm)
	void SetReactantType(int i) { SetIntValue(MP_TYPE, i); }
	int GetReactantType() { return GetIntValue(MP_TYPE); }
    
	// get/set (solute or sbm) index
	void SetIndex(int i) { SetIntValue(MP_ID, i); }
	int GetIndex() { return GetIntValue(MP_ID); }
    
	// get stoichiometric coefficient
	int GetCoef() { return GetIntValue(MP_NU); }
	void SetCoeff(int n) { SetIntValue(MP_NU, n); }

protected:
	DECLARE_REGISTERED(FEReactantMaterial);
};

//-----------------------------------------------------------------------------
// The FEProductMaterial is used as a component of a chemical reaction material
class FEProductMaterial : public FEMaterial
{
public:
	// material parameters
	enum { MP_NU , MP_TYPE , MP_ID };
    
public:
	FEProductMaterial();
    
	// get/set type (solute or sbm)
	void SetProductType(int i) { SetIntValue(MP_TYPE, i); }
	int GetProductType() { return GetIntValue(MP_TYPE); }
    
	// get/set (solute or sbm) index
	void SetIndex(int i) { SetIntValue(MP_ID, i); }
	int GetIndex() { return GetIntValue(MP_ID); }
	
	// get stoichiometric coefficient
	int GetCoef() { return GetIntValue(MP_NU); }
	void SetCoeff(int n) { SetIntValue(MP_NU, n); }

protected:
	DECLARE_REGISTERED(FEProductMaterial);
};

//-----------------------------------------------------------------------------
// chemical reaction parent class
class FEReactionMaterial : public FEMaterial
{
public:
	// material parameters
	enum { MP_VBAR , MP_OVRD };

public:
	// types for the rectant and product species
	enum SpeciesType{
		SOLUTE_SPECIES = 1,
		SBM_SPECIES    = 2
	};
    
public:
    FEReactionMaterial(int ntype);

    void SetOvrd(bool bovrd);

    bool GetOvrd();
    
	// set forward rate
	void SetForwardRate(FEMaterial* pm);
	FEMaterial* GetForwardRate();

	// set reverse rate
	void SetReverseRate(FEMaterial* pm);
	FEMaterial* GetReverseRate();

	int Reactants();
	FEReactantMaterial* Reactant(int i);

	int Products();
	FEProductMaterial* Product(int i);

	// add reactant/product component
	void AddReactantMaterial(FEReactantMaterial* pm);
	void AddProductMaterial(FEProductMaterial* pm);

	void GetSoluteReactants(vector<int>& solR);
	void GetSBMReactants(vector<int>& sbmR);
	void GetSoluteProducts(vector<int>& solP);
	void GetSBMProducts(vector<int>& sbmP);

	void ClearReactants();
	void ClearProducts();
};

string buildReactionEquation(FEReactionMaterial* mat, FEModel& fem);

//-----------------------------------------------------------------------------
// multiphasic
class FEMultiphasicMaterial : public FEMultiMaterial
{
public:
	// material parameters
	enum { MP_PHI0, MP_CHARGE };

	// material properties
	enum { SOLID, PERM, OSMC, SOLUTE, SBM, REACTION };
    
public:
	FEMultiphasicMaterial();

	// set/get elastic component 
	void SetSolidMaterial(FEMaterial* pm);

	// set/get permeability
	void SetPermeability(FEMaterial* pm);

	// set/get osmotic coefficient
	void SetOsmoticCoefficient(FEMaterial* pm);

	// add solute component
	void AddSoluteMaterial(FESoluteMaterial* pm);

	// add SBM component
	void AddSBMMaterial(FESBMMaterial* pm);
    
	// add chemical reaction component
	void AddReactionMaterial(FEReactionMaterial* pm);
 
    // get solute global index from local index
    int GetSoluteIndex(const int isol);
     
    // get SBM global index from local index
    int GetSBMIndex(const int isbm);
    
    // count reaction components
    int Reactions();

	// see if this material has a solute with global ID
	bool HasSolute(int nid);

	// see if this material has a sbm with global ID
	bool HasSBM(int nid);
    
    // get reaction component
    FEReactionMaterial* GetReaction(int n);
    
	DECLARE_REGISTERED(FEMultiphasicMaterial);
};

//-----------------------------------------------------------------------------
// mass action forward chemical reaction
class FEMassActionForward : public FEReactionMaterial
{
public:
	FEMassActionForward();

protected:
	DECLARE_REGISTERED(FEMassActionForward);
};

//-----------------------------------------------------------------------------
// mass action reversible chemical reaction
class FEMassActionReversible : public FEReactionMaterial
{
public:
	FEMassActionReversible();
    
protected:
	DECLARE_REGISTERED(FEMassActionReversible);
};

//-----------------------------------------------------------------------------
// Michaelis-Menten chemical reaction
class FEMichaelisMenten : public FEReactionMaterial
{
public:
	// material parameters
	enum { MP_KM, MP_C0 };
    
public:
	FEMichaelisMenten();
    
protected:
	DECLARE_REGISTERED(FEMichaelisMenten);
};

//-----------------------------------------------------------------------------
// fluid
class FEFluidMaterial : public FEMaterial
{
public:
    // material parameters
    enum { MP_RHO, MP_K };
    
public:
    // constructor
    FEFluidMaterial();
    
    // set/get viscous component
    void SetViscousMaterial(FEMaterial* pm) { ReplaceProperty(0, pm); }
    FEMaterial* GetViscousMaterial() { return GetProperty(0).GetMaterial(); }
    
    DECLARE_REGISTERED(FEFluidMaterial);
};

//-----------------------------------------------------------------------------
// fluid FSI
class FEFluidFSIMaterial : public FEMultiMaterial
{
public:
    // constructor
    FEFluidFSIMaterial();
    
    // set/get fluid component
    void SetFluidMaterial(FEMaterial* pm) { ReplaceProperty(0, pm); }
    FEMaterial* GetFluidMaterial() { return GetProperty(0).GetMaterial(); }
    
    // set/get solid component
    void SetSolidMaterial(FEMaterial* pm) { ReplaceProperty(1, pm); }
    FEMaterial* GetSolidMaterial() { return GetProperty(1).GetMaterial(); }
    
    DECLARE_REGISTERED(FEFluidFSIMaterial);
};

//-----------------------------------------------------------------------------
class FESpeciesMaterial : public FEMaterial
{
public:
	// parameters
	enum {MP_SOL, MP_DIFFUSIVITY};

public:
	FESpeciesMaterial();

	int GetSpeciesIndex();
	void SetSpeciesIndex(int n);

	DECLARE_REGISTERED(FESpeciesMaterial);
};

//-----------------------------------------------------------------------------
class FESolidSpeciesMaterial : public FEMaterial
{
public:
	enum { MP_NSBM, MP_RHO0, MP_RMIN, MP_RMAX };

public:
	FESolidSpeciesMaterial();

	// get/set solid-bound molecule index
	void SetSBMIndex(int i) { SetIntValue(MP_NSBM, i); }
	int GetSBMIndex() { return GetIntValue(MP_NSBM); }

protected:
	DECLARE_REGISTERED(FESolidSpeciesMaterial);
};

//-----------------------------------------------------------------------------
// Reaction-diffusion
class FEReactionDiffusionMaterial : public FEMaterial
{
public:
	// material parameters
	enum {MP_RHO};

	void AddSpeciesMaterial(FESpeciesMaterial* pm);

	void AddSolidSpeciesMaterial(FESolidSpeciesMaterial* pm);

	void AddReactionMaterial(FEReactionMaterial* pm);

public:
	// constructor
	FEReactionDiffusionMaterial();

	DECLARE_REGISTERED(FEReactionDiffusionMaterial);
};

//-----------------------------------------------------------------------------
// generation
class FEGeneration : public FEMaterial
{
public:
    // material parameters
    enum { MP_ST };
    
public:
    // constructor
    FEGeneration();
    
    // set/get solid component
    void SetSolidMaterial(FEMaterial* pm) { ReplaceProperty(0, pm); }
    FEMaterial* GetSolidMaterial() { return GetProperty(0).GetMaterial(); }
    
    DECLARE_REGISTERED(FEGeneration);
};

//-----------------------------------------------------------------------------
// multigeneration material class
class FEMultiGeneration : public FEMaterial
{
public:
    FEMultiGeneration();
    
    DECLARE_REGISTERED(FEMultiGeneration);
};

//-----------------------------------------------------------------------------
// prestrain material
class FEPrestrainMaterial : public FEMaterial
{
public:
	// constructor
	FEPrestrainMaterial();

	// set the elastic component of the material
	void SetElasticMaterial(FEMaterial* pm) { ReplaceProperty(0, pm); }
	FEMaterial* GetElasticMaterial() { return GetProperty(0).GetMaterial(); }

	DECLARE_REGISTERED(FEPrestrainMaterial);
};

//-----------------------------------------------------------------------------
// uncoupled prestrain material
class FEUncoupledPrestrainMaterial : public FEMaterial
{
public:
	// constructor
	FEUncoupledPrestrainMaterial();

	// set the elastic component of the material
	void SetElasticMaterial(FEMaterial* pm) { ReplaceProperty(0, pm); }
	FEMaterial* GetElasticMaterial() { return GetProperty(0).GetMaterial(); }

	DECLARE_REGISTERED(FEUncoupledPrestrainMaterial);
};
