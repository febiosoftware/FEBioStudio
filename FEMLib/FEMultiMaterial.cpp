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
#include <FECore/units.h>

using std::stringstream;

//=============================================================================
//								VISCO-ELASTIC
//=============================================================================

REGISTER_MATERIAL(FSViscoElastic, MODULE_MECH, FE_VISCO_ELASTIC, FE_MAT_ELASTIC, "viscoelastic", MaterialFlags::TOPLEVEL);

FSViscoElastic::FSViscoElastic() : FSMaterial(FE_VISCO_ELASTIC)
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

REGISTER_MATERIAL(FSUncoupledViscoElastic, MODULE_MECH, FE_UNCOUPLED_VISCO_ELASTIC, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled viscoelastic", MaterialFlags::TOPLEVEL);

FSUncoupledViscoElastic::FSUncoupledViscoElastic() : FSMaterial(FE_UNCOUPLED_VISCO_ELASTIC)
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
FSMultiMaterial::FSMultiMaterial(int ntype) : FSMaterial(ntype)
{

}

//=============================================================================
//									BIPHASIC
//=============================================================================

REGISTER_MATERIAL(FSBiphasic, MODULE_BIPHASIC, FE_BIPHASIC_MATERIAL, FE_MAT_MULTIPHASIC, "biphasic", MaterialFlags::TOPLEVEL);

FSBiphasic::FSBiphasic() : FSMultiMaterial(FE_BIPHASIC_MATERIAL)
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

REGISTER_MATERIAL(FSSoluteMaterial, MODULE_MULTIPHASIC, FE_SOLUTE_MATERIAL, FE_MAT_SOLUTE, "solute", 0);

FSSoluteMaterial::FSSoluteMaterial() : FSMaterial(FE_SOLUTE_MATERIAL)
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

REGISTER_MATERIAL(FSSBMMaterial, MODULE_MULTIPHASIC, FE_SBM_MATERIAL, FE_MAT_SBM, "solid_bound", 0);

FSSBMMaterial::FSSBMMaterial() : FSMaterial(FE_SBM_MATERIAL)
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

REGISTER_MATERIAL(FSBiphasicSolute, MODULE_MULTIPHASIC, FE_BIPHASIC_SOLUTE, FE_MAT_MULTIPHASIC, "biphasic-solute", MaterialFlags::TOPLEVEL);

FSBiphasicSolute::FSBiphasicSolute() : FSMultiMaterial(FE_BIPHASIC_SOLUTE)
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

REGISTER_MATERIAL(FSTriphasicMaterial, MODULE_MULTIPHASIC, FE_TRIPHASIC_MATERIAL, FE_MAT_MULTIPHASIC, "triphasic", MaterialFlags::TOPLEVEL);

FSTriphasicMaterial::FSTriphasicMaterial() : FSMultiMaterial(FE_TRIPHASIC_MATERIAL)
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
void FSTriphasicMaterial::SetSoluteMaterial(FSSoluteMaterial* pm, int i)
{ 
	ReplaceProperty(3, pm, i); 
}

FSMaterial* FSTriphasicMaterial::GetSoluteMaterial(int i) 
{ 
	return GetProperty(3).GetMaterial(i); 
}

//=============================================================================
//								SOLID MIXTURE
//=============================================================================

REGISTER_MATERIAL(FSSolidMixture, MODULE_MECH, FE_SOLID_MIXTURE, FE_MAT_ELASTIC, "solid mixture", MaterialFlags::TOPLEVEL);

//-----------------------------------------------------------------------------
FSSolidMixture::FSSolidMixture() : FSMaterial(FE_SOLID_MIXTURE)
{
    AddScienceParam(1, UNIT_DENSITY, "density", "density");
    
	AddProperty("solid", FE_MAT_ELASTIC, FSMaterialProperty::NO_FIXED_SIZE);
}

//=============================================================================
//								UNCOUPLED SOLID MIXTURE
//=============================================================================

REGISTER_MATERIAL(FSUncoupledSolidMixture, MODULE_MECH, FE_UNCOUPLED_SOLID_MIXTURE, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled solid mixture", MaterialFlags::TOPLEVEL);

FSUncoupledSolidMixture::FSUncoupledSolidMixture() : FSMaterial(FE_UNCOUPLED_SOLID_MIXTURE) 
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density");
	AddScienceParam(0, UNIT_PRESSURE , "k", "bulk modulus" );

	AddProperty("solid", FE_MAT_ELASTIC_UNCOUPLED, FSMaterialProperty::NO_FIXED_SIZE);
}

//=============================================================================
//							CONTINUOUS FIBER DISTRIBUTION
//=============================================================================

REGISTER_MATERIAL(FSCFDMaterial, MODULE_MECH, FE_CFD_MATERIAL, FE_MAT_ELASTIC, "continuous fiber distribution", 0);

FSCFDMaterial::FSCFDMaterial() : FSMaterial(FE_CFD_MATERIAL)
{
	SetAxisMaterial(new FSAxisMaterial);

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

REGISTER_MATERIAL(FSCFDUCMaterial, MODULE_MECH, FE_CFD_MATERIAL_UC, FE_MAT_ELASTIC_UNCOUPLED, "continuous fiber distribution uncoupled", 0);

FSCFDUCMaterial::FSCFDUCMaterial() : FSMaterial(FE_CFD_MATERIAL_UC)
{
	SetAxisMaterial(new FSAxisMaterial);

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

REGISTER_MATERIAL(FSElasticDamageMaterial, MODULE_MECH, FE_DMG_MATERIAL, FE_MAT_ELASTIC, "elastic damage", MaterialFlags::TOPLEVEL);

FSElasticDamageMaterial::FSElasticDamageMaterial() : FSMaterial(FE_DMG_MATERIAL)
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

REGISTER_MATERIAL(FSElasticDamageMaterialUC, MODULE_MECH, FE_DMG_MATERIAL_UC, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled elastic damage", MaterialFlags::TOPLEVEL);

FSElasticDamageMaterialUC::FSElasticDamageMaterialUC() : FSMaterial(FE_DMG_MATERIAL_UC)
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

REGISTER_MATERIAL(FSReactiveViscoelasticMaterial, MODULE_MECH, FE_RV_MATERIAL, FE_MAT_ELASTIC, "reactive viscoelastic", MaterialFlags::TOPLEVEL);

FSReactiveViscoelasticMaterial::FSReactiveViscoelasticMaterial() : FSMaterial(FE_RV_MATERIAL)
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
}

//=============================================================================
//                      UNCOUPLED REACTIVE VISCOELASTIC
//=============================================================================

REGISTER_MATERIAL(FSReactiveViscoelasticMaterialUC, MODULE_MECH, FE_RV_MATERIAL_UC, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled reactive viscoelastic", MaterialFlags::TOPLEVEL);

FSReactiveViscoelasticMaterialUC::FSReactiveViscoelasticMaterialUC() : FSMaterial(FE_RV_MATERIAL_UC)
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
}

//=============================================================================
//							CHEMICAL REACTION
//=============================================================================

FSReactionMaterial::FSReactionMaterial(int ntype) : FSMaterial(ntype)
{
	// the optional Vbar parameter is hidden by default.
	AddScienceParam(0, UNIT_MOLAR_VOLUME, "Vbar", "Vbar")->SetState(Param_HIDDEN);
	AddBoolParam(false, 0, 0)->SetState(Param_HIDDEN);
    
	// Add reaction rate properties
	AddProperty("forward_rate", FE_MAT_REACTION_RATE);
	AddProperty("reverse_rate", FE_MAT_REACTION_RATE);

	AddProperty("vR", FE_MAT_REACTION_REACTANTS, FSMaterialProperty::NO_FIXED_SIZE, FSMaterialProperty::EDITABLE | FSMaterialProperty::NON_EXTENDABLE);
	AddProperty("vP", FE_MAT_REACTION_PRODUCTS , FSMaterialProperty::NO_FIXED_SIZE, FSMaterialProperty::EDITABLE | FSMaterialProperty::NON_EXTENDABLE);
}


void FSReactionMaterial::SetOvrd(bool bovrd) {
	SetBoolValue(MP_OVRD, bovrd);
	if (bovrd) GetParam(MP_VBAR).SetState(Param_ALLFLAGS);
	else GetParam(MP_VBAR).SetState(Param_HIDDEN);
}

bool FSReactionMaterial::GetOvrd() { return GetBoolValue(MP_OVRD); }

// set forward rate
void FSReactionMaterial::SetForwardRate(FSMaterial* pm) { ReplaceProperty(0, pm); }
FSMaterial* FSReactionMaterial::GetForwardRate() { return GetProperty(0).GetMaterial(); }

// set reverse rate
void FSReactionMaterial::SetReverseRate(FSMaterial* pm) { ReplaceProperty(1, pm); }
FSMaterial* FSReactionMaterial::GetReverseRate() { return GetProperty(1).GetMaterial(); }

int FSReactionMaterial::Reactants() { return GetProperty(2).Size(); }
int FSReactionMaterial::Products() { return GetProperty(3).Size(); }

FSReactantMaterial* FSReactionMaterial::Reactant(int i)
{
	return dynamic_cast<FSReactantMaterial*>(GetProperty(2).GetMaterial(i));
}

FSProductMaterial* FSReactionMaterial::Product(int i)
{
	return dynamic_cast<FSProductMaterial*>(GetProperty(3).GetMaterial(i));
}

// add reactant/product component
void FSReactionMaterial::AddReactantMaterial(FSReactantMaterial* pm) { AddProperty(2, pm); }
void FSReactionMaterial::AddProductMaterial (FSProductMaterial* pm) { AddProperty(3, pm); }

void FSReactionMaterial::ClearReactants()
{
	GetProperty(2).Clear();
}

void FSReactionMaterial::ClearProducts()
{
	GetProperty(3).Clear();
}

void FSReactionMaterial::GetSoluteReactants(vector<int>& solR)
{
	solR.clear();
	FSMaterialProperty& p = GetProperty(2);
	int N = p.Size();
	for (int i=0; i<N; ++i)
	{
		FSReactantMaterial* ri = dynamic_cast<FSReactantMaterial*>(p.GetMaterial(i)); assert(ri);
		if (ri && (ri->GetReactantType() == FSReactionSpecies::SOLUTE_SPECIES)) solR.push_back(ri->GetIndex());
	}
}

void FSReactionMaterial::GetSBMReactants(vector<int>& sbmR)
{
	sbmR.clear();
	FSMaterialProperty& p = GetProperty(2);
	int N = p.Size();
	for (int i = 0; i<N; ++i)
	{
		FSReactantMaterial* ri = dynamic_cast<FSReactantMaterial*>(p.GetMaterial(i)); assert(ri);
		if (ri && (ri->GetReactantType() == FSReactionSpecies::SBM_SPECIES)) sbmR.push_back(ri->GetIndex());
	}
}

void FSReactionMaterial::GetSoluteProducts(vector<int>& solP)
{
	solP.clear();
	FSMaterialProperty& p = GetProperty(3);
	int N = p.Size();
	for (int i = 0; i<N; ++i)
	{
		FSProductMaterial* ri = dynamic_cast<FSProductMaterial*>(p.GetMaterial(i)); assert(ri);
		if (ri && (ri->GetProductType() == FSReactionSpecies::SOLUTE_SPECIES)) solP.push_back(ri->GetIndex());
	}
}

void FSReactionMaterial::GetSBMProducts(vector<int>& sbmP)
{
	sbmP.clear();
	FSMaterialProperty& p = GetProperty(3);
	int N = p.Size();
	for (int i = 0; i<N; ++i)
	{
		FSProductMaterial* ri = dynamic_cast<FSProductMaterial*>(p.GetMaterial(i)); assert(ri);
		if (ri && (ri->GetProductType() == FSReactionSpecies::SBM_SPECIES)) sbmP.push_back(ri->GetIndex());
	}
}

string buildReactionEquation(FSReactionMaterial* mat, FSModel& fem)
{
	stringstream ss;
	int NR = mat->Reactants();
	for (int i = 0; i<NR; ++i)
	{
		FSReactantMaterial* rm = mat->Reactant(i);

		string name;
		int m = rm->GetIndex();
		int ntype = rm->GetReactantType();
		if (ntype == FSReactionSpecies::SOLUTE_SPECIES)
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
		FSProductMaterial* pm = mat->Product(i);

		string name;
		int m = pm->GetIndex();
		int ntype = pm->GetProductType();
		if (ntype == FSReactionSpecies::SOLUTE_SPECIES)
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

FSMembraneReactionMaterial::FSMembraneReactionMaterial(int ntype) : FSMaterial(ntype)
{
    // the optional Vbar parameter is hidden by default.
    AddScienceParam(0, UNIT_MOLAR_VOLUME, "Vbar", "Vbar")->SetState(Param_HIDDEN);
    AddBoolParam(false, 0, 0)->SetState(Param_HIDDEN);
    
    // Add reaction rate properties
    AddProperty("forward_rate", FE_MAT_MREACTION_RATE);
    AddProperty("reverse_rate", FE_MAT_MREACTION_RATE);
    
    AddProperty("vR" , FE_MAT_REACTION_REACTANTS  , FSMaterialProperty::NO_FIXED_SIZE, FSMaterialProperty::EDITABLE | FSMaterialProperty::NON_EXTENDABLE);
    AddProperty("vP" , FE_MAT_REACTION_PRODUCTS   , FSMaterialProperty::NO_FIXED_SIZE, FSMaterialProperty::EDITABLE | FSMaterialProperty::NON_EXTENDABLE);
    AddProperty("vRi", FE_MAT_MREACTION_IREACTANTS, FSMaterialProperty::NO_FIXED_SIZE, FSMaterialProperty::EDITABLE | FSMaterialProperty::NON_EXTENDABLE);
    AddProperty("vPi", FE_MAT_MREACTION_IPRODUCTS , FSMaterialProperty::NO_FIXED_SIZE, FSMaterialProperty::EDITABLE | FSMaterialProperty::NON_EXTENDABLE);
    AddProperty("vRe", FE_MAT_MREACTION_EREACTANTS, FSMaterialProperty::NO_FIXED_SIZE, FSMaterialProperty::EDITABLE | FSMaterialProperty::NON_EXTENDABLE);
    AddProperty("vPe", FE_MAT_MREACTION_EPRODUCTS , FSMaterialProperty::NO_FIXED_SIZE, FSMaterialProperty::EDITABLE | FSMaterialProperty::NON_EXTENDABLE);
}


void FSMembraneReactionMaterial::SetOvrd(bool bovrd) {
    SetBoolValue(MP_OVRD, bovrd);
    if (bovrd) GetParam(MP_VBAR).SetState(Param_ALLFLAGS);
    else GetParam(MP_VBAR).SetState(Param_HIDDEN);
}

bool FSMembraneReactionMaterial::GetOvrd() { return GetBoolValue(MP_OVRD); }

// set forward rate
void FSMembraneReactionMaterial::SetForwardRate(FSMaterial* pm) { ReplaceProperty(0, pm); }
FSMaterial* FSMembraneReactionMaterial::GetForwardRate() { return GetProperty(0).GetMaterial(); }

// set reverse rate
void FSMembraneReactionMaterial::SetReverseRate(FSMaterial* pm) { ReplaceProperty(1, pm); }
FSMaterial* FSMembraneReactionMaterial::GetReverseRate() { return GetProperty(1).GetMaterial(); }

int FSMembraneReactionMaterial::Reactants() { return GetProperty(2).Size(); }
int FSMembraneReactionMaterial::Products() { return GetProperty(3).Size(); }
int FSMembraneReactionMaterial::InternalReactants() { return GetProperty(4).Size(); }
int FSMembraneReactionMaterial::InternalProducts() { return GetProperty(5).Size(); }
int FSMembraneReactionMaterial::ExternalReactants() { return GetProperty(6).Size(); }
int FSMembraneReactionMaterial::ExternalProducts() { return GetProperty(7).Size(); }

FSReactantMaterial* FSMembraneReactionMaterial::Reactant(int i)
{
    return dynamic_cast<FSReactantMaterial*>(GetProperty(2).GetMaterial(i));
}

FSProductMaterial* FSMembraneReactionMaterial::Product(int i)
{
    return dynamic_cast<FSProductMaterial*>(GetProperty(3).GetMaterial(i));
}

FSInternalReactantMaterial* FSMembraneReactionMaterial::InternalReactant(int i)
{
    return dynamic_cast<FSInternalReactantMaterial*>(GetProperty(4).GetMaterial(i));
}

FSInternalProductMaterial* FSMembraneReactionMaterial::InternalProduct(int i)
{
    return dynamic_cast<FSInternalProductMaterial*>(GetProperty(5).GetMaterial(i));
}

FSExternalReactantMaterial* FSMembraneReactionMaterial::ExternalReactant(int i)
{
    return dynamic_cast<FSExternalReactantMaterial*>(GetProperty(6).GetMaterial(i));
}

FSExternalProductMaterial* FSMembraneReactionMaterial::ExternalProduct(int i)
{
    return dynamic_cast<FSExternalProductMaterial*>(GetProperty(7).GetMaterial(i));
}

// add reactant/product component
void FSMembraneReactionMaterial::AddReactantMaterial(FSReactantMaterial* pm) { AddProperty(2, pm); }
void FSMembraneReactionMaterial::AddProductMaterial (FSProductMaterial* pm) { AddProperty(3, pm); }
void FSMembraneReactionMaterial::AddInternalReactantMaterial(FSInternalReactantMaterial* pm) { AddProperty(4, pm); }
void FSMembraneReactionMaterial::AddInternalProductMaterial (FSInternalProductMaterial* pm) { AddProperty(5, pm); }
void FSMembraneReactionMaterial::AddExternalReactantMaterial(FSExternalReactantMaterial* pm) { AddProperty(6, pm); }
void FSMembraneReactionMaterial::AddExternalProductMaterial (FSExternalProductMaterial* pm) { AddProperty(7, pm); }

void FSMembraneReactionMaterial::ClearReactants()
{
    GetProperty(2).Clear();
    GetProperty(4).Clear();
    GetProperty(6).Clear();
}

void FSMembraneReactionMaterial::ClearProducts()
{
    GetProperty(3).Clear();
    GetProperty(5).Clear();
    GetProperty(7).Clear();
}

void FSMembraneReactionMaterial::GetSoluteReactants(vector<int>& solR)
{
    solR.clear();
    FSMaterialProperty& p = GetProperty(2);
    int N = p.Size();
    for (int i=0; i<N; ++i)
    {
        FSReactantMaterial* ri = dynamic_cast<FSReactantMaterial*>(p.GetMaterial(i)); assert(ri);
        if (ri && (ri->GetReactantType() == FSReactionSpecies::SOLUTE_SPECIES)) solR.push_back(ri->GetIndex());
    }
}

void FSMembraneReactionMaterial::GetInternalSoluteReactants(vector<int>& solRi)
{
    solRi.clear();
    FSMaterialProperty& p = GetProperty(4);
    int N = p.Size();
    for (int i=0; i<N; ++i)
    {
        FSInternalReactantMaterial* ri = dynamic_cast<FSInternalReactantMaterial*>(p.GetMaterial(i)); assert(ri);
        if (ri && (ri->GetReactantType() == FSReactionSpecies::SOLUTE_SPECIES)) solRi.push_back(ri->GetIndex());
    }
}

void FSMembraneReactionMaterial::GetExternalSoluteReactants(vector<int>& solRe)
{
    solRe.clear();
    FSMaterialProperty& p = GetProperty(6);
    int N = p.Size();
    for (int i=0; i<N; ++i)
    {
        FSExternalReactantMaterial* ri = dynamic_cast<FSExternalReactantMaterial*>(p.GetMaterial(i)); assert(ri);
        if (ri && (ri->GetReactantType() == FSReactionSpecies::SOLUTE_SPECIES)) solRe.push_back(ri->GetIndex());
    }
}

void FSMembraneReactionMaterial::GetSBMReactants(vector<int>& sbmR)
{
    sbmR.clear();
    FSMaterialProperty& p = GetProperty(2);
    int N = p.Size();
    for (int i = 0; i<N; ++i)
    {
        FSReactantMaterial* ri = dynamic_cast<FSReactantMaterial*>(p.GetMaterial(i)); assert(ri);
        if (ri && (ri->GetReactantType() == FSReactionSpecies::SBM_SPECIES)) sbmR.push_back(ri->GetIndex());
    }
}

void FSMembraneReactionMaterial::GetSoluteProducts(vector<int>& solP)
{
    solP.clear();
    FSMaterialProperty& p = GetProperty(3);
    int N = p.Size();
    for (int i = 0; i<N; ++i)
    {
        FSProductMaterial* ri = dynamic_cast<FSProductMaterial*>(p.GetMaterial(i)); assert(ri);
        if (ri && (ri->GetProductType() == FSReactionSpecies::SOLUTE_SPECIES)) solP.push_back(ri->GetIndex());
    }
}

void FSMembraneReactionMaterial::GetInternalSoluteProducts(vector<int>& solPi)
{
    solPi.clear();
    FSMaterialProperty& p = GetProperty(5);
    int N = p.Size();
    for (int i = 0; i<N; ++i)
    {
        FSInternalProductMaterial* ri = dynamic_cast<FSInternalProductMaterial*>(p.GetMaterial(i)); assert(ri);
        if (ri && (ri->GetProductType() == FSReactionSpecies::SOLUTE_SPECIES)) solPi.push_back(ri->GetIndex());
    }
}

void FSMembraneReactionMaterial::GetExternalSoluteProducts(vector<int>& solPe)
{
    solPe.clear();
    FSMaterialProperty& p = GetProperty(7);
    int N = p.Size();
    for (int i = 0; i<N; ++i)
    {
        FSExternalProductMaterial* ri = dynamic_cast<FSExternalProductMaterial*>(p.GetMaterial(i)); assert(ri);
        if (ri && (ri->GetProductType() == FSReactionSpecies::SOLUTE_SPECIES)) solPe.push_back(ri->GetIndex());
    }
}

void FSMembraneReactionMaterial::GetSBMProducts(vector<int>& sbmP)
{
    sbmP.clear();
    FSMaterialProperty& p = GetProperty(3);
    int N = p.Size();
    for (int i = 0; i<N; ++i)
    {
        FSProductMaterial* ri = dynamic_cast<FSProductMaterial*>(p.GetMaterial(i)); assert(ri);
        if (ri && (ri->GetProductType() == FSReactionSpecies::SBM_SPECIES)) sbmP.push_back(ri->GetIndex());
    }
}

string buildMembraneReactionEquation(FSMembraneReactionMaterial* mat, FSModel& fem)
{
    stringstream ss;
    int NR = mat->Reactants();
    for (int i = 0; i<NR; ++i)
    {
        FSReactantMaterial* rm = mat->Reactant(i);
        
        string name;
        int m = rm->GetIndex();
        int ntype = rm->GetReactantType();
        if (ntype == FSReactionSpecies::SOLUTE_SPECIES)
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
        FSInternalReactantMaterial* rm = mat->InternalReactant(i);
        
        string name;
        int m = rm->GetIndex();
        int ntype = rm->GetReactantType();
        if (ntype == FSReactionSpecies::SOLUTE_SPECIES)
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
        FSExternalReactantMaterial* rm = mat->ExternalReactant(i);
        
        string name;
        int m = rm->GetIndex();
        int ntype = rm->GetReactantType();
        if (ntype == FSReactionSpecies::SOLUTE_SPECIES)
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
        FSProductMaterial* pm = mat->Product(i);
        
        string name;
        int m = pm->GetIndex();
        int ntype = pm->GetProductType();
        if (ntype == FSReactionSpecies::SOLUTE_SPECIES)
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
        FSInternalProductMaterial* pm = mat->InternalProduct(i);
        
        string name;
        int m = pm->GetIndex();
        int ntype = pm->GetProductType();
        if (ntype == FSReactionSpecies::SOLUTE_SPECIES)
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
        FSExternalProductMaterial* pm = mat->ExternalProduct(i);
        
        string name;
        int m = pm->GetIndex();
        int ntype = pm->GetProductType();
        if (ntype == FSReactionSpecies::SOLUTE_SPECIES)
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

REGISTER_MATERIAL(FSMultiphasicMaterial, MODULE_MULTIPHASIC, FE_MULTIPHASIC_MATERIAL, FE_MAT_MULTIPHASIC, "multiphasic", MaterialFlags::TOPLEVEL);

FSMultiphasicMaterial::FSMultiphasicMaterial() : FSMultiMaterial(FE_MULTIPHASIC_MATERIAL)
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
	AddProperty("solute", FE_MAT_SOLUTE, FSMaterialProperty::NO_FIXED_SIZE);

	// Add solid bound property
	AddProperty("solid_bound", FE_MAT_SBM, FSMaterialProperty::NO_FIXED_SIZE);

	// add reaction material
	AddProperty("reaction", FE_MAT_REACTION, FSMaterialProperty::NO_FIXED_SIZE, FSMaterialProperty::NON_EXTENDABLE);

    // add reaction material
    AddProperty("membrane_reaction", FE_MAT_MREACTION, FSMaterialProperty::NO_FIXED_SIZE, FSMaterialProperty::NON_EXTENDABLE);
}

// set/get elastic component 
void FSMultiphasicMaterial::SetSolidMaterial(FSMaterial* pm)
{ 
	ReplaceProperty(SOLID, pm); 
}

// set/get permeability
void FSMultiphasicMaterial::SetPermeability(FSMaterial* pm)
{ 
	ReplaceProperty(PERM, pm); 
}

// set/get osmotic coefficient
void FSMultiphasicMaterial::SetOsmoticCoefficient(FSMaterial* pm)
{ 
	ReplaceProperty(OSMC, pm); 
}

// add solute component
void FSMultiphasicMaterial::AddSoluteMaterial(FSSoluteMaterial* pm)
{
	GetProperty(SOLUTE).AddMaterial(pm);
}

// add SBM component
void FSMultiphasicMaterial::AddSBMMaterial(FSSBMMaterial* pm)
{
	GetProperty(SBM).AddMaterial(pm);
}

// add chemical reaction component
void FSMultiphasicMaterial::AddReactionMaterial(FSReactionMaterial* pm)
{
	GetProperty(REACTION).AddMaterial(pm);
}

// add membrane reaction component
void FSMultiphasicMaterial::AddMembraneReactionMaterial(FSMembraneReactionMaterial* pm)
{
    GetProperty(MREACTION).AddMaterial(pm);
}

// get solute global index from local index
int FSMultiphasicMaterial::GetSoluteIndex(const int isol) 
{
	FSMaterialProperty& p = GetProperty(SOLUTE);
	FSSoluteMaterial* mp = dynamic_cast<FSSoluteMaterial*>(p.GetMaterial(isol));
	return mp->GetSoluteIndex();
}

// get SBM global index from local index
int FSMultiphasicMaterial::GetSBMIndex(const int isbm) 
{
	FSMaterialProperty& p = GetProperty(SBM);
	FSSBMMaterial* mp = dynamic_cast<FSSBMMaterial*>(p.GetMaterial(isbm));
	return mp->GetSBMIndex();
}

// count reaction components
int FSMultiphasicMaterial::Reactions() 
{
	return GetProperty(REACTION).Size();
}

// get reaction component
FSReactionMaterial* FSMultiphasicMaterial::GetReaction(int n)
{
	FSMaterialProperty& p = GetProperty(REACTION);
	FSReactionMaterial* prm = dynamic_cast<FSReactionMaterial*>(p.GetMaterial(n));
	return prm;
}

// count membrane reaction components
int FSMultiphasicMaterial::MembraneReactions()
{
    return GetProperty(MREACTION).Size();
}

// get reaction component
FSMembraneReactionMaterial* FSMultiphasicMaterial::GetMembraneReaction(int n)
{
    FSMaterialProperty& p = GetProperty(MREACTION);
    FSMembraneReactionMaterial* prm = dynamic_cast<FSMembraneReactionMaterial*>(p.GetMaterial(n));
    return prm;
}

// see if this material has a solute with global ID
bool FSMultiphasicMaterial::HasSolute(int nid)
{
	FSMaterialProperty& p = GetProperty(SOLUTE);
	for (int i=0; i<p.Size(); ++i)
	{
		FSSoluteMaterial* mp = dynamic_cast<FSSoluteMaterial*>(p.GetMaterial(i));
		int sid = mp->GetSoluteIndex();
		if (mp && (mp->GetSoluteIndex() == nid)) return true;
	}
	return false;
}

// see if this material has a sbm with global ID
bool FSMultiphasicMaterial::HasSBM(int nid)
{
	FSMaterialProperty& p = GetProperty(SBM);
	for (int i = 0; i<p.Size(); ++i)
	{
		FSSBMMaterial* mp = dynamic_cast<FSSBMMaterial*>(p.GetMaterial(i));
		int sid = mp->GetSBMIndex();
		if (mp && (mp->GetSBMIndex() == nid)) return true;
	}
	return false;
}

//=============================================================================
FSReactionSpecies::FSReactionSpecies(int ntype) : FSMaterial(ntype)
{
    // add the stoichiometric coefficient
    AddIntParam(1, "v", "v"); // stoichiometric coefficient
    // add the type
    AddIntParam(-1, 0, 0)->SetState(Param_HIDDEN);
    // add the index
    AddIntParam(-1, 0, 0)->SetState(Param_HIDDEN);
}

void FSReactionSpecies::Load(IArchive& ar)
{
    FSMaterial::Load(ar);

    // This is a temporary hack to remove species type > 2, which should now all be solutes
    if (GetSpeciesType() > 2) SetSpeciesType(SOLUTE_SPECIES);

}

//=============================================================================
//								REACTANT
//=============================================================================

REGISTER_MATERIAL(FSReactantMaterial, MODULE_REACTIONS, FE_REACTANT_MATERIAL, FE_MAT_REACTION_REACTANTS, "Reactant", 0);

FSReactantMaterial::FSReactantMaterial() : FSReactionSpecies(FE_REACTANT_MATERIAL)
{
}

//=============================================================================
//								PRODUCT
//=============================================================================

REGISTER_MATERIAL(FSProductMaterial, MODULE_REACTIONS, FE_PRODUCT_MATERIAL, FE_MAT_REACTION_PRODUCTS, "Product", 0);

FSProductMaterial::FSProductMaterial() : FSReactionSpecies(FE_PRODUCT_MATERIAL)
{
}

//=============================================================================
//                          INTERNAL REACTANT
//=============================================================================

REGISTER_MATERIAL(FSInternalReactantMaterial, MODULE_REACTIONS, FE_INT_REACTANT_MATERIAL, FE_MAT_MREACTION_IREACTANTS, "Internal Reactant", 0);

FSInternalReactantMaterial::FSInternalReactantMaterial() : FSReactionSpecies(FE_INT_REACTANT_MATERIAL)
{
}

//=============================================================================
//                           INTERNAL PRODUCT
//=============================================================================

REGISTER_MATERIAL(FSInternalProductMaterial, MODULE_REACTIONS, FE_INT_PRODUCT_MATERIAL, FE_MAT_MREACTION_IPRODUCTS, "Internal Product", 0);

FSInternalProductMaterial::FSInternalProductMaterial() : FSReactionSpecies(FE_INT_PRODUCT_MATERIAL)
{
}

//=============================================================================
//                          EXTERNAL REACTANT
//=============================================================================

REGISTER_MATERIAL(FSExternalReactantMaterial, MODULE_REACTIONS, FE_EXT_REACTANT_MATERIAL, FE_MAT_MREACTION_EREACTANTS, "External Reactant", 0);

FSExternalReactantMaterial::FSExternalReactantMaterial() : FSReactionSpecies(FE_EXT_REACTANT_MATERIAL)
{
}

//=============================================================================
//                           EXTERNAL PRODUCT
//=============================================================================

REGISTER_MATERIAL(FSExternalProductMaterial, MODULE_REACTIONS, FE_EXT_PRODUCT_MATERIAL, FE_MAT_MREACTION_EPRODUCTS, "External Product", 0);

FSExternalProductMaterial::FSExternalProductMaterial() : FSReactionSpecies(FE_EXT_PRODUCT_MATERIAL)
{
}

//=============================================================================
//								MASS ACTION FORWARD REACTION
//=============================================================================

REGISTER_MATERIAL(FSMassActionForward, MODULE_REACTIONS, FE_MASS_ACTION_FORWARD, FE_MAT_REACTION, "mass-action-forward", 0);

FSMassActionForward::FSMassActionForward() : FSReactionMaterial(FE_MASS_ACTION_FORWARD)
{
}

//=============================================================================
//								MASS ACTION REVERSIBLE REACTION
//=============================================================================

REGISTER_MATERIAL(FSMassActionReversible, MODULE_REACTIONS, FE_MASS_ACTION_REVERSIBLE, FE_MAT_REACTION, "mass-action-reversible", 0);

FSMassActionReversible::FSMassActionReversible() : FSReactionMaterial(FE_MASS_ACTION_REVERSIBLE)
{
}

//=============================================================================
//								MICHAELIS MENTEN REACTION
//=============================================================================

REGISTER_MATERIAL(FSMichaelisMenten, MODULE_REACTIONS, FE_MICHAELIS_MENTEN, FE_MAT_REACTION, "Michaelis-Menten", 0);

FSMichaelisMenten::FSMichaelisMenten() : FSReactionMaterial(FE_MICHAELIS_MENTEN)
{
	AddScienceParam(0, UNIT_CONCENTRATION, "Km", "Km"); // concentration at half-maximum rate
	AddScienceParam(0, UNIT_CONCENTRATION, "c0", "c0"); // substrate trigger concentration
}

//=============================================================================
//                         MEMBRANE MASS ACTION FORWARD REACTION
//=============================================================================

REGISTER_MATERIAL(FSMembraneMassActionForward, MODULE_REACTIONS, FE_MMASS_ACTION_FORWARD, FE_MAT_MREACTION, "membrane-mass-action-forward", 0);

FSMembraneMassActionForward::FSMembraneMassActionForward() : FSMembraneReactionMaterial(FE_MMASS_ACTION_FORWARD)
{
}

//=============================================================================
//                         MEMBRANE MASS ACTION REVERSIBLE REACTION
//=============================================================================

REGISTER_MATERIAL(FSMembraneMassActionReversible, MODULE_REACTIONS, FE_MMASS_ACTION_REVERSIBLE, FE_MAT_MREACTION, "membrane-mass-action-reversible", 0);

FSMembraneMassActionReversible::FSMembraneMassActionReversible() : FSMembraneReactionMaterial(FE_MMASS_ACTION_REVERSIBLE)
{
}

//=============================================================================
//									FLUID
//=============================================================================
REGISTER_MATERIAL(FSFluidMaterial, MODULE_FLUID, FE_FLUID_MATERIAL, FE_MAT_FLUID, "fluid", MaterialFlags::TOPLEVEL);

FSFluidMaterial::FSFluidMaterial() : FSMaterial(FE_FLUID_MATERIAL)
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

REGISTER_MATERIAL(FSFluidFSIMaterial, MODULE_FLUID_FSI, FE_FLUID_FSI_MATERIAL, FE_MAT_FLUID_FSI, "fluid-FSI", MaterialFlags::TOPLEVEL);

FSFluidFSIMaterial::FSFluidFSIMaterial() : FSMultiMaterial(FE_FLUID_FSI_MATERIAL)
{
    // Add fluid component
    AddProperty("fluid", FE_MAT_FLUID);
    
    // Add solid component
    AddProperty("solid", FE_MAT_ELASTIC);
}

//=============================================================================
//                                  BIPHASIC-FSI
//=============================================================================

REGISTER_MATERIAL(FSBiphasicFSIMaterial, MODULE_FLUID_FSI, FE_BIPHASIC_FSI_MATERIAL, FE_MAT_FLUID_FSI, "biphasic-FSI", MaterialFlags::TOPLEVEL);

FSBiphasicFSIMaterial::FSBiphasicFSIMaterial() : FSMultiMaterial(FE_BIPHASIC_FSI_MATERIAL)
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

REGISTER_MATERIAL(FSSpeciesMaterial, MODULE_REACTIONS, FE_SPECIES_MATERIAL, FE_MAT_SPECIES, "species", 0);

FSSpeciesMaterial::FSSpeciesMaterial() : FSMaterial(FE_SPECIES_MATERIAL)
{
	// add the solute material index
	AddChoiceParam(0, "sol", "Solute")->SetEnumNames("$(Solutes)")->SetState(Param_EDITABLE | Param_PERSISTENT);

	// add the solute material index
	AddScienceParam(0, UNIT_DIFFUSIVITY, "diffusivity", "diffusivity");
}

int FSSpeciesMaterial::GetSpeciesIndex()
{
	return GetParam(0).GetIntValue();
}

void FSSpeciesMaterial::SetSpeciesIndex(int n)
{
	GetParam(0).SetIntValue(n);
}

//=============================================================================
//								SOLID SPECIES MATERIAL
//=============================================================================

REGISTER_MATERIAL(FSSolidSpeciesMaterial, MODULE_REACTIONS, FE_SOLID_SPECIES_MATERIAL, FE_MAT_SOLID_SPECIES, "solid_bound_species", 0);

FSSolidSpeciesMaterial::FSSolidSpeciesMaterial() : FSMaterial(FE_SOLID_SPECIES_MATERIAL)
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

REGISTER_MATERIAL(FSReactionDiffusionMaterial, MODULE_REACTION_DIFFUSION, FE_REACTION_DIFFUSION_MATERIAL, FE_MAT_REACTION_DIFFUSION, "reaction-diffusion", MaterialFlags::TOPLEVEL);

FSReactionDiffusionMaterial::FSReactionDiffusionMaterial() : FSMaterial(FE_REACTION_DIFFUSION_MATERIAL)
{
	// add parameters
	AddScienceParam(0.0, UNIT_NONE, "solid_volume_fraction", "solid_volume_fraction");

	// Add solute property
	AddProperty("species", FE_MAT_SPECIES, FSMaterialProperty::NO_FIXED_SIZE);

	// Add solid bound property
	AddProperty("solid_bound_species", FE_MAT_SOLID_SPECIES, FSMaterialProperty::NO_FIXED_SIZE);

	// add reaction material
	AddProperty("reaction", FE_MAT_REACTION, FSMaterialProperty::NO_FIXED_SIZE, 0);
}

void FSReactionDiffusionMaterial::AddSpeciesMaterial(FSSpeciesMaterial* pm)
{
	GetProperty(0).AddMaterial(pm);
}

void FSReactionDiffusionMaterial::AddSolidSpeciesMaterial(FSSolidSpeciesMaterial* pm)
{
	GetProperty(1).AddMaterial(pm);
}

void FSReactionDiffusionMaterial::AddReactionMaterial(FSReactionMaterial* pm)
{
	GetProperty(2).AddMaterial(pm);
}

//=============================================================================
//                                    GENERATION
//=============================================================================

REGISTER_MATERIAL(FSGeneration, MODULE_MECH, FE_GENERATION, FE_MAT_GENERATION, "generation", 0);

FSGeneration::FSGeneration() : FSMaterial(FE_GENERATION)
{
    // add parameters
    AddScienceParam(0, UNIT_TIME, "start_time", "generation start time");
    
    // Add solid component
    AddProperty("solid", FE_MAT_ELASTIC | FE_MAT_ELASTIC_UNCOUPLED);
}

//=============================================================================
//                                MULTIGENERATION
//=============================================================================

REGISTER_MATERIAL(FSMultiGeneration, MODULE_MECH, FE_MULTI_GENERATION, FE_MAT_ELASTIC, "multigeneration", MaterialFlags::TOPLEVEL);

//-----------------------------------------------------------------------------
FSMultiGeneration::FSMultiGeneration() : FSMaterial(FE_MULTI_GENERATION)
{
    AddProperty("generation", FE_MAT_GENERATION, FSMaterialProperty::NO_FIXED_SIZE);
}

//=============================================================================
//								PRESTRAIN
//=============================================================================

REGISTER_MATERIAL(FSPrestrainMaterial, MODULE_MECH, FE_PRESTRAIN_MATERIAL, FE_MAT_ELASTIC, "prestrain elastic", MaterialFlags::TOPLEVEL);

FSPrestrainMaterial::FSPrestrainMaterial() : FSMaterial(FE_PRESTRAIN_MATERIAL)
{
	AddScienceParam(1.0, UNIT_DENSITY, "density");

	// Add one component for the elastic material
	AddProperty("elastic", FE_MAT_ELASTIC | FE_MAT_ELASTIC_UNCOUPLED);
	AddProperty("prestrain", FE_MAT_PRESTRAIN_GRADIENT);
}

//=============================================================================
//								UNCOUPLED PRESTRAIN
//=============================================================================

REGISTER_MATERIAL(FSUncoupledPrestrainMaterial, MODULE_MECH, FE_UNCOUPLED_PRESTRAIN_MATERIAL, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled prestrain elastic", MaterialFlags::TOPLEVEL);

FSUncoupledPrestrainMaterial::FSUncoupledPrestrainMaterial() : FSMaterial(FE_UNCOUPLED_PRESTRAIN_MATERIAL)
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

REGISTER_MATERIAL(FSReactivePlasticity, MODULE_MECH, FE_REACTIVE_PLASTICITY, FE_MAT_ELASTIC, "reactive plasticity", MaterialFlags::TOPLEVEL);

FSReactivePlasticity::FSReactivePlasticity() : FSMaterial(FE_REACTIVE_PLASTICITY)
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

REGISTER_MATERIAL(FSReactivePlasticDamage, MODULE_MECH, FE_REACTIVE_PLASTIC_DAMAGE, FE_MAT_ELASTIC, "reactive plastic damage", MaterialFlags::TOPLEVEL);

FSReactivePlasticDamage::FSReactivePlasticDamage() : FSMaterial(FE_REACTIVE_PLASTIC_DAMAGE)
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
