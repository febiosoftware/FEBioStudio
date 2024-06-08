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
#include "FEBioFormat25.h"
#include <FEMLib/FERigidConstraint.h>
#include <GeomLib/FSGroup.h>
#include <GeomLib/GMeshObject.h>
#include <FEMLib/FEInitialCondition.h>
#include <FEMLib/FEBodyLoad.h>
#include <FEMLib/FEModelConstraint.h>
#include <FEMLib/GDiscreteObject.h>
#include <MeshLib/FEElementData.h>
#include <MeshLib/FESurfaceData.h>
#include <MeshLib/FENodeData.h>
#include <GeomLib/GModel.h>
#include <assert.h>
#include <sstream>
#include <FEBioLink/FEBioModule.h>

using std::stringstream;

FEBioFormat25::FEBioFormat25(FEBioFileImport* fileReader, FEBioInputModel& febio) : FEBioFormat(fileReader, febio)
{
	m_geomFormat = 0;
}

FEBioFormat25::~FEBioFormat25()
{
}

FEBioInputModel::Part* FEBioFormat25::DefaultPart()
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

bool FEBioFormat25::ParseSection(XMLTag& tag)
{
	if (m_geomOnly)
	{
		if (tag == "Geometry") ParseGeometrySection(tag);
		else tag.m_preader->SkipTag(tag);
	}
    else if (m_skipGeom)
    {
        if      (tag == "Module"     ) ParseModuleSection    (tag);
		else if (tag == "Control"    ) ParseControlSection   (tag);
		else if (tag == "Material"   ) ParseMaterialSection  (tag);
		else if (tag == "Boundary"   ) ParseBoundarySection  (tag);
		else if (tag == "Constraints") ParseConstraintSection(tag);
		else if (tag == "Loads"      ) ParseLoadsSection     (tag);
		else if (tag == "Contact"    ) ParseContactSection   (tag);
		else if (tag == "Discrete"   ) ParseDiscreteSection  (tag);
		else if (tag == "Initial"    ) ParseInitialSection   (tag);
		else if (tag == "Globals"    ) ParseGlobalsSection   (tag);
		else if (tag == "LoadData"   ) ParseLoadDataSection  (tag);
		else if (tag == "Output"     ) ParseOutputSection    (tag);
		else if (tag == "Step"       ) ParseStepSection      (tag);
        else tag.m_preader->SkipTag(tag);
    }
	else
	{
		if      (tag == "Module"     ) ParseModuleSection    (tag);
		else if (tag == "Control"    ) ParseControlSection   (tag);
		else if (tag == "Material"   ) ParseMaterialSection  (tag);
		else if (tag == "Geometry"   ) ParseGeometrySection  (tag);
		else if (tag == "MeshData"   ) ParseMeshDataSection  (tag);
		else if (tag == "Boundary"   ) ParseBoundarySection  (tag);
		else if (tag == "Constraints") ParseConstraintSection(tag);
		else if (tag == "Loads"      ) ParseLoadsSection     (tag);
		else if (tag == "Contact"    ) ParseContactSection   (tag);
		else if (tag == "Discrete"   ) ParseDiscreteSection  (tag);
		else if (tag == "Initial"    ) ParseInitialSection   (tag);
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
bool FEBioFormat25::ParseModuleSection(XMLTag &tag)
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
	else
	{
		FileReader()->AddLogEntry("Unknown module type. (line %d)", tag.currentLine());
		throw XMLReader::InvalidAttributeValue(tag, "type", atype.m_val.c_str());
		return false;
	}

	const char* sztype = atype.cvalue();
	int moduleId = FEBio::GetModuleId(sztype);
	if (moduleId < 0) { throw XMLReader::InvalidAttributeValue(tag, "type", sztype); }
	FileReader()->GetProject().SetModule(moduleId, false);

	// set the project's active modules
/*
	FSProject& prj = FileReader()->GetProject();
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
//                                G E O M E T R Y
//
//=============================================================================

//-----------------------------------------------------------------------------
//  Parses the geometry section from the xml file
//
bool FEBioFormat25::ParseGeometrySection(XMLTag& tag)
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
		else if (tag == "Part"       ) ParseGeometryPart       (tag);
		else if (tag == "Instance"   ) ParseGeometryInstance   (tag);
		else ParseUnknownTag(tag);

		++tag;
	} while (!tag.isend());

	if (m_geomFormat == 1)
	{
		// create a new instance
		FEBioInputModel& febio = GetFEBioModel();
		FEBioInputModel::Part* part = DefaultPart();
		part->Update();
		FEBioInputModel::PartInstance* instance = new FEBioInputModel::PartInstance(part);
		febio.AddInstance(instance);
		instance->SetName(part->GetName());
	}

	// don't forget to update the mesh
	GetFEBioModel().UpdateGeometry();

	return true;
}

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseGeometryNodes(FEBioInputModel::Part* part, XMLTag& tag)
{
	if (part == 0) throw XMLReader::InvalidTag(tag);

	vector<FEBioInputModel::NODE> nodes; nodes.reserve(10000);

	// create a node set if the name is definde
	const char* szname = tag.AttributeValue("name", true);
	std::string name;
	if (szname) name = szname;

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
	}
	while (!tag.isend());

	// create nodes
	int nn = nodes.size();
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
	if (name.empty() == false)
	{
		vector<int> nodeList(nn);
		for (int i = 0; i < nn; ++i) nodeList[i] = nodes[i].id - 1;
		FEBioInputModel::NodeSet nset(name, nodeList);
		part->AddNodeSet(nset);
	}
}

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseGeometryElements(FEBioInputModel::Part* part, XMLTag& tag)
{
	if (part == 0) throw XMLReader::InvalidTag(tag);

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

	// read the elements
	vector<FEBioInputModel::ELEM> elem;
	elem.reserve(25000);

	++tag;
	do
	{
		FEBioInputModel::ELEM el;
		if (tag == "elem")
		{
			int id = tag.AttributeValue<int>("id", -1);
			el.id = id;
			tag.value(el.n, FSElement::MAX_NODES);
			elem.push_back(el);
		}
		else throw XMLReader::InvalidTag(tag);

		++tag;
	}
	while (!tag.isend());


	// create elements
	FSMesh& mesh = *part->GetFEMesh();
	int NTE = mesh.Elements();
	int elems = (int)elem.size();
	mesh.Create(0, elems + NTE);

	// generate the part id
	int pid = part->Domains() - 1;

	// read element data
	for (int i = NTE; i<elems + NTE; ++i)
	{
		FSElement& el = mesh.Element(i);
		FEBioInputModel::ELEM& els = elem[i - NTE];
		el.SetType(ntype);
		el.m_gid = pid;
		dom->AddElement(i);
		el.m_nid = els.id;
		for (int j = 0; j < el.Nodes(); ++j) el.m_node[j] = els.n[j];
	}
}


//-----------------------------------------------------------------------------
void FEBioFormat25::ParseGeometryNodeSet(FEBioInputModel::Part* part, XMLTag& tag)
{
	// make sure there is a name attribute
	std::string name = tag.AttributeValue("name");

	// list to store node numbers
	vector<int> list;

	if (tag.isleaf())
	{
		tag.value(list);
	}
	else
	{

		++tag;
		do
		{
			if (tag == "node")
			{
				int nid = tag.AttributeValue<int>("id", -1);
				if (nid == -1) throw XMLReader::MissingAttribute(tag, "id");
				list.push_back(nid);
			}
			else if (tag == "node_set")
			{
				const char* szset = tag.AttributeValue("nset");
				FEBioInputModel::NodeSet* ps = part->FindNodeSet(szset);
				if (ps == 0) throw XMLReader::InvalidAttributeValue(tag, "nset", szset);
				list.insert(list.end(), ps->nodeList().begin(), ps->nodeList().end());
			}
			else throw XMLReader::InvalidTag(tag);
			++tag;
		}
		while (!tag.isend());
	}

	// create a new node set
	part->AddNodeSet(FEBioInputModel::NodeSet(name, list));
}

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseGeometryDiscreteSet(FEBioInputModel::Part* part, XMLTag& tag)
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
void FEBioFormat25::ParseGeometrySurfacePair(FEBioInputModel::Part* part, XMLTag& tag)
{
	if (part == 0) throw XMLReader::InvalidTag(tag);

	std::string name = tag.AttributeValue("name");
	int surf1ID = -1, surf2ID = -1;
	++tag;
	do
	{
		if (tag == "master")
		{
			const char* szsurf = tag.AttributeValue("surface");
			surf2ID = part->FindSurfaceIndex(szsurf);
			if (surf2ID == -1) throw XMLReader::InvalidAttributeValue(tag, "master", szsurf);
		}
		else if (tag == "slave")
		{
			const char* szsurf = tag.AttributeValue("surface");
			surf1ID = part->FindSurfaceIndex(szsurf);
			if (surf1ID == -1) throw XMLReader::InvalidAttributeValue(tag, "slave", szsurf);
		}
		else throw XMLReader::InvalidTag(tag);
		++tag;
	}
	while (!tag.isend());

	part->AddSurfacePair(FEBioInputModel::SurfacePair(name, surf1ID, surf2ID));
}

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseGeometrySurface(FEBioInputModel::Part* part, XMLTag& tag)
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
void FEBioFormat25::ParseGeometryElementSet(FEBioInputModel::Part* part, XMLTag& tag)
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
void FEBioFormat25::ParseGeometryPart(XMLTag& tag)
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
void FEBioFormat25::ParseGeometryInstance(XMLTag& tag)
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
bool FEBioFormat25::ParseMeshDataSection(XMLTag& tag)
{
	if (tag.isleaf()) return true;

	++tag;
	do
	{
		if (tag == "NodeData")
		{
			ParseNodeData(tag);
		}
		else if (tag == "ElementData")
		{
			ParseElementData(tag);
        }
		else if (tag == "SurfaceData")
		{
			ParseSurfaceData(tag);
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

			// TODO: Not sure if this is always true! Looks like some 
			// data is read into the actual mesh. The test for Qactive
			// is a hack! Need to figure this out! 
			if (e1.m_Qactive)
			{
				e0.m_Q = e1.m_Q;
				e0.m_Qactive = e1.m_Qactive;
				e0.m_fiber = e1.m_fiber;
			}
		}
	}

	return true;
}

bool FEBioFormat25::ParseNodeData(XMLTag& tag)
{
	// Read the data and store it as a mesh data section
	FEBioInputModel& feb = GetFEBioModel();

	// Make sure to skip generators
	const char* szgen = tag.AttributeValue("generator", true);
	if (szgen) {
		ParseUnknownTag(tag); return false;
	}

	// read the nodal data
	const char* szset = tag.AttributeValue("node_set");
	FSNodeSet* pg = feb.BuildFENodeSet(szset);
	if (pg == nullptr) { ParseUnknownTag(tag); return false; }

	// get the name
	const char* szname = tag.AttributeValue("name");

	FSMesh* mesh = pg->GetMesh();
	FENodeData* pd = mesh->AddNodeDataField(szname, pg, DATA_SCALAR);

	double val;
	int lid;
	++tag;
	do
	{
		tag.AttributePtr("lid")->value(lid);
		tag.value(val);

		pd->setScalar(lid - 1, val);

		++tag;
	} while (!tag.isend());

	return true;
}

bool FEBioFormat25::ParseElementData(XMLTag& tag)
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

			++tag;
			do
			{
				int lid = tag.AttributeValue<int>("lid", 0) - 1;
				if (lid >= 0)
				{
					int id = dom->ElementID(lid);
					FSElement& el = mesh->Element(id);

					vec3d a, d;
					++tag;
					do
					{
						if      (tag == "a") tag.value(a);
						else if (tag == "d") tag.value(d);
						++tag;
					} while (!tag.isend());
					el.setAxes(a, d);
				}
				++tag;
			} while (!tag.isend());
		}
		else
		{
			FSElemSet* pg = feb.BuildFEElemSet(szset);
			if (pg == nullptr) ParseUnknownAttribute(tag, "elem_set");
			else
			{
				vector<int> items = pg->CopyItems();
				vector<int>::iterator it = items.begin();
				FSMesh* mesh = pg->GetMesh();
				++tag;
				do
				{
					int lid = tag.AttributeValue<int>("lid", 0) - 1;
					if ((lid >= 0) && (it != items.end()))
					{
						int id = *it; // looks like this is already zero-based
						FSElement& el = mesh->Element(id);
						vec3d a, d;
						++tag;
						do
						{
							if      (tag == "a") tag.value(a);
							else if (tag == "d") tag.value(d);
							++tag;
						} while (!tag.isend());
						el.setAxes(a, d);
					}
					++it;
					++tag;
				} while (!tag.isend());
				delete pg;
			}
		}
	}
	else
	{
		// Read the data and store it as a mesh data section
		FEBioInputModel& feb = GetFEBioModel();
		FSModel& fem = feb.GetFSModel();

		const char* szgen = tag.AttributeValue("generator", true);
		if (szgen)
		{
			const char* szset = tag.AttributeValue("elem_set");
			if (strcmp(szgen, "surface-to-surface map") == 0)
			{
/*				FSSurfaceToSurfaceMap* s2s = new FSSurfaceToSurfaceMap;
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
					if      (tag == "bottom_surface") { tag.value(tmp); s2s->SetBottomSurface(tmp); }
					else if (tag == "top_surface"   ) { tag.value(tmp); s2s->SetTopSurface(tmp); }
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
								else ParseUnknownTag(tag);
								++tag;
							} while (!tag.isend());
//							p->SetLoadCurve(lc);
						}
					}
					else ParseUnknownTag(tag);
					++tag;
				} while (!tag.isend());

				feb.GetFSModel().AddDataMap(s2s);
				*/
			}
		}
		else 
		{
			const char* szset = tag.AttributeValue("elem_set");
			FEBioInputModel::Domain* dom = feb.FindDomain(szset);
			if (dom)
			{
				FSElemSet* pg = feb.BuildFEElemSet(dom);
				if (pg)
				{
					FSMesh* mesh = pg->GetMesh();
					pg->GetGObject()->AddFEElemSet(pg);
					FEElementData* pd = mesh->AddElementDataField(var->cvalue(), pg, DATA_SCALAR);

					double scale = tag.AttributeValue("scale", 1.0);
					pd->SetScaleFactor(scale);

					double val;
					int lid;
					++tag;
					do
					{
						tag.AttributePtr("lid")->value(lid);
						tag.value(val);

						(*pd)[lid - 1] = val;

						++tag;
					} while (!tag.isend());
				}
				else ParseUnknownTag(tag);
			}
			else ParseUnknownTag(tag);
		}
	}

	return true;
}

