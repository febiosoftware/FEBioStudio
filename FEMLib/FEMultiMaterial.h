#pragma once
#include "FEMaterial.h"
#include <vector>

class FSMultiphasicMaterial;
class FSModel;

//-----------------------------------------------------------------------------
// visco-elastic
class FSViscoElastic : public FSMaterial
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
	FSViscoElastic(FSModel* fem);

	// set the elastic component of the material
	void SetElasticMaterial(FSMaterial* pm) { ReplaceProperty(0, pm); }
	FSMaterial* GetElasticMaterial() { return GetMaterialProperty(0); }

	DECLARE_REGISTERED(FSViscoElastic);
};


//-----------------------------------------------------------------------------
// uncoupled visco-elastic
class FSUncoupledViscoElastic : public FSMaterial
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
	FSUncoupledViscoElastic(FSModel* fem);

	// set the elastic component of the material
	void SetElasticMaterial(FSMaterial* pm) { ReplaceProperty(0, pm); }
	FSMaterial* GetElasticMaterial() { return GetMaterialProperty(0); }

	DECLARE_REGISTERED(FSUncoupledViscoElastic);
};

//-----------------------------------------------------------------------------
// The FSMultiMaterial class is used as a base class for materials that define
// material properties for multi-physics problems. 
class FSMultiMaterial : public FSMaterial
{
public:
	FSMultiMaterial(int ntype, FSModel* fem);
};

//-----------------------------------------------------------------------------
// biphasic
class FSBiphasic : public FSMultiMaterial
{
public:
	// material parameters
	enum { MP_PHI0, MP_RHOTW };

public:
	// constructor
	FSBiphasic(FSModel* fem);

	// set/get solid component 
	void SetSolidMaterial(FSMaterial* pm) { ReplaceProperty(0, pm); }
	FSMaterial* GetSolidMaterial() { return GetMaterialProperty(0); }

	// set/get permeability
	void SetPermeability(FSMaterial* pm) { ReplaceProperty(1, pm); }
	FSMaterial* GetPermeability() { return GetMaterialProperty(1); }

	DECLARE_REGISTERED(FSBiphasic);
};


//-----------------------------------------------------------------------------
// The FSSoluteMaterial is used as a component of a multiphasic material
class FSSoluteMaterial : public FSMaterialProp
{
public:
	enum { MP_NSOL };

public:
	FSSoluteMaterial(FSModel* fem);

	// set/get diffusivity
	void SetDiffusivity(FSMaterial* pm) { ReplaceProperty(0, pm); }
	FSMaterial* GetDiffusivity() { return GetMaterialProperty(0); }

	// set/get solubility
	void SetSolubility(FSMaterial* pm) { ReplaceProperty(1, pm); }
	FSMaterial* GetSolubility() { return GetMaterialProperty(1); }

	// get/set solute index
	void SetSoluteIndex(int i) { SetIntValue(MP_NSOL, i); }
	int GetSoluteIndex() { return GetIntValue(MP_NSOL); }

protected:
	DECLARE_REGISTERED(FSSoluteMaterial);
};

//-----------------------------------------------------------------------------
// The FSSBMMaterial is used as a component of a multiphasic material
class FSSBMMaterial : public FSMaterialProp
{
public:
	enum { MP_NSBM , MP_RHO0 , MP_RMIN , MP_RMAX };
    
public:
	FSSBMMaterial(FSModel* fem);
    
	// get/set solid-bound molecule index
	void SetSBMIndex(int i) { SetIntValue(MP_NSBM, i); }
	int GetSBMIndex() { return GetIntValue(MP_NSBM); }

protected:
	DECLARE_REGISTERED(FSSBMMaterial);
};

//-----------------------------------------------------------------------------
// biphasic-solute
class FSBiphasicSolute : public FSMultiMaterial
{
public:
	// material parameters
	enum {
		MP_PHI0
	};

public:
	FSBiphasicSolute(FSModel* fem);

	// set/get elastic component 
	void SetSolidMaterial(FSMaterial* pm) { ReplaceProperty(0, pm); }
	FSMaterial* GetSolidMaterial() { return GetMaterialProperty(0); }

	// set/get permeability
	void SetPermeability(FSMaterial* pm) { ReplaceProperty(1, pm); }
	FSMaterial* GetPermeability() { return GetMaterialProperty(1); }

	// set/get osmotic coefficient
	void SetOsmoticCoefficient(FSMaterial* pm) { ReplaceProperty(2, pm); }
	FSMaterial* GetOsmoticCoefficient() { return GetMaterialProperty(2); }

	// set/get solute
	void SetSoluteMaterial(FSSoluteMaterial* pm) { ReplaceProperty(3, pm); }
	FSMaterial* GetSoluteMaterial() { return GetMaterialProperty(3); }

	DECLARE_REGISTERED(FSBiphasicSolute);
};

//-----------------------------------------------------------------------------
// triphasic
class FSTriphasicMaterial : public FSMultiMaterial
{
public:
	// material parameters
	enum { MP_PHI0, MP_CHARGE };

public:
	FSTriphasicMaterial(FSModel* fem);

