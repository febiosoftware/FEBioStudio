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
#include <MeshLib/FSItemListBuilder.h>
#include <FECore/fecore_enum.h>

FSSurfaceLoad::FSSurfaceLoad(int ntype, FSModel* ps) : FSLoad(ntype, ps)
{
	SetMeshItemType(FE_FACE_FLAG);
}

FSSurfaceLoad::FSSurfaceLoad(int ntype, FSModel* ps, FSItemListBuilder* pi, int nstep) : FSLoad(ntype, ps, pi, nstep)
{
	SetMeshItemType(FE_FACE_FLAG);
}

//-----------------------------------------------------------------------------
FSPressureLoad::FSPressureLoad(FSModel* ps, FSItemListBuilder* pi, int nstep) : FSSurfaceLoad(FE_PRESSURE_LOAD, ps, pi, nstep)
{
	SetTypeString("pressure");
	AddScienceParam(1, UNIT_PRESSURE, "pressure", "pressure")->MakeVariable(true);
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
//	case 2: *GetLoadCurve() = *p.GetLoadCurve(); break;
	}
}

//-----------------------------------------------------------------------------

FSFluidFlux::FSFluidFlux(FSModel* ps, FSItemListBuilder* pi, int nstep) : FSSurfaceLoad(FE_FLUID_FLUX, ps, pi, nstep)
{
	SetTypeString("fluidflux");
	AddDoubleParam(1, "flux", "flux");
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
//	case 2: *GetLoadCurve() = *p.GetLoadCurve(); break;
	case 3: SetMixtureFlag(p.GetIntValue() == 1); break;
	}
}

//-----------------------------------------------------------------------------

FSBPNormalTraction::FSBPNormalTraction(FSModel* ps, FSItemListBuilder* pi, int nstep) : FSSurfaceLoad(FE_BP_NORMAL_TRACTION, ps, pi, nstep)
{
	SetTypeString("normal_traction");
	AddScienceParam(0, "P", "traction");
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
//	case 2: *GetLoadCurve() = *p.GetLoadCurve(); break;
	case 3: SetMixtureFlag(p.GetIntValue() == 1); break;
	}
}

//-----------------------------------------------------------------------------
FSSoluteFlux::FSSoluteFlux(FSModel* ps, FSItemListBuilder* pi, int nstep) : FSSurfaceLoad(FE_SOLUTE_FLUX, ps, pi, nstep)
{
	SetTypeString("soluteflux");
	AddDoubleParam(1, "flux", "flux");
	AddBoolParam(0, "linear", "Type")->SetEnumNames("Nonlinear\0Linear\0");
	AddChoiceParam(0, "solute_id", "Solute")->SetEnumNames("$(solutes)");// ->SetOffset(1);
}

// used only for reading parameters for old file formats
void FSSoluteFlux::LoadParam(const Param& p)
{
	switch (p.GetParamID())
	{
	case 0: SetLinearFlag(p.GetIntValue() == 1); break;
	case 1: SetLoad(p.GetFloatValue()); break;
//	case 2: *GetLoadCurve() = *p.GetLoadCurve(); break;
	case 3: SetBC(p.GetIntValue()); break;
	}
}

//-----------------------------------------------------------------------------
FSSoluteNaturalFlux::FSSoluteNaturalFlux(FSModel* ps, FSItemListBuilder* pi, int nstep) : FSSurfaceLoad(FE_SOLUTE_NATURAL_FLUX, ps, pi, nstep)
{
    SetTypeString("solute natural flux");
    AddChoiceParam(0, "solute_id", "Solute")->SetEnumNames("$(solutes)");
    AddBoolParam(0, "shell_bottom", "Apply to shell bottom")->SetCheckable(false);
}

//-----------------------------------------------------------------------------

FSMatchingOsmoticCoefficient::FSMatchingOsmoticCoefficient(FSModel* ps, FSItemListBuilder* pi, int nstep) : FSSurfaceLoad(FE_MATCHING_OSM_COEF, ps, pi, nstep)
{
    SetTypeString("matching_osm_coef");
    AddScienceParam(0, "P", "ambient_pressure");
    AddScienceParam(0, "c", "ambient_osmolarity");
    AddBoolParam(0, "shell_bottom", "Apply to shell bottom")->SetCheckable(false);
}

//-----------------------------------------------------------------------------

FSHeatFlux::FSHeatFlux(FSModel* ps, FSItemListBuilder* pi, int nstep) : FSSurfaceLoad(FE_HEAT_FLUX, ps, pi, nstep)
{
	SetTypeString("heatflux");
	AddDoubleParam(0.0, "flux", "flux");
}

