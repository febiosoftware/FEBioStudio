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
#include "FEBioFormat3.h"
#include <FEMLib/FERigidConstraint.h>
#include <MeshTools/FEGroup.h>
#include <GeomLib/GMeshObject.h>
#include <FEMLib/FEInitialCondition.h>
#include <FEMLib/FEBodyLoad.h>
#include <FEMLib/FEModelConstraint.h>
#include <MeshTools/GDiscreteObject.h>
#include <MeshTools/FEElementData.h>
#include <MeshTools/FESurfaceData.h>
#include <MeshTools/FENodeData.h>
#include <MeshTools/GModel.h>
#include <FEBioLink/FEBioInterface.h>
#include <FEBioLink/FEBioModule.h>
#include <assert.h>
#include <sstream>
using namespace std;

#define CREATE_SURFACE_LOAD(className) createNewSurfaceLoad(new className(&fem), #className, CountLoads<className>(fem))

//-----------------------------------------------------------------------------
FESurfaceLoad* createNewSurfaceLoad(FESurfaceLoad* psl, const char* szclass, int N)
{
	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "%s%d", szclass + 2, N + 1);
	psl->SetName(szname);
	return psl;
}

//-----------------------------------------------------------------------------
static vector<string> GetDOFList(string sz)
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

static int GetDOFDir(vector<string> sz)
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

static int GetROTDir(vector<string> sz)
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

static bool validate_dof(string bc)
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
FEBioFormat3::FEBioFormat3(FEBioImport* fileReader, FEBioModel& febio) : FEBioFormat(fileReader, febio)
{
	m_geomFormat = 0;
}

FEBioFormat3::~FEBioFormat3()
{
}

FEBioModel::Part* FEBioFormat3::DefaultPart()
{
	FEBioModel& febio = GetFEBioModel();
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
	bool ret = true;
	if (m_geomOnly)
	{
		if		(tag == "Mesh"       ) ret = ParseMeshSection(tag);
		else if (tag == "MeshDomains") ret = ParseMeshDomainsSection(tag);
		else if (tag == "MeshData"   ) ret = ParseMeshDataSection(tag);
		else tag.m_preader->SkipTag(tag);
	}
	else
	{
		if      (tag == "Module"     ) ret = ParseModuleSection     (tag);
		else if (tag == "Control"    ) ret = ParseControlSection    (tag);
		else if (tag == "Material"   ) ret = ParseMaterialSection   (tag);
		else if (tag == "Mesh"       ) ret = ParseMeshSection       (tag);
		else if (tag == "MeshDomains") ret = ParseMeshDomainsSection(tag);
		else if (tag == "MeshData"   ) ret = ParseMeshDataSection   (tag);
		else if (tag == "Boundary"   ) ret = ParseBoundarySection   (tag);
		else if (tag == "Constraints") ret = ParseConstraintSection (tag);
		else if (tag == "Loads"      ) ret = ParseLoadsSection      (tag);
		else if (tag == "Contact"    ) ret = ParseContactSection    (tag);
		else if (tag == "Discrete"   ) ret = ParseDiscreteSection   (tag);
		else if (tag == "Initial"    ) ret = ParseInitialSection    (tag);
		else if (tag == "Rigid"      ) ret = ParseRigidSection      (tag);
		else if (tag == "Globals"    ) ret = ParseGlobalsSection    (tag);
		else if (tag == "LoadData"   ) ret = ParseLoadDataSection   (tag);
		else if (tag == "Output"     ) ret = ParseOutputSection     (tag);
		else if (tag == "Step"       ) ret = ParseStepSection       (tag);
		else return false;
	}
	
	return ret;
}

//-----------------------------------------------------------------------------
// Parse the Module section
bool FEBioFormat3::ParseModuleSection(XMLTag &tag)
{
	m_nAnalysis = -1;
	XMLAtt& atype = tag.Attribute("type");
	m_nAnalysis = FEBio::GetModuleId(atype.cvalue());
	if (m_nAnalysis < 0) { throw XMLReader::InvalidAttributeValue(tag, "type", atype.cvalue()); }
	FEBio::SetActiveModule(m_nAnalysis);
	FileReader()->GetProject().SetModule(m_nAnalysis);
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

	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	// create a new analysis step from these control settings
	if (m_pstep == 0) m_pstep = NewStep(fem, m_nAnalysis);
	FEAnalysisStep* pstep = dynamic_cast<FEAnalysisStep*>(m_pstep);
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
				else if (strcmp(sz, "PLOT_AUGMENTATIONS") == 0) ops.plot_level = FE_PLOT_AUGMENTS;
				else if (strcmp(sz, "PLOT_STEP_FINAL"   ) == 0) ops.plot_level = FE_PLOT_STEP_FINAL;
				else
				{
					FileReader()->AddLogEntry("unknown plot_level (line %d)", tag.currentLine());
				}
			}
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
		if ((ntype == FE_STEP_BIPHASIC) || (ntype == FE_STEP_BIPHASIC_SOLUTE) || (ntype == FE_STEP_MULTIPHASIC) || (ntype == FE_STEP_FLUID) || (ntype == FE_STEP_FLUID_FSI)) ops.nanalysis = FE_DYNAMIC;
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
		FELoadCurve* plc = pstep->GetMustPointLoadCurve();
		febio.AddParamCurve(plc, nmplc - 1);
	}
	else ops.bmust = false;

	return true;
}

//=============================================================================
//
//                                M A T E R I A L
//
//=============================================================================

