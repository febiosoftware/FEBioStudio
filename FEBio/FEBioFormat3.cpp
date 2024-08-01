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
#include "FEBioFormat3.h"
#include <FEMLib/FERigidConstraint.h>
#include <GeomLib/GMeshObject.h>
#include <FEMLib/FEInitialCondition.h>
#include <FEMLib/FEBodyLoad.h>
#include <FEMLib/FEModelConstraint.h>
#include <FEMLib/GDiscreteObject.h>
#include <MeshLib/FEElementData.h>
#include <MeshLib/FESurfaceData.h>
#include <MeshLib/FENodeData.h>
#include <GeomLib/GModel.h>
#include <GeomLib/GGroup.h>
#include <GeomLib/FSGroup.h>
#include <FEBioLink/FEBioModule.h>
#include <FEBioLink/FEBioClass.h>
#include <FEMLib/FERigidLoad.h>
#include <assert.h>
#include <sstream>
using namespace std;

#define CREATE_SURFACE_LOAD(className) createNewSurfaceLoad(new className(&fem), #className, CountLoads<className>(fem))

//-----------------------------------------------------------------------------
FSSurfaceLoad* createNewSurfaceLoad(FSSurfaceLoad* psl, const char* szclass, int N)
{
	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "%s%d", szclass + 2, N + 1);
	psl->SetName(szname);
	return psl;
}

//-----------------------------------------------------------------------------
vector<string> GetDOFList(string sz)
{
    vector<string> dofs;
    int nc = 0;
    while (nc != -1) {
        nc = (int)sz.find(",");
        dofs.push_back(sz.substr(0,nc));
        sz = sz.substr(nc+1);
    }
    
    return dofs;
}

int GetDOFDir(vector<string> sz)
{
    int dof = 0;
    for (int i=0; i<sz.size(); ++i)
    {
        if (sz[i].find("x") != string::npos) dof |= 1;
        if (sz[i].find("y") != string::npos) dof |= (1 << 1);
        if (sz[i].find("z") != string::npos) dof |= (1 << 2);
    }
    return dof;
}

int GetROTDir(vector<string> sz)
{
    int dof = 0;
    for (int i=0; i<sz.size(); ++i)
    {
        if (sz[i].find("u") != string::npos) dof |= 1;
        if (sz[i].find("v") != string::npos) dof |= (1 << 1);
        if (sz[i].find("w") != string::npos) dof |= (1 << 2);
    }
    return dof;
}

bool validate_dof(string bc)
{
    if      (bc == "x") return true;
    else if (bc == "y") return true;
    else if (bc == "z") return true;
    else if (bc == "T") return true;
    else if (bc == "p") return true;
    else if (bc == "q") return true;
    else if (bc == "wx") return true;
    else if (bc == "wy") return true;
    else if (bc == "wz") return true;
    else if (bc == "ef") return true;
    else if (bc == "gx") return true;
    else if (bc == "gy") return true;
    else if (bc == "gz") return true;
    else if (bc == "u")  return true;
    else if (bc == "v")  return true;
    else if (bc == "w")  return true;
    else if (bc == "sx") return true;
    else if (bc == "sy") return true;
    else if (bc == "sz") return true;
    else if (bc == "c")  return true;
    else if (bc.compare(0,1,"c") == 0) {
        int isol = 0;
        sscanf(bc.substr(1).c_str(),"%d",&isol);
        if (isol > 0) return true;
    }
    else if (bc == "d")  return true;
    else if (bc.compare(0,1,"d") == 0) {
        int isol = 0;
        sscanf(bc.substr(1).c_str(),"%d",&isol);
        if (isol > 0) return true;
    }
    return false;
}

//-----------------------------------------------------------------------------
FEBioFormat3::FEBioFormat3(FEBioFileImport* fileReader, FEBioInputModel& febio) : FEBioFormat(fileReader, febio)
{
	m_geomFormat = 0;
}

FEBioFormat3::~FEBioFormat3()
{
}

FEBioInputModel::Part* FEBioFormat3::DefaultPart()
{
	FEBioInputModel& febio = GetFEBioModel();
	if (m_geomFormat == 0)
	{
		m_geomFormat = 1;
		assert(febio.Parts() == 0);
		return febio.AddPart("Object01");
	}
	else if (m_geomFormat == 1) return &febio.GetPart(0);
	else return 0;
} 

bool FEBioFormat3::ParseSection(XMLTag& tag)
{
	if (m_geomOnly)
	{
		if		(tag == "Mesh"       ) ParseMeshSection(tag);
		else if (tag == "MeshDomains") ParseMeshDomainsSection(tag);
		else if (tag == "MeshData"   ) ParseMeshDataSection(tag);
		else tag.m_preader->SkipTag(tag);
	}
	else
	{
		// make sure the module section was read in
		if ((m_nAnalysis == -1) && (tag != "Module"))
		{
			throw std::runtime_error("Required Module section is missing.");
		}

		if      (tag == "Module"     ) ParseModuleSection     (tag);
		else if (tag == "Control"    ) ParseControlSection    (tag);
		else if (tag == "Material"   ) ParseMaterialSection   (tag);
		else if (tag == "Mesh"       ) ParseMeshSection       (tag);
		else if (tag == "Geometry"   ) ParseGeometrySection   (tag);
		else if (tag == "MeshDomains") ParseMeshDomainsSection(tag);
		else if (tag == "MeshData"   ) ParseMeshDataSection   (tag);
		else if (tag == "MeshAdaptor") ParseMeshAdaptorSection(tag);
		else if (tag == "Boundary"   ) ParseBoundarySection   (tag);
		else if (tag == "Constraints") ParseConstraintSection (tag);
		else if (tag == "Loads"      ) ParseLoadsSection      (tag);
		else if (tag == "Contact"    ) ParseContactSection    (tag);
		else if (tag == "Discrete"   ) ParseDiscreteSection   (tag);
		else if (tag == "Initial"    ) ParseInitialSection    (tag);
		else if (tag == "Rigid"      ) ParseRigidSection      (tag);
		else if (tag == "Globals"    ) ParseGlobalsSection    (tag);
		else if (tag == "LoadData"   ) ParseLoadDataSection   (tag);
		else if (tag == "Output"     ) ParseOutputSection     (tag);
		else if (tag == "Step"       ) ParseStepSection       (tag);
		else return false;
	}
	
	return true;
}

//-----------------------------------------------------------------------------
// Parse the Module section
bool FEBioFormat3::ParseModuleSection(XMLTag &tag)
{
	m_nAnalysis = -1;
	XMLAtt& atype = tag.Attribute("type");
	if      (atype == "solid"      ) m_nAnalysis = FE_STEP_MECHANICS;
	else if (atype == "heat"       ) m_nAnalysis = FE_STEP_HEAT_TRANSFER;
	else if (atype == "biphasic"   ) m_nAnalysis = FE_STEP_BIPHASIC;
	else if (atype == "solute"     ) m_nAnalysis = FE_STEP_BIPHASIC_SOLUTE;
	else if (atype == "multiphasic") m_nAnalysis = FE_STEP_MULTIPHASIC;
	else if (atype == "fluid"      ) m_nAnalysis = FE_STEP_FLUID;
    else if (atype == "fluid-FSI"  ) m_nAnalysis = FE_STEP_FLUID_FSI;
	else if (atype == "reaction-diffusion") m_nAnalysis = FE_STEP_REACTION_DIFFUSION;
    else if (atype == "polar fluid") m_nAnalysis = FE_STEP_POLAR_FLUID;
    else if (atype == "fluid-solutes" ) m_nAnalysis = FE_STEP_FLUID_SOLUTES;
    else if (atype == "thermo-fluid" ) m_nAnalysis = FE_STEP_THERMO_FLUID;
	else if (atype == "explicit-solid") m_nAnalysis = FE_STEP_EXPLICIT_SOLID;
	else
	{
		FileReader()->AddLogEntry("Unknown module type. (line %d)", tag.currentLine());
		throw XMLReader::InvalidAttributeValue(tag, "type", atype.m_val.c_str());
		return false;
	}

	const char* sztype = atype.cvalue();
	if (strcmp(sztype, "explicit-solid") == 0) sztype = "solid";

	int moduleId = FEBio::GetModuleId(sztype);
	if (moduleId < 0) { throw XMLReader::InvalidAttributeValue(tag, "type", sztype); }
	FileReader()->GetProject().SetModule(moduleId, false);

	// set the project's active modules
/*	FEProject& prj = FileReader()->GetProject();
	switch (m_nAnalysis)
	{
	case FE_STEP_MECHANICS         : prj.SetModule(MODULE_MECH); break;
	case FE_STEP_HEAT_TRANSFER     : prj.SetModule(MODULE_HEAT); break;
	case FE_STEP_BIPHASIC          : prj.SetModule(MODULE_MECH | MODULE_BIPHASIC); break;
	case FE_STEP_BIPHASIC_SOLUTE   : prj.SetModule(MODULE_MECH | MODULE_BIPHASIC | MODULE_SOLUTES); break;
	case FE_STEP_MULTIPHASIC       : prj.SetModule(MODULE_MECH | MODULE_BIPHASIC | MODULE_MULTIPHASIC | MODULE_SOLUTES | MODULE_REACTIONS); break;
	case FE_STEP_FLUID             : prj.SetModule(MODULE_FLUID); break;
	case FE_STEP_FLUID_FSI         : prj.SetModule(MODULE_MECH | MODULE_FLUID | MODULE_FLUID_FSI); break;
	case FE_STEP_REACTION_DIFFUSION: prj.SetModule(MODULE_REACTIONS | MODULE_SOLUTES | MODULE_REACTION_DIFFUSION); break;
	default:
		assert(false);
	}
*/
	return (m_nAnalysis != -1);
}
//=============================================================================
//
//                                C O N T R O L
//
//=============================================================================

//-----------------------------------------------------------------------------
//  This function parses the control section from the xml file
//
bool FEBioFormat3::ParseControlSection(XMLTag& tag)
{
	// make sure the section is not empty
	if (tag.isleaf()) return true;

	// initialize default settings
	STEP_SETTINGS ops; ops.Defaults();
	ops.bauto = false;
	int nmplc = -1;
	ops.nanalysis = -1;

	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// create a new analysis step from these control settings
	if (m_pstep == 0) m_pstep = NewStep(fem, m_nAnalysis);
	FSAnalysisStep* pstep = dynamic_cast<FSAnalysisStep*>(m_pstep);
	assert(pstep);

	// parse the settings
	++tag;
	do
	{
		if (ReadParam(*pstep, tag) == false)
		{
			if (tag == "analysis")
			{
				string analysis = tag.szvalue();
				if      ((analysis == "static"      )||(analysis == "STATIC"      )) ops.nanalysis = FE_STATIC;
				else if ((analysis == "steady-state")||(analysis == "STEADY-STATE")) ops.nanalysis = FE_STATIC;
				else if ((analysis == "dynamic"     )||(analysis == "DYNAMIC"     )) ops.nanalysis = FE_DYNAMIC;
				else if ((analysis == "transient"   )||(analysis == "TRANSIENT"   )) ops.nanalysis = FE_DYNAMIC;
				else FileReader()->AddLogEntry("unknown type in analysis. Assuming static analysis (line %d)", tag.currentLine());
			}
			else if (tag == "time_steps") tag.value(ops.ntime);
			else if (tag == "final_time") tag.value(ops.tfinal);
			else if (tag == "step_size") tag.value(ops.dt);
			else if (tag == "solver")
			{
				++tag;
				do
				{
					if      (tag == "max_refs") tag.value(ops.maxref);
					else if (tag == "max_ups")
					{
						tag.value(ops.ilimit);
//						if (ops.ilimit == 0)
//						{
//							ops.mthsol = 1;
//							ops.ilimit = 10;
//						}
					}
					else if (tag == "symmetric_stiffness")
					{
						int nval; tag.value(nval);
						if (nval == 1) ops.nmatfmt = 1; else ops.nmatfmt = 2;
					}
					else if (tag == "diverge_reform") tag.value(ops.bdivref);
					else if (tag == "reform_each_time_step") tag.value(ops.brefstep);
					else if (tag == "logSolve") tag.value(ops.logSolve);
					else if (tag == "equation_scheme") tag.value(ops.neqscheme);
					else if (tag == "optimize_bw") tag.value(ops.bminbw);
					else ReadParam(*pstep, tag);
					++tag;
				}
				while (!tag.isend());
			}
			else if (tag == "time_stepper")
			{
				ops.bauto = true;
				++tag;
				do
				{
					if (tag == "dtmin") tag.value(ops.dtmin);
					else if (tag == "dtmax")
					{
						tag.value(ops.dtmax);
						XMLAtt* pa = tag.AttributePtr("lc");
						if (pa)
						{
							pa->value(nmplc);
						}
					}
					else if (tag == "max_retries") tag.value(ops.mxback);
					else if (tag == "opt_iter") tag.value(ops.iteopt);
					else if (tag == "aggressiveness") tag.value(ops.ncut);
					else if (tag == "dtforce") tag.value(ops.dtforce);
					else ParseUnknownTag(tag);

					++tag;
				} while (!tag.isend());
			}
			else if (tag == "alpha") 
			{
				tag.value(ops.alpha); ops.override_rhoi = true;
			}
			else if (tag == "beta") tag.value(ops.beta);
			else if (tag == "gamma") tag.value(ops.gamma);
			else if (tag == "optimize_bw") tag.value(ops.bminbw);
			else if (tag == "plot_level")
			{
				char sz[256]; tag.value(sz);
				ops.plot_level = FE_PLOT_MAJOR_ITRS;
				if      (strcmp(sz, "PLOT_NEVER"        ) == 0) ops.plot_level = FE_PLOT_NEVER;
				else if (strcmp(sz, "PLOT_MAJOR_ITRS"   ) == 0) ops.plot_level = FE_PLOT_MAJOR_ITRS;
				else if (strcmp(sz, "PLOT_MINOR_ITRS"   ) == 0) ops.plot_level = FE_PLOT_MINOR_ITRS;
				else if (strcmp(sz, "PLOT_MUST_POINTS"  ) == 0) ops.plot_level = FE_PLOT_MUST_POINTS;
				else if (strcmp(sz, "PLOT_FINAL"        ) == 0) ops.plot_level = FE_PLOT_FINAL;
				else if (strcmp(sz, "PLOT_AUGMENTATIONS") == 0) ops.plot_level = FE_PLOT_AUGMENTATIONS;
				else if (strcmp(sz, "PLOT_STEP_FINAL"   ) == 0) ops.plot_level = FE_PLOT_STEP_FINAL;
				else
				{
					FileReader()->AddLogEntry("unknown plot_level (line %d)", tag.currentLine());
				}
			}
            else if (tag == "plot_stride") tag.value(ops.plot_stride);
			else if (tag == "plot_zero_state") tag.value(ops.plot_zero);
			else if (tag == "plot_range")
			{
				tag.value(ops.plot_range, 2);
			}
			else if (tag == "output_level") tag.value(ops.output_level);
			else if (tag == "adaptor_re_solve") tag.value(ops.adapter_re_solve);
			else ParseUnknownTag(tag);
		}
		++tag;
	} 
	while (!tag.isend());

	// check the analysis flag
	if (ops.nanalysis == -1)
	{
		// default analysis depends on step type
		int ntype = m_pstep->GetType();
		if ((ntype == FE_STEP_BIPHASIC) || (ntype == FE_STEP_BIPHASIC_SOLUTE) || (ntype == FE_STEP_MULTIPHASIC) || (ntype == FE_STEP_FLUID) || (ntype == FE_STEP_FLUID_FSI) || (ntype == FE_STEP_POLAR_FLUID)) ops.nanalysis = FE_DYNAMIC;
		else ops.nanalysis = FE_STATIC;
	}

	// check if final_time was set on import
	if ((ops.tfinal > 0) && (ops.dt > 0)) ops.ntime = (int)(ops.tfinal / ops.dt);

	// copy settings
	pstep->GetSettings() = ops;
	if (nmplc >= 0)
	{
		STEP_SETTINGS& ops = pstep->GetSettings();
		ops.bmust = true;
		LoadCurve* plc = pstep->GetMustPointLoadCurve();
		febio.AddParamCurve(plc, nmplc - 1);
	}
	else ops.bmust = false;

	return true;
}



//=============================================================================
//
//                                G E O M E T R Y
//
//=============================================================================

