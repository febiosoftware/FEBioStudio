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
#include "FSProject.h"
#include <sstream>
#include <FECore/units.h>
#include <FEBioLink/FEBioClass.h>

using namespace std;

//=============================================================================
//								VISCO-ELASTIC
//=============================================================================

REGISTER_MATERIAL(FSViscoElastic, MODULE_MECH, FE_VISCO_ELASTIC, FE_MAT_ELASTIC, "viscoelastic", MaterialFlags::TOPLEVEL);

FSViscoElastic::FSViscoElastic(FSModel* fem) : FSMaterial(FE_VISCO_ELASTIC, fem)
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

REGISTER_MATERIAL(FSUncoupledViscoElastic, MODULE_MECH, FE_UNCOUPLED_VISCO_ELASTIC, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled viscoelastic", MaterialFlags::TOPLEVEL);

FSUncoupledViscoElastic::FSUncoupledViscoElastic(FSModel* fem) : FSMaterial(FE_UNCOUPLED_VISCO_ELASTIC, fem)
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
FSMultiMaterial::FSMultiMaterial(int ntype, FSModel* fem) : FSMaterial(ntype, fem)
{

}

//=============================================================================
//									BIPHASIC
//=============================================================================

REGISTER_MATERIAL(FSBiphasic, MODULE_BIPHASIC, FE_BIPHASIC_MATERIAL, FE_MAT_MULTIPHASIC, "biphasic", MaterialFlags::TOPLEVEL);

FSBiphasic::FSBiphasic(FSModel* fem) : FSMultiMaterial(FE_BIPHASIC_MATERIAL, fem)
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

FSSoluteMaterial::FSSoluteMaterial(FSModel* fem) : FSMaterialProp(FE_SOLUTE_MATERIAL, fem)
{
	// add the solute material index
	AddChoiceParam(0, "sol", "Solute")->SetEnumNames("$(solutes)")->SetState(Param_EDITABLE | Param_PERSISTENT | Param_VISIBLE);

	// Add diffusivity property
	AddProperty("diffusivity", FE_MAT_DIFFUSIVITY);

	// Add solubility property
	AddProperty("solubility", FE_MAT_SOLUBILITY);
}

//=============================================================================
//								SOLID-BOUND MOLECULES MATERIAL
//=============================================================================

REGISTER_MATERIAL(FSSBMMaterial, MODULE_MULTIPHASIC, FE_SBM_MATERIAL, FE_MAT_SBM, "solid_bound", 0);

FSSBMMaterial::FSSBMMaterial(FSModel* fem) : FSMaterialProp(FE_SBM_MATERIAL, fem)
{
	// add the SBM material index
	AddIntParam(0, "sbm", "Solid-bound molecule")->SetEnumNames("$(sbms)")->SetState(Param_EDITABLE | Param_PERSISTENT | Param_VISIBLE);
    
	// add parameters
	AddScienceParam(0, UNIT_DENSITY, "rho0", "apparent density");
	AddScienceParam(0, UNIT_DENSITY, "rhomin", "min density");
	AddScienceParam(0, UNIT_DENSITY, "rhomax", "max density");
}

//=============================================================================
//								BIPHASIC-SOLUTE
//=============================================================================

REGISTER_MATERIAL(FSBiphasicSolute, MODULE_MULTIPHASIC, FE_BIPHASIC_SOLUTE, FE_MAT_MULTIPHASIC, "biphasic-solute", MaterialFlags::TOPLEVEL);

FSBiphasicSolute::FSBiphasicSolute(FSModel* fem) : FSMultiMaterial(FE_BIPHASIC_SOLUTE, fem)
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

FSTriphasicMaterial::FSTriphasicMaterial(FSModel* fem) : FSMultiMaterial(FE_TRIPHASIC_MATERIAL, fem)
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
	return GetMaterialProperty(3, i);
}

//=============================================================================
//								SOLID MIXTURE
//=============================================================================

REGISTER_MATERIAL(FSSolidMixture, MODULE_MECH, FE_SOLID_MIXTURE, FE_MAT_ELASTIC, "solid mixture", MaterialFlags::TOPLEVEL);

//-----------------------------------------------------------------------------
FSSolidMixture::FSSolidMixture(FSModel* fem) : FSMaterial(FE_SOLID_MIXTURE, fem)
{
    AddScienceParam(1, UNIT_DENSITY, "density", "density");
    
	AddProperty("solid", FE_MAT_ELASTIC, FSProperty::NO_FIXED_SIZE);
}

//=============================================================================
//								UNCOUPLED SOLID MIXTURE
//=============================================================================

REGISTER_MATERIAL(FSUncoupledSolidMixture, MODULE_MECH, FE_UNCOUPLED_SOLID_MIXTURE, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled solid mixture", MaterialFlags::TOPLEVEL);

FSUncoupledSolidMixture::FSUncoupledSolidMixture(FSModel* fem) : FSMaterial(FE_UNCOUPLED_SOLID_MIXTURE, fem)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density");
	AddScienceParam(0, UNIT_PRESSURE , "k", "bulk modulus" );

	AddProperty("solid", FE_MAT_ELASTIC_UNCOUPLED, FSProperty::NO_FIXED_SIZE);
}

//=============================================================================
//							CONTINUOUS FIBER DISTRIBUTION
//=============================================================================

REGISTER_MATERIAL(FSCFDMaterial, MODULE_MECH, FE_CFD_MATERIAL, FE_MAT_ELASTIC, "continuous fiber distribution", 0);

