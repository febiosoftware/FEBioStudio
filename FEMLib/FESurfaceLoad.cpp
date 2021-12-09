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

#include "FESurfaceLoad.h"
#include <FECore/units.h>
#include <MeshTools/FEItemListBuilder.h>
#include "enums.h"

FSSurfaceLoad::FSSurfaceLoad(int ntype, FSModel* ps) : FSLoad(ntype, ps)
{
	m_superClassID = FE_SURFACE_LOAD;
	SetMeshItemType(FE_FACE_FLAG);
}

FSSurfaceLoad::FSSurfaceLoad(int ntype, FSModel* ps, FEItemListBuilder* pi, int nstep) : FSLoad(ntype, ps, pi, nstep)
{
	m_superClassID = FE_SURFACE_LOAD;
	SetMeshItemType(FE_FACE_FLAG);
}

//-----------------------------------------------------------------------------
FSPressureLoad::FSPressureLoad(FSModel* ps, FEItemListBuilder* pi, int nstep) : FSSurfaceLoad(FE_PRESSURE_LOAD, ps, pi, nstep)
{
	SetTypeString("Pressure Load");
	Param* p = AddScienceParam(1, UNIT_PRESSURE, "pressure", "pressure");
	p->SetLoadCurve();
	p->MakeVariable(true);
	AddBoolParam(false, "linear", "Type")->SetEnumNames("Nonlinear\0Linear\0");
	AddBoolParam(true , "symmetric_stiffness", "Formulation")->SetEnumNames("Nonsymmetric\0Symmetric\0");
	AddBoolParam(false, "shell_bottom", "Apply to shell bottom");
}

// used only for reading parameters for old file formats
void FSPressureLoad::LoadParam(const Param& p)
{
	switch (p.GetParamID())
	{
	case 0: SetLinearFlag(p.GetIntValue() == 1); break;
	case 1: SetLoad(p.GetFloatValue()); break;
	case 2: *GetLoadCurve() = *p.GetLoadCurve(); break;
	}
}

//-----------------------------------------------------------------------------

FSFluidFlux::FSFluidFlux(FSModel* ps, FEItemListBuilder* pi, int nstep) : FSSurfaceLoad(FE_FLUID_FLUX, ps, pi, nstep)
{
	SetTypeString("Fluid volumetric flow rate");
	AddDoubleParam(1, "flux", "flux")->SetLoadCurve();
	AddBoolParam(0, "linear", "Type")->SetEnumNames("Nonlinear\0Linear\0");
	AddBoolParam(0, "mixture", "Flux")->SetEnumNames("Fluid\0Mixture\0");
}

// used only for reading parameters for old file formats
void FSFluidFlux::LoadParam(const Param& p)
{
	switch (p.GetParamID())
	{
	case 0: SetLinearFlag(p.GetIntValue() == 1); break;
	case 1: SetLoad(p.GetFloatValue()); break;
	case 2: *GetLoadCurve() = *p.GetLoadCurve(); break;
	case 3: SetMixtureFlag(p.GetIntValue() == 1); break;
	}
}

//-----------------------------------------------------------------------------

FSBPNormalTraction::FSBPNormalTraction(FSModel* ps, FEItemListBuilder* pi, int nstep) : FSSurfaceLoad(FE_BP_NORMAL_TRACTION, ps, pi, nstep)
{
	SetTypeString("Mixture normal traction");
	AddScienceParam(0, "P", "traction")->SetLoadCurve();
	AddBoolParam(0, "linear", "Type")->SetEnumNames("Nonlinear\0Linear\0");
	AddBoolParam(0, "effective", "Traction")->SetEnumNames("Mixture\0Effective\0");
}

// used only for reading parameters for old file formats
void FSBPNormalTraction::LoadParam(const Param& p)
{
	switch (p.GetParamID())
	{
	case 0: SetLinearFlag(p.GetIntValue() == 1); break;
	case 1: SetLoad(p.GetFloatValue()); break;
	case 2: *GetLoadCurve() = *p.GetLoadCurve(); break;
	case 3: SetMixtureFlag(p.GetIntValue() == 1); break;
	}
}

