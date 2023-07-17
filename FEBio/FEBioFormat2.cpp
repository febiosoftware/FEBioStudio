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
#include "FEBioFormat2.h"
#include <FEMLib/FERigidConstraint.h>
#include <GeomLib/GMeshObject.h>
#include <FEMLib/FEInitialCondition.h>
#include <FEMLib/FEBodyLoad.h>
#include <FEMLib/FEModelConstraint.h>
#include <FEMLib/FEMKernel.h>
#include <FEMLib/GDiscreteObject.h>
#include <MeshLib/FEElementData.h>
#include <GeomLib/GModel.h>
#include <sstream>
#include <FEBioLink/FEBioModule.h>

using std::stringstream;

FEBioFormat2::FEBioFormat2(FEBioFileImport* fileReader, FEBioInputModel& febio) : FEBioFormat(fileReader, febio)
{
}

FEBioFormat2::~FEBioFormat2()
{
}

bool FEBioFormat2::ParseSection(XMLTag& tag)
{
	if      (tag == "Module"     ) ParseModuleSection    (tag);
	else if (tag == "Control"    ) ParseControlSection   (tag);
	else if (tag == "Material"   ) ParseMaterialSection  (tag);
	else if (tag == "Geometry"   ) if (m_skipGeom == false) ParseGeometrySection(tag); else tag.m_preader->SkipTag(tag);
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
	
	return true;
}

//-----------------------------------------------------------------------------
// Parse the Module section
bool FEBioFormat2::ParseModuleSection(XMLTag &tag)
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
		m_nAnalysis = FE_STEP_MECHANICS;
		FileReader()->AddLogEntry("unknown module type. Assuming solid module (line %d)", tag.currentLine());
	}

	const char* sztype = atype.cvalue();
	if (strcmp(sztype, "explicit-solid") == 0) sztype = "solid";

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
	return (m_nAnalysis != -1);}

//=============================================================================
//
//                                G E O M E T R Y
//
//=============================================================================