	// set/get elastic component 
	void SetSolidMaterial(FSMaterial* pm) { ReplaceProperty(0, pm); }
	FSMaterial* GetSolidMaterial() { return GetMaterialProperty(0); }

	// set/get permeability
	void SetPermeability(FSMaterial* pm) { ReplaceProperty(1, pm); }
	FSMaterial* GetPermeability() { return GetMaterialProperty(1); }

	// set/get osmotic coefficient
	void SetOsmoticCoefficient(FSMaterial* pm) { ReplaceProperty(2, pm); }
	FSMaterial* GetOsmoticCoefficient() { return GetMaterialProperty(2); }

	// set/get solute i
	void SetSoluteMaterial(FSSoluteMaterial* pm, int i);
	FSMaterial* GetSoluteMaterial(int i);

	DECLARE_REGISTERED(FSTriphasicMaterial);
};

//-----------------------------------------------------------------------------
// solid mixture class
class FSSolidMixture : public FSMaterial
{
public:
	FSSolidMixture(FSModel* fem);

	DECLARE_REGISTERED(FSSolidMixture);
};

//-----------------------------------------------------------------------------
// uncoupled solid mixture class
class FSUncoupledSolidMixture : public FSMaterial
{
public:
	FSUncoupledSolidMixture(FSModel* fem);

	DECLARE_REGISTERED(FSUncoupledSolidMixture);
};

//-----------------------------------------------------------------------------
// continuous fiber distribution
class FSCFDMaterial : public FSMaterial
{
public:
    // constructor
    FSCFDMaterial(FSModel* fem);
    
    // set/get fiber material
    void SetFiberMaterial(FSMaterial* pm) { ReplaceProperty(0, pm); }
    FSMaterial* GetFiberMaterial() { return GetMaterialProperty(0); }
    
    // set/get distribution
    void SetDistribution(FSMaterial* pm) { ReplaceProperty(1, pm); }
    FSMaterial* GetDistribution() { return GetMaterialProperty(1); }
    
    // set/get scheme
    void SetScheme(FSMaterial* pm) { ReplaceProperty(2, pm); }
    FSMaterial* GetScheme() { return GetMaterialProperty(2); }
    
public:
    DECLARE_REGISTERED(FSCFDMaterial);
};

//-----------------------------------------------------------------------------
// continuous fiber distribution uncoupled
class FSCFDUCMaterial : public FSMaterial
{
public:
    // constructor
    FSCFDUCMaterial(FSModel* fem);
    
    // set/get fiber material
    void SetFiberMaterial(FSMaterial* pm) { ReplaceProperty(0, pm); }
    FSMaterial* GetFiberMaterial() { return GetMaterialProperty(0); }
    
    // set/get distribution
    void SetDistribution(FSMaterial* pm) { ReplaceProperty(1, pm); }
    FSMaterial* GetDistribution() { return GetMaterialProperty(1); }
    
    // set/get scheme
    void SetScheme(FSMaterial* pm) { ReplaceProperty(2, pm); }
    FSMaterial* GetScheme() { return GetMaterialProperty(2); }
    
public:
    DECLARE_REGISTERED(FSCFDUCMaterial);
};

//-----------------------------------------------------------------------------
// elastic damage material
class FSElasticDamageMaterial : public FSMaterial
{
public:
    // constructor
    FSElasticDamageMaterial(FSModel* fem);
    
    // set/get elastic material
    void SetElasticMaterial(FSMaterial* pm) { ReplaceProperty(0, pm); }
    FSMaterial* GetElasticMaterial() { return GetMaterialProperty(0); }
    
    // set/get damage material
    void SetDamageMaterial(FSMaterial* pm) { ReplaceProperty(1, pm); }
    FSMaterial* GetDamageMaterial() { return GetMaterialProperty(1); }
    
    // set/get criterion
    void SetCriterion(FSMaterial* pm) { ReplaceProperty(2, pm); }
    FSMaterial* GetCriterion() { return GetMaterialProperty(2); }
    
public:
    DECLARE_REGISTERED(FSElasticDamageMaterial);
};

//-----------------------------------------------------------------------------
// uncoupled elastic damage material
class FSElasticDamageMaterialUC : public FSMaterial
{
public:
    // constructor
    FSElasticDamageMaterialUC(FSModel* fem);
    
    // set/get elastic material
    void SetElasticMaterial(FSMaterial* pm) { ReplaceProperty(0, pm); }
    FSMaterial* GetElasticMaterial() { return GetMaterialProperty(0); }
    
    // set/get damage material
    void SetDamageMaterial(FSMaterial* pm) { ReplaceProperty(1, pm); }
    FSMaterial* GetDamageMaterial() { return GetMaterialProperty(1); }
    
    // set/get criterion
    void SetCriterion(FSMaterial* pm) { ReplaceProperty(2, pm); }
    FSMaterial* GetCriterion() { return GetMaterialProperty(2); }
    
public:
    DECLARE_REGISTERED(FSElasticDamageMaterialUC);
};

