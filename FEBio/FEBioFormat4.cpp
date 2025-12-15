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
#include <GeomLib/GMeshObject.h>
#include <FEMLib/FEInitialCondition.h>
#include <FEMLib/FEBodyLoad.h>
#include <FEMLib/FEElementFormulation.h>
#include <FEMLib/FEModelConstraint.h>
#include <FEMLib/FERigidLoad.h>
#include <FEMLib/GDiscreteObject.h>
#include <MeshLib/FSElementData.h>
#include <MeshLib/FSSurfaceData.h>
#include <MeshLib/FSNodeData.h>
#include <GeomLib/GModel.h>
#include <GeomLib/GGroup.h>
#include <GeomLib/FSGroup.h>
#include <FEBioLink/FEBioInterface.h>
#include <FEBioLink/FEBioModule.h>
#include <FSCore/Palette.h>
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
    else if (bc == "gx") return true;
    else if (bc == "gy") return true;
    else if (bc == "gz") return true;
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
FEBioFormat4::FEBioFormat4(FEBioFileImport* fileReader, FEBioInputModel& febio) : FEBioFormat(fileReader, febio)
{
	m_geomFormat = 0;
}

FEBioFormat4::~FEBioFormat4()
{
}

FEBioInputModel::Part* FEBioFormat4::DefaultPart()
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
		// make sure the module section was read in
		if ((m_nAnalysis == -1) && (tag != "Module"))
		{
			throw std::runtime_error("Required Module section is missing.");
		}

		if      (tag == "Module"     ) ParseModuleSection    (tag);
		else if (tag == "Control"    ) ParseControlSection   (tag);
		else if (tag == "Material"   ) ParseMaterialSection  (tag);
		else if (tag == "Mesh"       ) ParseMeshSection      (tag);
		else if (tag == "MeshDomains") ParseMeshDomainsSection(tag);
		else if (tag == "MeshData"   ) ParseMeshDataSection  (tag);
		else if (tag == "MeshAdaptor") ParseMeshAdaptorSection(tag);
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
	XMLAtt& atype = tag.Attribute("type");
	int moduleId = FEBio::GetModuleId(atype.cvalue()); assert(moduleId >= 0);
	if (moduleId < -1) throw XMLReader::InvalidAttributeValue(tag, "type", atype.m_val.c_str());
	
	FSProject& prj = FileReader()->GetProject();
	prj.SetModule(moduleId, false);
	m_nAnalysis = moduleId;

	if (tag.isempty() == false)
	{
		++tag;
		do {
			if (tag == "units")
			{
				// NOTE: the values are defined in FEBioStudio\units.h. 
				const char* sz = tag.szvalue();
				if      (strcmp(sz, "SI"     ) == 0) prj.SetUnits(2);
				else if (strcmp(sz, "mm-N-s" ) == 0) prj.SetUnits(3);
				else if (strcmp(sz, "mm-kg-s") == 0) prj.SetUnits(4);
				else if (strcmp(sz, "um-nN-s") == 0) prj.SetUnits(5);
				else if (strcmp(sz, "CGS"    ) == 0) prj.SetUnits(6);
                else if (strcmp(sz, "mm-g-s" ) == 0) prj.SetUnits(7);
                else if (strcmp(sz, "mm-mg-s") == 0) prj.SetUnits(8);
				else AddLogEntry("Unrecognized unit system.");
			}
			++tag;
		} 
		while (!tag.isend());
	}

	return (moduleId != -1);
}
//=============================================================================
//
//                                C O N T R O L
//
//=============================================================================

//-----------------------------------------------------------------------------
//! Create a new step
FSStep* FEBioFormat4::NewStep(FSModel& fem, const std::string& typeStr, const char* szname)
{
	assert(typeStr.empty() == false);
	FSStep* pstep = FEBio::CreateStep(typeStr, &fem); assert(pstep);

	if ((szname == 0) || (strlen(szname) == 0))
	{
		char sz[256] = { 0 };
		sprintf(sz, "Step%02d", fem.Steps());
		pstep->SetName(sz);
	}
	else pstep->SetName(szname);
	fem.AddStep(pstep);

	return pstep;
}

