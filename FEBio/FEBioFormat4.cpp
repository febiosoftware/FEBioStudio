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
#include "FEBioFormat4.h"
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
FEBioFormat4::FEBioFormat4(FEBioImport* fileReader, FEBioModel& febio) : FEBioFormat(fileReader, febio)
{
	m_geomFormat = 0;
}

FEBioFormat4::~FEBioFormat4()
{
}

FEBioModel::Part* FEBioFormat4::DefaultPart()
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

bool FEBioFormat4::ParseSection(XMLTag& tag)
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
		if      (tag == "Module"     ) ParseModuleSection    (tag);
		else if (tag == "Control"    ) ParseControlSection   (tag);
		else if (tag == "Material"   ) ParseMaterialSection  (tag);
		else if (tag == "Mesh"       ) ParseMeshSection      (tag);
		else if (tag == "MeshDomains") ParseMeshDomainsSection(tag);
		else if (tag == "MeshData"   ) ParseMeshDataSection  (tag);
		else if (tag == "Boundary"   ) ParseBoundarySection  (tag);
		else if (tag == "Constraints") ParseConstraintSection(tag);
		else if (tag == "Loads"      ) ParseLoadsSection     (tag);
		else if (tag == "Contact"    ) ParseContactSection   (tag);
		else if (tag == "Discrete"   ) ParseDiscreteSection  (tag);
		else if (tag == "Initial"    ) ParseInitialSection   (tag);
		else if (tag == "Rigid"      ) ParseRigidSection     (tag);
		else if (tag == "Globals"    ) ParseGlobalsSection   (tag);
		else if (tag == "LoadData"   ) ParseLoadDataSection  (tag);
		else if (tag == "Output"     ) ParseOutputSection    (tag);
		else if (tag == "Step"       ) ParseStepSection      (tag);
		else return false;
	}
	
	return true;
}

//-----------------------------------------------------------------------------
// Parse the Module section
bool FEBioFormat4::ParseModuleSection(XMLTag &tag)
{
	m_nAnalysis = -1;
	XMLAtt& atype = tag.Attribute("type");
	m_nAnalysis = FEBio::GetModuleId(atype.cvalue()); assert(m_nAnalysis >= 0);
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
bool FEBioFormat4::ParseControlSection(XMLTag& tag)
{
	// make sure the section is not empty
	if (tag.isleaf()) return true;

	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	// create a new analysis step from these control settings
	FEStep* pstep = nullptr;
	if (m_pstep == 0)
	{
		pstep = new FEBioAnalysisStep(&fem);
		FEBio::CreateStep("analysis", pstep);
		m_pstep = pstep;

		const char* szname = tag.AttributeValue("name", true);
		if ((szname == 0) || (strlen(szname) == 0))
		{
			char sz[256] = { 0 };
			sprintf(sz, "Step%02d", fem.Steps());
			pstep->SetName(sz);
		}
		else pstep->SetName(szname);
		fem.AddStep(pstep);
	}
	else pstep = m_pstep;

	// parse the settings
	++tag;
	do
	{
		if (ReadParam(*pstep, tag) == false)
		{
			if (pstep->ControlProperties() > 0)
			{
				const char* sztag = tag.Name();
				FEStepControlProperty* pc = pstep->FindControlProperty(sztag); assert(pc);

				// see if this is a property
				const char* sztype = tag.AttributeValue("type", true);
				if (sztype == 0)
				{
					sztype = tag.Name();

					// The default solver should be the solver with the same name as the module
					if (strcmp(sztype, "solver") == 0) sztype = FEBio::GetModuleName(m_nAnalysis);
				}

				if (pc->m_prop == nullptr)
				{
					FEStepComponent* psc = new FEStepComponent;
					FEBio::CreateModelComponent(pc->m_nSuperClassId, sztype, psc);
					pc->m_prop = psc;
				}

				// read the parameters
				ReadParameters(*pc->m_prop, tag);
			}
			else ParseUnknownTag(tag);
		}
		++tag;
	} 
	while (!tag.isend());

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
bool FEBioFormat4::ParseMaterialSection(XMLTag& tag)
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
			ParseUnknownAttribute(tag, "type");
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
void FEBioFormat4::ParseMaterial(XMLTag& tag, FEMaterial* pmat)
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
				FEMaterialProperty* pmc = pmat->FindProperty(sztag); assert(pmc);

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
			else ParseUnknownTag(tag);

			++tag;
		}
		else ++tag;
	} 
	while (!tag.isend());
}