bool FEBioFormat25::ParseSurfaceData(XMLTag& tag)
{
	FEBioInputModel& feb = GetFEBioModel();

	XMLAtt* name = tag.AttributePtr("name");
	XMLAtt* dataTypeAtt = tag.AttributePtr("datatype");
	XMLAtt* surf = tag.AttributePtr("surface");

	DATA_TYPE dataType;
	if ((dataTypeAtt == nullptr) || (*dataTypeAtt == "scalar")) dataType = DATA_SCALAR;
	else if (*dataTypeAtt == "vector") dataType = DATA_VEC3;
	else return false;

	FSSurface* feSurf = feb.BuildFESurface(surf->cvalue());
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

//=============================================================================
//
//                                B O U N D A R Y 
//
//=============================================================================

//-----------------------------------------------------------------------------
//  Parses the boundary section from the xml file (format 2.5)
bool FEBioFormat25::ParseBoundarySection(XMLTag& tag)
{
	if (tag.isleaf()) return true;

	++tag;
	do
	{
		if      (tag == "fix"       ) ParseBCFixed     (m_pBCStep, tag);
		else if (tag == "prescribe" ) ParseBCPrescribed(m_pBCStep, tag);
		else if (tag == "rigid_body") ParseBCRigidBody (m_pBCStep, tag);
		else if (tag == "rigid"     ) ParseBCRigid     (m_pBCStep, tag);
		else ParseUnknownTag(tag);
		++tag;
	}
	while (!tag.isend());

	return true;
}

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseBCFixed(FSStep* pstep, XMLTag &tag)
{
	// get the bc attribute
	XMLAtt& abc = tag.Attribute("bc");

	// figure out the bc value
	int bc = GetDOFCode(abc.cvalue());
	if (bc == 0) throw XMLReader::InvalidAttributeValue(tag, "bc", abc.cvalue());

	FEBioInputModel& febio = GetFEBioModel();

	// get the mesh
	FSModel& fem = GetFSModel();

	// get the node set
	const char* szset = tag.AttributeValue("node_set");
	FSNodeSet* pg = febio.BuildFENodeSet(szset);
	if (pg == 0) FileReader()->AddLogEntry("Cannot find node_set \"%s\"", szset);

	// create the constraint
	char szname[256] = { 0 };
	if (bc < 8)
	{
		FSFixedDisplacement* pbc = new FSFixedDisplacement(&fem, pg, bc, pstep->GetID());
		sprintf(szname, "FixedDisplacement%02d", CountBCs<FSFixedDisplacement>(fem)+1);
		pbc->SetName(szname);
		pstep->AddComponent(pbc);
	}
	else if (bc < 64)
	{
		bc = bc >> 3;
		FSFixedRotation* pbc = new FSFixedRotation(&fem, pg, bc, pstep->GetID());
		sprintf(szname, "FixedRotation%02d", CountBCs<FSFixedRotation>(fem)+1);
		pbc->SetName(szname);
		pstep->AddComponent(pbc);
	}
	else if (bc == 64)
	{
		FSFixedTemperature* pbc = new FSFixedTemperature(&fem, pg, 1, pstep->GetID());
		sprintf(szname, "FixedTemperature%02d", CountBCs<FSFixedTemperature>(fem)+1);
		pbc->SetName(szname);
		pstep->AddComponent(pbc);
	}
	else if (bc == 128)
	{
		FSFixedFluidPressure* pbc = new FSFixedFluidPressure(&fem, pg, 1, pstep->GetID());
		sprintf(szname, "FixedFluidPressure%02d", CountBCs<FSFixedFluidPressure>(fem)+1);
		pbc->SetName(szname);
		pstep->AddComponent(pbc);
	}
	else if ((bc < 2048) && (bc >= 256))
	{
		bc = bc >> 8;
		FSFixedFluidVelocity* pbc = new FSFixedFluidVelocity(&fem, pg, bc, pstep->GetID());
		sprintf(szname, "FixedFluidVelocity%02d", CountBCs<FSFixedFluidVelocity>(fem)+1);
		pbc->SetName(szname);
		pstep->AddComponent(pbc);
	}
	else if (bc == 2048)
	{
		FSFixedFluidDilatation* pbc = new FSFixedFluidDilatation(&fem, pg, 1, pstep->GetID());
		sprintf(szname, "FixedFluidDilatation%02d", CountBCs<FSFixedFluidDilatation>(fem)+1);
		pbc->SetName(szname);
		pstep->AddComponent(pbc);
	}
	else if (bc < (1 << 15))
	{
		bc = bc >> 12;
		FSFixedShellDisplacement* pbc = new FSFixedShellDisplacement(&fem, pg, bc, pstep->GetID());
		sprintf(szname, "FixedShellDisplacement%02d", CountBCs<FSFixedShellDisplacement>(fem)+1);
		pbc->SetName(szname);
		pstep->AddComponent(pbc);
	}
	else
	{
		bc = bc >> 15;
		if (bc < 256)
		{
			FSFixedConcentration* pbc = new FSFixedConcentration(&fem, pg, bc, pstep->GetID());
			sprintf(szname, "FixedConcentration%02d", CountBCs<FSFixedConcentration>(fem)+1);
			pbc->SetName(szname);
			pstep->AddComponent(pbc);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseBCPrescribed(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// determine bc
	XMLAtt& abc = tag.Attribute("bc");
	int bc = 0;
	if      (abc == "x") bc = 0;
	else if (abc == "y") bc = 1;
	else if (abc == "z") bc = 2;
	else if (abc == "T") bc = 3;
	else if (abc == "p") bc = 4;
	else if (abc == "wx") bc = 5;
	else if (abc == "wy") bc = 6;
	else if (abc == "wz") bc = 7;
	else if (abc == "ef") bc = 8;
	else if (abc == "c") bc = 9;
	else if (abc == "c1") bc = 9;
	else if (abc == "c2") bc = 10;
	else if (abc == "c3") bc = 11;
	else if (abc == "c4") bc = 12;
	else if (abc == "c5") bc = 13;
	else if (abc == "c6") bc = 14;
	else if (abc == "u" ) bc = 15;
	else if (abc == "v") bc = 16;
	else if (abc == "w") bc = 17;
    else if (abc == "sx" ) bc = 18;
    else if (abc == "sy") bc = 19;
    else if (abc == "sz") bc = 20;
	else throw XMLReader::InvalidAttributeValue(tag, "bc", abc.cvalue());

	XMLAtt& set = tag.Attribute("node_set");
	FSNodeSet* pg = febio.BuildFENodeSet(set.cvalue());
	if (pg == 0) FileReader()->AddLogEntry("Cannot find node_set \"%s\"", set.cvalue());

	// make a new boundary condition
	FSPrescribedDOF* pbc = 0;
	switch (bc)
	{
	case 0:
	case 1:
	case 2: pbc = new FSPrescribedDisplacement (&fem, pg, bc, 1, pstep->GetID()); break;
	case 3: pbc = new FSPrescribedTemperature  (&fem, pg, bc, pstep->GetID()); break;
	case 4: pbc = new FSPrescribedFluidPressure(&fem, pg, 1, pstep->GetID()); break;
	case 5:
	case 6:
	case 7:
		bc = bc - 5;
		pbc = new FSPrescribedFluidVelocity(&fem, pg, bc, 1, pstep->GetID());
		break;
	case 8:
		pbc = new FSPrescribedFluidDilatation(&fem, pg, 1, pstep->GetID());
		break;
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		bc = bc - 9;
		pbc = new FSPrescribedConcentration(&fem, pg, bc, 1.0, pstep->GetID());
		break;
	case 15:
	case 16:
	case 17:
		bc = bc - 15;
		pbc = new FSPrescribedRotation(&fem, pg, bc, 1.0, pstep->GetID());
		break;
    case 18:
    case 19:
    case 20:
        bc = bc - 18;
        pbc = new FSPrescribedShellDisplacement(&fem, pg, bc, 1.0, pstep->GetID());
        break;
	}
	if (pbc == 0) throw XMLReader::InvalidAttributeValue(tag, "bc", abc.cvalue());

	// get the optional name
	string name;
	const char* szname = tag.AttributeValue("name", true);
	if(pg)
    {
        if (szname == 0) name = pg->GetName(); else name = szname;
    }    
	pbc->SetName(name);
	pstep->AddComponent(pbc);

	++tag;
	do
	{
		if (tag == "relative")
		{
			bool brel;
			tag.value(brel);
			pbc->SetRelativeFlag(brel);
		}
		else if (tag == "scale")
		{
			Param* pp = pbc->GetParam("scale"); assert(pp);
			if (pp && pp->IsVariable()) ParseMappedParameter(tag, pp);
			int lc = tag.AttributeValue<int>("lc", -1);
			if (lc != -1) febio.AddParamCurve(&pbc->GetParam(FSPrescribedDOF::SCALE), lc-1);
		}
		else if (tag == "value")
		{
			// NOTE: This parameter is deprecated, but we support it here to assist in 
			//       converting older files. The map basically gets assigned to the "scale" parameter.
			const char* sznodedata = tag.AttributeValue("node_data", true);
			if (sznodedata == nullptr) sznodedata = tag.szvalue();
			Param* pp = pbc->GetParam("scale"); assert(pp);
			if (pp && pp->IsVariable())
			{
				pp->SetParamType(Param_STRING);
				pp->SetStringValue(sznodedata);
			}
		}
		else ParseUnknownTag(tag);
		++tag;
	}
	while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseBCRigid(FSStep* pstep, XMLTag& tag)
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

	// read rigid material ID
	int nrb = tag.AttributeValue<int>("rb", -1);
	if (nrb == -1) throw XMLReader::MissingAttribute(tag, "rb");

	// read node set
	const char* szset = tag.AttributeValue("node_set");
	FEBioInputModel& febio = GetFEBioModel();
	FSNodeSet* pg = febio.BuildFENodeSet(szset);

	GMaterial* pmat = 0;
	if ((nrb > 0) && (nrb <= febio.Materials())) pmat = febio.GetMaterial(nrb - 1);
	else FileReader()->AddLogEntry("Invalid material in rigid contact.");

	// create the interface
	FSRigidInterface* pi = new FSRigidInterface(&fem, pmat, pg, pstep->GetID());
	pi->SetName(name.c_str());
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseBCRigidBody(FSStep* pstep, XMLTag& tag)
{
	// get the material ID
	int nid = tag.Attribute("mat").value<int>() - 1;

	// get the rigid material
	FEBioInputModel& febio = GetFEBioModel();
	FSModel* fem = &febio.GetFSModel();
	GMaterial* pgm = 0;
	if (nid >= 0) pgm = febio.GetMaterial(nid);
	int matid = (pgm ? pgm->GetID() : -1);
	assert(dynamic_cast<FSRigidMaterial*>(pgm->GetMaterialProperties()));

	// get the (optional) name 
	bool hasName = false;
	char szname[256] = { 0 };
	const char* sz = tag.AttributeValue("name", true);
	if (sz) { strcpy(szname, sz); hasName = true; }

	FSRigidFixed* pc = 0; // fixed constraint
	double v;

	++tag;
	do
	{
		// get the bc
		XMLAtt& bc = tag.Attribute("bc");
		int nbc = -1;
		if (bc == "x") nbc = 0;
		else if (bc == "y") nbc = 1;
		else if (bc == "z") nbc = 2;
		else if (bc == "Rx") nbc = 3;
		else if (bc == "Ry") nbc = 4;
		else if (bc == "Rz") nbc = 5;
		else throw XMLReader::InvalidAttributeValue(tag, "bc", bc.cvalue());

		if (tag == "fixed")
		{
			if (pc == 0) pc = new FSRigidFixed(fem, pstep->GetID());
			pc->SetDOF(nbc, true);
		}
		else if (tag == "prescribed")
		{
			int lc = tag.AttributeValue<int>("lc", 0);
			tag.value(v);
			FSRigidDisplacement* pd = new FSRigidDisplacement(nbc, matid, v, pstep->GetID());

			static int n = 1;
			if (hasName == false) sprintf(szname, "RigidDisplacement%02d", n++);
			pd->SetName(szname);
			pstep->AddRC(pd);
			febio.AddParamCurve(&pd->GetParam(FSRigidDisplacement::VALUE), lc - 1);
		}
		else if (tag == "force")
		{
			int lc = tag.AttributeValue<int>("lc", 0);
			tag.value(v);
			FSRigidForce* pf = new FSRigidForce(nbc, matid, v, pstep->GetID());

			const char* sztype = tag.AttributeValue("type", true);
			if (sztype)
			{
				int ntype = 0;
				if (strcmp(sztype, "follow") == 0) pf->SetForceType(1);
				if (strcmp(sztype, "ramp") == 0) pf->SetForceType(2);
			}

			static int n = 1;
			if (hasName == false) sprintf(szname, "RigidForce%02d", n++);
			pf->SetName(szname);
			pstep->AddRC(pf);
			if (lc > 0) febio.AddParamCurve(&pf->GetParam(FSRigidDisplacement::VALUE), lc - 1);
		}
		else ParseUnknownTag(tag);

		++tag;
	}
	while (!tag.isend());

	if (pc)
	{
		static int n = 1;
		pc->SetMaterialID(pgm ? pgm->GetID() : -1);
		if (hasName == false) sprintf(szname, "RigidFixed%02d", n++);
		pc->SetName(szname);
		pstep->AddRC(pc);
	}
}

//=============================================================================
//
//                                L O A D S
//
//=============================================================================

//-----------------------------------------------------------------------------
//!  Parses the loads section from the xml file
bool FEBioFormat25::ParseLoadsSection(XMLTag& tag)
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
void FEBioFormat25::ParseNodeLoad(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// read the bc attribute
	XMLAtt& abc = tag.Attribute("bc");
	int bc = 0;
	if (abc == "x") bc = 0;
	else if (abc == "y") bc = 1;
	else if (abc == "z") bc = 2;
	else if (abc == "p") bc = 3;
	else if (abc == "c1") bc = 4;
	else if (abc == "c2") bc = 5;
	else if (abc == "c3") bc = 6;
	else if (abc == "c4") bc = 7;
	else if (abc == "c5") bc = 8;
	else if (abc == "c6") bc = 9;
	else throw XMLReader::InvalidAttributeValue(tag, "bc", abc.cvalue());

	// get the load curve ID
	XMLAtt& aset = tag.Attribute("node_set");

	// get the (optional) name
	string name;
	const char* szname = tag.AttributeValue("name", true);
	if (szname == nullptr)
	{
		char szname[256];
		sprintf(szname, "ForceNodeset%02d", CountLoads<FSNodalDOFLoad>(fem) + 1);
		name = szname;
	}
	else name = szname;

	// create the node set
	FSNodeSet* pg = febio.BuildFENodeSet(aset.cvalue());
	if (pg == 0) throw XMLReader::InvalidAttributeValue(tag, aset);
	char szbuf[256];
	sprintf(szbuf, "ForceNodeset%02d", CountLoads<FSNodalDOFLoad>(fem)+1);
	pg->SetName(szbuf);

	// create the nodal load
	FSNodalDOFLoad* pbc = new FSNodalDOFLoad(&fem, pg, bc, 1, pstep->GetID());
	pbc->SetName(name);
	pstep->AddComponent(pbc);

	// assign nodes to node sets
	++tag;
	do
	{
		if (tag == "scale")
		{
			int lc = tag.Attribute("lc").value<int>() - 1;
			if (lc == -1) throw XMLReader::InvalidAttributeValue(tag, "lc", 0);
			febio.AddParamCurve(&pbc->GetParam(FSNodalDOFLoad::LOAD), lc);

			double val;
			tag.value(val);
			pbc->SetLoad(val);
		}
		++tag;
	}
	while (!tag.isend());
}

//-----------------------------------------------------------------------------
//! Parses the surface_load section.
void FEBioFormat25::ParseSurfaceLoad(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();

	std::string comment = tag.comment();

	// find the surface
	XMLAtt& surf = tag.Attribute("surface");
	FSSurface* psurf = febio.BuildFESurface(surf.cvalue());
	if (psurf == 0) throw XMLReader::InvalidAttributeValue(tag, surf);

	// get the optional name
	string name = tag.AttributeValue<string>("name", "");

	FSSurfaceLoad* psl = 0;
	XMLAtt& att = tag.Attribute("type");
	if      (att == "pressure"           ) psl = ParseLoadPressure          (tag);
	else if (att == "traction"           ) psl = ParseLoadTraction          (tag);
	else if (att == "fluidflux"          ) psl = ParseLoadFluidFlux         (tag);
	else if (att == "soluteflux"         ) psl = ParseLoadSoluteFlux        (tag);
	else if (att == "concentration flux" ) psl = ParseConcentrationFlux     (tag);
	else if (att == "normal_traction"    ) psl = ParseLoadNormalTraction    (tag);
    else if (att == "matching_osm_coef"             ) psl = ParseLoadMatchingOsmoticCoefficient(tag);
	else if (att == "heatflux"           ) psl = ParseLoadHeatFlux          (tag);
	else if (att == "convective_heatflux") psl = ParseLoadConvectiveHeatFlux(tag);
	else if (att == "fluid viscous traction") psl = ParseLoadFluidTraction     (tag);
    else if (att == "fluid pressure"                ) psl = ParseLoadFluidPressure               (tag);
    else if (att == "fluid velocity"                ) psl = ParseLoadFluidVelocity               (tag);
    else if (att == "fluid normal velocity"         ) psl = ParseLoadFluidNormalVelocity         (tag);
    else if (att == "fluid rotational velocity"     ) psl = ParseLoadFluidRotationalVelocity     (tag);
    else if (att == "fluid resistance"              ) psl = ParseLoadFluidFlowResistance         (tag);
    else if (att == "fluid RCR"                     ) psl = ParseLoadFluidFlowRCR                (tag);
    else if (att == "fluid backflow stabilization"  ) psl = ParseLoadFluidBackFlowStabilization  (tag);
    else if (att == "fluid tangential stabilization") psl = ParseLoadFluidTangentialStabilization(tag);
    else if (att == "fluid-FSI traction"            ) psl = ParseLoadFSITraction       (tag);
    else if (att == "biphasic-FSI traction"         ) psl = ParseLoadBFSITraction      (tag);
	else ParseUnknownAttribute(tag, "type");

	if (psl)
	{
		// set the name if it was defined
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
FSSurfaceLoad* FEBioFormat25::ParseLoadPressure(XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// create a new surface load
	FSPressureLoad* pbc = new FSPressureLoad(&fem);

	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "PressureLoad%d", CountLoads<FSPressureLoad>(fem) + 1);
	pbc->SetName(szname);

	// read the parameters
	ReadParameters(*pbc, tag);

	return pbc;
}

//-----------------------------------------------------------------------------
FSSurfaceLoad* FEBioFormat25::ParseLoadTraction(XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// create a new surface load
	FSSurfaceTraction* pbc = new FSSurfaceTraction(&fem);

	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "TractionLoad%d", CountLoads<FSSurfaceTraction>(fem) + 1);
	pbc->SetName(szname);

	// read the parameters
	++tag;
	do
	{
		if (tag == "scale")
		{
			int lc = tag.Attribute("lc").value<int>() - 1;
			febio.AddParamCurve(&pbc->GetParam(FSSurfaceTraction::LOAD), lc);

			double s = 0.0;
			tag.value(s);
			pbc->SetScale(s);
		}
		else if (tag == "traction")
		{
			vec3d t; tag.value(t);
			pbc->SetTraction(t);
		}
		++tag;
	} 
	while (!tag.isend());

	return pbc;
}

//-----------------------------------------------------------------------------
FSSurfaceLoad* FEBioFormat25::ParseLoadFluidTraction(XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// create a new surface load
	FSFluidTraction* pbc = new FSFluidTraction(&fem);

	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "FluidTractionLoad%d", CountLoads<FSFluidTraction>(fem));
	pbc->SetName(szname);

	// read the parameters
	++tag;
	do
	{
		if (tag == "scale")
		{
			int lc = tag.Attribute("lc").value<int>() - 1;
			febio.AddParamCurve(&pbc->GetParam(FSFluidTraction::LOAD), lc);

			double s;
			tag.value(s);
			pbc->SetScale(s);
		}
		else if (tag == "traction")
		{
			vec3d t; tag.value(t);
			pbc->SetTraction(t);
		}
		++tag;
	}
	while (!tag.isend());

	return pbc;
}

//-----------------------------------------------------------------------------
FSSurfaceLoad* FEBioFormat25::ParseLoadFluidPressure(XMLTag& tag)
{
    FSModel& fem = GetFSModel();
    
    // create a new surface load
    FSFluidPressureLoad* pbc = new FSFluidPressureLoad(&fem);
    
    // set the name
    char szname[256] = { 0 };
    sprintf(szname, "FluidPressureLoad%d", CountLoads<FSFluidPressureLoad>(fem) + 1);
    pbc->SetName(szname);
    
    // read the parameters
    ReadParameters(*pbc, tag);
    
    return pbc;
}

//-----------------------------------------------------------------------------
FSSurfaceLoad* FEBioFormat25::ParseLoadFluidVelocity(XMLTag& tag)
{
    FSModel& fem = GetFSModel();
    FSFluidVelocity* psl = new FSFluidVelocity(&fem);
    char szname[128] = { 0 };
    sprintf(szname, "FluidVelocity%02d", CountLoads<FSFluidVelocity>(fem)+1);
    psl->SetName(szname);
    
	ReadParameters(*psl, tag);
    
    return psl;
}

//-----------------------------------------------------------------------------
FSSurfaceLoad* FEBioFormat25::ParseLoadFluidNormalVelocity(XMLTag& tag)
{
    FEBioInputModel& febio = GetFEBioModel();
    FSModel& fem = GetFSModel();
    FSFluidNormalVelocity* psl = new FSFluidNormalVelocity(&fem);
    char szname[128] = { 0 };
    sprintf(szname, "FluidNormalVelocity%02d", CountLoads<FSFluidNormalVelocity>(fem)+1);
    psl->SetName(szname);
    
    ++tag;
    do
    {
        if (tag == "velocity")
        {
            double v; tag.value(v);
            psl->SetLoad(v);
            
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(&psl->GetParam(FSFluidNormalVelocity::LOAD), lc);
        }
        else if (tag == "prescribe_nodal_velocities") {
            bool b; tag.value(b);
            psl->SetBP(b);
        }
        else if (tag == "parabolic") {
            bool b; tag.value(b);
            psl->SetBParab(b);
        }
        else if (tag == "prescribe_rim_pressure") {
            bool b; tag.value(b);
            psl->SetBRimP(b);
        }
        else ParseUnknownTag(tag);
        ++tag;
    } while (!tag.isend());
    
    return psl;
}

//-----------------------------------------------------------------------------
FSSurfaceLoad* FEBioFormat25::ParseLoadFluidRotationalVelocity(XMLTag& tag)
{
    FEBioInputModel& febio = GetFEBioModel();
    FSModel& fem = GetFSModel();
    FSFluidRotationalVelocity* psl = new FSFluidRotationalVelocity(&fem);
    char szname[128] = { 0 };
    sprintf(szname, "FluidRotationalVelocity%02d", CountLoads<FSFluidRotationalVelocity>(fem)+1);
    psl->SetName(szname);
    
    ++tag;
    do
    {
        if (tag == "angular_speed")
        {
            double as; tag.value(as);
            psl->SetLoad(as);
            
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(&psl->GetParam(FSFluidRotationalVelocity::LOAD), lc);
        }
        else if (tag == "axis") {
            vec3d a; tag.value(a);
            psl->SetAxis(a);
        }
        else if (tag == "origin") {
            vec3d o; tag.value(o);
            psl->SetOrigin(o); }
        else ParseUnknownTag(tag);
        ++tag;
    } while (!tag.isend());
    
    return psl;
}

//-----------------------------------------------------------------------------
FSSurfaceLoad* FEBioFormat25::ParseLoadFluidFlowResistance(XMLTag& tag)
{
    FEBioInputModel& febio = GetFEBioModel();
    FSModel& fem = GetFSModel();
    FSFluidFlowResistance* psl = new FSFluidFlowResistance(&fem);
    char szname[128] = { 0 };
    sprintf(szname, "FluidFlowResistance%02d", CountLoads<FSFluidFlowResistance>(fem)+1);
    psl->SetName(szname);
    
    ++tag;
    do
    {
        if (tag == "R")
        {
            double R; tag.value(R);
            psl->SetLoad(R);
        }
        else if (tag == "pressure_offset")
        {
            double po; tag.value(po);
            psl->SetPO(po);
            
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(&psl->GetParam(FSFluidFlowResistance::PO), lc);
        }
        else ParseUnknownTag(tag);
        ++tag;
    } while (!tag.isend());
    
    return psl;
}

//-----------------------------------------------------------------------------
FSSurfaceLoad* FEBioFormat25::ParseLoadFluidFlowRCR(XMLTag& tag)
{
    FEBioInputModel& febio = GetFEBioModel();
    FSModel& fem = GetFSModel();
    FSFluidFlowRCR* psl = new FSFluidFlowRCR(&fem);
    char szname[128] = { 0 };
    sprintf(szname, "FluidFlowRCR%02d", CountLoads<FSFluidFlowRCR>(fem)+1);
    psl->SetName(szname);
    
    ++tag;
    do
    {
        if (tag == "R")
        {
            double R; tag.value(R);
            psl->SetLoad(R);
        }
        else if (tag == "Rd")
        {
            double rd; tag.value(rd);
            psl->SetRD(rd);
            
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(&psl->GetParam(FSFluidFlowRCR::RD), lc);
        }
        else if (tag == "capacitance")
        {
            double  co; tag.value(co);
            psl->SetCO(co);
            
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(&psl->GetParam(FSFluidFlowRCR::CO), lc);
        }
        else if (tag == "pressure_offset")
        {
            double po; tag.value(po);
            psl->SetPO(po);
            
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(&psl->GetParam(FSFluidFlowRCR::PO), lc);
        }
        else if (tag == "initial_pressure")
        {
            double ip; tag.value(ip);
            psl->SetIP(ip);
        }
        else ParseUnknownTag(tag);
        ++tag;
    } while (!tag.isend());
    
    return psl;
}

//-----------------------------------------------------------------------------
FSSurfaceLoad* FEBioFormat25::ParseLoadFluidBackFlowStabilization(XMLTag& tag)
{
    FEBioInputModel& febio = GetFEBioModel();
    FSModel& fem = GetFSModel();
    FSFluidBackflowStabilization* psl = new FSFluidBackflowStabilization(&fem);
    char szname[128] = { 0 };
    sprintf(szname, "FluidBackflowStabilization%02d", CountLoads<FSFluidBackflowStabilization>(fem)+1);
    psl->SetName(szname);
    
    ++tag;
    do
    {
        if (tag == "beta")
        {
            double b; tag.value(b);
            psl->SetLoad(b);
            
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(&psl->GetParam(FSFluidBackflowStabilization::LOAD), lc);
        }
        else ParseUnknownTag(tag);
        ++tag;
    } while (!tag.isend());
    
    return psl;
}

//-----------------------------------------------------------------------------
FSSurfaceLoad* FEBioFormat25::ParseLoadFluidTangentialStabilization(XMLTag& tag)
{
    FEBioInputModel& febio = GetFEBioModel();
    FSModel& fem = GetFSModel();
    FSFluidTangentialStabilization* psl = new FSFluidTangentialStabilization(&fem);
    char szname[128] = { 0 };
    sprintf(szname, "FluidTangentialStabilization%02d", CountLoads<FSFluidTangentialStabilization>(fem)+1);
    psl->SetName(szname);
    
    ++tag;
    do
    {
        if (tag == "beta")
        {
            double b; tag.value(b);
            psl->SetLoad(b);
            
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(&psl->GetParam(FSFluidTangentialStabilization::LOAD), lc);
        }
        else ParseUnknownTag(tag);
        ++tag;
    } while (!tag.isend());
    
    return psl;
}

//-----------------------------------------------------------------------------
FSSurfaceLoad* FEBioFormat25::ParseLoadFSITraction(XMLTag& tag)
{
    FEBioInputModel& febio = GetFEBioModel();
    FSModel& fem = GetFSModel();
    FSFSITraction* psl = new FSFSITraction(&fem);
    char szname[128] = { 0 };
    sprintf(szname, "FSInterfaceTraction%02d", CountLoads<FSFSITraction>(fem)+1);
    psl->SetName(szname);
    
    return psl;
}

//-----------------------------------------------------------------------------
FSSurfaceLoad* FEBioFormat25::ParseLoadBFSITraction(XMLTag& tag)
{
    FEBioInputModel& febio = GetFEBioModel();
    FSModel& fem = GetFSModel();
    FSBFSITraction* psl = new FSBFSITraction(&fem);
    char szname[128] = { 0 };
    sprintf(szname, "BFSInterfaceTraction%02d", CountLoads<FSBFSITraction>(fem)+1);
    psl->SetName(szname);
    
    return psl;
}

//-----------------------------------------------------------------------------
FSSurfaceLoad* FEBioFormat25::ParseLoadFluidFlux(XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// create a new surface load
	FSFluidFlux* pbc = new FSFluidFlux(&fem);

	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "FluidFlux%d", CountLoads<FSFluidFlux>(fem));
	pbc->SetName(szname);

	// read the parameters
	++tag;
	do
	{
		if (tag == "linear")
		{
			bool b; tag.value(b);
			pbc->SetLinearFlag(b);
		}
		else if (tag == "mixture")
		{
			bool b; tag.value(b);
			pbc->SetMixtureFlag(b);
		}
		else if (tag == "flux")
		{
			int lc = tag.Attribute("lc").value<int>() - 1;
			febio.AddParamCurve(&pbc->GetParam(FSFluidFlux::LOAD), lc);

			double s; tag.value(s);
			pbc->SetLoad(s);
		}
		++tag;
	}
	while (!tag.isend());

	return pbc;
}

//-----------------------------------------------------------------------------
FSSurfaceLoad* FEBioFormat25::ParseLoadSoluteFlux(XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// create a new surface load
	FSSoluteFlux* pbc = new FSSoluteFlux(&fem);

	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "SoluteFlux%d", CountLoads<FSSoluteFlux>(fem));
	pbc->SetName(szname);

	// read the parameters
	++tag;
	do
	{
		if (tag == "linear")
		{
			bool b; tag.value(b);
			pbc->SetLinearFlag(b);
		}
		else if (tag == "solute_id")
		{
			int n; tag.value(n);
			pbc->SetBC(n - 1);
		}
		else if (tag == "flux")
		{
			XMLAtt* alc = tag.AttributePtr("lc");
			if (alc)
			{
				int lc = alc->value<int>() - 1;
				febio.AddParamCurve(&pbc->GetParam(FSSoluteFlux::LOAD), lc);
			}
			double s; tag.value(s);
			pbc->SetLoad(s);
		}
		++tag;
	}
	while (!tag.isend());

	return pbc;
}

//-----------------------------------------------------------------------------
FSSurfaceLoad* FEBioFormat25::ParseConcentrationFlux(XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// create a new surface load
	FSConcentrationFlux* pcf = new FSConcentrationFlux(&fem);

	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "ConcentrationFlux%d", CountLoads<FSConcentrationFlux>(fem) +1);
	pcf->SetName(szname);

	// read the parameters
	++tag;
	do
	{
		if (tag == "solute_id")
		{
			int n; tag.value(n);
			pcf->SetSoluteID(n - 1);
		}
		else if (tag == "flux")
		{
			XMLAtt* alc = tag.AttributePtr("lc");
			if (alc)
			{
				int lc = alc->value<int>() - 1;
				febio.AddParamCurve(&pcf->GetParam(FSConcentrationFlux::FLUX), lc);
			}
			double s; tag.value(s);
			pcf->SetFlux(s);
		}
		else ParseUnknownTag(tag);
		++tag;
	}
	while (!tag.isend());

	return pcf;
}

//-----------------------------------------------------------------------------
FSSurfaceLoad* FEBioFormat25::ParseLoadNormalTraction(XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// create a new surface load
	FSBPNormalTraction* pbc = new FSBPNormalTraction(&fem);

	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "NormalTraction%d", CountLoads<FSBPNormalTraction>(fem));
	pbc->SetName(szname);

	// read the parameters
	++tag;
	do
	{
		if (tag == "linear")
		{
			bool b; tag.value(b);
			pbc->SetLinearFlag(b);
		}
		else if (tag == "mixture")
		{
			// I initially called the "mixture" flag the "effective" flag
			// but I had the values swapped.
			// Whence this little hack
			bool b; tag.value(b);
			pbc->SetMixtureFlag(!b);
		}
		else if (tag == "effective")
		{
			bool b; tag.value(b);
			pbc->SetMixtureFlag(b);
		}
		else if (tag == "traction")
		{
			int lc = tag.Attribute("lc").value<int>() - 1;
			febio.AddParamCurve(&pbc->GetParam(FSBPNormalTraction::LOAD), lc);
			double s; tag.value(s);
			pbc->SetLoad(s);
		}
		++tag;
	}
	while (!tag.isend());

	return pbc;
}