//-----------------------------------------------------------------------------
//  This function parses the control section from the xml file
//
bool FEBioFormat4::ParseControlSection(XMLTag& tag)
{
	// make sure the section is not empty
	if (tag.isleaf()) return true;

	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// create a new analysis step from these control settings
	const char* szmod = FEBio::GetActiveModuleName();
	if (m_pstep == 0) m_pstep = NewStep(fem, szmod);
	FSStep* pstep = m_pstep; assert(pstep);

	// parse the settings
	++tag;
	do
	{
		if (ReadParam(*pstep, tag) == false)
		{
			if (pstep->Properties() > 0)
			{
				const char* sztag = tag.Name();
				FSProperty* pc = pstep->FindProperty(sztag); assert(pc);

				// see if this is a property
				std::string typeStr;
				const char* sztype = tag.AttributeValue("type", true);
				if (sztype == nullptr)
				{
					// The default solver should be the solver with the same name as the module
					if (tag.Name() == "solver") typeStr = FEBio::GetActiveModuleName();
					else
					{
						typeStr = pc->GetDefaultType();
						if (typeStr.empty()) typeStr = tag.Name();
					}
				}
				else typeStr = sztype;

				if (pc->GetComponent() == nullptr)
				{
					FSModelComponent* psc = FEBio::CreateClass(pc->GetSuperClassID(), typeStr, &fem);
					pc->SetComponent(psc);
				}

				// read the parameters
				ParseModelComponent(dynamic_cast<FSModelComponent*>(pc->GetComponent()), tag);
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

	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

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
		FSMaterial* pmat = FEBio::CreateMaterial(sztype, &fem);
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
		ParseModelComponent(pmat, tag);

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
		else if (tag == "Edge"       ) ParseGeometryEdgeSet    (DefaultPart(), tag);
		else if (tag == "Surface"    ) ParseGeometrySurface    (DefaultPart(), tag);
		else if (tag == "ElementSet" ) ParseGeometryElementSet (DefaultPart(), tag);
		else if (tag == "PartList"   ) ParseGeometryPartList   (DefaultPart(), tag);
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
bool FEBioFormat4::ParseMeshDomainsSection(XMLTag& tag)
{
	// make sure the section is not empty
	if (!tag.isleaf())
	{
		// loop over all sections
		++tag;
		do
		{
			if      (tag == "SolidDomain") ParseSolidDomain(tag);
			else if (tag == "ShellDomain") ParseShellDomain(tag);
			else if (tag == "BeamDomain" ) ParseBeamDomain (tag);
			else ParseUnknownTag(tag);
			++tag;
		} while (!tag.isend());
	}

	// don't forget to update the mesh
	GetFEBioModel().UpdateGeometry();

	// copy all mesh selections to named selections
	GetFEBioModel().CopyMeshSelections();
    
    return true;
}

//-----------------------------------------------------------------------------
void FEBioFormat4::ParseSolidDomain(XMLTag& tag)
{
	FEBioInputModel::Part* part = DefaultPart();

	const char* szname = tag.AttributeValue("name");
	const char* szmat = tag.AttributeValue("mat", true);
	if (szmat)
	{
		FEBioInputModel& febio = GetFEBioModel();
		int matID = febio.GetMaterialIndex(szmat);
		if (matID == -1) matID = atoi(szmat) - 1;

		FEBioInputModel::Domain* dom = part->FindDomain(szname);
		if (dom) dom->SetMatID(matID);
		dom->SetType(FEBioInputModel::Domain::SOLID);

		FESolidFormulation* eform = nullptr;
		const char* szelem = tag.AttributeValue("type", true);
		if (szelem) eform = FEBio::CreateSolidFormulation(szelem, &febio.GetFSModel());
		dom->SetElementFormulation(eform);

		// read the domain parameters
		if (eform)
			ParseModelComponent(eform, tag);
		else if (!tag.isleaf())
			ParseUnknownAttribute(tag, "type");
	}
}

//-----------------------------------------------------------------------------
void FEBioFormat4::ParseShellDomain(XMLTag& tag)
{
	FEBioInputModel::Part* part = DefaultPart();

	const char* szname = tag.AttributeValue("name");
	const char* szmat = tag.AttributeValue("mat", true);
	if (szmat)
	{
		FEBioInputModel& febio = GetFEBioModel();
		int matID = febio.GetMaterialIndex(szmat);
		if (matID == -1) matID = atoi(szmat) - 1;

		FEBioInputModel::Domain* dom = part->FindDomain(szname);
		if (dom) dom->SetMatID(matID);

		FEShellFormulation* shell = nullptr;
		const char* szelem = tag.AttributeValue("type", true);
		if (szelem) shell = FEBio::CreateShellFormulation(szelem, &febio.GetFSModel());

		dom->SetElementFormulation(shell);
		dom->SetType(FEBioInputModel::Domain::SHELL);

		// read the domain parameters
		if (tag.isleaf() == false)
		{
			if (shell)
				ReadParameters(*shell, tag);
			else
			{
				++tag;
				do {
					if (tag == "shell_thickness")
					{
						double h = 0.0;
						tag.value(h);
						dom->SetDefaultShellThickness(h);
					}
					else ParseUnknownTag(tag);
					++tag;
				} 
				while (!tag.isend());
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioFormat4::ParseBeamDomain(XMLTag& tag)
{
	FEBioInputModel::Part* part = DefaultPart();

	const char* szname = tag.AttributeValue("name");
	const char* szmat = tag.AttributeValue("mat", true);
	if (szmat)
	{
		FEBioInputModel& febio = GetFEBioModel();
		int matID = febio.GetMaterialIndex(szmat);
		if (matID == -1) matID = atoi(szmat) - 1;

		FEBioInputModel::Domain* dom = part->FindDomain(szname);
		if (dom) dom->SetMatID(matID);

		FEBeamFormulation* beam = nullptr;
		const char* szelem = tag.AttributeValue("type", true);
		if (szelem) beam = FEBio::CreateBeamFormulation(szelem, &febio.GetFSModel());

		dom->SetElementFormulation(beam);
		dom->SetType(FEBioInputModel::Domain::BEAM);

		// read the domain parameters
		if (tag.isleaf() == false)
		{
			if (beam)
				ReadParameters(*beam, tag);
			else
			{
				ParseUnknownTag(tag);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioFormat4::ParseGeometryNodes(FEBioInputModel::Part* part, XMLTag& tag)
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
		node.m_nid = nd.id;
		node.r = nd.r;
	}
}

// helper function for converting the element's type attribute to FSElementType
FSElementType ConvertStringToElementType(const char* sztype)
{
	FSElementType ntype = FE_INVALID_ELEMENT_TYPE;
	if      (strcmp(sztype, "hex8"   ) == 0) ntype = FE_HEX8;
	else if (strcmp(sztype, "hex20"  ) == 0) ntype = FE_HEX20;
	else if (strcmp(sztype, "hex27"  ) == 0) ntype = FE_HEX27;
	else if (strcmp(sztype, "penta6" ) == 0) ntype = FE_PENTA6;
	else if (strcmp(sztype, "tet4"   ) == 0) ntype = FE_TET4;
	else if (strcmp(sztype, "tet5"   ) == 0) ntype = FE_TET5;
	else if (strcmp(sztype, "tet10"  ) == 0) ntype = FE_TET10;
	else if (strcmp(sztype, "tet15"  ) == 0) ntype = FE_TET15;
	else if (strcmp(sztype, "tet20"  ) == 0) ntype = FE_TET20;
	else if (strcmp(sztype, "quad4"  ) == 0) ntype = FE_QUAD4;
	else if (strcmp(sztype, "quad8"  ) == 0) ntype = FE_QUAD8;
	else if (strcmp(sztype, "quad9"  ) == 0) ntype = FE_QUAD9;
	else if (strcmp(sztype, "tri3"   ) == 0) ntype = FE_TRI3;
	else if (strcmp(sztype, "tri6"   ) == 0) ntype = FE_TRI6;
	else if (strcmp(sztype, "pyra5"  ) == 0) ntype = FE_PYRA5;
	else if (strcmp(sztype, "penta15") == 0) ntype = FE_PENTA15;
	else if (strcmp(sztype, "pyra13" ) == 0) ntype = FE_PYRA13;
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
	else if (strcmp(sztype, "line2"       ) == 0) ntype = FE_BEAM2;
	else if (strcmp(sztype, "line3"       ) == 0) ntype = FE_BEAM3;
	return ntype;
}

void FEBioFormat4::ParseGeometryElements(FEBioInputModel::Part* part, XMLTag& tag)
{
	if (part == 0) throw XMLReader::InvalidTag(tag);

	// first we need to figure out how many elements there are
	int elems = tag.children();

	// get the required type attribute
	const char* sztype = tag.AttributeValue("type");
	FSElementType elemType = ConvertStringToElementType(sztype);
	if (elemType == FE_INVALID_ELEMENT_TYPE) throw XMLReader::InvalidAttributeValue(tag, "type", sztype);

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
	if (szname == 0) throw XMLReader::MissingAttribute(tag, "name");

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
		el.SetType(elemType);
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
}

void FEBioFormat4::ParseGeometryNodeSet(FEBioInputModel::Part* part, XMLTag& tag)
{
	// make sure there is a name attribute
	std::string name = tag.AttributeValue("name");

	// list to store node numbers
	vector<int> list;
	tag.value(list);

	// create a new node set
	part->AddNodeSet(FEBioInputModel::NodeSet(name, list));
}

//-----------------------------------------------------------------------------
void FEBioFormat4::ParseGeometryDiscreteSet(FEBioInputModel::Part* part, XMLTag& tag)
{
	if (part == 0) throw XMLReader::InvalidTag(tag);

	FEBioInputModel::DiscreteSet ds;
	const char* szname = tag.AttributeValue("name");
	ds.SetName(szname);
	ds.SetPart(part);

	if (!tag.isleaf())
	{
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
	}

	part->AddDiscreteSet(ds);
}

//-----------------------------------------------------------------------------
void FEBioFormat4::ParseGeometrySurfacePair(FEBioInputModel::Part* part, XMLTag& tag)
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
void FEBioFormat4::ParseGeometryEdgeSet(FEBioInputModel::Part* part, XMLTag& tag)
{
	if (part == 0) throw XMLReader::InvalidTag(tag);

	// get the name
	const char* szname = tag.AttributeValue("name");

	// see if a edgeset with this name is already defined
	// if found, we'll continue, but we'll generate a warning.
	FEBioInputModel::EdgeSet* ps = part->FindEdgeSet(szname);
	if (ps) FileReader()->AddLogEntry("An edge named %s is already defined.", szname);

	// create a new edge
	FEBioInputModel::EdgeSet s;
	s.m_name = szname;

	if (tag.isleaf() == false)
	{
		// read the surface data
		int nf[FSElement::MAX_NODES], N;
		++tag;
		do
		{
			// read the line element
			if (tag == "line2") N = 2;
			else if (tag == "line3") N = 3;
			else throw XMLReader::InvalidTag(tag);

			// read the node numbers
			tag.value(nf, N);

			vector<int> node(N);
			for (int j = 0; j < N; ++j) node[j] = nf[j];
			s.m_edge.push_back(node);

			++tag;
		} while (!tag.isend());
	}

	part->AddEdgeSet(s);
}


//-----------------------------------------------------------------------------
void FEBioFormat4::ParseGeometrySurface(FEBioInputModel::Part* part, XMLTag& tag)
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
			else throw XMLReader::InvalidTag(tag);

			// read the node numbers
			tag.value(nf, N);

			// copy to vector
			vector<int> node(N);
			for (int j = 0; j < N; ++j) node[j] = nf[j];
			s.m_face.push_back(node);

			++tag;
		} while (!tag.isend());
	}

	part->AddSurface(s);
}

//-----------------------------------------------------------------------------
void FEBioFormat4::ParseGeometryElementSet(FEBioInputModel::Part* part, XMLTag& tag)
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
	if (tag.isleaf()) tag.value(elem);
	else
	{
		// This is not supported in feb4 format.
		assert(false);
		++tag;
		do
		{
			int eid = tag.AttributeValue<int>("id", -1);
			if (eid == -1) throw XMLReader::InvalidTag(tag);
			elem.push_back(eid);

			++tag;
		} while (!tag.isend());
	}

	part->AddElementSet(FEBioInputModel::ElementSet(sname, elem, part));
}

//-----------------------------------------------------------------------------
void FEBioFormat4::ParseGeometryPartList(FEBioInputModel::Part* part, XMLTag& tag)
{
	if (part == 0) throw XMLReader::InvalidTag(tag);

	// get the name
	const char* szname = tag.AttributeValue("name");
	if (szname == 0) FileReader()->AddLogEntry("Part list defined without a name.");
	string sname = (szname ? szname : "");

	// see if a part list with this name is already defined
	// if found, we'll continue, but we'll generate a warning.
	FEBioInputModel::DomainList* ps = part->FindDomainList(szname);
	if (ps) FileReader()->AddLogEntry("A part list set named %s is already defined.", szname);

	// read the part names
	vector<string> partNames;
	tag.value(partNames);

	// add the parts to the part list
	std::vector<FEBioInputModel::Domain*> domList;
	for (string s : partNames)
	{
		FEBioInputModel::Domain* pg = part->FindDomain(s);
		if (pg) domList.push_back(pg);
	}

	part->AddDomainList(FEBioInputModel::DomainList(sname, domList));
}

//-----------------------------------------------------------------------------
void FEBioFormat4::ParseGeometryPart(XMLTag& tag)
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
void FEBioFormat4::ParseGeometryInstance(XMLTag& tag)
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

			if (e0.IsShell())
			{
				assert(e1.IsShell());
				int ne = e0.Nodes(); assert(ne == e1.Nodes());
				for (int k = 0; k < ne; ++k)
				{
					e0.m_h[k] = e1.m_h[k];
				}
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
	FEBioInputModel& feb = GetFEBioModel();

	XMLAtt* name = tag.AttributePtr("name");
	XMLAtt* dataTypeAtt = tag.AttributePtr("data_type");
	XMLAtt* nset = tag.AttributePtr("node_set");

	DATA_TYPE dataType;
	if (dataTypeAtt)
	{
		if      (*dataTypeAtt == "scalar") dataType = DATA_TYPE::DATA_SCALAR;
		else if (*dataTypeAtt == "vec3"  ) dataType = DATA_TYPE::DATA_VEC3;
		else return false;
	}
	else dataType = DATA_TYPE::DATA_SCALAR;

	const char* szgen = tag.AttributeValue("type", true);

	if (szgen)
	{
		FSModel* fem = &feb.GetFSModel();
		// allocate mesh data generator
		FSMeshDataGenerator* gen = FEBio::CreateNodeDataGenerator(szgen, fem);
		if (gen)
		{
			FSItemListBuilder* nodeSet = feb.FindNamedSelection(nset->cvalue(), MESH_ITEM_FLAGS::FE_NODE_FLAG);

			gen->SetName(name->cvalue());
			gen->SetItemList(nodeSet);

			ParseModelComponent(gen, tag);

			fem->AddMeshDataGenerator(gen);
		}
		else ParseUnknownAttribute(tag, "type");
	}
	else
	{
		FSNodeSet* nodeSet = feb.FindNamedNodeSet(nset->cvalue());
		if (nodeSet)
		{	
			FSMesh* feMesh = nodeSet->GetMesh();

			FSNodeData* nodeData = feMesh->AddNodeDataField(name->cvalue(), nodeSet, dataType);
			const int items = nodeData->Size();

			++tag;
			do
			{
				int lid = -1;
				tag.AttributePtr("lid")->value(lid);
				int index = lid - 1;
				if ((index < 0) || (index >= items)) throw XMLReader::InvalidAttributeValue(tag, "lid");

				switch (dataType)
				{
				case DATA_SCALAR:
				{
					double val = 0.0;
					tag.value(val);
					nodeData->setScalar(index, val);
				}
				break;
				case DATA_VEC3:
				{
					vec3d val;
					tag.value(val);
					nodeData->setVec3d(index, val);
				}
				break;
				default:
					assert(false);
				}

				++tag;
			} while (!tag.isend());
		}
		else tag.skip();
	}

	return true;

}

bool FEBioFormat4::ParseSurfaceDataSection(XMLTag& tag)
{
	FEBioInputModel& feb = GetFEBioModel();

	XMLAtt* name = tag.AttributePtr("name");
	XMLAtt* dataTypeAtt = tag.AttributePtr("data_type");
	XMLAtt* surf = tag.AttributePtr("surface");
	XMLAtt* type = tag.AttributePtr("type");

	if (type)
	{
		FEBioInputModel& feb = GetFEBioModel();
		FSModel* fem = &feb.GetFSModel();
		// allocate mesh data generator
		FSMeshDataGenerator* gen = nullptr;
		const char* sztype = type->cvalue();
		if (strcmp(sztype, "const") == 0)
		{
			// "const" data generator needs to be handled differently
			DATA_TYPE dataType = DATA_TYPE::DATA_SCALAR;
			if (dataTypeAtt)
			{
				if      (*dataTypeAtt == "scalar") dataType = DATA_TYPE::DATA_SCALAR;
				else if (*dataTypeAtt == "vec3"  ) dataType = DATA_TYPE::DATA_VEC3;
				else if (*dataTypeAtt == "mat3"  ) dataType = DATA_TYPE::DATA_MAT3;
				else return false;
			}
			FSConstFaceDataGenerator* constGen = new FSConstFaceDataGenerator(fem, dataType);
			gen = constGen;
		}
		else
		{
			// allocate febio data generator
			gen = FEBio::CreateFaceDataGenerator(sztype, fem);
		}

		if (gen)
		{
			XMLAtt* name = tag.AttributePtr("name");
			gen->SetName(name->cvalue());

			const char* szset = surf->cvalue();
			GMeshObject* po = feb.GetInstance(0)->GetGObject();
			FSSurface* ps = feb.FindNamedSurface(surf->cvalue());

			gen->SetItemList(ps);

			ParseModelComponent(gen, tag);
			fem->AddMeshDataGenerator(gen);
		}
		else ParseUnknownAttribute(tag, "type");
	}
	else
	{
		DATA_TYPE dataType;
		if (dataTypeAtt)
		{
			if      (*dataTypeAtt == "scalar") dataType = DATA_TYPE::DATA_SCALAR;
			else if (*dataTypeAtt == "vec3"  ) dataType = DATA_TYPE::DATA_VEC3;
			else return false;
		}
		else dataType = DATA_TYPE::DATA_SCALAR;

		FSSurface* feSurf = feb.FindNamedSurface(surf->cvalue());
		FSMesh* feMesh = feSurf->GetMesh();

		FSSurfaceData* sd = feMesh->AddSurfaceDataField(name->cvalue(), feSurf, dataType);

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
	}

	return true;
}

bool FEBioFormat4::ParseElementDataSection(XMLTag& tag)
{
	XMLAtt* type = tag.AttributePtr("type");
	if (type && (*type == "shell thickness"))
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

					if (el.IsShell() && (m == el.Nodes()))
					{
						for (int i = 0; i < m; ++i) el.m_h[i] = h[i];
					}
					else throw XMLReader::InvalidValue(tag);
				}
				++tag;
			} while (!tag.isend());
		}
		else
		{
			FEBioInputModel::ElementSet* eset = feb.FindElementSet(szset);
			if (eset && eset->m_part)
			{
				FSMesh* mesh = eset->m_part->GetFEMesh();

				double h[FSElement::MAX_NODES] = { 0 };
				++tag;
				do
				{
					int m = tag.value(h, FSElement::MAX_NODES);
					int lid = tag.AttributeValue<int>("lid", 0) - 1;
					if (lid >= 0)
					{
						int id = mesh->ElementIndexFromID(eset->element(lid));
						FSElement& el = mesh->Element(id);

						if (el.IsShell() && (m == el.Nodes()))
						{
							for (int i = 0; i < m; ++i) el.m_h[i] = h[i];
						}
						else throw XMLReader::InvalidValue(tag);
					}
					++tag;
				} while (!tag.isend());
			}
			else ParseUnknownAttribute(tag, "elem_set");
		}
	}
	else if (type && (*type == "fiber"))
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
	else if (type && (*type == "mat_axis"))
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
	else if (type)
	{
		FEBioInputModel& feb = GetFEBioModel();
		FSModel* fem = &feb.GetFSModel();
		// allocate mesh data generator
		const char* sztype = type->cvalue();
		FSMeshDataGenerator* gen = FEBio::CreateElemDataGenerator(sztype, fem);
		if (gen)
		{
			XMLAtt* name = tag.AttributePtr("name");
			XMLAtt* elset = tag.AttributePtr("elem_set");

			const char* szset = elset->cvalue();
			if (strncmp(szset, "@part_list:", 11) == 0)
			{
				GPartList* pg = feb.FindNamedPartList(szset+11);
				if (pg == nullptr) AddLogEntry("Cannot find part list %s", elset->cvalue());
				else gen->SetItemList(pg);
			}
			else
			{
				GMeshObject* po = feb.GetInstance(0)->GetGObject();
				FSMesh* pm = po->GetFEMesh();
				FSPartSet* ps = new FSPartSet(pm);
				ps->SetName(name->cvalue());
				for (int i = 0; i < po->Parts(); ++i)
				{
					GPart* pg = po->Part(i);
					if (pg->GetName() == string(szset))
					{
						ps->add(i);
					}
				}

				if (ps->size() > 0)
				{
					pm->AddFEPartSet(ps);
					gen->SetItemList(ps);
				}
				else
				{
					delete ps;
					AddLogEntry("Cannot find part %s", elset->cvalue());
				}
			}

			gen->SetName(name->cvalue());

			ParseModelComponent(gen, tag);
			fem->AddMeshDataGenerator(gen);
		}
		else ParseUnknownAttribute(tag, "type");
	}
	else if (type == nullptr)
	{
		FEBioInputModel& feb = GetFEBioModel();

		XMLAtt* name = tag.AttributePtr("name");
		XMLAtt* dataTypeAtt = tag.AttributePtr("data_type");
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

		FSMeshData* meshData = nullptr;
		int offset = 0;

		// see if we already have this data map
		FSPartData* partData = mesh->FindPartDataField(sname);
		if (partData)
		{
			GPart* pg = po->FindPartFromName(set->cvalue()); assert(pg);
			offset = partData->DataItems();
			partData->AddPart(pg->GetLocalID());
			meshData = partData;
		}
		else
		{
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
				FSPartSet* partSet = new FSPartSet(mesh);
				partSet->SetName(sname);
				mesh->AddFEPartSet(partSet);
				partSet->add(pg->GetLocalID());

				meshData = mesh->AddPartDataField(sname, partSet, dataType);
			}
			else
			{
				meshData = mesh->AddElementDataField(sname, pg, dataType);
			}
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

				meshData->set(offset + lid - 1, val);

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
				meshData->set(offset + lid - 1, val);
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
				meshData->set(offset + lid - 1, val);
				++tag;
			} while (!tag.isend());
		}
	}
	else ParseUnknownTag(tag);

	return true;
}