FSCFDMaterial::FSCFDMaterial(FSModel* fem) : FSMaterial(FE_CFD_MATERIAL, fem)
{
	SetAxisMaterial(new FSAxisMaterial(fem));

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

FSCFDUCMaterial::FSCFDUCMaterial(FSModel* fem) : FSMaterial(FE_CFD_MATERIAL_UC, fem)
{
	SetAxisMaterial(new FSAxisMaterial(fem));

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

FSElasticDamageMaterial::FSElasticDamageMaterial(FSModel* fem) : FSMaterial(FE_DMG_MATERIAL, fem)
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

FSElasticDamageMaterialUC::FSElasticDamageMaterialUC(FSModel* fem) : FSMaterial(FE_DMG_MATERIAL_UC, fem)
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

FSReactiveViscoelasticMaterial::FSReactiveViscoelasticMaterial(FSModel* fem) : FSMaterial(FE_RV_MATERIAL, fem)
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

REGISTER_MATERIAL(FSReactiveViscoelasticMaterialUC, MODULE_MECH, FE_RV_MATERIAL_UC, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled reactive viscoelastic", MaterialFlags::TOPLEVEL);

FSReactiveViscoelasticMaterialUC::FSReactiveViscoelasticMaterialUC(FSModel* fem) : FSMaterial(FE_RV_MATERIAL_UC, fem)
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

REGISTER_MATERIAL(FSRVDamageMaterial, MODULE_MECH, FE_RV_DAMAGE_MATERIAL, FE_MAT_ELASTIC, "reactive viscoelastic damage", MaterialFlags::TOPLEVEL);

FSRVDamageMaterial::FSRVDamageMaterial(FSModel* fem) : FSMaterial(FE_RV_DAMAGE_MATERIAL, fem)
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

FSReactionMaterial::FSReactionMaterial(int ntype, FSModel* fem) : FSMaterialProp(ntype, fem)
{
	// the optional Vbar parameter is hidden by default.
	AddScienceParam(0, UNIT_MOLAR_VOLUME, "Vbar", "Vbar")->SetState(Param_HIDDEN);
	AddBoolParam(false, "override_vbar", 0)->SetState(Param_HIDDEN);
    
	// Add reaction rate properties
	AddProperty("forward_rate", FE_MAT_REACTION_RATE);
	AddProperty("reverse_rate", FE_MAT_REACTION_RATE);

    AddProperty("vR", FE_MAT_REACTION_REACTANTS, 0);// FSProperty::NO_FIXED_SIZE, FSProperty::EDITABLE | FSProperty::NON_EXTENDABLE);
    AddProperty("vP", FE_MAT_REACTION_PRODUCTS , 0);// FSProperty::NO_FIXED_SIZE, FSProperty::EDITABLE | FSProperty::NON_EXTENDABLE);
}


void FSReactionMaterial::SetOvrd(bool bovrd) {
	SetBoolValue(MP_OVRD, bovrd);
	if (bovrd) GetParam(MP_VBAR).SetState(Param_ALLFLAGS);
	else GetParam(MP_VBAR).SetState(Param_HIDDEN);
}

bool FSReactionMaterial::GetOvrd() { return GetBoolValue(MP_OVRD); }

// set forward rate
void FSReactionMaterial::SetForwardRate(FSMaterial* pm) { ReplaceProperty(0, pm); }
FSMaterial* FSReactionMaterial::GetForwardRate() { return GetMaterialProperty(0); }

// set reverse rate
void FSReactionMaterial::SetReverseRate(FSMaterial* pm) { ReplaceProperty(1, pm); }
FSMaterial* FSReactionMaterial::GetReverseRate() { return GetMaterialProperty(1); }

int FSReactionMaterial::Reactants() { return GetProperty(2).Size(); }
int FSReactionMaterial::Products() { return GetProperty(3).Size(); }

FSReactantMaterial* FSReactionMaterial::Reactant(int i)
{
	return dynamic_cast<FSReactantMaterial*>(GetMaterialProperty(2, i));
}

FSProductMaterial* FSReactionMaterial::Product(int i)
{
	return dynamic_cast<FSProductMaterial*>(GetMaterialProperty(3, i));
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
	FSProperty& p = GetProperty(2);
	int N = p.Size();
	for (int i=0; i<N; ++i)
	{
		FSReactantMaterial* ri = dynamic_cast<FSReactantMaterial*>(p.GetComponent(i)); assert(ri);
		if (ri && (ri->GetReactantType() == FSReactionSpecies::SOLUTE_SPECIES)) solR.push_back(ri->GetIndex());
	}
}

void FSReactionMaterial::GetSBMReactants(vector<int>& sbmR)
{
	sbmR.clear();
	FSProperty& p = GetProperty(2);
	int N = p.Size();
	for (int i = 0; i<N; ++i)
	{
		FSReactantMaterial* ri = dynamic_cast<FSReactantMaterial*>(p.GetComponent(i)); assert(ri);
		if (ri && (ri->GetReactantType() == FSReactionSpecies::SBM_SPECIES)) sbmR.push_back(ri->GetIndex());
	}
}

void FSReactionMaterial::GetSoluteProducts(vector<int>& solP)
{
	solP.clear();
	FSProperty& p = GetProperty(3);
	int N = p.Size();
	for (int i = 0; i<N; ++i)
	{
		FSProductMaterial* ri = dynamic_cast<FSProductMaterial*>(p.GetComponent(i)); assert(ri);
		if (ri && (ri->GetProductType() == FSReactionSpecies::SOLUTE_SPECIES)) solP.push_back(ri->GetIndex());
	}
}

void FSReactionMaterial::GetSBMProducts(vector<int>& sbmP)
{
	sbmP.clear();
	FSProperty& p = GetProperty(3);
	int N = p.Size();
	for (int i = 0; i<N; ++i)
	{
		FSProductMaterial* ri = dynamic_cast<FSProductMaterial*>(p.GetComponent(i)); assert(ri);
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
FEBioReactionMaterial::FEBioReactionMaterial(FSMaterialProperty* reaction)
{
    m_reaction = reaction;
}

void FEBioReactionMaterial::SetReaction(FSMaterialProperty* reaction)
{
    m_reaction = reaction;
}

void FEBioReactionMaterial::SetForwardRate(FSMaterialProperty* fwdRate)
{
    m_reaction->FindProperty("forward_rate")->SetComponent(fwdRate);
}

void FEBioReactionMaterial::SetReverseRate(FSMaterialProperty* revRate)
{
    FSProperty* p = m_reaction->FindProperty("reverse_rate");
    if (p) p->SetComponent(revRate);
}

void FEBioReactionMaterial::SetName(const std::string& name)
{
    m_reaction->SetName(name);
}

int FEBioReactionMaterial::Type() const
{
    return m_reaction->Type();
}

std::string FEBioReactionMaterial::GetName() const
{
    return m_reaction->GetName();
}

bool FEBioReactionMaterial::IsReversible() const
{
    return (m_reaction->FindProperty("reverse_rate") != nullptr);
}

FSMaterialProperty* FEBioReactionMaterial::GetForwardRate()
{
    return dynamic_cast<FSMaterialProperty*>(m_reaction->FindProperty("forward_rate")->GetComponent(0));
}

FSMaterialProperty* FEBioReactionMaterial::GetReverseRate()
{
    FSProperty* revRate = m_reaction->FindProperty("reverse_rate");
    if (revRate)
        return dynamic_cast<FSMaterialProperty*>(revRate->GetComponent(0));
    else
        return nullptr;
}

bool FEBioReactionMaterial::GetOvrd() const
{
	bool b = false;
	if (m_reaction)
	{
		Param* p = m_reaction->GetParam("override_vbar"); assert(p);
		if (p) b = p->GetBoolValue();
	}
	return b;
}

void FEBioReactionMaterial::SetOvrd(bool b)
{
	if (m_reaction)
	{
		Param* p = m_reaction->GetParam("override_vbar"); assert(p);
		if (p) p->SetBoolValue(b);
	}
}

int FEBioReactionMaterial::Reactants() const
{
    return m_reaction->FindProperty("vR")->Size();
}

int FEBioReactionMaterial::Products() const
{
    return m_reaction->FindProperty("vP")->Size();
}

FSMaterialProperty* FEBioReactionMaterial::Reactant(int i)
{
    FSProperty* pp = m_reaction->FindProperty("vR");
    return dynamic_cast<FSMaterialProperty*>(pp->GetComponent(i));
}

FSMaterialProperty* FEBioReactionMaterial::Product(int i)
{
    FSProperty* pp = m_reaction->FindProperty("vP");
    return dynamic_cast<FSMaterialProperty*>(pp->GetComponent(i));
}

void FEBioReactionMaterial::GetSoluteReactants(vector<int>& solR)
{
    FSModel* fem = m_reaction->GetFSModel();
    int nsol = fem->Solutes();
    solR.clear();
    FSProperty& p = *m_reaction->FindProperty("vR");
    int N = p.Size();
    for (int i = 0; i < N; ++i)
    {
        FSMaterialProperty* ri = dynamic_cast<FSMaterialProperty*>(p.GetComponent(i)); assert(ri);
        int m = ri->GetParam("species")->GetIntValue();
        if ((m >= 0) && (m < nsol)) solR.push_back(m);
    }
}

void FEBioReactionMaterial::GetSBMReactants(vector<int>& sbmR)
{
    FSModel* fem = m_reaction->GetFSModel();
    int nsol = fem->Solutes();
    sbmR.clear();
    FSProperty& p = *m_reaction->FindProperty("vR");
    int N = p.Size();
    for (int i = 0; i < N; ++i)
    {
        FSMaterialProperty* ri = dynamic_cast<FSMaterialProperty*>(p.GetComponent(i)); assert(ri);
        int m = ri->GetParam("species")->GetIntValue();
        if ((m >= 0) && (m >= nsol)) sbmR.push_back(m - nsol);
    }
}

void FEBioReactionMaterial::GetSoluteProducts(vector<int>& solP)
{
    FSModel* fem = m_reaction->GetFSModel();
    int nsol = fem->Solutes();
    solP.clear();
    FSProperty& p = *m_reaction->FindProperty("vP");
    int N = p.Size();
    for (int i = 0; i < N; ++i)
    {
        FSMaterialProperty* ri = dynamic_cast<FSMaterialProperty*>(p.GetComponent(i)); assert(ri);
        int m = ri->GetParam("species")->GetIntValue();
        if ((m >= 0) && (m < nsol)) solP.push_back(m);
    }
}

void FEBioReactionMaterial::GetSBMProducts(vector<int>& sbmP)
{
    FSModel* fem = m_reaction->GetFSModel();
    int nsol = fem->Solutes();
    sbmP.clear();
    FSProperty& p = *m_reaction->FindProperty("vP");
    int N = p.Size();
    for (int i = 0; i < N; ++i)
    {
        FSMaterialProperty* ri = dynamic_cast<FSMaterialProperty*>(p.GetComponent(i)); assert(ri);
        int m = ri->GetParam("species")->GetIntValue();
        if ((m >= 0) && (m >= nsol)) sbmP.push_back(m - nsol);
    }
}

void FEBioReactionMaterial::ClearReactants()
{
    m_reaction->FindProperty("vR")->Clear();
}

void FEBioReactionMaterial::ClearProducts()
{
    m_reaction->FindProperty("vP")->Clear();
}

void FEBioReactionMaterial::AddReactantMaterial(FSMaterialProperty* pm)
{ 
    m_reaction->FindProperty("vR")->AddComponent(pm);
}

void FEBioReactionMaterial::AddProductMaterial(FSMaterialProperty* pm)
{ 
    m_reaction->FindProperty("vP")->AddComponent(pm);
}


//=============================================================================
FEBioMembraneReactionMaterial::FEBioMembraneReactionMaterial(FSMaterialProperty* reaction)
{
    m_reaction = reaction;
}

void FEBioMembraneReactionMaterial::SetReaction(FSMaterialProperty* reaction)
{
    m_reaction = reaction;
}

void FEBioMembraneReactionMaterial::SetForwardRate(FSMaterialProperty* fwdRate)
{
    m_reaction->FindProperty("forward_rate")->SetComponent(fwdRate);
}

void FEBioMembraneReactionMaterial::SetReverseRate(FSMaterialProperty* revRate)
{
    FSProperty* p = m_reaction->FindProperty("reverse_rate");
    if (p) p->SetComponent(revRate);
}

void FEBioMembraneReactionMaterial::SetName(const std::string& name)
{
    m_reaction->SetName(name);
}

int FEBioMembraneReactionMaterial::Type() const
{
    return m_reaction->Type();
}

std::string FEBioMembraneReactionMaterial::GetName() const
{
    return m_reaction->GetName();
}

bool FEBioMembraneReactionMaterial::IsReversible() const
{
    return (m_reaction->FindProperty("reverse_rate") != nullptr);
}

FSMaterialProperty* FEBioMembraneReactionMaterial::GetForwardRate()
{
    return dynamic_cast<FSMaterialProperty*>(m_reaction->FindProperty("forward_rate")->GetComponent(0));
}

FSMaterialProperty* FEBioMembraneReactionMaterial::GetReverseRate()
{
    FSProperty* revRate = m_reaction->FindProperty("reverse_rate");
    if (revRate)
        return dynamic_cast<FSMaterialProperty*>(revRate->GetComponent(0));
    else
        return nullptr;
}

bool FEBioMembraneReactionMaterial::GetOvrd() const
{
    // TODO:
    return false;
}

void FEBioMembraneReactionMaterial::SetOvrd(bool b)
{
    // TODO:
    assert(false);
}

int FEBioMembraneReactionMaterial::Reactants() const
{
    return m_reaction->FindProperty("vR")->Size();
}

int FEBioMembraneReactionMaterial::InternalReactants() const
{
    return m_reaction->FindProperty("vRi")->Size();
}

int FEBioMembraneReactionMaterial::ExternalReactants() const
{
    return m_reaction->FindProperty("vRe")->Size();
}

int FEBioMembraneReactionMaterial::Products() const
{
    return m_reaction->FindProperty("vP")->Size();
}

int FEBioMembraneReactionMaterial::InternalProducts() const
{
    return m_reaction->FindProperty("vPi")->Size();
}

int FEBioMembraneReactionMaterial::ExternalProducts() const
{
    return m_reaction->FindProperty("vPe")->Size();
}

FSMaterialProperty* FEBioMembraneReactionMaterial::Reactant(int i)
{
    FSProperty* pp = m_reaction->FindProperty("vR");
    return dynamic_cast<FSMaterialProperty*>(pp->GetComponent(i));
}

FSMaterialProperty* FEBioMembraneReactionMaterial::InternalReactant(int i)
{
    FSProperty* pp = m_reaction->FindProperty("vRe");
    return dynamic_cast<FSMaterialProperty*>(pp->GetComponent(i));
}

FSMaterialProperty* FEBioMembraneReactionMaterial::ExternalReactant(int i)
{
    FSProperty* pp = m_reaction->FindProperty("vRe");
    return dynamic_cast<FSMaterialProperty*>(pp->GetComponent(i));
}

FSMaterialProperty* FEBioMembraneReactionMaterial::Product(int i)
{
    FSProperty* pp = m_reaction->FindProperty("vP");
    return dynamic_cast<FSMaterialProperty*>(pp->GetComponent(i));
}

FSMaterialProperty* FEBioMembraneReactionMaterial::InternalProduct(int i)
{
    FSProperty* pp = m_reaction->FindProperty("vPi");
    return dynamic_cast<FSMaterialProperty*>(pp->GetComponent(i));
}

FSMaterialProperty* FEBioMembraneReactionMaterial::ExternalProduct(int i)
{
    FSProperty* pp = m_reaction->FindProperty("vPe");
    return dynamic_cast<FSMaterialProperty*>(pp->GetComponent(i));
}

void FEBioMembraneReactionMaterial::GetSoluteReactants(vector<int>& solR)
{
    FSModel* fem = m_reaction->GetFSModel();
    int nsol = fem->Solutes();
    solR.clear();
    FSProperty& p = *m_reaction->FindProperty("vR");
    int N = p.Size();
    for (int i = 0; i < N; ++i)
    {
        FSMaterialProperty* ri = dynamic_cast<FSMaterialProperty*>(p.GetComponent(i)); assert(ri);
        int m = ri->GetParam("species")->GetIntValue();
        if ((m >= 0) && (m < nsol)) solR.push_back(m);
    }
}

void FEBioMembraneReactionMaterial::GetInternalSoluteReactants(vector<int>& solRi)
{
    FSModel* fem = m_reaction->GetFSModel();
    int nsol = fem->Solutes();
    solRi.clear();
    FSProperty& p = *m_reaction->FindProperty("vRi");
    int N = p.Size();
    for (int i = 0; i < N; ++i)
    {
        FSMaterialProperty* ri = dynamic_cast<FSMaterialProperty*>(p.GetComponent(i)); assert(ri);
        int m = ri->GetParam("species")->GetIntValue();
        if ((m >= 0) && (m < nsol)) solRi.push_back(m);
    }
}

void FEBioMembraneReactionMaterial::GetExternalSoluteReactants(vector<int>& solRe)
{
    FSModel* fem = m_reaction->GetFSModel();
    int nsol = fem->Solutes();
    solRe.clear();
    FSProperty& p = *m_reaction->FindProperty("vRe");
    int N = p.Size();
    for (int i = 0; i < N; ++i)
    {
        FSMaterialProperty* ri = dynamic_cast<FSMaterialProperty*>(p.GetComponent(i)); assert(ri);
        int m = ri->GetParam("species")->GetIntValue();
        if ((m >= 0) && (m < nsol)) solRe.push_back(m);
    }
}

void FEBioMembraneReactionMaterial::GetSBMReactants(vector<int>& sbmR)
{
    FSModel* fem = m_reaction->GetFSModel();
    int nsol = fem->Solutes();
    sbmR.clear();
    FSProperty& p = *m_reaction->FindProperty("vR");
    int N = p.Size();
    for (int i = 0; i < N; ++i)
    {
        FSMaterialProperty* ri = dynamic_cast<FSMaterialProperty*>(p.GetComponent(i)); assert(ri);
        int m = ri->GetParam("species")->GetIntValue();
        if ((m >= 0) && (m >= nsol)) sbmR.push_back(m - nsol);
    }
}

void FEBioMembraneReactionMaterial::GetSoluteProducts(vector<int>& solP)
{
    FSModel* fem = m_reaction->GetFSModel();
    int nsol = fem->Solutes();
    solP.clear();
    FSProperty& p = *m_reaction->FindProperty("vP");
    int N = p.Size();
    for (int i = 0; i < N; ++i)
    {
        FSMaterialProperty* ri = dynamic_cast<FSMaterialProperty*>(p.GetComponent(i)); assert(ri);
        int m = ri->GetParam("species")->GetIntValue();
        if ((m >= 0) && (m < nsol)) solP.push_back(m);
    }
}

void FEBioMembraneReactionMaterial::GetInternalSoluteProducts(vector<int>& solPi)
{
    FSModel* fem = m_reaction->GetFSModel();
    int nsol = fem->Solutes();
    solPi.clear();
    FSProperty& p = *m_reaction->FindProperty("vPi");
    int N = p.Size();
    for (int i = 0; i < N; ++i)
    {
        FSMaterialProperty* ri = dynamic_cast<FSMaterialProperty*>(p.GetComponent(i)); assert(ri);
        int m = ri->GetParam("species")->GetIntValue();
        if ((m >= 0) && (m < nsol)) solPi.push_back(m);
    }
}

void FEBioMembraneReactionMaterial::GetExternalSoluteProducts(vector<int>& solPe)
{
    FSModel* fem = m_reaction->GetFSModel();
    int nsol = fem->Solutes();
    solPe.clear();
    FSProperty& p = *m_reaction->FindProperty("vP");
    int N = p.Size();
    for (int i = 0; i < N; ++i)
    {
        FSMaterialProperty* ri = dynamic_cast<FSMaterialProperty*>(p.GetComponent(i)); assert(ri);
        int m = ri->GetParam("species")->GetIntValue();
        if ((m >= 0) && (m < nsol)) solPe.push_back(m);
    }
}

void FEBioMembraneReactionMaterial::GetSBMProducts(vector<int>& sbmP)
{
    FSModel* fem = m_reaction->GetFSModel();
    int nsol = fem->Solutes();
    sbmP.clear();
    FSProperty& p = *m_reaction->FindProperty("vP");
    int N = p.Size();
    for (int i = 0; i < N; ++i)
    {
        FSMaterialProperty* ri = dynamic_cast<FSMaterialProperty*>(p.GetComponent(i)); assert(ri);
        int m = ri->GetParam("species")->GetIntValue();
        if ((m >= 0) && (m >= nsol)) sbmP.push_back(m - nsol);
    }
}

void FEBioMembraneReactionMaterial::ClearReactants()
{
    m_reaction->FindProperty("vR")->Clear();
}

void FEBioMembraneReactionMaterial::ClearProducts()
{
    m_reaction->FindProperty("vP")->Clear();
}

void FEBioMembraneReactionMaterial::AddReactantMaterial(FSMaterialProperty* pm)
{
    m_reaction->FindProperty("vR")->AddComponent(pm);
}

void FEBioMembraneReactionMaterial::AddProductMaterial(FSMaterialProperty* pm)
{
    m_reaction->FindProperty("vP")->AddComponent(pm);
}

//=============================================================================
FEBioMultiphasic::FEBioMultiphasic(FSMaterial* mat)
{
    assert(strcmp(mat->GetTypeString(), "multiphasic") == 0);
    m_mat = mat;
}

// see if this material has a solute with global ID
bool FEBioMultiphasic::HasSolute(int nid) const
{
    const FSProperty& p = *m_mat->FindProperty("solute");
    for (int i = 0; i < p.Size(); ++i)
    {
        const FSMaterialProperty* mp = dynamic_cast<const FSMaterialProperty*>(p.GetComponent(i));
        int sid = mp->GetParam("sol")->GetIntValue();
        if (mp && (sid == nid)) return true;
    }
    return false;
}

// see if this material has a solute with global ID
bool FEBioMultiphasic::HasSBM(int nid) const
{
    const FSProperty& p = *m_mat->FindProperty("solid_bound");
    for (int i = 0; i < p.Size(); ++i)
    {
        const FSMaterialProperty* mp = dynamic_cast<const FSMaterialProperty*>(p.GetComponent(i));
        int sid = mp->GetParam("sbm")->GetIntValue();
        if (mp && (sid == nid)) return true;
    }
    return false;
}

//=============================================================================
//                            MEMBRANE REACTION
//=============================================================================

FSMembraneReactionMaterial::FSMembraneReactionMaterial(int ntype, FSModel* fem) : FSMaterial(ntype, fem)
{
    // the optional Vbar parameter is hidden by default.
    AddScienceParam(0, UNIT_MOLAR_VOLUME, "Vbar", "Vbar")->SetState(Param_HIDDEN);
    AddBoolParam(false, 0, 0)->SetState(Param_HIDDEN);
    
    // Add reaction rate properties
    AddProperty("forward_rate", FE_MAT_MREACTION_RATE);
    AddProperty("reverse_rate", FE_MAT_MREACTION_RATE);
    
    AddProperty("vR" , FE_MAT_REACTION_REACTANTS  , FSProperty::NO_FIXED_SIZE, 0);//FSProperty::EDITABLE | FSProperty::NON_EXTENDABLE);
    AddProperty("vP" , FE_MAT_REACTION_PRODUCTS   , FSProperty::NO_FIXED_SIZE, 0);//FSProperty::EDITABLE | FSProperty::NON_EXTENDABLE);
    AddProperty("vRi", FE_MAT_MREACTION_IREACTANTS, FSProperty::NO_FIXED_SIZE, 0);//FSProperty::EDITABLE | FSProperty::NON_EXTENDABLE);
    AddProperty("vPi", FE_MAT_MREACTION_IPRODUCTS , FSProperty::NO_FIXED_SIZE, 0);//FSProperty::EDITABLE | FSProperty::NON_EXTENDABLE);
    AddProperty("vRe", FE_MAT_MREACTION_EREACTANTS, FSProperty::NO_FIXED_SIZE, 0);//FSProperty::EDITABLE | FSProperty::NON_EXTENDABLE);
    AddProperty("vPe", FE_MAT_MREACTION_EPRODUCTS , FSProperty::NO_FIXED_SIZE, 0);//FSProperty::EDITABLE | FSProperty::NON_EXTENDABLE);
}


void FSMembraneReactionMaterial::SetOvrd(bool bovrd) {
    SetBoolValue(MP_OVRD, bovrd);
    if (bovrd) GetParam(MP_VBAR).SetState(Param_ALLFLAGS);
    else GetParam(MP_VBAR).SetState(Param_HIDDEN);
}

bool FSMembraneReactionMaterial::GetOvrd() { return GetBoolValue(MP_OVRD); }

// set forward rate
void FSMembraneReactionMaterial::SetForwardRate(FSMaterial* pm) { ReplaceProperty(0, pm); }
FSMaterial* FSMembraneReactionMaterial::GetForwardRate() { return GetMaterialProperty(0); }

// set reverse rate
void FSMembraneReactionMaterial::SetReverseRate(FSMaterial* pm) { ReplaceProperty(1, pm); }
FSMaterial* FSMembraneReactionMaterial::GetReverseRate() { return GetMaterialProperty(1); }

int FSMembraneReactionMaterial::Reactants() { return GetProperty(2).Size(); }
int FSMembraneReactionMaterial::Products() { return GetProperty(3).Size(); }
int FSMembraneReactionMaterial::InternalReactants() { return GetProperty(4).Size(); }
int FSMembraneReactionMaterial::InternalProducts() { return GetProperty(5).Size(); }
int FSMembraneReactionMaterial::ExternalReactants() { return GetProperty(6).Size(); }
int FSMembraneReactionMaterial::ExternalProducts() { return GetProperty(7).Size(); }

FSReactantMaterial* FSMembraneReactionMaterial::Reactant(int i)
{
    return dynamic_cast<FSReactantMaterial*>(GetMaterialProperty(2, i));
}

FSProductMaterial* FSMembraneReactionMaterial::Product(int i)
{
    return dynamic_cast<FSProductMaterial*>(GetMaterialProperty(3, i));
}

FSInternalReactantMaterial* FSMembraneReactionMaterial::InternalReactant(int i)
{
    return dynamic_cast<FSInternalReactantMaterial*>(GetMaterialProperty(4, i));
}

FSInternalProductMaterial* FSMembraneReactionMaterial::InternalProduct(int i)
{
    return dynamic_cast<FSInternalProductMaterial*>(GetMaterialProperty(5, i));
}

FSExternalReactantMaterial* FSMembraneReactionMaterial::ExternalReactant(int i)
{
    return dynamic_cast<FSExternalReactantMaterial*>(GetMaterialProperty(6, i));
}

FSExternalProductMaterial* FSMembraneReactionMaterial::ExternalProduct(int i)
{
    return dynamic_cast<FSExternalProductMaterial*>(GetMaterialProperty(7, i));
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
    FSProperty& p = GetProperty(2);
    int N = p.Size();
    for (int i=0; i<N; ++i)
    {
        FSReactantMaterial* ri = dynamic_cast<FSReactantMaterial*>(p.GetComponent(i)); assert(ri);
        if (ri && (ri->GetReactantType() == FSReactionSpecies::SOLUTE_SPECIES)) solR.push_back(ri->GetIndex());
    }
}

void FSMembraneReactionMaterial::GetInternalSoluteReactants(vector<int>& solRi)
{
    solRi.clear();
    FSProperty& p = GetProperty(4);
    int N = p.Size();
    for (int i=0; i<N; ++i)
    {
        FSInternalReactantMaterial* ri = dynamic_cast<FSInternalReactantMaterial*>(p.GetComponent(i)); assert(ri);
        if (ri && (ri->GetReactantType() == FSReactionSpecies::SOLUTE_SPECIES)) solRi.push_back(ri->GetIndex());
    }
}

void FSMembraneReactionMaterial::GetExternalSoluteReactants(vector<int>& solRe)
{
    solRe.clear();
    FSProperty& p = GetProperty(6);
    int N = p.Size();
    for (int i=0; i<N; ++i)
    {
        FSExternalReactantMaterial* ri = dynamic_cast<FSExternalReactantMaterial*>(p.GetComponent(i)); assert(ri);
        if (ri && (ri->GetReactantType() == FSReactionSpecies::SOLUTE_SPECIES)) solRe.push_back(ri->GetIndex());
    }
}

void FSMembraneReactionMaterial::GetSBMReactants(vector<int>& sbmR)
{
    sbmR.clear();
    FSProperty& p = GetProperty(2);
    int N = p.Size();
    for (int i = 0; i<N; ++i)
    {
        FSReactantMaterial* ri = dynamic_cast<FSReactantMaterial*>(p.GetComponent(i)); assert(ri);
        if (ri && (ri->GetReactantType() == FSReactionSpecies::SBM_SPECIES)) sbmR.push_back(ri->GetIndex());
    }
}

void FSMembraneReactionMaterial::GetSoluteProducts(vector<int>& solP)
{
    solP.clear();
    FSProperty& p = GetProperty(3);
    int N = p.Size();
    for (int i = 0; i<N; ++i)
    {
        FSProductMaterial* ri = dynamic_cast<FSProductMaterial*>(p.GetComponent(i)); assert(ri);
        if (ri && (ri->GetProductType() == FSReactionSpecies::SOLUTE_SPECIES)) solP.push_back(ri->GetIndex());
    }
}

void FSMembraneReactionMaterial::GetInternalSoluteProducts(vector<int>& solPi)
{
    solPi.clear();
    FSProperty& p = GetProperty(5);
    int N = p.Size();
    for (int i = 0; i<N; ++i)
    {
        FSInternalProductMaterial* ri = dynamic_cast<FSInternalProductMaterial*>(p.GetComponent(i)); assert(ri);
        if (ri && (ri->GetProductType() == FSReactionSpecies::SOLUTE_SPECIES)) solPi.push_back(ri->GetIndex());
    }
}

void FSMembraneReactionMaterial::GetExternalSoluteProducts(vector<int>& solPe)
{
    solPe.clear();
    FSProperty& p = GetProperty(7);
    int N = p.Size();
    for (int i = 0; i<N; ++i)
    {
        FSExternalProductMaterial* ri = dynamic_cast<FSExternalProductMaterial*>(p.GetComponent(i)); assert(ri);
        if (ri && (ri->GetProductType() == FSReactionSpecies::SOLUTE_SPECIES)) solPe.push_back(ri->GetIndex());
    }
}

void FSMembraneReactionMaterial::GetSBMProducts(vector<int>& sbmP)
{
    sbmP.clear();
    FSProperty& p = GetProperty(3);
    int N = p.Size();
    for (int i = 0; i<N; ++i)
    {
        FSProductMaterial* ri = dynamic_cast<FSProductMaterial*>(p.GetComponent(i)); assert(ri);
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

FSMultiphasicMaterial::FSMultiphasicMaterial(FSModel* fem) : FSMultiMaterial(FE_MULTIPHASIC_MATERIAL, fem)
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
	AddProperty("solute", FE_MAT_SOLUTE, FSProperty::NO_FIXED_SIZE);

	// Add solid bound property
	AddProperty("solid_bound", FE_MAT_SBM, FSProperty::NO_FIXED_SIZE);

	// add reaction material
    AddProperty("reaction", FE_MAT_REACTION, FSProperty::NO_FIXED_SIZE, 0);// FSProperty::NON_EXTENDABLE);

    // add reaction material
    AddProperty("membrane_reaction", FE_MAT_MREACTION, FSProperty::NO_FIXED_SIZE, 0);// FSProperty::NON_EXTENDABLE);
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
	GetProperty(SOLUTE).AddComponent(pm);
}

// add SBM component
void FSMultiphasicMaterial::AddSBMMaterial(FSSBMMaterial* pm)
{
	GetProperty(SBM).AddComponent(pm);
}

// add chemical reaction component
void FSMultiphasicMaterial::AddReactionMaterial(FSReactionMaterial* pm)
{
	GetProperty(REACTION).AddComponent(pm);
}

// add membrane reaction component
void FSMultiphasicMaterial::AddMembraneReactionMaterial(FSMembraneReactionMaterial* pm)
{
    GetProperty(MREACTION).AddComponent(pm);
}

// get solute global index from local index
int FSMultiphasicMaterial::GetSoluteIndex(const int isol) 
{
	FSProperty& p = GetProperty(SOLUTE);
	FSSoluteMaterial* mp = dynamic_cast<FSSoluteMaterial*>(p.GetComponent(isol));
	return mp->GetSoluteIndex();
}

// get SBM global index from local index
int FSMultiphasicMaterial::GetSBMIndex(const int isbm) 
{
	FSProperty& p = GetProperty(SBM);
	FSSBMMaterial* mp = dynamic_cast<FSSBMMaterial*>(p.GetComponent(isbm));
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
	FSProperty& p = GetProperty(REACTION);
	FSReactionMaterial* prm = dynamic_cast<FSReactionMaterial*>(p.GetComponent(n));
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
    FSProperty& p = GetProperty(MREACTION);
    FSMembraneReactionMaterial* prm = dynamic_cast<FSMembraneReactionMaterial*>(p.GetComponent(n));
    return prm;
}

// see if this material has a solute with global ID
bool FSMultiphasicMaterial::HasSolute(int nid)
{
	FSProperty& p = GetProperty(SOLUTE);
	for (int i=0; i<p.Size(); ++i)
	{
		FSSoluteMaterial* mp = dynamic_cast<FSSoluteMaterial*>(p.GetComponent(i));
		int sid = mp->GetSoluteIndex();
		if (mp && (mp->GetSoluteIndex() == nid)) return true;
	}
	return false;
}

// see if this material has a sbm with global ID
bool FSMultiphasicMaterial::HasSBM(int nid)
{
	FSProperty& p = GetProperty(SBM);
	for (int i = 0; i<p.Size(); ++i)
	{
		FSSBMMaterial* mp = dynamic_cast<FSSBMMaterial*>(p.GetComponent(i));
		int sid = mp->GetSBMIndex();
		if (mp && (mp->GetSBMIndex() == nid)) return true;
	}
	return false;
}

//=============================================================================
FSReactionSpecies::FSReactionSpecies(int ntype, FSModel* fem) : FSMaterialProp(ntype, fem)
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

REGISTER_MATERIAL(FSReactantMaterial, MODULE_REACTIONS, FE_REACTANT_MATERIAL, FE_MAT_REACTION_REACTANTS, "vR", 0);

FSReactantMaterial::FSReactantMaterial(FSModel* fem) : FSReactionSpecies(FE_REACTANT_MATERIAL, fem)
{
}

//=============================================================================
//								PRODUCT
//=============================================================================

REGISTER_MATERIAL(FSProductMaterial, MODULE_REACTIONS, FE_PRODUCT_MATERIAL, FE_MAT_REACTION_PRODUCTS, "vP", 0);

FSProductMaterial::FSProductMaterial(FSModel* fem) : FSReactionSpecies(FE_PRODUCT_MATERIAL, fem)
{
}

//=============================================================================
//                          INTERNAL REACTANT
//=============================================================================

REGISTER_MATERIAL(FSInternalReactantMaterial, MODULE_REACTIONS, FE_INT_REACTANT_MATERIAL, FE_MAT_MREACTION_IREACTANTS, "vRi", 0);

FSInternalReactantMaterial::FSInternalReactantMaterial(FSModel* fem) : FSReactionSpecies(FE_INT_REACTANT_MATERIAL, fem)
{
}

//=============================================================================
//                           INTERNAL PRODUCT
//=============================================================================

REGISTER_MATERIAL(FSInternalProductMaterial, MODULE_REACTIONS, FE_INT_PRODUCT_MATERIAL, FE_MAT_MREACTION_IPRODUCTS, "vPi", 0);

FSInternalProductMaterial::FSInternalProductMaterial(FSModel* fem) : FSReactionSpecies(FE_INT_PRODUCT_MATERIAL, fem)
{
}

//=============================================================================
//                          EXTERNAL REACTANT
//=============================================================================

REGISTER_MATERIAL(FSExternalReactantMaterial, MODULE_REACTIONS, FE_EXT_REACTANT_MATERIAL, FE_MAT_MREACTION_EREACTANTS, "vRe", 0);

FSExternalReactantMaterial::FSExternalReactantMaterial(FSModel* fem) : FSReactionSpecies(FE_EXT_REACTANT_MATERIAL, fem)
{
}

//=============================================================================
//                           EXTERNAL PRODUCT
//=============================================================================

REGISTER_MATERIAL(FSExternalProductMaterial, MODULE_REACTIONS, FE_EXT_PRODUCT_MATERIAL, FE_MAT_MREACTION_EPRODUCTS, "vPe", 0);

FSExternalProductMaterial::FSExternalProductMaterial(FSModel* fem) : FSReactionSpecies(FE_EXT_PRODUCT_MATERIAL, fem)
{
}

//=============================================================================
//								MASS ACTION REACTION
//=============================================================================

REGISTER_MATERIAL(FSMassActionReaction, MODULE_REACTIONS, FE_MASS_ACTION_REACTION, FE_MAT_REACTION, "mass action", 0);

FSMassActionReaction::FSMassActionReaction(FSModel* fem) : FSReactionMaterial(FE_MASS_ACTION_REACTION, fem)
{
	AddStringParam("", "equation");
	AddDoubleParam(0, "rate_constant");
}

//=============================================================================
//								MASS ACTION FORWARD REACTION
//=============================================================================

REGISTER_MATERIAL(FSMassActionForward, MODULE_REACTIONS, FE_MASS_ACTION_FORWARD, FE_MAT_REACTION, "mass-action-forward", 0);

FSMassActionForward::FSMassActionForward(FSModel* fem) : FSReactionMaterial(FE_MASS_ACTION_FORWARD, fem)
{
}

//=============================================================================
//								MASS ACTION REVERSIBLE REACTION
//=============================================================================

REGISTER_MATERIAL(FSMassActionReversible, MODULE_REACTIONS, FE_MASS_ACTION_REVERSIBLE, FE_MAT_REACTION, "mass-action-reversible", 0);

FSMassActionReversible::FSMassActionReversible(FSModel* fem) : FSReactionMaterial(FE_MASS_ACTION_REVERSIBLE, fem)
{
}

//=============================================================================
//								MICHAELIS MENTEN REACTION
//=============================================================================

REGISTER_MATERIAL(FSMichaelisMenten, MODULE_REACTIONS, FE_MICHAELIS_MENTEN, FE_MAT_REACTION, "Michaelis-Menten", 0);

FSMichaelisMenten::FSMichaelisMenten(FSModel* fem) : FSReactionMaterial(FE_MICHAELIS_MENTEN, fem)
{
	AddScienceParam(0, UNIT_CONCENTRATION, "Km", "Km"); // concentration at half-maximum rate
	AddScienceParam(0, UNIT_CONCENTRATION, "c0", "c0"); // substrate trigger concentration
}

//=============================================================================
//                         MEMBRANE MASS ACTION FORWARD REACTION
//=============================================================================

REGISTER_MATERIAL(FSMembraneMassActionForward, MODULE_REACTIONS, FE_MMASS_ACTION_FORWARD, FE_MAT_MREACTION, "membrane-mass-action-forward", 0);

FSMembraneMassActionForward::FSMembraneMassActionForward(FSModel* fem) : FSMembraneReactionMaterial(FE_MMASS_ACTION_FORWARD, fem)
{
}

//=============================================================================
//                         MEMBRANE MASS ACTION REVERSIBLE REACTION
//=============================================================================

REGISTER_MATERIAL(FSMembraneMassActionReversible, MODULE_REACTIONS, FE_MMASS_ACTION_REVERSIBLE, FE_MAT_MREACTION, "membrane-mass-action-reversible", 0);

FSMembraneMassActionReversible::FSMembraneMassActionReversible(FSModel* fem) : FSMembraneReactionMaterial(FE_MMASS_ACTION_REVERSIBLE, fem)
{
}

//=============================================================================
//									FLUID
//=============================================================================
REGISTER_MATERIAL(FSFluidMaterial, MODULE_FLUID, FE_FLUID_MATERIAL, FE_MAT_FLUID, "fluid", MaterialFlags::TOPLEVEL);

FSFluidMaterial::FSFluidMaterial(FSModel* fem) : FSMaterial(FE_FLUID_MATERIAL, fem)
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

FSFluidFSIMaterial::FSFluidFSIMaterial(FSModel* fem) : FSMultiMaterial(FE_FLUID_FSI_MATERIAL, fem)
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

FSBiphasicFSIMaterial::FSBiphasicFSIMaterial(FSModel* fem) : FSMultiMaterial(FE_BIPHASIC_FSI_MATERIAL, fem)
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
//                                   POLAR FLUID
//=============================================================================

REGISTER_MATERIAL(FSPolarFluidMaterial, MODULE_POLAR_FLUID, FE_POLAR_FLUID_MATERIAL, FE_MAT_POLAR_FLUID, "polar fluid", MaterialFlags::TOPLEVEL);

FSPolarFluidMaterial::FSPolarFluidMaterial(FSModel* fem) : FSMaterial(FE_POLAR_FLUID_MATERIAL, fem)
{
    // add parameters
    AddScienceParam(1.0, UNIT_DENSITY, "density", "density");
    
    // add parameters
    AddScienceParam(1.0, UNIT_PRESSURE, "k", "bulk modulus");
    
    // add parameters
    AddScienceParam(0, UNIT_LENGTH, "kg", "radius of gyration");
    
    // Add viscous component
    AddProperty("viscous", FE_MAT_FLUID_VISCOSITY);
    
    // Add fluid component
    AddProperty("polar", FE_MAT_POLAR_FLUID_VISCOSITY);
}

//=============================================================================
//								SPECIES MATERIAL
//=============================================================================

REGISTER_MATERIAL(FSSpeciesMaterial, MODULE_REACTIONS, FE_SPECIES_MATERIAL, FE_MAT_SPECIES, "species", 0);

FSSpeciesMaterial::FSSpeciesMaterial(FSModel* fem) : FSMaterial(FE_SPECIES_MATERIAL, fem)
{
	// add the solute material index
	AddChoiceParam(0, "sol", "Solute")->SetEnumNames("$(solutes)")->SetState(Param_EDITABLE | Param_PERSISTENT);

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

FSSolidSpeciesMaterial::FSSolidSpeciesMaterial(FSModel* fem) : FSMaterial(FE_SOLID_SPECIES_MATERIAL, fem)
{
	// add the SBM material index
	AddIntParam(0, "sbm", "Solid-bound molecule")->SetEnumNames("$(sbms)")->SetState(Param_EDITABLE | Param_PERSISTENT);

	// add parameters
	AddScienceParam(0, UNIT_DENSITY, "rho0", "apparent density");
	AddScienceParam(0, UNIT_DENSITY, "rhomin", "min density");
	AddScienceParam(0, UNIT_DENSITY, "rhomax", "max density");
}

//=============================================================================
//									REACTION DIFFUSION
//=============================================================================

REGISTER_MATERIAL(FSReactionDiffusionMaterial, MODULE_REACTION_DIFFUSION, FE_REACTION_DIFFUSION_MATERIAL, FE_MAT_REACTION_DIFFUSION, "reaction-diffusion", MaterialFlags::TOPLEVEL);

FSReactionDiffusionMaterial::FSReactionDiffusionMaterial(FSModel* fem) : FSMaterial(FE_REACTION_DIFFUSION_MATERIAL, fem)
{
	// add parameters
	AddScienceParam(0.0, UNIT_NONE, "solid_volume_fraction", "solid_volume_fraction");

	// Add solute property
	AddProperty("species", FE_MAT_SPECIES, FSProperty::NO_FIXED_SIZE);

	// Add solid bound property
	AddProperty("solid_bound_species", FE_MAT_SOLID_SPECIES, FSProperty::NO_FIXED_SIZE);

	// add reaction material
	AddProperty("reaction", FE_MAT_REACTION, FSProperty::NO_FIXED_SIZE, 0);
}

void FSReactionDiffusionMaterial::AddSpeciesMaterial(FSSpeciesMaterial* pm)
{
	GetProperty(0).AddComponent(pm);
}

void FSReactionDiffusionMaterial::AddSolidSpeciesMaterial(FSSolidSpeciesMaterial* pm)
{
	GetProperty(1).AddComponent(pm);
}

void FSReactionDiffusionMaterial::AddReactionMaterial(FSReactionMaterial* pm)
{
	GetProperty(2).AddComponent(pm);
}

//=============================================================================
//                                    GENERATION
//=============================================================================

REGISTER_MATERIAL(FSGeneration, MODULE_MECH, FE_GENERATION, FE_MAT_GENERATION, "generation", 0);

FSGeneration::FSGeneration(FSModel* fem) : FSMaterialProp(FE_GENERATION, fem)
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
FSMultiGeneration::FSMultiGeneration(FSModel* fem) : FSMaterial(FE_MULTI_GENERATION, fem)
{
    AddProperty("generation", FE_MAT_GENERATION, FSProperty::NO_FIXED_SIZE);
}

//=============================================================================
//								PRESTRAIN
//=============================================================================

REGISTER_MATERIAL(FSPrestrainMaterial, MODULE_MECH, FE_PRESTRAIN_MATERIAL, FE_MAT_ELASTIC, "prestrain elastic", MaterialFlags::TOPLEVEL);

FSPrestrainMaterial::FSPrestrainMaterial(FSModel* fem) : FSMaterial(FE_PRESTRAIN_MATERIAL, fem)
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

FSUncoupledPrestrainMaterial::FSUncoupledPrestrainMaterial(FSModel* fem) : FSMaterial(FE_UNCOUPLED_PRESTRAIN_MATERIAL, fem)
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

FSReactivePlasticity::FSReactivePlasticity(FSModel* fem) : FSMaterial(FE_REACTIVE_PLASTICITY, fem)
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

FSReactivePlasticDamage::FSReactivePlasticDamage(FSModel* fem) : FSMaterial(FE_REACTIVE_PLASTIC_DAMAGE, fem)
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

REGISTER_MATERIAL(FSReactiveFatigue, MODULE_MECH, FE_REACTIVE_FATIGUE, FE_MAT_ELASTIC, "reactive fatigue", MaterialFlags::TOPLEVEL);

FSReactiveFatigue::FSReactiveFatigue(FSModel* fem) : FSMaterial(FE_REACTIVE_FATIGUE, fem)
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

REGISTER_MATERIAL(FSUncoupledReactiveFatigue, MODULE_MECH, FE_UNCOUPLED_REACTIVE_FATIGUE, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled reactive fatigue", MaterialFlags::TOPLEVEL);

FSUncoupledReactiveFatigue::FSUncoupledReactiveFatigue(FSModel* fem) : FSMaterial(FE_UNCOUPLED_REACTIVE_FATIGUE, fem)
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