//-----------------------------------------------------------------------------
FSSoluteFlux::FSSoluteFlux(FSModel* ps, FEItemListBuilder* pi, int nstep) : FSSurfaceLoad(FE_SOLUTE_FLUX, ps, pi, nstep)
{
	SetTypeString("Solute molar flow rate");
	AddDoubleParam(1, "flux", "flux")->SetLoadCurve();
	AddBoolParam(0, "linear", "Type")->SetEnumNames("Nonlinear\0Linear\0");
	AddChoiceParam(0, "solute_id", "Solute")->SetEnumNames("$(Solutes)")->SetOffset(1);
}

// used only for reading parameters for old file formats
void FSSoluteFlux::LoadParam(const Param& p)
{
	switch (p.GetParamID())
	{
	case 0: SetLinearFlag(p.GetIntValue() == 1); break;
	case 1: SetLoad(p.GetFloatValue()); break;
	case 2: *GetLoadCurve() = *p.GetLoadCurve(); break;
	case 3: SetBC(p.GetIntValue()); break;
	}
}

//-----------------------------------------------------------------------------

FSMatchingOsmoticCoefficient::FSMatchingOsmoticCoefficient(FSModel* ps, FEItemListBuilder* pi, int nstep) : FSSurfaceLoad(FE_MATCHING_OSM_COEF, ps, pi, nstep)
{
    SetTypeString("Matching osmotic coefficient");
    AddScienceParam(0, "P", "ambient_pressure")->SetLoadCurve();
    AddScienceParam(0, "c", "ambient_osmolarity")->SetLoadCurve();
    AddBoolParam(0, "shell_bottom", "Apply to shell bottom")->SetCheckable(false);
}

//-----------------------------------------------------------------------------

FSHeatFlux::FSHeatFlux(FSModel* ps, FEItemListBuilder* pi, int nstep) : FSSurfaceLoad(FE_HEAT_FLUX, ps, pi, nstep)
{
	SetTypeString("Heat Flux");
	AddDoubleParam(0.0, "flux", "flux")->SetLoadCurve();
}

// used only for reading parameters for old file formats
void FSHeatFlux::LoadParam(const Param& p)
{
	switch (p.GetParamID())
	{
	case 0: SetLoad(p.GetFloatValue()); break;
	case 1: *GetLoadCurve() = *p.GetLoadCurve(); break;
	}
}


//-----------------------------------------------------------------------------

FSConvectiveHeatFlux::FSConvectiveHeatFlux(FSModel* ps, FEItemListBuilder* pi, int nstep) : FSSurfaceLoad(FE_CONV_HEAT_FLUX, ps, pi, nstep)
{
	SetTypeString("Convective Heat Flux");
	AddDoubleParam(1, "hc", "hc");
	AddDoubleParam(0, "Ta", "Ta")->SetLoadCurve();
}

// used only for reading parameters for old file formats
void FSConvectiveHeatFlux::LoadParam(const Param& p)
{
	switch (p.GetParamID())
	{
	case 0: SetCoefficient(p.GetFloatValue()); break;
	case 1: SetTemperature(p.GetFloatValue()); break;
	case 2: *GetLoadCurve() = *p.GetLoadCurve(); break;
	}
}

//-----------------------------------------------------------------------------

FSSurfaceTraction::FSSurfaceTraction(FSModel* ps, FEItemListBuilder* pi, int nstep) : FSSurfaceLoad(FE_SURFACE_TRACTION, ps, pi, nstep)
{
	SetTypeString("Surface Traction");
	AddDoubleParam(1.0, "scale")->SetLoadCurve();
	AddVecParam(vec3d(0,0,1), "traction")->SetUnit(UNIT_PRESSURE);
}

// used only for reading parameters for old file formats
void FSSurfaceTraction::LoadParam(const Param& p)
{
	switch (p.GetParamID())
	{
	case 0: SetTraction(p.GetVec3dValue()); break;
	case 1: *GetLoadCurve() = *p.GetLoadCurve(); break;
	}
}