//-----------------------------------------------------------------------------
FSSurfaceLoad* FEBioFormat25::ParseLoadMatchingOsmoticCoefficient(XMLTag& tag)
{
    FEBioInputModel& febio = GetFEBioModel();
    FSModel& fem = GetFSModel();

    // create a new surface load
    FSMatchingOsmoticCoefficient* pbc = new FSMatchingOsmoticCoefficient(&fem);

    // set the name
    char szname[256] = { 0 };
    sprintf(szname, "MatchingOsmCoef%d", CountLoads<FSMatchingOsmoticCoefficient>(fem));
    pbc->SetName(szname);

    // read the parameters
    ++tag;
    do
    {
        if (tag == "shell_bottom")
        {
            bool b; tag.value(b);
            pbc->SetShellBottomFlag(b);
        }
        else if (tag == "ambient_pressure")
        {
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(&pbc->GetParam(FSMatchingOsmoticCoefficient::AMBP), lc);
            double s; tag.value(s);
            pbc->SetLoadP(s);
        }
        else if (tag == "ambient_osmolarity")
        {
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(&pbc->GetParam(FSMatchingOsmoticCoefficient::AMBC), lc);
            double s; tag.value(s);
            pbc->SetLoadC(s);
        }
        ++tag;
    }
    while (!tag.isend());

    return pbc;
}