//=============================================================================
//
//                                M E S H A D A P T O R 
//
//=============================================================================

//-----------------------------------------------------------------------------
bool FEBioFormat4::ParseMeshAdaptorSection(XMLTag& tag)
{
	if (tag.isleaf()) return true;

	FEBioInputModel& feb = GetFEBioModel();
	FSModel* fem = &GetFSModel();

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
					GPartList* partList = new GPartList(&fem->GetModel());
					partList->add(pg->GetID());
					mda->SetItemList(partList);
				}
				else
				{
					if (strstr(szset, "@part_list:"))
					{
						GPartList* pg = feb.FindNamedPartList(szset + 11);
						if (pg)
						{
							mda->SetItemList(pg);
						}
						else AddLogEntry("Failed to find element set %s", szset);
					}
					else AddLogEntry("Failed to find element set %s", szset);
				}
			}

			m_pBCStep->AddComponent(mda);

			ParseModelComponent(mda, tag);
		}
		else ParseUnknownTag(tag);
		++tag;
	}
	while (!tag.isleaf());

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
void FEBioFormat4::ParseBC(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// get the node set/surface 
	XMLAtt* aset = tag.Attribute("node_set", true);
	XMLAtt* asrf = tag.Attribute("surface", true);

	// create the node set
	FSItemListBuilder* pg = nullptr;
	if (aset)
	{
		pg = febio.FindNamedSelection(aset->cvalue());
		if (pg == 0) FileReader()->AddLogEntry("Unknown node set \"%s\". (line %d)", aset->cvalue(), tag.m_nstart_line);
	}
	else if (asrf)
	{
		pg = febio.FindNamedSurface(asrf->cvalue());
		if (pg == 0) FileReader()->AddLogEntry("Unknown surface \"%s\". (line %d)", aset->cvalue(), tag.m_nstart_line);
	}

	// get the type attribute
	const char* sztype = tag.AttributeValue("type");

	// get the (optional) name
	string name;
	const char* szname = tag.AttributeValue("name", true);
	if (szname) name = szname; else name = sztype;

	// create the boundary condition
	FSBoundaryCondition* pbc = FEBio::CreateBoundaryCondition(sztype, &fem);
	if (pbc == nullptr)
	{
		ParseUnknownAttribute(tag, "type");
		return;
	}

	pbc->SetItemList(pg);
	pbc->SetName(name);
	pstep->AddComponent(pbc);

	ParseModelComponent(pbc, tag);
}