//-----------------------------------------------------------------------------
// reactive viscoelastic material
class FSReactiveViscoelasticMaterial : public FSMaterial
{
public:
    // material parameters
    enum { MP_KNTCS, MP_TRGGR, MP_WMIN, MP_EMIN };
    
public:
    // constructor
    FSReactiveViscoelasticMaterial(FSModel* fem);
    
    // set/get elastic material
    void SetElasticMaterial(FSMaterial* pm) { ReplaceProperty(0, pm); }
    FSMaterial* GetElasticMaterial() { return GetMaterialProperty(0); }
    
    // set/get bond material
    void SetBondMaterial(FSMaterial* pm) { ReplaceProperty(1, pm); }
    FSMaterial* GetBondMaterial() { return GetMaterialProperty(1); }
    
    // set/get relaxation
    void SetRelaxation(FSMaterial* pm) { ReplaceProperty(2, pm); }
    FSMaterial* GetRelaxation() { return GetMaterialProperty(2); }
    
    // set/get recruitment
    void SetRecrutiment(FSMaterial* pm) { ReplaceProperty(3, pm); }
    FSMaterial* GetRecruitment() { return GetMaterialProperty(3); }

public:
    DECLARE_REGISTERED(FSReactiveViscoelasticMaterial);
};

//-----------------------------------------------------------------------------
// uncoupled reactive viscoelastic material
class FSReactiveViscoelasticMaterialUC : public FSMaterial
{
public:
    // material parameters
    enum { MP_KNTCS, MP_TRGGR, MP_WMIN, MP_EMIN };

public:
    // constructor
    FSReactiveViscoelasticMaterialUC(FSModel* fem);
    
    // set/get elastic material
    void SetElasticMaterial(FSMaterial* pm) { ReplaceProperty(0, pm); }
    FSMaterial* GetElasticMaterial() { return GetMaterialProperty(0); }
    
    // set/get bond material
    void SetBondMaterial(FSMaterial* pm) { ReplaceProperty(1, pm); }
    FSMaterial* GetBondMaterial() { return GetMaterialProperty(1); }
    
    // set/get relaxation
    void SetRelaxation(FSMaterial* pm) { ReplaceProperty(2, pm); }
    FSMaterial* GetRelaxation() { return GetMaterialProperty(2); }
    
    // set/get recruitment
    void SetRecrutiment(FSMaterial* pm) { ReplaceProperty(3, pm); }
    FSMaterial* GetRecruitment() { return GetMaterialProperty(3); }
    
public:
    DECLARE_REGISTERED(FSReactiveViscoelasticMaterialUC);
};

//-----------------------------------------------------------------------------
// reactive viscoelastic damage material
class FSRVDamageMaterial : public FSMaterial
{
public:
    // material parameters
    enum { MP_KNTCS, MP_TRGGR, MP_WMIN, MP_EMIN };
    
public:
    // constructor
    FSRVDamageMaterial(FSModel* fem);
    
    // set/get elastic material
    void SetElasticMaterial(FSMaterial* pm) { ReplaceProperty(0, pm); }
    FSMaterial* GetElasticMaterial() { return GetMaterialProperty(0); }
    
    // set/get bond material
    void SetBondMaterial(FSMaterial* pm) { ReplaceProperty(1, pm); }
    FSMaterial* GetBondMaterial() { return GetMaterialProperty(1); }
    
    // set/get relaxation
    void SetRelaxation(FSMaterial* pm) { ReplaceProperty(2, pm); }
    FSMaterial* GetRelaxation() { return GetMaterialProperty(2); }
    
    // set/get damage material
    void SetDamageMaterial(FSMaterial* pm) { ReplaceProperty(3, pm); }
    FSMaterial* GetDamageMaterial() { return GetMaterialProperty(3); }
    
    // set/get criterion
    void SetCriterion(FSMaterial* pm) { ReplaceProperty(4, pm); }
    FSMaterial* GetCriterion() { return GetMaterialProperty(4); }

public:
    DECLARE_REGISTERED(FSRVDamageMaterial);
};

//-----------------------------------------------------------------------------
class FSReactionSpecies : public FSMaterialProp
{
public:
    // material parameters
    enum { MP_NU, MP_TYPE, MP_ID };

    // types for the rectant and product species
    enum SpeciesType {
        SOLUTE_SPECIES = 1,
        SBM_SPECIES = 2
    };

public:
    FSReactionSpecies(int ntype, FSModel* fem);

    // get/set type (solute or sbm)
    void SetSpeciesType(int i) { SetIntValue(MP_TYPE, i); }
    int GetSpeciesType() const { return GetIntValue(MP_TYPE); }

    // get/set (solute or sbm) index
    void SetIndex(int i) { SetIntValue(MP_ID, i); }
    int GetIndex() const { return GetIntValue(MP_ID); }

    // get stoichiometric coefficient
    int GetCoef() const { return GetIntValue(MP_NU); }
    void SetCoeff(int n) { SetIntValue(MP_NU, n); }

private:
    void Load(IArchive& ar) override;
};