//-----------------------------------------------------------------------------
FSFluidPressureLoad::FSFluidPressureLoad(FSModel* ps, FEItemListBuilder* pi, int nstep) : FSSurfaceLoad(FE_FLUID_PRESSURE_LOAD, ps, pi, nstep)
{
    SetTypeString("Fluid pressure");
    Param* p = AddScienceParam(1, UNIT_PRESSURE, "pressure", "pressure");
    p->SetLoadCurve();
    p->MakeVariable(true);
}

// used only for reading parameters for old file formats
void FSFluidPressureLoad::LoadParam(const Param& p)
{
    switch (p.GetParamID())
    {
        case 0: SetLoad(p.GetFloatValue()); break;
        case 1: *GetLoadCurve() = *p.GetLoadCurve(); break;
    }
}

//-----------------------------------------------------------------------------

FSFluidTraction::FSFluidTraction(FSModel* ps, FEItemListBuilder* pi, int nstep) : FSSurfaceLoad(FE_FLUID_TRACTION, ps, pi, nstep)
{
	SetTypeString("Fluid viscous traction");
	AddDoubleParam(1.0, "scale")->SetLoadCurve();
	AddVecParam(vec3d(0, 0, 0), "traction")->SetUnit(UNIT_PRESSURE);
}

LoadCurve* FSFluidTraction::GetLoadCurve() { return GetParamLC(LOAD); }

void FSFluidTraction::SetScale(double s)
{
	SetFloatValue(LOAD, s);
}

double FSFluidTraction::GetScale()
{
	return GetFloatValue(LOAD);
}

void FSFluidTraction::SetTraction(const vec3d& t) { SetVecValue(TRACTION, t); }
vec3d FSFluidTraction::GetTraction() { return GetVecValue(TRACTION); }

// used only for reading parameters for old file formats
void FSFluidTraction::LoadParam(const Param& p)
{
	switch (p.GetParamID())
	{
	case 0: SetTraction(p.GetVec3dValue()); break;
	case 1: *GetLoadCurve() = *p.GetLoadCurve(); break;
	}
}

//-----------------------------------------------------------------------------

FSFluidVelocity::FSFluidVelocity(FSModel* ps) : FSSurfaceLoad(FE_FLUID_VELOCITY, ps)
{
    SetTypeString("Fluid Velocity Vector");
    AddVecParam(vec3d(0,0,0), "velocity", "fluid velocity");
	AddDoubleParam(1, "scale", "scale");
}

FSFluidVelocity::FSFluidVelocity(FSModel* ps, FEItemListBuilder* pi, vec3d t, int nstep) : FSSurfaceLoad(FE_FLUID_VELOCITY, ps, pi, nstep)
{
    SetTypeString("Fluid Velocity Vector");
    AddVecParam(t, "velocity", "fluid velocity")->SetLoadCurve();
	AddDoubleParam(1, "scale", "scale");
}

//-----------------------------------------------------------------------------

FSFluidNormalVelocity::FSFluidNormalVelocity(FSModel* ps) : FSSurfaceLoad(FE_FLUID_NORMAL_VELOCITY, ps)
{
    SetTypeString("Fluid Normal Velocity");
    AddDoubleParam(1, "velocity", "velocity")->SetLoadCurve();
    AddBoolParam(true, "prescribe_nodal_velocities", "prescribe nodal velocities");
    AddBoolParam(false, "parabolic", "parabolic velocity profile");
    AddBoolParam(false, "prescribe_rim_pressure", "prescribe rim pressure");
}

FSFluidNormalVelocity::FSFluidNormalVelocity(FSModel* ps, FEItemListBuilder* pi, double vn, bool bp, bool bparab, bool brimp, int nstep) : FSSurfaceLoad(FE_FLUID_NORMAL_VELOCITY, ps, pi, nstep)
{
    SetTypeString("Fluid Normal Velocity");
    AddDoubleParam(vn, "velocity", "velocity")->SetLoadCurve();
    AddBoolParam(bp, "prescribe_nodal_velocities", "prescribe nodal velocities");
    AddBoolParam(bparab, "parabolic", "parabolic velocity profile");
    AddBoolParam(brimp, "prescribe_rim_pressure", "prescribe rim pressure");
}