//-----------------------------------------------------------------------------
bool FEBioFormat3::ParseMeshSection(XMLTag& tag)
{
	// make sure the section is not empty
	if (tag.isleaf()) return true;

	// loop over all sections
	++tag;
	do
	{
		if      (tag == "Nodes"      ) ParseGeometryNodes      (DefaultPart(), tag);
		else if (tag == "Elements"   ) ParseGeometryElements   (DefaultPart(), tag);
		else if (tag == "NodeSet"    ) ParseGeometryNodeSet    (DefaultPart(), tag);
		else if (tag == "Surface"    ) ParseGeometrySurface    (DefaultPart(), tag);
		else if (tag == "ElementSet" ) ParseGeometryElementSet (DefaultPart(), tag);
		else if (tag == "DiscreteSet") ParseGeometryDiscreteSet(DefaultPart(), tag);
		else if (tag == "SurfacePair") ParseGeometrySurfacePair(DefaultPart(), tag);
		else ParseUnknownTag(tag);

		++tag;
	} while (!tag.isend());

	// create a new instance
	FEBioInputModel& febio = GetFEBioModel();
	FEBioInputModel::Part* part = DefaultPart();
	part->Update();
	FEBioInputModel::PartInstance* instance = new FEBioInputModel::PartInstance(part);
	febio.AddInstance(instance);
	instance->SetName(part->GetName());

	return true;
}

//-----------------------------------------------------------------------------
bool FEBioFormat3::ParseMeshDomainsSection(XMLTag& tag)
{
	// make sure the section is not empty
	if (tag.isleaf()) return true;

	// loop over all sections
	++tag;
	do
	{
		if (tag == "SolidDomain")
		{
			if (ParseSolidDomainSection(tag) == false) return false;
		}
		else if (tag == "ShellDomain")
		{
			if (ParseShellDomainSection(tag) == false) return false;
		}
		else ParseUnknownTag(tag);
		++tag;
	} 
	while (!tag.isend());

	// don't forget to update the mesh
	GetFEBioModel().UpdateGeometry();

	// copy all mesh selections to named selections
	GetFEBioModel().CopyMeshSelections();

	return true;
}

bool FEBioFormat3::ParseGeometrySection(XMLTag& tag)
{
	// make sure the section is not empty
	if (tag.isleaf()) return true;

	// loop over all sections
	++tag;
	do
	{
		if      (tag == "Nodes"      ) ParseGeometryNodes      (DefaultPart(), tag);
		else if (tag == "Elements"   ) ParseGeometryElements   (DefaultPart(), tag);
		else if (tag == "NodeSet"    ) ParseGeometryNodeSet    (DefaultPart(), tag);
		else if (tag == "Surface"    ) ParseGeometrySurface    (DefaultPart(), tag);
		else if (tag == "ElementSet" ) ParseGeometryElementSet (DefaultPart(), tag);
		else if (tag == "DiscreteSet") ParseGeometryDiscreteSet(DefaultPart(), tag);
		else if (tag == "SurfacePair") ParseGeometrySurfacePair(DefaultPart(), tag);
		else ParseUnknownTag(tag);

		++tag;
	} while (!tag.isend());

	// create a new instance
	FEBioInputModel& febio = GetFEBioModel();
	FEBioInputModel::Part* part = DefaultPart();
	part->Update();
	FEBioInputModel::PartInstance* instance = new FEBioInputModel::PartInstance(part);
	febio.AddInstance(instance);
	instance->SetName(part->GetName());

	// don't forget to update the mesh
	GetFEBioModel().UpdateGeometry();

	// copy all mesh selections to named selections
	GetFEBioModel().CopyMeshSelections();

	return true;
}

//-----------------------------------------------------------------------------
bool FEBioFormat3::ParseSolidDomainSection(XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();

	FEBioInputModel::Part* part = DefaultPart();

	const char* szname = tag.AttributeValue("name");
	FEBioInputModel::Domain* dom = part->FindDomain(szname);
	if (dom == nullptr) return false;

	const char* szmat = tag.AttributeValue("mat", true);
	if (szmat)
	{
		int matID = febio.GetMaterialIndex(szmat);
		if (matID == -1) matID = atoi(szmat) - 1;
		dom->SetMatID(matID);
	}

	const char* eltype = tag.AttributeValue("elem_type", true);
	if (eltype && (strcmp(eltype, "ut4") == 0))
	{
		FESolidFormulation* eform = FEBio::CreateSolidFormulation("ut4-solid", &GetFSModel());
		ReadParameters(*eform, tag);
		dom->SetElementFormulation(eform);
		return true;
	}

	// see if any parameters are defined
	if (tag.isleaf()) return true;

	// three-field parameters
	bool is3field = false;
	bool blaugon = false;
	double atol = 0.0;
	int minaug = 0;
	int maxaug = 0;

	++tag;
	do
	{
		if (tag == "laugon")
		{
			tag.value(blaugon);
			is3field = true;
		}
		else if (tag == "atol")
		{
			tag.value(atol);
			is3field = true;
		}
		else if (tag == "minaug")
		{
			tag.value(minaug);
			is3field = true;
		}
		else if (tag == "maxaug")
		{
			tag.value(maxaug);
			is3field = true;
		}
		else ParseUnknownTag(tag);
		++tag;
	} 
	while (!tag.isend());

	if (is3field)
	{
		FESolidFormulation* eform = FEBio::CreateSolidFormulation("three-field-solid", &GetFSModel());
		eform->SetParamBool("laugon", blaugon);
		eform->SetParamFloat("atol", atol);
		eform->SetParamInt("minaug", minaug);
		eform->SetParamInt("maxaug", maxaug);
		dom->SetElementFormulation(eform);
	}

	return true;
}