//-----------------------------------------------------------------------------
FSSurfaceLoad* FEBioFormat25::ParseLoadHeatFlux(XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// create a new surface load
	FSHeatFlux* pbc = new FSHeatFlux(&fem);

	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "HeatFlux%d", CountLoads<FSHeatFlux>(fem));
	pbc->SetName(szname);

	// read the parameters
	++tag;
	do
	{
		if (tag == "flux")
		{
			int lc = tag.Attribute("lc").value<int>() - 1;
			febio.AddParamCurve(&pbc->GetParam(FSHeatFlux::FLUX), lc);
			double s; tag.value(s);
			pbc->SetLoad(s);
		}
		++tag;
	}
	while (!tag.isend());

	return pbc;
}

//-----------------------------------------------------------------------------
FSSurfaceLoad* FEBioFormat25::ParseLoadConvectiveHeatFlux(XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// create a new surface load
	FSConvectiveHeatFlux* pbc = new FSConvectiveHeatFlux(&fem);

	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "ConvectiveHeatFlux%d", CountLoads<FSConvectiveHeatFlux>(fem));
	pbc->SetName(szname);

	// read the parameters
	++tag;
	do
	{
		if (tag == "Ta")
		{
			int lc = tag.Attribute("lc").value<int>() - 1;
			febio.AddParamCurve(&pbc->GetParam(FSConvectiveHeatFlux::TREF), lc);
			double s; tag.value(s);
			pbc->SetTemperature(s);
		}
		if (tag == "hc")
		{
			double s; tag.value(s);
			pbc->SetCoefficient(s);
		}
		++tag;
	}
	while (!tag.isend());

	return pbc;
}