//-----------------------------------------------------------------------------
// The FSReactantMaterial is used as a component of a chemical reaction material
class FSReactantMaterial : public FSReactionSpecies
{
public:
	FSReactantMaterial(FSModel* fem);
    int GetReactantType() { return GetSpeciesType(); }
    void SetReactantType(int i) { SetSpeciesType(i); }

protected:
	DECLARE_REGISTERED(FSReactantMaterial);
};

//-----------------------------------------------------------------------------
// The FSProductMaterial is used as a component of a chemical reaction material
class FSProductMaterial : public FSReactionSpecies
{
public:
    FSProductMaterial(FSModel* fem);
    int GetProductType() { return GetSpeciesType(); }
    void SetProductType(int i) { SetSpeciesType(i); }

protected:
    DECLARE_REGISTERED(FSProductMaterial);
};

//-----------------------------------------------------------------------------
// The FSInternalReactantMaterial is used as a component of a membrane reaction material
class FSInternalReactantMaterial : public FSReactionSpecies
{
public:
    FSInternalReactantMaterial(FSModel* fem);
    void SetReactantType(int i) { SetSpeciesType(i); }
    int GetReactantType() { return GetSpeciesType(); }
   
protected:
    DECLARE_REGISTERED(FSInternalReactantMaterial);
};

//-----------------------------------------------------------------------------
// The FSExternalReactantMaterial is used as a component of a membrane reaction material
class FSExternalReactantMaterial : public FSReactionSpecies
{
public:
    FSExternalReactantMaterial(FSModel* fem);
    void SetReactantType(int i) { SetSpeciesType(i); }
    int GetReactantType() { return GetSpeciesType(); }

protected:
    DECLARE_REGISTERED(FSExternalReactantMaterial);
};

//-----------------------------------------------------------------------------
// The FSInternalProductMaterial is used as a component of a membrane reaction material
class FSInternalProductMaterial : public FSReactionSpecies
{
public:
    FSInternalProductMaterial(FSModel* fem);
    void SetProductType(int i) { SetSpeciesType(i); }
    int GetProductType() { return GetSpeciesType(); }
   
protected:
    DECLARE_REGISTERED(FSInternalProductMaterial);
};

//-----------------------------------------------------------------------------
// The FSExternalProductMaterial is used as a component of a membrane reaction material
class FSExternalProductMaterial : public FSReactionSpecies
{
public:
    FSExternalProductMaterial(FSModel* fem);
    void SetProductType(int i) { SetSpeciesType(i); }
    int GetProductType() { return GetSpeciesType(); }
    
protected:
    DECLARE_REGISTERED(FSExternalProductMaterial);
};

//-----------------------------------------------------------------------------
// chemical reaction parent class
class FSReactionMaterial : public FSMaterialProp
{
public:
	// material parameters
	enum { MP_VBAR , MP_OVRD };

public:
    FSReactionMaterial(int ntype, FSModel* fem);

    void SetOvrd(bool bovrd);

    bool GetOvrd();
    
	// set forward rate
	void SetForwardRate(FSMaterial* pm);
	FSMaterial* GetForwardRate();

	// set reverse rate
	void SetReverseRate(FSMaterial* pm);
	FSMaterial* GetReverseRate();

	int Reactants();
	FSReactantMaterial* Reactant(int i);

	int Products();
	FSProductMaterial* Product(int i);

	// add reactant/product component
	void AddReactantMaterial(FSReactantMaterial* pm);
	void AddProductMaterial(FSProductMaterial* pm);

	void GetSoluteReactants(std::vector<int>& solR);
	void GetSBMReactants(std::vector<int>& sbmR);
	void GetSoluteProducts(std::vector<int>& solP);
	void GetSBMProducts(std::vector<int>& sbmP);

	void ClearReactants();
	void ClearProducts();
};

std::string buildReactionEquation(FSReactionMaterial* mat, FSModel& fem);

// helper class for managing chemical reactions using the FEBio classes
class FEBioReactionMaterial
{
public:
    FEBioReactionMaterial(FSMaterialProperty* reaction);

    void SetReaction(FSMaterialProperty* reaction);

    void SetForwardRate(FSMaterialProperty* fwdRate);

    void SetReverseRate(FSMaterialProperty* revRate);

    int Type() const;

    void SetName(const std::string& name);

    std::string GetName() const;

    operator FSCoreBase* () { return m_reaction; }

    bool IsReversible() const;

    FSMaterialProperty* GetForwardRate();

    FSMaterialProperty* GetReverseRate();

    bool GetOvrd() const;

    void SetOvrd(bool b);

    int Reactants() const;

    int Products() const;

    FSMaterialProperty* Reactant(int n);

    FSMaterialProperty* Product(int n);

    void AddReactantMaterial(FSMaterialProperty* pm);
    void AddProductMaterial(FSMaterialProperty* pm);

    void GetSoluteReactants(std::vector<int>& solR);
    void GetSBMReactants(std::vector<int>& sbmR);
    void GetSoluteProducts(std::vector<int>& solP);
    void GetSBMProducts(std::vector<int>& sbmP);

    void ClearReactants();
    void ClearProducts();

private:
    FSMaterialProperty* m_reaction;
};


