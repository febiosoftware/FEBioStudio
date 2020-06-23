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
#include "FEBioFormat25.h"
#include <FEMLib/FERigidConstraint.h>
#include <MeshTools/FEGroup.h>
#include <GeomLib/GMeshObject.h>
#include <FEMLib/FEInitialCondition.h>
#include <FEMLib/FEBodyLoad.h>
#include <FEMLib/FEModelConstraint.h>
#include <MeshTools/GDiscreteObject.h>
#include <MeshTools/FEElementData.h>
#include <MeshTools/FESurfaceData.h>
#include <MeshTools/GModel.h>
#include <assert.h>
#include <sstream>

FEBioFormat25::FEBioFormat25(FEBioImport* fileReader, FEBioModel& febio) : FEBioFormat(fileReader, febio)
{
	m_geomFormat = 0;
}

FEBioFormat25::~FEBioFormat25()
{
}

FEBioModel::Part* FEBioFormat25::DefaultPart()
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

bool FEBioFormat25::ParseSection(XMLTag& tag)
{
	if (m_geomOnly)
	{
		if (tag == "Geometry") ParseGeometrySection(tag);
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
		FEBioModel& febio = GetFEBioModel();
		FEBioModel::Part* part = DefaultPart();
		part->Update();
		FEBioModel::PartInstance* instance = new FEBioModel::PartInstance(part);
		febio.AddInstance(instance);
		instance->SetName(part->GetName());
	}

	// don't forget to update the mesh
	GetFEBioModel().UpdateGeometry();

	return true;
}

//-----------------------------------------------------------------------------
// TODO: Create a node set if the name attribute is defined
void FEBioFormat25::ParseGeometryNodes(FEBioModel::Part* part, XMLTag& tag)
{
	if (part == 0) throw XMLReader::InvalidTag(tag);

	vector<FEBioModel::NODE> nodes; nodes.reserve(10000);

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
	}
	while (!tag.isend());

	// create nodes
	int nn = nodes.size();
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
}

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseGeometryElements(FEBioModel::Part* part, XMLTag& tag)
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

	// read the elements
	vector<FEBioModel::ELEM> elem;
	elem.reserve(25000);

	++tag;
	do
	{
		FEBioModel::ELEM el;
		if (tag == "elem")
		{
			int id = tag.AttributeValue<int>("id", -1);
			el.id = id;
			tag.value(el.n, FEElement::MAX_NODES);
			elem.push_back(el);
		}
		else throw XMLReader::InvalidTag(tag);

		++tag;
	}
	while (!tag.isend());


	// create elements
	FEMesh& mesh = *part->GetFEMesh();
	int NTE = mesh.Elements();
	int elems = (int)elem.size();
	mesh.Create(0, elems + NTE);

	// generate the part id
	int pid = part->Domains() - 1;

	// read element data
	for (int i = NTE; i<elems + NTE; ++i)
	{
		FEElement& el = mesh.Element(i);
		FEBioModel::ELEM& els = elem[i - NTE];
		el.SetType(ntype);
		el.m_gid = pid;
		dom->AddElement(i);
		el.m_nid = els.id;
		for (int j = 0; j < el.Nodes(); ++j) el.m_node[j] = els.n[j];
	}
}


