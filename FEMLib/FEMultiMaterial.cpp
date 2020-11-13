/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include "stdafx.h"
#include "FEMultiMaterial.h"
#include "FEMaterialFactory.h"
#include <MeshTools/FEProject.h>
#include <sstream>
#include <FSCore/paramunit.h>

//=============================================================================
//								VISCO-ELASTIC
//=============================================================================

REGISTER_MATERIAL(FEViscoElastic, MODULE_MECH, FE_VISCO_ELASTIC, FE_MAT_ELASTIC, "viscoelastic", MaterialFlags::TOPLEVEL);

FEViscoElastic::FEViscoElastic() : FEMaterial(FE_VISCO_ELASTIC)
{
    AddScienceParam(1, UNIT_DENSITY, "density", "density"     )->SetPersistent(false);

    AddScienceParam(0, UNIT_NONE, "g1", "coeffient G1");
	AddScienceParam(0, UNIT_NONE, "g2", "coeffient G2");
	AddScienceParam(0, UNIT_NONE, "g3", "coeffient G3");
	AddScienceParam(0, UNIT_NONE, "g4", "coeffient G4");
	AddScienceParam(0, UNIT_NONE, "g5", "coeffient G5");
	AddScienceParam(0, UNIT_NONE, "g6", "coeffient G6");

	AddScienceParam(1, UNIT_TIME, "t1", "relaxation time t1");
	AddScienceParam(1, UNIT_TIME, "t2", "relaxation time t2");
	AddScienceParam(1, UNIT_TIME, "t3", "relaxation time t3");
	AddScienceParam(1, UNIT_TIME, "t4", "relaxation time t4");
	AddScienceParam(1, UNIT_TIME, "t5", "relaxation time t5");
	AddScienceParam(1, UNIT_TIME, "t6", "relaxation time t6");

	// Add one component for the elastic material
	AddProperty("elastic", FE_MAT_ELASTIC);
}

//=============================================================================
//								UNCOUPLED VISCO-ELASTIC
//=============================================================================

