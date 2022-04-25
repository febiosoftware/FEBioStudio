/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
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

using std::stringstream;

//=============================================================================
//								VISCO-ELASTIC
//=============================================================================

REGISTER_MATERIAL(FEViscoElastic, MODULE_MECH, FE_VISCO_ELASTIC, FE_MAT_ELASTIC, "viscoelastic", MaterialFlags::TOPLEVEL);

FEViscoElastic::FEViscoElastic() : FEMaterial(FE_VISCO_ELASTIC)
{
    AddScienceParam(1, UNIT_DENSITY, "density", "density"     )->SetPersistent(false);

    AddScienceParam(0, UNIT_NONE, "g1", "coeffient γ1");
	AddScienceParam(0, UNIT_NONE, "g2", "coeffient γ2");
	AddScienceParam(0, UNIT_NONE, "g3", "coeffient γ3");
	AddScienceParam(0, UNIT_NONE, "g4", "coeffient γ4");
	AddScienceParam(0, UNIT_NONE, "g5", "coeffient γ5");
	AddScienceParam(0, UNIT_NONE, "g6", "coeffient γ6");

	AddScienceParam(1, UNIT_TIME, "t1", "relaxation time τ1");
	AddScienceParam(1, UNIT_TIME, "t2", "relaxation time τ2");
	AddScienceParam(1, UNIT_TIME, "t3", "relaxation time τ3");
	AddScienceParam(1, UNIT_TIME, "t4", "relaxation time τ4");
	AddScienceParam(1, UNIT_TIME, "t5", "relaxation time τ5");
	AddScienceParam(1, UNIT_TIME, "t6", "relaxation time τ6");

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
    
	AddScienceParam(0, UNIT_NONE, "g1", "coeffient γ1");
	AddScienceParam(0, UNIT_NONE, "g2", "coeffient γ2");
	AddScienceParam(0, UNIT_NONE, "g3", "coeffient γ3");
	AddScienceParam(0, UNIT_NONE, "g4", "coeffient γ4");
	AddScienceParam(0, UNIT_NONE, "g5", "coeffient γ5");
	AddScienceParam(0, UNIT_NONE, "g6", "coeffient γ6");

	AddScienceParam(1, UNIT_TIME, "t1", "relaxation time τ1");
	AddScienceParam(1, UNIT_TIME, "t2", "relaxation time τ2");
	AddScienceParam(1, UNIT_TIME, "t3", "relaxation time τ3");
	AddScienceParam(1, UNIT_TIME, "t4", "relaxation time τ4");
	AddScienceParam(1, UNIT_TIME, "t5", "relaxation time τ5");
	AddScienceParam(1, UNIT_TIME, "t6", "relaxation time τ6");

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
    AddDoubleParam(0.0, "tau", "tau");

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
	AddChoiceParam(0, "sol", "Solute")->SetEnumNames("$(Solutes)")->SetState(Param_EDITABLE | Param_PERSISTENT | Param_VISIBLE);

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
	AddIntParam(0, "sbm", "Solid-bound molecule")->SetEnumNames("$(SBMs)")->SetState(Param_EDITABLE | Param_PERSISTENT | Param_VISIBLE);
    
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
	AddScienceParam(0, UNIT_CONCENTRATION, "fixed_charge_density", "fixed charge density")->MakeVariable(true);

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

    // add parameters
    AddScienceParam(1, UNIT_DENSITY, "density", "density");
    
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

    // add parameters
    AddScienceParam(1, UNIT_DENSITY, "density", "density");
    AddScienceParam(0, UNIT_PRESSURE , "k", "bulk modulus" );
    
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
    // add parameters
    AddScienceParam(1, UNIT_DENSITY, "density", "density");
    
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
    // add parameters
    AddScienceParam(1, UNIT_DENSITY, "density", "density");
    AddScienceParam(0, UNIT_PRESSURE , "k", "bulk modulus" );
    
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
    AddScienceParam(1, UNIT_DENSITY, "density", "density");
    AddIntParam(1, "kinetics", "kinetics"); // "bond kinetics type (1 or 2)");
    AddIntParam(0, "trigger" , "trigger" ); // "bond breaking trigger (0=any, 1=distortion, or 2=dilatation)");
    AddScienceParam(0, UNIT_NONE, "wmin", "wmin");
    AddScienceParam(0, UNIT_NONE, "emin", "emin");

    // Add elastic material component
    AddProperty("elastic", FE_MAT_ELASTIC);
    
    // Add bond material component
	AddProperty("bond", FE_MAT_ELASTIC);
    
    // Add relaxation component
	AddProperty("relaxation", FE_MAT_RV_RELAX);
    
    // Add recruitment component
    AddProperty("recruitment", FE_MAT_DAMAGE);
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
    AddScienceParam(1, UNIT_DENSITY, "density", "density");
    AddScienceParam(0, UNIT_PRESSURE , "k", "bulk modulus" );
    AddScienceParam(0, UNIT_NONE, "wmin", "wmin");
    AddScienceParam(0, UNIT_NONE, "emin", "emin");

    // Add elastic material component
    AddProperty("elastic", FE_MAT_ELASTIC_UNCOUPLED);
    
    // Add bond material component
	AddProperty("bond", FE_MAT_ELASTIC_UNCOUPLED);
    
    // Add relaxation component
	AddProperty("relaxation", FE_MAT_RV_RELAX);
    
    // Add recruitment component
    AddProperty("recruitment", FE_MAT_DAMAGE);
}