//-----------------------------------------------------------------------------
//  This function parses the material section from the xml file
//
bool FEBioFormat3::ParseMaterialSection(XMLTag& tag)
{
	char szname[256] = { 0 };

	// make sure the section is not empty
	if (tag.isleaf()) return true;

	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	++tag;
	do
	{
		// get the material type
		XMLAtt& mtype = tag.Attribute("type");
		const char* sztype = mtype.cvalue();

		// get the material name
		XMLAtt* pan = tag.AttributePtr("name");
		if (pan) strcpy(szname, pan->cvalue());
		else sprintf(szname, "Material%02d", febio.Materials() + 1);

		// get the comment
		std::string comment = tag.comment();

		// allocate a new material
		FEMaterial* pmat = FEBio::CreateMaterial(sztype, &fem);
		if (pmat == nullptr)
		{
			ParseUnknownTag(tag);
			return true;
		}

		// see if a material already exists with this name
		GMaterial* gmat = fem.FindMaterial(szname);
		if (gmat)
		{
			FileReader()->AddLogEntry("Material with name \"%s\" already exists.", szname);

			string oldName = szname;
			int n = 2;
			while (gmat)
			{
				sprintf(szname, "%s(%d)", oldName.c_str(), n++);
				gmat = fem.FindMaterial(szname);
			}
		}

		// parse material
		ParseMaterial(tag, pmat);

		// if pmat is set we need to add the material to the list
		gmat = new GMaterial(pmat);
		gmat->SetName(szname);
		gmat->SetInfo(comment);
		febio.AddMaterial(gmat);
		fem.AddMaterial(gmat);

		++tag;
	} while (!tag.isend());

	return true;
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseMaterial(XMLTag& tag, FEMaterial* pmat)
{
	// first, process potential attribute parameters
	// (e.g. for solutes)
	for (int i = 0; i < tag.m_natt; ++i)
	{
		XMLAtt& att = tag.m_att[i];
		Param* param = pmat->GetParam(att.m_sztag);
		if (param)
		{
			switch (param->GetParamType())
			{
			case Param_INT:
			{
				int n = atoi(att.m_szval);
				param->SetIntValue(n);
			}
			break;
			case Param_CHOICE:
			{
				if (param->GetEnumNames())
				{
					// TODO: This is hack for reading solute IDs.
					int n = atoi(att.m_szval);
					param->SetIntValue(n - 1);
				}
			}
			break;
			default:
				assert(false);
			}
		}
	}

	if (tag.isleaf()) return;

	// read the tags
	++tag;
	do
	{
		if (ReadParam(*pmat, tag) == false)
		{
			if (pmat->Properties() > 0)
			{
				const char* sztag = tag.Name();
				FEMaterialProperty* pmc = pmat->FindProperty(sztag);
				if (pmc == nullptr)
				{
					ParseUnknownTag(tag);
				}
				else
				{
					// see if this is a material property
					const char* sztype = tag.AttributeValue("type", true);
					if (sztype == 0)
					{
						const std::string& defType = pmc->GetDefaultType();
						if (defType.empty() == false) sztype = defType.c_str();
						else sztype = tag.Name();
					}

					FEBioMaterial* propMat = new FEBioMaterial;
					FEBio::CreateMaterialProperty(pmc->GetSuperClassID(), sztype, propMat);

					if (pmc)
					{
						pmc->AddMaterial(propMat);
						ParseMaterial(tag, propMat);
					}
				}
			}
			else ParseUnknownTag(tag);

			++tag;
		}
		else ++tag;
	} while (!tag.isend());
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
	FEBioModel& febio = GetFEBioModel();
	FEBioModel::Part* part = DefaultPart();
	part->Update();
	FEBioModel::PartInstance* instance = new FEBioModel::PartInstance(part);
	febio.AddInstance(instance);
	instance->SetName(part->GetName());

	return true;
}

//-----------------------------------------------------------------------------
bool FEBioFormat3::ParseMeshDomainsSection(XMLTag& tag)
{
	// make sure the section is not empty
	if (!tag.isleaf())
	{
		FEBioModel::Part* part = DefaultPart();

		// loop over all sections
		++tag;
		do
		{
			if ((tag == "SolidDomain") || (tag == "ShellDomain"))
			{
				const char* szname = tag.AttributeValue("name");
				const char* szmat = tag.AttributeValue("mat", true);
				if (szmat)
				{
					FEBioModel& febio = GetFEBioModel();
					int matID = febio.GetMaterialIndex(szmat);
					if (matID == -1) matID = atoi(szmat) - 1;

					FEBioModel::Domain* dom = part->FindDomain(szname);
					if (dom) dom->SetMatID(matID);

					if (tag.isleaf() == false)
					{
						++tag;
						do
						{
							if (tag == "shell_normal_nodal")
							{
								if (dom) tag.value(dom->m_bshellNodalNormals);
							}
                            else if (tag == "laugon")
                            {
                                if (dom) tag.value(dom->m_blaugon);
                            }
                            else if (tag == "atol")
                            {
                                if (dom) tag.value(dom->m_augtol);
                            }
							else ParseUnknownTag(tag);
							++tag;
						}
						while (!tag.isend());
					}
				}
			}
			else ParseUnknownTag(tag);
			++tag;
		} while (!tag.isend());
	}

	// don't forget to update the mesh
	GetFEBioModel().UpdateGeometry();
    
    return true;
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseGeometryNodes(FEBioModel::Part* part, XMLTag& tag)
{
	if (part == 0) throw XMLReader::InvalidTag(tag);

	vector<FEBioModel::NODE> nodes; nodes.reserve(10000);

	// create a node set if the name is definde
	const char* szname = tag.AttributeValue("name", true);
	std::string name;
	if (szname) name = szname;

	// read nodal coordinates
	++tag;
	do
	{
		FEBioModel::NODE node;
		tag.value(node.r);
		int nid = tag.AttributeValue<int>("id", -1); assert(nid != -1);
		node.id = nid;

		nodes.push_back(node);
		++tag;
	} while (!tag.isend());

	// create nodes
	int nn = (int)nodes.size();
	FEMesh& mesh = *part->GetFEMesh();
	int N0 = mesh.Nodes();
	mesh.Create(N0 + nn, 0);

	for (int i = 0; i < nn; ++i)
	{
		FEBioModel::NODE& nd = nodes[i];
		FENode& node = mesh.Node(N0 + i);
		node.m_ntag = nd.id;
		node.r = nd.r;
	}

	// create the nodeset 
	if (name.empty() == false)
	{
		vector<int> nodeList(nn);
		for (int i = 0; i < nn; ++i) nodeList[i] = nodes[i].id - 1;
		FEBioModel::NodeSet nset(name, nodeList);
		part->AddNodeSet(nset);
	}
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseGeometryElements(FEBioModel::Part* part, XMLTag& tag)
{
	if (part == 0) throw XMLReader::InvalidTag(tag);

	// first we need to figure out how many elements there are
	int elems = tag.children();

	// get the required type attribute
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
	else throw XMLReader::InvalidTag(tag);

	// get the optional material attribute
	const char* szmat = tag.AttributeValue("mat", true);
	int matID = -1;
	if (szmat)
	{
		FEBioModel& febio = GetFEBioModel();
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
	FEBioModel::Domain* dom = part->AddDomain(name, matID);
	dom->m_bshellNodalNormals = GetFEBioModel().m_shellNodalNormals;

	// create elements
	FEMesh& mesh = *part->GetFEMesh();
	int NTE = mesh.Elements();
	mesh.Create(0, elems + NTE);

	// generate the part id
	int pid = part->Domains() - 1;

	// read element data
	++tag;
	vector<int> elemSet; elemSet.reserve(elems);
	for (int i = NTE; i<elems + NTE; ++i)
	{
		FEElement& el = mesh.Element(i);
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
	FEBioModel::ElementSet* set = new FEBioModel::ElementSet(szname, elemSet);
	part->AddElementSet(*set);
}


//-----------------------------------------------------------------------------
void FEBioFormat3::ParseGeometryNodeSet(FEBioModel::Part* part, XMLTag& tag)
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
			list.push_back(nid - 1);
		}
		else if (tag == "node_set")
		{
			const char* szset = tag.szvalue();
			if (part)
			{
				FEBioModel::NodeSet* ps = part->FindNodeSet(szset);
				if (ps == 0) throw XMLReader::InvalidValue(tag);
				list.insert(list.end(), ps->nodeList().begin(), ps->nodeList().end());
			}
		}
		else throw XMLReader::InvalidTag(tag);
		++tag;
	}
	while (!tag.isend());

	// create a new node set
	part->AddNodeSet(FEBioModel::NodeSet(name, list));
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseGeometryDiscreteSet(FEBioModel::Part* part, XMLTag& tag)
{
	if (tag.isempty()) return;
	if (part == 0) throw XMLReader::InvalidTag(tag);

	FEBioModel::DiscreteSet ds;
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
			ds.Add(n[0] - 1, n[1] - 1);
		}
		else ParseUnknownTag(tag);
		++tag;
	} while (!tag.isend());

	part->AddDiscreteSet(ds);
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseGeometrySurfacePair(FEBioModel::Part* part, XMLTag& tag)
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

	part->AddSurfacePair(FEBioModel::SurfacePair(name, surf2, surf1));
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseGeometrySurface(FEBioModel::Part* part, XMLTag& tag)
{
	if (part == 0) throw XMLReader::InvalidTag(tag);

	// get the name
	const char* szname = tag.AttributeValue("name");

	// see if a surface with this name is already defined
	// if found, we'll continue, but we'll generate a warning.
	FEBioModel::Surface* ps = part->FindSurface(szname);
	if (ps) FileReader()->AddLogEntry("A surface named %s is already defined.", szname);

	// create a new surface
	FEBioModel::Surface s;
	s.m_name = szname;

	if (tag.isleaf() == false)
	{
		// read the surface data
		int nf[FEElement::MAX_NODES], N;
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
			else throw XMLReader::InvalidTag(tag);

			// read the node numbers
			tag.value(nf, N);

			// make zero-based
			vector<int> node(N);
			for (int j = 0; j < N; ++j) node[j] = nf[j] - 1;
			s.m_face.push_back(node);

			++tag;
		} while (!tag.isend());
	}

	part->AddSurface(s);
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseGeometryElementSet(FEBioModel::Part* part, XMLTag& tag)
{
	if (part == 0) throw XMLReader::InvalidTag(tag);

	// get the name
	const char* szname = tag.AttributeValue("name");
	if (szname == 0) FileReader()->AddLogEntry("Element set defined without a name.");
	string sname = (szname ? szname : "");

	// see if a set with this name is already defined
	// if found, we'll continue, but we'll generate a warning.
	FEBioModel::ElementSet* ps = part->FindElementSet(szname);
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

	part->AddElementSet(FEBioModel::ElementSet(sname, elem));
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseGeometryPart(XMLTag& tag)
{
	const char* szname = tag.AttributeValue("name");
	if (szname == 0) throw XMLReader::InvalidAttributeValue(tag, "name", szname);

	// create a new object with this name
	FEBioModel& febio = GetFEBioModel();
	FEBioModel::Part* part = febio.AddPart(szname);

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
	FEBioModel& febio = GetFEBioModel();
	FEBioModel::Part* part = febio.FindPart(szpart);
	if (part == 0) throw XMLReader::InvalidAttributeValue(tag, "part", szpart);

	// create a new instance
	FEBioModel::PartInstance* instance = new FEBioModel::PartInstance(part);
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
	FEBioModel& febio = GetFEBioModel();
	for (int i=0; i<febio.Instances(); ++i)
	{
		FEBioModel::PartInstance* instance = febio.GetInstance(i);
		FEMesh* pdst = instance->GetMesh();
		FEMesh* psrc = instance->GetPart()->GetFEMesh();

		assert(pdst->Elements()==psrc->Elements());
		for (int j=0; j<pdst->Elements(); ++j)
		{
			FEElement& e0 = pdst->Element(j);
			FEElement& e1 = psrc->Element(j);

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
	FEBioModel& feb = GetFEBioModel();

	XMLAtt* name = tag.AttributePtr("name");
	XMLAtt* dataTypeAtt = tag.AttributePtr("data_type");
	XMLAtt* nset = tag.AttributePtr("node_set");

	FEMeshData::DATA_TYPE dataType;
	if (dataTypeAtt)
	{
		if      (*dataTypeAtt == "scalar") dataType = FEMeshData::DATA_TYPE::DATA_SCALAR;
//		else if (*dataTypeAtt == "vector") dataType = FEMeshData::DATA_TYPE::DATA_VEC3D;
		else return false;
	}
	else dataType = FEMeshData::DATA_TYPE::DATA_SCALAR;

	FENodeSet* nodeSet = feb.BuildFENodeSet(nset->cvalue());
	FEMesh* feMesh = nodeSet->GetMesh();

	FENodeData* nodeData = feMesh->AddNodeDataField(name->cvalue(), nodeSet, dataType);

	double val;
	int lid;
	++tag;
	do
	{
		tag.AttributePtr("lid")->value(lid);
		tag.value(val);

		nodeData->set(lid - 1, val);

		++tag;
	}
	while (!tag.isend());

	return true;

}

bool FEBioFormat3::ParseSurfaceDataSection(XMLTag& tag)
{
	FEBioModel& feb = GetFEBioModel();

	XMLAtt* name = tag.AttributePtr("name");
	XMLAtt* dataTypeAtt = tag.AttributePtr("data_type");
	XMLAtt* surf = tag.AttributePtr("surface");

	FEMeshData::DATA_TYPE dataType;
	if (dataTypeAtt)
	{
		if      (*dataTypeAtt == "scalar") dataType = FEMeshData::DATA_TYPE::DATA_SCALAR;
		else if (*dataTypeAtt == "vector") dataType = FEMeshData::DATA_TYPE::DATA_VEC3D;
		else return false;
	}
	else dataType = FEMeshData::DATA_TYPE::DATA_SCALAR;

	FESurface* feSurf = feb.BuildFESurface(surf->cvalue());
	FEMesh* feMesh = feSurf->GetMesh();

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
		FEBioModel& feb = GetFEBioModel();

		FEBioModel::Domain* dom = feb.FindDomain(szset);
		if (dom)
		{
			FEMesh* mesh = dom->GetPart()->GetFEMesh();

			double h[FEElement::MAX_NODES] = { 0 };
			++tag;
			do
			{
				int m = tag.value(h, FEElement::MAX_NODES);
				int lid = tag.AttributeValue<int>("lid", 0) - 1;
				if (lid >= 0)
				{
					int id = dom->ElementID(lid);
					FEElement& el = mesh->Element(id);

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
		FEBioModel& feb = GetFEBioModel();

		FEBioModel::Domain* dom = feb.FindDomain(szset);
		if (dom)
		{
			FEMesh* mesh = dom->GetPart()->GetFEMesh();

			vec3d a;
			++tag;
			do
			{
				int lid = tag.AttributeValue<int>("lid", 0) - 1;
				if (lid >= 0)
				{
					int id = dom->ElementID(lid);
					FEElement& el = mesh->Element(id);
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
		FEBioModel& feb = GetFEBioModel();

		FEBioModel::Domain* dom = feb.FindDomain(szset);
		if (dom)
		{
			FEMesh* mesh = dom->GetPart()->GetFEMesh();

			vec3d a, b, c, d;
			++tag;
			do
			{
				int lid = tag.AttributeValue<int>("lid", 0) - 1;
				if (lid >= 0)
				{
					int id = dom->ElementID(lid);
					FEElement& el = mesh->Element(id);

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
	else if (var == nullptr)
	{
		FEBioModel& feb = GetFEBioModel();

		XMLAtt* name = tag.AttributePtr("name");
		XMLAtt* dataTypeAtt = tag.AttributePtr("data_type");
		XMLAtt* set = tag.AttributePtr("elem_set");

		FEMeshData::DATA_TYPE dataType;
		if (dataTypeAtt)
		{
			if (*dataTypeAtt == "scalar") dataType = FEMeshData::DATA_TYPE::DATA_SCALAR;
			else if (*dataTypeAtt == "vector") dataType = FEMeshData::DATA_TYPE::DATA_VEC3D;
			else return false;
		}
		else dataType = FEMeshData::DATA_TYPE::DATA_SCALAR;

		FEPart* pg = feb.BuildFEPart(set->cvalue());
		if (pg == nullptr) throw XMLReader::InvalidAttributeValue(tag, "elem_set", set->cvalue());

		FEMesh* mesh = pg->GetMesh();
		FEElementData* elemData = mesh->AddElementDataField(name->cvalue(), pg, dataType);

		double val;
		int lid;
		++tag;
		do
		{
			tag.AttributePtr("lid")->value(lid);
			tag.value(val);

			(*elemData)[lid - 1] = val;

			++tag;
		} while (!tag.isend());
	}
	else ParseUnknownTag(tag);

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
			else if (type == "fluid rotational velocity")
			{
				ParseBCFluidRotationalVelocity(m_pBCStep, tag);
			}
			else ParseUnknownTag(tag);
		}
		else ParseUnknownTag(tag);
		++tag;
	}
	while (!tag.isend());

	return true;
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseBCFixed(FEStep* pstep, XMLTag &tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	// get the name attribute
	string name;
	const char* szname = tag.AttributeValue("name", true);
	if (szname) name = szname;

	// get the node set
	const char* szset = tag.AttributeValue("node_set");
	FEItemListBuilder* pg = febio.BuildItemList(szset);
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
	FEBoundaryCondition* pbc = nullptr;
	if ((bc == "x") || (bc == "y") || (bc == "z"))
	{
		pbc = FEBio::CreateBoundaryCondition("zero displacement", &fem); assert(pbc);

		// map the dofs
		vector<int> dofList;
		for (int i = 0; i < dofs.size(); ++i)
		{
			string& di = dofs[i];
			if (di == "x") { dofList.push_back(0); }
			if (di == "y") { dofList.push_back(1); }
			if (di == "z") { dofList.push_back(2); }
		}			
		pbc->SetParamVectorInt("dofs", dofList);

		// set the name
		if (name.empty())
		{
			sprintf(szbuf, "FixedDisplacement%02d", CountBCs<FEBioBoundaryCondition>(fem) + 1);
			name = szbuf;
		}
	}
	else if ((bc == "u") || (bc == "v") || (bc == "w"))
	{
		pbc = FEBio::CreateBoundaryCondition("zero rotation", &fem); assert(pbc);

		// map the dofs
		vector<int> dofList;
		for (int i = 0; i < dofs.size(); ++i)
		{
			string& di = dofs[i];
			if (di == "u") { dofList.push_back(0); }
			if (di == "v") { dofList.push_back(1); }
			if (di == "w") { dofList.push_back(2); }
		}
		pbc->SetParamVectorInt("dofs", dofList);

		// set the name
		if (name.empty())
		{
			sprintf(szbuf, "FixedRotation%02d", CountBCs<FEBioBoundaryCondition>(fem) + 1);
			name = szbuf;
		}
	}
	else if (bc == "T")
	{
		pbc = FEBio::CreateBoundaryCondition("zero temperature", &fem); assert(pbc);

		// set the name
		if (name.empty())
		{
			sprintf(szbuf, "ZeroTemperature%02d", CountBCs<FEBioBoundaryCondition>(fem) + 1);
			name = szbuf;
		}
	}
	else if (bc == "p")
	{
		pbc = FEBio::CreateBoundaryCondition("zero fluid pressure", &fem); assert(pbc);

		// set the name
		if (name.empty())
		{
			sprintf(szbuf, "ZeroFluidPressure%02d", CountBCs<FEBioBoundaryCondition>(fem) + 1);
			name = szbuf;
		}
	}
	else if ((bc == "wx") || (bc == "wy") || (bc == "wz"))
	{
		pbc = FEBio::CreateBoundaryCondition("zero fluid velocity", &fem); assert(pbc);

		// map the dofs
		vector<int> dofList;
		for (int i = 0; i < dofs.size(); ++i)
		{
			string& di = dofs[i];
			if (di == "wx") { dofList.push_back(0); }
			if (di == "wy") { dofList.push_back(1); }
			if (di == "wz") { dofList.push_back(2); }
		}
		pbc->SetParamVectorInt("dofs", dofList);

		// set the name
		if (name.empty())
		{
			sprintf(szbuf, "FixedFluidVelocity%02d", CountBCs<FEBioBoundaryCondition>(fem) + 1);
			name = szbuf;
		}
	}
	else if (bc == "ef")
	{
		pbc = FEBio::CreateBoundaryCondition("zero fluid dilatation", &fem); assert(pbc);

		// set the name
		if (name.empty())
		{
			sprintf(szbuf, "FixedFluidDilatation%02d", CountBCs<FEBioBoundaryCondition>(fem) + 1);
			name = szbuf;
		}
	}
	else if ((bc == "sx") || (bc == "sy") || (bc == "sz"))
	{
		pbc = FEBio::CreateBoundaryCondition("zero shell displacement", &fem); assert(pbc);

		// map the dofs
		vector<int> dofList;
		for (int i = 0; i < dofs.size(); ++i)
		{
			string& di = dofs[i];
			if (di == "sx") { dofList.push_back(0); }
			if (di == "sy") { dofList.push_back(1); }
			if (di == "sz") { dofList.push_back(2); }
		}			
		pbc->SetParamVectorInt("dofs", dofList);

		// set the name
		if (name.empty())
		{
			sprintf(szbuf, "FixedDisplacement%02d", CountBCs<FEBioBoundaryCondition>(fem) + 1);
			name = szbuf;
		}
	}
	else if (bc == "c")
	{
		assert(false);
/*		FEFixedConcentration* pbc = new FEFixedConcentration(&fem, pg, 1, pstep->GetID());
		if (name.empty())
		{
			sprintf(szbuf, "FixedConcentration%02d", CountBCs<FEFixedConcentration>(fem) + 1);
			name = szbuf;
		}
*/
	}
	else if (bc.compare(0, 1, "c") == 0)
	{
		pbc = FEBio::CreateBoundaryCondition("zero concentration", &fem); assert(pbc);

		// map the dofs
		vector<int> dofList;
		for (int i = 0; i < dofs.size(); ++i)
		{
			string& di = dofs[i];
			if (di.size() == 2)
			{
				int n = atoi(di.c_str() + 1);
				dofList.push_back(n - 1);
			}
		}
		pbc->GetParam("dofs")->SetVectorIntValue(dofList);

		if (name.empty())
		{
			sprintf(szbuf, "FixedConcentration%02d", CountBCs<FEBioBoundaryCondition>(fem) + 1);
			name = szbuf;
		}
	}

	// assign the name
	pbc->SetName(name);

	// assign the item list
	pbc->SetItemList(pg);

	// add it to the active step
	pstep->AddComponent(pbc);
}

//-----------------------------------------------------------------------------

void FEBioFormat3::ParseBCPrescribed(FEStep* pstep, XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	// get the name attribute
	string name;
	const char* szname = tag.AttributeValue("name", true);
	if (szname) name = szname;

	XMLAtt& set = tag.Attribute("node_set");
	FEItemListBuilder* pg = febio.BuildItemList(set.cvalue());
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
	FEBoundaryCondition* pbc = 0;
	if (bc == "x")
	{
//		pbc = new FEPrescribedDisplacement(&fem, pg, 0, 1, pstep->GetID());
		pbc = FEBio::CreateBoundaryCondition("prescribed displacement", &fem);
		pbc->SetItemList(pg);
		pbc->SetParamInt("dof", 0);
	}
	else if (bc == "y")
	{
//		pbc = new FEPrescribedDisplacement(&fem, pg, 1, 1, pstep->GetID());
		pbc = FEBio::CreateBoundaryCondition("prescribed displacement", &fem);
		pbc->SetItemList(pg);
		pbc->SetParamInt("dof", 1);
	}
	else if (bc == "z")
	{
//		pbc = new FEPrescribedDisplacement(&fem, pg, 2, 1, pstep->GetID());
		pbc = FEBio::CreateBoundaryCondition("prescribed displacement", &fem);
		pbc->SetItemList(pg);
		pbc->SetParamInt("dof", 2);
	}
	else if (bc == "T")
	{
//		pbc = new FEPrescribedTemperature(&fem, pg, 1, pstep->GetID());
		pbc = FEBio::CreateBoundaryCondition("prescribed temperature", &fem);
		pbc->SetItemList(pg);
	}
	else if (bc == "p")
	{
//		pbc = new FEPrescribedFluidPressure(&fem, pg, 1, pstep->GetID());
		pbc = FEBio::CreateBoundaryCondition("prescribed fluid pressure", &fem);
		pbc->SetItemList(pg);
	}
	else if (bc == "vx")
	{
//		pbc = new FEPrescribedFluidVelocity(&fem, pg, 0, 1, pstep->GetID());
		pbc = FEBio::CreateBoundaryCondition("prescribed fluid velocity", &fem);
		pbc->SetItemList(pg);
		pbc->SetParamInt("dof", 0);
	}
	else if (bc == "vy")
	{
//		pbc = new FEPrescribedFluidVelocity(&fem, pg, 1, 1, pstep->GetID());
		pbc = FEBio::CreateBoundaryCondition("prescribed fluid velocity", &fem);
		pbc->SetItemList(pg);
		pbc->SetParamInt("dof", 1);
	}
	else if (bc == "vz")
	{
//		pbc = new FEPrescribedFluidVelocity(&fem, pg, 2, 1, pstep->GetID());
		pbc = FEBio::CreateBoundaryCondition("prescribed fluid velocity", &fem);
		pbc->SetItemList(pg);
		pbc->SetParamInt("dof", 2);
	}
	else if (bc == "wx")
	{
//		pbc = new FEPrescribedFluidVelocity(&fem, pg, 0, 1, pstep->GetID());
		pbc = FEBio::CreateBoundaryCondition("prescribed fluid velocity", &fem);
		pbc->SetItemList(pg);
		pbc->SetParamInt("dof", 0);
	}
	else if (bc == "wy")
	{
//		pbc = new FEPrescribedFluidVelocity(&fem, pg, 1, 1, pstep->GetID());
		pbc = FEBio::CreateBoundaryCondition("prescribed fluid velocity", &fem);
		pbc->SetItemList(pg);
		pbc->SetParamInt("dof", 1);
	}
	else if (bc == "wz")
	{
//		pbc = new FEPrescribedFluidVelocity(&fem, pg, 2, 1, pstep->GetID());
		pbc = FEBio::CreateBoundaryCondition("prescribed fluid velocity", &fem);
		pbc->SetItemList(pg);
		pbc->SetParamInt("dof", 2);
	}
	else if (bc == "ef")
	{
//		pbc = new FEPrescribedFluidDilatation(&fem, pg, 1, pstep->GetID());
		pbc = FEBio::CreateBoundaryCondition("prescribed fluid dilatation", &fem);
		pbc->SetItemList(pg);
	}
	else if (bc == "sx")
	{
//		pbc = new FEPrescribedShellDisplacement(&fem, pg, 0, 1, pstep->GetID());
		pbc = FEBio::CreateBoundaryCondition("prescribed shell displacement", &fem);
		pbc->SetItemList(pg);
		pbc->SetParamInt("dof", 0);
	}
	else if (bc == "sy")
	{
//		pbc = new FEPrescribedShellDisplacement(&fem, pg, 1, 1, pstep->GetID());
		pbc = FEBio::CreateBoundaryCondition("prescribed shell displacement", &fem);
		pbc->SetItemList(pg);
		pbc->SetParamInt("dof", 1);
	}
	else if (bc == "sz")
	{
//		pbc = new FEPrescribedShellDisplacement(&fem, pg, 2, 1, pstep->GetID());
		pbc = FEBio::CreateBoundaryCondition("prescribed shell displacement", &fem);
		pbc->SetItemList(pg);
		pbc->SetParamInt("dof", 2);
	}
	else if (bc == "u")
	{
//		pbc = new FEPrescribedRotation(&fem, pg, 0, 1, pstep->GetID());
		pbc = FEBio::CreateBoundaryCondition("prescribed rotation", &fem);
		pbc->SetItemList(pg);
		pbc->SetParamInt("dof", 0);
	}
	else if (bc == "v")
	{
//		pbc = new FEPrescribedRotation(&fem, pg, 1, 1, pstep->GetID());
		pbc = FEBio::CreateBoundaryCondition("prescribed rotation", &fem);
		pbc->SetItemList(pg);
		pbc->SetParamInt("dof", 1);
	}
	else if (bc == "w")
	{
//		pbc = new FEPrescribedRotation(&fem, pg, 2, 1, pstep->GetID());
		pbc = FEBio::CreateBoundaryCondition("prescribed rotation", &fem);
		pbc->SetItemList(pg);
		pbc->SetParamInt("dof", 2);
	}
    else if (bc.compare(0,1,"c") == 0) {
        int isol;
        sscanf(bc.substr(1).c_str(),"%d",&isol);
 //       pbc = new FEPrescribedConcentration(&fem, pg, isol-1, 1.0, pstep->GetID());
		pbc = FEBio::CreateBoundaryCondition("prescribed concentration", &fem);
		pbc->SetItemList(pg);
		pbc->SetParamInt("dof", isol - 1);
    }

	// get the optional name
	if (name.empty()) name = pg->GetName();
	pbc->SetName(name);
	pstep->AddComponent(pbc);

	// process scale value
	Param* pp = pbc->GetParam("value"); assert(pp);
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

	pbc->SetParamBool("relative", relative);
	
	if (lc != -1) febio.AddParamCurve(pp, lc - 1);
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseBCRigid(FEStep* pstep, XMLTag& tag)
{
	FEModel& fem = GetFEModel();

	// read the name attribute
	string name;
	const char* sz = tag.AttributeValue("name", true);
	if (sz == 0)
	{
		char szbuf[256] = { 0 };
		sprintf(szbuf, "RigidInterface%02d", CountInterfaces<FERigidInterface>(fem)+1);
	}
	else name = string(sz);

	// read node set
	const char* szset = tag.AttributeValue("node_set");
	FEBioModel& febio = GetFEBioModel();
	FEItemListBuilder* pg = febio.BuildItemList(szset);

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
	FERigidInterface* pi = new FERigidInterface(&fem, pmat, pg, pstep->GetID());
	pi->SetName(name.c_str());
	pstep->AddComponent(pi);
}

// TODO: The fluid-rotational velocity is a BC in FEBio, but a surface load in FEBioStudio.
//       Need to create proper boundary condition class for this component.
void FEBioFormat3::ParseBCFluidRotationalVelocity(FEStep* pstep, XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	std::string comment = tag.comment();

	// read the (optional) name
	string name = tag.AttributeValue("name", "");

	// find the surface
	XMLAtt& nodeSet = tag.Attribute("node_set");
	FEItemListBuilder* psurf = febio.BuildItemList(nodeSet.cvalue());
	if (psurf == 0)
	{
		AddLogEntry("Failed creating selection for fluid rotational velocity \"%s\"", name.c_str());
	}

	// create the surface load
	FESurfaceLoad* psl = nullptr;
	XMLAtt& att = tag.Attribute("type");
    if (att == "fluid rotational velocity"     ) psl = CREATE_SURFACE_LOAD(FEFluidRotationalVelocity);
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

//=============================================================================
//
//                                R I G I D
//
//=============================================================================

//-----------------------------------------------------------------------------
static FERigidConstraint* createNewRigidConstraint(FERigidConstraint* prc, const char* szclass, int N)
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
		if      (tag == "rigid_constraint") ParseRigidConstraint(m_pBCStep, tag);
		else if (tag == "rigid_connector" ) ParseRigidConnector (m_pBCStep, tag);
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
void FEBioFormat3::ParseNodeLoad(FEStep* pstep, XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	// get the load curve ID
	XMLAtt& aset = tag.Attribute("node_set");

	// create the node set
	FEItemListBuilder* pg = febio.BuildItemList(aset.cvalue());
	if (pg == 0) FileReader()->AddLogEntry("Cannot find node set \"%s\" (line %d)", aset.cvalue(), tag.m_nstart_line);

	// get the (optional) name
	string name;
	const char* szname = tag.AttributeValue("name", true);
	if (szname == nullptr)
	{
		char szname[256];
		sprintf(szname, "ForceNodeset%02d", CountLoads<FENodalDOFLoad>(fem) + 1);
		name = szname;
	}
	else name = szname;

	// create the nodal load
	FENodalDOFLoad* pbc = new FENodalDOFLoad(&fem, pg, 0, 1, pstep->GetID());
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
			else if (abc == "p") bc = 3;
            else if (abc.compare(0,1,"c") == 0) {
                int isol = 0;
                sscanf(abc.substr(1).c_str(),"%d",&isol);
                bc = isol+3;
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
void FEBioFormat3::ParseSurfaceLoad(FEStep* pstep, XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	std::string comment = tag.comment();

	// find the surface
	XMLAtt& surf = tag.Attribute("surface");
	FESurface* psurf = febio.BuildFESurface(surf.cvalue());
	if (psurf == 0) throw XMLReader::InvalidAttributeValue(tag, surf);

	// get the type attribute
	XMLAtt& att = tag.Attribute("type");

	// read the (optional) name
	stringstream defaultName; defaultName << "SurfaceLoad" << CountLoads<FESurfaceLoad>(fem) + 1;
	string name = tag.AttributeValue("name", defaultName.str());

	// create the surface load
	FESurfaceLoad* psl = FEBio::CreateSurfaceLoad(att.cvalue(), &fem);
	if (psl == nullptr)
	{
		ParseUnknownAttribute(tag, "type");
		return;
	}

	// process the load
	psl->SetName(name);
	psl->SetItemList(psurf);
	psl->SetInfo(comment);
	pstep->AddComponent(psl);

	// read the parameters
	ReadParameters(*psl, tag);
}

//-----------------------------------------------------------------------------
FEBodyLoad* createNewBodyLoad(FEBodyLoad* pbl, const char* szclass, int N)
{
	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "%s%d", szclass + 2, N + 1);
	pbl->SetName(szname);
	return pbl;
}

//-----------------------------------------------------------------------------
//! Parses the body_load section.
void FEBioFormat3::ParseBodyLoad(FEStep* pstep, XMLTag& tag)
{
	FEModel& fem = GetFEModel();

	// read the comment
	std::string comment = tag.comment();

	// read the (optional) name
	stringstream defaultName; defaultName << "BodyLoad" << CountLoads<FEBodyLoad>(fem) + 1;
	string name = tag.AttributeValue("name", defaultName.str());

	// create new body load
	XMLAtt& att = tag.Attribute("type");
	FEBodyLoad* pbl = FEBio::CreateBodyLoad(att.cvalue(), &fem);
	if (pbl == nullptr)
	{
		ParseUnknownAttribute(tag, "type");
		return;
	}

	// process body load
	pbl->SetInfo(comment);
	pbl->SetName(name);
	pstep->AddComponent(pbl);
	ReadParameters(*pbl, tag);
}

//-----------------------------------------------------------------------------
bool FEBioFormat3::ParseInitialSection(XMLTag& tag)
{
	// make sure the section is not empty
	if (tag.isleaf()) return true;

	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	char szname[256] = {0};

	++tag;
	do
	{
		if (tag == "ic")
		{
			const char* sztype = tag.AttributeValue("type");

			char szbuf[64] = { 0 };
			const char* szname = tag.AttributeValue("name", true);

			if (strcmp(sztype, "init_dof") == 0)
			{
				const char* szset = tag.AttributeValue("node_set");
				FEItemListBuilder* pg = febio.BuildItemList(szset);
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
				FEInitialCondition* pic = 0;
				char szname[64] = { 0 };
                if (bc == "T")
                {
					pic = new FEInitTemperature(&fem, pg, val, m_pBCStep->GetID());
					sprintf(szname, "InitialTemperature%02d", CountICs<FEInitTemperature>(fem) + 1);
                }
                else if (bc == "p")
				{
					pic = new FEInitFluidPressure(&fem, pg, val, m_pBCStep->GetID());
					sprintf(szname, "InitialFluidPressure%02d", CountICs<FEInitFluidPressure>(fem) + 1);

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
                    pic = new FEInitShellFluidPressure(&fem, pg, val, m_pBCStep->GetID());
                    sprintf(szname, "InitialShellFluidPressure%02d", CountICs<FEInitShellFluidPressure>(fem) + 1);
                }
                else if (bc == "vx")
                {
					pic = new FENodalVelocities(&fem, pg, vec3d(val, 0, 0), m_pBCStep->GetID());
					sprintf(szname, "InitialVelocity%02d", CountICs<FENodalVelocities>(fem) + 1);
                }
                else if (bc == "vy")
                {
					pic = new FENodalVelocities(&fem, pg, vec3d(0, val, 0), m_pBCStep->GetID());
					sprintf(szname, "InitialVelocity%02d", CountICs<FENodalVelocities>(fem) + 1);
                }
                else if (bc == "vz")
                {
					pic = new FENodalVelocities(&fem, pg, vec3d(0, 0, val), m_pBCStep->GetID());
					sprintf(szname, "InitialVelocity%02d", CountICs<FENodalVelocities>(fem) + 1);
                }
				else if (bc == "svx")
                {
					pic = new FENodalShellVelocities(&fem, pg, vec3d(val, 0, 0), m_pBCStep->GetID());
					sprintf(szname, "InitShellVelocity%02d", CountICs<FENodalShellVelocities>(fem) + 1);
                }
                else if (bc == "svy")
                {
					pic = new FENodalShellVelocities(&fem, pg, vec3d(0, val, 0), m_pBCStep->GetID());
					sprintf(szname, "InitShellVelocity%02d", CountICs<FENodalShellVelocities>(fem) + 1);
                }
                else if (bc == "svz")
                {
					pic = new FENodalShellVelocities(&fem, pg, vec3d(0, 0, val), m_pBCStep->GetID());
					sprintf(szname, "InitShellVelocity%02d", CountICs<FENodalShellVelocities>(fem) + 1);
                }
                else if (bc == "ef")
                {
					pic = new FEInitFluidDilatation(&fem, pg, val, m_pBCStep->GetID());
					sprintf(szname, "InitialFluidDilatation%02d", CountICs<FEInitFluidDilatation>(fem) + 1);
                }
                else if (bc.compare(0,1,"c") == 0)
                {
                    int nsol;
                    sscanf(bc.substr(1).c_str(),"%d",&nsol);
                    pic = new FEInitConcentration(&fem, pg, nsol-1, val, m_pBCStep->GetID());
                    sprintf(szname, "InitConcentration%02d", CountICs<FEInitConcentration>(fem) + 1);
                }
                else if (bc.compare(0,1,"d") == 0)
                {
                    int nsol;
                    sscanf(bc.substr(1).c_str(),"%d",&nsol);
                    pic = new FEInitShellConcentration(&fem, pg, nsol-1, val, m_pBCStep->GetID());
                    sprintf(szname, "InitShellConcentration%02d", CountICs<FEInitShellConcentration>(fem) + 1);
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
				FEItemListBuilder* pg = febio.BuildItemList(szset);
				if (pg == 0) throw XMLReader::MissingTag(tag, "node_set");

				vec3d v(0, 0, 0);
				++tag;
				do
				{
					if (tag == "value") tag.value(v);
					++tag;
				} while (!tag.isend());
				FENodalVelocities* pic = new FENodalVelocities(&fem, pg, v, m_pBCStep->GetID());

				if (szname == nullptr)
				{
					sprintf(szbuf, "InitialVelocity%02d", CountICs<FENodalVelocities>(fem) + 1);
					szname = szbuf;
				}

				pic->SetName(szname);
				m_pBCStep->AddComponent(pic);
			}
			else if (strcmp(sztype, "prestrain") == 0)
			{
				FEInitPrestrain* pip = new FEInitPrestrain(&fem);

				if (szname == nullptr)
				{
					sprintf(szbuf, "InitPrestrain%d", CountConstraints<FEInitPrestrain>(fem) + 1);
					szname = szbuf;
				}
				pip->SetName(szname);
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
void FEBioFormat3::ParseContact(FEStep *pstep, XMLTag &tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel* fem = &febio.GetFEModel();

	// get the contact interface type
	XMLAtt& atype = tag.Attribute("type");

	// check special cases
	if      (atype == "rigid_wall"       ) ParseRigidWall       (pstep, tag);
	else if (atype == "linear constraint") ParseLinearConstraint(pstep, tag);
	else if (atype == "rigid joint"      ) ParseContactJoint    (pstep, tag);
	else
	{
		const char* szpair = tag.AttributeValue("surface_pair");
		FEBioModel::SurfacePair* surfPair = febio.FindSurfacePair(szpair);
		if (surfPair == 0) throw XMLReader::InvalidAttributeValue(tag, "surface_pair", szpair);

		// create a new interfaces
		FEPairedInterface* pci = FEBio::CreatePairedInterface(atype.cvalue(), fem);
		if (pci == nullptr)
		{
			ParseUnknownAttribute(tag, "type");
			return;
		}

		// get the (optional) name
		stringstream ss; ss << "ContactInterface" << CountInterfaces<FEPairedInterface>(*fem) + 1;
		string name = tag.AttributeValue("name", ss.str());
		pci->SetName(name);

		// read the parameters
		ReadParameters(*pci, tag);

		// assign surfaces
		FEBioModel::Part* part = surfPair->GetPart();
		assert(part);
		if (part)
		{
			if (surfPair->masterID() >= 0)
			{
				string name1 = part->GetSurface(surfPair->masterID()).name();
				FESurface* master = febio.BuildFESurface(name1.c_str());
				pci->SetSecondarySurface(master);
			}

			if (surfPair->slaveID() >= 0)
			{
				string name2 = part->GetSurface(surfPair->slaveID()).name();
				FESurface* slave = febio.BuildFESurface(name2.c_str());
				pci->SetPrimarySurface(slave);
			}
		}

		// add to the analysis step
		pstep->AddComponent(pci);
	}
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseRigidWall(FEStep* pstep, XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	// create a new interface
	FEModelConstraint* pci = FEBio::CreateNLConstraint("rigid_wall", &fem);

	// set name
	char szname[256];
	sprintf(szname, "RigidWall%02d", CountInterfaces<FERigidWallInterface>(fem)+1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pci->SetName(szname);

	// get the surface
	const char* szsurf = tag.AttributeValue("surface", true);
	if (szsurf)
	{
		FESurface* psurf = febio.BuildFESurface(szsurf);
		if (psurf == 0) throw XMLReader::InvalidAttributeValue(tag, "surface", szsurf);
		pci->SetItemList(psurf);
	}

	// read parameters
	ReadParameters(*pci, tag);

	// add interface to step
	pstep->AddComponent(pci);
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseContactJoint(FEStep *pstep, XMLTag &tag)
{
	FEModel& fem = GetFEModel();

	FERigidJoint* pi = new FERigidJoint(&fem, pstep->GetID());
	char szname[256];
	sprintf(szname, "RigidJoint%02d", CountInterfaces<FERigidJoint>(fem)+1);
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

	pi->SetFloatValue(FERigidJoint::TOL, tol);
	pi->SetFloatValue(FERigidJoint::PENALTY, pen);
	pi->SetVecValue(FERigidJoint::RJ, rj);

	FEBioModel& febio = GetFEBioModel();

	if (na >= 0) pi->m_pbodyA = febio.GetMaterial(na - 1);
	if (nb >= 0) pi->m_pbodyB = febio.GetMaterial(nb - 1);
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseRigidConstraint(FEStep* pstep, XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	// get the name 
	stringstream ss;
	ss << "RigidConstraint" << CountConnectors<FERigidConstraint>(fem) + 1;
	std::string name = tag.AttributeValue("name", ss.str());

	// get the type attribute
	const char* sztype = tag.AttributeValue("type");

	// check for some special cases
	if      (strcmp(sztype, "fix"      ) == 0) sztype = "rigid_fixed";
	else if (strcmp(sztype, "prescribe") == 0) sztype = "rigid_prescribed";
	else if (strcmp(sztype, "force"    ) == 0) sztype = "rigid_force";

	// allocate class
	FEBioRigidConstraint* pi = new FEBioRigidConstraint(&fem, pstep->GetID());
	if (FEBio::CreateModelComponent(FE_RIGID_CONSTRAINT, sztype, pi) == false)
	{
		throw XMLReader::InvalidAttributeValue(tag, "type", sztype);
	}
	pi->SetName(name);
	pstep->AddRC(pi);

	++tag;
	do
	{
		if (ReadParam(*pi, tag) == false)
		{
			if (tag == "rb")
			{
				int na = -1;
				tag.value(na);
				if (na >= 0) pi->SetMaterialID(febio.GetMaterial(na - 1)->GetID());
			}
			else ParseUnknownTag(tag);
		}
		++tag;
	} while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseRigidConnector(FEStep *pstep, XMLTag &tag)
{
	FEModel& fem = GetFEModel();

	// get the name 
	stringstream ss;
	ss << "RigidConnector" << CountConnectors<FERigidConnector>(fem) + 1;
	std::string name = tag.AttributeValue("name", ss.str());

	// get the type attribute
	const char* sztype = tag.AttributeValue("type");

	// allocate class
	FEBioRigidConnector* pi = new FEBioRigidConnector(&fem, pstep->GetID());
	if (FEBio::CreateModelComponent(FE_RIGID_CONNECTOR, sztype, pi) == false)
	{
		throw XMLReader::InvalidAttributeValue(tag, "type", sztype);
	}
	pi->SetName(name);
	pstep->AddRigidConnector(pi);

	FEBioModel& febio = GetFEBioModel();
	++tag;
	do
	{
		if (ReadParam(*pi, tag) == false)
		{
			if (tag == "body_a") 
			{
				int na = -1;
				tag.value(na);
				if (na >= 0) pi->SetRigidBody1(febio.GetMaterial(na - 1)->GetID());
			}
			else if (tag == "body_b") 
			{
				int nb = -1;
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
void FEBioFormat3::ParseRigidJoint(FEStep* pstep, XMLTag& tag)
{
	FEModel& fem = GetFEModel();

	FERigidJoint* pi = new FERigidJoint(&fem, pstep->GetID());
	char szname[256];
	sprintf(szname, "RigidJoint%02d", CountInterfaces<FERigidJoint>(fem) + 1);
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

	pi->SetFloatValue(FERigidJoint::TOL, tol);
	pi->SetFloatValue(FERigidJoint::PENALTY, pen);
	pi->SetVecValue(FERigidJoint::RJ, rj);

	FEBioModel& febio = GetFEBioModel();

	if (na >= 0) pi->m_pbodyA = febio.GetMaterial(na - 1);
	if (nb >= 0) pi->m_pbodyB = febio.GetMaterial(nb - 1);
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseLinearConstraint(FEStep* pstep, XMLTag& tag)
{
	FEModel& fem = GetFEModel();

	FELinearConstraintSet* pset = new FELinearConstraintSet;
	pstep->AddLinearConstraint(pset);

	// read the linear constraints
	++tag;
	do
	{
		if (tag == "linear_constraint")
		{
			FELinearConstraintSet::LinearConstraint LC;
			FELinearConstraintSet::LinearConstraint::DOF dof;
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

	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();
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
				FELinearSpringMaterial* mat = new FELinearSpringMaterial();
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
				FENonLinearSpringMaterial* mat = new FENonLinearSpringMaterial();
				GDiscreteSpringSet* pg = new GDiscreteSpringSet(&gm);
				pg->SetMaterial(mat);
				pg->SetName(szname);
				fem.GetModel().AddDiscreteObject(pg);
				++tag;
				do
				{
					if (tag == "force")
					{
						double F;
						tag.value(F);
						int lc = tag.AttributeValue<int>("lc", -1);
						// TODO: assign the force to the material
					}
					else ParseUnknownTag(tag);
					++tag;
				} while (!tag.isend());
				set.push_back(pg);
			}
			else if (strcmp(sztype, "Hill") == 0)
			{
				FEHillContractileMaterial* mat = new FEHillContractileMaterial();
				GDiscreteSpringSet* pg = new GDiscreteSpringSet(&gm);
				pg->SetMaterial(mat);
				pg->SetName(szname);
				fem.GetModel().AddDiscreteObject(pg);
				++tag;
				do
				{
					if (ReadParam(*mat, tag) == false)
					{
						FEMaterialProperty* prop = mat->FindProperty(tag.m_sztag);
						FE1DPointFunction* pf1d = dynamic_cast<FE1DPointFunction*>(prop ? prop->GetMaterial(0) : nullptr);
						if (pf1d)
						{
							FELoadCurve lc;
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

			FEBioModel& feb = GetFEBioModel();
			if (feb.BuildDiscreteSet(*ps, szset) == false)
			{
				assert(false);
			}
		}
		else ParseUnknownTag(tag);
		++tag;
	}
	while (!tag.isend());

	return true;
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

	FEStep* pstep = m_pBCStep;

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
			else throw XMLReader::InvalidAttributeValue(tag, "type", sztype);
		}
		else ParseUnknownTag(tag);
		++tag;
	}
	while (!tag.isend());

	return true;
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseVolumeConstraint(FEStep* pstep, XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	// make sure there is something to read
	if (tag.isempty()) return;

	// get the (optional) contact name
	char szbuf[256];
	const char* szname = tag.AttributeValue("name", true);
	if (szname == 0)
	{
		sprintf(szbuf, "VolumeConstraint%02d", CountConstraints<FEVolumeConstraint>(fem)+1);
		szname = szbuf;
	}

	// find the surface
	const char* szsurf = tag.AttributeValue("surface");
	FESurface* psurf = febio.BuildFESurface(szsurf);
	if (psurf == 0) throw XMLReader::InvalidAttributeValue(tag, "surface", szsurf);

	// create a new volume constraint
	FEVolumeConstraint* pi = new FEVolumeConstraint(&fem, pstep->GetID());
	pi->SetName(szname);
	pi->SetItemList(psurf);
	pstep->AddComponent(pi);

	// read parameters
	ReadParameters(*pi, tag);
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseSymmetryPlane(FEStep* pstep, XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	// make sure there is something to read
	if (tag.isempty()) return;

	// get the (optional) contact name
	char szbuf[256];
	const char* szname = tag.AttributeValue("name", true);
	if (szname == 0)
	{
		sprintf(szbuf, "SymmetryPlane%02d", CountConstraints<FESymmetryPlane>(fem)+1);
		szname = szbuf;
	}

	// find the surface
	const char* szsurf = tag.AttributeValue("surface");
	FESurface* psurf = febio.BuildFESurface(szsurf);
	if (psurf == 0) throw XMLReader::InvalidAttributeValue(tag, "surface", szsurf);

	// create a new symmetry plane
	FESymmetryPlane* pi = new FESymmetryPlane(&fem, pstep->GetID());
	pi->SetName(szname);
	pi->SetItemList(psurf);
	pstep->AddComponent(pi);

	// read parameters
	ReadParameters(*pi, tag);
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseInSituStretchConstraint(FEStep* pstep, XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	// make sure there is something to read
	if (tag.isempty()) return;

	// get the name
	char szbuf[256];
	const char* szname = tag.AttributeValue("name", true);
	if (szname == 0)
	{
		sprintf(szbuf, "InSituStretch%02d", CountConstraints<FEInSituStretchConstraint>(fem) + 1);
		szname = szbuf;
	}

	// create a new constraint
	FEInSituStretchConstraint* pi = new FEInSituStretchConstraint(&fem);
	pi->SetName(szname);
	pstep->AddComponent(pi);

	// read parameters
	ReadParameters(*pi, tag);
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParsePrestrainConstraint(FEStep* pstep, XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	// make sure there is something to read
	if (tag.isempty()) return;

	// get the name
	char szbuf[256];
	const char* szname = tag.AttributeValue("name", true);
	if (szname == 0)
	{
		sprintf(szbuf, "PrestrainConstraint%02d", CountConstraints<FEPrestrainConstraint>(fem) + 1);
		szname = szbuf;
	}

	// create a new constraint
	FEPrestrainConstraint* pi = new FEPrestrainConstraint(&fem);
	pi->SetName(szname);
	pstep->AddComponent(pi);

	// read parameters
	ReadParameters(*pi, tag);
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseNrmlFldVlctSrf(FEStep* pstep, XMLTag& tag)
{
    FEBioModel& febio = GetFEBioModel();
    FEModel& fem = GetFEModel();
    
    // make sure there is something to read
    if (tag.isempty()) return;
    
    // get the (optional) contact name
    char szbuf[256];
    const char* szname = tag.AttributeValue("name", true);
    if (szname == 0)
    {
        sprintf(szbuf, "NormalFlowSurface%02d", CountConstraints<FENormalFlowSurface>(fem)+1);
        szname = szbuf;
    }
    
    // find the surface
    const char* szsurf = tag.AttributeValue("surface");
    FESurface* psurf = febio.BuildFESurface(szsurf);
    if (psurf == 0) throw XMLReader::InvalidAttributeValue(tag, "surface", szsurf);
    
    // create a new constrained normal fluid flow surface
    FENormalFlowSurface* pi = new FENormalFlowSurface(&fem, pstep->GetID());
    pi->SetName(szname);
    pi->SetItemList(psurf);
    pstep->AddComponent(pi);
    
    // read parameters
    ReadParameters(*pi, tag);
}

//-----------------------------------------------------------------------------
void FEBioFormat3::ParseFrictionlessFluidWall(FEStep* pstep, XMLTag& tag)
{
    FEBioModel& febio = GetFEBioModel();
    FEModel& fem = GetFEModel();

    // make sure there is something to read
    if (tag.isempty()) return;

    // get the (optional) contact name
    char szbuf[256];
    const char* szname = tag.AttributeValue("name", true);
    if (szname == 0)
    {
        sprintf(szbuf, "FrictionlessFluidWall%02d", CountConstraints<FEFrictionlessFluidWall>(fem)+1);
        szname = szbuf;
    }

    // find the surface
    const char* szsurf = tag.AttributeValue("surface");
    FESurface* psurf = febio.BuildFESurface(szsurf);
    if (psurf == 0) throw XMLReader::InvalidAttributeValue(tag, "surface", szsurf);

    // create a new frictionless fluid wall
    FEFrictionlessFluidWall* pi = new FEFrictionlessFluidWall(&fem, pstep->GetID());
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

	FEBioModel &febio = GetFEBioModel();

	// read all loadcurves
	++tag;
	do
	{
		if (tag == "load_controller")
		{
			// create the loadcurve
			FELoadCurve lc;

			// remove default points
			lc.Clear();

			// get the load curve ID
			int nid = tag.Attribute("id").value<int>();
			lc.SetID(nid);

			ParseLoadCurve(tag, lc);

			febio.AddLoadCurve(lc);
		}
		else ParseUnknownTag(tag);

		++tag;
	}
	while (!tag.isend());

	return true;
}

bool FEBioFormat3::ParseLoadCurve(XMLTag& tag, FELoadCurve& lc)
{
	int nid = lc.GetID();

	++tag;
	do
	{
		if (tag == "interpolate")
		{
			string interpolate = tag.szvalue();
			if      ((interpolate == "step"  ) || (interpolate == "STEP"  )) lc.SetType(FELoadCurve::LC_STEP);
			else if ((interpolate == "linear") || (interpolate == "LINEAR")) lc.SetType(FELoadCurve::LC_LINEAR);
			else if ((interpolate == "smooth") || (interpolate == "SMOOTH")) lc.SetType(FELoadCurve::LC_SMOOTH);
            else if ((interpolate == "cubic spline") || (interpolate == "CUBIC SPLINE")) lc.SetType(FELoadCurve::LC_CSPLINE);
            else if ((interpolate == "control points") || (interpolate == "CONTROL POINTS")) lc.SetType(FELoadCurve::LC_CPOINTS);
            else if ((interpolate == "approximation") || (interpolate == "APPROXIMATION")) lc.SetType(FELoadCurve::LC_APPROX);
			else FileReader()->AddLogEntry("unknown interpolation type for loadcurve %d (line %d)", nid, tag.m_nstart_line);
		}
		else if (tag == "extend")
		{
			string extend = tag.szvalue();
			if      ((extend == "constant"     ) || (extend == "CONSTANT"     )) lc.SetExtend(FELoadCurve::EXT_CONSTANT);
			else if ((extend == "extrapolate"  ) || (extend == "EXTRAPOLATE"  )) lc.SetExtend(FELoadCurve::EXT_EXTRAPOLATE);
			else if ((extend == "repeat"       ) || (extend == "REPEAT"       )) lc.SetExtend(FELoadCurve::EXT_REPEAT);
			else if ((extend == "repeat offset") || (extend == "REPEAT OFFSET")) lc.SetExtend(FELoadCurve::EXT_REPEAT_OFFSET);
			else FileReader()->AddLogEntry("unknown extend mode for loadcurve %d (line %d)", nid, tag.m_nstart_line);
		}
		else if (tag == "points")
		{
			// read the points
			double d[2];
			++tag;
			do
			{
				tag.value(d, 2);

				LOADPOINT pt;
				pt.time = d[0];
				pt.load = d[1];
				lc.Add(pt);

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
	if (m_pstep == 0) m_pstep = NewStep(GetFEModel(), m_nAnalysis, szname);
	m_pBCStep = m_pstep;

	do
	{
		if      (tag == "Control"    ) ParseControlSection   (tag);
		else if (tag == "Boundary"   ) ParseBoundarySection  (tag);
		else if (tag == "Constraints") ParseConstraintSection(tag);
		else if (tag == "Loads"      ) ParseLoadsSection     (tag);
		else if (tag == "Contact"    ) ParseContactSection   (tag);
		else if (tag == "Rigid"      ) ParseRigidSection     (tag);
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

//-----------------------------------------------------------------------------
FEBioModel::DiscreteSet FEBioFormat3::ParseDiscreteSet(XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();

	const char* szset = tag.AttributeValue("dset", true);
	if (szset)
	{
/*		FEBioModel::DiscreteSet* ps = febio.FindDiscreteSet(szset);
		if (ps) return *ps;
		else
*/		{
			FEBioModel::DiscreteSet ds;
			return ds;
		}
	}
	else
	{
		FEBioModel::DiscreteSet ds;
		++tag;
		do
		{
			if (tag == "delem")
			{
				int n[2];
				tag.value(n, 2);
				ds.Add(n[0] - 1, n[1] - 1);
			}
			else ParseUnknownTag(tag);
			++tag;
		} while (!tag.isend());

		return ds;
	}
}