// used only for reading parameters for old file formats
void FSHeatFlux::LoadParam(const Param& p)
{
	switch (p.GetParamID())
	{
	case 0: SetLoad(p.GetFloatValue()); break;
//	case 1: *GetLoadCurve() = *p.GetLoadCurve(); break;
	}
}


//-----------------------------------------------------------------------------

FSConvectiveHeatFlux::FSConvectiveHeatFlux(FSModel* ps, FSItemListBuilder* pi, int nstep) : FSSurfaceLoad(FE_CONV_HEAT_FLUX, ps, pi, nstep)
{
	SetTypeString("convective_heatflux");
	AddDoubleParam(1, "hc", "hc");
	AddDoubleParam(0, "Ta", "Ta");
}

// used only for reading parameters for old file formats
void FSConvectiveHeatFlux::LoadParam(const Param& p)
{
	switch (p.GetParamID())
	{
	case 0: SetCoefficient(p.GetFloatValue()); break;
	case 1: SetTemperature(p.GetFloatValue()); break;
//	case 2: *GetLoadCurve() = *p.GetLoadCurve(); break;
	}
}

//-----------------------------------------------------------------------------

FSSurfaceTraction::FSSurfaceTraction(FSModel* ps, FSItemListBuilder* pi, int nstep) : FSSurfaceLoad(FE_SURFACE_TRACTION, ps, pi, nstep)
{
	SetTypeString("traction");
	AddDoubleParam(1.0, "scale");
	AddVecParam(vec3d(0,0,1), "traction")->SetUnit(UNIT_PRESSURE);
}

// used only for reading parameters for old file formats
void FSSurfaceTraction::LoadParam(const Param& p)
{
	switch (p.GetParamID())
	{
	case 0: SetTraction(p.GetVec3dValue()); break;
//	case 1: *GetLoadCurve() = *p.GetLoadCurve(); break;
	}
}

//-----------------------------------------------------------------------------

FSSurfaceForceUniform::FSSurfaceForceUniform(FSModel* ps, FSItemListBuilder* pi, int nstep) : FSSurfaceLoad(FE_SURFACE_FORCE, ps, pi, nstep)
{
    SetTypeString("force");
    AddDoubleParam(1.0, "scale");
    AddVecParam(vec3d(0,0,0), "force")->SetUnit(UNIT_FORCE);
}

//-----------------------------------------------------------------------------
FSBearingLoad::FSBearingLoad(FSModel* ps, FSItemListBuilder* pi, int nstep) : FSSurfaceLoad(FE_BEARING_LOAD, ps, pi, nstep)
{
    SetTypeString("bearing load");
    AddDoubleParam(1.0, "scale");
    AddVecParam(vec3d(0,0,0), "force")->SetUnit(UNIT_FORCE);
    AddIntParam(1, "profile")->SetEnumNames("Sinusoidal\0Parabolic\0");
    AddBoolParam(true, "linear", "Type")->SetEnumNames("Nonlinear\0Linear\0");
    AddBoolParam(false , "symmetric_stiffness", "Formulation")->SetEnumNames("Nonsymmetric\0Symmetric\0");
    AddBoolParam(false, "shell_bottom", "Apply to shell bottom");
}

//-----------------------------------------------------------------------------
FSFluidPressureLoad::FSFluidPressureLoad(FSModel* ps, FSItemListBuilder* pi, int nstep) : FSSurfaceLoad(FE_FLUID_PRESSURE_LOAD, ps, pi, nstep)
{
    SetTypeString("fluid pressure");
    AddScienceParam(1, UNIT_PRESSURE, "pressure", "pressure")->MakeVariable(true);
}

// used only for reading parameters for old file formats
void FSFluidPressureLoad::LoadParam(const Param& p)
{
    switch (p.GetParamID())
    {
        case 0: SetLoad(p.GetFloatValue()); break;
//        case 1: *GetLoadCurve() = *p.GetLoadCurve(); break;
    }
}

//-----------------------------------------------------------------------------

FSFluidTraction::FSFluidTraction(FSModel* ps, FSItemListBuilder* pi, int nstep) : FSSurfaceLoad(FE_FLUID_TRACTION, ps, pi, nstep)
{
	SetTypeString("fluid viscous traction");
	AddDoubleParam(1.0, "scale");
	AddVecParam(vec3d(0, 0, 0), "traction")->SetUnit(UNIT_PRESSURE);
}

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
//	case 1: *GetLoadCurve() = *p.GetLoadCurve(); break;
	}
}