//-----------------------------------------------------------------------------
//! Parses the body_load section.
void FEBioFormat25::ParseBodyLoad(FSStep* pstep, XMLTag& tag)
{
	XMLAtt& att = tag.Attribute("type");
	if      (att == "const"      ) ParseBodyForce (pstep, tag);
	else if (att == "non-const"  ) ParseNonConstBodyForce(pstep, tag);
	else if (att == "heat_source") ParseHeatSource(pstep, tag);
    else if (att == "centrifugal") ParseCentrifugalBodyForce(pstep, tag);
	else ParseUnknownAttribute(tag, "type");
}

//-----------------------------------------------------------------------------
bool FEBioFormat25::ParseInitialSection(XMLTag& tag)
{
	// make sure the section is not empty
	if (tag.isleaf()) return true;

	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	char szname[256] = {0};

	++tag;
	do
	{
		if (tag == "rigid_body")
		{
			// get the material ID
			int nid = tag.Attribute("mat").value<int>() - 1;

			// get the rigid material
			GMaterial* pgm = 0;
			if (nid >= 0) pgm = febio.GetMaterial(nid);
			int matid = (pgm ? pgm->GetID() : -1);
			assert(dynamic_cast<FSRigidMaterial*>(pgm->GetMaterialProperties()));

			++tag;
			do
			{
				if (tag == "initial_velocity")
				{
					FSRigidVelocity* pv = new FSRigidVelocity(&fem, m_pBCStep->GetID());
					vec3d vi;
					tag.value(vi);
					pv->SetVelocity(vi);
					pv->SetMaterialID(matid);

					static int n = 1;
					sprintf(szname, "RigidVelocity%02d", n++);
					pv->SetName(szname);
					m_pBCStep->AddRC(pv);

					++tag;
				}
				else if (tag == "initial_angular_velocity")
				{
					FSRigidAngularVelocity* pv = new FSRigidAngularVelocity(&fem, m_pBCStep->GetID());
					vec3d vi;
					tag.value(vi);
					pv->SetVelocity(vi);
					pv->SetMaterialID(matid);

					static int n = 1;
					sprintf(szname, "RigiAngulardVelocity%02d", n++);
					pv->SetName(szname);
					m_pBCStep->AddRC(pv);

					++tag;
				}
				else ParseUnknownTag(tag);
			}
			while (!tag.isend());
		}
		else if (tag == "init")
		{
			// determine bc
			XMLAtt& abc = tag.Attribute("bc");
			int bc = 0;
			if (abc == "t") bc = 3;
			else if (abc == "p") bc = 4;
			else if (abc == "vx") bc = 5;
			else if (abc == "vy") bc = 6;
			else if (abc == "vz") bc = 7;
//			else if (abc == "ef") bc = 8;
			else if (abc == "c") bc = 9;
			else if (abc == "c1") bc = 9;
			else if (abc == "c2") bc = 10;
			else if (abc == "c3") bc = 11;
			else if (abc == "c4") bc = 12;
			else if (abc == "c5") bc = 13;
			else if (abc == "c6") bc = 14;
            else if (abc == "q") bc = 15;
            else if (abc == "d") bc = 16;
            else if (abc == "d1") bc = 16;
            else if (abc == "d2") bc = 17;
            else if (abc == "d3") bc = 18;
            else if (abc == "d4") bc = 19;
            else if (abc == "d5") bc = 20;
            else if (abc == "d6") bc = 21;
			else if (abc == "svx") bc = 22;
			else if (abc == "svy") bc = 23;
			else if (abc == "svz") bc = 24;
            else if (abc == "ef") bc = 25;
			else throw XMLReader::InvalidAttributeValue(tag, "bc", abc.cvalue());

			double val = 0.0;

			const char* szset = tag.AttributeValue("node_set");
			FSNodeSet* pg = febio.BuildFENodeSet(szset);
			if (pg == 0) AddLogEntry("Missing node_set \"%s\"", szset);

			++tag;
			do
			{
				if (tag == "value") tag.value(val);
				else ParseUnknownTag(tag);
				++tag;
			}
			while (!tag.isend());

			// create a new initial velocity BC
			FSInitialCondition* pic = 0;
			char szname[64] = { 0 };
			switch (bc)
			{
			case 3:
				pic = new FSInitTemperature(&fem, pg, val, m_pBCStep->GetID());
				sprintf(szname, "InitialTemperature%02d", CountICs<FSInitTemperature>(fem)+1);
				break;
			case 4:
				pic = new FSInitFluidPressure(&fem, pg, val, m_pBCStep->GetID());
				sprintf(szname, "InitialFluidPressure%02d", CountICs<FSInitFluidPressure>(fem)+1);
				break;
			case 5:
				pic = new FSNodalVelocities(&fem, pg, vec3d(val, 0, 0), m_pBCStep->GetID());
				sprintf(szname, "InitialVelocity%02d", CountICs<FSNodalVelocities>(fem)+1);
				break;
			case 6:
				pic = new FSNodalVelocities(&fem, pg, vec3d(0, val, 0), m_pBCStep->GetID());
				sprintf(szname, "InitialVelocity%02d", CountICs<FSNodalVelocities>(fem)+1);
				break;
			case 7:
				pic = new FSNodalVelocities(&fem, pg, vec3d(0, 0, val), m_pBCStep->GetID());
				sprintf(szname, "InitialVelocity%02d", CountICs<FSNodalVelocities>(fem)+1);
				break;
            case 15:
                pic = new FSInitShellFluidPressure(&fem, pg, val, m_pBCStep->GetID());
				sprintf(szname, "InitialShellFluidPressure%02d", CountICs<FSInitShellFluidPressure>(fem)+1);
                break;
			case 22:
				pic = new FSNodalShellVelocities(&fem, pg, vec3d(val, 0, 0), m_pBCStep->GetID());
				sprintf(szname, "InitShellVelocity%02d", CountICs<FSNodalShellVelocities>(fem) +1);
				break;
			case 23:
				pic = new FSNodalShellVelocities(&fem, pg, vec3d(0, val, 0), m_pBCStep->GetID());
				sprintf(szname, "InitShellVelocity%02d", CountICs<FSNodalShellVelocities>(fem) +1);
				break;
			case 24:
				pic = new FSNodalShellVelocities(&fem, pg, vec3d(0, 0, val), m_pBCStep->GetID());
				sprintf(szname, "InitShellVelocity%02d", CountICs<FSNodalShellVelocities>(fem) +1);
				break;
            case 25:
                pic = new FSInitFluidDilatation(&fem, pg, val, m_pBCStep->GetID());
                sprintf(szname, "InitialFluidDilatation%02d", CountICs<FSInitFluidDilatation>(fem)+1);
                break;
			default:
				if ((bc >= 9) && (bc <= 14))
				{
					int nsol = bc - 9;
					pic = new FSInitConcentration(&fem, pg, nsol, val, m_pBCStep->GetID());
					sprintf(szname, "InitConcentration%02d", CountICs<FSInitConcentration>(fem)+1);
				}
                else if ((bc >= 16) && (bc <= 21))
                {
                    int nsol = bc - 16;
                    pic = new FSInitShellConcentration(&fem, pg, nsol, val, m_pBCStep->GetID());
					sprintf(szname, "InitShellConcentration%02d", CountICs<FSInitShellConcentration>(fem)+1);
                }
			}

			if (pic)
			{
				pic->SetName(szname);
				m_pBCStep->AddComponent(pic);
			}
		}
		else if (tag == "ic")
		{
			const char* sztype = tag.AttributeValue("type");

			char szbuf[64] = { 0 };
			const char* szname = tag.AttributeValue("name", true);

			if (strcmp(sztype, "prestrain") == 0)
			{
				FSInitPrestrain* pip = new FSInitPrestrain(&fem);

				if (szname == nullptr)
				{
					sprintf(szbuf, "InitPrestrain%d", CountConstraints<FSInitPrestrain>(fem) + 1);
					szname = szbuf;
				}
				pip->SetName(szname);
				m_pBCStep->AddComponent(pip);

				ReadParameters(*pip, tag);
			}
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
bool FEBioFormat25::ParseContactSection(XMLTag& tag)
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
void FEBioFormat25::ParseContact(FSStep *pstep, XMLTag &tag)
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
		if (surfPair == 0) AddLogEntry("Missing surface_pair \"%s\"", szpair);

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
		else 
		{
			// NOTE: These interfaces are all obsolete since FEBio 2.7, but continue to be supported for now.
			if      (atype == "sliding_with_gaps"          ) pci = ParseContactSliding   (pstep, tag);
			else if (atype == "facet-to-facet sliding"     ) pci = ParseContactF2FSliding(pstep, tag);
			else if (atype == "sliding-tension-compression") pci = ParseContactTC        (pstep, tag);
			else if (atype == "tied"                       ) pci = ParseContactTied      (pstep, tag);
			else if (atype == "sliding2"                   ) pci = ParseContactBiphasic  (pstep, tag);
			else if (atype == "sliding3"                   ) pci = ParseContactSolute         (pstep, tag);
			else ParseUnknownTag(tag);
		}

		if (pci)
		{
			const char* szname = tag.AttributeValue("name", true);
			if (szname) pci->SetName(szname);

			// read the parameters
			if (tag.isleaf() == false)
			{
				++tag;
				do
				{
					// try to read the parameters
					if (ReadParam(*pci, tag) == false)
					{
						if (tag == "flip_slave")
						{
							Param* pp = pci->GetParam("flip_primary"); assert(pp);
							if (pp)
							{
								bool b = false;
								tag.value(b);
								pp->SetBoolValue(b);
							}
						}
						else if (tag == "flip_master")
						{
							Param* pp = pci->GetParam("flip_secondary"); assert(pp);
							if (pp)
							{
								bool b = false;
								tag.value(b);
								pp->SetBoolValue(b);
							}
						}
						else ParseUnknownTag(tag);
					}
					++tag;
				} while (!tag.isend());
			}


			// assign surfaces
            if (surfPair)
            {
                FEBioInputModel::Part* part = surfPair->GetPart();
                assert(part);
                if (part)
                {
                    string name1 = part->GetSurface(surfPair->PrimarySurfaceID()).name();
                    string name2 = part->GetSurface(surfPair->SecondarySurfaceID()).name();
                    FSSurface* surf1 = febio.BuildFESurface(name1.c_str());
                    FSSurface* surf2  = febio.BuildFESurface(name2.c_str());

                    pci->SetPrimarySurface(surf1);
                    pci->SetSecondarySurface(surf2);
                }
            }
			
            // add to the analysis step
			pstep->AddComponent(pci);
		}
	}
}

//-----------------------------------------------------------------------------
FSPairedInterface* FEBioFormat25::ParseContactSliding(FSStep* pstep, XMLTag& tag)
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
FSPairedInterface* FEBioFormat25::ParseContactF2FSliding(FSStep* pstep, XMLTag& tag)
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
FSPairedInterface* FEBioFormat25::ParseContactBiphasic(FSStep* pstep, XMLTag& tag)
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
FSPairedInterface* FEBioFormat25::ParseContactSolute(FSStep* pstep, XMLTag& tag)
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
FSPairedInterface* FEBioFormat25::ParseContactMultiphasic(FSStep* pstep, XMLTag& tag)
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
FSPairedInterface* FEBioFormat25::ParseContactTied(FSStep *pstep, XMLTag &tag)
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
FSPairedInterface* FEBioFormat25::ParseContactF2FTied(FSStep *pstep, XMLTag &tag)
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
FSPairedInterface* FEBioFormat25::ParseContactTiedElastic(FSStep* pstep, XMLTag& tag)
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
FSPairedInterface* FEBioFormat25::ParseContactSticky(FSStep *pstep, XMLTag &tag)
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
FSPairedInterface* FEBioFormat25::ParseContactPeriodic(FSStep *pstep, XMLTag &tag)
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
FSPairedInterface* FEBioFormat25::ParseContactTC(FSStep *pstep, XMLTag &tag)
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
FSPairedInterface* FEBioFormat25::ParseContactTiedPoro(FSStep *pstep, XMLTag &tag)
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
FSPairedInterface* FEBioFormat25::ParseContactTiedMultiphasic(FSStep *pstep, XMLTag &tag)
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
FSPairedInterface* FEBioFormat25::ParseContactGapHeatFlux(FSStep* pstep, XMLTag& tag)
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
void FEBioFormat25::ParseRigidWall(FSStep* pstep, XMLTag& tag)
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

	// assign surface
	const char* szsurf = tag.AttributeValue("surface", true);
	if (szsurf)
	{
		FSSurface* surface = febio.BuildFESurface(szsurf);
		if (surface) pci->SetItemList(surface);
	}

	// read parameters
	++tag;
	do
	{
		// read parameters
		if (tag == "plane"    )
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
		else ReadParam(*pci, tag);
		++tag;
	}
	while (!tag.isend());

	// add interface to step
	pstep->AddComponent(pci);
}

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseContactJoint(FSStep *pstep, XMLTag &tag)
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
void FEBioFormat25::ParseConnector(FSStep *pstep, XMLTag &tag, const int rc)
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
	pstep->AddRigidConnector(pi);

	// NOTE: In febio3, the default value is false, but in FEBioStudio it is true
	Param* autoPenalty = pi->GetParam("auto_penalty");
	if (autoPenalty) autoPenalty->SetIntValue(0);

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
void FEBioFormat25::ParseLinearConstraint(FSStep* pstep, XMLTag& tag)
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
					if (strcmp(szbc, "x") == 0) dof.bc = 0;
					else if (strcmp(szbc, "y") == 0) dof.bc = 1;
					else if (strcmp(szbc, "z") == 0) dof.bc = 2;
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