//-----------------------------------------------------------------------------
// helper class for managing membrane reactions using the FEBio classes
class FEBioMembraneReactionMaterial
{
public:
    FEBioMembraneReactionMaterial(FSMaterialProperty* reaction);

    void SetReaction(FSMaterialProperty* reaction);

    void SetForwardRate(FSMaterialProperty* fwdRate);

    void SetReverseRate(FSMaterialProperty* revRate);

    int Type() const;

    void SetName(const std::string& name);

    std::string GetName() const;

    operator FSCoreBase* () { return m_reaction; }

    bool IsReversible() const;

    FSMaterialProperty* GetForwardRate();

    FSMaterialProperty* GetReverseRate();

    bool GetOvrd() const;

    void SetOvrd(bool b);

    int Reactants() const;
    int InternalReactants() const;
    int ExternalReactants() const;

    int Products() const;
    int InternalProducts() const;
    int ExternalProducts() const;

    FSMaterialProperty* Reactant(int n);
    FSMaterialProperty* InternalReactant(int n);
    FSMaterialProperty* ExternalReactant(int n);

    FSMaterialProperty* Product(int n);
    FSMaterialProperty* InternalProduct(int n);
    FSMaterialProperty* ExternalProduct(int n);

    void AddReactantMaterial(FSMaterialProperty* pm);
    void AddProductMaterial(FSMaterialProperty* pm);

    void GetSoluteReactants(std::vector<int>& solR);
    void GetInternalSoluteReactants(std::vector<int>& solRi);
    void GetExternalSoluteReactants(std::vector<int>& solRe);
    void GetSBMReactants(std::vector<int>& sbmR);
    void GetSoluteProducts(std::vector<int>& solP);
    void GetInternalSoluteProducts(std::vector<int>& solPi);
    void GetExternalSoluteProducts(std::vector<int>& solPe);
    void GetSBMProducts(std::vector<int>& sbmP);

    void ClearReactants();
    void ClearProducts();

private:
    FSMaterialProperty* m_reaction;
};

//-----------------------------------------------------------------------------
// helper class for handling FEBio multiphasic materials
class FEBioMultiphasic
{
public:
    FEBioMultiphasic(FSMaterial* mat);

    bool HasSolute(int solID) const;

    bool HasSBM(int solID) const;

private:
    FSMaterial* m_mat;
};

//-----------------------------------------------------------------------------
// membrane reaction parent class
class FSMembraneReactionMaterial : public FSMaterial
{
public:
    // material parameters
    enum { MP_VBAR , MP_OVRD };
    
public:
    FSMembraneReactionMaterial(int ntype, FSModel* fem);
    
    void SetOvrd(bool bovrd);
    
    bool GetOvrd();
    
    // set forward rate
    void SetForwardRate(FSMaterial* pm);
    FSMaterial* GetForwardRate();
    
    // set reverse rate
    void SetReverseRate(FSMaterial* pm);
    FSMaterial* GetReverseRate();
    
    int Reactants();
    FSReactantMaterial* Reactant(int i);
    
    int Products();
    FSProductMaterial* Product(int i);

    int InternalReactants();
    FSInternalReactantMaterial* InternalReactant(int i);
    
    int InternalProducts();
    FSInternalProductMaterial* InternalProduct(int i);
    
    int ExternalReactants();
    FSExternalReactantMaterial* ExternalReactant(int i);
    
    int ExternalProducts();
    FSExternalProductMaterial* ExternalProduct(int i);
    
    // add reactant/product component
    void AddReactantMaterial(FSReactantMaterial* pm);
    void AddProductMaterial(FSProductMaterial* pm);
    void AddInternalReactantMaterial(FSInternalReactantMaterial* pm);
    void AddInternalProductMaterial(FSInternalProductMaterial* pm);
    void AddExternalReactantMaterial(FSExternalReactantMaterial* pm);
    void AddExternalProductMaterial(FSExternalProductMaterial* pm);

    void GetSoluteReactants(std::vector<int>& solR);
    void GetSBMReactants(std::vector<int>& sbmR);
    void GetSoluteProducts(std::vector<int>& solP);
    void GetSBMProducts(std::vector<int>& sbmP);
    void GetInternalSoluteReactants(std::vector<int>& solRi);
    void GetInternalSoluteProducts(std::vector<int>& solPi);
    void GetExternalSoluteReactants(std::vector<int>& solRe);
    void GetExternalSoluteProducts(std::vector<int>& solPe);

    void ClearReactants();
    void ClearProducts();
};

std::string buildMembraneReactionEquation(FSMembraneReactionMaterial* mat, FSModel& fem);

//-----------------------------------------------------------------------------
// multiphasic
class FSMultiphasicMaterial : public FSMultiMaterial
{
public:
	// material parameters
	enum { MP_PHI0, MP_CHARGE };

	// material properties
	enum { SOLID, PERM, OSMC, SOLUTE, SBM, REACTION, MREACTION };
    
public:
	FSMultiphasicMaterial(FSModel* fem);

	// set/get elastic component 
	void SetSolidMaterial(FSMaterial* pm);

	// set/get permeability
	void SetPermeability(FSMaterial* pm);