//=============================================================================
//
//                                G E O M E T R Y
//
//=============================================================================

//-----------------------------------------------------------------------------
bool FEBioFormat4::ParseMeshSection(XMLTag& tag)
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
bool FEBioFormat4::ParseMeshDomainsSection(XMLTag& tag)
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
void FEBioFormat4::ParseGeometryNodes(FEBioModel::Part* part, XMLTag& tag)
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
void FEBioFormat4::ParseGeometryElements(FEBioModel::Part* part, XMLTag& tag)
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
void FEBioFormat4::ParseGeometryNodeSet(FEBioModel::Part* part, XMLTag& tag)
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
void FEBioFormat4::ParseGeometryDiscreteSet(FEBioModel::Part* part, XMLTag& tag)
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
void FEBioFormat4::ParseGeometrySurfacePair(FEBioModel::Part* part, XMLTag& tag)
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
void FEBioFormat4::ParseGeometrySurface(FEBioModel::Part* part, XMLTag& tag)
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
void FEBioFormat4::ParseGeometryElementSet(FEBioModel::Part* part, XMLTag& tag)
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
void FEBioFormat4::ParseGeometryPart(XMLTag& tag)
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
void FEBioFormat4::ParseGeometryInstance(XMLTag& tag)
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
bool FEBioFormat4::ParseMeshDataSection(XMLTag& tag)
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

bool FEBioFormat4::ParseNodeDataSection(XMLTag& tag)
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

bool FEBioFormat4::ParseSurfaceDataSection(XMLTag& tag)
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

bool FEBioFormat4::ParseElementDataSection(XMLTag& tag)
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
bool FEBioFormat4::ParseBoundarySection(XMLTag& tag)
{
	if (tag.isleaf()) return true;

	++tag;
	do
	{
		if (tag == "bc")
		{
			ParseBC(m_pBCStep, tag);
		}
		else ParseUnknownTag(tag);
		++tag;
	}
	while (!tag.isend());

	return true;
}