//-----------------------------------------------------------------------------

FSFluidRotationalVelocity::FSFluidRotationalVelocity(FSModel* ps) : FSSurfaceLoad(FE_FLUID_ROTATIONAL_VELOCITY, ps)
{
    SetTypeString("Fluid Rotational Velocity");
    AddDoubleParam(1, "angular_speed", "angular speed")->SetLoadCurve();
    AddVecParam(vec3d(0,0,1), "axis", "axis");
    AddVecParam(vec3d(0,0,0), "origin", "origin")->SetUnit(UNIT_LENGTH);
}

FSFluidRotationalVelocity::FSFluidRotationalVelocity(FSModel* ps, FEItemListBuilder* pi, double w, vec3d n, vec3d p, int nstep) : FSSurfaceLoad(FE_FLUID_ROTATIONAL_VELOCITY, ps, pi, nstep)
{
    SetTypeString("Fluid Normal Velocity");
    AddDoubleParam(w, "angular_speed", "angular_speed")->SetLoadCurve();
    AddVecParam(n, "axis", "axis");
    AddVecParam(p, "origin", "origin")->SetUnit(UNIT_LENGTH);
}

//-----------------------------------------------------------------------------

FSFluidFlowResistance::FSFluidFlowResistance(FSModel* ps) : FSSurfaceLoad(FE_FLUID_FLOW_RESISTANCE, ps)
{
    SetTypeString("Fluid Flow Resistance");
    AddDoubleParam(0, "R", "resistance")->SetLoadCurve();
    AddDoubleParam(0, "pressure_offset", "pressure_offset")->SetLoadCurve();
}

FSFluidFlowResistance::FSFluidFlowResistance(FSModel* ps, FEItemListBuilder* pi, double b, double po, int nstep) : FSSurfaceLoad(FE_FLUID_FLOW_RESISTANCE, ps, pi, nstep)
{
    SetTypeString("Fluid Flow Resistance");
    AddDoubleParam(b, "R", "resistance")->SetLoadCurve();
    AddDoubleParam(po, "pressure_offset", "pressure_offset")->SetLoadCurve();
}

//-----------------------------------------------------------------------------

FSFluidFlowRCR::FSFluidFlowRCR(FSModel* ps) : FSSurfaceLoad(FE_FLUID_FLOW_RCR, ps)
{
    SetTypeString("Fluid RCR");
    AddDoubleParam(0, "R", "proximal resistance")->SetLoadCurve();
    AddDoubleParam(0, "Rd", "distal resistance")->SetLoadCurve();
    AddDoubleParam(0, "capacitance", "compliance")->SetLoadCurve();
    AddDoubleParam(0, "pressure_offset", "pressure offset")->SetLoadCurve();
    AddDoubleParam(0, "initial_pressure", "initial pressure")->SetLoadCurve();
    AddBoolParam(true, "Bernoulli", "Bernoulli");

}

FSFluidFlowRCR::FSFluidFlowRCR(FSModel* ps, FEItemListBuilder* pi, double rp, double rd, double co, double po, double ip, bool be, int nstep) : FSSurfaceLoad(FE_FLUID_FLOW_RCR, ps, pi, nstep)
{
    SetTypeString("Fluid RCR");
    AddDoubleParam(rp, "R", "resistance")->SetLoadCurve();
    AddDoubleParam(rd, "Rd", "distal resistance")->SetLoadCurve();
    AddDoubleParam(co, "capacitance", "compliance")->SetLoadCurve();
    AddDoubleParam(po, "pressure_offset", "pressure offset")->SetLoadCurve();
    AddDoubleParam(ip, "initial_pressure", "initial pressure")->SetLoadCurve();
    AddBoolParam(be, "Bernoulli", "Bernoulli");
}

//-----------------------------------------------------------------------------