	// set/get osmotic coefficient
	void SetOsmoticCoefficient(FSMaterial* pm);

	// add solute component
	void AddSoluteMaterial(FSSoluteMaterial* pm);

	// add SBM component
	void AddSBMMaterial(FSSBMMaterial* pm);
    
	// add chemical reaction component
	void AddReactionMaterial(FSReactionMaterial* pm);
 
    // add membrane reaction component
    void AddMembraneReactionMaterial(FSMembraneReactionMaterial* pm);
    
    // get solute global index from local index
    int GetSoluteIndex(const int isol);
     
    // get SBM global index from local index
    int GetSBMIndex(const int isbm);
    
    // count reaction components
    int Reactions();

    // count membrane reaction components
    int MembraneReactions();
    
	// see if this material has a solute with global ID
	bool HasSolute(int nid);

	// see if this material has a sbm with global ID
	bool HasSBM(int nid);
    
    // get reaction component
    FSReactionMaterial* GetReaction(int n);
    
    // get membrane reaction component
    FSMembraneReactionMaterial* GetMembraneReaction(int n);
    
	DECLARE_REGISTERED(FSMultiphasicMaterial);
};

//-----------------------------------------------------------------------------
// mass action chemical reaction
class FSMassActionReaction : public FSReactionMaterial
{
public:
	FSMassActionReaction(FSModel* fem);

protected:
	DECLARE_REGISTERED(FSMassActionReaction);
};


//-----------------------------------------------------------------------------
// mass action forward chemical reaction
class FSMassActionForward : public FSReactionMaterial
{
public:
	FSMassActionForward(FSModel* fem);

protected:
	DECLARE_REGISTERED(FSMassActionForward);
};

//-----------------------------------------------------------------------------
// mass action reversible chemical reaction
class FSMassActionReversible : public FSReactionMaterial
{
public:
	FSMassActionReversible(FSModel* fem);
    
protected:
	DECLARE_REGISTERED(FSMassActionReversible);
};

//-----------------------------------------------------------------------------
// Michaelis-Menten chemical reaction
class FSMichaelisMenten : public FSReactionMaterial
{
public:
	// material parameters
	enum { MP_KM, MP_C0 };
    
public:
	FSMichaelisMenten(FSModel* fem);
    
protected:
	DECLARE_REGISTERED(FSMichaelisMenten);
};

//-----------------------------------------------------------------------------
// membrane mass action forward chemical reaction
class FSMembraneMassActionForward : public FSMembraneReactionMaterial
{
public:
    FSMembraneMassActionForward(FSModel* fem);
    
protected:
    DECLARE_REGISTERED(FSMembraneMassActionForward);
};

//-----------------------------------------------------------------------------
// membrane mass action reversible chemical reaction
class FSMembraneMassActionReversible : public FSMembraneReactionMaterial
{
public:
    FSMembraneMassActionReversible(FSModel* fem);
    
protected:
    DECLARE_REGISTERED(FSMembraneMassActionReversible);
};

//-----------------------------------------------------------------------------
// fluid
class FSFluidMaterial : public FSMaterial
{
public:
    // material parameters
    enum { MP_RHO, MP_K };
    
public:
    // constructor
    FSFluidMaterial(FSModel* fem);
    
    // set/get viscous component
    void SetViscousMaterial(FSMaterial* pm) { ReplaceProperty(0, pm); }
    FSMaterial* GetViscousMaterial() { return GetMaterialProperty(0); }
    
    DECLARE_REGISTERED(FSFluidMaterial);
};

//-----------------------------------------------------------------------------
// fluid FSI
class FSFluidFSIMaterial : public FSMultiMaterial
{
public:
    // constructor
    FSFluidFSIMaterial(FSModel* fem);
    
    // set/get fluid component
    void SetFluidMaterial(FSMaterial* pm) { ReplaceProperty(0, pm); }
    FSMaterial* GetFluidMaterial() { return GetMaterialProperty(0); }
    
    // set/get solid component
    void SetSolidMaterial(FSMaterial* pm) { ReplaceProperty(1, pm); }
    FSMaterial* GetSolidMaterial() { return GetMaterialProperty(1); }
    
    DECLARE_REGISTERED(FSFluidFSIMaterial);
};

//-----------------------------------------------------------------------------
// biphasic FSI
class FSBiphasicFSIMaterial : public FSMultiMaterial
{
public:
    // material parameters
    enum { MP_PHI0 };
    
public:
    // constructor
    FSBiphasicFSIMaterial(FSModel* fem);
    
    // set/get fluid component
    void SetFluidMaterial(FSMaterial* pm) { ReplaceProperty(0, pm); }
    FSMaterial* GetFluidMaterial() { return GetMaterialProperty(0); }
    
    // set/get solid component
    void SetSolidMaterial(FSMaterial* pm) { ReplaceProperty(1, pm); }
    FSMaterial* GetSolidMaterial() { return GetMaterialProperty(1); }
    
    // set/get permeability
    void SetPermeability(FSMaterial* pm) { ReplaceProperty(2, pm); }
    FSMaterial* GetPermeability() { return GetMaterialProperty(2); }
    