//-----------------------------------------------------------------------------
void FEBioFormat25::ParseGeometryNodeSet(FEBioModel::Part* part, XMLTag& tag)
{
	// make sure there is a name attribute
	std::string name = tag.AttributeValue("name");

	// list to store node numbers
	vector<int> list;

	if (tag.isleaf())
	{
		tag.value(list);

		// make the list zero-based
		for (int i = 0; i < list.size(); ++i) list[i]--;
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
				list.push_back(nid - 1);
			}
			else if (tag == "node_set")
			{
				const char* szset = tag.AttributeValue("nset");
				FEBioModel::NodeSet* ps = part->FindNodeSet(szset);
				if (ps == 0) throw XMLReader::InvalidAttributeValue(tag, "nset", szset);
				list.insert(list.end(), ps->nodeList().begin(), ps->nodeList().end());
			}
			else throw XMLReader::InvalidTag(tag);
			++tag;
		}
		while (!tag.isend());
	}

	// create a new node set
	part->AddNodeSet(FEBioModel::NodeSet(name, list));
}

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseGeometryDiscreteSet(FEBioModel::Part* part, XMLTag& tag)
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
void FEBioFormat25::ParseGeometrySurfacePair(FEBioModel::Part* part, XMLTag& tag)
{
	if (part == 0) throw XMLReader::InvalidTag(tag);

	std::string name = tag.AttributeValue("name");
	int masterID = -1, slaveID = -1;
	++tag;
	do
	{
		if (tag == "master")
		{
			const char* szsurf = tag.AttributeValue("surface");
			masterID = part->FindSurfaceIndex(szsurf);
			if (masterID == -1) throw XMLReader::InvalidAttributeValue(tag, "master", szsurf);
		}
		else if (tag == "slave")
		{
			const char* szsurf = tag.AttributeValue("surface");
			slaveID = part->FindSurfaceIndex(szsurf);
			if (slaveID == -1) throw XMLReader::InvalidAttributeValue(tag, "slave", szsurf);
		}
		else throw XMLReader::InvalidTag(tag);
		++tag;
	}
	while (!tag.isend());

	part->AddSurfacePair(FEBioModel::SurfacePair(name, masterID, slaveID));
}

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseGeometrySurface(FEBioModel::Part* part, XMLTag& tag)
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
void FEBioFormat25::ParseGeometryElementSet(FEBioModel::Part* part, XMLTag& tag)
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
void FEBioFormat25::ParseGeometryPart(XMLTag& tag)
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
void FEBioFormat25::ParseGeometryInstance(XMLTag& tag)
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
bool FEBioFormat25::ParseMeshDataSection(XMLTag& tag)
{
	if (tag.isleaf()) return true;

	++tag;
	do
	{
		if (tag == "ElementData")
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
					
					double h[FEElement::MAX_NODES] = {0};
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
							for (int i=0; i<m; ++i) el.m_h[i] = h[i];
						}
						++tag;
					}
					while (!tag.isend());
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
                            vec3d c = a^b;
                            b = c^a;
                            // make sure they are unit vectors
                            b.Normalize();
                            c.Normalize();
                            el.m_Q = mat3d(a.x, b.x, c.x,
                                           a.y, b.y, c.y,
                                           a.z, b.z, c.z);
                            el.m_fiber = a;
                        }
                        ++tag;
                    }
                    while (!tag.isend());
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
                            }
                            while (!tag.isend());
                            a.Normalize();
                            c = a ^ d; c.Normalize();
                            b = c ^ a; b.Normalize();
                            el.m_Q = mat3d(a.x, b.x, c.x,
                                           a.y, b.y, c.y,
                                           a.z, b.z, c.z);
                            el.m_Qactive = true;
                        }
                        ++tag;
                    }
                    while (!tag.isend());
                }
                else ParseUnknownTag(tag);
            }
			else
			{
				// Read the data and store it as a mesh data section
				FEBioModel& feb = GetFEBioModel();

				const char* szset = tag.AttributeValue("elem_set");
				FEBioModel::Domain* dom = feb.FindDomain(szset);
				if (dom)
				{
					FEPart* pg = feb.BuildFEPart(dom);
					if (pg)
					{
						FEMesh* mesh = pg->GetMesh();
						FEElementData* pd = mesh->AddElementDataField(var->cvalue(), pg, FEMeshData::DATA_TYPE::DATA_SCALAR);

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
		else if (tag == "SurfaceData")
		{
			FEBioModel& feb = GetFEBioModel();

			XMLAtt* name = tag.AttributePtr("name");
			XMLAtt* dataTypeAtt = tag.AttributePtr("data_type");
			XMLAtt* surf = tag.AttributePtr("surface");

			FEMeshData::DATA_TYPE dataType;
			if(*dataTypeAtt == "scalar") dataType = FEMeshData::DATA_TYPE::DATA_SCALAR;
			else if(*dataTypeAtt == "vector") dataType = FEMeshData::DATA_TYPE::DATA_VEC3D;
			else return false;

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
			}while (!tag.isend());

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
void FEBioFormat25::ParseBCFixed(FEStep* pstep, XMLTag &tag)
{
	// get the bc attribute
	XMLAtt& abc = tag.Attribute("bc");

	// figure out the bc value
	int bc = GetDOFCode(abc.cvalue());
	if (bc == 0) throw XMLReader::InvalidAttributeValue(tag, "bc", abc.cvalue());

	FEBioModel& febio = GetFEBioModel();

	// get the mesh
	FEModel& fem = GetFEModel();

	// get the node set
	const char* szset = tag.AttributeValue("node_set");
	FENodeSet* pg = febio.BuildFENodeSet(szset);
	if (pg == 0) FileReader()->AddLogEntry("Cannot find node_set \"%s\"", szset);

	// create the constraint
	char szname[256] = { 0 };
	if (bc < 8)
	{
		FEFixedDisplacement* pbc = new FEFixedDisplacement(&fem, pg, bc, pstep->GetID());
		sprintf(szname, "FixedDisplacement%02d", CountBCs<FEFixedDisplacement>(fem)+1);
		pbc->SetName(szname);
		pstep->AddComponent(pbc);
	}
	else if (bc < 64)
	{
		bc = bc >> 3;
		FEFixedRotation* pbc = new FEFixedRotation(&fem, pg, bc, pstep->GetID());
		sprintf(szname, "FixedRotation%02d", CountBCs<FEFixedRotation>(fem)+1);
		pbc->SetName(szname);
		pstep->AddComponent(pbc);
	}
	else if (bc == 64)
	{
		FEFixedTemperature* pbc = new FEFixedTemperature(&fem, pg, 1, pstep->GetID());
		sprintf(szname, "FixedTemperature%02d", CountBCs<FEFixedTemperature>(fem)+1);
		pbc->SetName(szname);
		pstep->AddComponent(pbc);
	}
	else if (bc == 128)
	{
		FEFixedFluidPressure* pbc = new FEFixedFluidPressure(&fem, pg, 1, pstep->GetID());
		sprintf(szname, "FixedFluidPressure%02d", CountBCs<FEFixedFluidPressure>(fem)+1);
		pbc->SetName(szname);
		pstep->AddComponent(pbc);
	}
	else if ((bc < 2048) && (bc >= 256))
	{
		bc = bc >> 8;
		FEFixedFluidVelocity* pbc = new FEFixedFluidVelocity(&fem, pg, bc, pstep->GetID());
		sprintf(szname, "FixedFluidVelocity%02d", CountBCs<FEFixedFluidVelocity>(fem)+1);
		pbc->SetName(szname);
		pstep->AddComponent(pbc);
	}
	else if (bc == 2048)
	{
		FEFixedFluidDilatation* pbc = new FEFixedFluidDilatation(&fem, pg, 1, pstep->GetID());
		sprintf(szname, "FixedFluidDilatation%02d", CountBCs<FEFixedFluidDilatation>(fem)+1);
		pbc->SetName(szname);
		pstep->AddComponent(pbc);
	}
	else if (bc < (1 << 15))
	{
		bc = bc >> 12;
		FEFixedShellDisplacement* pbc = new FEFixedShellDisplacement(&fem, pg, bc, pstep->GetID());
		sprintf(szname, "FixedShellDisplacement%02d", CountBCs<FEFixedShellDisplacement>(fem)+1);
		pbc->SetName(szname);
		pstep->AddComponent(pbc);
	}
	else
	{
		bc = bc >> 15;
		if (bc < 256)
		{
			FEFixedConcentration* pbc = new FEFixedConcentration(&fem, pg, bc, pstep->GetID());
			sprintf(szname, "FixedConcentration%02d", CountBCs<FEFixedConcentration>(fem)+1);
			pbc->SetName(szname);
			pstep->AddComponent(pbc);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseBCPrescribed(FEStep* pstep, XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

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
	else throw XMLReader::InvalidAttributeValue(tag, "bc", abc.cvalue());

	XMLAtt& set = tag.Attribute("node_set");
	FENodeSet* pg = febio.BuildFENodeSet(set.cvalue());
	if (pg == 0) FileReader()->AddLogEntry("Cannot find node_set \"%s\"", set.cvalue());

	// make a new boundary condition
	FEPrescribedDOF* pbc = 0;
	switch (bc)
	{
	case 0:
	case 1:
	case 2: pbc = new FEPrescribedDisplacement (&fem, pg, bc, 1, pstep->GetID()); break;
	case 3: pbc = new FEPrescribedTemperature  (&fem, pg, bc, pstep->GetID()); break;
	case 4: pbc = new FEPrescribedFluidPressure(&fem, pg, 1, pstep->GetID()); break;
	case 5:
	case 6:
	case 7:
		bc = bc - 5;
		pbc = new FEPrescribedFluidVelocity(&fem, pg, bc, 1, pstep->GetID());
		break;
	case 8:
		pbc = new FEPrescribedFluidDilatation(&fem, pg, 1, pstep->GetID());
		break;
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		bc = bc - 9;
		pbc = new FEPrescribedConcentration(&fem, pg, bc, 1.0, pstep->GetID());
		break;
	case 15:
	case 16:
	case 17:
		bc = bc - 15;
		pbc = new FEPrescribedRotation(&fem, pg, bc, 1.0, pstep->GetID());
		break;
	}
	if (pbc == 0) throw XMLReader::InvalidAttributeValue(tag, "bc", abc.cvalue());

	// get the optional name
	string name;
	const char* szname = tag.AttributeValue("name", true);
	if (szname == 0) name = pg->GetName(); else name = szname;
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
			double scale;
			tag.value(scale);
			pbc->SetScaleFactor(scale);

			int lc = tag.AttributeValue<int>("lc", -1);
			if (lc != -1) febio.AddParamCurve(pbc->GetLoadCurve(), lc-1);
		}
		++tag;
	}
	while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseBCRigid(FEStep* pstep, XMLTag& tag)
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

	// read rigid material ID
	int nrb = tag.AttributeValue<int>("rb", -1);
	if (nrb == -1) throw XMLReader::MissingAttribute(tag, "rb");

	// read node set
	const char* szset = tag.AttributeValue("node_set");
	FEBioModel& febio = GetFEBioModel();
	FENodeSet* pg = febio.BuildFENodeSet(szset);

	GMaterial* pmat = 0;
	if ((nrb > 0) && (nrb <= febio.Materials())) pmat = febio.GetMaterial(nrb - 1);
	else FileReader()->AddLogEntry("Invalid material in rigid contact.");

	// create the interface
	FERigidInterface* pi = new FERigidInterface(&fem, pmat, pg, pstep->GetID());
	pi->SetName(name.c_str());
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseBCRigidBody(FEStep* pstep, XMLTag& tag)
{
	// get the material ID
	int nid = tag.Attribute("mat").value<int>() - 1;

	// get the rigid material
	FEBioModel& febio = GetFEBioModel();
	FEModel* fem = &febio.GetFEModel();
	GMaterial* pgm = 0;
	if (nid >= 0) pgm = febio.GetMaterial(nid);
	int matid = (pgm ? pgm->GetID() : -1);
	assert(dynamic_cast<FERigidMaterial*>(pgm->GetMaterialProperties()));

	// get the (optional) name 
	bool hasName = false;
	char szname[256] = { 0 };
	const char* sz = tag.AttributeValue("name", true);
	if (sz) { strcpy(szname, sz); hasName = true; }

	FERigidFixed* pc = 0; // fixed constraint
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
			if (pc == 0) pc = new FERigidFixed(fem, pstep->GetID());
			pc->SetDOF(nbc, true);
		}
		else if (tag == "prescribed")
		{
			int lc = tag.AttributeValue<int>("lc", 0);
			tag.value(v);
			FERigidDisplacement* pd = new FERigidDisplacement(nbc, matid, v, pstep->GetID());

			static int n = 1;
			if (hasName == false) sprintf(szname, "RigidDisplacement%02d", n++);
			pd->SetName(szname);
			pstep->AddRC(pd);
			febio.AddParamCurve(pd->GetLoadCurve(), lc - 1);
		}
		else if (tag == "force")
		{
			int lc = tag.AttributeValue<int>("lc", 0);
			tag.value(v);
			FERigidForce* pf = new FERigidForce(nbc, matid, v, pstep->GetID());

			static int n = 1;
			if (hasName == false) sprintf(szname, "RigidForce%02d", n++);
			pf->SetName(szname);
			pstep->AddRC(pf);
			febio.AddParamCurve(pf->GetLoadCurve(), lc - 1);
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
void FEBioFormat25::ParseNodeLoad(FEStep* pstep, XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

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

	// create the node set
	FENodeSet* pg = febio.BuildFENodeSet(aset.cvalue());
	if (pg == 0) throw XMLReader::InvalidAttributeValue(tag, aset);
	char szname[256];
	sprintf(szname, "ForceNodeset%02d", CountLoads<FENodalLoad>(fem)+1);
	pg->SetName(szname);

	// create the nodal load
	FENodalLoad* pbc = new FENodalLoad(&fem, pg, bc, 1, pstep->GetID());
	sprintf(szname, "ForceLoad%02d", CountLoads<FENodalLoad>(fem)+1);
	pbc->SetName(szname);
	pstep->AddComponent(pbc);

	// assign nodes to node sets
	++tag;
	do
	{
		if (tag == "scale")
		{
			int lc = tag.Attribute("lc").value<int>() - 1;
			if (lc == -1) throw XMLReader::InvalidAttributeValue(tag, "lc", 0);
			febio.AddParamCurve(pbc->GetLoadCurve(), lc);

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
void FEBioFormat25::ParseSurfaceLoad(FEStep* pstep, XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();

	std::string comment = tag.comment();

	// find the surface
	XMLAtt& surf = tag.Attribute("surface");
	FESurface* psurf = febio.BuildFESurface(surf.cvalue());
	if (psurf == 0) throw XMLReader::InvalidAttributeValue(tag, surf);

	FESurfaceLoad* psl = 0;
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
    else if (att == "fluid velocity"                ) psl = ParseLoadFluidVelocity               (tag);
    else if (att == "fluid normal velocity"         ) psl = ParseLoadFluidNormalVelocity         (tag);
    else if (att == "fluid rotational velocity"     ) psl = ParseLoadFluidRotationalVelocity     (tag);
    else if (att == "fluid resistance"              ) psl = ParseLoadFluidFlowResistance         (tag);
    else if (att == "fluid RCR"                     ) psl = ParseLoadFluidFlowRCR                (tag);
    else if (att == "fluid backflow stabilization"  ) psl = ParseLoadFluidBackFlowStabilization  (tag);
    else if (att == "fluid tangential stabilization") psl = ParseLoadFluidTangentialStabilization(tag);
    else if (att == "fluid-FSI traction" ) psl = ParseLoadFSITraction       (tag);
	else ParseUnknownAttribute(tag, "type");

	if (psl)
	{
		// assign the surface
		psl->SetItemList(psurf);

		// set the comment
		psl->SetInfo(comment);

		// add to the step
		pstep->AddComponent(psl);
	}
}

//-----------------------------------------------------------------------------
FESurfaceLoad* FEBioFormat25::ParseLoadPressure(XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	// create a new surface load
	FEPressureLoad* pbc = new FEPressureLoad(&fem);

	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "PressureLoad%d", CountLoads<FEPressureLoad>(fem) + 1);
	pbc->SetName(szname);

	// read the parameters
	++tag;
	do
	{
		if (tag == "linear")
		{
			bool n; tag.value(n);
			pbc->SetBoolValue(FEPressureLoad::NTYPE, n);
		}
		else if (tag == "pressure")
		{
			double s; tag.value(s);
			pbc->SetLoad(s);

			int lc = tag.Attribute("lc").value<int>() - 1;
			febio.AddParamCurve(pbc->GetLoadCurve(), lc);
		}
		++tag;
	}
	while (!tag.isend());

	return pbc;
}

//-----------------------------------------------------------------------------
FESurfaceLoad* FEBioFormat25::ParseLoadTraction(XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	// create a new surface load
	FESurfaceTraction* pbc = new FESurfaceTraction(&fem);

	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "TractionLoad%d", CountLoads<FESurfaceTraction>(fem) + 1);
	pbc->SetName(szname);

	// read the parameters
	++tag;
	do
	{
		if (tag == "scale")
		{
			int lc = tag.Attribute("lc").value<int>() - 1;
			febio.AddParamCurve(pbc->GetLoadCurve(), lc);

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
FESurfaceLoad* FEBioFormat25::ParseLoadFluidTraction(XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	// create a new surface load
	FEFluidTraction* pbc = new FEFluidTraction(&fem);

	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "FluidTractionLoad%d", CountLoads<FEFluidTraction>(fem));
	pbc->SetName(szname);

	// read the parameters
	++tag;
	do
	{
		if (tag == "scale")
		{
			int lc = tag.Attribute("lc").value<int>() - 1;
			febio.AddParamCurve(pbc->GetLoadCurve(), lc);

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
FESurfaceLoad* FEBioFormat25::ParseLoadFluidVelocity(XMLTag& tag)
{
    FEBioModel& febio = GetFEBioModel();
    FEModel& fem = GetFEModel();
    FEFluidVelocity* psl = new FEFluidVelocity(&fem);
    char szname[128] = { 0 };
    sprintf(szname, "FluidVelocity%02d", CountLoads<FEFluidVelocity>(fem)+1);
    psl->SetName(szname);
    
    ++tag;
    do
    {
        if (tag == "scale")
        {
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(psl->GetLoadCurve(), lc);
        }
        else if (tag == "velocity") {
            vec3d t; tag.value(t);
            psl->SetLoad(t);
        }
        else ParseUnknownTag(tag);
        ++tag;
    }
    while (!tag.isend());
    
    return psl;
}

//-----------------------------------------------------------------------------
FESurfaceLoad* FEBioFormat25::ParseLoadFluidNormalVelocity(XMLTag& tag)
{
    FEBioModel& febio = GetFEBioModel();
    FEModel& fem = GetFEModel();
    FEFluidNormalVelocity* psl = new FEFluidNormalVelocity(&fem);
    char szname[128] = { 0 };
    sprintf(szname, "FluidNormalVelocity%02d", CountLoads<FEFluidNormalVelocity>(fem)+1);
    psl->SetName(szname);
    
    ++tag;
    do
    {
        if (tag == "velocity")
        {
            double v; tag.value(v);
            psl->SetLoad(v);
            
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(psl->GetLoadCurve(), lc);
        }
        else if (tag == "prescribe_nodal_velocities") {
            bool b; tag.value(b);
            psl->SetBP(b);
        }
        else if (tag == "parabolic") {
            bool b; tag.value(b);
            psl->SetBParab(b);
        }
        else ParseUnknownTag(tag);
        ++tag;
    } while (!tag.isend());
    
    return psl;
}

//-----------------------------------------------------------------------------
FESurfaceLoad* FEBioFormat25::ParseLoadFluidRotationalVelocity(XMLTag& tag)
{
    FEBioModel& febio = GetFEBioModel();
    FEModel& fem = GetFEModel();
    FEFluidRotationalVelocity* psl = new FEFluidRotationalVelocity(&fem);
    char szname[128] = { 0 };
    sprintf(szname, "FluidRotationalVelocity%02d", CountLoads<FEFluidRotationalVelocity>(fem)+1);
    psl->SetName(szname);
    
    ++tag;
    do
    {
        if (tag == "angular_speed")
        {
            double as; tag.value(as);
            psl->SetLoad(as);
            
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(psl->GetLoadCurve(), lc);
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
FESurfaceLoad* FEBioFormat25::ParseLoadFluidFlowResistance(XMLTag& tag)
{
    FEBioModel& febio = GetFEBioModel();
    FEModel& fem = GetFEModel();
    FEFluidFlowResistance* psl = new FEFluidFlowResistance(&fem);
    char szname[128] = { 0 };
    sprintf(szname, "FluidFlowResistance%02d", CountLoads<FEFluidFlowResistance>(fem)+1);
    psl->SetName(szname);
    
    ++tag;
    do
    {
        if (tag == "R")
        {
            double R; tag.value(R);
            psl->SetLoad(R);
            
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(psl->GetLoadCurve(), lc);
        }
        else if (tag == "pressure_offset")
        {
            double po; tag.value(po);
            psl->SetPO(po);
            
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(psl->GetPOLoadCurve(), lc);
        }
        else ParseUnknownTag(tag);
        ++tag;
    } while (!tag.isend());
    
    return psl;
}

//-----------------------------------------------------------------------------
FESurfaceLoad* FEBioFormat25::ParseLoadFluidFlowRCR(XMLTag& tag)
{
    FEBioModel& febio = GetFEBioModel();
    FEModel& fem = GetFEModel();
    FEFluidFlowRCR* psl = new FEFluidFlowRCR(&fem);
    char szname[128] = { 0 };
    sprintf(szname, "FluidFlowRCR%02d", CountLoads<FEFluidFlowRCR>(fem)+1);
    psl->SetName(szname);
    
    ++tag;
    do
    {
        if (tag == "R")
        {
            double R; tag.value(R);
            psl->SetLoad(R);
            
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(psl->GetLoadCurve(), lc);
        }
        else if (tag == "Rd")
        {
            double rd; tag.value(rd);
            psl->SetRD(rd);
            
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(psl->GetRDLoadCurve(), lc);
        }
        else if (tag == "capacitance")
        {
            double  co; tag.value(co);
            psl->SetCO(co);
            
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(psl->GetCOLoadCurve(), lc);
        }
        else if (tag == "pressure_offset")
        {
            double po; tag.value(po);
            psl->SetPO(po);
            
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(psl->GetPOLoadCurve(), lc);
        }
        else if (tag == "initial_pressure")
        {
            double ip; tag.value(ip);
            psl->SetIP(ip);
            
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(psl->GetIPLoadCurve(), lc);
        }
        else if (tag == "Bernoulli")
        {
            bool be; tag.value(be);
            psl->SetBE(be);
        }
        else ParseUnknownTag(tag);
        ++tag;
    } while (!tag.isend());
    
    return psl;
}

//-----------------------------------------------------------------------------
FESurfaceLoad* FEBioFormat25::ParseLoadFluidBackFlowStabilization(XMLTag& tag)
{
    FEBioModel& febio = GetFEBioModel();
    FEModel& fem = GetFEModel();
    FEFluidBackflowStabilization* psl = new FEFluidBackflowStabilization(&fem);
    char szname[128] = { 0 };
    sprintf(szname, "FluidBackflowStabilization%02d", CountLoads<FEFluidBackflowStabilization>(fem)+1);
    psl->SetName(szname);
    
    ++tag;
    do
    {
        if (tag == "beta")
        {
            double b; tag.value(b);
            psl->SetLoad(b);
            
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(psl->GetLoadCurve(), lc);
        }
        else ParseUnknownTag(tag);
        ++tag;
    } while (!tag.isend());
    
    return psl;
}

//-----------------------------------------------------------------------------
FESurfaceLoad* FEBioFormat25::ParseLoadFluidTangentialStabilization(XMLTag& tag)
{
    FEBioModel& febio = GetFEBioModel();
    FEModel& fem = GetFEModel();
    FEFluidTangentialStabilization* psl = new FEFluidTangentialStabilization(&fem);
    char szname[128] = { 0 };
    sprintf(szname, "FluidTangentialStabilization%02d", CountLoads<FEFluidTangentialStabilization>(fem)+1);
    psl->SetName(szname);
    
    ++tag;
    do
    {
        if (tag == "beta")
        {
            double b; tag.value(b);
            psl->SetLoad(b);
            
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(psl->GetLoadCurve(), lc);
        }
        else ParseUnknownTag(tag);
        ++tag;
    } while (!tag.isend());
    
    return psl;
}

//-----------------------------------------------------------------------------
FESurfaceLoad* FEBioFormat25::ParseLoadFSITraction(XMLTag& tag)
{
    FEBioModel& febio = GetFEBioModel();
    FEModel& fem = GetFEModel();
    FEFSITraction* psl = new FEFSITraction(&fem);
    char szname[128] = { 0 };
    sprintf(szname, "FSInterfaceTraction%02d", CountLoads<FEFSITraction>(fem)+1);
    psl->SetName(szname);
    
    return psl;
}

//-----------------------------------------------------------------------------
FESurfaceLoad* FEBioFormat25::ParseLoadFluidFlux(XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	// create a new surface load
	FEFluidFlux* pbc = new FEFluidFlux(&fem);

	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "FluidFlux%d", CountLoads<FEFluidFlux>(fem));
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
			febio.AddParamCurve(pbc->GetLoadCurve(), lc);

			double s; tag.value(s);
			pbc->SetLoad(s);
		}
		++tag;
	}
	while (!tag.isend());

	return pbc;
}

//-----------------------------------------------------------------------------
FESurfaceLoad* FEBioFormat25::ParseLoadSoluteFlux(XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	// create a new surface load
	FESoluteFlux* pbc = new FESoluteFlux(&fem);

	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "SoluteFlux%d", CountLoads<FESoluteFlux>(fem));
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
				febio.AddParamCurve(pbc->GetLoadCurve(), lc);
			}
			else pbc->GetLoadCurve()->Clear();
			double s; tag.value(s);
			pbc->SetLoad(s);
		}
		++tag;
	}
	while (!tag.isend());

	return pbc;
}

//-----------------------------------------------------------------------------
FESurfaceLoad* FEBioFormat25::ParseConcentrationFlux(XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	// create a new surface load
	FEConcentrationFlux* pcf = new FEConcentrationFlux(&fem);

	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "ConcentrationFlux%d", CountLoads<FEConcentrationFlux>(fem) +1);
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
				febio.AddParamCurve(pcf->GetLoadCurve(), lc);
			}
			else pcf->GetLoadCurve()->Clear();
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
FESurfaceLoad* FEBioFormat25::ParseLoadNormalTraction(XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	// create a new surface load
	FEBPNormalTraction* pbc = new FEBPNormalTraction(&fem);

	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "NormalTraction%d", CountLoads<FEBPNormalTraction>(fem));
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
		else if (tag == "effective")
		{
			// I initially called the "mixture" flag the "effective" flag
			// but I had the values swapped.
			// Whence this little hack
			bool b; tag.value(b);
			pbc->SetMixtureFlag(!b);
		}
		else if (tag == "traction")
		{
			int lc = tag.Attribute("lc").value<int>() - 1;
			febio.AddParamCurve(pbc->GetLoadCurve(), lc);
			double s; tag.value(s);
			pbc->SetLoad(s);
		}
		++tag;
	}
	while (!tag.isend());

	return pbc;
}

//-----------------------------------------------------------------------------
FESurfaceLoad* FEBioFormat25::ParseLoadMatchingOsmoticCoefficient(XMLTag& tag)
{
    FEBioModel& febio = GetFEBioModel();
    FEModel& fem = GetFEModel();

    // create a new surface load
    FEMatchingOsmoticCoefficient* pbc = new FEMatchingOsmoticCoefficient(&fem);

    // set the name
    char szname[256] = { 0 };
    sprintf(szname, "MatchingOsmCoef%d", CountLoads<FEMatchingOsmoticCoefficient>(fem));
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
            febio.AddParamCurve(pbc->GetLoadCurve(), lc);
            double s; tag.value(s);
            pbc->SetLoadP(s);
        }
        else if (tag == "ambient_osmolarity")
        {
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(pbc->GetLoadCurveC(), lc);
            double s; tag.value(s);
            pbc->SetLoadC(s);
        }
        ++tag;
    }
    while (!tag.isend());

    return pbc;
}

//-----------------------------------------------------------------------------
FESurfaceLoad* FEBioFormat25::ParseLoadHeatFlux(XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	// create a new surface load
	FEHeatFlux* pbc = new FEHeatFlux(&fem);

	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "HeatFlux%d", CountLoads<FEHeatFlux>(fem));
	pbc->SetName(szname);

	// read the parameters
	++tag;
	do
	{
		if (tag == "flux")
		{
			int lc = tag.Attribute("lc").value<int>() - 1;
			febio.AddParamCurve(pbc->GetLoadCurve(), lc);
			double s; tag.value(s);
			pbc->SetLoad(s);
		}
		++tag;
	}
	while (!tag.isend());

	return pbc;
}

//-----------------------------------------------------------------------------
FESurfaceLoad* FEBioFormat25::ParseLoadConvectiveHeatFlux(XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	// create a new surface load
	FEConvectiveHeatFlux* pbc = new FEConvectiveHeatFlux(&fem);

	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "ConvectiveHeatFlux%d", CountLoads<FEConvectiveHeatFlux>(fem));
	pbc->SetName(szname);

	// read the parameters
	++tag;
	do
	{
		if (tag == "Ta")
		{
			int lc = tag.Attribute("lc").value<int>() - 1;
			febio.AddParamCurve(pbc->GetLoadCurve(), lc);
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
void FEBioFormat25::ParseBodyLoad(FEStep* pstep, XMLTag& tag)
{
	XMLAtt& att = tag.Attribute("type");
	if      (att == "const"      ) ParseBodyForce (pstep, tag);
	else if (att == "non-const"  ) ParseNonConstBodyForce(pstep, tag);
	else if (att == "heat_source") ParseHeatSource(pstep, tag);
	else ParseUnknownAttribute(tag, "type");
}

//-----------------------------------------------------------------------------
bool FEBioFormat25::ParseInitialSection(XMLTag& tag)
{
	// make sure the section is not empty
	if (tag.isleaf()) return true;

	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

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
			assert(dynamic_cast<FERigidMaterial*>(pgm->GetMaterialProperties()));

			++tag;
			do
			{
				if (tag == "initial_velocity")
				{
					FERigidVelocity* pv = new FERigidVelocity(&fem, m_pBCStep->GetID());
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
					FERigidAngularVelocity* pv = new FERigidAngularVelocity(&fem, m_pBCStep->GetID());
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
			FENodeSet* pg = febio.BuildFENodeSet(szset);
			if (pg == 0) throw XMLReader::MissingTag(tag, "node_set");

			++tag;
			do
			{
				if (tag == "value") tag.value(val);
				else ParseUnknownTag(tag);
				++tag;
			}
			while (!tag.isend());

			// create a new initial velocity BC
			FEInitialCondition* pic = 0;
			char szname[64] = { 0 };
			switch (bc)
			{
			case 3:
				pic = new FEInitTemperature(&fem, pg, val, m_pBCStep->GetID());
				sprintf(szname, "InitialTemperature%02d", CountICs<FEInitTemperature>(fem)+1);
				break;
			case 4:
				pic = new FEInitFluidPressure(&fem, pg, val, m_pBCStep->GetID());
				sprintf(szname, "InitialFluidPressure%02d", CountICs<FEInitFluidPressure>(fem)+1);
				break;
			case 5:
				pic = new FENodalVelocities(&fem, pg, vec3d(val, 0, 0), m_pBCStep->GetID());
				sprintf(szname, "InitialVelocity%02d", CountICs<FENodalVelocities>(fem)+1);
				break;
			case 6:
				pic = new FENodalVelocities(&fem, pg, vec3d(0, val, 0), m_pBCStep->GetID());
				sprintf(szname, "InitialVelocity%02d", CountICs<FENodalVelocities>(fem)+1);
				break;
			case 7:
				pic = new FENodalVelocities(&fem, pg, vec3d(0, 0, val), m_pBCStep->GetID());
				sprintf(szname, "InitialVelocity%02d", CountICs<FENodalVelocities>(fem)+1);
				break;
            case 15:
                pic = new FEInitShellFluidPressure(&fem, pg, val, m_pBCStep->GetID());
				sprintf(szname, "InitialShellFluidPressure%02d", CountICs<FEInitShellFluidPressure>(fem)+1);
                break;
			case 22:
				pic = new FENodalShellVelocities(&fem, pg, vec3d(val, 0, 0), m_pBCStep->GetID());
				sprintf(szname, "InitShellVelocity%02d", CountICs<FENodalShellVelocities>(fem) +1);
				break;
			case 23:
				pic = new FENodalShellVelocities(&fem, pg, vec3d(0, val, 0), m_pBCStep->GetID());
				sprintf(szname, "InitShellVelocity%02d", CountICs<FENodalShellVelocities>(fem) +1);
				break;
			case 24:
				pic = new FENodalShellVelocities(&fem, pg, vec3d(0, 0, val), m_pBCStep->GetID());
				sprintf(szname, "InitShellVelocity%02d", CountICs<FENodalShellVelocities>(fem) +1);
				break;
            case 25:
                pic = new FEInitFluidDilatation(&fem, pg, val, m_pBCStep->GetID());
                sprintf(szname, "InitialFluidDilatation%02d", CountICs<FEInitFluidDilatation>(fem)+1);
                break;
			default:
				if ((bc >= 9) && (bc <= 14))
				{
					int nsol = bc - 9;
					pic = new FEInitConcentration(&fem, pg, nsol, val, m_pBCStep->GetID());
					sprintf(szname, "InitConcentration%02d", CountICs<FEInitConcentration>(fem)+1);
				}
                else if ((bc >= 16) && (bc <= 21))
                {
                    int nsol = bc - 16;
                    pic = new FEInitShellConcentration(&fem, pg, nsol, val, m_pBCStep->GetID());
					sprintf(szname, "InitShellConcentration%02d", CountICs<FEInitShellConcentration>(fem)+1);
                }
			}

			if (pic)
			{
				pic->SetName(szname);
				m_pBCStep->AddComponent(pic);
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
void FEBioFormat25::ParseContact(FEStep *pstep, XMLTag &tag)
{
	FEBioModel& febio = GetFEBioModel();

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

		// standard contact interfaces
		FEPairedInterface* pci = 0;
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
			ReadParameters(*pci, tag);

			// assign surfaces
			FEBioModel::Part* part = surfPair->GetPart();
			assert(part);
			if (part)
			{
				string name1 = part->GetSurface(surfPair->masterID()).name();
				string name2 = part->GetSurface(surfPair->slaveID()).name();
				FESurface* master = febio.BuildFESurface(name1.c_str());
				FESurface* slave  = febio.BuildFESurface(name2.c_str());

				pci->SetMaster(master);
				pci->SetSlave(slave);
			}

			// add to the analysis step
			pstep->AddComponent(pci);
		}
	}
}

//-----------------------------------------------------------------------------
FEPairedInterface* FEBioFormat25::ParseContactSliding(FEStep* pstep, XMLTag& tag)
{
	FEModel& fem = GetFEModel();

	// create new sliding interface
	FESlidingWithGapsInterface* pi = new FESlidingWithGapsInterface(&fem, pstep->GetID());

	// get the (optional) contact name
	char szbuf[256];
	const char* szname = tag.AttributeValue("name", true);
	if (szname) sprintf(szbuf, "%s", szname);
	else sprintf(szbuf, "SlidingInterface%02d", CountInterfaces<FESlidingWithGapsInterface>(fem)+1);
	pi->SetName(szbuf);

	return pi;
}

//-----------------------------------------------------------------------------
FEPairedInterface* FEBioFormat25::ParseContactF2FSliding(FEStep* pstep, XMLTag& tag)
{
	FEModel& fem = GetFEModel();

	// create new sliding interface
	FEFacetOnFacetInterface* pi = new FEFacetOnFacetInterface(&fem, pstep->GetID());

	// get the (optional) contact name
	char szbuf[256];
	const char* szname = tag.AttributeValue("name", true);
	if (szname) sprintf(szbuf, "%s", szname);
	else sprintf(szbuf, "SlidingContact%02d", CountInterfaces<FEFacetOnFacetInterface>(fem)+1);
	pi->SetName(szbuf);

	return pi;
}

//-----------------------------------------------------------------------------
FEPairedInterface* FEBioFormat25::ParseContactBiphasic(FEStep* pstep, XMLTag& tag)
{
	FEModel& fem = GetFEModel();

	// create new contact interface
	FEPoroContact* pi = new FEPoroContact(&fem, pstep->GetID());

	// read the name
	char szname[256];
	sprintf(szname, "BiphasicContact%02d", CountInterfaces<FEPoroContact>(fem)+1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	return pi;
}

//-----------------------------------------------------------------------------
FEPairedInterface* FEBioFormat25::ParseContactSolute(FEStep* pstep, XMLTag& tag)
{
	FEModel& fem = GetFEModel();

	// create new interface
	FEPoroSoluteContact* pi = new FEPoroSoluteContact(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "BiphasicSoluteContact%02d", CountInterfaces<FEPoroSoluteContact>(fem)+1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	return pi;
}

//-----------------------------------------------------------------------------
FEPairedInterface* FEBioFormat25::ParseContactMultiphasic(FEStep* pstep, XMLTag& tag)
{
	FEModel& fem = GetFEModel();

	// create new interface
	FEMultiphasicContact* pi = new FEMultiphasicContact(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "MultiphasicContact%02d", CountInterfaces<FEMultiphasicContact>(fem)+1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	return pi;
}

//-----------------------------------------------------------------------------
FEPairedInterface* FEBioFormat25::ParseContactTied(FEStep *pstep, XMLTag &tag)
{
	FEModel& fem = GetFEModel();

	// create new interface
	FETiedInterface* pi = new FETiedInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "TiedInterface%02d", CountInterfaces<FETiedInterface>(fem)+1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	return pi;
}

//-----------------------------------------------------------------------------
FEPairedInterface* FEBioFormat25::ParseContactF2FTied(FEStep *pstep, XMLTag &tag)
{
	FEModel& fem = GetFEModel();

	// create new interface
	FEF2FTiedInterface* pi = new FEF2FTiedInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "F2FTiedInterface%02d", CountInterfaces<FEF2FTiedInterface>(fem)+1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	return pi;
}

//-----------------------------------------------------------------------------
FEPairedInterface* FEBioFormat25::ParseContactTiedElastic(FEStep* pstep, XMLTag& tag)
{
	FEModel& fem = GetFEModel();

	// create new interface
	FETiedElasticInterface* pi = new FETiedElasticInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "TiedElasticInterface%02d", CountInterfaces<FETiedElasticInterface>(fem)+1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	return pi;
}

//-----------------------------------------------------------------------------
FEPairedInterface* FEBioFormat25::ParseContactSticky(FEStep *pstep, XMLTag &tag)
{
	FEModel& fem = GetFEModel();

	// create new interface
	FEStickyInterface* pi = new FEStickyInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "StickyInterface%02d", CountInterfaces<FEStickyInterface>(fem)+1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	return pi;
}

//-----------------------------------------------------------------------------
FEPairedInterface* FEBioFormat25::ParseContactPeriodic(FEStep *pstep, XMLTag &tag)
{
	FEModel& fem = GetFEModel();

	// create new interface
	FEPeriodicBoundary* pi = new FEPeriodicBoundary(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "PeriodicBoundary%02d", CountInterfaces<FEPeriodicBoundary>(fem)+1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	return pi;
}

//-----------------------------------------------------------------------------
FEPairedInterface* FEBioFormat25::ParseContactTC(FEStep *pstep, XMLTag &tag)
{
	FEModel& fem = GetFEModel();

	// create new interface
	FETensionCompressionInterface* pi = new FETensionCompressionInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "TCInterface%02d", CountInterfaces<FETensionCompressionInterface>(fem)+1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	return pi;
}

//-----------------------------------------------------------------------------
FEPairedInterface* FEBioFormat25::ParseContactTiedPoro(FEStep *pstep, XMLTag &tag)
{
	FEModel& fem = GetFEModel();

	// create new interface
	FETiedBiphasicInterface* pi = new FETiedBiphasicInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "TiedBiphasicInterface%02d", CountInterfaces<FETiedBiphasicInterface>(fem)+1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	return pi;
}

//-----------------------------------------------------------------------------
FEPairedInterface* FEBioFormat25::ParseContactTiedMultiphasic(FEStep *pstep, XMLTag &tag)
{
	FEModel& fem = GetFEModel();

	// create new interface
	FETiedMultiphasicInterface* pi = new FETiedMultiphasicInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "TiedMultiphasicInterface%02d", CountInterfaces<FETiedMultiphasicInterface>(fem)+1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	return pi;
}

//-----------------------------------------------------------------------------
FEPairedInterface* FEBioFormat25::ParseContactGapHeatFlux(FEStep* pstep, XMLTag& tag)
{
	FEModel& fem = GetFEModel();

	// create new interface
	FEGapHeatFluxInterface* pi = new FEGapHeatFluxInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "GapHeatFlux%02d", CountInterfaces<FEGapHeatFluxInterface>(fem)+1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	return pi;
}

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseRigidWall(FEStep* pstep, XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	// create a new interface
	FERigidWallInterface* pci = new FERigidWallInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "RigidWall%02d", CountInterfaces<FERigidWallInterface>(fem)+1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pci->SetName(szname);

	// assign surface
	const char* szsurf = tag.AttributeValue("surface", true);
	if (szsurf)
	{
		FESurface* surface = febio.BuildFESurface(szsurf);
		if (surface) pci->SetItemList(surface);
	}

	// read parameters
	++tag;
	do
	{
		// read parameters
		if      (tag == "laugon"   ) { int n; tag.value(n); pci->SetBoolValue(FERigidWallInterface::LAUGON, (n == 0 ? false : true)); }
		else if (tag == "tolerance") { double f; tag.value(f); pci->SetFloatValue(FERigidWallInterface::ALTOL, f); }
		else if (tag == "penalty"  ) { double f; tag.value(f); pci->SetFloatValue(FERigidWallInterface::PENALTY, f); }
		if      (tag == "plane"    )
		{
			double n[4];
			tag.value(n, 4);
			pci->SetFloatValue(FERigidWallInterface::PA, n[0]);
			pci->SetFloatValue(FERigidWallInterface::PB, n[1]);
			pci->SetFloatValue(FERigidWallInterface::PC, n[2]);
			pci->SetFloatValue(FERigidWallInterface::PD, n[3]);

			const char* szlc = tag.AttributeValue("lc", true);
			if (szlc) febio.AddParamCurve(&pci->GetParam(FERigidWallInterface::OFFSET), atoi(szlc) - 1);
		}
		++tag;
	}
	while (!tag.isend());

	// add interface to step
	pstep->AddComponent(pci);
}

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseContactJoint(FEStep *pstep, XMLTag &tag)
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
void FEBioFormat25::ParseConnector(FEStep *pstep, XMLTag &tag, const int rc)
{
	FEModel& fem = GetFEModel();

	FERigidConnector* pi;
	char szname[256];

	switch (rc) {
	case 0:
		pi = new FERigidSphericalJoint(&fem, pstep->GetID());
		sprintf(szname, "RigidSphericalJoint%02d", CountConnectors<FERigidSphericalJoint>(fem)+1);
		break;
	case 1:
		pi = new FERigidRevoluteJoint(&fem, pstep->GetID());
		sprintf(szname, "RigidrevoluteJoint%02d", CountConnectors<FERigidRevoluteJoint>(fem)+1);
		break;
	case 2:
		pi = new FERigidPrismaticJoint(&fem, pstep->GetID());
		sprintf(szname, "RigidPrismaticJoint%02d", CountConnectors<FERigidPrismaticJoint>(fem)+1);
		break;
	case 3:
		pi = new FERigidCylindricalJoint(&fem, pstep->GetID());
		sprintf(szname, "RigidCylindricalJoint%02d", CountConnectors<FERigidCylindricalJoint>(fem)+1);
		break;
	case 4:
		pi = new FERigidPlanarJoint(&fem, pstep->GetID());
		sprintf(szname, "RigidPlanarJoint%02d", CountConnectors<FERigidPlanarJoint>(fem)+1);
		break;
    case 5:
        pi = new FERigidLock(&fem, pstep->GetID());
        sprintf(szname, "RigidLock%02d", CountConnectors<FERigidLock>(fem)+1);
        break;
	case 6:
		pi = new FERigidSpring(&fem, pstep->GetID());
		sprintf(szname, "RigidSpring%02d", CountConnectors<FERigidSpring>(fem)+1);
		break;
	case 7:
		pi = new FERigidDamper(&fem, pstep->GetID());
		sprintf(szname, "RigidDamper%02d", CountConnectors<FERigidDamper>(fem)+1);
		break;
	case 8:
		pi = new FERigidAngularDamper(&fem, pstep->GetID());
		sprintf(szname, "RigidAngularDamper%02d", CountConnectors<FERigidAngularDamper>(fem)+1);
		break;
	case 9:
		pi = new FERigidContractileForce(&fem, pstep->GetID());
		sprintf(szname, "RigidContractileForce%02d", CountConnectors<FERigidContractileForce>(fem)+1);
		break;
	case 10:
		pi = new FEGenericRigidJoint(&fem, pstep->GetID());
		sprintf(szname, "GenericRigidJoint%02d", CountConnectors<FEGenericRigidJoint>(fem) + 1);
		break;
	default:
		assert(false);
		break;
	}
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);
	pstep->AddRigidConnector(pi);

	int na = -1, nb = -1;

	FEBioModel& febio = GetFEBioModel();

	++tag;
	do
	{
		if (ReadParam(*pi, tag) == false)
		{
			if (tag == "body_a") 
			{
				tag.value(na);
				if (na >= 0) pi->m_rbA = febio.GetMaterial(na - 1)->GetID();
			}
			else if (tag == "body_b") 
			{
				tag.value(nb);
				if (nb >= 0) pi->m_rbB = febio.GetMaterial(nb - 1)->GetID();
			}
			else ParseUnknownTag(tag);
		}
		++tag;
	}
	while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseLinearConstraint(FEStep* pstep, XMLTag& tag)
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

	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

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
				FELinearSpringMaterial* mat = new FELinearSpringMaterial();
				GDiscreteSpringSet* pg = new GDiscreteSpringSet();
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
				GDiscreteSpringSet* pg = new GDiscreteSpringSet();
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
				FEMaterial* mat = ParseMaterial(tag, sztype);
				FEDiscreteMaterial* dmat = dynamic_cast<FEDiscreteMaterial*>(mat);
				if (dmat)
				{
					GDiscreteSpringSet* pg = new GDiscreteSpringSet();
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

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseBodyForce(FEStep *pstep, XMLTag &tag)
{
	FEModel& fem = GetFEModel();

	FEBodyForce* pbl = new FEBodyForce(&fem, pstep->GetID());
	pstep->AddComponent(pbl);

	++tag;
	do
	{
		if (ReadParam(*pbl, tag) == false) ParseUnknownTag(tag);
		else ++tag;
	}
	while (!tag.isend());

	char szname[256] = { 0 };
	sprintf(szname, "BodyForce%02d", CountLoads<FEBodyForce>(fem));
	pbl->SetName(szname);
}

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseNonConstBodyForce(FEStep *pstep, XMLTag &tag)
{
	FEModel& fem = GetFEModel();

	FEBodyForce* pbl = new FEBodyForce(&fem, pstep->GetID());
	pstep->AddComponent(pbl);

	++tag;
	do
	{
		if (ReadParam(*pbl, tag) == false) ParseUnknownTag(tag);
		else ++tag;
	} 
	while (!tag.isend());

	char szname[256] = { 0 };
	sprintf(szname, "BodyForce%02d", CountLoads<FEBodyForce>(fem));
	pbl->SetName(szname);
}

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseHeatSource(FEStep *pstep, XMLTag &tag)
{
	FEModel& fem = GetFEModel();

	FEHeatSource* phs = new FEHeatSource(&fem, pstep->GetID());
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
			phs->GetLoadCurve()->SetID(atoi(szlc) - 1);
		}
		else throw XMLReader::InvalidTag(tag);

		++tag;
	} while (!tag.isend());

	char szname[256] = { 0 };
	sprintf(szname, "HeatSource%02d", CountLoads<FEHeatSource>(fem));
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

	FEStep* pstep = m_pBCStep;

	++tag;
	do
	{
		if (tag == "constraint")
		{
			const char* sztype = tag.AttributeValue("type");
			if      (strcmp(sztype, "volume"                 ) == 0) ParseVolumeConstraint(pstep, tag);
			else if (strcmp(sztype, "symmetry plane"         ) == 0) ParseSymmetryPlane(pstep, tag);
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
void FEBioFormat25::ParseVolumeConstraint(FEStep* pstep, XMLTag& tag)
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
void FEBioFormat25::ParseSymmetryPlane(FEStep* pstep, XMLTag& tag)
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
	FESymmetryPlane* pi = new FESymmetryPlane(&fem);
	pi->SetName(szname);
	pi->SetItemList(psurf);
	pstep->AddComponent(pi);

	// read parameters
	ReadParameters(*pi, tag);
}

//-----------------------------------------------------------------------------
void FEBioFormat25::ParseNrmlFldVlctSrf(FEStep* pstep, XMLTag& tag)
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
void FEBioFormat25::ParseFrictionlessFluidWall(FEStep* pstep, XMLTag& tag)
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
    FEFrictionlessFluidWall* pi = new FEFrictionlessFluidWall(&fem);
    pi->SetName(szname);
    pi->SetItemList(psurf);
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
FEBioModel::DiscreteSet FEBioFormat25::ParseDiscreteSet(XMLTag& tag)
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