//-----------------------------------------------------------------------------
void FEBioFormat4::ParseBC(FEStep* pstep, XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	// get the node set
	XMLAtt& aset = tag.Attribute("node_set");

	// create the node set
	FEItemListBuilder* pg = febio.BuildItemList(aset.cvalue());
	if (pg == 0) FileReader()->AddLogEntry("Unknown node set \"%s\". (line %d)", aset.cvalue(), tag.m_nstart_line);

	// get the type attribute
	const char* sztype = tag.AttributeValue("type");

	// get the (optional) name
	string name;
	const char* szname = tag.AttributeValue("name", true);
	if (szname) name = szname; else name = sztype;

	// create the boundary condition
	FEBoundaryCondition* pbc = FEBio::CreateBoundaryCondition(sztype, &fem);
	if (pbc == nullptr)
	{
		ParseUnknownAttribute(tag, "type");
		return;
	}

	pbc->SetItemList(pg);
	pbc->SetName(name);
	pstep->AddComponent(pbc);

	ReadParameters(*pbc, tag);
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
bool FEBioFormat4::ParseRigidSection(XMLTag& tag)
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
bool FEBioFormat4::ParseLoadsSection(XMLTag& tag)
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
void FEBioFormat4::ParseNodeLoad(FEStep* pstep, XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	// get the load curve ID
	XMLAtt& aset = tag.Attribute("node_set");

	// create the node set
	FEItemListBuilder* pg = febio.BuildItemList(aset.cvalue());
	if (pg == 0) throw XMLReader::InvalidAttributeValue(tag, aset);
	char szbuf[256];
	sprintf(szbuf, "NodalLoadSet%02d", CountLoads<FENodalLoad>(fem)+1);
	pg->SetName(szbuf);

	// get the (optional) name
	string name;
	const char* szname = tag.AttributeValue("name", true);
	if (szname == nullptr)
	{
		char szname[256];
		sprintf(szname, "NodalLoad%02d", CountLoads<FENodalLoad>(fem) + 1);
		name = szname;
	}
	else name = szname;

	const char* sztype = tag.AttributeValue("type");

	// create the nodal load
	FENodalLoad* pnl = FEBio::CreateNodalLoad(sztype, &fem);
	if (pnl == nullptr)
	{
		ParseUnknownAttribute(tag, "type");
		return;
	}
	pnl->SetName(name);
	pstep->AddComponent(pnl);

	ReadParameters(*pnl, tag);
}

//-----------------------------------------------------------------------------
//! Parses the surface_load section.
void FEBioFormat4::ParseSurfaceLoad(FEStep* pstep, XMLTag& tag)
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
//! Parses the body_load section.
void FEBioFormat4::ParseBodyLoad(FEStep* pstep, XMLTag& tag)
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
	if (name.empty() == false) pbl->SetName(name);
	pstep->AddComponent(pbl);
	ReadParameters(*pbl, tag);
}