//=============================================================================
//                        REACTIVE VISCOELASTIC DAMAGE
//=============================================================================

REGISTER_MATERIAL(FERVDamageMaterial, MODULE_MECH, FE_RV_DAMAGE_MATERIAL, FE_MAT_ELASTIC, "reactive viscoelastic damage", MaterialFlags::TOPLEVEL);

FERVDamageMaterial::FERVDamageMaterial() : FEMaterial(FE_RV_DAMAGE_MATERIAL)
{
    // add parameters
    AddScienceParam(1, UNIT_DENSITY, "density", "density");
    AddIntParam(1, "kinetics", "kinetics"); // "bond kinetics type (1 or 2)");
    AddIntParam(0, "trigger" , "trigger" ); // "bond breaking trigger (0=any, 1=distortion, or 2=dilatation)");
    AddScienceParam(0, UNIT_NONE, "wmin", "wmin");
    AddScienceParam(0, UNIT_NONE, "emin", "emin");
    
    // Add elastic material component
    AddProperty("elastic", FE_MAT_ELASTIC);
    
    // Add bond material component
    AddProperty("bond", FE_MAT_ELASTIC);
    
    // Add relaxation component
    AddProperty("relaxation", FE_MAT_RV_RELAX);
    
    // Add damage component
    AddProperty("damage", FE_MAT_DAMAGE);
    
    // Add criterion component
    AddProperty("criterion", FE_MAT_DAMAGE_CRITERION);
}

//=============================================================================
//							CHEMICAL REACTION
//=============================================================================