//-----------------------------------------------------------------------------
//  Parses the geometry section from the xml file (version 2.0 and up)
//
bool FEBioFormat2::ParseGeometrySection(XMLTag& tag)
{
	// make sure the section is not empty
	if (tag.isleaf()) return true;

	// make sure we haven't been here before
	FEBioInputModel& febio = GetFEBioModel();
	if (febio.Parts() != 0) throw XMLReader::InvalidTag(tag);

	// Add the one-and-only part
	FEBioInputModel::Part& part = *febio.AddPart("Object01");

	// loop over all sections
	++tag;
	do
	{
		if      (tag == "Nodes"      ) ParseGeometryNodes      (part, tag);
		else if (tag == "Elements"   ) ParseGeometryElements   (part, tag);
		else if (tag == "ElementData") ParseGeometryElementData(part, tag);
		else if (tag == "NodeSet"    ) ParseGeometryNodeSet    (part, tag);
		else if (tag == "Surface"    ) ParseGeometrySurface    (part, tag);
		else ParseUnknownTag(tag);

		++tag;
	}
	while (!tag.isend());

	// add a new instance
	assert(febio.Instances() == 0);
	FEBioInputModel::PartInstance* instance = new FEBioInputModel::PartInstance(&part);
	febio.AddInstance(instance);

	// don't forget to update the geometry
	febio.UpdateGeometry();

	// copy all mesh selections to named selections
	GetFEBioModel().CopyMeshSelections();

	return true;
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseGeometryNodes(FEBioInputModel::Part& part, XMLTag& tag)
{
	// first we need to figure out how many nodes there are
	int nn = tag.children();

	// create nodes
	FSMesh* pm = part.GetFEMesh();
	int N0 = pm->Nodes();
	pm->Create(N0 + nn, 0);

	// read nodal coordinates
	++tag;
	for (int i = 0; i<nn; ++i)
	{
		FSNode& node = pm->Node(N0 + i);
		tag.value(node.r);
		++tag;
	}
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseGeometryElements(FEBioInputModel::Part& part, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();

	// first we need to figure out how many elements there are
	int elems = tag.children();

	// get the required type attribute
	const char* sztype = tag.AttributeValue("type");
	FEElementType ntype = FE_INVALID_ELEMENT_TYPE;
	if      (strcmp(sztype, "hex8") == 0) ntype = FE_HEX8;
	else if (strcmp(sztype, "hex20") == 0) ntype = FE_HEX20;
	else if (strcmp(sztype, "hex27") == 0) ntype = FE_HEX27;
	else if (strcmp(sztype, "penta6") == 0) ntype = FE_PENTA6;
	else if (strcmp(sztype, "tet4") == 0) ntype = FE_TET4;
	else if (strcmp(sztype, "tet10") == 0) ntype = FE_TET10;
	else if (strcmp(sztype, "tet15") == 0) ntype = FE_TET15;
	else if (strcmp(sztype, "tet20") == 0) ntype = FE_TET20;
	else if (strcmp(sztype, "quad4") == 0) ntype = FE_QUAD4;
	else if (strcmp(sztype, "quad8") == 0) ntype = FE_QUAD8;
	else if (strcmp(sztype, "quad9") == 0) ntype = FE_QUAD9;
	else if (strcmp(sztype, "tri3") == 0) ntype = FE_TRI3;
	else if (strcmp(sztype, "tri6") == 0) ntype = FE_TRI6;
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
	int matID = tag.AttributeValue("mat", 0) - 1;

	// get the optional elset attribute
	const char* szname = tag.AttributeValue("elset", true);
	char sztmp[256] = {0};
	if (szname == 0)
	{
		sprintf(sztmp, "Part%d", part.Domains() + 1);
		szname = sztmp;
	}

	// add domain to list
	FEBioInputModel::Domain* p = part.AddDomain(szname, matID);

	// create elements
	FSMesh* pm = part.GetFEMesh();
	int NTE = pm->Elements();
	pm->Create(0, elems + NTE);

	// generate the part id
	int pid = part.Domains() - 1;

	// read element data
	++tag;
	int n[FSElement::MAX_NODES];
	for (int i = NTE; i<elems + NTE; ++i)
	{
		FSElement& el = pm->Element(i);
		el.SetType(ntype);
		el.m_gid = pid;
		el.m_nid = -1;
		if (tag == "elem")
		{
			el.m_nid = tag.AttributeValue<int>("id", -1);
			tag.value(n, el.Nodes());
			for (int j = 0; j<el.Nodes(); ++j) el.m_node[j] = n[j] - 1;
		}
		else throw XMLReader::InvalidTag(tag);

		assert(el.m_nid != -1);

		++tag;
	}
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseGeometryElementData(FEBioInputModel::Part& part, XMLTag& tag)
{
	FSMesh* pm = part.GetFEMesh();

	FEElementData* pdata = 0;
	// read additional element data
	int i = 0;
	++tag;
	do
	{
		if (tag == "element")
		{
			// get the element ID
			// NOTE: This assumes IDs start at one, which is no longer required!
			int id = tag.Attribute("id").value<int>() - 1;

			// get the element 
			FSElement& el = pm->Element(id);

			// read the data
			++tag;
			do
			{
				if (tag == "fiber")
				{
					vec3d a;

					// read the fiber direction
					tag.value(a);

					// normalize fiber
					a.Normalize();

					// set up a orthonormal coordinate system
					vec3d b(0, 1, 0);
					if (fabs(fabs(a*b) - 1) < 1e-7) b = vec3d(0, 0, 1);
					vec3d c = a^b;
					b = c^a;

					// make sure they are unit vectors
					b.Normalize();
					c.Normalize();

					// assign to element
					mat3d& m = el.m_Q;
					m.zero();
					m[0][0] = a.x; m[0][1] = b.x; m[0][2] = c.x;
					m[1][0] = a.y; m[1][1] = b.y; m[1][2] = c.y;
					m[2][0] = a.z; m[2][1] = b.z; m[2][2] = c.z;

					el.m_fiber = a;
				}
				else if (tag == "mat_axis")
				{
					vec3d a, d;

					++tag;
					do
					{
						if (tag == "a") tag.value(a);
						else if (tag == "d") tag.value(d);
						else ParseUnknownTag(tag);

						++tag;
					} while (!tag.isend());

					vec3d c = a^d;
					vec3d b = c^a;

					// normalize
					a.Normalize();
					b.Normalize();
					c.Normalize();

					// assign to element
					mat3d& m = el.m_Q;
					m.zero();
					m[0][0] = a.x; m[0][1] = b.x; m[0][2] = c.x;
					m[1][0] = a.y; m[1][1] = b.y; m[1][2] = c.y;
					m[2][0] = a.z; m[2][1] = b.z; m[2][2] = c.z;

					el.m_Qactive = true;
				}
				else if (tag == "thickness")
				{
					if (!el.IsShell()) return throw XMLReader::InvalidTag(tag);
					tag.value(el.m_h, el.Nodes());
				}
				else if (tag == "area")
				{
					if (!el.IsBeam()) return throw XMLReader::InvalidTag(tag);;
					tag.value(el.m_a0);
				}
				else if (tag.isleaf())
				{
					string dataName(tag.Name());

					// see if we have a data field
					if ((pdata == 0) || (pdata->GetName() != dataName))
					{
						// see if we can find this data field
						pdata = dynamic_cast<FEElementData*>(pm->FindMeshDataField(dataName));

						// if we couldn't find it, let's create a new one
						if (pdata == 0)
						{
//							pdata = pm->AddElementDataField(dataName);
//							pdata->ClearTags();
						}

						// make sure we have a data field at this point
						if (pdata == 0) return throw XMLReader::InvalidTag(tag);
					}

					// get the value
					double v;
					tag.value(v);

					// set the data value
					pdata->set(id, v);
				}
				else ParseUnknownTag(tag);
				++tag;
			} while (!tag.isend());
		}
		++tag;
	} while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseGeometryNodeSet(FEBioInputModel::Part& part, XMLTag& tag)
{
	// make sure there is a name attribute
	std::string name = tag.AttributeValue("name");

	// list to store node numbers
	vector<int> list;

	if (tag.isleaf())
	{
		// old, obsolete format
		// read the list
		tag.value(list);
	}
	else
	{
		// new, preferred format
		++tag;
		do
		{
			if (tag == "node")
			{
				int nid = tag.AttributeValue<int>("id", -1);
				if (nid == -1) throw XMLReader::MissingAttribute(tag, "id");
				list.push_back(nid);
			}
			else throw XMLReader::InvalidTag(tag);
			++tag;
		}
		while (!tag.isend());
	}

	// the node list is one-based, so make it zero-based
	for (int i = 0; i<(int)list.size(); ++i) list[i] -= 1;

	// create a new node set
	part.AddNodeSet(FEBioInputModel::NodeSet(name, list));
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseGeometrySurface(FEBioInputModel::Part& part, XMLTag& tag)
{
	// get the name
	const char* szname = tag.AttributeValue("name");

	// see if a surface with this name is already defined
	// if found, we'll continue, but we'll generate a warning.
	FEBioInputModel::Surface* ps = part.FindSurface(szname);
	if (ps) FileReader()->AddLogEntry("A surface named %s is already defined.", szname);

	// create a new surface
	FEBioInputModel::Surface s;
	s.m_name = szname;

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
		for (int j = 0; j<N; ++j) node[j] = nf[j] - 1;
		s.m_face.push_back(node);

		++tag;
	} while (!tag.isend());

	part.AddSurface(s);
}

//=============================================================================
//
//                                B O U N D A R Y 
//
//=============================================================================

//-----------------------------------------------------------------------------
//  Parses the boundary section from the xml file
//
bool FEBioFormat2::ParseBoundarySection(XMLTag& tag)
{
	if (tag.isleaf()) return true;
	++tag;
	do
	{
		if      (tag == "fix"       ) ParseBCFixed     (m_pBCStep, tag);
		else if (tag == "prescribe" ) ParseBCPrescribed(m_pBCStep, tag);
		else if (tag == "rigid_body") ParseRigidConstraint(m_pBCStep, tag);
		else ParseUnknownTag(tag);
		++tag;
	} while (!tag.isend());

	return true;
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseBCFixed(FSStep* pstep, XMLTag &tag)
{
	// get the bc attribute
	XMLAtt& abc = tag.Attribute("bc");

	// figure out the bc value
	int bc = 0;
	if (abc == "x") bc |= 1;
	else if (abc == "y") bc |= 2;
	else if (abc == "z") bc |= 4;
	else if (abc == "xy") bc |= 3;
	else if (abc == "yz") bc |= 6;
	else if (abc == "xz") bc |= 5;
	else if (abc == "xyz") bc |= 7;
	else if (abc == "u") bc |= 8;
	else if (abc == "v") bc |= 16;
	else if (abc == "w") bc |= 32;
	else if (abc == "uv") bc |= 24;
	else if (abc == "vw") bc |= 48;
	else if (abc == "uw") bc |= 40;
	else if (abc == "xyzuvw") bc |= 63;
	else if (abc == "uvw") bc |= 56;
	else if (abc == "t") bc |= 64;
	else if (abc == "p") bc |= 128;
	else if (abc == "X") bc |= 256;
	else if (abc == "Y") bc |= 512;
	else if (abc == "Z") bc |= 1024;
	else if (abc == "XY") bc |= 768;
	else if (abc == "YZ") bc |= 1536;
	else if (abc == "XZ") bc |= 1280;
	else if (abc == "XYZ") bc |= 1792;
	else if (abc == "wx") bc |= 256;
	else if (abc == "wy") bc |= 512;
	else if (abc == "wz") bc |= 1024;
	else if (abc == "wxy") bc |= 768;
	else if (abc == "wyz") bc |= 1536;
	else if (abc == "wxz") bc |= 1280;
	else if (abc == "wxyz") bc |= 1792;
	else if (abc == "sxyz") bc |= (4096 + 8192 + 16384);
	else if (abc == "ef") bc |= 2048;
	else if (abc == "c") bc |= (1 << 15);
	else if (abc == "c1") bc |= (1 << 15);
	else if (abc == "c2") bc |= (1 << 16);
	else if (abc == "c3") bc |= (1 << 17);
	else if (abc == "c4") bc |= (1 << 18);
	else if (abc == "c5") bc |= (1 << 19);
	else if (abc == "c6") bc |= (1 << 20);
	else 
	{
		char bc_copy[256]={0};
		strcpy(bc_copy, abc.cvalue());
		bc = GetDOFCode(bc_copy);
		if (bc == 0) throw XMLReader::InvalidAttributeValue(tag, "bc", abc.cvalue());
	}

	// get the mesh
	FSMesh* pm = &GetFEMesh();

	// create a nodeset for this BC
	FEBioInputModel& febio = GetFEBioModel();
	FEBioInputModel::PartInstance& part = GetInstance();
	GMeshObject* po = GetGObject();
	FSNodeSet* pg = 0;

	// see if the set attribute is defined
	const char* szset = tag.AttributeValue("set", true);
	if (szset)
	{
		// see if we can find the nodeset
		pg = febio.FindNamedNodeSet(szset);

		// make sure the set is found
		if (pg == 0) throw XMLReader::InvalidAttributeValue(tag, "set", szset);
	}
	else
	{
		pg = new FSNodeSet(po);
	}

	// read the node list
	std::vector<int> nodeList;
	if (tag.isleaf() == false)
	{
		++tag;
		do
		{
			// get the node ID
			int n = tag.Attribute("id").value<int>() - 1;

			// assign the node to this group
			nodeList.push_back(n);

			++tag;
		} while (!tag.isend());
		pg->clear();
		pg->add(nodeList);
	}
	else
	{
		nodeList = pg->CopyItems();
	}

	// create the constraint
	FSModel& fem = GetFSModel();
	char szname[256] = {0};
	if (bc < 64)
	{
		if (bc & 0x07)
		{
			int ddof = bc & 0x07;
			FSFixedDisplacement* pbc = new FSFixedDisplacement(&fem, pg, ddof, pstep->GetID());
			sprintf(szname, "FixedDisplacement%02d", CountBCs<FSFixedDisplacement>(fem) + 1);
			pbc->SetName(szname);
			pstep->AddComponent(pbc);
			pg = nullptr;
		}

		if (bc & 0x38)
		{
			if (pg == nullptr) {
				pg = new FSNodeSet(po); pg->add(nodeList);
			}

			int rdof = (bc >> 3);
			FSFixedRotation* pbc = new FSFixedRotation(&fem, pg, rdof, pstep->GetID());
			sprintf(szname, "FixedRotation%02d", CountBCs<FSFixedRotation>(fem) + 1);
			pbc->SetName(szname);
			pstep->AddComponent(pbc);
		}
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
void FEBioFormat2::ParseBCPrescribed(FSStep* pstep, XMLTag& tag)
{
	// check for optional type attribute
	bool brel = false;
	const char* szr = tag.AttributeValue("type", true);
	if (szr && (strcmp(szr, "relative") == 0)) brel = true;

	// determine bc
	XMLAtt& abc = tag.Attribute("bc");
	int bc = 0;
	if (abc == "x") bc = 0;
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
	else XMLReader::InvalidAttributeValue(tag, "bc", abc.cvalue());

	// get the loadcurve ID
	int lc = -1;
	XMLAtt* palc = tag.AttributePtr("lc");
	if (palc) lc = palc->value<int>() - 1;

	// make a new node set
	FEBioInputModel& febio = GetFEBioModel();
	FEBioInputModel::PartInstance& part = GetInstance();
	GMeshObject* po = GetGObject();
	FSMesh* pm = &GetFEMesh();
	FSNodeSet* pg = 0;
	char szname[256] = { 0 };

	// see if the set attribute is defined
	const char* szset = tag.AttributeValue("set", true);
	if (szset)
	{
		// make sure this is a leaf
		if (tag.isleaf() == false) throw XMLReader::InvalidValue(tag);

		// see if we can find the nodeset
		pg = febio.FindNamedNodeSet(szset);

		// make sure the set is found
		if (pg == 0) throw XMLReader::InvalidAttributeValue(tag, "set", szset);
	}
	else
	{
		pg = new FSNodeSet(po);
	}

	// make a new boundary condition
	FSModel& fem = GetFSModel();
	FSPrescribedDOF* pbc = 0;
	switch (bc)
	{
	case 0:
	case 1:
	case 2:
		{
			FSPrescribedDisplacement* pd = new FSPrescribedDisplacement(&fem, pg, bc, 1, pstep->GetID());
			pd->SetRelativeFlag(brel);
			febio.AddParamCurve(&pd->GetParam(FSPrescribedDisplacement::SCALE), lc);
			pbc = pd;
		}
		break;
	case 3:
		{
			  FSPrescribedTemperature* pd = new FSPrescribedTemperature(&fem, pg, bc, pstep->GetID());
			  pd->SetRelativeFlag(brel);
			  febio.AddParamCurve(&pd->GetParam(FSPrescribedDisplacement::SCALE), lc);
			  pbc = pd;
		}
		break;
	case 4:
		{
			  FSPrescribedFluidPressure* pd = new FSPrescribedFluidPressure(&fem, pg, 1, pstep->GetID());
			  pd->SetRelativeFlag(brel);
			  febio.AddParamCurve(&pd->GetParam(FSPrescribedDisplacement::SCALE), lc);
			  pbc = pd;
		}
		break;
	case 5:
	case 6:
	case 7:
		{
			  bc = bc - 5;
			  FSPrescribedFluidVelocity* pd = new FSPrescribedFluidVelocity(&fem, pg, bc, 1, pstep->GetID());
			  pd->SetRelativeFlag(brel);
			  febio.AddParamCurve(&pd->GetParam(FSPrescribedDisplacement::SCALE), lc);
			  pbc = pd;
		}
		break;
	case 8:
		{
			  FSPrescribedFluidDilatation* pd = new FSPrescribedFluidDilatation(&fem, pg, 1, pstep->GetID());
			  pd->SetRelativeFlag(brel);
			  febio.AddParamCurve(&pd->GetParam(FSPrescribedDisplacement::SCALE), lc);
			  pbc = pd;
		}
		break;
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		{
			   bc = bc - 9;
			   FSPrescribedConcentration* pd = new FSPrescribedConcentration(&fem, pg, bc, 1.0, pstep->GetID());
			   pd->SetRelativeFlag(brel);
			   febio.AddParamCurve(&pd->GetParam(FSPrescribedDisplacement::SCALE), lc);
			   pbc = pd;
		}
		break;
	}
	static int n = 1;
	sprintf(szname, "PrescribedBC%02d", n++);
	pbc->SetName(szname);
	pstep->AddComponent(pbc);

	if (tag.isleaf() == false)
	{
		pg->SetName(szname);

		// assign nodes to node sets
		++tag;
		do
		{
			// get the node ID
			int n = tag.Attribute("id").value<int>() - 1;

			// add the node
			pg->add(n);

			// get the value
			double scale = 1.0;
			tag.value(scale);
			pbc->SetScaleFactor(scale);
			++tag;
		} while (!tag.isend());
	}
	else
	{
		// set the scale factor
		double scale = 1.0;
		tag.value(scale);
		pbc->SetScaleFactor(scale);
	}
}

//=============================================================================
//
//                                L O A D S
//
//=============================================================================

//-----------------------------------------------------------------------------
//!  Parses the loads section from the xml file (version 2.0 and up)
bool FEBioFormat2::ParseLoadsSection(XMLTag& tag)
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
void FEBioFormat2::ParseNodeLoad(FSStep* pstep, XMLTag& tag)
{
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
	int lc = tag.AttributeValue<int>("lc", 0) - 1;

	FEBioInputModel& febio = GetFEBioModel();
	FEBioInputModel::PartInstance& part = GetInstance();
	GMeshObject* po = part.GetGObject();

	// create the node set
	FSMesh* pm = &GetFEMesh();
	char szname[256];
	FSNodeSet* pg = new FSNodeSet(po);

	static int n = 1;
	sprintf(szname, "ForceNodeset%02d", n++);
	pg->SetName(szname);

	// create the nodal load
	FSModel& fem = GetFSModel();
	FSNodalDOFLoad* pbc = new FSNodalDOFLoad(&fem, pg, bc, 1, pstep->GetID());
	sprintf(szname, "ForceLoad%02d", CountLoads<FSNodalDOFLoad>(fem)+1);
	pbc->SetName(szname);
	febio.AddParamCurve(&pbc->GetParam(FSNodalDOFLoad::LOAD), lc);
	pstep->AddComponent(pbc);

	// assign nodes to node sets
	double s = 1.0;
	++tag;
	do
	{
		int n = tag.Attribute("id").value<int>() - 1;
		tag.value(s);
		pg->add(n);
		++tag;
	}
	while (!tag.isend());

	pbc->SetLoad(s);
}

//-----------------------------------------------------------------------------
//! Parses the surface_load section.
void FEBioFormat2::ParseSurfaceLoad(FSStep* pstep, XMLTag& tag)
{
	XMLAtt& att = tag.Attribute("type");
	if      (att == "pressure"           ) ParseLoadPressure           (pstep, tag);
	else if (att == "traction"           ) ParseLoadTraction           (pstep, tag);
	else if (att == "fluidflux"          ) ParseLoadFluidFlux          (pstep, tag);
	else if (att == "soluteflux"         ) ParseLoadSoluteFlux         (pstep, tag);
	else if (att == "normal_traction"    ) ParseLoadNormalTraction     (pstep, tag);
	else if (att == "heatflux"           ) ParseLoadHeatFlux           (pstep, tag);
	else if (att == "convective_heatflux") ParseLoadConvectiveHeatFlux (pstep, tag);
	else if (att == "fluid viscous traction") ParseLoadFluidTraction      (pstep, tag);
    else if (att == "fluid velocity"                ) ParseLoadFluidVelocity               (pstep, tag);
    else if (att == "fluid normal velocity"         ) ParseLoadFluidNormalVelocity         (pstep, tag);
    else if (att == "fluid rotational velocity"     ) ParseLoadFluidRotationalVelocity     (pstep, tag);
    else if (att == "fluid resistance"              ) ParseLoadFluidFlowResistance         (pstep, tag);
    else if (att == "fluid backflow stabilization"  ) ParseLoadFluidBackflowStabilization  (pstep, tag);
    else if (att == "fluid tangential stabilization") ParseLoadFluidTangentialStabilization(pstep, tag);
    else if (att == "fluid-FSI traction" ) ParseLoadFSITraction        (pstep, tag);
	else if (att == "concentration flux" ) ParseLoadConcentrationFlux  (pstep, tag);
	else ParseUnknownAttribute(tag, "type");
}

//-----------------------------------------------------------------------------
FSSurface* FEBioFormat2::ParseLoadSurface(XMLTag& tag)
{
	// create a new surface
	FEBioInputModel& febio = GetFEBioModel();
	FEBioInputModel::PartInstance& part = GetInstance();

	// see if the set is defined 
	if (tag.isempty())
	{
		// get the surface name
		const char* szset = tag.AttributeValue("set");

		// find the surface
		FSSurface* ps = febio.FindNamedSurface(szset);
		if (ps == 0) throw XMLReader::InvalidAttributeValue(tag, "set", szset);

		return ps;
	}
	else
	{
		FEBioInputModel::Surface surf;
		// read the pressure data
		int nf[FSElement::MAX_NODES], N;
		++tag;
		do
		{
			// read the facet
			if (tag == "quad4") N = 4;
			else if (tag == "quad8") N = 8;
			else if (tag == "tri3") N = 3;
			else if (tag == "tri6") N = 6;
			tag.value(nf, N);

			vector<int> node(N);
			for (int j = 0; j<N; ++j) node[j] = nf[j] - 1;
			surf.m_face.push_back(node);

			++tag;
		}
		while (!tag.isend());

		FSSurface* ps = part.BuildFESurface(surf);
		part.GetGObject()->AddFESurface(ps);

		return ps;
	}
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadPressure(FSStep* pstep, XMLTag& tag)
{
	// create a new surface load
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();
	FSPressureLoad* pbc = new FSPressureLoad(&fem);
	pstep->AddComponent(pbc);

	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "PressureLoad%d", CountLoads<FSPressureLoad>(fem));
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
		else if (tag == "pressure")
		{
			int lc = tag.Attribute("lc").value<int>() - 1;
			febio.AddParamCurve(&pbc->GetParam(FSPressureLoad::LOAD), lc);
			double s; tag.value(s);
			pbc->SetLoad(s);
		}
		else if (tag == "surface")
		{
			FSSurface* psurf = ParseLoadSurface(tag);
			pbc->SetItemList(psurf);
		}
		++tag;
	} while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadTraction(FSStep* pstep, XMLTag& tag)
{
	// create a new surface load
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();
	FSSurfaceTraction* pbc = new FSSurfaceTraction(&fem);
	pstep->AddComponent(pbc);

	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "TractionLoad%d", CountLoads<FSSurfaceTraction>(fem));
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
		else if (tag == "surface")
		{
			FSSurface* psurf = ParseLoadSurface(tag);
			pbc->SetItemList(psurf);
		}
		++tag;
	}
	while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadFluidTraction(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();

	// create a new surface load
	FSModel& fem = GetFSModel();
	FSFluidTraction* pbc = new FSFluidTraction(&fem);
	pstep->AddComponent(pbc);

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
		else if (tag == "surface")
		{
			FSSurface* psurf = ParseLoadSurface(tag);
			pbc->SetItemList(psurf);
		}
		++tag;
	} 
	while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadFluidVelocity(FSStep* pstep, XMLTag& tag)
{
    FEBioInputModel& febio = GetFEBioModel();
    // create a new surface load
    FSModel& fem = GetFSModel();
    FSFluidVelocity* pbc = new FSFluidVelocity(&fem);
    pstep->AddComponent(pbc);
    
    // set the name
    char szname[256] = { 0 };
    sprintf(szname, "FluidVelocity%d", CountLoads<FSFluidVelocity>(fem));
    pbc->SetName(szname);
    
    // read the parameters
    ++tag;
    do
    {
        if (tag == "scale")
        {
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(&pbc->GetParam(FSFluidVelocity::LOAD), lc);
        }
        else if (tag == "velocity")
        {
            vec3d t; tag.value(t);
            pbc->SetLoad(t);
        }
        else if (tag == "surface")
        {
            FSSurface* psurf = ParseLoadSurface(tag);
            pbc->SetItemList(psurf);
        }
        ++tag;
    } while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadFluidNormalVelocity(FSStep* pstep, XMLTag& tag)
{
    FEBioInputModel& febio = GetFEBioModel();
    // create a new surface load
    FSModel& fem = GetFSModel();
    FSFluidNormalVelocity* pbc = new FSFluidNormalVelocity(&fem);
    pstep->AddComponent(pbc);
    
    // set the name
    char szname[256] = { 0 };
    sprintf(szname, "FluidNormalVelocity%d", CountLoads<FSFluidNormalVelocity>(fem));
    pbc->SetName(szname);
    
    // read the parameters
    ++tag;
    do
    {
        if (tag == "velocity")
        {
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(&pbc->GetParam(FSFluidNormalVelocity::LOAD), lc);
            double s; tag.value(s);
            pbc->SetLoad(s);
        }
        else if (tag == "prescribe_nodal_velocities")
        {
            bool s; tag.value(s);
            pbc->SetBP(s);
        }
        else if (tag == "parabolic")
        {
            bool s; tag.value(s);
            pbc->SetBParab(s);
        }
        else if (tag == "prescribe_rim_pressure")
        {
            bool s; tag.value(s);
            pbc->SetBRimP(s);
        }
        else if (tag == "surface")
        {
            FSSurface* psurf = ParseLoadSurface(tag);
            pbc->SetItemList(psurf);
        }
        ++tag;
    } while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadFluidRotationalVelocity(FSStep* pstep, XMLTag& tag)
{
    FEBioInputModel& febio = GetFEBioModel();
    // create a new surface load
    FSModel& fem = GetFSModel();
    FSFluidRotationalVelocity* pbc = new FSFluidRotationalVelocity(&fem);
    pstep->AddComponent(pbc);
    
    // set the name
    char szname[256] = { 0 };
    sprintf(szname, "FluidRotationalVelocity%d", CountLoads<FSFluidRotationalVelocity>(fem));
    pbc->SetName(szname);
    
    // read the parameters
    ++tag;
    do
    {
        if (tag == "angular_speed")
        {
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(&pbc->GetParam(FSFluidRotationalVelocity::LOAD), lc);
            double s; tag.value(s);
            pbc->SetLoad(s);
        }
        else if (tag == "axis")
        {
            vec3d t; tag.value(t);
            pbc->SetAxis(t);
        }
        else if (tag == "origin")
        {
            vec3d t; tag.value(t);
            pbc->SetOrigin(t);
        }
        else if (tag == "surface")
        {
            FSSurface* psurf = ParseLoadSurface(tag);
            pbc->SetItemList(psurf);
        }
        ++tag;
    } while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadFluidFlowResistance(FSStep* pstep, XMLTag& tag)
{
    FEBioInputModel& febio = GetFEBioModel();
    // create a new surface load
    FSModel& fem = GetFSModel();
    FSFluidFlowResistance* pbc = new FSFluidFlowResistance(&fem);
    pstep->AddComponent(pbc);
    
    // set the name
    char szname[256] = { 0 };
    sprintf(szname, "FluidResistance%d", CountLoads<FSFluidFlowResistance>(fem));
    pbc->SetName(szname);
    
    // read the parameters
    ++tag;
    do
    {
        if (tag == "R")
        {
            double s; tag.value(s);
            pbc->SetLoad(s);
        }
        else if (tag == "pressure_offset")
        {
			int lc = tag.AttributeValue<int>("lc", 0) - 1;
			if (lc >= 0)
			{
				febio.AddParamCurve(&pbc->GetParam(FSFluidFlowResistance::PO), lc);
			}
			double s; tag.value(s);
            pbc->SetPO(s);
        }
        else if (tag == "surface")
        {
            FSSurface* psurf = ParseLoadSurface(tag);
            pbc->SetItemList(psurf);
        }
        ++tag;
    } while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadFluidBackflowStabilization(FSStep* pstep, XMLTag& tag)
{
    FEBioInputModel& febio = GetFEBioModel();
    // create a new surface load
    FSModel& fem = GetFSModel();
    FSFluidBackflowStabilization* pbc = new FSFluidBackflowStabilization(&fem);
    pstep->AddComponent(pbc);
    
    // set the name
    char szname[256] = { 0 };
    sprintf(szname, "BackflowStabilization%d", CountLoads<FSFluidBackflowStabilization>(fem));
    pbc->SetName(szname);
    
    // read the parameters
    ++tag;
    do
    {
        if (tag == "beta")
        {
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(&pbc->GetParam(FSFluidBackflowStabilization::LOAD), lc);
            double s; tag.value(s);
            pbc->SetLoad(s);
        }
        else if (tag == "surface")
        {
            FSSurface* psurf = ParseLoadSurface(tag);
            pbc->SetItemList(psurf);
        }
        ++tag;
    } while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadFluidTangentialStabilization(FSStep* pstep, XMLTag& tag)
{
    FEBioInputModel& febio = GetFEBioModel();
    // create a new surface load
    FSModel& fem = GetFSModel();
    FSFluidTangentialStabilization* pbc = new FSFluidTangentialStabilization(&fem);
    pstep->AddComponent(pbc);
    
    // set the name
    char szname[256] = { 0 };
    sprintf(szname, "TangentialStabilization%d", CountLoads<FSFluidTangentialStabilization>(fem));
    pbc->SetName(szname);
    
    // read the parameters
    ++tag;
    do
    {
        if (tag == "beta")
        {
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(&pbc->GetParam(FSFluidTangentialStabilization::LOAD), lc);
            double s; tag.value(s);
            pbc->SetLoad(s);
        }
        else if (tag == "surface")
        {
            FSSurface* psurf = ParseLoadSurface(tag);
            pbc->SetItemList(psurf);
        }
        ++tag;
    } while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadFSITraction(FSStep* pstep, XMLTag& tag)
{
    FEBioInputModel& febio = GetFEBioModel();
    // create a new surface load
    FSModel& fem = GetFSModel();
    FSFSITraction* pbc = new FSFSITraction(&fem);
    pstep->AddComponent(pbc);
    
    // set the name
    char szname[256] = { 0 };
    sprintf(szname, "FSInterfaceTraction%d", CountLoads<FSFSITraction>(fem));
    pbc->SetName(szname);
    
    // read the parameters
    ++tag;
    do
    {
        if (tag == "surface")
        {
            FSSurface* psurf = ParseLoadSurface(tag);
            pbc->SetItemList(psurf);
        }
        ++tag;
    } while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadFluidFlux(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// create a new surface load
	FSFluidFlux* pbc = new FSFluidFlux(&fem);
	pstep->AddComponent(pbc);

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
		else if (tag == "surface")
		{
			FSSurface* psurf = ParseLoadSurface(tag);
			pbc->SetItemList(psurf);
		}
		++tag;
	} while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadSoluteFlux(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();

	FSModel& fem = GetFSModel();

	// create a new surface load
	FSSoluteFlux* pbc = new FSSoluteFlux(&fem);
	pstep->AddComponent(pbc);

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
			int lc = tag.Attribute("lc").value<int>() - 1;
			febio.AddParamCurve(&pbc->GetParam(FSSoluteFlux::LOAD), lc);
			double s; tag.value(s);
			pbc->SetLoad(s);
		}
		else if (tag == "surface")
		{
			FSSurface* psurf = ParseLoadSurface(tag);
			pbc->SetItemList(psurf);
		}
		++tag;
	} while (!tag.isend());
}


//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadConcentrationFlux(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();

	FSModel& fem = GetFSModel();

	// create a new surface load
	FSConcentrationFlux* pcf = new FSConcentrationFlux(&fem);
	pstep->AddComponent(pcf);

	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "ConcentrationFlux%d", CountLoads<FSConcentrationFlux>(fem));
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
			int lc = tag.Attribute("lc").value<int>() - 1;
			febio.AddParamCurve(&pcf->GetParam(FSConcentrationFlux::FLUX), lc);
			double s; tag.value(s);
			pcf->SetFlux(s);
		}
		else if (tag == "surface")
		{
			FSSurface* psurf = ParseLoadSurface(tag);
			pcf->SetItemList(psurf);
		}
		++tag;
	}
	while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadNormalTraction(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();

	FSModel& fem = GetFSModel();

	// create a new surface load
	FSBPNormalTraction* pbc = new FSBPNormalTraction(&fem);
	pstep->AddComponent(pbc);
	pbc->SetStep(pstep->GetID());

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
		else if (tag == "surface")
		{
			FSSurface* psurf = ParseLoadSurface(tag);
			pbc->SetItemList(psurf);
		}
		++tag;
	} while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadHeatFlux(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();

	FSModel& fem = GetFSModel();

	// create a new surface load
	FSHeatFlux* pbc = new FSHeatFlux(&fem);
	pstep->AddComponent(pbc);

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
		else if (tag == "surface")
		{
			FSSurface* psurf = ParseLoadSurface(tag);
			pbc->SetItemList(psurf);
		}
		++tag;
	} while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadConvectiveHeatFlux(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();

	FSModel& fem = GetFSModel();

	// create a new surface load
	FSConvectiveHeatFlux* pbc = new FSConvectiveHeatFlux(&fem);
	pstep->AddComponent(pbc);

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
		else if (tag == "surface")
		{
			FSSurface* psurf = ParseLoadSurface(tag);
			pbc->SetItemList(psurf);
		}
		++tag;
	} while (!tag.isend());
}

//-----------------------------------------------------------------------------
//! Parses the body_load section.
void FEBioFormat2::ParseBodyLoad(FSStep* pstep, XMLTag& tag)
{
	XMLAtt& att = tag.Attribute("type");
	if      (att == "const"      ) ParseBodyForce(pstep, tag);
	else if (att == "heat_source") ParseHeatSource(pstep, tag);
	else ParseUnknownAttribute(tag, "type");
}

//-----------------------------------------------------------------------------
bool FEBioFormat2::ParseInitialSection(XMLTag& tag)
{
	GMeshObject* po = GetGObject();
	FSModel& fem = GetFSModel();
	FSMesh* pm = &GetFEMesh();

	++tag;
	do
	{
		if (tag == "velocity")	// initial velocity BC
		{
			FSNodeSet* pg = 0;
			vec3d v;

			// read the nodes
			++tag;
			do
			{
				if (tag == "node")
				{
					// NOTE: it is assumed that all nodes have the same value
					int id = tag.Attribute("id").value<int>() - 1;
					tag.value(v);

					// see if we need to make a new group, otherwise add the node to the group
					if (pg == 0) pg = new FSNodeSet(po);
					pg->add(id);
				}
				else ParseUnknownTag(tag);
				++tag;
			} while (!tag.isend());

			// create a new initial velocity BC
			FSNodalVelocities* pbc = new FSNodalVelocities(&fem, pg, v, m_pBCStep->GetID());
			char szname[64] = { 0 };
			sprintf(szname, "InitialVelocity%02d", CountICs<FSNodalVelocities>(fem)+1);
			pbc->SetName(szname);
			m_pBCStep->AddComponent(pbc);
		}
		else if (tag == "concentration")	// initial concentration BC
		{
			FSNodeSet* pg = 0;
			double c;

			const char* szc = tag.AttributeValue("sol", true);
			int bc = 0;
			if (szc)
			{
				bc = atoi(szc) - 1;
				if (bc < 0) throw ReadError();
			}

			// read the nodes
			++tag;
			do
			{
				if (tag == "node")
				{
					// NOTE: it is assumed that all nodes have the same value
					int id = tag.Attribute("id").value<int>() - 1;
					tag.value(c);

					// see if we need to make a new group, otherwise add the node to the group
					if (pg == 0) pg = new FSNodeSet(po);
					pg->add(id);
				}
				else ParseUnknownTag(tag);
				++tag;
			} while (!tag.isend());

			// create a new initial concentration BC
			FSInitConcentration* pbc = new FSInitConcentration(&fem, pg, bc, c, m_pBCStep->GetID());
			char szname[64] = { 0 };
			sprintf(szname, "InitialConcentration%02d", CountICs<FSInitConcentration>(fem)+1);
			pbc->SetName(szname);
			m_pBCStep->AddComponent(pbc);
		}
		else if (tag == "fluid_pressure")	// initial fluid pressure
		{
			FSNodeSet* pg = 0;
			double p;

			// read the nodes
			++tag;
			do
			{
				if (tag == "node")
				{
					// NOTE: it is assumed that all nodes have the same value
					int id = tag.Attribute("id").value<int>() - 1;
					tag.value(p);

					// see if we need to make a new group, otherwise add the node to the group
					if (pg == 0) pg = new FSNodeSet(po);
					pg->add(id);
				}
				else ParseUnknownTag(tag);
				++tag;
			} while (!tag.isend());

			// create a new initial BC
			FSInitFluidPressure* pbc = new FSInitFluidPressure(&fem, pg, p, m_pBCStep->GetID());
			char szname[64] = { 0 };
			sprintf(szname, "InitialFluidPressure%02d", CountICs<FSInitFluidPressure>(fem)+1);
			pbc->SetName(szname);
			m_pBCStep->AddComponent(pbc);
		}
		else if (tag == "temperature")	// initial temperature
		{
			FSNodeSet* pg = 0;
			double p;

			// read the nodes
			++tag;
			do
			{
				if (tag == "node")
				{
					// NOTE: it is assumed that all nodes have the same value
					int id = tag.Attribute("id").value<int>() - 1;
					tag.value(p);

					// see if we need to make a new group, otherwise add the node to the group
					if (pg == 0) pg = new FSNodeSet(po);
					pg->add(id);
				}
				else ParseUnknownTag(tag);
				++tag;
			} while (!tag.isend());

			// create a new initial BC
			FSInitTemperature* pbc = new FSInitTemperature(&fem, pg, p, m_pBCStep->GetID());
			char szname[64] = { 0 };
			sprintf(szname, "InitialTemperature%02d", CountICs<FSInitTemperature>(fem)+1);
			pbc->SetName(szname);
			m_pBCStep->AddComponent(pbc);
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
bool FEBioFormat2::ParseContactSection(XMLTag& tag)
{
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
void FEBioFormat2::ParseContact(FSStep *pstep, XMLTag &tag)
{
	XMLAtt& atype = tag.Attribute("type");
	if      (atype == "sliding-node-on-facet"  ) ParseContactSliding    (pstep, tag);
	else if (atype == "sliding-facet-on-facet" ) ParseContactF2FSliding (pstep, tag);
	else if (atype == "sliding-biphasic"       ) ParseContactBiphasic   (pstep, tag);
	else if (atype == "sliding-biphasic-solute") ParseContactSolute     (pstep, tag);
	else if (atype == "sliding-multiphasic"    ) ParseContactMultiphasic(pstep, tag);
	else if (atype == "tied"                   ) ParseContactTied       (pstep, tag);
	else if (atype == "tied-facet-on-facet"    ) ParseContactTiedF2F    (pstep, tag);
	else if (atype == "tied-elastic"           ) ParseContactTiedElastic(pstep, tag);
	else if (atype == "sticky"                 ) ParseContactSticky     (pstep, tag);
	else if (atype == "periodic boundary"      ) ParseContactPeriodic   (pstep, tag);
	else if (atype == "rigid"                  ) ParseContactRigid      (pstep, tag);
	else if (atype == "rigid joint"            ) ParseContactJoint      (pstep, tag);
	else if (atype == "sliding-elastic"        ) ParseContactTC         (pstep, tag);
	else if (atype == "tied-biphasic"          ) ParseContactTiedPoro   (pstep, tag);
	else if (atype == "rigid_wall"             ) ParseRigidWall         (pstep, tag);
	else if (atype == "linear constraint"      ) ParseLinearConstraint  (pstep, tag);
	else if (atype == "tied-multiphasic"       ) ParseContactTiedMultiphasic(pstep, tag);
	else
	{
		// these are some old names
		if      (atype == "sliding_with_gaps"          ) ParseContactSliding   (pstep, tag);
		else if (atype == "facet-to-facet sliding"     ) ParseContactF2FSliding(pstep, tag);
		else if (atype == "sliding2"                   ) ParseContactBiphasic  (pstep, tag);
		else if (atype == "sliding3"                   ) ParseContactSolute    (pstep, tag);
		else if (atype == "sliding-tension-compression") ParseContactTC        (pstep, tag);
		else ParseUnknownAttribute(tag, "type");
	}
}

//-----------------------------------------------------------------------------
FSSurface* FEBioFormat2::ParseContactSurface(XMLTag& tag, int format)
{
	FEBioInputModel& febio = GetFEBioModel();
	FEBioInputModel::PartInstance& part = GetInstance();
	GMeshObject* po = part.GetGObject();

	// see if the set is defined 
	if (tag.isempty())
	{
		// get the surface name
		const char* szset = tag.AttributeValue("set");

		// find the surface
		FSSurface* psurf = febio.FindNamedSurface(szset);
		if (psurf == 0) throw XMLReader::InvalidAttributeValue(tag, "set", szset);

		return psurf;
	}
	else
	{
		FEBioInputModel::Surface surf;

		if (format == 0)
		{
			int nf[FSFace::MAX_NODES], N;

			// read faces
			++tag;
			do
			{
				// read face data
				if      (tag == "quad4") N = 4;
				else if (tag == "quad8") N = 8;
				else if (tag == "quad9") N = 9;
				else if (tag == "tri3" ) N = 3;
				else if (tag == "tri6" ) N = 6;
				else if (tag == "tri7" ) N = 7;
				else assert(false);

				tag.value(nf, N);

				// make zero-based
				vector<int> node(N);
				for (int j = 0; j<N; ++j) node[j] = nf[j] - 1;
				surf.m_face.push_back(node);

				++tag;
			}
			while (!tag.isend());
		}
		else
		{
			FEBioMesh& mesh = GetFEBioMesh();

			int n[2];
			// read the faces in element-face format
			++tag;
			do
			{
				tag.value(n, 2);

				FSElement* el = mesh.FindElementFromID(n[0]); assert(el);
				if (el)
				{
					int nf = el->Faces();
					if ((n[1] >= 1) && (n[1] <= nf))
					{
						FSFace f = el->GetFace(n[1] - 1);
						vector<int> node(f.Nodes());
						for (int i=0; i<f.Nodes(); ++i) node[i] = f.n[i];
						surf.m_face.push_back(node);
					}
				}

				++tag;
			}
			while (!tag.isend());
		}

		FEBioInputModel& febio = GetFEBioModel();
		FSSurface *psurf = part.BuildFESurface(surf);
		part.GetGObject()->AddFESurface(psurf);
		return psurf;
	}
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseConstraint(FSStep* pstep, XMLTag& tag)
{
	FSModel* fem = &GetFSModel();

	const char* sztype = tag.AttributeValue("type");

	FSModelConstraint* plc = fscore_new<FSModelConstraint>(fem, FENLCONSTRAINT_ID, sztype);
	if (plc == nullptr) throw XMLReader::InvalidAttributeValue(tag, "type", sztype);

	string name;
	const char* szname = tag.AttributeValue("name", true);
	if (szname == nullptr)
	{
		int n = CountConstraints<FSModelConstraint>(*fem);
		name = Namify(sztype);
		stringstream ss; 
		ss << name << n + 1;
		name = ss.str();
	}
	else name = szname;

	plc->SetName(name);

	ReadParameters(*plc, tag);

	pstep->AddComponent(plc);
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseContactParams(XMLTag& tag, FSPairedInterface* pi, int nid)
{
	char szbuf[256] = {0};
	++tag;
	do
	{
		// try to read the parameters
		if (ReadParam(*pi, tag) == false)
		{
			// read the surface definition
			if (tag == "surface")
			{
				// get the type attribute to see if this is the master or slave surface
				const char* sztype = tag.AttributeValue("type");
				int ntype = 0;
				if (strcmp(sztype, "master") == 0) ntype = 1;
				else if (strcmp(sztype, "slave") == 0) ntype = 2;

				// get the (optional) name
				// if no name is provided a default one will be provided
				const char* szn = tag.AttributeValue("name", true);

				// get the (optional) format
				int nfmt = 0;
				const char* szfmt = tag.AttributeValue("format", true);
				if (szfmt && (strcmp(szfmt, "element face") == 0)) nfmt = 1;

				// create a new surface
				FSSurface* ps = ParseContactSurface(tag, nfmt);
				if (ntype == 1)
				{
					sprintf(szbuf, "MasterSurface%02d", nid);
					ps->SetName(szn ? szn : szbuf);
					pi->SetSecondarySurface(ps);
				}
				else
				{
					sprintf(szbuf, "SlaveSurface%02d", nid);
					ps->SetName(szn ? szn : szbuf);
					pi->SetPrimarySurface(ps);
				}
			}
			else ParseUnknownTag(tag);
		}
		++tag;
	}
	while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseContactSliding(FSStep* pstep, XMLTag& tag)
{
	char szbuf[256];

	FSModel& fem = GetFSModel();

	// create new sliding interface
	FSSlidingWithGapsInterface* pi = new FSSlidingWithGapsInterface(&fem, pstep->GetID());

	int nid = CountInterfaces<FSSlidingWithGapsInterface>(fem) +1;

	// get the (optional) contact name
	const char* szname = tag.AttributeValue("name", true);
	if (szname) sprintf(szbuf, "%s", szname);
	else sprintf(szbuf, "SlidingInterface%02d", nid);
	pi->SetName(szbuf);

	FSSurface *pms = 0, *pss = 0;
	FSMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();

	ParseContactParams(tag, pi, nid);
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseContactF2FSliding(FSStep* pstep, XMLTag& tag)
{
	char szbuf[256];

	FSModel& fem = GetFSModel();

	// create new sliding interface
	FSFacetOnFacetInterface* pi = new FSFacetOnFacetInterface(&fem, pstep->GetID());

	int nid = CountInterfaces<FSFacetOnFacetInterface>(fem) +1;

	// get the (optional) contact name
	const char* szname = tag.AttributeValue("name", true);
	if (szname) sprintf(szbuf, "%s", szname);
	else sprintf(szbuf, "SlidingContact%02d", nid);
	pi->SetName(szbuf);

	ParseContactParams(tag, pi, nid);
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseContactBiphasic(FSStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();

	// create new contact interface
	FSPoroContact* pi = new FSPoroContact(&fem, pstep->GetID());

	int nid = CountInterfaces<FSPoroContact>(fem)+1;

	// read the name
	char szname[256];
	sprintf(szname, "BiphasicContact%02d", nid);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	ParseContactParams(tag, pi, nid);

	// add interface to step
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseContactSolute(FSStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();
	FSMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();

	// create new interface
	FSPoroSoluteContact* pi = new FSPoroSoluteContact(&fem, pstep->GetID());

	int nid = CountInterfaces<FSPoroSoluteContact>(fem) +1;

	// set name
	char szname[256];
	sprintf(szname, "BiphasicSoluteContact%02d", nid);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	ParseContactParams(tag, pi, nid);

	// add interface to step
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseContactMultiphasic(FSStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();
	FSMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();

	// create new interface
	FSMultiphasicContact* pi = new FSMultiphasicContact(&fem, pstep->GetID());

	// set name
	int nid = CountInterfaces<FSMultiphasicContact>(fem) +1;
	char szname[256];
	sprintf(szname, "MultiphasicContact%02d", nid);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	ParseContactParams(tag, pi, nid);

	// add interface to step
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseContactTiedMultiphasic(FSStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();
	FSMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();

	// create new interface
	FSTiedMultiphasicInterface* pi = new FSTiedMultiphasicInterface(&fem, pstep->GetID());

	// set name
	int nid = CountInterfaces<FSTiedMultiphasicInterface>(fem) +1;
	char szname[256];
	sprintf(szname, "TiedMultiphasicContact%02d", nid);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	ParseContactParams(tag, pi, nid);

	// add interface to step
	pstep->AddComponent(pi);
}
//-----------------------------------------------------------------------------
void FEBioFormat2::ParseContactTied(FSStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();

	// create new interface
	FSTiedInterface* pi = new FSTiedInterface(&fem, pstep->GetID());

	// set name
	int nid = CountInterfaces<FSTiedInterface>(fem) +1;
	char szname[256];
	sprintf(szname, "TiedInterface%02d", nid);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	ParseContactParams(tag, pi, nid);

	// add interface to step
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseContactTiedF2F(FSStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();

	// create new interface
	FSF2FTiedInterface* pi = new FSF2FTiedInterface(&fem, pstep->GetID());

	// set name
	int nid = CountInterfaces<FSF2FTiedInterface>(fem) + 1;
	char szname[256];
	sprintf(szname, "TiedF2FInterface%02d", nid);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	ParseContactParams(tag, pi, nid);

	// add interface to step
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseContactTiedElastic(FSStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();

	// create new interface
	FSTiedElasticInterface* pi = new FSTiedElasticInterface(&fem, pstep->GetID());

	// set name
	int nid = CountInterfaces<FSTiedElasticInterface>(fem) +1;
	char szname[256];
	sprintf(szname, "TiedElasticInterface%02d", nid);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	ParseContactParams(tag, pi, nid);

	// add interface to step
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseContactSticky(FSStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();
	FSMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();

	// create new interface
	FSStickyInterface* pi = new FSStickyInterface(&fem, pstep->GetID());

	// set name
	int nid = CountInterfaces<FSStickyInterface>(fem) +1;
	char szname[256];
	sprintf(szname, "StickyInterface%02d", nid);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	ParseContactParams(tag, pi, nid);

	// add interface to step
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseContactPeriodic(FSStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();
	FSMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();

	// create new interface
	FSPeriodicBoundary* pi = new FSPeriodicBoundary(&fem, pstep->GetID());

	// set name
	int nid = CountInterfaces<FSPeriodicBoundary>(fem) +1;
	char szname[256];
	sprintf(szname, "PeriodicBoundary%02d", nid);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	ParseContactParams(tag, pi, nid);

	// add interface to step
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseContactTC(FSStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();
	FSMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();

	// create new interface
	FSTensionCompressionInterface* pi = new FSTensionCompressionInterface(&fem, pstep->GetID());

	// set name
	int nid = CountInterfaces<FSTensionCompressionInterface>(fem) +1;
	char szname[256];
	sprintf(szname, "TCInterface%02d", nid);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	ParseContactParams(tag, pi, nid);

	// add interface to step
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseContactTiedPoro(FSStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();
	FSMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();

	// create new interface
	FSTiedBiphasicInterface* pi = new FSTiedBiphasicInterface(&fem, pstep->GetID());

	// set name
	int nid = CountInterfaces<FSTiedBiphasicInterface>(fem) +1;
	char szname[256];
	sprintf(szname, "TiedBiphasicInterface%02d", nid);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	ParseContactParams(tag, pi, nid);

	// add interface to step
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseRigidWall(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();
	FSMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();

	// create a new interface
	FSRigidWallInterface* pci = new FSRigidWallInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "RigidWall%02d", CountInterfaces<FSRigidWallInterface>(fem) + 1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pci->SetName(szname);

	FSSurface *pms = 0, *pss = 0;

	++tag;
	do
	{
		// read parameters
		if (tag == "laugon") { int n; tag.value(n); pci->SetBoolValue(FSRigidWallInterface::LAUGON, (n == 0 ? false : true)); }
		else if (tag == "tolerance") { double f; tag.value(f); pci->SetFloatValue(FSRigidWallInterface::ALTOL, f); }
		else if (tag == "penalty") { double f; tag.value(f); pci->SetFloatValue(FSRigidWallInterface::PENALTY, f); }
		if (tag == "plane")
		{
			double n[4];
			tag.value(n, 4);
			pci->SetFloatValue(FSRigidWallInterface::PA, n[0]);
			pci->SetFloatValue(FSRigidWallInterface::PB, n[1]);
			pci->SetFloatValue(FSRigidWallInterface::PC, n[2]);
			pci->SetFloatValue(FSRigidWallInterface::PD, n[3]);

			// if some older formats the lc was set on the plane parameter, although now it should be on the offset parameter
			const char* szlc = tag.AttributeValue("lc", true);
			if (szlc)
			{
				febio.AddParamCurve(&pci->GetParam(FSRigidWallInterface::OFFSET), atoi(szlc) - 1);
				pci->GetParam(FSRigidWallInterface::OFFSET).SetFloatValue(1.0);
			}
		}
		else if (tag == "offset")
		{
			double s = 0.0;
			tag.value(s);

			const char* szlc = tag.AttributeValue("lc", true);
			if (szlc)
			{
				febio.AddParamCurve(&pci->GetParam(FSRigidWallInterface::OFFSET), atoi(szlc) - 1);
				pci->GetParam(FSRigidWallInterface::OFFSET).SetFloatValue(s);
			}
		}
		else if (tag == "surface")
		{
			const char* szn = tag.AttributeValue("name", true);

			FSSurface* ps = ParseContactSurface(tag);
			pci->SetItemList(ps);

			if (szn) ps->SetName(szn);
		}
		++tag;
	} while (!tag.isend());

	// add interface to step
	pstep->AddComponent(pci);
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseContactRigid(FSStep *pstep, XMLTag &tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();
	FSMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();

	int NMAT = febio.Materials();
	for (int i = 0; i<NMAT; ++i)
	{
		GMaterial* pmat = febio.GetMaterial(i);
		if (pmat) pmat->m_ntag = 0;
	}

	const char* sz = tag.AttributeValue("name", true);
	char szname[256];
	if (sz) strcpy(szname, sz);

	for (int i = 0; i<pm->Nodes(); ++i) pm->Node(i).m_ntag = -1;

	++tag;
	do
	{
		int id = tag.Attribute("id").value<int>() - 1;
		int rb = tag.Attribute("rb").value<int>() - 1;

		if ((rb >= 0) && (rb<NMAT))
		{
			GMaterial* pmat = febio.GetMaterial(rb);

			FSNode& node = pm->Node(id);
			node.m_ntag = pmat->GetID();
			pmat->m_ntag++;
		}
		else FileReader()->AddLogEntry("Invalid material in rigid contact.");

		++tag;
	}
	while (!tag.isend());

	char szbuf[256];
	for (int i = 0; i<NMAT; ++i)
	{
		GMaterial* pmat = febio.GetMaterial(i);
		if (pmat && (pmat->m_ntag > 0))
		{
			int id = pmat->GetID();
			// create the node set
			FSNodeSet* pn = new FSNodeSet(po);
			//			pm->AddNodeSet(pn);
			sprintf(szbuf, "RigidNodeset%2d", CountInterfaces<FSRigidInterface>(fem) + 1);
			pn->SetName(szbuf);

			int NN = pm->Nodes();
			for (int j = 0; j<NN; ++j)
			{
				FSNode& node = pm->Node(j);
				if (node.m_ntag == id) pn->add(j);
			}

			// create the interface
			FSRigidInterface* pi = new FSRigidInterface(&fem, pmat, pn, pstep->GetID());
			if (sz) sprintf(szbuf, "%s", szname);
			else sprintf(szbuf, "RigidInterface%02d", i + 1);
			pi->SetName(szbuf);
			pstep->AddComponent(pi);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseContactJoint(FSStep *pstep, XMLTag &tag)
{
	FEBioInputModel& febio = GetFEBioModel();
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

	if (na >= 0) pi->m_pbodyA = febio.GetMaterial(na - 1);
	if (nb >= 0) pi->m_pbodyB = febio.GetMaterial(nb - 1);
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseConnector(FSStep *pstep, XMLTag &tag, const int rc)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	FSRigidConnector* pi = nullptr;
	char szname[256];

	switch (rc) {
	case 0:
		pi = new FSRigidSphericalJoint(&fem, pstep->GetID());
		sprintf(szname, "RigidSphericalJoint%02d", CountConnectors<FSRigidSphericalJoint>(fem) + 1);
		break;
	case 1:
		pi = new FSRigidRevoluteJoint(&fem, pstep->GetID());
		sprintf(szname, "RigidrevoluteJoint%02d", CountConnectors<FSRigidRevoluteJoint>(fem) +1);
		break;
	case 2:
		pi = new FSRigidPrismaticJoint(&fem, pstep->GetID());
		sprintf(szname, "RigidPrismaticJoint%02d", CountConnectors<FSRigidPrismaticJoint>(fem) +1);
		break;
	case 3:
		pi = new FSRigidCylindricalJoint(&fem, pstep->GetID());
		sprintf(szname, "RigidCylindricalJoint%02d", CountConnectors<FSRigidCylindricalJoint>(fem) +1);
		break;
	case 4:
		pi = new FSRigidPlanarJoint(&fem, pstep->GetID());
		sprintf(szname, "RigidPlanarJoint%02d", CountConnectors<FSRigidPlanarJoint>(fem) +1);
		break;
    case 5:
        pi = new FSRigidLock(&fem, pstep->GetID());
        sprintf(szname, "RigidLock%02d", CountConnectors<FSRigidLock>(fem) +1);
        break;
	case 6:
		pi = new FSRigidSpring(&fem, pstep->GetID());
		sprintf(szname, "RigidSpring%02d", CountConnectors<FSRigidSpring>(fem) +1);
		break;
	case 7:
		pi = new FSRigidDamper(&fem, pstep->GetID());
		sprintf(szname, "RigidDamper%02d", CountConnectors<FSRigidDamper>(fem) +1);
		break;
	case 8:
		pi = new FSRigidAngularDamper(&fem, pstep->GetID());
		sprintf(szname, "RigidAngularDamper%02d", CountConnectors<FSRigidAngularDamper>(fem) +1);
		break;
	case 9:
		pi = new FSRigidContractileForce(&fem, pstep->GetID());
		sprintf(szname, "RigidContractileForce%02d", CountConnectors<FSRigidContractileForce>(fem) +1);
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

	++tag;
	do
	{
		if (tag == "body_a") {
			tag.value(na);
			if (na >= 0) pi->SetRigidBody1(febio.GetMaterial(na - 1)->GetID());
		}
		else if (tag == "body_b") {
			tag.value(nb);
			if (nb >= 0) pi->SetRigidBody2(febio.GetMaterial(nb - 1)->GetID());
		}
		else
		{
			if (ReadParam(*pi, tag) == false) ParseUnknownTag(tag);
		}
		++tag;
	} while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseSprings(FSStep *pstep, XMLTag &tag)
{
	FEBioInputModel& febio = GetFEBioModel();

	FSModel &fem = GetFSModel();
	GModel& gm = fem.GetModel();
	GMeshObject* po = GetGObject();
	
	int n[2], lc = -1;
	double E = 0;

	int ntype = FE_DISCRETE_SPRING;
	XMLAtt* pat = tag.AttributePtr("type");
	if (pat)
	{
		if (*pat == "linear") ntype = FE_DISCRETE_SPRING;
		else if (*pat == "nonlinear") ntype = FE_GENERAL_SPRING;
	}

	++tag;
	do
	{
		if (tag == "node") tag.value(n, 2);
		else if (tag == "E") tag.value(E);
		else if (tag == "force")
		{
			tag.value(E);
			lc = tag.Attribute("lc").value<int>() - 1;
		}
		else ParseUnknownTag(tag);
		++tag;
	} while (!tag.isend());

	n[0] = po->MakeGNode(n[0] - 1);
	n[1] = po->MakeGNode(n[1] - 1);

	int N = fem.GetModel().DiscreteObjects() + 1;

	GDiscreteObject* pd = 0;
	switch (ntype)
	{
	case FE_DISCRETE_SPRING:
		{
			GLinearSpring* ps = new GLinearSpring(&gm, n[0], n[1]);
			char szname[256];
			sprintf(szname, "Spring%02d", N);
			ps->SetName(szname);
			ps->GetParam(GLinearSpring::MP_E).SetFloatValue(E);
			pd = ps;
		}
		break;
	case FE_GENERAL_SPRING:
		{
			GGeneralSpring* pg = new GGeneralSpring(&gm, n[0], n[1]);
			char szname[256];
			sprintf(szname, "Spring%02d", N);
			pg->SetName(szname);
			pg->GetParam(GGeneralSpring::MP_F).SetFloatValue(E);
			febio.AddParamCurve(&pg->GetParam(GGeneralSpring::MP_F), lc);
			pd = pg;
		}
		break;
	}

	fem.GetModel().AddDiscreteObject(pd);
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLinearConstraint(FSStep* pstep, XMLTag& tag)
{
	FSModel &fem = GetFSModel();

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
	}
	while (!tag.isend());
}

//=============================================================================
//
//                                D I S C R E T E
//
//=============================================================================

bool FEBioFormat2::ParseDiscreteSection(XMLTag& tag)
{
	++tag;
	do
	{
		if (tag == "spring") ParseSprings(m_pBCStep, tag);
		else ParseUnknownTag(tag);
		++tag;
	} while (!tag.isend());
	return true;
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseBodyForce(FSStep *pstep, XMLTag &tag)
{
	FSModel &fem = GetFSModel();

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
void FEBioFormat2::ParseHeatSource(FSStep *pstep, XMLTag &tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel &fem = GetFSModel();

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

//=============================================================================
//
//                            C O N S T R A I N T S
//
//=============================================================================

//-----------------------------------------------------------------------------
bool FEBioFormat2::ParseConstraintSection(XMLTag& tag)
{
	if (tag.isleaf()) return true;

	FSStep* pstep = m_pBCStep;

	++tag;
	do
	{
		if (tag == "rigid_body") ParseRigidConstraint(pstep, tag);
		else if (tag == "constraint")
		{
			const char* sztype = tag.AttributeValue("type");
			if      (strcmp(sztype, "volume"                 ) == 0) ParseVolumeConstraint(pstep, tag);
			else if (strcmp(sztype, "symmetry plane"         ) == 0) ParseSymmetryPlane   (pstep, tag);
            else if (strcmp(sztype, "normal fluid flow"      ) == 0) ParseNrmlFldVlctSrf  (pstep, tag);
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
			else if (strcmp(sztype, "rigid joint"            ) == 0) ParseContactJoint(pstep, tag);
			else if (strcmp(sztype, "warp-image"             ) == 0) ParseConstraint(pstep, tag);
			else ParseUnknownTag(tag);
		}
		else ParseUnknownTag(tag);
		++tag;
	} while (!tag.isend());

	return true;
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseRigidConstraint(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// get the material ID
	int nid = tag.Attribute("mat").value<int>() - 1;

	// get the rigid material
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
		if (tag == "initial_velocity")
		{
			FSRigidVelocity* pv = new FSRigidVelocity(&fem, pstep->GetID());
			vec3d vi;
			tag.value(vi);
			pv->SetVelocity(vi);
			pv->SetMaterialID(matid);

			static int n = 1;
			if (hasName == false) sprintf(szname, "RigidVelocity%02d", n++);
			pv->SetName(szname);
			pstep->AddRC(pv);
		}
		else if (tag == "initial_angular_velocity")
		{
			FSRigidAngularVelocity* pv = new FSRigidAngularVelocity(&fem, pstep->GetID());
			vec3d vi;
			tag.value(vi);
			pv->SetVelocity(vi);
			pv->SetMaterialID(matid);

			static int n = 1;
			if (hasName == false) sprintf(szname, "RigiAngulardVelocity%02d", n++);
			pv->SetName(szname);
			pstep->AddRC(pv);
		}
		else {
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
				if (pc == 0) pc = new FSRigidFixed(&fem, pstep->GetID());
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
				if (lc > 0) febio.AddParamCurve(&pd->GetParam(FSRigidDisplacement::VALUE), lc - 1);
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
					if (strcmp(sztype, "ramp"  ) == 0) pf->SetForceType(2);
				}

				static int n = 1;
				if (hasName == false) sprintf(szname, "RigidForce%02d", n++);
				pf->SetName(szname);
				pstep->AddRC(pf);
				if (lc > 0) febio.AddParamCurve(&pf->GetParam(FSRigidDisplacement::VALUE), lc - 1);
			}
			else ParseUnknownTag(tag);
		}

		++tag;
	} while (!tag.isend());

	if (pc)
	{
		static int n = 1;
		pc->SetMaterialID(pgm ? pgm->GetID() : -1);
		if (hasName == false) sprintf(szname, "RigidFixed%02d", n++);
		pc->SetName(szname);
		pstep->AddRC(pc);
	}
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseVolumeConstraint(FSStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();

	// make sure there is something to read
	if (tag.isempty()) return;

	// create a new volume constraint
	FSVolumeConstraint* pi = new FSVolumeConstraint(&fem, pstep->GetID());
	pstep->AddComponent(pi);

	// get the (optional) contact name
	char szbuf[256];
	const char* szname = tag.AttributeValue("name", true);
	if (szname) sprintf(szbuf, "%s", szname);
	else sprintf(szbuf, "VolumeConstraint%02d", CountConstraints<FSVolumeConstraint>(fem) + 1);
	pi->SetName(szbuf);

	// get the mesh (need it for defining the surface)
	FSMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();

	// read parameters
	++tag;
	do
	{
		// try to read the parameters
		if (ReadParam(*pi, tag) == false)
		{
			// read the surface definition
			if (tag == "surface")
			{
				// get the (optional) name
				// if no name is provided a default one will be provided
				const char* szn = tag.AttributeValue("name", true);

				// create a new surface
				FSSurface* ps = ParseContactSurface(tag);
				sprintf(szbuf, "VolumeConstraintSurface%02d", CountConstraints<FSVolumeConstraint>(fem));
				ps->SetName(szn ? szn : szbuf);

				// assign the surface
				pi->SetItemList(ps);
			}
			else ParseUnknownTag(tag);
		}

		// go to the next tag
		++tag;
	} while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseSymmetryPlane(FSStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();

	// make sure there is something to read
	if (tag.isempty()) return;

	// create a new volume constraint
	FSSymmetryPlane* pi = new FSSymmetryPlane(&fem, pstep->GetID());
	pstep->AddComponent(pi);

	// get the (optional) contact name
	char szbuf[256];
	const char* szname = tag.AttributeValue("name", true);
	if (szname) sprintf(szbuf, "%s", szname);
	else sprintf(szbuf, "SymmetrtPlane%02d", CountConstraints<FSSymmetryPlane>(fem) +1);
	pi->SetName(szbuf);

	// read parameters
	++tag;
	do
	{
		// try to read the parameters
		if (ReadParam(*pi, tag) == false)
		{
			// read the surface definition
			if (tag == "surface")
			{
				// get the (optional) name
				// if no name is provided a default one will be provided
				const char* szn = tag.AttributeValue("name", true);

				// create a new surface
				FSSurface* ps = ParseContactSurface(tag);
				sprintf(szbuf, "SymmetryPlaneSurface%02d", CountConstraints<FSSymmetryPlane>(fem));
				ps->SetName(szn ? szn : szbuf);

				// assign the surface
				pi->SetItemList(ps);
			}
			else ParseUnknownTag(tag);
		}

		// go to the next tag
		++tag;
	} while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseNrmlFldVlctSrf(FSStep* pstep, XMLTag& tag)
{
    FSModel& fem = GetFSModel();
    
    // make sure there is something to read
    if (tag.isempty()) return;
    
    // create a new volume constraint
    FSNormalFlowSurface* pi = new FSNormalFlowSurface(&fem, pstep->GetID());
    pstep->AddComponent(pi);
    
    // get the (optional) contact name
    char szbuf[256];
    const char* szname = tag.AttributeValue("name", true);
    if (szname) sprintf(szbuf, "%s", szname);
    else sprintf(szbuf, "NormalFlowSurface%02d", CountConstraints<FSNormalFlowSurface>(fem) +1);
    pi->SetName(szbuf);
    
    // read parameters
    ++tag;
    do
    {
        // try to read the parameters
        if (ReadParam(*pi, tag) == false)
        {
            // read the surface definition
            if (tag == "surface")
            {
                // get the (optional) name
                // if no name is provided a default one will be provided
                const char* szn = tag.AttributeValue("name", true);
                
                // create a new surface
                FSSurface* ps = ParseContactSurface(tag);
                sprintf(szbuf, "NormalFlowSurface%02d", CountConstraints<FSNormalFlowSurface>(fem));
                ps->SetName(szn ? szn : szbuf);
                
                // assign the surface
                pi->SetItemList(ps);
            }
            else ParseUnknownTag(tag);
        }
        
        // go to the next tag
        ++tag;
    } while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseFrictionlessFluidWall(FSStep* pstep, XMLTag& tag)
{
    FSModel& fem = GetFSModel();

    // make sure there is something to read
    if (tag.isempty()) return;

    // create a new volume constraint
    FSFrictionlessFluidWall* pi = new FSFrictionlessFluidWall(&fem, pstep->GetID());
    pstep->AddComponent(pi);

    // get the (optional) contact name
    char szbuf[256];
    const char* szname = tag.AttributeValue("name", true);
    if (szname) sprintf(szbuf, "%s", szname);
    else sprintf(szbuf, "FrictionlessFluidWall%02d", CountConstraints<FSFrictionlessFluidWall>(fem) +1);
    pi->SetName(szbuf);

    // read parameters
    ++tag;
    do
    {
        // try to read the parameters
        if (ReadParam(*pi, tag) == false)
        {
            // read the surface definition
            if (tag == "surface")
            {
                // get the (optional) name
                // if no name is provided a default one will be provided
                const char* szn = tag.AttributeValue("name", true);

                // create a new surface
                FSSurface* ps = ParseContactSurface(tag);
                sprintf(szbuf, "FrictionlessFluidWall%02d", CountConstraints<FSFrictionlessFluidWall>(fem));
                ps->SetName(szn ? szn : szbuf);

                // assign the surface
                pi->SetItemList(ps);
            }
            else ParseUnknownTag(tag);
        }

        // go to the next tag
        ++tag;
    } while (!tag.isend());
}

//=============================================================================
//
//                                S T E P
//
//=============================================================================

//-----------------------------------------------------------------------------
bool FEBioFormat2::ParseStepSection(XMLTag &tag)
{
	char szname[128] = { 0 };
	const char* szval = tag.AttributeValue("name", true);
	if (szval) strcpy(szname, szval);

	++tag;

	// See if the file defines a Module section
	// This must be the first section in the step
	if (tag == "Module")
	{
		ParseModuleSection(tag);
		++tag;
	}

	// If not, we assume that the analysis type has not changed.
	FSModel& fem = GetFSModel();
	if (m_pstep == 0) m_pstep = NewStep(fem, m_nAnalysis, szname);
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

	m_pstep = 0;
	m_pBCStep = 0;

	return true;
}