REGISTER_MATERIAL(FEUncoupledViscoElastic, MODULE_MECH, FE_UNCOUPLED_VISCO_ELASTIC, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled viscoelastic", MaterialFlags::TOPLEVEL);

FEUncoupledViscoElastic::FEUncoupledViscoElastic() : FEMaterial(FE_UNCOUPLED_VISCO_ELASTIC)
{
    AddScienceParam(1, UNIT_DENSITY, "density", "density"     )->SetPersistent(false);
    
	AddScienceParam(0, UNIT_NONE, "g1", "coeffient G1");
	AddScienceParam(0, UNIT_NONE, "g2", "coeffient G2");
	AddScienceParam(0, UNIT_NONE, "g3", "coeffient G3");
	AddScienceParam(0, UNIT_NONE, "g4", "coeffient G4");
	AddScienceParam(0, UNIT_NONE, "g5", "coeffient G5");
	AddScienceParam(0, UNIT_NONE, "g6", "coeffient G6");

	AddScienceParam(1, UNIT_TIME, "t1", "relaxation time t1");
	AddScienceParam(1, UNIT_TIME, "t2", "relaxation time t2");
	AddScienceParam(1, UNIT_TIME, "t3", "relaxation time t3");
	AddScienceParam(1, UNIT_TIME, "t4", "relaxation time t4");
	AddScienceParam(1, UNIT_TIME, "t5", "relaxation time t5");
	AddScienceParam(1, UNIT_TIME, "t6", "relaxation time t6");

    AddScienceParam(0, UNIT_PRESSURE , "k"      , "bulk modulus")->SetPersistent(false);
    
	// Add the elastic material property
	AddProperty("elastic", FE_MAT_ELASTIC_UNCOUPLED);
}

//=============================================================================
FEMultiMaterial::FEMultiMaterial(int ntype) : FEMaterial(ntype)
{

}

//=============================================================================
//									BIPHASIC
//=============================================================================

REGISTER_MATERIAL(FEBiphasic, MODULE_BIPHASIC, FE_BIPHASIC_MATERIAL, FE_MAT_MULTIPHASIC, "biphasic", MaterialFlags::TOPLEVEL);

FEBiphasic::FEBiphasic() : FEMultiMaterial(FE_BIPHASIC_MATERIAL)
{
	// add parameters
	AddScienceParam(0, UNIT_NONE, "phi0", "solid volume fraction");
    AddScienceParam(1.0, UNIT_DENSITY, "fluid_density", "fluid density");

	// Add elastic component
	AddProperty("solid", FE_MAT_ELASTIC | FE_MAT_ELASTIC_UNCOUPLED);

	// Add permeability component
	AddProperty("permeability", FE_MAT_PERMEABILITY);

	// Add solvent supply
	AddProperty("solvent_supply", FE_MAT_SOLVENT_SUPPLY);
}

//=============================================================================
//								SOLUTE MATERIAL
//=============================================================================

REGISTER_MATERIAL(FESoluteMaterial, MODULE_MULTIPHASIC, FE_SOLUTE_MATERIAL, FE_MAT_SOLUTE, "solute", 0);

FESoluteMaterial::FESoluteMaterial() : FEMaterial(FE_SOLUTE_MATERIAL)
{
	// add the solute material index
	AddChoiceParam(0, "sol", "Solute")->SetEnumNames("$(Solutes)")->SetState(Param_EDITABLE | Param_PERSISTENT);

	// Add diffusivity property
	AddProperty("diffusivity", FE_MAT_DIFFUSIVITY);

	// Add solubility property
	AddProperty("solubility", FE_MAT_SOLUBILITY);
}

//=============================================================================
//								SOLID-BOUND MOLECULES MATERIAL
//=============================================================================

REGISTER_MATERIAL(FESBMMaterial, MODULE_MULTIPHASIC, FE_SBM_MATERIAL, FE_MAT_SBM, "solid_bound", 0);

FESBMMaterial::FESBMMaterial() : FEMaterial(FE_SBM_MATERIAL)
{
	// add the SBM material index
	AddIntParam(0, "sbm", "Solid-bound molecule")->SetEnumNames("$(SBMs)")->SetState(Param_EDITABLE | Param_PERSISTENT);
    
	// add parameters
	AddScienceParam(0, UNIT_DENSITY, "rho0", "apparent density");
	AddScienceParam(0, UNIT_DENSITY, "rhomin", "min density");
	AddScienceParam(0, UNIT_DENSITY, "rhomax", "max density");
}

//=============================================================================
//								BIPHASIC-SOLUTE
//=============================================================================

REGISTER_MATERIAL(FEBiphasicSolute, MODULE_MULTIPHASIC, FE_BIPHASIC_SOLUTE, FE_MAT_MULTIPHASIC, "biphasic-solute", MaterialFlags::TOPLEVEL);

FEBiphasicSolute::FEBiphasicSolute() : FEMultiMaterial(FE_BIPHASIC_SOLUTE)
{
	// add parameters
	AddScienceParam(0, UNIT_NONE, "phi0", "solid volume fraction");

	// Add elastic component
	AddProperty("solid", FE_MAT_ELASTIC | FE_MAT_ELASTIC_UNCOUPLED);

	// Add permeability component
	AddProperty("permeability", FE_MAT_PERMEABILITY);

	// Add osmotic coefficient component
	AddProperty("osmotic_coefficient", FE_MAT_OSMOTIC_COEFFICIENT);

	// Add the first solute component
	AddProperty("solute", FE_MAT_SOLUTE);
}

//=============================================================================
//								TRIPHASIC
//=============================================================================

REGISTER_MATERIAL(FETriphasicMaterial, MODULE_MULTIPHASIC, FE_TRIPHASIC_MATERIAL, FE_MAT_MULTIPHASIC, "triphasic", MaterialFlags::TOPLEVEL);

FETriphasicMaterial::FETriphasicMaterial() : FEMultiMaterial(FE_TRIPHASIC_MATERIAL)
{
	// add parameters
	AddScienceParam(0, UNIT_NONE, "phi0", "solid volume fraction");
	AddScienceParam(0, UNIT_CONCENTRATION, "fixed_charge_density", "fixed charge density");

	// Add elastic component
	AddProperty("solid", FE_MAT_ELASTIC | FE_MAT_ELASTIC_UNCOUPLED);

	// Add permeability component
	AddProperty("permeability", FE_MAT_PERMEABILITY);

	// Add osmotic coefficient component
	AddProperty("osmotic_coefficient", FE_MAT_OSMOTIC_COEFFICIENT);

	// Add the first solute component
	AddProperty("solute", FE_MAT_SOLUTE, 2);
}

// set/get solute i
void FETriphasicMaterial::SetSoluteMaterial(FESoluteMaterial* pm, int i)
{ 
	ReplaceProperty(3, pm, i); 
}

FEMaterial* FETriphasicMaterial::GetSoluteMaterial(int i) 
{ 
	return GetProperty(3).GetMaterial(i); 
}

//=============================================================================
//								SOLID MIXTURE
//=============================================================================

REGISTER_MATERIAL(FESolidMixture, MODULE_MECH, FE_SOLID_MIXTURE, FE_MAT_ELASTIC, "solid mixture", MaterialFlags::TOPLEVEL);

//-----------------------------------------------------------------------------
FESolidMixture::FESolidMixture() : FEMaterial(FE_SOLID_MIXTURE)
{
    AddScienceParam(1, UNIT_DENSITY, "density", "density");
    
	AddProperty("solid", FE_MAT_ELASTIC, FEMaterialProperty::NO_FIXED_SIZE);
}

//=============================================================================
//								UNCOUPLED SOLID MIXTURE
//=============================================================================

REGISTER_MATERIAL(FEUncoupledSolidMixture, MODULE_MECH, FE_UNCOUPLED_SOLID_MIXTURE, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled solid mixture", MaterialFlags::TOPLEVEL);

FEUncoupledSolidMixture::FEUncoupledSolidMixture() : FEMaterial(FE_UNCOUPLED_SOLID_MIXTURE) 
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density");
	AddScienceParam(0, UNIT_PRESSURE , "k", "bulk modulus" );

	AddProperty("solid", FE_MAT_ELASTIC_UNCOUPLED, FEMaterialProperty::NO_FIXED_SIZE);
}

//=============================================================================
//							CONTINUOUS FIBER DISTRIBUTION
//=============================================================================

REGISTER_MATERIAL(FECFDMaterial, MODULE_MECH, FE_CFD_MATERIAL, FE_MAT_ELASTIC, "continuous fiber distribution", 0);

FECFDMaterial::FECFDMaterial() : FEMaterial(FE_CFD_MATERIAL)
{
	SetAxisMaterial(new FEAxisMaterial);

    // Add fiber component
	AddProperty("fibers", FE_MAT_CFD_FIBER);
    
    // Add distribution component
	AddProperty("distribution", FE_MAT_CFD_DIST);
    
    // Add scheme component
	AddProperty("scheme", FE_MAT_CFD_SCHEME);
}

//=============================================================================
//                      CONTINUOUS FIBER DISTRIBUTION UNCOUPLED
//=============================================================================

REGISTER_MATERIAL(FECFDUCMaterial, MODULE_MECH, FE_CFD_MATERIAL_UC, FE_MAT_ELASTIC_UNCOUPLED, "continuous fiber distribution uncoupled", 0);

FECFDUCMaterial::FECFDUCMaterial() : FEMaterial(FE_CFD_MATERIAL_UC)
{
	SetAxisMaterial(new FEAxisMaterial);

    // Add fiber component
	AddProperty("fibers", FE_MAT_CFD_FIBER_UC);
    
    // Add distribution component
	AddProperty("distribution", FE_MAT_CFD_DIST);
    
    // Add scheme component
	AddProperty("scheme", FE_MAT_CFD_SCHEME);
}

//=============================================================================
//                                 ELASTIC DAMAGE
//=============================================================================

REGISTER_MATERIAL(FEElasticDamageMaterial, MODULE_MECH, FE_DMG_MATERIAL, FE_MAT_ELASTIC, "elastic damage", MaterialFlags::TOPLEVEL);

FEElasticDamageMaterial::FEElasticDamageMaterial() : FEMaterial(FE_DMG_MATERIAL)
{
    // Add elastic component
    AddProperty("elastic", FE_MAT_ELASTIC);
    
    // Add damage component
	AddProperty("damage", FE_MAT_DAMAGE);
    
    // Add criterion component
	AddProperty("criterion", FE_MAT_DAMAGE_CRITERION);
}

//=============================================================================
//                             UNCOUPLED ELASTIC DAMAGE
//=============================================================================

REGISTER_MATERIAL(FEElasticDamageMaterialUC, MODULE_MECH, FE_DMG_MATERIAL_UC, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled elastic damage", MaterialFlags::TOPLEVEL);

FEElasticDamageMaterialUC::FEElasticDamageMaterialUC() : FEMaterial(FE_DMG_MATERIAL_UC)
{
    // Add elastic component
	AddProperty("elastic", FE_MAT_ELASTIC_UNCOUPLED);
    
    // Add damage component
	AddProperty("damage", FE_MAT_DAMAGE);
    
    // Add criterion component
	AddProperty("criterion", FE_MAT_DAMAGE_CRITERION);
}

//=============================================================================
//							REACTIVE VISCOELASTIC
//=============================================================================

REGISTER_MATERIAL(FEReactiveViscoelasticMaterial, MODULE_MECH, FE_RV_MATERIAL, FE_MAT_ELASTIC, "reactive viscoelastic", MaterialFlags::TOPLEVEL);

FEReactiveViscoelasticMaterial::FEReactiveViscoelasticMaterial() : FEMaterial(FE_RV_MATERIAL)
{
    // add parameters
    AddIntParam(1, "kinetics", "kinetics"); // "bond kinetics type (1 or 2)");
    AddIntParam(0, "trigger" , "trigger" ); // "bond breaking trigger (0=any, 1=distortion, or 2=dilatation)");
    
    // Add elastic material component
    AddProperty("elastic", FE_MAT_ELASTIC);
    
    // Add bond material component
	AddProperty("bond", FE_MAT_ELASTIC);
    
    // Add relaxation component
	AddProperty("relaxation", FE_MAT_RV_RELAX);
}

//=============================================================================
//                      UNCOUPLED REACTIVE VISCOELASTIC
//=============================================================================

REGISTER_MATERIAL(FEReactiveViscoelasticMaterialUC, MODULE_MECH, FE_RV_MATERIAL_UC, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled reactive viscoelastic", MaterialFlags::TOPLEVEL);

FEReactiveViscoelasticMaterialUC::FEReactiveViscoelasticMaterialUC() : FEMaterial(FE_RV_MATERIAL_UC)
{
    // add parameters
    AddIntParam(1, "kinetics", "kinetics"); // "bond kinetics type (1 or 2)");
    AddIntParam(0, "trigger" , "trigger" ); // "bond breaking trigger (0=any, 1=distortion, or 2=dilatation)");
	AddDoubleParam(0, "k", "bulk modulus")->SetPersistent(false);
    
    // Add elastic material component
    AddProperty("elastic", FE_MAT_ELASTIC_UNCOUPLED);
    
    // Add bond material component
	AddProperty("bond", FE_MAT_ELASTIC_UNCOUPLED);
    
    // Add relaxation component
	AddProperty("relaxation", FE_MAT_RV_RELAX);
}

//=============================================================================
//								REACTION
//=============================================================================

FEReactionMaterial::FEReactionMaterial(int ntype) : FEMaterial(ntype)
{
	// the optional Vbar parameter is hidden by default.
	AddScienceParam(0, UNIT_MOLAR_VOLUME, "Vbar", "Vbar")->SetState(Param_HIDDEN);
	AddBoolParam(false, 0, 0)->SetState(Param_HIDDEN);
    
	// Add reaction rate properties
	AddProperty("forward_rate", FE_MAT_REACTION_RATE);
	AddProperty("reverse_rate", FE_MAT_REACTION_RATE);

	AddProperty("vR", FE_MAT_REACTION_REACTANTS, FEMaterialProperty::NO_FIXED_SIZE);
	AddProperty("vP", FE_MAT_REACTION_PRODUCTS , FEMaterialProperty::NO_FIXED_SIZE);
}


void FEReactionMaterial::SetOvrd(bool bovrd) {
	SetBoolValue(MP_OVRD, bovrd);
	if (bovrd) GetParam(MP_VBAR).SetState(Param_ALLFLAGS);
	else GetParam(MP_VBAR).SetState(Param_HIDDEN);
}

bool FEReactionMaterial::GetOvrd() { return GetBoolValue(MP_OVRD); }

// set forward rate
void FEReactionMaterial::SetForwardRate(FEMaterial* pm) { ReplaceProperty(0, pm); }
FEMaterial* FEReactionMaterial::GetForwardRate() { return GetProperty(0).GetMaterial(); }

// set reverse rate
void FEReactionMaterial::SetReverseRate(FEMaterial* pm) { ReplaceProperty(1, pm); }
FEMaterial* FEReactionMaterial::GetReverseRate() { return GetProperty(1).GetMaterial(); }

int FEReactionMaterial::Reactants() { return GetProperty(2).Size(); }
int FEReactionMaterial::Products() { return GetProperty(3).Size(); }

FEReactantMaterial* FEReactionMaterial::Reactant(int i)
{
	return dynamic_cast<FEReactantMaterial*>(GetProperty(2).GetMaterial(i));
}

FEProductMaterial* FEReactionMaterial::Product(int i)
{
	return dynamic_cast<FEProductMaterial*>(GetProperty(3).GetMaterial(i));
}

// add reactant/product component
void FEReactionMaterial::AddReactantMaterial(FEReactantMaterial* pm) { AddProperty(2, pm); }
void FEReactionMaterial::AddProductMaterial (FEProductMaterial* pm) { AddProperty(3, pm); }

void FEReactionMaterial::ClearReactants()
{
	GetProperty(2).Clear();
}

void FEReactionMaterial::ClearProducts()
{
	GetProperty(3).Clear();
}

void FEReactionMaterial::GetSoluteReactants(vector<int>& solR)
{
	solR.clear();
	FEMaterialProperty& p = GetProperty(2);
	int N = p.Size();
	for (int i=0; i<N; ++i)
	{
		FEReactantMaterial* ri = dynamic_cast<FEReactantMaterial*>(p.GetMaterial(i)); assert(ri);
		if (ri && (ri->GetReactantType() == FEReactionMaterial::SOLUTE_SPECIES)) solR.push_back(ri->GetIndex());
	}
}

void FEReactionMaterial::GetSBMReactants(vector<int>& sbmR)
{
	sbmR.clear();
	FEMaterialProperty& p = GetProperty(2);
	int N = p.Size();
	for (int i = 0; i<N; ++i)
	{
		FEReactantMaterial* ri = dynamic_cast<FEReactantMaterial*>(p.GetMaterial(i)); assert(ri);
		if (ri && (ri->GetReactantType() == FEReactionMaterial::SBM_SPECIES)) sbmR.push_back(ri->GetIndex());
	}
}

void FEReactionMaterial::GetSoluteProducts(vector<int>& solP)
{
	solP.clear();
	FEMaterialProperty& p = GetProperty(3);
	int N = p.Size();
	for (int i = 0; i<N; ++i)
	{
		FEProductMaterial* ri = dynamic_cast<FEProductMaterial*>(p.GetMaterial(i)); assert(ri);
		if (ri && (ri->GetProductType() == FEReactionMaterial::SOLUTE_SPECIES)) solP.push_back(ri->GetIndex());
	}
}

void FEReactionMaterial::GetSBMProducts(vector<int>& sbmP)
{
	sbmP.clear();
	FEMaterialProperty& p = GetProperty(3);
	int N = p.Size();
	for (int i = 0; i<N; ++i)
	{
		FEProductMaterial* ri = dynamic_cast<FEProductMaterial*>(p.GetMaterial(i)); assert(ri);
		if (ri && (ri->GetProductType() == FEReactionMaterial::SBM_SPECIES)) sbmP.push_back(ri->GetIndex());
	}
}

string buildReactionEquation(FEReactionMaterial* mat, FEModel& fem)
{
	stringstream ss;
	int NR = mat->Reactants();
	for (int i = 0; i<NR; ++i)
	{
		FEReactantMaterial* rm = mat->Reactant(i);

		string name;
		int m = rm->GetIndex();
		int ntype = rm->GetReactantType();
		if (ntype == FEReactionMaterial::SOLUTE_SPECIES)
			name = fem.GetSoluteData(m).GetName();
		else
			name = fem.GetSBMData(m).GetName();

		int n = rm->GetCoef();
		if (n == 1)
			ss << name;
		else
			ss << n << "*" << name;
		if (i != NR - 1) ss << "+";
	}

	bool rev = (mat->GetReverseRate() != 0);
	if (rev) ss << "<-->";
	else ss << "-->";

	int NP = mat->Products();
	for (int i = 0; i<NP; ++i)
	{
		FEProductMaterial* pm = mat->Product(i);

		string name;
		int m = pm->GetIndex();
		int ntype = pm->GetProductType();
		if (ntype == FEReactionMaterial::SOLUTE_SPECIES)
			name = fem.GetSoluteData(m).GetName();
		else
			name = fem.GetSBMData(m).GetName();

		int n = pm->GetCoef();
		if (n == 1)
			ss << name;
		else
			ss << n << "*" << name;
		if (i != NP - 1) ss << "+";
	}
	return ss.str();
}

//=============================================================================
//								MULTIPHASIC
//=============================================================================

REGISTER_MATERIAL(FEMultiphasicMaterial, MODULE_MULTIPHASIC, FE_MULTIPHASIC_MATERIAL, FE_MAT_MULTIPHASIC, "multiphasic", MaterialFlags::TOPLEVEL);

FEMultiphasicMaterial::FEMultiphasicMaterial() : FEMultiMaterial(FE_MULTIPHASIC_MATERIAL)
{
	// add parameters
	AddScienceParam(0, UNIT_NONE, "phi0", "solid volume fraction");
	AddScienceParam(0, UNIT_CONCENTRATION, "fixed_charge_density", "fixed charge density");

	// Add elastic component
	AddProperty("solid", FE_MAT_ELASTIC | FE_MAT_ELASTIC_UNCOUPLED);

	// Add permeability component
	AddProperty("permeability", FE_MAT_PERMEABILITY);

	// Add osmotic coefficient component
	AddProperty("osmotic_coefficient", FE_MAT_OSMOTIC_COEFFICIENT);

	// Add solute property
	AddProperty("solute", FE_MAT_SOLUTE, FEMaterialProperty::NO_FIXED_SIZE);

	// Add solid bound property
	AddProperty("solid_bound", FE_MAT_SBM, FEMaterialProperty::NO_FIXED_SIZE);

	// add reaction material
	AddProperty("reaction", FE_MAT_REACTION, FEMaterialProperty::NO_FIXED_SIZE, 0);
}

// set/get elastic component 
void FEMultiphasicMaterial::SetSolidMaterial(FEMaterial* pm)
{ 
	ReplaceProperty(SOLID, pm); 
}

// set/get permeability
void FEMultiphasicMaterial::SetPermeability(FEMaterial* pm)
{ 
	ReplaceProperty(PERM, pm); 
}

// set/get osmotic coefficient
void FEMultiphasicMaterial::SetOsmoticCoefficient(FEMaterial* pm)
{ 
	ReplaceProperty(OSMC, pm); 
}

// add solute component
void FEMultiphasicMaterial::AddSoluteMaterial(FESoluteMaterial* pm)
{
	GetProperty(SOLUTE).AddMaterial(pm);
}

// add SBM component
void FEMultiphasicMaterial::AddSBMMaterial(FESBMMaterial* pm)
{
	GetProperty(SBM).AddMaterial(pm);
}

// add chemical reaction component
void FEMultiphasicMaterial::AddReactionMaterial(FEReactionMaterial* pm)
{
	GetProperty(REACTION).AddMaterial(pm);
}

// get solute global index from local index
int FEMultiphasicMaterial::GetSoluteIndex(const int isol) 
{
	FEMaterialProperty& p = GetProperty(SOLUTE);
	FESoluteMaterial* mp = dynamic_cast<FESoluteMaterial*>(p.GetMaterial(isol));
	return mp->GetSoluteIndex();
}

// get SBM global index from local index
int FEMultiphasicMaterial::GetSBMIndex(const int isbm) 
{
	FEMaterialProperty& p = GetProperty(SBM);
	FESBMMaterial* mp = dynamic_cast<FESBMMaterial*>(p.GetMaterial(isbm));
	return mp->GetSBMIndex();
}

// count reaction components
int FEMultiphasicMaterial::Reactions() 
{
	return GetProperty(REACTION).Size();
}

// get reaction component
FEReactionMaterial* FEMultiphasicMaterial::GetReaction(int n)
{
	FEMaterialProperty& p = GetProperty(REACTION);
	FEReactionMaterial* prm = dynamic_cast<FEReactionMaterial*>(p.GetMaterial(n));
	return prm;
}

// see if this material has a solute with global ID
bool FEMultiphasicMaterial::HasSolute(int nid)
{
	FEMaterialProperty& p = GetProperty(SOLUTE);
	for (int i=0; i<p.Size(); ++i)
	{
		FESoluteMaterial* mp = dynamic_cast<FESoluteMaterial*>(p.GetMaterial(i));
		int sid = mp->GetSoluteIndex();
		if (mp && (mp->GetSoluteIndex() == nid)) return true;
	}
	return false;
}

// see if this material has a sbm with global ID
bool FEMultiphasicMaterial::HasSBM(int nid)
{
	FEMaterialProperty& p = GetProperty(SBM);
	for (int i = 0; i<p.Size(); ++i)
	{
		FESBMMaterial* mp = dynamic_cast<FESBMMaterial*>(p.GetMaterial(i));
		int sid = mp->GetSBMIndex();
		if (mp && (mp->GetSBMIndex() == nid)) return true;
	}
	return false;
}

//=============================================================================
//								REACTANT
//=============================================================================

REGISTER_MATERIAL(FEReactantMaterial, MODULE_REACTIONS, FE_REACTANT_MATERIAL, FE_MAT_REACTION_REACTANTS, "vR", 0);

FEReactantMaterial::FEReactantMaterial() : FEMaterial(FE_REACTANT_MATERIAL)
{
	// add the stoichiometric coefficient
	AddIntParam(1, "vR", "vR"); // reactant stoichiometric coefficient
	// add the type
	AddIntParam(-1, 0, 0)->SetState(Param_HIDDEN);
	// add the index
	AddIntParam(-1, 0, 0)->SetState(Param_HIDDEN);
}

//=============================================================================
//								PRODUCT
//=============================================================================

REGISTER_MATERIAL(FEProductMaterial, MODULE_REACTIONS, FE_PRODUCT_MATERIAL, FE_MAT_REACTION_PRODUCTS, "vP", 0);

FEProductMaterial::FEProductMaterial() : FEMaterial(FE_PRODUCT_MATERIAL)
{
	// add the stoichiometric coefficient
	AddIntParam(1, "vP", "vP"); // product stoichiometric coefficient
	// add the type
	AddIntParam(-1, 0, 0)->SetState(Param_HIDDEN);
	// add the index
	AddIntParam(-1, 0, 0)->SetState(Param_HIDDEN);
}

//=============================================================================
//								MASS ACTION FORWARD REACTION
//=============================================================================

REGISTER_MATERIAL(FEMassActionForward, MODULE_REACTIONS, FE_MASS_ACTION_FORWARD, FE_MAT_REACTION, "mass-action-forward", 0);

FEMassActionForward::FEMassActionForward() : FEReactionMaterial(FE_MASS_ACTION_FORWARD)
{
}

//=============================================================================
//								MASS ACTION REVERSIBLE REACTION
//=============================================================================

REGISTER_MATERIAL(FEMassActionReversible, MODULE_REACTIONS, FE_MASS_ACTION_REVERSIBLE, FE_MAT_REACTION, "mass-action-reversible", 0);

FEMassActionReversible::FEMassActionReversible() : FEReactionMaterial(FE_MASS_ACTION_REVERSIBLE)
{
}

//=============================================================================
//								MICHAELIS MENTEN REACTION
//=============================================================================

REGISTER_MATERIAL(FEMichaelisMenten, MODULE_REACTIONS, FE_MICHAELIS_MENTEN, FE_MAT_REACTION, "Michaelis-Menten", 0);

FEMichaelisMenten::FEMichaelisMenten() : FEReactionMaterial(FE_MICHAELIS_MENTEN)
{
	AddScienceParam(0, UNIT_CONCENTRATION, "Km", "Km"); // concentration at half-maximum rate
	AddScienceParam(0, UNIT_CONCENTRATION, "c0", "c0"); // substrate trigger concentration
}

//=============================================================================
//									FLUID
//=============================================================================
REGISTER_MATERIAL(FEFluidMaterial, MODULE_FLUID, FE_FLUID_MATERIAL, FE_MAT_FLUID, "fluid", MaterialFlags::TOPLEVEL);

FEFluidMaterial::FEFluidMaterial() : FEMaterial(FE_FLUID_MATERIAL)
{
    // add parameters
    AddScienceParam(1.0, UNIT_DENSITY, "density", "density");
    
    // add parameters
    AddScienceParam(1.0, UNIT_PRESSURE, "k", "bulk modulus");
    
    // Add viscous component
	AddProperty("viscous", FE_MAT_FLUID_VISCOSITY);
}

//=============================================================================
//                                    FLUID-FSI
//=============================================================================

REGISTER_MATERIAL(FEFluidFSIMaterial, MODULE_FLUID_FSI, FE_FLUID_FSI_MATERIAL, FE_MAT_FLUID_FSI, "fluid-FSI", MaterialFlags::TOPLEVEL);

FEFluidFSIMaterial::FEFluidFSIMaterial() : FEMultiMaterial(FE_FLUID_FSI_MATERIAL)
{
    // Add fluid component
    AddProperty("fluid", FE_MAT_FLUID);
    
    // Add solid component
    AddProperty("solid", FE_MAT_ELASTIC);
}

//=============================================================================
//                                  BIPHASIC-FSI
//=============================================================================

REGISTER_MATERIAL(FEBiphasicFSIMaterial, MODULE_FLUID_FSI, FE_BIPHASIC_FSI_MATERIAL, FE_MAT_FLUID_FSI, "biphasic-FSI", MaterialFlags::TOPLEVEL);

FEBiphasicFSIMaterial::FEBiphasicFSIMaterial() : FEMultiMaterial(FE_BIPHASIC_FSI_MATERIAL)
{
    // add parameters
    AddScienceParam(0, UNIT_NONE, "phi0", "solid volume fraction");
    
    // Add fluid component
    AddProperty("fluid", FE_MAT_FLUID);
    
    // Add solid component
    AddProperty("solid", FE_MAT_ELASTIC);
    
    // Add permeability component
    AddProperty("permeability", FE_MAT_PERMEABILITY);
}

//=============================================================================
//								SPECIES MATERIAL
//=============================================================================

REGISTER_MATERIAL(FESpeciesMaterial, MODULE_REACTIONS, FE_SPECIES_MATERIAL, FE_MAT_SPECIES, "species", 0);

FESpeciesMaterial::FESpeciesMaterial() : FEMaterial(FE_SPECIES_MATERIAL)
{
	// add the solute material index
	AddChoiceParam(0, "sol", "Solute")->SetEnumNames("$(Solutes)")->SetState(Param_EDITABLE | Param_PERSISTENT);

	// add the solute material index
	AddScienceParam(0, UNIT_DIFFUSIVITY, "diffusivity", "diffusivity");
}

int FESpeciesMaterial::GetSpeciesIndex()
{
	return GetParam(0).GetIntValue();
}

void FESpeciesMaterial::SetSpeciesIndex(int n)
{
	GetParam(0).SetIntValue(n);
}

//=============================================================================
//								SOLID SPECIES MATERIAL
//=============================================================================

REGISTER_MATERIAL(FESolidSpeciesMaterial, MODULE_REACTIONS, FE_SOLID_SPECIES_MATERIAL, FE_MAT_SOLID_SPECIES, "solid_bound_species", 0);

FESolidSpeciesMaterial::FESolidSpeciesMaterial() : FEMaterial(FE_SOLID_SPECIES_MATERIAL)
{
	// add the SBM material index
	AddIntParam(0, "sbm", "Solid-bound molecule")->SetEnumNames("$(SBMs)")->SetState(Param_EDITABLE | Param_PERSISTENT);

	// add parameters
	AddScienceParam(0, UNIT_DENSITY, "rho0", "apparent density");
	AddScienceParam(0, UNIT_DENSITY, "rhomin", "min density");
	AddScienceParam(0, UNIT_DENSITY, "rhomax", "max density");
}

//=============================================================================
//									REACTION DIFFUSION
//=============================================================================

REGISTER_MATERIAL(FEReactionDiffusionMaterial, MODULE_REACTION_DIFFUSION, FE_REACTION_DIFFUSION_MATERIAL, FE_MAT_REACTION_DIFFUSION, "reaction-diffusion", MaterialFlags::TOPLEVEL);

FEReactionDiffusionMaterial::FEReactionDiffusionMaterial() : FEMaterial(FE_REACTION_DIFFUSION_MATERIAL)
{
	// add parameters
	AddScienceParam(0.0, UNIT_NONE, "solid_volume_fraction", "solid_volume_fraction");

	// Add solute property
	AddProperty("species", FE_MAT_SPECIES, FEMaterialProperty::NO_FIXED_SIZE);

	// Add solid bound property
	AddProperty("solid_bound_species", FE_MAT_SOLID_SPECIES, FEMaterialProperty::NO_FIXED_SIZE);

	// add reaction material
	AddProperty("reaction", FE_MAT_REACTION, FEMaterialProperty::NO_FIXED_SIZE, 0);
}

void FEReactionDiffusionMaterial::AddSpeciesMaterial(FESpeciesMaterial* pm)
{
	GetProperty(0).AddMaterial(pm);
}

void FEReactionDiffusionMaterial::AddSolidSpeciesMaterial(FESolidSpeciesMaterial* pm)
{
	GetProperty(1).AddMaterial(pm);
}

void FEReactionDiffusionMaterial::AddReactionMaterial(FEReactionMaterial* pm)
{
	GetProperty(2).AddMaterial(pm);
}

//=============================================================================
//                                    GENERATION
//=============================================================================

REGISTER_MATERIAL(FEGeneration, MODULE_MECH, FE_GENERATION, FE_MAT_GENERATION, "generation", 0);

FEGeneration::FEGeneration() : FEMaterial(FE_GENERATION)
{
    // add parameters
    AddScienceParam(0, UNIT_TIME, "start_time", "generation start time");
    
    // Add solid component
    AddProperty("solid", FE_MAT_ELASTIC | FE_MAT_ELASTIC_UNCOUPLED);
}

//=============================================================================
//                                MULTIGENERATION
//=============================================================================

REGISTER_MATERIAL(FEMultiGeneration, MODULE_MECH, FE_MULTI_GENERATION, FE_MAT_ELASTIC, "multigeneration", MaterialFlags::TOPLEVEL);

//-----------------------------------------------------------------------------
FEMultiGeneration::FEMultiGeneration() : FEMaterial(FE_MULTI_GENERATION)
{
    AddProperty("generation", FE_MAT_GENERATION, FEMaterialProperty::NO_FIXED_SIZE);
}

//=============================================================================
//								PRESTRAIN
//=============================================================================

REGISTER_MATERIAL(FEPrestrainMaterial, MODULE_MECH, FE_PRESTRAIN_MATERIAL, FE_MAT_ELASTIC, "prestrain elastic", MaterialFlags::TOPLEVEL);

FEPrestrainMaterial::FEPrestrainMaterial() : FEMaterial(FE_PRESTRAIN_MATERIAL)
{
	// Add one component for the elastic material
	AddProperty("elastic", FE_MAT_ELASTIC);
	AddProperty("prestrain", FE_MAT_PRESTRAIN_GRADIENT);
}

//=============================================================================
//								UNCOUPLED PRESTRAIN
//=============================================================================

REGISTER_MATERIAL(FEUncoupledPrestrainMaterial, MODULE_MECH, FE_UNCOUPLED_PRESTRAIN_MATERIAL, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled prestrain elastic", MaterialFlags::TOPLEVEL);

FEUncoupledPrestrainMaterial::FEUncoupledPrestrainMaterial() : FEMaterial(FE_UNCOUPLED_PRESTRAIN_MATERIAL)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus");

	// Add one component for the elastic material
	AddProperty("elastic", FE_MAT_ELASTIC_UNCOUPLED);
	AddProperty("prestrain", FE_MAT_PRESTRAIN_GRADIENT);
}

//=============================================================================
//                              REACTIVE PLASTICITY
//=============================================================================

REGISTER_MATERIAL(FEReactivePlasticity, MODULE_MECH, FE_REACTIVE_PLASTICITY, FE_MAT_ELASTIC, "reactive plasticity", MaterialFlags::TOPLEVEL);

FEReactivePlasticity::FEReactivePlasticity() : FEMaterial(FE_REACTIVE_PLASTICITY)
{
    AddScienceParam(1, UNIT_DENSITY, "density", "density"     )->SetPersistent(false);
    
    AddScienceParam(1, UNIT_NONE    , "nf"  , "no. of bond families");
    AddScienceParam(0, UNIT_PRESSURE, "Y0"  , "minimum yield threshold");
    AddScienceParam(0, UNIT_PRESSURE, "Ymax", "maximum yield threshold");
    AddScienceParam(1, UNIT_NONE    , "w0"  , "initial fraction of yielding bonds");
    AddScienceParam(0, UNIT_NONE    , "we"  , "fraction of unyielding bonds");
    AddScienceParam(1, UNIT_NONE    , "r"   , "bias");
    AddBoolParam   (true,"isochoric", "isochoric plastic flow");
    
    // Add one component for the elastic material
    AddProperty("elastic", FE_MAT_ELASTIC);
    
    // Add criterion component
    AddProperty("yield_criterion", FE_MAT_DAMAGE_CRITERION);
}

//=============================================================================
//                            REACTIVE PLASTIC DAMAGE
//=============================================================================

REGISTER_MATERIAL(FEReactivePlasticDamage, MODULE_MECH, FE_REACTIVE_PLASTIC_DAMAGE, FE_MAT_ELASTIC, "reactive plastic damage", MaterialFlags::TOPLEVEL);

FEReactivePlasticDamage::FEReactivePlasticDamage() : FEMaterial(FE_REACTIVE_PLASTIC_DAMAGE)
{
    AddScienceParam(1, UNIT_DENSITY, "density", "density"     )->SetPersistent(false);
    
    AddScienceParam(1, UNIT_NONE    , "nf"  , "no. of bond families");
    AddScienceParam(0, UNIT_PRESSURE, "Y0"  , "minimum yield threshold");
    AddScienceParam(0, UNIT_PRESSURE, "Ymax", "maximum yield threshold");
    AddScienceParam(1, UNIT_NONE    , "w0"  , "initial fraction of yielding bonds");
    AddScienceParam(0, UNIT_NONE    , "we"  , "fraction of unyielding bonds");
    AddScienceParam(1, UNIT_NONE    , "r"   , "bias");
    AddBoolParam   (true,"isochoric", "isochoric plastic flow");
    
    // Add one component for the elastic material
    AddProperty("elastic", FE_MAT_ELASTIC);
    
    // Add criterion component
    AddProperty("yield_criterion", FE_MAT_DAMAGE_CRITERION);
    
    // Add yield damage material
    AddProperty("yield_damage", FE_MAT_DAMAGE);

    // Add yield damage criterion component
    AddProperty("yield_damage_criterion", FE_MAT_DAMAGE_CRITERION);
    
    // Add intact damage material
    AddProperty("intact_damage", FE_MAT_DAMAGE);
    
    // Add intact damage criterion component
    AddProperty("intact_damage_criterion", FE_MAT_DAMAGE_CRITERION);
    
}