//-----------------------------------------------------------------------------

FSFluidVelocity::FSFluidVelocity(FSModel* ps) : FSSurfaceLoad(FE_FLUID_VELOCITY, ps)
{
    SetTypeString("fluid velocity");
    AddVecParam(vec3d(0,0,0), "velocity", "fluid velocity")->SetUnit(UNIT_VELOCITY);
	AddDoubleParam(1, "scale", "scale");
}

FSFluidVelocity::FSFluidVelocity(FSModel* ps, FSItemListBuilder* pi, vec3d t, int nstep) : FSSurfaceLoad(FE_FLUID_VELOCITY, ps, pi, nstep)
{
    SetTypeString("fluid velocity");
    AddVecParam(t, "velocity", "fluid velocity");
	AddDoubleParam(1, "scale", "scale");
}

//-----------------------------------------------------------------------------

FSFluidNormalVelocity::FSFluidNormalVelocity(FSModel* ps) : FSSurfaceLoad(FE_FLUID_NORMAL_VELOCITY, ps)
{
    SetTypeString("fluid normal velocity");
    AddScienceParam(1, UNIT_VELOCITY, "velocity", "normal velocity")->MakeVariable(true);
    AddBoolParam(true, "prescribe_nodal_velocities", "prescribe nodal velocities");
    AddBoolParam(false, "parabolic", "parabolic velocity profile");
    AddBoolParam(false, "prescribe_rim_pressure", "prescribe rim pressure");
}

FSFluidNormalVelocity::FSFluidNormalVelocity(FSModel* ps, FSItemListBuilder* pi, double vn, bool bp, bool bparab, bool brimp, int nstep) : FSSurfaceLoad(FE_FLUID_NORMAL_VELOCITY, ps, pi, nstep)
{
	SetTypeString("fluid normal velocity");
	AddScienceParam(vn, UNIT_VELOCITY, "velocity", "normal velocity");
    AddBoolParam(bp, "prescribe_nodal_velocities", "prescribe nodal velocities");
    AddBoolParam(bparab, "parabolic", "parabolic velocity profile");
    AddBoolParam(brimp, "prescribe_rim_pressure", "prescribe rim pressure");
}

//-----------------------------------------------------------------------------

FSFluidRotationalVelocity::FSFluidRotationalVelocity(FSModel* ps) : FSSurfaceLoad(FE_FLUID_ROTATIONAL_VELOCITY, ps)
{
    SetTypeString("Fluid Rotational Velocity");
    AddScienceParam(1, UNIT_ANGULAR_VELOCITY, "angular_speed", "angular speed");
    AddVecParam(vec3d(0,0,1), "axis", "axis");
    AddVecParam(vec3d(0,0,0), "origin", "origin")->SetUnit(UNIT_LENGTH);
}

FSFluidRotationalVelocity::FSFluidRotationalVelocity(FSModel* ps, FSItemListBuilder* pi, double w, vec3d n, vec3d p, int nstep) : FSSurfaceLoad(FE_FLUID_ROTATIONAL_VELOCITY, ps, pi, nstep)
{
    SetTypeString("Fluid Normal Velocity");
    AddScienceParam(w, UNIT_ANGULAR_VELOCITY, "angular_speed", "angular speed");
    AddVecParam(n, "axis", "axis");
    AddVecParam(p, "origin", "origin")->SetUnit(UNIT_LENGTH);
}

//-----------------------------------------------------------------------------

FSFluidFlowResistance::FSFluidFlowResistance(FSModel* ps) : FSSurfaceLoad(FE_FLUID_FLOW_RESISTANCE, ps)
{
    SetTypeString("fluid resistance");
    AddDoubleParam(0, "R", "resistance");
    AddDoubleParam(0, "pressure_offset", "pressure_offset");
}

FSFluidFlowResistance::FSFluidFlowResistance(FSModel* ps, FSItemListBuilder* pi, double b, double po, int nstep) : FSSurfaceLoad(FE_FLUID_FLOW_RESISTANCE, ps, pi, nstep)
{
    SetTypeString("fluid resistance");
    AddDoubleParam(b, "R", "resistance");
    AddDoubleParam(po, "pressure_offset", "pressure_offset");
}

//-----------------------------------------------------------------------------