bool FEBioFormat3::ParseShellDomainSection(XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();

	FEBioInputModel::Part* part = DefaultPart();

	const char* szname = tag.AttributeValue("name");
	FEBioInputModel::Domain* dom = part->FindDomain(szname);
	if (dom == nullptr) return false;

	const char* szmat = tag.AttributeValue("mat", true);
	if (szmat)
	{
		int matID = febio.GetMaterialIndex(szmat);
		if (matID == -1) matID = atoi(szmat) - 1;
		dom->SetMatID(matID);
	}

	// see if any parameters are defined
	if (tag.isleaf()) return true;

	// shell parameters
	bool is3field = false;
	bool laugon = false;
	double atol = 0.0;
	bool shellNodalNormals = false;
	double shellThickness = 0.0;
	int minaug = 0;
	int maxaug = 0;

	++tag;
	do
	{
		if (tag == "shell_normal_nodal")
		{
			tag.value(shellNodalNormals);
		}
		else if (tag == "shell_thickness")
		{
			tag.value(shellThickness);
		}
		else if (tag == "laugon")
		{
			tag.value(laugon);
			is3field = true;
		}
		else if (tag == "atol")
		{
			tag.value(atol);
			is3field = true;
		}
		else if (tag == "minaug")
		{
			tag.value(minaug);
			is3field = true;
		}
		else if (tag == "maxaug")
		{
			tag.value(maxaug);
			is3field = true;
		}
		else ParseUnknownTag(tag);
		++tag;
	}
	while (!tag.isend());

	FSModel* fem = &GetFSModel();

	FEElementFormulation* shell = dom->GetElementFormulation();
	if (shell == nullptr)
	{
		if (is3field)
		{
			FEShellFormulation* eform = FEBio::CreateShellFormulation("three-field-shell", fem);
			eform->SetParamBool("laugon", laugon);
			eform->SetParamFloat("atol", atol);
			eform->SetParamInt("minaug", minaug);
			eform->SetParamInt("maxaug", maxaug);
			eform->SetParamBool("shell_normal_nodal", shellNodalNormals);
			eform->SetParamFloat("shell_thickness", shellThickness);
			dom->SetElementFormulation(eform);
		}
		else
		{
			int nmat = dom->MatID();
			GMaterial* gmat = fem->GetMaterial(nmat);
			if (gmat && gmat->GetMaterialProperties())
			{
				FSMaterial* pm = gmat->GetMaterialProperties();
				FEShellFormulation* eform = nullptr;
				if (pm->IsRigid())
				{
					eform = FEBio::CreateShellFormulation("rigid-shell", fem);
					if (eform) eform->SetParamFloat("shell_thickness", shellThickness);
				}
				else
				{
					int baseClass = FEBio::GetBaseClassIndex("FEUncoupledMaterial");
					if (FEBio::HasBaseClass(pm, "FEUncoupledMaterial"))
					{
						eform = FEBio::CreateShellFormulation("three-field-shell", fem);
					}
					else eform = FEBio::CreateShellFormulation("elastic-shell", fem);

					eform->SetParamBool("shell_normal_nodal", shellNodalNormals);
					eform->SetParamFloat("shell_thickness", shellThickness);
				}

				if (eform)
				{
					dom->SetElementFormulation(eform);
				}
			}
		}
	}
	else
	{
		Param* p = shell->GetParam("shell_normal_nodal");
		if (p) p->SetBoolValue(shellNodalNormals);

		Param* ph = shell->GetParam("shell_thickness");
		if (ph) ph->SetFloatValue(shellThickness);
	}

	return true;
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseGeometryNodes(FEBioInputModel::Part* part, XMLTag& tag)
{
	if (part == 0) throw XMLReader::InvalidTag(tag);

	vector<FEBioInputModel::NODE> nodes; nodes.reserve(10000);

	// create a node set if the name is defined
	const char* szname = tag.AttributeValue("name", true);
	std::string name;
	if (szname) name = szname;
	if (szname) part->SetName(szname);

	// read nodal coordinates
	++tag;
	do
	{
		FEBioInputModel::NODE node;
		tag.value(node.r);
		int nid = tag.AttributeValue<int>("id", -1); assert(nid != -1);
		node.id = nid;

		nodes.push_back(node);
		++tag;
	} while (!tag.isend());

	// create nodes
	int nn = (int)nodes.size();
	FSMesh& mesh = *part->GetFEMesh();
	int N0 = mesh.Nodes();
	mesh.Create(N0 + nn, 0);

	for (int i = 0; i < nn; ++i)
	{
		FEBioInputModel::NODE& nd = nodes[i];
		FSNode& node = mesh.Node(N0 + i);
		node.m_ntag = nd.id;
		node.m_nid = nd.id;
		node.r = nd.r;
	}

	// create the nodeset 
/*	if (name.empty() == false)
	{
		vector<int> nodeList(nn);
		for (int i = 0; i < nn; ++i) nodeList[i] = nodes[i].id - 1;
		FEBioInputModel::NodeSet nset(name, nodeList);
		part->AddNodeSet(nset);
	}
*/
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseGeometryElements(FEBioInputModel::Part* part, XMLTag& tag)
{
	if (part == 0) throw XMLReader::InvalidTag(tag);

	// first we need to figure out how many elements there are
	int elems = tag.children();

	// get the required type attribute
	const char* szshell = nullptr;
	const char* sztype = tag.AttributeValue("type");
	FEElementType ntype = FE_INVALID_ELEMENT_TYPE;
	if      (strcmp(sztype, "hex8"  ) == 0) ntype = FE_HEX8;
	else if (strcmp(sztype, "hex20" ) == 0) ntype = FE_HEX20;
	else if (strcmp(sztype, "hex27" ) == 0) ntype = FE_HEX27;
	else if (strcmp(sztype, "penta6") == 0) ntype = FE_PENTA6;
	else if (strcmp(sztype, "tet4"  ) == 0) ntype = FE_TET4;
	else if (strcmp(sztype, "tet5"  ) == 0) ntype = FE_TET5;
	else if (strcmp(sztype, "tet10" ) == 0) ntype = FE_TET10;
	else if (strcmp(sztype, "tet15" ) == 0) ntype = FE_TET15;
	else if (strcmp(sztype, "tet20" ) == 0) ntype = FE_TET20;
	else if (strcmp(sztype, "quad4" ) == 0) ntype = FE_QUAD4;
	else if (strcmp(sztype, "quad8" ) == 0) ntype = FE_QUAD8;
	else if (strcmp(sztype, "quad9" ) == 0) ntype = FE_QUAD9;
	else if (strcmp(sztype, "tri3"  ) == 0) ntype = FE_TRI3;
	else if (strcmp(sztype, "tri6"  ) == 0) ntype = FE_TRI6;
	else if (strcmp(sztype, "pyra5") == 0) ntype = FE_PYRA5;
	else if (strcmp(sztype, "penta15") == 0) ntype = FE_PENTA15;
    else if (strcmp(sztype, "pyra13") == 0) ntype = FE_PYRA13;
	else if (strcmp(sztype, "TET10G4"     ) == 0) ntype = FE_TET10;
	else if (strcmp(sztype, "TET10G8"     ) == 0) ntype = FE_TET10;
	else if (strcmp(sztype, "TET10GL11"   ) == 0) ntype = FE_TET10;
	else if (strcmp(sztype, "TET10G4_S3"  ) == 0) ntype = FE_TET10;
	else if (strcmp(sztype, "TET10G8_S3"  ) == 0) ntype = FE_TET10;
	else if (strcmp(sztype, "TET10GL11_S3") == 0) ntype = FE_TET10;
	else if (strcmp(sztype, "TET10G4_S4"  ) == 0) ntype = FE_TET10;
	else if (strcmp(sztype, "TET10G8_S4"  ) == 0) ntype = FE_TET10;
	else if (strcmp(sztype, "TET10GL11_S4") == 0) ntype = FE_TET10;
	else if (strcmp(sztype, "TET10G4_S7"  ) == 0) ntype = FE_TET10;
	else if (strcmp(sztype, "TET10G8_S7"  ) == 0) ntype = FE_TET10;
	else if (strcmp(sztype, "TET10GL11_S7") == 0) ntype = FE_TET10;
	else if (strcmp(sztype, "TET15G8"     ) == 0) ntype = FE_TET15;
	else if (strcmp(sztype, "TET15G11"    ) == 0) ntype = FE_TET15;
	else if (strcmp(sztype, "TET15G15"    ) == 0) ntype = FE_TET15;
	else if (strcmp(sztype, "TET15G8_S3"  ) == 0) ntype = FE_TET15;
	else if (strcmp(sztype, "TET15G11_S3" ) == 0) ntype = FE_TET15;
	else if (strcmp(sztype, "TET15G15_S3" ) == 0) ntype = FE_TET15;
	else if (strcmp(sztype, "TET15G8_S4"  ) == 0) ntype = FE_TET15;
	else if (strcmp(sztype, "TET15G11_S4" ) == 0) ntype = FE_TET15;
	else if (strcmp(sztype, "TET15G15_S4" ) == 0) ntype = FE_TET15;
	else if (strcmp(sztype, "TET15G8_S7"  ) == 0) ntype = FE_TET15;
	else if (strcmp(sztype, "TET15G11_S7" ) == 0) ntype = FE_TET15;
	else if (strcmp(sztype, "TET15G15_S7" ) == 0) ntype = FE_TET15;
	else if (strcmp(sztype, "PENTA15G8"   ) == 0) ntype = FE_PENTA15;
	else if (strcmp(sztype, "HEX20G8"     ) == 0) ntype = FE_HEX20;
	else if (strcmp(sztype, "QUAD4G8"     ) == 0) ntype = FE_QUAD4;
	else if (strcmp(sztype, "QUAD4G12"    ) == 0) ntype = FE_QUAD4;
	else if (strcmp(sztype, "QUAD8G18"    ) == 0) ntype = FE_QUAD8;
	else if (strcmp(sztype, "QUAD8G27"    ) == 0) ntype = FE_QUAD8;
	else if (strcmp(sztype, "TRI3G6"      ) == 0) ntype = FE_TRI3;
	else if (strcmp(sztype, "TRI3G9"      ) == 0) ntype = FE_TRI3;
	else if (strcmp(sztype, "TRI6G14"     ) == 0) ntype = FE_TRI6;
	else if (strcmp(sztype, "TRI6G21"     ) == 0) ntype = FE_TRI6;
	else if (strcmp(sztype, "q4eas"       ) == 0) { ntype = FE_QUAD4; szshell = "elastic-shell-eas"; }
	else if (strcmp(sztype, "q4ans"       ) == 0) { ntype = FE_QUAD4; szshell = "elastic-shell-ans"; }
	else throw XMLReader::InvalidTag(tag);

	// get the optional material attribute
	const char* szmat = tag.AttributeValue("mat", true);
	int matID = -1;
	if (szmat)
	{
		FEBioInputModel& febio = GetFEBioModel();
		matID = febio.GetMaterialIndex(szmat);
		if (matID == -1) matID = atoi(szmat)-1;
	}

	// get the name attribute
	// (we also allow "elset", although "name" is the correct attribute)
	const char* szname = tag.AttributeValue("name", true);
	if (szname == 0) szname = tag.AttributeValue("elset", true);
	if (szname == 0) szname = "_no_name";

	// make sure no parts have the same name
	string name = szname;
	int n = 2;
	while (part->FindDomain(name))
	{
		if (n == 2) FileReader()->AddLogEntry("Part with name \"%s\" already defined.", szname);

		stringstream ss;
		ss << szname << "(" << n++ << ")";
		name = ss.str();
	}

	// add domain to list
	FEBioInputModel::Domain* dom = part->AddDomain(name, matID);
//	dom->m_bshellNodalNormals = GetFEBioModel().m_shellNodalNormals;

	if (szshell) dom->SetElementFormulation(FEBio::CreateShellFormulation(szshell, &GetFSModel()));

	// create elements
	FSMesh& mesh = *part->GetFEMesh();
	int NTE = mesh.Elements();
	mesh.Create(0, elems + NTE);

	// generate the part id
	int pid = part->Domains() - 1;

	// read element data
	++tag;
	vector<int> elemSet; elemSet.reserve(elems);
	for (int i = NTE; i<elems + NTE; ++i)
	{
		FSElement& el = mesh.Element(i);
		el.SetType(ntype);
		el.m_gid = pid;
		dom->AddElement(i);
		if ((tag == "e") || (tag == "elem"))
		{
			int id = tag.AttributeValue<int>("id", -1);
			el.m_nid = id;
			tag.value(el.m_node, el.Nodes());
			elemSet.push_back(id);
		}
		else throw XMLReader::InvalidTag(tag);

		++tag;
	}

	// create new element set
//	FEBioInputModel::ElementSet* set = new FEBioInputModel::ElementSet(szname, elemSet);
//	part->AddElementSet(*set);
}


//-----------------------------------------------------------------------------
void FEBioFormat3::ParseGeometryNodeSet(FEBioInputModel::Part* part, XMLTag& tag)
{
	// make sure there is a name attribute
	std::string name = tag.AttributeValue("name");

	// list to store node numbers
	vector<int> list;

	++tag;
	do
	{
		if ((tag == "node") || (tag == "n"))
		{
			int nid = tag.AttributeValue<int>("id", -1);
			if (nid == -1) throw XMLReader::MissingAttribute(tag, "id");
			list.push_back(nid);
		}
		else if (tag == "node_set")
		{
			const char* szset = tag.szvalue();
			if (part)
			{
				FEBioInputModel::NodeSet* ps = part->FindNodeSet(szset);
				if (ps == 0) throw XMLReader::InvalidValue(tag);
				list.insert(list.end(), ps->nodeList().begin(), ps->nodeList().end());
			}
		}
		else throw XMLReader::InvalidTag(tag);
		++tag;
	}
	while (!tag.isend());

	// create a new node set
	part->AddNodeSet(FEBioInputModel::NodeSet(name, list));
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseGeometryDiscreteSet(FEBioInputModel::Part* part, XMLTag& tag)
{
	if (tag.isempty()) return;
	if (part == 0) throw XMLReader::InvalidTag(tag);

	FEBioInputModel::DiscreteSet ds;
	const char* szname = tag.AttributeValue("name");
	ds.SetName(szname);
	ds.SetPart(part);

	++tag;
	do
	{
		if (tag == "delem")
		{
			int n[2];
			tag.value(n, 2);
			ds.Add(n[0], n[1]);
		}
		else ParseUnknownTag(tag);
		++tag;
	} while (!tag.isend());

	part->AddDiscreteSet(ds);
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseGeometrySurfacePair(FEBioInputModel::Part* part, XMLTag& tag)
{
	if (part == 0) throw XMLReader::InvalidTag(tag);

	std::string name = tag.AttributeValue("name");
	int surf2 = -1, surf1 = -1;
	++tag;
	do
	{
		if (tag == "secondary")
		{
			const char* szsurf = tag.szvalue();
			surf2 = part->FindSurfaceIndex(szsurf);
			if (surf2 == -1)
			{
				FileReader()->AddLogEntry("Invalid value for secondary: %s", szsurf);
			}
		}
		else if (tag == "primary")
		{
			const char* szsurf = tag.szvalue();
			surf1 = part->FindSurfaceIndex(szsurf);
			if (surf1 == -1)
			{
				FileReader()->AddLogEntry("Invalid value for primary: %s", szsurf);
			}
		}
		else throw XMLReader::InvalidTag(tag);
		++tag;
	}
	while (!tag.isend());

	part->AddSurfacePair(FEBioInputModel::SurfacePair(name, surf1, surf2));
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseGeometrySurface(FEBioInputModel::Part* part, XMLTag& tag)
{
	if (part == 0) throw XMLReader::InvalidTag(tag);

	// get the name
	const char* szname = tag.AttributeValue("name");

	// see if a surface with this name is already defined
	// if found, we'll continue, but we'll generate a warning.
	FEBioInputModel::Surface* ps = part->FindSurface(szname);
	if (ps) FileReader()->AddLogEntry("A surface named %s is already defined.", szname);

	// create a new surface
	FEBioInputModel::Surface s;
	s.m_name = szname;

	if (tag.isleaf() == false)
	{
		// read the surface data
		int nf[FSElement::MAX_NODES], N;
		++tag;
		do
		{
			// read the facet
			if (tag == "quad4") N = 4;
			else if (tag == "quad8") N = 8;
			else if (tag == "quad9") N = 9;
			else if (tag == "tri3") N = 3;
			else if (tag == "tri6") N = 6;
			else if (tag == "tri7") N = 7;
			else if (tag == "tri10") N = 10;
			else throw XMLReader::InvalidTag(tag);

			// read the node numbers
			tag.value(nf, N);

			// make zero-based
			vector<int> node(N);
			for (int j = 0; j < N; ++j) node[j] = nf[j];
			s.m_face.push_back(node);

			++tag;
		} while (!tag.isend());
	}

	part->AddSurface(s);
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseGeometryElementSet(FEBioInputModel::Part* part, XMLTag& tag)
{
	if (part == 0) throw XMLReader::InvalidTag(tag);

	// get the name
	const char* szname = tag.AttributeValue("name");
	if (szname == 0) FileReader()->AddLogEntry("Element set defined without a name.");
	string sname = (szname ? szname : "");

	// see if a set with this name is already defined
	// if found, we'll continue, but we'll generate a warning.
	FEBioInputModel::ElementSet* ps = part->FindElementSet(szname);
	if (ps) FileReader()->AddLogEntry("An element set named %s is already defined.", szname);

	vector<int> elem;
	++tag;
	do 
	{
		int eid = tag.AttributeValue<int>("id", -1);
		if (eid == -1) throw XMLReader::InvalidTag(tag);
		elem.push_back(eid);

		++tag;
	}
	while (!tag.isend());

	part->AddElementSet(FEBioInputModel::ElementSet(sname, elem));
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseGeometryPart(XMLTag& tag)
{
	const char* szname = tag.AttributeValue("name");
	if (szname == 0) throw XMLReader::InvalidAttributeValue(tag, "name", szname);

	// create a new object with this name
	FEBioInputModel& febio = GetFEBioModel();
	FEBioInputModel::Part* part = febio.AddPart(szname);

	// make sure we set the format flag in case of an error
	m_geomFormat = 2;

	++tag;
	do
	{
		if      (tag == "Nodes"      ) ParseGeometryNodes      (part, tag);
		else if (tag == "Elements"   ) ParseGeometryElements   (part, tag);
		else if (tag == "NodeSet"    ) ParseGeometryNodeSet    (part, tag);
		else if (tag == "Surface"    ) ParseGeometrySurface    (part, tag);
		else if (tag == "ElementSet" ) ParseGeometryElementSet (part, tag);
		else if (tag == "DiscreteSet") ParseGeometryDiscreteSet(part, tag);
		else if (tag == "SurfacePair") ParseGeometrySurfacePair(part, tag);
		else ParseUnknownTag(tag);
		++tag;
	}
	while (!tag.isend());

	// Update the part.
	// This essentially converts the global node ID to local IDs for elements
	// It is assumed that the node global ID is stored in its tag
	part->Update();
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseGeometryInstance(XMLTag& tag)
{
	// get the part name
	const char* szpart = tag.AttributeValue("part");

	// get the instance name
	const char* szname = tag.AttributeValue("name", true);
	if (szname == 0) szname = szpart;

	// see if the part exists
	FEBioInputModel& febio = GetFEBioModel();
	FEBioInputModel::Part* part = febio.FindPart(szpart);
	if (part == 0) throw XMLReader::InvalidAttributeValue(tag, "part", szpart);

	// create a new instance
	FEBioInputModel::PartInstance* instance = new FEBioInputModel::PartInstance(part);
	febio.AddInstance(instance);
	instance->SetName(szname);

	// read the transform info (if any)
	if (tag.isleaf() == false)
	{
		double d[4];
		++tag;
		do
		{
			if (tag == "scale")
			{
				tag.value(d, 3);
				instance->m_scl = vec3d(d[0], d[1], d[2]);
			}
			else if (tag == "translate")
			{
				tag.value(d, 3);
				instance->m_pos = vec3d(d[0], d[1], d[2]);
			}
			else if (tag == "rotate")
			{
				const char* sztype = tag.AttributeValue("type", true);
				if (sztype == 0) sztype = "quaternion";

				if (strcmp(sztype, "quaternion") == 0)
				{
					tag.value(d, 4);
					instance->m_rot = quatd(d[0], d[1], d[2], d[3]);
					instance->m_rot.MakeUnit();
				}
				else if (strcmp(sztype, "vector") == 0)
				{
					tag.value(d, 3);
					d[0] *= DEG2RAD;
					d[1] *= DEG2RAD;
					d[2] *= DEG2RAD;
					vec3d v(d[0], d[1], d[2]);
					double w = v.Length(); v.Normalize();
					instance->m_rot = quatd(w, v);
				}
				else if (strcmp(sztype, "Euler") == 0)
				{
					tag.value(d, 3);
					instance->m_rot.SetEuler(d[0], d[1], d[2]);
				}
				else ParseUnknownAttribute(tag, "type");
			}
			else ParseUnknownTag(tag);
			++tag;
		}
		while (!tag.isend());
	}	
}

//=============================================================================
//
//                                M E S H D A T A
//
//=============================================================================

//-----------------------------------------------------------------------------
//  Parses the mesh data section from the xml file
//
bool FEBioFormat3::ParseMeshDataSection(XMLTag& tag)
{
	if (tag.isleaf()) return true;

	++tag;
	do
	{
		if (tag == "ElementData")
		{
			if (ParseElementDataSection(tag) == false) return false;
		}
		else if (tag == "NodeData")
		{
			if (ParseNodeDataSection(tag) == false) return false;
		}
		else if (tag == "SurfaceData")
		{
			if (ParseSurfaceDataSection(tag) == false) return false;
		}
		else ParseUnknownTag(tag);
		++tag;
    }
	while (!tag.isend());

	// TODO: The shell thickness/fiber/mat axis data was read into the part's mesh, not the instance's mesh
	// This is a hack to copy that data from the part to the instance
	FEBioInputModel& febio = GetFEBioModel();
	for (int i=0; i<febio.Instances(); ++i)
	{
		FEBioInputModel::PartInstance* instance = febio.GetInstance(i);
		FSMesh* pdst = instance->GetMesh();
		FSMesh* psrc = instance->GetPart()->GetFEMesh();

		assert(pdst->Elements()==psrc->Elements());
		for (int j=0; j<pdst->Elements(); ++j)
		{
			FSElement& e0 = pdst->Element(j);
			FSElement& e1 = psrc->Element(j);

			int ne = e0.Nodes(); assert(ne == e1.Nodes());
			for (int k=0; k<ne; ++k)
			{
				e0.m_h[k] = e1.m_h[k];
			}
            e0.m_Q = e1.m_Q;
            e0.m_Qactive = e1.m_Qactive;
            e0.m_fiber = e1.m_fiber;
		}
	}

	return true;
}

bool FEBioFormat3::ParseNodeDataSection(XMLTag& tag)
{
	FEBioInputModel& feb = GetFEBioModel();

	XMLAtt* name = tag.AttributePtr("name");
	XMLAtt* dataTypeAtt = tag.AttributePtr("datatype");
	XMLAtt* nset = tag.AttributePtr("node_set");

	DATA_TYPE dataType;
	if (dataTypeAtt)
	{
		if      (*dataTypeAtt == "scalar") dataType = DATA_SCALAR;
		else if (*dataTypeAtt == "vec3"  ) dataType = DATA_VEC3;
		else return false;
	}
	else dataType = DATA_SCALAR;

	FSNodeSet* nodeSet = feb.FindNamedNodeSet(nset->cvalue());
	if (nodeSet)
	{
		FSMesh* feMesh = nodeSet->GetMesh();

		const char* szgen = tag.AttributeValue("generator", true);
		if (szgen)
		{
			FSModel* fem = &feb.GetFSModel();
			FSMeshDataGenerator* gen = FEBio::CreateNodeDataGenerator(szgen, fem);
			if (gen == nullptr)
			{
				tag.skip();
			}
			else
			{
				ParseModelComponent(gen, tag);
				gen->SetItemList(nodeSet);
				gen->SetName(name->cvalue());
				fem->AddMeshDataGenerator(gen);
			}
		}
		else
		{
			FENodeData* nodeData = feMesh->AddNodeDataField(name->cvalue(), nodeSet, dataType);
			++tag;
			do
			{
				int lid = -1;
				tag.AttributePtr("lid")->value(lid);
				switch (dataType)
				{
				case DATA_SCALAR: 
				{
					double val = 0.0;
					tag.value(val);
					nodeData->setScalar(lid - 1, val);
				}
				break;
				case DATA_VEC3:
				{
					vec3d val;
					tag.value(val);
					nodeData->setVec3d(lid - 1, val);
				}
				break;
				default:
					assert(false);
				}				

				++tag;
			} while (!tag.isend());
		}
	}
	else tag.skip();

	return true;

}

bool FEBioFormat3::ParseSurfaceDataSection(XMLTag& tag)
{
	FEBioInputModel& feb = GetFEBioModel();
	FSModel* fem = &feb.GetFSModel();

	XMLAtt* name = tag.AttributePtr("name");
	XMLAtt* dataTypeAtt = tag.AttributePtr("data_type");
	XMLAtt* surf = tag.AttributePtr("surface");

	DATA_TYPE dataType;
	if (dataTypeAtt)
	{
		if      (*dataTypeAtt == "scalar") dataType = DATA_TYPE::DATA_SCALAR;
		else if (*dataTypeAtt == "vector") dataType = DATA_TYPE::DATA_VEC3;
		else return false;
	}
	else dataType = DATA_TYPE::DATA_SCALAR;

	const char* szgen = tag.AttributeValue("generator", true);
	if (szgen)
	{
		if (strcmp(szgen, "const") == 0)
		{
			// "const" data generator needs to be handled differently
			DATA_TYPE dataType = DATA_TYPE::DATA_SCALAR;
			if (dataTypeAtt)
			{
				if      (*dataTypeAtt == "scalar") dataType = DATA_TYPE::DATA_SCALAR;
				else if (*dataTypeAtt == "vec3"  ) dataType = DATA_TYPE::DATA_VEC3;
				else return false;
			}
			FSConstFaceDataGenerator* gen = new FSConstFaceDataGenerator(fem, dataType);

			gen->SetName(name->cvalue());

			const char* szset = surf->cvalue();
			GMeshObject* po = feb.GetInstance(0)->GetGObject();
			FSSurface* ps = feb.FindNamedSurface(surf->cvalue());

			gen->SetItemList(ps);

			ParseModelComponent(gen, tag);
			fem->AddMeshDataGenerator(gen);
		}
		else
		{
			tag.skip();
		}
		return true;
	}

	FSSurface* feSurf = feb.FindNamedSurface(surf->cvalue());
	FSMesh* feMesh = feSurf->GetMesh();

	FESurfaceData* sd = feMesh->AddSurfaceDataField(name->cvalue(), feSurf, dataType);

	double val;
	int lid;
	++tag;
	do
	{
		tag.AttributePtr("lid")->value(lid);
		tag.value(val);

		(*sd)[lid - 1] = val;

		++tag;
	} while (!tag.isend());

	return true;
}

bool FEBioFormat3::ParseElementDataSection(XMLTag& tag)
{
	XMLAtt* var = tag.AttributePtr("var");
	if (var && (*var == "shell thickness"))
	{
		const char* szset = tag.AttributeValue("elem_set");
		FEBioInputModel& feb = GetFEBioModel();

		FEBioInputModel::Domain* dom = feb.FindDomain(szset);
		if (dom)
		{
			FSMesh* mesh = dom->GetPart()->GetFEMesh();

			double h[FSElement::MAX_NODES] = { 0 };
			++tag;
			do
			{
				int m = tag.value(h, FSElement::MAX_NODES);
				int lid = tag.AttributeValue<int>("lid", 0) - 1;
				if (lid >= 0)
				{
					int id = dom->ElementID(lid);
					FSElement& el = mesh->Element(id);

					assert(m == el.Nodes());
					for (int i = 0; i < m; ++i) el.m_h[i] = h[i];
				}
				++tag;
			} while (!tag.isend());
		}
		else ParseUnknownTag(tag);
	}
	else if (var && (*var == "fiber"))
	{
		const char* szset = tag.AttributeValue("elem_set");
		FEBioInputModel& feb = GetFEBioModel();

		FEBioInputModel::Domain* dom = feb.FindDomain(szset);
		if (dom)
		{
			FSMesh* mesh = dom->GetPart()->GetFEMesh();

			vec3d a;
			++tag;
			do
			{
				int lid = tag.AttributeValue<int>("lid", 0) - 1;
				if (lid >= 0)
				{
					int id = dom->ElementID(lid);
					FSElement& el = mesh->Element(id);
					tag.value(a);
					a.Normalize();
					// set up a orthonormal coordinate system
					vec3d b(0, 1, 0);
					if (fabs(fabs(a*b) - 1) < 1e-7) b = vec3d(0, 0, 1);
					vec3d c = a ^ b;
					b = c ^ a;
					// make sure they are unit vectors
					b.Normalize();
					c.Normalize();
					el.m_Q = mat3d(a.x, b.x, c.x,
						a.y, b.y, c.y,
						a.z, b.z, c.z);
					el.m_fiber = a;
				}
				++tag;
			} while (!tag.isend());
		}
		else ParseUnknownTag(tag);
	}
	else if (var && (*var == "mat_axis"))
	{
		const char* szset = tag.AttributeValue("elem_set");
		FEBioInputModel& feb = GetFEBioModel();

		FEBioInputModel::Domain* dom = feb.FindDomain(szset);
		if (dom)
		{
			FSMesh* mesh = dom->GetPart()->GetFEMesh();

			vec3d a, b, c, d;
			++tag;
			do
			{
				int lid = tag.AttributeValue<int>("lid", 0) - 1;
				if (lid >= 0)
				{
					int id = dom->ElementID(lid);
					FSElement& el = mesh->Element(id);

					++tag;
					do
					{
						if (tag == "a") tag.value(a);
						else if (tag == "d") tag.value(d);
						++tag;
					} while (!tag.isend());
					a.Normalize();
					c = a ^ d; c.Normalize();
					b = c ^ a; b.Normalize();
					el.m_Q = mat3d(a.x, b.x, c.x,
						a.y, b.y, c.y,
						a.z, b.z, c.z);
					el.m_Qactive = true;
				}
				++tag;
			} while (!tag.isend());
		}
		else ParseUnknownTag(tag);
	}
	else if (var)
	{
		const char* szgen = tag.AttributeValue("generator", true);
		if (szgen)
		{
			// Read the data and store it as a mesh data section
			FEBioInputModel& feb = GetFEBioModel();
			FSModel& fem = feb.GetFSModel();


			// TODO: add support for this
			ParseUnknownTag(tag);

/*			const char* szset = tag.AttributeValue("elem_set");
			if (strcmp(szgen, "surface-to-surface map") == 0)
			{
				FESurfaceToSurfaceMap* s2s = new FESurfaceToSurfaceMap;
				s2s->m_generator = szgen;
				s2s->m_var = var->cvalue();
				s2s->m_elset = szset;

				// get the name
				const char* szname = tag.AttributeValue("name", true);
				string sname;
				if (szname == nullptr)
				{
					stringstream ss;
					ss << "DataMap" << fem.DataMaps() + 1;
					sname = ss.str();
				}
				else sname = szname;
				s2s->SetName(sname);

				string tmp;
				++tag;
				do
				{
					if (tag == "bottom_surface") { tag.value(tmp); s2s->SetBottomSurface(tmp); }
					else if (tag == "top_surface") { tag.value(tmp); s2s->SetTopSurface(tmp); }
					else if (tag == "function")
					{
						Param* p = s2s->GetParam("function"); assert(p);

						const char* szlc = tag.AttributeValue("lc", true);
						if (szlc)
						{
							int lc = atoi(szlc);
							GetFEBioModel().AddParamCurve(p, lc - 1);

							double v = 0.0;
							tag.value(v);
							p->SetFloatValue(v);
						}

						if (tag.isleaf() == false)
						{
							LoadCurve lc; lc.Clear();
							++tag;
							do {
								if (tag == "points")
								{
									// read the points
									++tag;
									do
									{
										double d[2];
										tag.value(d, 2);
										lc.Add(d[0], d[1]);

										++tag;
									} while (!tag.isend());
								}
								else ParseUnknownTag(tag);
								++tag;
							} while (!tag.isend());
							p->SetLoadCurve(lc);
						}
					}
					else ParseUnknownTag(tag);
					++tag;
				} while (!tag.isend());

				feb.GetFSModel().AddDataMap(s2s);
			}
		*/
		}
		else ParseUnknownTag(tag);
	}
	else if (var == nullptr)
	{
		FEBioInputModel& feb = GetFEBioModel();

		XMLAtt* name = tag.AttributePtr("name");
		XMLAtt* dataTypeAtt = tag.AttributePtr("datatype");
		XMLAtt* set = tag.AttributePtr("elem_set");

		DATA_TYPE dataType;
		if (dataTypeAtt)
		{
			if      (*dataTypeAtt == "scalar") dataType = DATA_TYPE::DATA_SCALAR;
			else if (*dataTypeAtt == "vec3"  ) dataType = DATA_TYPE::DATA_VEC3;
			else if (*dataTypeAtt == "mat3"  ) dataType = DATA_TYPE::DATA_MAT3;
			else return false;
		}
		else dataType = DATA_TYPE::DATA_SCALAR;

		GObject* po = feb.GetInstance(0)->GetGObject();
		FSMesh* mesh = po->GetFEMesh();

		string sname;
		if (name) sname = name->cvalue();
		else
		{
			int n = mesh->MeshDataFields();
			stringstream ss;
			ss << "MeshData" << n + 1;
			sname = ss.str();
		}

		FEMeshData* meshData = nullptr;

		FSElemSet* pg = feb.FindNamedElementSet(set->cvalue());
		if (pg == nullptr)
		{
			// we didn't find a named selection, but it could be a domain
			FEBioInputModel::Domain* dom = feb.FindDomain(set->cvalue());
			if (dom == nullptr)
			{
				throw XMLReader::InvalidAttributeValue(tag, "elem_set", set->cvalue());
			}

			// okay, let's build a part set for this then instead
			GPart* pg = po->FindPartFromName(set->cvalue());
			FSPartSet* partSet = new FSPartSet(po);
			partSet->SetName(sname);
			po->AddFEPartSet(partSet);
			partSet->add(pg->GetLocalID());

			meshData = mesh->AddPartDataField(sname, partSet, dataType);
		}
		else
		{
			meshData = mesh->AddElementDataField(sname, pg, dataType);
		}

		if (dataType == DATA_SCALAR)
		{
			double val;
			int lid;
			++tag;
			do
			{
				tag.AttributePtr("lid")->value(lid);
				tag.value(val);

				meshData->set(lid - 1, val);

				++tag;
			} while (!tag.isend());
		}
		else if (dataType == DATA_VEC3)
		{
			vec3d val;
			int lid;
			++tag;
			do
			{
				tag.AttributePtr("lid")->value(lid);
				tag.value(val);
				meshData->set(lid - 1, val);
				++tag;
			} while (!tag.isend());
		}
		else if (dataType == DATA_MAT3)
		{
			mat3d val;
			int lid;
			++tag;
			do
			{
				tag.AttributePtr("lid")->value(lid);
				tag.value(val);
				meshData->set(lid - 1, val);
				++tag;
			} while (!tag.isend());
		}
	}
	else ParseUnknownTag(tag);

	return true;
}

//=============================================================================
//
//                            M E S H   A D A P T O R
//
//=============================================================================

bool FEBioFormat3::ParseMeshAdaptorSection(XMLTag& tag)
{
	if (tag.isempty()) return true;

	FEBioInputModel& feb = GetFEBioModel();
	FSModel* fem = &GetFSModel();
	GModel* gm = &fem->GetModel();

	++tag;
	do {
		if (tag == "mesh_adaptor")
		{
			const char* szname = tag.AttributeValue("name", true);
			const char* sztype = tag.AttributeValue("type");

			FSMeshAdaptor* mda = FEBio::CreateMeshAdaptor(sztype, fem);
			if (mda == nullptr) throw XMLReader::InvalidAttributeValue(tag, "type", sztype);

			if (szname) mda->SetName(szname);
			else
			{
				stringstream ss;
				ss << "MeshAdaptor" << CountMeshAdaptors<FSMeshAdaptor>(*fem) + 1;
				mda->SetName(ss.str());
			}

			const char* szset = tag.AttributeValue("elem_set", true);
			if (szset)
			{
				GPart* pg = feb.FindGPart(szset);
				if (pg)
				{
					GPartList* partList = new GPartList(gm);
					partList->add(pg->GetID());
					mda->SetItemList(partList);
				}
				else AddLogEntry("Failed to find element set %s", szset);
			}
			m_pBCStep->AddComponent(mda);

			ParseModelComponent(mda, tag);
		}
		else ParseUnknownTag(tag);
		++tag;
	}
	while (!tag.isend());

	return true;
}


//=============================================================================
//
//                                B O U N D A R Y 
//
//=============================================================================

//-----------------------------------------------------------------------------
//  Parses the boundary section from the xml file (format 2.5)
bool FEBioFormat3::ParseBoundarySection(XMLTag& tag)
{
	if (tag.isleaf()) return true;

	++tag;
	do
	{
		if (tag == "bc")
		{
			string type = tag.AttributeValue("type");
			if      (type == "fix"       ) ParseBCFixed(m_pBCStep, tag);
			else if (type == "prescribe" ) ParseBCPrescribed(m_pBCStep, tag);
			else if (type == "rigid"     ) ParseBCRigid(m_pBCStep, tag);
			else if (type == "fluid rotational velocity") ParseBCFluidRotationalVelocity(m_pBCStep, tag);
			else if (type == "normal displacement") ParseBCNormalDisplacement(m_pBCStep, tag);
			else if (type == "linear constraint") ParseBCLinearConstraint(m_pBCStep, tag);
			else ParseUnknownTag(tag);
		}
		else ParseUnknownTag(tag);
		++tag;
	}
	while (!tag.isend());

	return true;
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseBCFixed(FSStep* pstep, XMLTag &tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// get the name attribute
	string name;
	const char* szname = tag.AttributeValue("name", true);
	if (szname) name = szname;

	// get the node set
	const char* szset = tag.AttributeValue("node_set");
	FEItemListBuilder* pg = febio.FindNamedSelection(szset);
	if (pg == 0) FileReader()->AddLogEntry("Cannot find node_set \"%s\"", szset);

	string dofList;
    vector<string> dofs;
	++tag;
	do
	{
		if (tag == "dofs")
		{
			// figure out the bc value
			tag.value(dofList);
            dofs = GetDOFList(dofList);
            for (int i=0; i<dofs.size(); ++i)
                if (validate_dof(dofs[i]) == false) throw XMLReader::InvalidValue(tag);
		}
		++tag;
	}
	while (!tag.isend());

	// create the constraint
    char szbuf[256] = { 0 };
    string bc = dofs[0];
    if ((bc=="x") || (bc=="y") || (bc=="z")) {
        {
            FSFixedDisplacement* pbc = new FSFixedDisplacement(&fem, pg, GetDOFDir(dofs), pstep->GetID());
            if (name.empty())
            {
                sprintf(szbuf, "FixedDisplacement%02d", CountBCs<FSFixedDisplacement>(fem) + 1);
                name = szbuf;
            }
            pbc->SetName(name);
            pstep->AddComponent(pbc);
        }
    }
    else if ((bc=="u") || (bc=="v") || (bc=="w"))
    {
        FSFixedRotation* pbc = new FSFixedRotation(&fem, pg, GetROTDir(dofs), pstep->GetID());
        if (name.empty())
        {
            sprintf(szbuf, "FixedRotation%02d", CountBCs<FSFixedRotation>(fem) + 1);
            name = szbuf;
        }
        pbc->SetName(name);
        pstep->AddComponent(pbc);
    }
    else if (bc == "T")
    {
        FSFixedTemperature* pbc = new FSFixedTemperature(&fem, pg, 1, pstep->GetID());
        if (name.empty())
        {
            sprintf(szbuf, "FixedTemperature%02d", CountBCs<FSFixedTemperature>(fem) + 1);
            name = szbuf;
        }
        pbc->SetName(name);
        pstep->AddComponent(pbc);
    }
    else if (bc == "p")
    {
        FSFixedFluidPressure* pbc = new FSFixedFluidPressure(&fem, pg, 1, pstep->GetID());
        if (name.empty())
        {
            sprintf(szbuf, "FixedFluidPressure%02d", CountBCs<FSFixedFluidPressure>(fem) + 1);
            name = szbuf;
        }
        pbc->SetName(name);
        pstep->AddComponent(pbc);
    }
    else if ((bc=="wx") || (bc=="wy") || (bc=="wz"))
    {
        FSFixedFluidVelocity* pbc = new FSFixedFluidVelocity(&fem, pg, GetDOFDir(dofs), pstep->GetID());
        if (name.empty())
        {
            sprintf(szbuf, "FixedFluidVelocity%02d", CountBCs<FSFixedFluidVelocity>(fem) + 1);
            name = szbuf;
        }
        pbc->SetName(name);
        pstep->AddComponent(pbc);
    }
    else if (bc == "ef")
    {
        FSFixedFluidDilatation* pbc = new FSFixedFluidDilatation(&fem, pg, 1, pstep->GetID());
        if (name.empty())
        {
            sprintf(szbuf, "FixedFluidDilatation%02d", CountBCs<FSFixedFluidDilatation>(fem) + 1);
            name = szbuf;
        }
        pbc->SetName(name);
        pstep->AddComponent(pbc);
    }
    else if ((bc=="gx") || (bc=="gy") || (bc=="gz"))
    {
        FSFixedFluidAngularVelocity* pbc = new FSFixedFluidAngularVelocity(&fem, pg, GetDOFDir(dofs), pstep->GetID());
        if (name.empty())
        {
            sprintf(szbuf, "FixedFluidAngularVelocity%02d", CountBCs<FSFixedFluidAngularVelocity>(fem) + 1);
            name = szbuf;
        }
        pbc->SetName(name);
        pstep->AddComponent(pbc);
    }
    else if ((bc=="sx") || (bc=="sy") || (bc=="sz"))
    {
        FSFixedShellDisplacement* pbc = new FSFixedShellDisplacement(&fem, pg, GetDOFDir(dofs), pstep->GetID());
        if (name.empty())
        {
            sprintf(szbuf, "FixedShellDisplacement%02d", CountBCs<FSFixedShellDisplacement>(fem) + 1);
            name = szbuf;
        }
        pbc->SetName(name);
        pstep->AddComponent(pbc);
    }
    else if (bc=="c")
    {
        FSFixedConcentration* pbc = new FSFixedConcentration(&fem, pg, 1, pstep->GetID());
        if (name.empty())
        {
            sprintf(szbuf, "FixedConcentration%02d", CountBCs<FSFixedConcentration>(fem) + 1);
            name = szbuf;
        }
        pbc->SetName(name);
        pstep->AddComponent(pbc);
    }
    else if (bc.compare(0,1,"c") == 0)
    {
        int isol = 0;
        sscanf(bc.substr(1).c_str(),"%d",&isol);
        if (isol > 0)
        {
            FSFixedConcentration* pbc = new FSFixedConcentration(&fem, pg, isol, pstep->GetID());
            if (name.empty())
            {
                sprintf(szbuf, "FixedConcentration%02d", CountBCs<FSFixedConcentration>(fem) + 1);
                name = szbuf;
            }
            pbc->SetName(name);
            pstep->AddComponent(pbc);
        }
    }
}

//-----------------------------------------------------------------------------

void FEBioFormat3::ParseBCPrescribed(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// get the name attribute
	string name;
	const char* szname = tag.AttributeValue("name", true);
	if (szname) name = szname;

	XMLAtt& set = tag.Attribute("node_set");
	FEItemListBuilder* pg = febio.FindNamedSelection(set.cvalue());
	if (pg == 0) FileReader()->AddLogEntry("Cannot find node_set \"%s\"", set.cvalue());

	string bc;
	bool relative = false;
	string scaleType("");
	string scaleValue("");
	int lc = -1;
	++tag;
	do
	{
		if (tag == "dof")
		{
			// determine bc
			bc = string(tag.szvalue());
			if (validate_dof(bc) == false) throw XMLReader::InvalidValue(tag);
		}
		else if (tag == "scale")
		{
			const char* sztype = tag.AttributeValue("type", true);
			if (sztype) scaleType = sztype;
			scaleValue = tag.szvalue();
			lc = tag.AttributeValue<int>("lc", -1);

			if (tag.isleaf() == false)
			{
				++tag;
				do {
					if (tag == "math") scaleValue = tag.szvalue();
					++tag;
				} while (!tag.isend());
			}
		}
		else if (tag == "relative") tag.value(relative);
		++tag;
	}
	while (!tag.isend());

	// make a new boundary condition
	FSPrescribedDOF* pbc = 0;
    if (bc=="x") pbc = new FSPrescribedDisplacement (&fem, pg, 0, 1, pstep->GetID());
    else if (bc=="y") pbc = new FSPrescribedDisplacement (&fem, pg, 1, 1, pstep->GetID());
    else if (bc=="z") pbc = new FSPrescribedDisplacement (&fem, pg, 2, 1, pstep->GetID());
    else if (bc=="T") pbc = new FSPrescribedTemperature  (&fem, pg, 1, pstep->GetID());
    else if (bc=="p") pbc = new FSPrescribedFluidPressure(&fem, pg, 1, pstep->GetID());
    else if (bc=="wx") pbc = new FSPrescribedFluidVelocity(&fem, pg, 0, 1, pstep->GetID());
    else if (bc=="wy") pbc = new FSPrescribedFluidVelocity(&fem, pg, 1, 1, pstep->GetID());
    else if (bc=="wz") pbc = new FSPrescribedFluidVelocity(&fem, pg, 2, 1, pstep->GetID());
    else if (bc=="ef") pbc = new FSPrescribedFluidDilatation(&fem, pg, 1, pstep->GetID());
    else if (bc=="sx") pbc = new FSPrescribedShellDisplacement(&fem, pg, 0, 1, pstep->GetID());
    else if (bc=="sy") pbc = new FSPrescribedShellDisplacement(&fem, pg, 1, 1, pstep->GetID());
    else if (bc=="sz") pbc = new FSPrescribedShellDisplacement(&fem, pg, 2, 1, pstep->GetID());
    else if (bc=="u") pbc = new FSPrescribedRotation(&fem, pg, 0, 1, pstep->GetID());
    else if (bc=="v") pbc = new FSPrescribedRotation(&fem, pg, 1, 1, pstep->GetID());
    else if (bc=="w") pbc = new FSPrescribedRotation(&fem, pg, 2, 1, pstep->GetID());
    else if (bc=="gx") pbc = new FSPrescribedFluidAngularVelocity(&fem, pg, 0, 1, pstep->GetID());
    else if (bc=="gy") pbc = new FSPrescribedFluidAngularVelocity(&fem, pg, 1, 1, pstep->GetID());
    else if (bc=="gz") pbc = new FSPrescribedFluidAngularVelocity(&fem, pg, 2, 1, pstep->GetID());
    else if (bc.compare(0,1,"c") == 0) {
        int isol;
        sscanf(bc.substr(1).c_str(),"%d",&isol);
        pbc = new FSPrescribedConcentration(&fem, pg, isol-1, 1.0, pstep->GetID());
    }

	// get the optional name
	if (name.empty()) name = pg->GetName();
	pbc->SetName(name);
	pstep->AddComponent(pbc);

	// process scale value
	Param* pp = pbc->GetParam("scale"); assert(pp);
	if (scaleType == "math")
	{
		pp->SetParamType(Param_MATH);
		pp->SetMathString(scaleValue);
	}
	else if (scaleType == "map")
	{
		pp->SetParamType(Param_STRING);
		pp->SetStringValue(scaleValue);
	}
	else
	{
		double s = atof(scaleValue.c_str());
		pp->SetParamType(Param_FLOAT);
		pp->SetFloatValue(s);
	}

	pbc->SetRelativeFlag(relative);
	
	if (lc != -1) febio.AddParamCurve(&pbc->GetParam(FSPrescribedDOF::SCALE), lc - 1);
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseBCRigid(FSStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();

	// read the name attribute
	string name;
	const char* sz = tag.AttributeValue("name", true);
	if (sz == 0)
	{
		char szbuf[256] = { 0 };
		sprintf(szbuf, "RigidInterface%02d", CountInterfaces<FSRigidInterface>(fem)+1);
	}
	else name = string(sz);

	// read node set
	const char* szset = tag.AttributeValue("node_set");
	FEBioInputModel& febio = GetFEBioModel();
	FEItemListBuilder* pg = febio.FindNamedSelection(szset);

	GMaterial* pmat = 0;
	++tag;
	do
	{
		if (tag == "rb")
		{
			// read rigid material ID
			int nrb = -1;
			tag.value(nrb);
			if ((nrb > 0) && (nrb <= febio.Materials())) pmat = febio.GetMaterial(nrb - 1);
			else FileReader()->AddLogEntry("Invalid material in rigid contact.");
		}
		else ParseUnknownTag(tag);
		++tag;
	}
	while (!tag.isend());

	// create the interface
	FSRigidInterface* pi = new FSRigidInterface(&fem, pmat, pg, pstep->GetID());
	pi->SetName(name.c_str());
	pstep->AddComponent(pi);
}

// TODO: The fluid-rotational velocity is a BC in FEBio, but a surface load in FEBioStudio.
//       Need to create proper boundary condition class for this component.
void FEBioFormat3::ParseBCFluidRotationalVelocity(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	std::string comment = tag.comment();

	// read the (optional) name
	string name = tag.AttributeValue("name", "");

	// find the surface
	XMLAtt& nodeSet = tag.Attribute("node_set");
	FEItemListBuilder* psurf = febio.FindNamedSelection(nodeSet.cvalue());
	if (psurf == 0)
	{
		AddLogEntry("Failed creating selection for fluid rotational velocity \"%s\"", name.c_str());
	}

	// create the surface load
	FSSurfaceLoad* psl = nullptr;
	XMLAtt& att = tag.Attribute("type");
    if (att == "fluid rotational velocity"     ) psl = CREATE_SURFACE_LOAD(FSFluidRotationalVelocity);
	else ParseUnknownAttribute(tag, "type");

	if (psurf) psl->SetItemList(psurf);

	// process surface load
	if (psl)
	{
		// read the parameters
		ReadParameters(*psl, tag);

		// set the name
		if (name.empty() == false) psl->SetName(name);

		// assign the surface
		psl->SetItemList(psurf);

		// set the comment
		psl->SetInfo(comment);

		// add to the step
		pstep->AddComponent(psl);
	}
}

void FEBioFormat3::ParseBCNormalDisplacement(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	std::string comment = tag.comment();

	// read the name attribute
	string name;
	const char* sz = tag.AttributeValue("name", true);
	if (sz == 0)
	{
		char szbuf[256] = { 0 };
		sprintf(szbuf, "NormalDisplacement%02d", CountBCs<FSNormalDisplacementBC>(fem) + 1);
		name = szbuf;
	}
	else name = string(sz);

	// find the surface
	XMLAtt& surfAtt = tag.Attribute("surface");
	FEItemListBuilder* psurf = febio.FindNamedSurface(surfAtt.cvalue());
	if (psurf == 0)
	{
		AddLogEntry("Failed creating selection for normal displacement\"%s\"", name.c_str());
	}

	// create the surface load
	FSBoundaryCondition* pbc = new FSNormalDisplacementBC(&fem);
	if (pbc)
	{
		// read the parameters
		ReadParameters(*pbc, tag);

		// set the name
		if (name.empty() == false) pbc->SetName(name);

		// assign the surface
		if (psurf) pbc->SetItemList(psurf);

		// set the comment
		pbc->SetInfo(comment);

		// add to the step
		pstep->AddComponent(pbc);
	}
}

void FEBioFormat3::ParseBCLinearConstraint(FSStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();
	std::string comment = tag.comment();

	// read the name attribute
	string name;
	const char* sz = tag.AttributeValue("name", true);
	if (sz == 0)
	{
		char szbuf[256] = { 0 };
		sprintf(szbuf, "LinearConstraint%02d", CountBCsByTypeString("linear constraint", fem) + 1);
		name = szbuf;
	}
	else name = string(sz);

	FSBoundaryCondition* pbc = FEBio::CreateBoundaryCondition("linear constraint", &fem);
	pbc->SetName(name);
	pbc->SetInfo(comment);

	pstep->AddComponent(pbc);

	ParseModelComponent(pbc, tag);
}

//=============================================================================
//
//                                R I G I D
//
//=============================================================================

//-----------------------------------------------------------------------------
FSRigidConstraint* createNewRigidConstraint(FSRigidConstraint* prc, const char* szclass, int N)
{
	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "%s%d", szclass + 2, N + 1);
	prc->SetName(szname);
	return prc;
}

#define CREATE_RIGID_CONSTRAINT(className) dynamic_cast<className*>(createNewRigidConstraint(new className(&fem), #className, CountRigidConstraints<className>(fem)))

//-----------------------------------------------------------------------------
bool FEBioFormat3::ParseRigidSection(XMLTag& tag)
{
	// make sure the section is not empty
	if (tag.isleaf()) return true;

	++tag;
	do
	{
		if (tag == "rigid_constraint")
		{
			ParseRigidConstraint(m_pBCStep, tag);
		}
		else if (tag == "rigid_connector")
		{ 
			const char* sztype = tag.AttributeValue("type");
			if      (strcmp(sztype, "rigid spherical joint"  ) == 0) ParseRigidConnector(m_pBCStep, tag, 0);
			else if (strcmp(sztype, "rigid revolute joint"   ) == 0) ParseRigidConnector(m_pBCStep, tag, 1);
			else if (strcmp(sztype, "rigid prismatic joint"  ) == 0) ParseRigidConnector(m_pBCStep, tag, 2);
			else if (strcmp(sztype, "rigid cylindrical joint") == 0) ParseRigidConnector(m_pBCStep, tag, 3);
			else if (strcmp(sztype, "rigid planar joint"     ) == 0) ParseRigidConnector(m_pBCStep, tag, 4);
			else if (strcmp(sztype, "rigid lock"             ) == 0) ParseRigidConnector(m_pBCStep, tag, 5);
			else if (strcmp(sztype, "rigid spring"           ) == 0) ParseRigidConnector(m_pBCStep, tag, 6);
			else if (strcmp(sztype, "rigid damper"           ) == 0) ParseRigidConnector(m_pBCStep, tag, 7);
			else if (strcmp(sztype, "rigid angular damper"   ) == 0) ParseRigidConnector(m_pBCStep, tag, 8);
			else if (strcmp(sztype, "rigid contractile force") == 0) ParseRigidConnector(m_pBCStep, tag, 9);
			else if (strcmp(sztype, "generic rigid joint"    ) == 0) ParseRigidConnector(m_pBCStep, tag, 10);
			else if (strcmp(sztype, "rigid joint"            ) == 0) ParseRigidJoint(m_pBCStep, tag);
		}
		else ParseUnknownTag(tag);

		++tag;
	}
	while (!tag.isend());

	return true;
}

//=============================================================================
//
//                                L O A D S
//
//=============================================================================

//-----------------------------------------------------------------------------
//!  Parses the loads section from the xml file
bool FEBioFormat3::ParseLoadsSection(XMLTag& tag)
{
	if (tag.isleaf()) return true;
	++tag;
	do
	{
		if      (tag == "nodal_load"  ) ParseNodeLoad   (m_pBCStep, tag);
		else if (tag == "surface_load") ParseSurfaceLoad(m_pBCStep, tag);
		else if (tag == "body_load"   ) ParseBodyLoad   (m_pBCStep, tag);
		else ParseUnknownTag(tag);
		++tag;
	} while (!tag.isend());

	return true;
}

//-----------------------------------------------------------------------------
//! Parses the nodal_load section.
void FEBioFormat3::ParseNodeLoad(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// get the load curve ID
	XMLAtt& aset = tag.Attribute("node_set");

	// create the node set
	FEItemListBuilder* pg = febio.FindNamedSelection(aset.cvalue());
	if (pg == 0) throw XMLReader::InvalidAttributeValue(tag, aset);

	// get the (optional) name
	string name;
	const char* szname = tag.AttributeValue("name", true);
	if (szname == nullptr)
	{
		char szname[256];
		sprintf(szname, "ForceNodeset%02d", CountLoads<FSNodalLoad>(fem) + 1);
		name = szname;
	}
	else name = szname;

	// create the nodal load
	FSNodalDOFLoad* pbc = new FSNodalDOFLoad(&fem, pg, 0, 1, pstep->GetID());
	pbc->SetName(name);
	pstep->AddComponent(pbc);

	// assign nodes to node sets
	++tag;
	do
	{
		if (tag == "scale")
		{
			ReadParam(*pbc, tag);
		}
		else if (tag == "dof")
		{
			// read the bc attribute
			string abc = tag.szvalue();
			int bc = 0;
			if      (abc == "x") bc = 0;
			else if (abc == "y") bc = 1;
			else if (abc == "z") bc = 2;
			else if (abc == "sx") bc = 3;
			else if (abc == "sy") bc = 4;
			else if (abc == "sz") bc = 5;
			else if (abc == "p") bc = 6;
			else if (abc.compare(0,1,"c") == 0) {
                int isol = 0;
                sscanf(abc.substr(1).c_str(),"%d",&isol);
                bc = isol+6;
            }
			else throw XMLReader::InvalidValue(tag);

			pbc->SetDOF(bc);
		}
		else ParseUnknownTag(tag);
		++tag;
	}
	while (!tag.isend());
}

//-----------------------------------------------------------------------------
//! Parses the surface_load section.
void FEBioFormat3::ParseSurfaceLoad(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	std::string comment = tag.comment();

	// find the surface
	XMLAtt& surf = tag.Attribute("surface");
	FSSurface* psurf = febio.FindNamedSurface(surf.cvalue());
	if (psurf == 0) throw XMLReader::InvalidAttributeValue(tag, surf);

	// read the (optional) name
	string name = tag.AttributeValue("name", "");

	// create the surface load
	FSSurfaceLoad* psl = nullptr;
	XMLAtt& att = tag.Attribute("type");
	if      (att == "pressure"                      ) psl = CREATE_SURFACE_LOAD(FSPressureLoad);
	else if (att == "traction"                      ) psl = CREATE_SURFACE_LOAD(FSSurfaceTraction);
    else if (att == "force"                         ) psl = CREATE_SURFACE_LOAD(FSSurfaceForceUniform);
    else if (att == "bearing load"                  ) psl = CREATE_SURFACE_LOAD(FSBearingLoad);
	else if (att == "fluidflux"                     ) psl = CREATE_SURFACE_LOAD(FSFluidFlux);
	else if (att == "soluteflux"                    ) psl = CREATE_SURFACE_LOAD(FSSoluteFlux);
    else if (att == "solute natural flux"           ) psl = CREATE_SURFACE_LOAD(FSSoluteNaturalFlux);
	else if (att == "concentration flux"            ) psl = CREATE_SURFACE_LOAD(FSConcentrationFlux);
	else if (att == "normal_traction"               ) psl = CREATE_SURFACE_LOAD(FSBPNormalTraction);
    else if (att == "matching_osm_coef"             ) psl = CREATE_SURFACE_LOAD(FSMatchingOsmoticCoefficient);
	else if (att == "heatflux"                      ) psl = CREATE_SURFACE_LOAD(FSHeatFlux);
	else if (att == "convective_heatflux"           ) psl = CREATE_SURFACE_LOAD(FSConvectiveHeatFlux);
	else if (att == "fluid viscous traction"        ) psl = CREATE_SURFACE_LOAD(FSFluidTraction);
    else if (att == "fluid pressure"                ) psl = CREATE_SURFACE_LOAD(FSFluidPressureLoad);
    else if (att == "fluid velocity"                ) psl = CREATE_SURFACE_LOAD(FSFluidVelocity);
    else if (att == "fluid normal velocity"         ) psl = CREATE_SURFACE_LOAD(FSFluidNormalVelocity);
    else if (att == "fluid rotational velocity"     ) psl = CREATE_SURFACE_LOAD(FSFluidRotationalVelocity);
    else if (att == "fluid resistance"              ) psl = CREATE_SURFACE_LOAD(FSFluidFlowResistance);
    else if (att == "fluid RCR"                     ) psl = CREATE_SURFACE_LOAD(FSFluidFlowRCR);
    else if (att == "fluid backflow stabilization"  ) psl = CREATE_SURFACE_LOAD(FSFluidBackflowStabilization);
    else if (att == "fluid tangential stabilization") psl = CREATE_SURFACE_LOAD(FSFluidTangentialStabilization);
    else if (att == "fluid-FSI traction"            ) psl = CREATE_SURFACE_LOAD(FSFSITraction);
    else if (att == "biphasic-FSI traction"         ) psl = CREATE_SURFACE_LOAD(FSBFSITraction);
	else ParseUnknownAttribute(tag, "type");

	// process surface load
	if (psl)
	{
		// we need to process the value parameter of the "fluid normal velocity" load
		if (dynamic_cast<FSFluidNormalVelocity*>(psl))
		{
			// read parameters
			++tag;
			do
			{
				if (ReadParam(*psl, tag) == false)
				{
					// try to read the parameters
					if (tag == "value")
					{
						const char* szatv = tag.AttributeValue("surface_data", true);
						if (szatv)
						{
							Param* val = psl->GetParam("velocity");
							double v = val->GetFloatValue();
							if (v == 1.0)
							{
								val->SetParamType(Param_STRING);
								val->SetStringValue(szatv);
							}
							else
							{
								val->SetParamType(Param_MATH);
								std::stringstream ss;
								ss << v << "*" << szatv;
								val->SetMathString(ss.str());
							}
						}
					}
					else ParseUnknownTag(tag);
				}
				++tag;
			} while (!tag.isend());

		}
		else
		{
			// read the parameters
			ReadParameters(*psl, tag);
		}

		// set the name
		if (name.empty() == false) psl->SetName(name);

		// assign the surface
		psl->SetItemList(psurf);

		// set the comment
		psl->SetInfo(comment);

		// add to the step
		pstep->AddComponent(psl);
	}
}

//-----------------------------------------------------------------------------
FSBodyLoad* createNewBodyLoad(FSBodyLoad* pbl, const char* szclass, int N)
{
	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "%s%d", szclass + 2, N + 1);
	pbl->SetName(szname);
	return pbl;
}

#define CREATE_BODY_LOAD(className) createNewBodyLoad(new className(&fem, pstep->GetID()), #className, CountLoads<className>(fem))

//-----------------------------------------------------------------------------
//! Parses the body_load section.
void FEBioFormat3::ParseBodyLoad(FSStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();

	// read the comment
	std::string comment = tag.comment();

	// read the (optional) name
	static int n = 1;
	char szbuf[32] = { 0 };
	sprintf(szbuf, "BodyLoad%d", n++);
	string name(szbuf);
	const char* sz = tag.AttributeValue("name", true);
	if (sz) name = sz;

	// create new body load
	FSBodyLoad* pbl = nullptr;
	XMLAtt& att = tag.Attribute("type");
	if      (att == "const"      ) pbl = CREATE_BODY_LOAD(FSConstBodyForce);
	else if (att == "heat_source") pbl = CREATE_BODY_LOAD(FSHeatSource);
	else if (att == "non-const"  ) pbl = CREATE_BODY_LOAD(FSNonConstBodyForce);
    else if (att == "centrifugal") pbl = CREATE_BODY_LOAD(FSCentrifugalBodyForce);
	else {
		// see if FEBio knows it
		pbl = FEBio::CreateBodyLoad(att.cvalue(), &fem);
		if (pbl == nullptr)
			ParseUnknownAttribute(tag, "type");
	}

	// process body load
	if (pbl)
	{
		pbl->SetInfo(comment);
		if (name.empty() == false) pbl->SetName(name);
		pstep->AddComponent(pbl);
		ReadParameters(*pbl, tag);
	}
}

//-----------------------------------------------------------------------------
bool FEBioFormat3::ParseInitialSection(XMLTag& tag)
{
	// make sure the section is not empty
	if (tag.isleaf()) return true;

	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	++tag;
	do
	{
		if (tag == "ic")
		{
			const char* sztype = tag.AttributeValue("type");

			string name;
			const char* szname = tag.AttributeValue("name", true);
			if (szname) name = szname;

			if (strcmp(sztype, "init_dof") == 0)
			{
				const char* szset = tag.AttributeValue("node_set");
				FEItemListBuilder* pg = febio.FindNamedSelection(szset);
				if (pg == 0) throw XMLReader::MissingTag(tag, "node_set");

				string scaleType("");
				string scaleValue("");
				string bc;
				++tag;
				do
				{
					if (tag == "value")
					{
						const char* sztype = tag.AttributeValue("type", true);
						if (sztype) scaleType = sztype;
						scaleValue = tag.szvalue();
					}
					else if (tag == "dof")
					{
						bc = tag.szvalue();
						if (validate_dof(bc) == false) throw XMLReader::InvalidValue(tag);
					}
					else ParseUnknownTag(tag);
					++tag;
				} while (!tag.isend());

				double val = 0.0;
				if (scaleType.empty())
				{
					val = atof(scaleValue.c_str());
				}

				// create a new initial velocity BC
				FSInitialCondition* pic = 0;
				char szname[64] = { 0 };
                if (bc == "T")
                {
					pic = new FSInitTemperature(&fem, pg, val, m_pBCStep->GetID());
					sprintf(szname, "InitialTemperature%02d", CountICs<FSInitTemperature>(fem) + 1);
                }
                else if (bc == "p")
				{
					pic = new FSInitFluidPressure(&fem, pg, val, m_pBCStep->GetID());
					sprintf(szname, "InitialFluidPressure%02d", CountICs<FSInitFluidPressure>(fem) + 1);

					// process value value
					Param* pp = pic->GetParam("value"); assert(pp);
					if (scaleType == "math")
					{
						pp->SetParamType(Param_MATH);
						pp->SetMathString(scaleValue);
					}
					else if (scaleType == "map")
					{
						pp->SetParamType(Param_STRING);
						pp->SetStringValue(scaleValue);
					}
				}
                else if (bc == "q")
                {
                    pic = new FSInitShellFluidPressure(&fem, pg, val, m_pBCStep->GetID());
                    sprintf(szname, "InitialShellFluidPressure%02d", CountICs<FSInitShellFluidPressure>(fem) + 1);
                }
                else if (bc == "vx")
                {
					pic = new FSNodalVelocities(&fem, pg, vec3d(val, 0, 0), m_pBCStep->GetID());
					sprintf(szname, "InitialVelocity%02d", CountICs<FSNodalVelocities>(fem) + 1);
                }
                else if (bc == "vy")
                {
					pic = new FSNodalVelocities(&fem, pg, vec3d(0, val, 0), m_pBCStep->GetID());
					sprintf(szname, "InitialVelocity%02d", CountICs<FSNodalVelocities>(fem) + 1);
                }
                else if (bc == "vz")
                {
					pic = new FSNodalVelocities(&fem, pg, vec3d(0, 0, val), m_pBCStep->GetID());
					sprintf(szname, "InitialVelocity%02d", CountICs<FSNodalVelocities>(fem) + 1);
                }
				else if (bc == "svx")
                {
					pic = new FSNodalShellVelocities(&fem, pg, vec3d(val, 0, 0), m_pBCStep->GetID());
					sprintf(szname, "InitShellVelocity%02d", CountICs<FSNodalShellVelocities>(fem) + 1);
                }
                else if (bc == "svy")
                {
					pic = new FSNodalShellVelocities(&fem, pg, vec3d(0, val, 0), m_pBCStep->GetID());
					sprintf(szname, "InitShellVelocity%02d", CountICs<FSNodalShellVelocities>(fem) + 1);
                }
                else if (bc == "svz")
                {
					pic = new FSNodalShellVelocities(&fem, pg, vec3d(0, 0, val), m_pBCStep->GetID());
					sprintf(szname, "InitShellVelocity%02d", CountICs<FSNodalShellVelocities>(fem) + 1);
                }
                else if (bc == "ef")
                {
					pic = new FSInitFluidDilatation(&fem, pg, val, m_pBCStep->GetID());
					sprintf(szname, "InitialFluidDilatation%02d", CountICs<FSInitFluidDilatation>(fem) + 1);
                }
                else if (bc.compare(0,1,"c") == 0)
                {
                    int nsol;
                    sscanf(bc.substr(1).c_str(),"%d",&nsol);
                    pic = new FSInitConcentration(&fem, pg, nsol-1, val, m_pBCStep->GetID());
                    sprintf(szname, "InitConcentration%02d", CountICs<FSInitConcentration>(fem) + 1);
                }
                else if (bc.compare(0,1,"d") == 0)
                {
                    int nsol;
                    sscanf(bc.substr(1).c_str(),"%d",&nsol);
                    pic = new FSInitShellConcentration(&fem, pg, nsol-1, val, m_pBCStep->GetID());
                    sprintf(szname, "InitShellConcentration%02d", CountICs<FSInitShellConcentration>(fem) + 1);
				}

				if (pic)
				{
					pic->SetName(szname);
					m_pBCStep->AddComponent(pic);
				}
			}
			else if (strcmp(sztype, "velocity") == 0)
			{
				const char* szset = tag.AttributeValue("node_set");
				FEItemListBuilder* pg = febio.FindNamedSelection(szset);
				if (pg == 0) throw XMLReader::MissingTag(tag, "node_set");

				if (name.empty())
				{
					char szbuf[32] = { 0 };
					sprintf(szbuf, "InitialVelocity%02d", CountICs<FSNodalVelocities>(fem) + 1);
					name = szbuf;
				}

				vec3d v(0, 0, 0);
				++tag;
				do
				{
					if (tag == "value") tag.value(v);
					++tag;
				} while (!tag.isend());
				FSNodalVelocities* pic = new FSNodalVelocities(&fem, pg, v, m_pBCStep->GetID());

				pic->SetName(name);
				m_pBCStep->AddComponent(pic);
			}
			else if (strcmp(sztype, "prestrain") == 0)
			{
				FSInitPrestrain* pip = new FSInitPrestrain(&fem);

				if (name.empty())
				{
					char szbuf[32] = { 0 };
					sprintf(szbuf, "InitPrestrain%d", CountConstraints<FSInitPrestrain>(fem) + 1);
					name = szbuf;
				}
				pip->SetName(name);
				m_pBCStep->AddComponent(pip);

				ReadParameters(*pip, tag);
			}
			else throw XMLReader::InvalidAttributeValue(tag, "type", sztype);
		}
		else ParseUnknownTag(tag);
		++tag;
	} while (!tag.isend());

	return true;
}

//=============================================================================
//
//                                C O N T A C T
//
//=============================================================================

//-----------------------------------------------------------------------------
//! Read the contact section (defined for version 2.0 and up).
bool FEBioFormat3::ParseContactSection(XMLTag& tag)
{
	// make sure the section is not empty
	if (tag.isleaf()) return true;

	++tag;
	do
	{
		if (tag == "contact") ParseContact(m_pBCStep, tag);
		else ParseUnknownTag(tag);
		++tag;
	} while (!tag.isend());
	return true;
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseContact(FSStep *pstep, XMLTag &tag)
{
	FEBioInputModel& febio = GetFEBioModel();

	// get the contact interface type
	XMLAtt& atype = tag.Attribute("type");

	// check special cases
	if      (atype == "rigid_wall"       ) ParseRigidWall       (pstep, tag);
	else if (atype == "linear constraint") ParseLinearConstraint(pstep, tag);
	else if (atype == "rigid joint"      ) ParseContactJoint    (pstep, tag);
	else
	{
		const char* szpair = tag.AttributeValue("surface_pair");
		FEBioInputModel::SurfacePair* surfPair = febio.FindSurfacePair(szpair);
		if (surfPair == 0) throw XMLReader::InvalidAttributeValue(tag, "surface_pair", szpair);

		// standard contact interfaces
		FSPairedInterface* pci = 0;
		if      (atype == "sliding-node-on-facet"      ) pci = ParseContactSliding        (pstep, tag);
		else if (atype == "sliding-facet-on-facet"     ) pci = ParseContactF2FSliding     (pstep, tag);
		else if (atype == "sliding-elastic"            ) pci = ParseContactTC             (pstep, tag);
		else if (atype == "sliding-biphasic"           ) pci = ParseContactBiphasic       (pstep, tag);
		else if (atype == "sliding-biphasic-solute"    ) pci = ParseContactSolute         (pstep, tag);
		else if (atype == "sliding-multiphasic"        ) pci = ParseContactMultiphasic    (pstep, tag);
		else if (atype == "tied-node-on-facet"         ) pci = ParseContactTied           (pstep, tag);
		else if (atype == "tied-facet-on-facet"        ) pci = ParseContactF2FTied        (pstep, tag);
		else if (atype == "tied-elastic"               ) pci = ParseContactTiedElastic    (pstep, tag);
		else if (atype == "sticky"                     ) pci = ParseContactSticky         (pstep, tag);
		else if (atype == "periodic boundary"          ) pci = ParseContactPeriodic       (pstep, tag);
		else if (atype == "tied-biphasic"              ) pci = ParseContactTiedPoro       (pstep, tag);
		else if (atype == "tied-multiphasic"           ) pci = ParseContactTiedMultiphasic(pstep, tag);
		else if (atype == "gap heat flux"              ) pci = ParseContactGapHeatFlux    (pstep, tag);
		else ParseUnknownTag(tag);

		if (pci)
		{
			const char* szname = tag.AttributeValue("name", true);
			if (szname) pci->SetName(szname);

			// read the parameters
			ReadParameters(*pci, tag);

			// assign surfaces
			FEBioInputModel::Part* part = surfPair->GetPart();
			assert(part);
			if (part)
			{
				if (surfPair->PrimarySurfaceID() >= 0)
				{
					string name1 = part->GetSurface(surfPair->PrimarySurfaceID()).name();
					FSSurface* surf1 = febio.FindNamedSurface(name1.c_str());
					pci->SetPrimarySurface(surf1);
				}

				if (surfPair->SecondarySurfaceID() >= 0)
				{
					string name2 = part->GetSurface(surfPair->SecondarySurfaceID()).name();
					FSSurface* surf2 = febio.FindNamedSurface(name2.c_str());
					pci->SetSecondarySurface(surf2);
				}
			}

			// add to the analysis step
			pstep->AddComponent(pci);
		}
	}
}

//-----------------------------------------------------------------------------
FSPairedInterface* FEBioFormat3::ParseContactSliding(FSStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();

	// create new sliding interface
	FSSlidingWithGapsInterface* pi = new FSSlidingWithGapsInterface(&fem, pstep->GetID());

	// get the (optional) contact name
	char szbuf[256];
	const char* szname = tag.AttributeValue("name", true);
	if (szname) sprintf(szbuf, "%s", szname);
	else sprintf(szbuf, "SlidingInterface%02d", CountInterfaces<FSSlidingWithGapsInterface>(fem)+1);
	pi->SetName(szbuf);

	return pi;
}

//-----------------------------------------------------------------------------
FSPairedInterface* FEBioFormat3::ParseContactF2FSliding(FSStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();

	// create new sliding interface
	FSFacetOnFacetInterface* pi = new FSFacetOnFacetInterface(&fem, pstep->GetID());

	// get the (optional) contact name
	char szbuf[256];
	const char* szname = tag.AttributeValue("name", true);
	if (szname) sprintf(szbuf, "%s", szname);
	else sprintf(szbuf, "SlidingContact%02d", CountInterfaces<FSFacetOnFacetInterface>(fem)+1);
	pi->SetName(szbuf);

	return pi;
}

//-----------------------------------------------------------------------------
FSPairedInterface* FEBioFormat3::ParseContactBiphasic(FSStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();

	// create new contact interface
	FSPoroContact* pi = new FSPoroContact(&fem, pstep->GetID());

	// read the name
	char szname[256];
	sprintf(szname, "BiphasicContact%02d", CountInterfaces<FSPoroContact>(fem)+1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	return pi;
}

//-----------------------------------------------------------------------------
FSPairedInterface* FEBioFormat3::ParseContactSolute(FSStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();

	// create new interface
	FSPoroSoluteContact* pi = new FSPoroSoluteContact(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "BiphasicSoluteContact%02d", CountInterfaces<FSPoroSoluteContact>(fem)+1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	return pi;
}

//-----------------------------------------------------------------------------
FSPairedInterface* FEBioFormat3::ParseContactMultiphasic(FSStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();

	// create new interface
	FSMultiphasicContact* pi = new FSMultiphasicContact(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "MultiphasicContact%02d", CountInterfaces<FSMultiphasicContact>(fem)+1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	return pi;
}

//-----------------------------------------------------------------------------
FSPairedInterface* FEBioFormat3::ParseContactTied(FSStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();

	// create new interface
	FSTiedInterface* pi = new FSTiedInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "TiedInterface%02d", CountInterfaces<FSTiedInterface>(fem)+1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	return pi;
}

//-----------------------------------------------------------------------------
FSPairedInterface* FEBioFormat3::ParseContactF2FTied(FSStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();

	// create new interface
	FSF2FTiedInterface* pi = new FSF2FTiedInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "F2FTiedInterface%02d", CountInterfaces<FSF2FTiedInterface>(fem)+1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	return pi;
}

//-----------------------------------------------------------------------------
FSPairedInterface* FEBioFormat3::ParseContactTiedElastic(FSStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();

	// create new interface
	FSTiedElasticInterface* pi = new FSTiedElasticInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "TiedElasticInterface%02d", CountInterfaces<FSTiedElasticInterface>(fem)+1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	return pi;
}

//-----------------------------------------------------------------------------
FSPairedInterface* FEBioFormat3::ParseContactSticky(FSStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();

	// create new interface
	FSStickyInterface* pi = new FSStickyInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "StickyInterface%02d", CountInterfaces<FSStickyInterface>(fem)+1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	return pi;
}

//-----------------------------------------------------------------------------
FSPairedInterface* FEBioFormat3::ParseContactPeriodic(FSStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();

	// create new interface
	FSPeriodicBoundary* pi = new FSPeriodicBoundary(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "PeriodicBoundary%02d", CountInterfaces<FSPeriodicBoundary>(fem)+1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	return pi;
}

//-----------------------------------------------------------------------------
FSPairedInterface* FEBioFormat3::ParseContactTC(FSStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();

	// create new interface
	FSTensionCompressionInterface* pi = new FSTensionCompressionInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "TCInterface%02d", CountInterfaces<FSTensionCompressionInterface>(fem)+1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	return pi;
}

//-----------------------------------------------------------------------------
FSPairedInterface* FEBioFormat3::ParseContactTiedPoro(FSStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();

	// create new interface
	FSTiedBiphasicInterface* pi = new FSTiedBiphasicInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "TiedBiphasicInterface%02d", CountInterfaces<FSTiedBiphasicInterface>(fem)+1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	return pi;
}

//-----------------------------------------------------------------------------
FSPairedInterface* FEBioFormat3::ParseContactTiedMultiphasic(FSStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();

	// create new interface
	FSTiedMultiphasicInterface* pi = new FSTiedMultiphasicInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "TiedMultiphasicInterface%02d", CountInterfaces<FSTiedMultiphasicInterface>(fem)+1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	return pi;
}

//-----------------------------------------------------------------------------
FSPairedInterface* FEBioFormat3::ParseContactGapHeatFlux(FSStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();

	// create new interface
	FSGapHeatFluxInterface* pi = new FSGapHeatFluxInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "GapHeatFlux%02d", CountInterfaces<FSGapHeatFluxInterface>(fem)+1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	return pi;
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseRigidWall(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// create a new interface
	FSRigidWallInterface* pci = new FSRigidWallInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "RigidWall%02d", CountInterfaces<FSRigidWallInterface>(fem)+1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pci->SetName(szname);

	// get the surface
	const char* szsurf = tag.AttributeValue("surface", true);
	if (szsurf)
	{
		FSSurface* psurf = febio.FindNamedSurface(szsurf);
		if (psurf == 0) throw XMLReader::InvalidAttributeValue(tag, "surface", szsurf);
		pci->SetItemList(psurf);
	}

	++tag;
	do
	{
		// read parameters
		if      (tag == "laugon"   ) { int n; tag.value(n); pci->SetBoolValue(FSRigidWallInterface::LAUGON, (n == 0 ? false : true)); }
		else if (tag == "tolerance") { double f; tag.value(f); pci->SetFloatValue(FSRigidWallInterface::ALTOL, f); }
		else if (tag == "penalty"  ) { ReadParam(*pci, tag); }
		else if (tag == "offset"   ) { ReadParam(*pci, tag); }
		else if (tag == "plane")
		{
			double n[4];
			tag.value(n, 4);
			pci->SetFloatValue(FSRigidWallInterface::PA, n[0]);
			pci->SetFloatValue(FSRigidWallInterface::PB, n[1]);
			pci->SetFloatValue(FSRigidWallInterface::PC, n[2]);
			pci->SetFloatValue(FSRigidWallInterface::PD, n[3]);

			const char* szlc = tag.AttributeValue("lc", true);
			if (szlc) febio.AddParamCurve(&pci->GetParam(FSRigidWallInterface::OFFSET), atoi(szlc) - 1);
		}
		++tag;
	}
	while (!tag.isend());

	// add interface to step
	pstep->AddComponent(pci);
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseContactJoint(FSStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();

	FSRigidJoint* pi = new FSRigidJoint(&fem, pstep->GetID());
	char szname[256];
	sprintf(szname, "RigidJoint%02d", CountInterfaces<FSRigidJoint>(fem)+1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);
	pstep->AddComponent(pi);

	int na = -1, nb = -1;

	double tol = 0, pen = 0;
	vec3d rj;
	++tag;
	do
	{
		if (tag == "tolerance") tag.value(tol);
		else if (tag == "penalty") tag.value(pen);
		else if (tag == "body_a") tag.value(na);
		else if (tag == "body_b") tag.value(nb);
		else if (tag == "joint") tag.value(rj);
		else ParseUnknownTag(tag);

		++tag;
	} while (!tag.isend());

	pi->SetFloatValue(FSRigidJoint::TOL, tol);
	pi->SetFloatValue(FSRigidJoint::PENALTY, pen);
	pi->SetVecValue(FSRigidJoint::RJ, rj);

	FEBioInputModel& febio = GetFEBioModel();

	if (na >= 0) pi->m_pbodyA = febio.GetMaterial(na - 1);
	if (nb >= 0) pi->m_pbodyB = febio.GetMaterial(nb - 1);
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseRigidConstraint(FSStep* pstep, XMLTag& tag)
{
	const char* szdof[6] = { "Rx", "Ry", "Rz", "Ru", "Rv", "Rw" };

	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// get the name attribute
	string name;
	const char* szname = tag.AttributeValue("name", true);
	if (szname) name = szname;

	// get the type attribute
	XMLAtt& type = tag.Attribute("type");

	if (type == "fix")
	{
		FSRigidFixed* pc = CREATE_RIGID_CONSTRAINT(FSRigidFixed);
		if (name.empty() == false) pc->SetName(name);

		++tag;
		do
		{
			if (tag == "dofs")
			{
				const char* sz = tag.szvalue();

				const char* ch = sz;
				while (ch)
				{
					const char* ch2 = strchr(ch, ',');
					int n = (ch2 ? ch2 - ch : strlen(ch));
					if (strncmp(ch, szdof[0], n) == 0) pc->SetDOF(0, true);
					if (strncmp(ch, szdof[1], n) == 0) pc->SetDOF(1, true);
					if (strncmp(ch, szdof[2], n) == 0) pc->SetDOF(2, true);
					if (strncmp(ch, szdof[3], n) == 0) pc->SetDOF(3, true);
					if (strncmp(ch, szdof[4], n) == 0) pc->SetDOF(4, true);
					if (strncmp(ch, szdof[5], n) == 0) pc->SetDOF(5, true);

					if (ch2) ch = ch2 + 1; else ch = 0;
				}
			}
			else if (tag == "rb")
			{
				int mid = -1;
				tag.value(mid);

				// get the rigid material
				GMaterial* pgm = 0;
				if (mid > 0) pgm = febio.GetMaterial(mid - 1);
				int matid = (pgm ? pgm->GetID() : -1);
				assert(dynamic_cast<FSRigidMaterial*>(pgm->GetMaterialProperties()));

				pc->SetMaterialID(matid);
			}
			++tag;
		} while (!tag.isend());
        pstep->AddComponent(pc);
	}
	else if (type == "prescribe")
	{
		FSRigidDisplacement* pc = CREATE_RIGID_CONSTRAINT(FSRigidDisplacement);
		if (name.empty() == false) pc->SetName(name);

		++tag;
		do
		{
			if (tag == "dof")
			{
				const char* sz = tag.szvalue();
				if (strcmp(sz, szdof[0]) == 0) pc->SetDOF(0);
				if (strcmp(sz, szdof[1]) == 0) pc->SetDOF(1);
				if (strcmp(sz, szdof[2]) == 0) pc->SetDOF(2);
				if (strcmp(sz, szdof[3]) == 0) pc->SetDOF(3);
				if (strcmp(sz, szdof[4]) == 0) pc->SetDOF(4);
				if (strcmp(sz, szdof[5]) == 0) pc->SetDOF(5);
			}
			else if (tag == "rb")
			{
				int mid = -1;
				tag.value(mid);

				// get the rigid material
				GMaterial* pgm = 0;
				if (mid > 0) pgm = febio.GetMaterial(mid - 1);
				int matid = (pgm ? pgm->GetID() : -1);
				assert(dynamic_cast<FSRigidMaterial*>(pgm->GetMaterialProperties()));

				pc->SetMaterialID(matid);
			}
			else ReadParam(*pc, tag);
			++tag;
		} while (!tag.isend());
        pstep->AddComponent(pc);
	}
	else if (type == "force")
	{
		FSRigidForce* pc = CREATE_RIGID_CONSTRAINT(FSRigidForce);
		if (name.empty() == false) pc->SetName(name);

		++tag;
		do
		{
			if (tag == "dof")
			{
				const char* sz = tag.szvalue();
				if (strcmp(sz, szdof[0]) == 0) pc->SetDOF(0);
				if (strcmp(sz, szdof[1]) == 0) pc->SetDOF(1);
				if (strcmp(sz, szdof[2]) == 0) pc->SetDOF(2);
				if (strcmp(sz, szdof[3]) == 0) pc->SetDOF(3);
				if (strcmp(sz, szdof[4]) == 0) pc->SetDOF(4);
				if (strcmp(sz, szdof[5]) == 0) pc->SetDOF(5);
			}
			else if (tag == "rb")
			{
				int mid = -1;
				tag.value(mid);

				// get the rigid material
				GMaterial* pgm = 0;
				if (mid > 0) pgm = febio.GetMaterial(mid - 1);
				int matid = (pgm ? pgm->GetID() : -1);
				assert(dynamic_cast<FSRigidMaterial*>(pgm->GetMaterialProperties()));

				pc->SetMaterialID(matid);
			}
			else ReadParam(*pc, tag);
			++tag;
		} while (!tag.isend());
        pstep->AddComponent(pc);
	}
	else if ((type == "rigid_velocity") || (type == "initial_rigid_velocity"))
	{
		FSRigidVelocity* pv = CREATE_RIGID_CONSTRAINT(FSRigidVelocity);
		if (name.empty() == false) pv->SetName(name);
		++tag;
		do
		{
			if (tag == "rb")
			{
				int mid = -1;
				tag.value(mid);

				// get the rigid material
				GMaterial* pgm = 0;
				if (mid > 0) pgm = febio.GetMaterial(mid - 1);
				int matid = (pgm ? pgm->GetID() : -1);
				assert(dynamic_cast<FSRigidMaterial*>(pgm->GetMaterialProperties()));

				pv->SetMaterialID(matid);
			}
			else ReadParam(*pv, tag);
			++tag;
		} while (!tag.isend());
        pstep->AddComponent(pv);
	}
	else if ((type == "rigid_angular_velocity") || (type == "initial_rigid_angular_velocity"))
	{
		FSRigidAngularVelocity* pv = CREATE_RIGID_CONSTRAINT(FSRigidAngularVelocity);
		if (name.empty() == false) pv->SetName(name);
		++tag;
		do
		{
			if (tag == "rb")
			{
				int mid = -1;
				tag.value(mid);

				// get the rigid material
				GMaterial* pgm = 0;
				if (mid > 0) pgm = febio.GetMaterial(mid - 1);
				int matid = (pgm ? pgm->GetID() : -1);
				assert(dynamic_cast<FSRigidMaterial*>(pgm->GetMaterialProperties()));

				pv->SetMaterialID(matid);
			}
			else ReadParam(*pv, tag);
			++tag;
		} while (!tag.isend());
        pstep->AddComponent(pv);
	}
	else ParseUnknownTag(tag);
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseRigidConnector(FSStep *pstep, XMLTag &tag, const int rc)
{
	FSModel& fem = GetFSModel();

	FSRigidConnector* pi = nullptr;
	char szname[256];

	switch (rc) {
	case 0:
		pi = new FSRigidSphericalJoint(&fem, pstep->GetID());
		sprintf(szname, "RigidSphericalJoint%02d", CountConnectors<FSRigidSphericalJoint>(fem)+1);
		break;
	case 1:
		pi = new FSRigidRevoluteJoint(&fem, pstep->GetID());
		sprintf(szname, "RigidrevoluteJoint%02d", CountConnectors<FSRigidRevoluteJoint>(fem)+1);
		break;
	case 2:
		pi = new FSRigidPrismaticJoint(&fem, pstep->GetID());
		sprintf(szname, "RigidPrismaticJoint%02d", CountConnectors<FSRigidPrismaticJoint>(fem)+1);
		break;
	case 3:
		pi = new FSRigidCylindricalJoint(&fem, pstep->GetID());
		sprintf(szname, "RigidCylindricalJoint%02d", CountConnectors<FSRigidCylindricalJoint>(fem)+1);
		break;
	case 4:
		pi = new FSRigidPlanarJoint(&fem, pstep->GetID());
		sprintf(szname, "RigidPlanarJoint%02d", CountConnectors<FSRigidPlanarJoint>(fem)+1);
		break;
    case 5:
        pi = new FSRigidLock(&fem, pstep->GetID());
        sprintf(szname, "RigidLock%02d", CountConnectors<FSRigidLock>(fem)+1);
        break;
	case 6:
		pi = new FSRigidSpring(&fem, pstep->GetID());
		sprintf(szname, "RigidSpring%02d", CountConnectors<FSRigidSpring>(fem)+1);
		break;
	case 7:
		pi = new FSRigidDamper(&fem, pstep->GetID());
		sprintf(szname, "RigidDamper%02d", CountConnectors<FSRigidDamper>(fem)+1);
		break;
	case 8:
		pi = new FSRigidAngularDamper(&fem, pstep->GetID());
		sprintf(szname, "RigidAngularDamper%02d", CountConnectors<FSRigidAngularDamper>(fem)+1);
		break;
	case 9:
		pi = new FSRigidContractileForce(&fem, pstep->GetID());
		sprintf(szname, "RigidContractileForce%02d", CountConnectors<FSRigidContractileForce>(fem)+1);
		break;
	case 10:
		pi = new FSGenericRigidJoint(&fem, pstep->GetID());
		sprintf(szname, "GenericRigidJoint%02d", CountConnectors<FSGenericRigidJoint>(fem) + 1);
		break;
	default:
		assert(false);
		break;
	}
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);
	pstep->AddComponent(pi);

	int na = -1, nb = -1;

	FEBioInputModel& febio = GetFEBioModel();

	++tag;
	do
	{
		if (ReadParam(*pi, tag) == false)
		{
			if (tag == "body_a") 
			{
				tag.value(na);
				if (na >= 0) pi->SetRigidBody1(febio.GetMaterial(na - 1)->GetID());
			}
			else if (tag == "body_b") 
			{
				tag.value(nb);
				if (nb >= 0) pi->SetRigidBody2(febio.GetMaterial(nb - 1)->GetID());
			}
			else ParseUnknownTag(tag);
		}
		++tag;
	}
	while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseRigidJoint(FSStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();

	FSRigidJoint* pi = new FSRigidJoint(&fem, pstep->GetID());
	char szname[256];
	sprintf(szname, "RigidJoint%02d", CountInterfaces<FSRigidJoint>(fem) + 1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);
	pstep->AddComponent(pi);

	int na = -1, nb = -1;

	double tol = 0, pen = 0;
	vec3d rj;
	++tag;
	do
	{
		if (tag == "tolerance") tag.value(tol);
		else if (tag == "penalty") tag.value(pen);
		else if (tag == "body_a") tag.value(na);
		else if (tag == "body_b") tag.value(nb);
		else if (tag == "joint") tag.value(rj);
		else ParseUnknownTag(tag);

		++tag;
	} while (!tag.isend());

	pi->SetFloatValue(FSRigidJoint::TOL, tol);
	pi->SetFloatValue(FSRigidJoint::PENALTY, pen);
	pi->SetVecValue(FSRigidJoint::RJ, rj);

	FEBioInputModel& febio = GetFEBioModel();

	if (na >= 0) pi->m_pbodyA = febio.GetMaterial(na - 1);
	if (nb >= 0) pi->m_pbodyB = febio.GetMaterial(nb - 1);
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseLinearConstraint(FSStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();

	FSLinearConstraintSet* pset = new FSLinearConstraintSet;
	pstep->AddLinearConstraint(pset);

	// read the linear constraints
	++tag;
	do
	{
		if (tag == "linear_constraint")
		{
			FSLinearConstraintSet::LinearConstraint LC;
			FSLinearConstraintSet::LinearConstraint::DOF dof;
			++tag;
			do
			{
				if (tag == "node")
				{
					tag.value(dof.s);
					dof.node = tag.AttributeValue<int>("id", 0);

					const char* szbc = tag.AttributeValue("bc");
                    int dofcode = fem.GetDOFIndex(szbc);
					if (dofcode != -1) dof.bc = dofcode;
					else throw XMLReader::InvalidAttributeValue(tag, "bc", szbc);

					LC.m_dof.push_back(dof);
				}
				else throw XMLReader::InvalidTag(tag);
				++tag;
			} while (!tag.isend());

			// add the linear constraint to the system
			pset->m_set.push_back(LC);
		}
		else if (tag == "tol") tag.value(pset->m_atol);
		else if (tag == "penalty") tag.value(pset->m_penalty);
		else if (tag == "maxaug") tag.value(pset->m_nmaxaug);
		else throw XMLReader::InvalidTag(tag);
		++tag;
	} while (!tag.isend());
}

//=============================================================================
//
//                                D I S C R E T E
//
//=============================================================================

bool FEBioFormat3::ParseDiscreteSection(XMLTag& tag)
{
	// make sure the section is not empty
	if (tag.isleaf()) return true;

	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();
	GModel& gm = fem.GetModel();

	vector<GDiscreteElementSet*> set;
	++tag;
	do
	{
		if (tag == "discrete_material")
		{
			const char* sztype = tag.AttributeValue("type");
			const char* szname = tag.AttributeValue("name");
			if ((strcmp(sztype, "linear spring") == 0) || (strcmp(sztype, "tension-only linear spring") == 0))
			{
				FSLinearSpringMaterial* mat = new FSLinearSpringMaterial(&fem);
				GDiscreteSpringSet* pg = new GDiscreteSpringSet(&gm);
				pg->SetMaterial(mat);
				pg->SetName(szname);
				fem.GetModel().AddDiscreteObject(pg);
				++tag;
				do
				{
					if (tag == "E")
					{
						double E;
						tag.value(E);
						mat->SetSpringConstant(E);
					}
					else ParseUnknownTag(tag);
					++tag;
				} while (!tag.isend());
				set.push_back(pg);
			}
			else if (strcmp(sztype, "nonlinear spring") == 0)
			{
				FSNonLinearSpringMaterial* mat = new FSNonLinearSpringMaterial(&fem);
				GDiscreteSpringSet* pg = new GDiscreteSpringSet(&gm);
				pg->SetMaterial(mat);
				pg->SetName(szname);
				fem.GetModel().AddDiscreteObject(pg);
				++tag;
				do
				{
					if (ReadParam(*mat, tag) == false)
					{
						ParseUnknownTag(tag);
					}
					++tag;
				} while (!tag.isend());
				set.push_back(pg);
			}
			else if (strcmp(sztype, "Hill") == 0)
			{
				FSHillContractileMaterial* mat = new FSHillContractileMaterial(&fem);
				GDiscreteSpringSet* pg = new GDiscreteSpringSet(&gm);
				pg->SetMaterial(mat);
				pg->SetName(szname);
				fem.GetModel().AddDiscreteObject(pg);
				++tag;
				do
				{
					if (ReadParam(*mat, tag) == false)
					{
						FSProperty* prop = mat->FindProperty(tag.m_sztag);
						FS1DPointFunction* pf1d = dynamic_cast<FS1DPointFunction*>(prop ? prop->GetComponent(0) : nullptr);
						if (pf1d)
						{
							LoadCurve lc;
							ParseLoadCurve(tag, lc);
							pf1d->SetPointCurve(lc);
						}
						else ParseUnknownTag(tag);
					}
					++tag;
				} while (!tag.isend());
				set.push_back(pg);
			}
			else
			{
				assert(false);
			}
		}
		else if (tag == "discrete")
		{
			int mid = tag.AttributeValue<int>("dmat", 0);
			GDiscreteElementSet* ps = set[mid - 1];
			const char* szset = tag.AttributeValue("discrete_set");

			FEBioInputModel& feb = GetFEBioModel();
			if (feb.BuildDiscreteSet(*ps, szset) == false)
			{
				assert(false);
			}
		}
		else if (tag == "rigid_cable")
		{
			ParseRigidCable(m_pBCStep, tag);
		}
		else ParseUnknownTag(tag);
		++tag;
	}
	while (!tag.isend());

	return true;
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseRigidCable(FSStep* pstep, XMLTag& tag)
{
	FSModel* fem = &GetFSModel();
	const char* szname = tag.AttributeValue("name", true);
	FSRigidLoad* prl = FEBio::CreateRigidLoad("rigid_cable", fem);
	if (prl == nullptr)
	{
		ParseUnknownTag(tag);
		return;
	}

	if (szname) prl->SetName(szname);
	else
	{
		int n = pstep->RigidLoads() + 1;
		char sz[100] = { 0 };
		sprintf(sz, "RigidLoad%d", n);
		prl->SetName(sz);
	}

	pstep->AddComponent(prl);

	// we need the rigid_cable_point property
	FSProperty* points = prl->FindProperty("rigid_cable_point");
	if (points == nullptr)
	{
		ParseUnknownTag(tag);
		return;
	}

	// process parameters
	++tag;
	do
	{
		// try to read the parameters
		if (ReadParam(*prl, tag) == false)
		{
			if (tag == "rigid_cable_point")
			{
				FSGenericClass* rcp = FEBio::CreateGenericClass("rigid_cable_point", fem);
				ReadParameters(*rcp, tag);
				points->AddComponent(rcp);
			}
			else ParseUnknownTag(tag);
		}
		++tag;
	} while (!tag.isend());
}

//=============================================================================
//
//                            C O N S T R A I N T S
//
//=============================================================================

//-----------------------------------------------------------------------------
bool FEBioFormat3::ParseConstraintSection(XMLTag& tag)
{
	if (tag.isleaf()) return true;

	FSStep* pstep = m_pBCStep;

	++tag;
	do
	{
		if (tag == "constraint")
		{
			const char* sztype = tag.AttributeValue("type");
			if      (strcmp(sztype, "volume"                 ) == 0) ParseVolumeConstraint(pstep, tag);
			else if (strcmp(sztype, "symmetry plane"         ) == 0) ParseSymmetryPlane(pstep, tag);
			else if (strcmp(sztype, "in-situ stretch"        ) == 0) ParseInSituStretchConstraint(pstep, tag);
			else if (strcmp(sztype, "prestrain"              ) == 0) ParsePrestrainConstraint(pstep, tag);
            else if (strcmp(sztype, "normal fluid flow"      ) == 0) ParseNrmlFldVlctSrf(pstep, tag);
            else if (strcmp(sztype, "frictionless fluid wall") == 0) ParseFrictionlessFluidWall(pstep, tag);
            else if (strcmp(sztype, "fixed normal displacement") == 0) ParseFixedNormalDisplacement(pstep, tag);
			else throw XMLReader::InvalidAttributeValue(tag, "type", sztype);
		}
		else ParseUnknownTag(tag);
		++tag;
	}
	while (!tag.isend());

	return true;
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseVolumeConstraint(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// make sure there is something to read
	if (tag.isempty()) return;

	// get the (optional) contact name
	char szbuf[256];
	const char* szname = tag.AttributeValue("name", true);
	if (szname == 0)
	{
		sprintf(szbuf, "VolumeConstraint%02d", CountConstraints<FSVolumeConstraint>(fem)+1);
		szname = szbuf;
	}

	// find the surface
	const char* szsurf = tag.AttributeValue("surface");
	FSSurface* psurf = febio.FindNamedSurface(szsurf);
	if (psurf == 0) throw XMLReader::InvalidAttributeValue(tag, "surface", szsurf);

	// create a new volume constraint
	FSVolumeConstraint* pi = new FSVolumeConstraint(&fem, pstep->GetID());
	pi->SetName(szname);
	pi->SetItemList(psurf);
	pstep->AddComponent(pi);

	// read parameters
	ReadParameters(*pi, tag);
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseSymmetryPlane(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// make sure there is something to read
	if (tag.isempty()) return;

	// get the (optional) contact name
	char szbuf[256];
	const char* szname = tag.AttributeValue("name", true);
	if (szname == 0)
	{
		sprintf(szbuf, "SymmetryPlane%02d", CountConstraints<FSSymmetryPlane>(fem)+1);
		szname = szbuf;
	}

	// find the surface
	const char* szsurf = tag.AttributeValue("surface");
	FSSurface* psurf = febio.FindNamedSurface(szsurf);
	if (psurf == 0) throw XMLReader::InvalidAttributeValue(tag, "surface", szsurf);

	// create a new symmetry plane
	FSSymmetryPlane* pi = new FSSymmetryPlane(&fem, pstep->GetID());
	pi->SetName(szname);
	pi->SetItemList(psurf);
	pstep->AddComponent(pi);

	// read parameters
	ReadParameters(*pi, tag);
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseInSituStretchConstraint(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// make sure there is something to read
	if (tag.isempty()) return;

	// get the name
	char szbuf[256];
	const char* szname = tag.AttributeValue("name", true);
	if (szname == 0)
	{
		sprintf(szbuf, "InSituStretch%02d", CountConstraints<FSInSituStretchConstraint>(fem) + 1);
		szname = szbuf;
	}

	// create a new constraint
	FSInSituStretchConstraint* pi = new FSInSituStretchConstraint(&fem);
	pi->SetName(szname);
	pstep->AddComponent(pi);

	// read parameters
	ReadParameters(*pi, tag);
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParsePrestrainConstraint(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// make sure there is something to read
	if (tag.isempty()) return;

	// get the name
	char szbuf[256];
	const char* szname = tag.AttributeValue("name", true);
	if (szname == 0)
	{
		sprintf(szbuf, "PrestrainConstraint%02d", CountConstraints<FSPrestrainConstraint>(fem) + 1);
		szname = szbuf;
	}

	// create a new constraint
	FSPrestrainConstraint* pi = new FSPrestrainConstraint(&fem);
	pi->SetName(szname);
	pstep->AddComponent(pi);

	// read parameters
	ReadParameters(*pi, tag);
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseNrmlFldVlctSrf(FSStep* pstep, XMLTag& tag)
{
    FEBioInputModel& febio = GetFEBioModel();
    FSModel& fem = GetFSModel();
    
    // make sure there is something to read
    if (tag.isempty()) return;
    
    // get the (optional) contact name
    char szbuf[256];
    const char* szname = tag.AttributeValue("name", true);
    if (szname == 0)
    {
        sprintf(szbuf, "NormalFlowSurface%02d", CountConstraints<FSNormalFlowSurface>(fem)+1);
        szname = szbuf;
    }
    
    // find the surface
    const char* szsurf = tag.AttributeValue("surface");
    FSSurface* psurf = febio.FindNamedSurface(szsurf);
    if (psurf == 0) throw XMLReader::InvalidAttributeValue(tag, "surface", szsurf);
    
    // create a new constrained normal fluid flow surface
    FSNormalFlowSurface* pi = new FSNormalFlowSurface(&fem, pstep->GetID());
    pi->SetName(szname);
    pi->SetItemList(psurf);
    pstep->AddComponent(pi);
    
    // read parameters
    ReadParameters(*pi, tag);
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseFrictionlessFluidWall(FSStep* pstep, XMLTag& tag)
{
    FEBioInputModel& febio = GetFEBioModel();
    FSModel& fem = GetFSModel();

    // make sure there is something to read
    if (tag.isempty()) return;

    // get the (optional) contact name
    char szbuf[256];
    const char* szname = tag.AttributeValue("name", true);
    if (szname == 0)
    {
        sprintf(szbuf, "FrictionlessFluidWall%02d", CountConstraints<FSFrictionlessFluidWall>(fem)+1);
        szname = szbuf;
    }

    // find the surface
    const char* szsurf = tag.AttributeValue("surface");
    FSSurface* psurf = febio.FindNamedSurface(szsurf);
    if (psurf == 0) throw XMLReader::InvalidAttributeValue(tag, "surface", szsurf);

    // create a new frictionless fluid wall
    FSFrictionlessFluidWall* pi = new FSFrictionlessFluidWall(&fem, pstep->GetID());
    pi->SetName(szname);
    pi->SetItemList(psurf);
    pstep->AddComponent(pi);

    // read parameters
    ReadParameters(*pi, tag);
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseFixedNormalDisplacement(FSStep* pstep, XMLTag& tag)
{
    FEBioInputModel& febio = GetFEBioModel();
    FSModel& fem = GetFSModel();
    
    // make sure there is something to read
    if (tag.isempty()) return;
    
    // get the (optional) contact name
    char szbuf[256];
    const char* szname = tag.AttributeValue("name", true);
    if (szname == 0)
    {
        sprintf(szbuf, "FixedNormalDisplacement%02d", CountConstraints<FSFixedNormalDisplacement>(fem)+1);
        szname = szbuf;
    }
    
    // find the surface
    const char* szsurf = tag.AttributeValue("surface");
    FSSurface* psurf = febio.FindNamedSurface(szsurf);
    if (psurf == 0) throw XMLReader::InvalidAttributeValue(tag, "surface", szsurf);
    
    // create a new fixed normal displacement
    FSFixedNormalDisplacement* pi = new FSFixedNormalDisplacement(&fem, pstep->GetID());
    pi->SetName(szname);
    pi->SetItemList(psurf);
    pstep->AddComponent(pi);
    
    // read parameters
    ReadParameters(*pi, tag);
}

//=============================================================================
//
//                                L O A D D A T A 
//
//=============================================================================

//-----------------------------------------------------------------------------
//  This function reads the load data section from the xml file
//
bool FEBioFormat3::ParseLoadDataSection(XMLTag& tag)
{
	// make sure the section is not empty
	if (tag.isleaf()) return true;

	// read all load controllers
	++tag;
	do
	{
		if (tag == "load_controller") ParseLoadController(tag);
		else ParseUnknownTag(tag);
		++tag;
	} while (!tag.isend());

	return true;
}

bool FEBioFormat3::ParseLoadController(XMLTag& tag)
{
	if (tag.isleaf()) return true;

	// get the type attribute
	const char* sztype = tag.AttributeValue("type");

	// create the load controller
	FSModel& fem = GetFSModel();
	FSLoadController* plc = FEBio::CreateLoadController(sztype, &fem);
	if (plc == nullptr)
	{
		ParseUnknownTag(tag);
		return false;
	}

	std::string name;
	const char* szname = tag.AttributeValue("name", true);
	if (szname) name = szname;
	else
	{
		int n = fem.LoadControllers();
		std::stringstream ss;
		ss << "LC" << n + 1;
		name = ss.str();
	}
	plc->SetName(name);

	fem.AddLoadController(plc);

	ParseModelComponent(plc, tag);

	return true;
}

bool FEBioFormat3::ParseLoadCurve(XMLTag& tag, LoadCurve& lc)
{
	int nid = lc.GetID();

	++tag;
	do
	{
		if (tag == "interpolate")
		{
			string interpolate = tag.szvalue();
			if      ((interpolate == "step"          ) || (interpolate == "STEP"          )) lc.SetInterpolator(PointCurve::STEP);
			else if ((interpolate == "linear"        ) || (interpolate == "LINEAR"        )) lc.SetInterpolator(PointCurve::LINEAR);
			else if ((interpolate == "smooth"        ) || (interpolate == "SMOOTH"        )) lc.SetInterpolator(PointCurve::SMOOTH);
			else if ((interpolate == "cubic spline"  ) || (interpolate == "CUBIC SPLINE"  )) lc.SetInterpolator(PointCurve::CSPLINE);
			else if ((interpolate == "control points") || (interpolate == "CONTROL POINTS")) lc.SetInterpolator(PointCurve::CPOINTS);
			else if ((interpolate == "approximation" ) || (interpolate == "APPROXIMATION" )) lc.SetInterpolator(PointCurve::APPROX);
			else if ((interpolate == "smooth step"   ) || (interpolate == "SMOOTH STEP"   )) lc.SetInterpolator(PointCurve::SMOOTH_STEP);
            else if ((interpolate == "C2-smooth"     ) || (interpolate == "C2-SMOOTH"     )) lc.SetInterpolator(PointCurve::C2SMOOTH);
			else FileReader()->AddLogEntry("unknown interpolation type for loadcurve %d (line %d)", nid, tag.m_nstart_line);
		}
		else if (tag == "extend")
		{
			string extend = tag.szvalue();
			if      ((extend == "constant"     ) || (extend == "CONSTANT"     )) lc.SetExtendMode(PointCurve::CONSTANT);
			else if ((extend == "extrapolate"  ) || (extend == "EXTRAPOLATE"  )) lc.SetExtendMode(PointCurve::EXTRAPOLATE);
			else if ((extend == "repeat"       ) || (extend == "REPEAT"       )) lc.SetExtendMode(PointCurve::REPEAT);
			else if ((extend == "repeat offset") || (extend == "REPEAT OFFSET")) lc.SetExtendMode(PointCurve::REPEAT_OFFSET);
			else FileReader()->AddLogEntry("unknown extend mode for loadcurve %d (line %d)", nid, tag.m_nstart_line);
		}
		else if (tag == "points")
		{
			// read the points
			++tag;
			do
			{
				double d[2];
				tag.value(d, 2);
				lc.Add(d[0], d[1]);
				++tag;
			} while (!tag.isend());
		}
		++tag;
	}
	while (!tag.isend());
    
    lc.Update();

	return true;
}

//=============================================================================
//
//                                S T E P
//
//=============================================================================

//-----------------------------------------------------------------------------
bool FEBioFormat3::ParseStepSection(XMLTag &tag)
{
	// make sure the section is not empty
	if (tag.isleaf()) return true;

	++tag;
	do
	{
		if (tag == "step") ParseStep(tag);
		++tag;
	}
	while (!tag.isend());

	return true;
}

bool FEBioFormat3::ParseStep(XMLTag& tag)
{
	char szname[128] = { 0 };

	const char* szval = tag.AttributeValue("name", true);
	if (szval) strcpy(szname, szval);

	++tag;

	// make sure the analysis flag was defined
	if (m_nAnalysis < 0) return false;

	// create a new step (unless this is the first step)
	if (m_pstep == 0) m_pstep = NewStep(GetFSModel(), m_nAnalysis, szname);
	m_pBCStep = m_pstep;

	do
	{
		if      (tag == "Control"    ) ParseControlSection    (tag);
		else if (tag == "Boundary"   ) ParseBoundarySection   (tag);
		else if (tag == "Constraints") ParseConstraintSection (tag);
		else if (tag == "Loads"      ) ParseLoadsSection      (tag);
		else if (tag == "Contact"    ) ParseContactSection    (tag);
		else if (tag == "Rigid"      ) ParseRigidSection      (tag);
		else if (tag == "MeshAdaptor") ParseMeshAdaptorSection(tag);
		else ParseUnknownTag(tag);

		// go to the next tag
		++tag;
	} 
	while (!tag.isend());

	// set step to zero so that we'll create a new step in the next Step section
	m_pstep = 0;
	m_pBCStep = 0;

	return true;
}