bool FEBioFormat25::ParseDiscreteSection(XMLTag& tag)
{
	// make sure the section is not empty
	if (tag.isleaf()) return true;

	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();
	GModel& gm = fem.GetModel();

	char buf[256] = { 0 };

	vector<GDiscreteElementSet*> set;
	int n = 1;
	++tag;
	do
	{
		if (tag == "discrete_material")
		{
			const char* sztype = tag.AttributeValue("type");
			const char* szname = tag.AttributeValue("name", true);
			if (szname == nullptr)
			{
				sprintf(buf, "discrete_material%d", n++);
				szname = buf;
			}

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
					if (tag == "force")
					{
						double F;
						tag.value(F);
						int lc = tag.AttributeValue<int>("lc", -1);
						mat->SetFloatValue(0, F);
						if (lc > 0) GetFEBioModel().AddParamCurve(mat->GetParamPtr(0), lc - 1);
					}
					else ParseUnknownTag(tag);
					++tag;
				} while (!tag.isend());
				set.push_back(pg);
			}
			else
			{
				FSMaterial* mat = ParseMaterial(tag, sztype);
				FSDiscreteMaterial* dmat = dynamic_cast<FSDiscreteMaterial*>(mat);
				if (dmat)
				{
					GDiscreteSpringSet* pg = new GDiscreteSpringSet(&gm);
					pg->SetMaterial(dmat);
					pg->SetName(szname);
					fem.GetModel().AddDiscreteObject(pg);
					set.push_back(pg);
				}
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
				AddLogEntry("Failed building discrete set \"%s\"", szset);
			}
		}
		else ParseUnknownTag(tag);
		++tag;
	}
	while (!tag.isend());

	return true;
}

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseBodyForce(FSStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();

	FSConstBodyForce* pbl = new FSConstBodyForce(&fem, pstep->GetID());
	pstep->AddComponent(pbl);

	++tag;
	do
	{
		if (ReadParam(*pbl, tag) == false) ParseUnknownTag(tag);
		else ++tag;
	}
	while (!tag.isend());

	char szname[256] = { 0 };
	sprintf(szname, "BodyForce%02d", CountLoads<FSConstBodyForce>(fem));
	pbl->SetName(szname);
}

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseNonConstBodyForce(FSStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();

	FSNonConstBodyForce* pbl = new FSNonConstBodyForce(&fem, pstep->GetID());
	pstep->AddComponent(pbl);

	++tag;
	do
	{
		if (ReadParam(*pbl, tag) == false) ParseUnknownTag(tag);
		else ++tag;
	} 
	while (!tag.isend());

	char szname[256] = { 0 };
	sprintf(szname, "BodyForce%02d", CountLoads<FSConstBodyForce>(fem));
	pbl->SetName(szname);
}

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseHeatSource(FSStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();
	FEBioInputModel& febio = GetFEBioModel();

	FSHeatSource* phs = new FSHeatSource(&fem, pstep->GetID());
	pstep->AddComponent(phs);

	++tag;
	const char* szlc;
	do
	{
		if (tag == "Q")
		{
			szlc = tag.AttributeValue("lc");
			double v; tag.value(v);
			phs->SetLoad(v);

			febio.AddParamCurve(&phs->GetParam(FSHeatSource::LOAD), atoi(szlc) - 1);
		}
		else throw XMLReader::InvalidTag(tag);

		++tag;
	} while (!tag.isend());

	char szname[256] = { 0 };
	sprintf(szname, "HeatSource%02d", CountLoads<FSHeatSource>(fem));
	phs->SetName(szname);
}

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseCentrifugalBodyForce(FSStep *pstep, XMLTag &tag)
{
    FSModel& fem = GetFSModel();
	FEBioInputModel& febio = GetFEBioModel();

    FSCentrifugalBodyForce* phs = new FSCentrifugalBodyForce(&fem, pstep->GetID());
    pstep->AddComponent(phs);
    
    ++tag;
    const char* szlc;
    do
    {
        if (tag == "angular_speed")
        {
            szlc = tag.AttributeValue("lc");
            double v; tag.value(v);
            phs->SetLoad(v);
			febio.AddParamCurve(&phs->GetParam(FSCentrifugalBodyForce::ANGSPD), atoi(szlc) - 1);
		}
        else if (ReadParam(*phs, tag) == false) ParseUnknownTag(tag);
        
        ++tag;
    } while (!tag.isend());
    
    char szname[256] = { 0 };
    sprintf(szname, "CentrifugalBodyForce%02d", CountLoads<FSCentrifugalBodyForce>(fem));
    phs->SetName(szname);
}