    DECLARE_REGISTERED(FSBiphasicFSIMaterial);
};

//-----------------------------------------------------------------------------
// polar fluid
class FSPolarFluidMaterial : public FSMaterial
{
public:
    // material parameters
    enum { MP_RHO, MP_K, MP_KG };
    
public:
    // constructor
    FSPolarFluidMaterial(FSModel* fem);
    
    // set/get viscous component
    void SetViscousMaterial(FSMaterial* pm) { ReplaceProperty(0, pm); }
    FSMaterial* GetViscousMaterial() { return GetMaterialProperty(0); }
    
    // set/get polar viscous component
    void SetPolarViscousMaterial(FSMaterial* pm) { ReplaceProperty(1, pm); }
    FSMaterial* GetPolarViscousMaterial() { return GetMaterialProperty(1); }
    
    DECLARE_REGISTERED(FSPolarFluidMaterial);
};

//-----------------------------------------------------------------------------
class FSSpeciesMaterial : public FSMaterial
{
public:
	// parameters
	enum {MP_SOL, MP_DIFFUSIVITY};

public:
	FSSpeciesMaterial(FSModel* fem);

	int GetSpeciesIndex();
	void SetSpeciesIndex(int n);

	DECLARE_REGISTERED(FSSpeciesMaterial);
};

//-----------------------------------------------------------------------------
class FSSolidSpeciesMaterial : public FSMaterial
{
public:
	enum { MP_NSBM, MP_RHO0, MP_RMIN, MP_RMAX };

public:
	FSSolidSpeciesMaterial(FSModel* fem);

	// get/set solid-bound molecule index
	void SetSBMIndex(int i) { SetIntValue(MP_NSBM, i); }
	int GetSBMIndex() { return GetIntValue(MP_NSBM); }

protected:
	DECLARE_REGISTERED(FSSolidSpeciesMaterial);
};

//-----------------------------------------------------------------------------
// Reaction-diffusion
class FSReactionDiffusionMaterial : public FSMaterial
{
public:
	// material parameters
	enum {MP_RHO};

	void AddSpeciesMaterial(FSSpeciesMaterial* pm);

	void AddSolidSpeciesMaterial(FSSolidSpeciesMaterial* pm);

	void AddReactionMaterial(FSReactionMaterial* pm);

public:
	// constructor
	FSReactionDiffusionMaterial(FSModel* fem);

	DECLARE_REGISTERED(FSReactionDiffusionMaterial);
};

//-----------------------------------------------------------------------------
// generation
class FSGeneration : public FSMaterialProp
{
public:
    // material parameters
    enum { MP_ST };
    
public:
    // constructor
    FSGeneration(FSModel* fem);
    
    // set/get solid component
    void SetSolidMaterial(FSMaterial* pm) { ReplaceProperty(0, pm); }
    FSMaterial* GetSolidMaterial() { return GetMaterialProperty(0); }
    
    DECLARE_REGISTERED(FSGeneration);
};

//-----------------------------------------------------------------------------
// multigeneration material class
class FSMultiGeneration : public FSMaterial
{
public:
    FSMultiGeneration(FSModel* fem);
    
    DECLARE_REGISTERED(FSMultiGeneration);
};

//-----------------------------------------------------------------------------
// prestrain material
class FSPrestrainMaterial : public FSMaterial
{
public:
	// constructor
	FSPrestrainMaterial(FSModel* fem);

	// set the elastic component of the material
	void SetElasticMaterial(FSMaterial* pm) { ReplaceProperty(0, pm); }
	FSMaterial* GetElasticMaterial() { return GetMaterialProperty(0); }

	DECLARE_REGISTERED(FSPrestrainMaterial);
};

//-----------------------------------------------------------------------------
// uncoupled prestrain material
class FSUncoupledPrestrainMaterial : public FSMaterial
{
public:
	// constructor
	FSUncoupledPrestrainMaterial(FSModel* fem);

	// set the elastic component of the material
	void SetElasticMaterial(FSMaterial* pm) { ReplaceProperty(0, pm); }
	FSMaterial* GetElasticMaterial() { return GetMaterialProperty(0); }

	DECLARE_REGISTERED(FSUncoupledPrestrainMaterial);
};

//-----------------------------------------------------------------------------
// reactive plasticity
class FSReactivePlasticity : public FSMaterial
{
public:
    // material parameters
    enum { MP_ISCHRC };
    
public:
    // constructor
    FSReactivePlasticity(FSModel* fem);
    
    // set the elastic component of the material
    void SetElasticMaterial(FSMaterial* pm) { ReplaceProperty(0, pm); }
    FSMaterial* GetElasticMaterial() { return GetMaterialProperty(0); }
    
    // set/get yield criterion
    void SetCriterion(FSMaterial* pm) { ReplaceProperty(1, pm); }
    FSMaterial* GetCriterion() { return GetMaterialProperty(1); }

    // set/get flow curve
    void SetFlowCurve(FSMaterial* pm) { ReplaceProperty(2, pm); }
    FSMaterial* GetFlowCurve() { return GetMaterialProperty(2); }
    