//=============================================================================
//
//                                R I G I D
//
//=============================================================================

//-----------------------------------------------------------------------------
bool FEBioFormat4::ParseRigidSection(XMLTag& tag)
{
	// make sure the section is not empty
	if (tag.isleaf()) return true;

	++tag;
	do
	{
		if      (tag == "rigid_bc"        ) ParseRigidBC       (m_pBCStep, tag);
		else if (tag == "rigid_ic"        ) ParseRigidIC       (m_pBCStep, tag);
		else if (tag == "rigid_load"      ) ParseRigidLoad     (m_pBCStep, tag);
		else if (tag == "rigid_connector" ) ParseRigidConnector(m_pBCStep, tag);
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
void FEBioFormat4::ParseNodeLoad(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// get the load curve ID
	XMLAtt& aset = tag.Attribute("node_set");

	// create the node set
	FSItemListBuilder* pg = febio.FindNamedSelection(aset.cvalue());
	if (pg == 0) throw XMLReader::InvalidAttributeValue(tag, aset);

	// get the (optional) name
	string name;
	const char* szname = tag.AttributeValue("name", true);
	if (szname == nullptr)
	{
		char szname[256];
		sprintf(szname, "NodalLoad%02d", CountLoads<FSNodalLoad>(fem) + 1);
		name = szname;
	}
	else name = szname;

	const char* sztype = tag.AttributeValue("type");

	// create the nodal load
	FSNodalLoad* pnl = FEBio::CreateNodalLoad(sztype, &fem);
	if (pnl == nullptr)
	{
		ParseUnknownAttribute(tag, "type");
		return;
	}
	pnl->SetName(name);
	pnl->SetItemList(pg);
	pstep->AddComponent(pnl);

	ParseModelComponent(pnl, tag);
}

//-----------------------------------------------------------------------------
//! Parses the surface_load section.
void FEBioFormat4::ParseSurfaceLoad(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	std::string comment = tag.comment();

	// find the surface
	XMLAtt& surf = tag.Attribute("surface");
	FSSurface* psurf = febio.FindNamedSurface(surf.cvalue());
	if (psurf == 0) throw XMLReader::InvalidAttributeValue(tag, surf);

	// get the type attribute
	XMLAtt& att = tag.Attribute("type");

	// read the (optional) name
	stringstream defaultName; defaultName << "SurfaceLoad" << CountLoads<FSSurfaceLoad>(fem) + 1;
	string name = tag.AttributeValue("name", defaultName.str());

	// create the surface load
	FSSurfaceLoad* psl = FEBio::CreateSurfaceLoad(att.cvalue(), &fem);
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
	ParseModelComponent(psl, tag);
}

//-----------------------------------------------------------------------------
//! Parses the body_load section.
void FEBioFormat4::ParseBodyLoad(FSStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();

	// read the comment
	std::string comment = tag.comment();

	// read the (optional) name
	stringstream defaultName; defaultName << "BodyLoad" << CountLoads<FSBodyLoad>(fem) + 1;
	string name = tag.AttributeValue("name", defaultName.str());

	// read the (optional) element set
	GPartList* partList = nullptr;
	const char* szelemSetName = tag.AttributeValue("elem_set", true);
	if (szelemSetName)
	{
		GModel& gm = fem.GetModel();
		FEBioInputModel& febio = GetFEBioModel();
		FEBioInputModel::PartInstance& part = *febio.GetInstance(0);
		GObject* po = part.GetGObject();

		partList = febio.FindNamedPartList(szelemSetName);
		if (partList == nullptr)
		{
			GPart* pg = po->FindPartFromName(szelemSetName);
			if (pg)
			{
				partList = new GPartList(&gm);
				partList->SetName(szelemSetName);
				partList->add(pg->GetID());
				gm.AddPartList(partList);
			}
			else
			{
				AddLogEntry("Cannot find part %s for %s", szelemSetName, name.c_str());
			}
		}
	}

	// create new body load
	XMLAtt& att = tag.Attribute("type");
	FSBodyLoad* pbl = FEBio::CreateBodyLoad(att.cvalue(), &fem);
	if (pbl == nullptr)
	{
		ParseUnknownAttribute(tag, "type");
		return;
	}

	// process body load
	if (partList) pbl->SetItemList(partList);
	pbl->SetInfo(comment);
	if (name.empty() == false) pbl->SetName(name);
	pstep->AddComponent(pbl);
	ParseModelComponent(pbl, tag);
}

//-----------------------------------------------------------------------------
bool FEBioFormat4::ParseInitialSection(XMLTag& tag)
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
			// get the type attribute
			const char* sztype = tag.AttributeValue("type");

			// get the (optional) name
			char szbuf[64] = { 0 };
			const char* szname = tag.AttributeValue("name", true);
			if (szname == nullptr)
			{
				sprintf(szbuf, "IC%d", CountICs<FSInitialCondition>(fem) + 1);
				szname = szbuf;
			}

			// allocate initial condition
			FSInitialCondition* pic = FEBio::CreateInitialCondition(sztype, &fem); assert(pic);
			if (pic == nullptr)
			{
				ParseUnknownTag(tag);
			}
			else
			{
				// get the node set
				const char* szset = tag.AttributeValue("node_set");
				FSItemListBuilder* pg = febio.FindNamedSelection(szset);
				if (pg == 0) AddLogEntry("Failed to create nodeset %s for %s", szset, szname);
				else
				{
					if (pg->GetName().empty()) pg->SetName(szname);
					pic->SetItemList(pg);
				}
				pic->SetName(szname);
				m_pBCStep->AddComponent(pic);
				ParseModelComponent(pic, tag);
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
void FEBioFormat4::ParseContact(FSStep *pstep, XMLTag &tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel* fem = &febio.GetFSModel();

	// get the contact interface type
	XMLAtt& atype = tag.Attribute("type");

	// get the surface pair
	const char* szpair = tag.AttributeValue("surface_pair");
	FEBioInputModel::SurfacePair* surfPair = febio.FindSurfacePair(szpair);
	if (surfPair == 0) throw XMLReader::InvalidAttributeValue(tag, "surface_pair", szpair);

	// create a new interfaces
	FSPairedInterface* pci = FEBio::CreatePairedInterface(atype.cvalue(), fem);
	if (pci == nullptr)
	{
		ParseUnknownAttribute(tag, "type");
		return;
	}

	// get the (optional) name
	stringstream ss; ss << "ContactInterface" << CountInterfaces<FSPairedInterface>(*fem) + 1;
	string name = tag.AttributeValue("name", ss.str());
	pci->SetName(name);

	// read the parameters
	ParseModelComponent(pci, tag);

	// assign surfaces
	GModel& m = GetFSModel().GetModel();
	FEBioInputModel::Part* part = surfPair->GetPart();
	assert(part);
	if (part)
	{
		int id1 = surfPair->PrimarySurfaceID();
		if (id1 >= 0)
		{
			if (id1 >= part->Surfaces())
			{
				FEBioInputModel::DomainList& dl = part->GetDomainList(id1 - part->Surfaces());
				GPartList* pg = m.FindPartList(dl.m_name);
				pci->SetPrimarySurface(pg);
			}
			else
			{
				string name1 = part->GetSurface(surfPair->PrimarySurfaceID()).name();
				FSSurface* surf1 = febio.FindNamedSurface(name1.c_str());
				pci->SetPrimarySurface(surf1);
			}
		}

		int id2 = surfPair->SecondarySurfaceID();
		if (id2 >= 0)
		{
			if (id2 >= part->Surfaces())
			{
				FEBioInputModel::DomainList& dl = part->GetDomainList(id2 - part->Surfaces());
				GPartList* pg = m.FindPartList(dl.m_name);
				pci->SetSecondarySurface(pg);
			}
			else
			{
				string name2 = part->GetSurface(surfPair->SecondarySurfaceID()).name();
				FSSurface* surf2 = febio.FindNamedSurface(name2.c_str());
				pci->SetSecondarySurface(surf2);
			}
		}
	}

	// add to the analysis step
	pstep->AddComponent(pci);
}

//-----------------------------------------------------------------------------
void FEBioFormat4::ParseRigidBC(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// get the name 
	stringstream ss;
	ss << "RigidConstraint" << CountRigidBCs<FEBioRigidBC>(fem) + 1;
	std::string name = tag.AttributeValue("name", ss.str());

	// get the type attribute
	const char* sztype = tag.AttributeValue("type");

	// allocate class
	FSRigidBC* pi = FEBio::CreateRigidBC(sztype, &fem);
	if (pi == nullptr)
	{
		ParseUnknownAttribute(tag, "type");
		return;
	}
	pi->SetName(name);
	pstep->AddComponent(pi);

	ParseModelComponent(pi, tag);
}

//-----------------------------------------------------------------------------
void FEBioFormat4::ParseRigidIC(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// get the name 
	stringstream ss;
	ss << "RigidIC" << CountRigidICs<FEBioRigidIC>(fem) + 1;
	std::string name = tag.AttributeValue("name", ss.str());

	// get the type attribute
	const char* sztype = tag.AttributeValue("type");

	// allocate class
	FSRigidIC* pi = FEBio::CreateRigidIC(sztype, &fem);
	if (pi == nullptr)
	{
		ParseUnknownAttribute(tag, "type");
		return;
	}
	pi->SetName(name);
	pstep->AddComponent(pi);
	ParseModelComponent(pi, tag);
}

//-----------------------------------------------------------------------------
void FEBioFormat4::ParseRigidLoad(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// get the name 
	stringstream ss;
	ss << "RigidLoad" << CountConnectors<FSRigidLoad>(fem) + 1;
	std::string name = tag.AttributeValue("name", ss.str());

	// get the type attribute
	const char* sztype = tag.AttributeValue("type");

	// allocate class
	FSRigidLoad* pi = FEBio::CreateRigidLoad(sztype, &fem);
	if (pi == nullptr)
	{
		ParseUnknownAttribute(tag, "type");
		return;
	}
	pi->SetName(name);
	pstep->AddComponent(pi);
	ParseModelComponent(pi, tag);
}

//-----------------------------------------------------------------------------
void FEBioFormat4::ParseRigidConnector(FSStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();

	// get the name 
	const char* szname = tag.AttributeValue("name", true);
	char name[256];
	if (szname == nullptr)
	{
		sprintf(name, "RigidConnector%02d", CountConnectors<FSRigidConnector>(fem) + 1);
		szname = name;
	}

	// get the type attribute
	const char* sztype = tag.AttributeValue("type");

	// allocate class
	FSRigidConnector* pi = FEBio::CreateRigidConnector(sztype, &fem);
	if (pi == nullptr)
	{
		ParseUnknownAttribute(tag, "type");
		return;
	}
	pi->SetName(szname);
	pstep->AddComponent(pi);
	ParseModelComponent(pi, tag);
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

	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();
	GModel& gm = fem.GetModel();

	CPaletteManager& PM = CPaletteManager::GetInstance();
	const CPalette& pal = PM.CurrentPalette();
	int NCOL = pal.Colors();

	vector<GDiscreteElementSet*> set;
	++tag;
	int nc = 0;
	do
	{
		if (tag == "discrete_material")
		{
			const char* sztype = tag.AttributeValue("type");
			const char* szname = tag.AttributeValue("name");

			FSDiscreteMaterial* pdm = FEBio::CreateDiscreteMaterial(sztype, &fem);
			if (pdm == nullptr) throw XMLReader::InvalidTag(tag);

			GDiscreteSpringSet* pg = new GDiscreteSpringSet(&gm);
			pg->SetColor(pal.Color((nc++) % NCOL));
			pg->SetMaterial(pdm);
			pg->SetName(szname);
			fem.GetModel().AddDiscreteObject(pg);
			set.push_back(pg);

			ParseModelComponent(pdm, tag);
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

	FSStep* pstep = m_pBCStep;

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
void FEBioFormat4::ParseNLConstraint(FSStep* pstep, XMLTag& tag)
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
		sprintf(szbuf, "NLConstraint%02d", CountConstraints<FSModelConstraint>(fem)+1);
		szname = szbuf;
	}

	// find the surface
	FSSurface* psurf = nullptr;
	const char* szsurf = tag.AttributeValue("surface", true);
	if (szsurf)
	{
		psurf = febio.FindNamedSurface(szsurf);
		if (psurf == 0) AddLogEntry("Failed creating surface %s", szsurf);
	}

	// get the type
	const char* sztype = tag.AttributeValue("type");

	// create a new constraint
	FSModelConstraint* pi = FEBio::CreateModelConstraint(sztype, &fem);
	if (pi == nullptr)
	{
		ParseUnknownTag(tag);
		return;
	}

	pi->SetName(szname);
	pi->SetItemList(psurf);
	pstep->AddComponent(pi);

	// read parameters
	ParseModelComponent(pi, tag);
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

	// read all load controllers
	++tag;
	do
	{
		if (tag == "load_controller") ParseLoadController(tag);
		else ParseUnknownTag(tag);
		++tag;
	}
	while (!tag.isend());

	return true;
}

bool FEBioFormat4::ParseLoadController(XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// get the type attribute
	const char* sztype = tag.AttributeValue("type");

	// create the load controller
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

	// create a new step (unless this is the first step)
	if (m_pstep == 0)
	{
		FSModel& fem = GetFSModel();
		const char* szmod = FEBio::GetActiveModuleName();
		m_pstep = FEBio::CreateStep(szmod, &fem); assert(m_pstep);
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