FEReactionMaterial::FEReactionMaterial(int ntype) : FEMaterial(ntype)
{
	// the optional Vbar parameter is hidden by default.
	AddScienceParam(0, UNIT_MOLAR_VOLUME, "Vbar", "Vbar")->SetState(Param_HIDDEN);
	AddBoolParam(false, 0, 0)->SetState(Param_HIDDEN);
    
	// Add reaction rate properties
	AddProperty("forward_rate", FE_MAT_REACTION_RATE);
	AddProperty("reverse_rate", FE_MAT_REACTION_RATE);

	AddProperty("vR", FE_MAT_REACTION_REACTANTS, FEMaterialProperty::NO_FIXED_SIZE, FEMaterialProperty::EDITABLE | FEMaterialProperty::NON_EXTENDABLE);
	AddProperty("vP", FE_MAT_REACTION_PRODUCTS , FEMaterialProperty::NO_FIXED_SIZE, FEMaterialProperty::EDITABLE | FEMaterialProperty::NON_EXTENDABLE);
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
		if (ri && (ri->GetReactantType() == FEReactionSpecies::SOLUTE_SPECIES)) solR.push_back(ri->GetIndex());
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
		if (ri && (ri->GetReactantType() == FEReactionSpecies::SBM_SPECIES)) sbmR.push_back(ri->GetIndex());
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
		if (ri && (ri->GetProductType() == FEReactionSpecies::SOLUTE_SPECIES)) solP.push_back(ri->GetIndex());
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
		if (ri && (ri->GetProductType() == FEReactionSpecies::SBM_SPECIES)) sbmP.push_back(ri->GetIndex());
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
		if (ntype == FEReactionSpecies::SOLUTE_SPECIES)
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
		if (ntype == FEReactionSpecies::SOLUTE_SPECIES)
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
//                            MEMBRANE REACTION
//=============================================================================

FEMembraneReactionMaterial::FEMembraneReactionMaterial(int ntype) : FEMaterial(ntype)
{
    // the optional Vbar parameter is hidden by default.
    AddScienceParam(0, UNIT_MOLAR_VOLUME, "Vbar", "Vbar")->SetState(Param_HIDDEN);
    AddBoolParam(false, 0, 0)->SetState(Param_HIDDEN);
    
    // Add reaction rate properties
    AddProperty("forward_rate", FE_MAT_MREACTION_RATE);
    AddProperty("reverse_rate", FE_MAT_MREACTION_RATE);
    
    AddProperty("vR" , FE_MAT_REACTION_REACTANTS  , FEMaterialProperty::NO_FIXED_SIZE, FEMaterialProperty::EDITABLE | FEMaterialProperty::NON_EXTENDABLE);
    AddProperty("vP" , FE_MAT_REACTION_PRODUCTS   , FEMaterialProperty::NO_FIXED_SIZE, FEMaterialProperty::EDITABLE | FEMaterialProperty::NON_EXTENDABLE);
    AddProperty("vRi", FE_MAT_MREACTION_IREACTANTS, FEMaterialProperty::NO_FIXED_SIZE, FEMaterialProperty::EDITABLE | FEMaterialProperty::NON_EXTENDABLE);
    AddProperty("vPi", FE_MAT_MREACTION_IPRODUCTS , FEMaterialProperty::NO_FIXED_SIZE, FEMaterialProperty::EDITABLE | FEMaterialProperty::NON_EXTENDABLE);
    AddProperty("vRe", FE_MAT_MREACTION_EREACTANTS, FEMaterialProperty::NO_FIXED_SIZE, FEMaterialProperty::EDITABLE | FEMaterialProperty::NON_EXTENDABLE);
    AddProperty("vPe", FE_MAT_MREACTION_EPRODUCTS , FEMaterialProperty::NO_FIXED_SIZE, FEMaterialProperty::EDITABLE | FEMaterialProperty::NON_EXTENDABLE);
}


void FEMembraneReactionMaterial::SetOvrd(bool bovrd) {
    SetBoolValue(MP_OVRD, bovrd);
    if (bovrd) GetParam(MP_VBAR).SetState(Param_ALLFLAGS);
    else GetParam(MP_VBAR).SetState(Param_HIDDEN);
}

bool FEMembraneReactionMaterial::GetOvrd() { return GetBoolValue(MP_OVRD); }

// set forward rate
void FEMembraneReactionMaterial::SetForwardRate(FEMaterial* pm) { ReplaceProperty(0, pm); }
FEMaterial* FEMembraneReactionMaterial::GetForwardRate() { return GetProperty(0).GetMaterial(); }

// set reverse rate
void FEMembraneReactionMaterial::SetReverseRate(FEMaterial* pm) { ReplaceProperty(1, pm); }
FEMaterial* FEMembraneReactionMaterial::GetReverseRate() { return GetProperty(1).GetMaterial(); }

int FEMembraneReactionMaterial::Reactants() { return GetProperty(2).Size(); }
int FEMembraneReactionMaterial::Products() { return GetProperty(3).Size(); }
int FEMembraneReactionMaterial::InternalReactants() { return GetProperty(4).Size(); }
int FEMembraneReactionMaterial::InternalProducts() { return GetProperty(5).Size(); }
int FEMembraneReactionMaterial::ExternalReactants() { return GetProperty(6).Size(); }
int FEMembraneReactionMaterial::ExternalProducts() { return GetProperty(7).Size(); }

FEReactantMaterial* FEMembraneReactionMaterial::Reactant(int i)
{
    return dynamic_cast<FEReactantMaterial*>(GetProperty(2).GetMaterial(i));
}

FEProductMaterial* FEMembraneReactionMaterial::Product(int i)
{
    return dynamic_cast<FEProductMaterial*>(GetProperty(3).GetMaterial(i));
}

FEInternalReactantMaterial* FEMembraneReactionMaterial::InternalReactant(int i)
{
    return dynamic_cast<FEInternalReactantMaterial*>(GetProperty(4).GetMaterial(i));
}

FEInternalProductMaterial* FEMembraneReactionMaterial::InternalProduct(int i)
{
    return dynamic_cast<FEInternalProductMaterial*>(GetProperty(5).GetMaterial(i));
}

FEExternalReactantMaterial* FEMembraneReactionMaterial::ExternalReactant(int i)
{
    return dynamic_cast<FEExternalReactantMaterial*>(GetProperty(6).GetMaterial(i));
}

FEExternalProductMaterial* FEMembraneReactionMaterial::ExternalProduct(int i)
{
    return dynamic_cast<FEExternalProductMaterial*>(GetProperty(7).GetMaterial(i));
}

// add reactant/product component
void FEMembraneReactionMaterial::AddReactantMaterial(FEReactantMaterial* pm) { AddProperty(2, pm); }
void FEMembraneReactionMaterial::AddProductMaterial (FEProductMaterial* pm) { AddProperty(3, pm); }
void FEMembraneReactionMaterial::AddInternalReactantMaterial(FEInternalReactantMaterial* pm) { AddProperty(4, pm); }
void FEMembraneReactionMaterial::AddInternalProductMaterial (FEInternalProductMaterial* pm) { AddProperty(5, pm); }
void FEMembraneReactionMaterial::AddExternalReactantMaterial(FEExternalReactantMaterial* pm) { AddProperty(6, pm); }
void FEMembraneReactionMaterial::AddExternalProductMaterial (FEExternalProductMaterial* pm) { AddProperty(7, pm); }

void FEMembraneReactionMaterial::ClearReactants()
{
    GetProperty(2).Clear();
    GetProperty(4).Clear();
    GetProperty(6).Clear();
}

void FEMembraneReactionMaterial::ClearProducts()
{
    GetProperty(3).Clear();
    GetProperty(5).Clear();
    GetProperty(7).Clear();
}

void FEMembraneReactionMaterial::GetSoluteReactants(vector<int>& solR)
{
    solR.clear();
    FEMaterialProperty& p = GetProperty(2);
    int N = p.Size();
    for (int i=0; i<N; ++i)
    {
        FEReactantMaterial* ri = dynamic_cast<FEReactantMaterial*>(p.GetMaterial(i)); assert(ri);
        if (ri && (ri->GetReactantType() == FEReactionSpecies::SOLUTE_SPECIES)) solR.push_back(ri->GetIndex());
    }
}

void FEMembraneReactionMaterial::GetInternalSoluteReactants(vector<int>& solRi)
{
    solRi.clear();
    FEMaterialProperty& p = GetProperty(4);
    int N = p.Size();
    for (int i=0; i<N; ++i)
    {
        FEInternalReactantMaterial* ri = dynamic_cast<FEInternalReactantMaterial*>(p.GetMaterial(i)); assert(ri);
        if (ri && (ri->GetReactantType() == FEReactionSpecies::SOLUTE_SPECIES)) solRi.push_back(ri->GetIndex());
    }
}

void FEMembraneReactionMaterial::GetExternalSoluteReactants(vector<int>& solRe)
{
    solRe.clear();
    FEMaterialProperty& p = GetProperty(6);
    int N = p.Size();
    for (int i=0; i<N; ++i)
    {
        FEExternalReactantMaterial* ri = dynamic_cast<FEExternalReactantMaterial*>(p.GetMaterial(i)); assert(ri);
        if (ri && (ri->GetReactantType() == FEReactionSpecies::SOLUTE_SPECIES)) solRe.push_back(ri->GetIndex());
    }
}

void FEMembraneReactionMaterial::GetSBMReactants(vector<int>& sbmR)
{
    sbmR.clear();
    FEMaterialProperty& p = GetProperty(2);
    int N = p.Size();
    for (int i = 0; i<N; ++i)
    {
        FEReactantMaterial* ri = dynamic_cast<FEReactantMaterial*>(p.GetMaterial(i)); assert(ri);
        if (ri && (ri->GetReactantType() == FEReactionSpecies::SBM_SPECIES)) sbmR.push_back(ri->GetIndex());
    }
}

void FEMembraneReactionMaterial::GetSoluteProducts(vector<int>& solP)
{
    solP.clear();
    FEMaterialProperty& p = GetProperty(3);
    int N = p.Size();
    for (int i = 0; i<N; ++i)
    {
        FEProductMaterial* ri = dynamic_cast<FEProductMaterial*>(p.GetMaterial(i)); assert(ri);
        if (ri && (ri->GetProductType() == FEReactionSpecies::SOLUTE_SPECIES)) solP.push_back(ri->GetIndex());
    }
}

void FEMembraneReactionMaterial::GetInternalSoluteProducts(vector<int>& solPi)
{
    solPi.clear();
    FEMaterialProperty& p = GetProperty(5);
    int N = p.Size();
    for (int i = 0; i<N; ++i)
    {
        FEInternalProductMaterial* ri = dynamic_cast<FEInternalProductMaterial*>(p.GetMaterial(i)); assert(ri);
        if (ri && (ri->GetProductType() == FEReactionSpecies::SOLUTE_SPECIES)) solPi.push_back(ri->GetIndex());
    }
}

void FEMembraneReactionMaterial::GetExternalSoluteProducts(vector<int>& solPe)
{
    solPe.clear();
    FEMaterialProperty& p = GetProperty(7);
    int N = p.Size();
    for (int i = 0; i<N; ++i)
    {
        FEExternalProductMaterial* ri = dynamic_cast<FEExternalProductMaterial*>(p.GetMaterial(i)); assert(ri);
        if (ri && (ri->GetProductType() == FEReactionSpecies::SOLUTE_SPECIES)) solPe.push_back(ri->GetIndex());
    }
}

void FEMembraneReactionMaterial::GetSBMProducts(vector<int>& sbmP)
{
    sbmP.clear();
    FEMaterialProperty& p = GetProperty(3);
    int N = p.Size();
    for (int i = 0; i<N; ++i)
    {
        FEProductMaterial* ri = dynamic_cast<FEProductMaterial*>(p.GetMaterial(i)); assert(ri);
        if (ri && (ri->GetProductType() == FEReactionSpecies::SBM_SPECIES)) sbmP.push_back(ri->GetIndex());
    }
}

string buildMembraneReactionEquation(FEMembraneReactionMaterial* mat, FEModel& fem)
{
    stringstream ss;
    int NR = mat->Reactants();
    for (int i = 0; i<NR; ++i)
    {
        FEReactantMaterial* rm = mat->Reactant(i);
        
        string name;
        int m = rm->GetIndex();
        int ntype = rm->GetReactantType();
        if (ntype == FEReactionSpecies::SOLUTE_SPECIES)
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
    
    int NRI = mat->InternalReactants();
    for (int i = 0; i<NRI; ++i)
    {
        FEInternalReactantMaterial* rm = mat->InternalReactant(i);
        
        string name;
        int m = rm->GetIndex();
        int ntype = rm->GetReactantType();
        if (ntype == FEReactionSpecies::SOLUTE_SPECIES)
            name = fem.GetSoluteData(m).GetName();
        
        int n = rm->GetCoef();
        if (n == 1)
            ss << name << "i";
        else
            ss << n << "*" << name << "i";
        if (i != NRI - 1) ss << "+";
    }
    
    int NRE = mat->ExternalReactants();
    for (int i = 0; i<NRE; ++i)
    {
        FEExternalReactantMaterial* rm = mat->ExternalReactant(i);
        
        string name;
        int m = rm->GetIndex();
        int ntype = rm->GetReactantType();
        if (ntype == FEReactionSpecies::SOLUTE_SPECIES)
            name = fem.GetSoluteData(m).GetName();
        
        int n = rm->GetCoef();
        if (n == 1)
            ss << name << "e";
        else
            ss << n << "*" << name << "e";
        if (i != NRE - 1) ss << "+";
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
        if (ntype == FEReactionSpecies::SOLUTE_SPECIES)
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
    
    int NPI = mat->InternalProducts();
    for (int i = 0; i<NPI; ++i)
    {
        FEInternalProductMaterial* pm = mat->InternalProduct(i);
        
        string name;
        int m = pm->GetIndex();
        int ntype = pm->GetProductType();
        if (ntype == FEReactionSpecies::SOLUTE_SPECIES)
            name = fem.GetSoluteData(m).GetName();
        
        int n = pm->GetCoef();
        if (n == 1)
            ss << name << "i";
        else
            ss << n << "*" << name << "i";
        if (i != NPI - 1) ss << "+";
    }
    
    int NPE = mat->ExternalProducts();
    for (int i = 0; i<NPE; ++i)
    {
        FEExternalProductMaterial* pm = mat->ExternalProduct(i);
        
        string name;
        int m = pm->GetIndex();
        int ntype = pm->GetProductType();
        if (ntype == FEReactionSpecies::SOLUTE_SPECIES)
            name = fem.GetSoluteData(m).GetName();
        
        int n = pm->GetCoef();
        if (n == 1)
            ss << name << "e";
        else
            ss << n << "*" << name << "e";
        if (i != NPI - 1) ss << "+";
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
	AddScienceParam(0, UNIT_CONCENTRATION, "fixed_charge_density", "fixed charge density")->MakeVariable(true);

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
	AddProperty("reaction", FE_MAT_REACTION, FEMaterialProperty::NO_FIXED_SIZE, FEMaterialProperty::NON_EXTENDABLE);

    // add reaction material
    AddProperty("membrane_reaction", FE_MAT_MREACTION, FEMaterialProperty::NO_FIXED_SIZE, FEMaterialProperty::NON_EXTENDABLE);
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

// add membrane reaction component
void FEMultiphasicMaterial::AddMembraneReactionMaterial(FEMembraneReactionMaterial* pm)
{
    GetProperty(MREACTION).AddMaterial(pm);
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

// count membrane reaction components
int FEMultiphasicMaterial::MembraneReactions()
{
    return GetProperty(MREACTION).Size();
}

// get reaction component
FEMembraneReactionMaterial* FEMultiphasicMaterial::GetMembraneReaction(int n)
{
    FEMaterialProperty& p = GetProperty(MREACTION);
    FEMembraneReactionMaterial* prm = dynamic_cast<FEMembraneReactionMaterial*>(p.GetMaterial(n));
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
FEReactionSpecies::FEReactionSpecies(int ntype) : FEMaterial(ntype)
{
    // add the stoichiometric coefficient
    AddIntParam(1, "v", "v"); // stoichiometric coefficient
    // add the type
    AddIntParam(-1, 0, 0)->SetState(Param_HIDDEN);
    // add the index
    AddIntParam(-1, 0, 0)->SetState(Param_HIDDEN);
}

void FEReactionSpecies::Load(IArchive& ar)
{
    FEMaterial::Load(ar);

    // This is a temporary hack to remove species type > 2, which should now all be solutes
    if (GetSpeciesType() > 2) SetSpeciesType(SOLUTE_SPECIES);

}

//=============================================================================
//								REACTANT
//=============================================================================

REGISTER_MATERIAL(FEReactantMaterial, MODULE_REACTIONS, FE_REACTANT_MATERIAL, FE_MAT_REACTION_REACTANTS, "Reactant", 0);

FEReactantMaterial::FEReactantMaterial() : FEReactionSpecies(FE_REACTANT_MATERIAL)
{
}

//=============================================================================
//								PRODUCT
//=============================================================================

REGISTER_MATERIAL(FEProductMaterial, MODULE_REACTIONS, FE_PRODUCT_MATERIAL, FE_MAT_REACTION_PRODUCTS, "Product", 0);

FEProductMaterial::FEProductMaterial() : FEReactionSpecies(FE_PRODUCT_MATERIAL)
{
}

//=============================================================================
//                          INTERNAL REACTANT
//=============================================================================

REGISTER_MATERIAL(FEInternalReactantMaterial, MODULE_REACTIONS, FE_INT_REACTANT_MATERIAL, FE_MAT_MREACTION_IREACTANTS, "Internal Reactant", 0);

FEInternalReactantMaterial::FEInternalReactantMaterial() : FEReactionSpecies(FE_INT_REACTANT_MATERIAL)
{
}

//=============================================================================
//                           INTERNAL PRODUCT
//=============================================================================

REGISTER_MATERIAL(FEInternalProductMaterial, MODULE_REACTIONS, FE_INT_PRODUCT_MATERIAL, FE_MAT_MREACTION_IPRODUCTS, "Internal Product", 0);

FEInternalProductMaterial::FEInternalProductMaterial() : FEReactionSpecies(FE_INT_PRODUCT_MATERIAL)
{
}

//=============================================================================
//                          EXTERNAL REACTANT
//=============================================================================

REGISTER_MATERIAL(FEExternalReactantMaterial, MODULE_REACTIONS, FE_EXT_REACTANT_MATERIAL, FE_MAT_MREACTION_EREACTANTS, "External Reactant", 0);

FEExternalReactantMaterial::FEExternalReactantMaterial() : FEReactionSpecies(FE_EXT_REACTANT_MATERIAL)
{
}

//=============================================================================
//                           EXTERNAL PRODUCT
//=============================================================================

REGISTER_MATERIAL(FEExternalProductMaterial, MODULE_REACTIONS, FE_EXT_PRODUCT_MATERIAL, FE_MAT_MREACTION_EPRODUCTS, "External Product", 0);

FEExternalProductMaterial::FEExternalProductMaterial() : FEReactionSpecies(FE_EXT_PRODUCT_MATERIAL)
{
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
//                         MEMBRANE MASS ACTION FORWARD REACTION
//=============================================================================

REGISTER_MATERIAL(FEMembraneMassActionForward, MODULE_REACTIONS, FE_MMASS_ACTION_FORWARD, FE_MAT_MREACTION, "membrane-mass-action-forward", 0);

FEMembraneMassActionForward::FEMembraneMassActionForward() : FEMembraneReactionMaterial(FE_MMASS_ACTION_FORWARD)
{
}

//=============================================================================
//                         MEMBRANE MASS ACTION REVERSIBLE REACTION
//=============================================================================

REGISTER_MATERIAL(FEMembraneMassActionReversible, MODULE_REACTIONS, FE_MMASS_ACTION_REVERSIBLE, FE_MAT_MREACTION, "membrane-mass-action-reversible", 0);

FEMembraneMassActionReversible::FEMembraneMassActionReversible() : FEMembraneReactionMaterial(FE_MMASS_ACTION_REVERSIBLE)
{
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
	AddScienceParam(1.0, UNIT_DENSITY, "density");

	// Add one component for the elastic material
	AddProperty("elastic", FE_MAT_ELASTIC | FE_MAT_ELASTIC_UNCOUPLED);
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
    AddBoolParam   (true,"isochoric", "isochoric plastic flow");
    
    // Add one component for the elastic material
    AddProperty("elastic", FE_MAT_ELASTIC);
    
    // Add criterion component
    AddProperty("yield_criterion", FE_MAT_DAMAGE_CRITERION);

    // add flow curve property
    AddProperty("flow_curve", FE_MAT_PLASTIC_FLOW_RULE);
}

//=============================================================================
//                            REACTIVE PLASTIC DAMAGE
//=============================================================================

REGISTER_MATERIAL(FEReactivePlasticDamage, MODULE_MECH, FE_REACTIVE_PLASTIC_DAMAGE, FE_MAT_ELASTIC, "reactive plastic damage", MaterialFlags::TOPLEVEL);

FEReactivePlasticDamage::FEReactivePlasticDamage() : FEMaterial(FE_REACTIVE_PLASTIC_DAMAGE)
{
    AddScienceParam(1, UNIT_DENSITY, "density", "density"     )->SetPersistent(false);
    AddBoolParam(true, "isochoric", "isochoric plastic flow");

    // Add one component for the elastic material
    AddProperty("elastic", FE_MAT_ELASTIC);
    
    // Add criterion component
    AddProperty("yield_criterion", FE_MAT_DAMAGE_CRITERION);
    
    // add flow curve property
    AddProperty("flow_curve", FE_MAT_PLASTIC_FLOW_RULE);

    // Add yield damage material
    AddProperty("plastic_damage", FE_MAT_DAMAGE);

    // Add yield damage criterion component
    AddProperty("plastic_damage_criterion", FE_MAT_DAMAGE_CRITERION);
    
    // Add intact damage material
    AddProperty("elastic_damage", FE_MAT_DAMAGE);
    
    // Add intact damage criterion component
    AddProperty("elastic_damage_criterion", FE_MAT_DAMAGE_CRITERION);    
}

//=============================================================================
//                            REACTIVE FATIGUE
//=============================================================================

REGISTER_MATERIAL(FEReactiveFatigue, MODULE_MECH, FE_REACTIVE_FATIGUE, FE_MAT_ELASTIC, "reactive fatigue", MaterialFlags::TOPLEVEL);

FEReactiveFatigue::FEReactiveFatigue() : FEMaterial(FE_REACTIVE_FATIGUE)
{
    AddScienceParam(1, UNIT_DENSITY, "density", "density"     )->SetPersistent(false);
    AddScienceParam(0, UNIT_NONE, "k0", "fatigue reaction rate");
    AddScienceParam(1, UNIT_NONE, "beta", "fatigue reaction exponent");

    // Add one component for the elastic material
    AddProperty("elastic", FE_MAT_ELASTIC);
    
    // Add elastic damage material
    AddProperty("elastic_damage", FE_MAT_DAMAGE);
    
    // Add elastic damage criterion component
    AddProperty("elastic_criterion", FE_MAT_DAMAGE_CRITERION);
    
    // Add fatigue damage material
    AddProperty("fatigue_damage", FE_MAT_DAMAGE);
    
    // Add fatigue damage criterion component
    AddProperty("fatigue_criterion", FE_MAT_DAMAGE_CRITERION);
    
}

//=============================================================================
//                        UNCOUPLED REACTIVE FATIGUE
//=============================================================================

REGISTER_MATERIAL(FEUncoupledReactiveFatigue, MODULE_MECH, FE_UNCOUPLED_REACTIVE_FATIGUE, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled reactive fatigue", MaterialFlags::TOPLEVEL);

FEUncoupledReactiveFatigue::FEUncoupledReactiveFatigue() : FEMaterial(FE_UNCOUPLED_REACTIVE_FATIGUE)
{
    AddScienceParam(1, UNIT_DENSITY, "density", "density"     )->SetPersistent(false);
    AddScienceParam(0, UNIT_NONE, "k0", "fatigue reaction rate");
    AddScienceParam(1, UNIT_NONE, "beta", "fatigue reaction exponent");
    AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus");

    // Add one component for the elastic material
    AddProperty("elastic", FE_MAT_ELASTIC_UNCOUPLED);
    
    // Add elastic damage material
    AddProperty("elastic_damage", FE_MAT_DAMAGE);
    
    // Add elastic damage criterion component
    AddProperty("elastic_criterion", FE_MAT_DAMAGE_CRITERION);
    
    // Add fatigue damage material
    AddProperty("fatigue_damage", FE_MAT_DAMAGE);
    
    // Add fatigue damage criterion component
    AddProperty("fatigue_criterion", FE_MAT_DAMAGE_CRITERION);
    
}