FSFluidFlowRCR::FSFluidFlowRCR(FSModel* ps) : FSSurfaceLoad(FE_FLUID_FLOW_RCR, ps)
{
    SetTypeString("fluid RCR");
    AddDoubleParam(0, "R", "proximal resistance");
    AddDoubleParam(0, "Rd", "distal resistance");
    AddDoubleParam(0, "capacitance", "compliance");
    AddDoubleParam(0, "pressure_offset", "pressure offset");
    AddDoubleParam(0, "initial_pressure", "initial pressure");

}

FSFluidFlowRCR::FSFluidFlowRCR(FSModel* ps, FSItemListBuilder* pi, double rp, double rd, double co, double po, double ip, bool be, int nstep) : FSSurfaceLoad(FE_FLUID_FLOW_RCR, ps, pi, nstep)
{
    SetTypeString("fluid RCR");
    AddDoubleParam(rp, "R", "resistance");
    AddDoubleParam(rd, "Rd", "distal resistance");
    AddDoubleParam(co, "capacitance", "compliance");
    AddDoubleParam(po, "pressure_offset", "pressure offset");
    AddDoubleParam(ip, "initial_pressure", "initial pressure");
}

//-----------------------------------------------------------------------------

FSFluidBackflowStabilization::FSFluidBackflowStabilization(FSModel* ps) : FSSurfaceLoad(FE_FLUID_BACKFLOW_STABIL, ps)
{
    SetTypeString("fluid backflow stabilization");
    AddDoubleParam(1, "beta", "beta");
}

FSFluidBackflowStabilization::FSFluidBackflowStabilization(FSModel* ps, FSItemListBuilder* pi, double b, int nstep) : FSSurfaceLoad(FE_FLUID_BACKFLOW_STABIL, ps, pi, nstep)
{
    SetTypeString("fluid backflow stabilization");
    AddDoubleParam(b, "beta", "beta");
}

//-----------------------------------------------------------------------------

FSFluidTangentialStabilization::FSFluidTangentialStabilization(FSModel* ps) : FSSurfaceLoad(FE_FLUID_TANGENTIAL_STABIL, ps)
{
    SetTypeString("fluid tangential stabilization");
    AddDoubleParam(1, "beta", "beta");
}

FSFluidTangentialStabilization::FSFluidTangentialStabilization(FSModel* ps, FSItemListBuilder* pi, double b, int nstep) : FSSurfaceLoad(FE_FLUID_TANGENTIAL_STABIL, ps, pi, nstep)
{
    SetTypeString("fluid tangential stabilization");
    AddDoubleParam(b, "beta", "beta");
}

//-----------------------------------------------------------------------------

FSFSITraction::FSFSITraction(FSModel* ps) : FSSurfaceLoad(FE_FSI_TRACTION, ps)
{
    SetTypeString("fluid-FSI traction");
}

FSFSITraction::FSFSITraction(FSModel* ps, FSItemListBuilder* pi, int nstep) : FSSurfaceLoad(FE_FSI_TRACTION, ps, pi, nstep)
{
    SetTypeString("fluid-FSI traction");
}

//-----------------------------------------------------------------------------

FSBFSITraction::FSBFSITraction(FSModel* ps) : FSSurfaceLoad(FE_BFSI_TRACTION, ps)
{
    SetTypeString("biphasic-FSI traction");
}

FSBFSITraction::FSBFSITraction(FSModel* ps, FSItemListBuilder* pi, int nstep) : FSSurfaceLoad(FE_BFSI_TRACTION, ps, pi, nstep)
{
    SetTypeString("biphasic-FSI traction");
}

//=======================================================================================
FSConcentrationFlux::FSConcentrationFlux(FSModel* ps) : FSSurfaceLoad(FE_CONCENTRATION_FLUX, ps)
{
	SetTypeString("concentration flux");
	AddChoiceParam(0, "solute_id", "Solute")->SetEnumNames("$(solutes)")->SetOffset(1);
	AddDoubleParam(0, "flux");
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

//-----------------------------------------------------------------------------
FSFluidSolutesNaturalFlux::FSFluidSolutesNaturalFlux(FSModel* ps, FSItemListBuilder* pi, int nstep) : FSSurfaceLoad(FE_FLUID_SOLUTES_NATURAL_FLUX, ps, pi, nstep)
{
    SetTypeString("solute natural flux");
    AddChoiceParam(0, "solute_id", "Solute")->SetEnumNames("$(solutes)");
}

//=======================================================================================
FEBioSurfaceLoad::FEBioSurfaceLoad(FSModel* ps) : FSSurfaceLoad(FE_FEBIO_SURFACE_LOAD, ps)
{
}

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