//=============================================================================
//
//                            C O N S T R A I N T S
//
//=============================================================================

//-----------------------------------------------------------------------------
bool FEBioFormat25::ParseConstraintSection(XMLTag& tag)
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
			else if (strcmp(sztype, "rigid spherical joint"  ) == 0) ParseConnector(pstep, tag, 0);
			else if (strcmp(sztype, "rigid revolute joint"   ) == 0) ParseConnector(pstep, tag, 1);
			else if (strcmp(sztype, "rigid prismatic joint"  ) == 0) ParseConnector(pstep, tag, 2);
			else if (strcmp(sztype, "rigid cylindrical joint") == 0) ParseConnector(pstep, tag, 3);
			else if (strcmp(sztype, "rigid planar joint"     ) == 0) ParseConnector(pstep, tag, 4);
            else if (strcmp(sztype, "rigid lock"             ) == 0) ParseConnector(pstep, tag, 5);
			else if (strcmp(sztype, "rigid spring"           ) == 0) ParseConnector(pstep, tag, 6);
			else if (strcmp(sztype, "rigid damper"           ) == 0) ParseConnector(pstep, tag, 7);
			else if (strcmp(sztype, "rigid angular damper"   ) == 0) ParseConnector(pstep, tag, 8);
			else if (strcmp(sztype, "rigid contractile force") == 0) ParseConnector(pstep, tag, 9);
			else if (strcmp(sztype, "generic rigid joint"    ) == 0) ParseConnector(pstep, tag, 10);
			else if (strcmp(sztype, "rigid joint") == 0) ParseContactJoint(pstep, tag);
			else ParseUnknownAttribute(tag, "type");
		}
		else ParseUnknownTag(tag);
		++tag;
	}
	while (!tag.isend());

	return true;
}

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseVolumeConstraint(FSStep* pstep, XMLTag& tag)
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
	FSSurface* psurf = febio.BuildFESurface(szsurf);
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
void FEBioFormat25::ParseSymmetryPlane(FSStep* pstep, XMLTag& tag)
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
	FSSurface* psurf = febio.BuildFESurface(szsurf);
	if (psurf == 0) throw XMLReader::InvalidAttributeValue(tag, "surface", szsurf);

	// create a new symmetry plane
	FSSymmetryPlane* pi = new FSSymmetryPlane(&fem);
	pi->SetName(szname);
	pi->SetItemList(psurf);
	pstep->AddComponent(pi);

	// read parameters
	ReadParameters(*pi, tag);
}

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseNrmlFldVlctSrf(FSStep* pstep, XMLTag& tag)
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
    FSSurface* psurf = febio.BuildFESurface(szsurf);
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
void FEBioFormat25::ParseFrictionlessFluidWall(FSStep* pstep, XMLTag& tag)
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
    FSSurface* psurf = febio.BuildFESurface(szsurf);
    if (psurf == 0) throw XMLReader::InvalidAttributeValue(tag, "surface", szsurf);

    // create a new frictionless fluid wall
    FSFrictionlessFluidWall* pi = new FSFrictionlessFluidWall(&fem);
    pi->SetName(szname);
    pi->SetItemList(psurf);
    pstep->AddComponent(pi);

    // read parameters
    ReadParameters(*pi, tag);
}

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseInSituStretchConstraint(FSStep* pstep, XMLTag& tag)
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
void FEBioFormat25::ParsePrestrainConstraint(FSStep* pstep, XMLTag& tag)
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

//=============================================================================
//
//                                S T E P
//
//=============================================================================

//-----------------------------------------------------------------------------
bool FEBioFormat25::ParseStepSection(XMLTag &tag)
{
	// make sure the section is not empty
	if (tag.isleaf()) return true;

	char szname[128] = { 0 };

	const char* szval = tag.AttributeValue("name", true);
	if (szval) strcpy(szname, szval);

	++tag;

	// create a new step (unless this is the first step)
	if (m_pstep == 0) m_pstep = NewStep(GetFSModel(), m_nAnalysis, szname);
	m_pBCStep = m_pstep;

	do
	{
		if      (tag == "Control"    ) ParseControlSection   (tag);
		else if (tag == "Initial"    ) ParseInitialSection   (tag);
		else if (tag == "Boundary"   ) ParseBoundarySection  (tag);
		else if (tag == "Constraints") ParseConstraintSection(tag);
		else if (tag == "Loads"      ) ParseLoadsSection     (tag);
		else if (tag == "Contact"    ) ParseContactSection   (tag);
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