//-----------------------------------------------------------------------------
bool FEBioFormat4::ParseInitialSection(XMLTag& tag)
{
	// make sure the section is not empty
	if (tag.isleaf()) return true;

	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	++tag;
	do
	{
		if (tag == "ic")
		{
			// get the type attribute
			const char* sztype = tag.AttributeValue("type");

			// get the (optional) name
			char szbuf[64] = { 0 };
			const char* szname = tag.AttributeValue("name", true);
			if (szname == nullptr)
			{
				sprintf(szbuf, "IC%d", CountICs<FEInitialCondition>(fem) + 1);
				szname = szbuf;
			}

			// allocate initial condition
			FEInitialCondition* pic = FEBio::CreateInitialCondition(sztype, &fem); assert(pic);
			if (pic == nullptr)
			{
				ParseUnknownTag(tag);
			}
			else
			{
				// get the node set
				const char* szset = tag.AttributeValue("node_set");
				FEItemListBuilder* pg = febio.BuildItemList(szset);
				if (pg == 0) AddLogEntry("Failed to create nodeset %s for %s", szset, szname);

				// process initial condition
				pic->SetItemList(pg);
				pic->SetName(szname);
				m_pBCStep->AddComponent(pic);
				ReadParameters(*pic, tag);
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
//                                C O N T A C T
//
//=============================================================================

//-----------------------------------------------------------------------------
//! Read the contact section (defined for version 2.0 and up).
bool FEBioFormat4::ParseContactSection(XMLTag& tag)
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
void FEBioFormat4::ParseContact(FEStep *pstep, XMLTag &tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel* fem = &febio.GetFEModel();

	// get the contact interface type
	XMLAtt& atype = tag.Attribute("type");

	// get the surface pair
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

//-----------------------------------------------------------------------------
void FEBioFormat4::ParseRigidConstraint(FEStep* pstep, XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	// get the name 
	stringstream ss;
	ss << "RigidConstraint" << CountConnectors<FERigidConstraint>(fem) + 1;
	std::string name = tag.AttributeValue("name", ss.str());

	// get the type attribute
	const char* sztype = tag.AttributeValue("type");

	// allocate class
	FERigidConstraint* pi = FEBio::CreateRigidConstraint(sztype, &fem);
	if (pi == nullptr)
	{
		ParseUnknownAttribute(tag, "type");
		return;
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
void FEBioFormat4::ParseRigidConnector(FEStep *pstep, XMLTag &tag)
{
	FEModel& fem = GetFEModel();

	// get the name 
	const char* szname = tag.AttributeValue("name", true);
	char name[256];
	if (szname == nullptr)
	{
		sprintf(name, "RigidConnector%02d", CountConnectors<FERigidConnector>(fem) + 1);
		szname = name;
	}

	// get the type attribute
	const char* sztype = tag.AttributeValue("type");

	// allocate class
	FERigidConnector* pi = FEBio::CreateRigidConnector(sztype, &fem);
	if (pi == nullptr)
	{
		ParseUnknownAttribute(tag, "type");
		return;
	}
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
void FEBioFormat4::ParseLinearConstraint(FEStep* pstep, XMLTag& tag)
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

bool FEBioFormat4::ParseDiscreteSection(XMLTag& tag)
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

			FEDiscreteMaterial* pdm = new FEBioDiscreteMaterial;
			if (FEBio::CreateModelComponent(FE_MATERIAL, sztype, pdm) == false)
			{
				delete pdm;
				throw XMLReader::InvalidTag(tag);
			}

			GDiscreteSpringSet* pg = new GDiscreteSpringSet(&gm);
			pg->SetMaterial(pdm);
			pg->SetName(szname);
			fem.GetModel().AddDiscreteObject(pg);

			ReadParameters(*pdm, tag);
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
bool FEBioFormat4::ParseConstraintSection(XMLTag& tag)
{
	if (tag.isleaf()) return true;

	FEStep* pstep = m_pBCStep;

	++tag;
	do
	{
		if (tag == "constraint") ParseNLConstraint(pstep, tag);
		else ParseUnknownTag(tag);
		++tag;
	}
	while (!tag.isend());

	return true;
}

//-----------------------------------------------------------------------------
void FEBioFormat4::ParseNLConstraint(FEStep* pstep, XMLTag& tag)
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
		sprintf(szbuf, "NLConstraint%02d", CountConstraints<FEModelConstraint>(fem)+1);
		szname = szbuf;
	}

	// find the surface
	FESurface* psurf = nullptr;
	const char* szsurf = tag.AttributeValue("surface", true);
	if (szsurf)
	{
		febio.BuildFESurface(szsurf);
		if (psurf == 0) AddLogEntry("Failed creating surface %s", szsurf);
	}

	// get the type
	const char* sztype = tag.AttributeValue("type");

	// create a new constraint
	FEModelConstraint* pi = FEBio::CreateNLConstraint(sztype, &fem);
	if (pi == nullptr)
	{
		ParseUnknownTag(tag);
		return;
	}

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
bool FEBioFormat4::ParseLoadDataSection(XMLTag& tag)
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

bool FEBioFormat4::ParseLoadCurve(XMLTag& tag, FELoadCurve& lc)
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
bool FEBioFormat4::ParseStepSection(XMLTag &tag)
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

bool FEBioFormat4::ParseStep(XMLTag& tag)
{
	char szname[128] = { 0 };

	const char* szval = tag.AttributeValue("name", true);
	if (szval) strcpy(szname, szval);

	++tag;

	// make sure the analysis flag was defined
	if (m_nAnalysis < 0) return false;

	// create a new step (unless this is the first step)
	if (m_pstep == 0)
	{
		FEModel& fem = GetFEModel();
		m_pstep = new FEBioAnalysisStep(&fem);
		FEBio::CreateStep("analysis", m_pstep);
		const char* szname = tag.AttributeValue("name", true);
		if ((szname == 0) || (strlen(szname) == 0))
		{
			char sz[256] = { 0 };
			sprintf(sz, "Step%02d", fem.Steps());
			m_pstep->SetName(sz);
		}
		else m_pstep->SetName(szname);
		fem.AddStep(m_pstep);
	}
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
FEBioModel::DiscreteSet FEBioFormat4::ParseDiscreteSet(XMLTag& tag)
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