    DECLARE_REGISTERED(FSReactivePlasticity);
};


//-----------------------------------------------------------------------------
// reactive plastic damage
class FSReactivePlasticDamage : public FSMaterial
{
public:
    // material parameters
    enum { MP_ISCHRC };
    
public:
    // constructor
    FSReactivePlasticDamage(FSModel* fem);
    
    // set the elastic component of the material
    void SetElasticMaterial(FSMaterial* pm) { ReplaceProperty(0, pm); }
    FSMaterial* GetElasticMaterial() { return GetMaterialProperty(0); }
    
    // set/get yield criterion
    void SetYieldCriterion(FSMaterial* pm) { ReplaceProperty(1, pm); }
    FSMaterial* GetYieldCriterion() { return GetMaterialProperty(1); }
    
    // set/get flow curve
    void SetFlowCurve(FSMaterial* pm) { ReplaceProperty(2, pm); }
    FSMaterial* GetFlowCurve() { return GetMaterialProperty(2); }
    
    // set/get yield damage material
    void SetYieldDamageMaterial(FSMaterial* pm) { ReplaceProperty(3, pm); }
    FSMaterial* GetYieldDamageMaterial() { return GetMaterialProperty(3); }
    
    // set/get yield damage criterion
    void SetYieldDamageCriterion(FSMaterial* pm) { ReplaceProperty(4, pm); }
    FSMaterial* GetYieldDamageCriterion() { return GetMaterialProperty(4); }

    // set/get damage material
    void SetIntactDamageMaterial(FSMaterial* pm) { ReplaceProperty(5, pm); }
    FSMaterial* GetIntactDamageMaterial() { return GetMaterialProperty(5); }
    
    // set/get criterion
    void SetIntactDamageCriterion(FSMaterial* pm) { ReplaceProperty(6, pm); }
    FSMaterial* GetIntactDamageCriterion() { return GetMaterialProperty(6); }

    DECLARE_REGISTERED(FSReactivePlasticDamage);
};

//-----------------------------------------------------------------------------
// reactive fatigue
class FSReactiveFatigue : public FSMaterial
{
public:
    // material parameters
    enum { MP_K0, MP_BETA };
    
public:
    // constructor
    FSReactiveFatigue(FSModel* fem);
    
    // set the elastic component of the material
    void SetElasticMaterial(FSMaterial* pm) { ReplaceProperty(0, pm); }
    FSMaterial* GetElasticMaterial() { return GetMaterialProperty(0); }
    
    // set/get elastic damage material
    void SetElasticDamageMaterial(FSMaterial* pm) { ReplaceProperty(1, pm); }
    FSMaterial* GetElasticDamageMaterial() { return GetMaterialProperty(1); }
    
    // set/get elastic damage criterion
    void SetElasticDamageCriterion(FSMaterial* pm) { ReplaceProperty(2, pm); }
    FSMaterial* GetElasticDamageCriterion() { return GetMaterialProperty(2); }
    
    // set/get fatigue damage material
    void SetFatigueDamageMaterial(FSMaterial* pm) { ReplaceProperty(3, pm); }
    FSMaterial* GetFatigueDamageMaterial() { return GetMaterialProperty(3); }
    
    // set/get fatigue criterion
    void SetFatigueDamageCriterion(FSMaterial* pm) { ReplaceProperty(4, pm); }
    FSMaterial* GetFatigueDamageCriterion() { return GetMaterialProperty(4); }
    
    DECLARE_REGISTERED(FSReactiveFatigue);
};

//-----------------------------------------------------------------------------
// uncoupled reactive fatigue
class FSUncoupledReactiveFatigue : public FSMaterial
{
public:
    // material parameters
    enum { MP_K0, MP_BETA, MP_K };
    
public:
    // constructor
    FSUncoupledReactiveFatigue(FSModel* fem);
    
    // set the elastic component of the material
    void SetElasticMaterial(FSMaterial* pm) { ReplaceProperty(0, pm); }
    FSMaterial* GetElasticMaterial() { return GetMaterialProperty(0); }
    
    // set/get elastic damage material
    void SetElasticDamageMaterial(FSMaterial* pm) { ReplaceProperty(1, pm); }
    FSMaterial* GetElasticDamageMaterial() { return GetMaterialProperty(1); }
    
    // set/get elastic damage criterion
    void SetElasticDamageCriterion(FSMaterial* pm) { ReplaceProperty(2, pm); }
    FSMaterial* GetElasticDamageCriterion() { return GetMaterialProperty(2); }
    
    // set/get fatigue damage material
    void SetFatigueDamageMaterial(FSMaterial* pm) { ReplaceProperty(3, pm); }
    FSMaterial* GetFatigueDamageMaterial() { return GetMaterialProperty(3); }
    
    // set/get fatigue criterion
    void SetFatigueDamageCriterion(FSMaterial* pm) { ReplaceProperty(4, pm); }
    FSMaterial* GetFatigueDamageCriterion() { return GetMaterialProperty(4); }
    
    DECLARE_REGISTERED(FSUncoupledReactiveFatigue);
};