FSFluidBackflowStabilization::FSFluidBackflowStabilization(FSModel* ps) : FSSurfaceLoad(FE_FLUID_BACKFLOW_STABIL, ps)
{
    SetTypeString("Fluid Backflow Stabilization");
    AddDoubleParam(1, "beta", "beta")->SetLoadCurve();
}

FSFluidBackflowStabilization::FSFluidBackflowStabilization(FSModel* ps, FEItemListBuilder* pi, double b, int nstep) : FSSurfaceLoad(FE_FLUID_BACKFLOW_STABIL, ps, pi, nstep)
{
    SetTypeString("Fluid Backflow Stabilization");
    AddDoubleParam(b, "beta", "beta")->SetLoadCurve();
}

//-----------------------------------------------------------------------------

FSFluidTangentialStabilization::FSFluidTangentialStabilization(FSModel* ps) : FSSurfaceLoad(FE_FLUID_TANGENTIAL_STABIL, ps)
{
    SetTypeString("Fluid Tangential Stabilization");
    AddDoubleParam(1, "beta", "beta")->SetLoadCurve();
}

FSFluidTangentialStabilization::FSFluidTangentialStabilization(FSModel* ps, FEItemListBuilder* pi, double b, int nstep) : FSSurfaceLoad(FE_FLUID_TANGENTIAL_STABIL, ps, pi, nstep)
{
    SetTypeString("Fluid Tangential Stabilization");
    AddDoubleParam(b, "beta", "beta")->SetLoadCurve();
}

//-----------------------------------------------------------------------------

FSFSITraction::FSFSITraction(FSModel* ps) : FSSurfaceLoad(FE_FSI_TRACTION, ps)
{
    SetTypeString("FSI Interface Traction");
}

FSFSITraction::FSFSITraction(FSModel* ps, FEItemListBuilder* pi, int nstep) : FSSurfaceLoad(FE_FSI_TRACTION, ps, pi, nstep)
{
    SetTypeString("FSI Interface Traction");
}

//-----------------------------------------------------------------------------

FSBFSITraction::FSBFSITraction(FSModel* ps) : FSSurfaceLoad(FE_BFSI_TRACTION, ps)
{
    SetTypeString("Biphasic-FSI Interface Traction");
}

FSBFSITraction::FSBFSITraction(FSModel* ps, FEItemListBuilder* pi, int nstep) : FSSurfaceLoad(FE_BFSI_TRACTION, ps, pi, nstep)
{
    SetTypeString("Biphasic-FSI Interface Traction");
}

//=======================================================================================
FSConcentrationFlux::FSConcentrationFlux(FSModel* ps) : FSSurfaceLoad(FE_CONCENTRATION_FLUX, ps)
{
	SetTypeString("concentration flux");
	AddChoiceParam(0, "solute_id", "Solute")->SetEnumNames("$(Solutes)")->SetOffset(1);
	AddDoubleParam(0, "flux");
}

LoadCurve* FSConcentrationFlux::GetLoadCurve()
{ 
	return GetParamLC(FLUX); 
}

void FSConcentrationFlux::SetFlux(double f)
{ 
	SetFloatValue(FLUX, f); 
}

double FSConcentrationFlux::GetFlux()
{ 
	return GetFloatValue(FLUX); 
}

int FSConcentrationFlux::GetSoluteID()
{ 
	return GetIntValue(SOL_ID);
}

void FSConcentrationFlux::SetSoluteID(int n)
{
	SetIntValue(SOL_ID, n);
}

//=======================================================================================
FEBioSurfaceLoad::FEBioSurfaceLoad(FSModel* ps) : FSSurfaceLoad(FE_FEBIO_SURFACE_LOAD, ps)
{
}

LoadCurve* FEBioSurfaceLoad::GetLoadCurve() { assert(false); return nullptr; }

void FEBioSurfaceLoad::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FSSurfaceLoad::Save(ar);
	}
	ar.EndChunk();
}

void FEBioSurfaceLoad::Load(IArchive& ar)
{
	TRACE("FEBioSurfaceLoad::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FSSurfaceLoad::Load(ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
}
