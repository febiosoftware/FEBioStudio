#include "stdafx.h"
#include "FEBioFormat2.h"
#include <FEMLib/FERigidConstraint.h>
#include <GeomLib/GMeshObject.h>
#include <FEMLib/FEInitialCondition.h>
#include <FEMLib/FEBodyLoad.h>
#include <FEMLib/FEModelConstraint.h>
#include <MeshTools/GDiscreteObject.h>

FEBioFormat2::FEBioFormat2(FEBioImport* fileReader, FEBioModel& febio) : FEBioFormat(fileReader, febio)
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
	else if (tag == "Geometry"   ) ParseGeometrySection  (tag);
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
	return (m_nAnalysis != -1);
}

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
	FEBioModel& febio = GetFEBioModel();
	if (febio.Parts() != 0) throw XMLReader::InvalidTag(tag);

	// Add the one-and-only part
	FEBioModel::Part& part = *febio.AddPart("Object01");

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
	FEBioModel::PartInstance* instance = new FEBioModel::PartInstance(&part);
	febio.AddInstance(instance);

	// don't forget to update the geometry
	febio.UpdateGeometry();

	return true;
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseGeometryNodes(FEBioModel::Part& part, XMLTag& tag)
{
	// first we need to figure out how many nodes there are
	int nn = tag.children();

	// create nodes
	FEMesh* pm = part.GetFEMesh();
	int N0 = pm->Nodes();
	pm->Create(N0 + nn, 0);

	// read nodal coordinates
	++tag;
	for (int i = 0; i<nn; ++i)
	{
		FENode& node = pm->Node(N0 + i);
		tag.value(node.r);
		++tag;
	}
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseGeometryElements(FEBioModel::Part& part, XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();

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
	FEBioModel::Domain* p = part.AddDomain(szname, matID);

	// create elements
	FEMesh* pm = part.GetFEMesh();
	int NTE = pm->Elements();
	pm->Create(0, elems + NTE);

	// generate the part id
	int pid = part.Domains() - 1;

	// read element data
	++tag;
	int n[FEElement::MAX_NODES];
	for (int i = NTE; i<elems + NTE; ++i)
	{
		FEElement& el = pm->Element(i);
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
void FEBioFormat2::ParseGeometryElementData(FEBioModel::Part& part, XMLTag& tag)
{
	FEMesh* pm = part.GetFEMesh();

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
			FEElement& el = pm->Element(id);

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
						pdata = pm->FindElementDataField(dataName);

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
void FEBioFormat2::ParseGeometryNodeSet(FEBioModel::Part& part, XMLTag& tag)
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
	part.AddNodeSet(FEBioModel::NodeSet(name, list));
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseGeometrySurface(FEBioModel::Part& part, XMLTag& tag)
{
	// get the name
	const char* szname = tag.AttributeValue("name");

	// see if a surface with this name is already defined
	// if found, we'll continue, but we'll generate a warning.
	FEBioModel::Surface* ps = part.FindSurface(szname);
	if (ps) FileReader()->AddLogEntry("A surface named %s is already defined.", szname);

	// create a new surface
	FEBioModel::Surface s;
	s.m_name = szname;

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
void FEBioFormat2::ParseBCFixed(FEStep* pstep, XMLTag &tag)
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
	FEMesh* pm = &GetFEMesh();

	// create a nodeset for this BC
	FEBioModel& febio = GetFEBioModel();
	FEBioModel::PartInstance& part = GetInstance();
	GMeshObject* po = GetGObject();
	FENodeSet* pg = 0;

	// see if the set attribute is defined
	const char* szset = tag.AttributeValue("set", true);
	if (szset)
	{
		// see if we can find the nodeset
		pg = part.BuildFENodeSet(szset);

		// make sure the set is found
		if (pg == 0) throw XMLReader::InvalidAttributeValue(tag, "set", szset);
	}
	else
	{
		pg = new FENodeSet(po);
	}

	// create the constraint
	FEModel& fem = GetFEModel();
	char szname[256] = {0};
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

	// read the node list
	if (tag.isleaf() == false)
	{
		++tag;
		do
		{
			// get the node ID
			int n = tag.Attribute("id").value<int>() - 1;

			// assign the node to this group
			pg->add(n);

			++tag;
		} 
		while (!tag.isend());
	}
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseBCPrescribed(FEStep* pstep, XMLTag& tag)
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
	FEBioModel& febio = GetFEBioModel();
	FEBioModel::PartInstance& part = GetInstance();
	GMeshObject* po = GetGObject();
	FEMesh* pm = &GetFEMesh();
	FENodeSet* pg = 0;
	char szname[256] = { 0 };

	// see if the set attribute is defined
	const char* szset = tag.AttributeValue("set", true);
	if (szset)
	{
		// make sure this is a leaf
		if (tag.isleaf() == false) throw XMLReader::InvalidValue(tag);

		// see if we can find the nodeset
		pg = part.BuildFENodeSet(szset);

		// make sure the set is found
		if (pg == 0) throw XMLReader::InvalidAttributeValue(tag, "set", szset);
	}
	else
	{
		pg = new FENodeSet(po);
	}

	// make a new boundary condition
	FEModel& fem = GetFEModel();
	FEPrescribedDOF* pbc = 0;
	switch (bc)
	{
	case 0:
	case 1:
	case 2:
		{
			FEPrescribedDisplacement* pd = new FEPrescribedDisplacement(&fem, pg, bc, 1, pstep->GetID());
			pd->SetRelativeFlag(brel);
			febio.AddParamCurve(pd->GetLoadCurve(), lc);
			pbc = pd;
		}
		break;
	case 3:
		{
			  FEPrescribedTemperature* pd = new FEPrescribedTemperature(&fem, pg, bc, pstep->GetID());
			  pd->SetRelativeFlag(brel);
			  febio.AddParamCurve(pd->GetLoadCurve(), lc);
			  pbc = pd;
		}
		break;
	case 4:
		{
			  FEPrescribedFluidPressure* pd = new FEPrescribedFluidPressure(&fem, pg, 1, pstep->GetID());
			  pd->SetRelativeFlag(brel);
			  febio.AddParamCurve(pd->GetLoadCurve(), lc);
			  pbc = pd;
		}
		break;
	case 5:
	case 6:
	case 7:
		{
			  bc = bc - 5;
			  FEPrescribedFluidVelocity* pd = new FEPrescribedFluidVelocity(&fem, pg, bc, 1, pstep->GetID());
			  pd->SetRelativeFlag(brel);
			  febio.AddParamCurve(pd->GetLoadCurve(), lc);
			  pbc = pd;
		}
		break;
	case 8:
		{
			  FEPrescribedFluidDilatation* pd = new FEPrescribedFluidDilatation(&fem, pg, 1, pstep->GetID());
			  pd->SetRelativeFlag(brel);
			  febio.AddParamCurve(pd->GetLoadCurve(), lc);
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
			   FEPrescribedConcentration* pd = new FEPrescribedConcentration(&fem, pg, bc, 1.0, pstep->GetID());
			   pd->SetRelativeFlag(brel);
			   febio.AddParamCurve(pd->GetLoadCurve(), lc);
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
void FEBioFormat2::ParseNodeLoad(FEStep* pstep, XMLTag& tag)
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

	FEBioModel& febio = GetFEBioModel();
	FEBioModel::PartInstance& part = GetInstance();
	GMeshObject* po = part.GetGObject();

	// create the node set
	FEMesh* pm = &GetFEMesh();
	char szname[256];
	FENodeSet* pg = new FENodeSet(po);

	static int n = 1;
	sprintf(szname, "ForceNodeset%02d", n++);
	pg->SetName(szname);

	// create the nodal load
	FEModel& fem = GetFEModel();
	FENodalLoad* pbc = new FENodalLoad(&fem, pg, bc, 1, pstep->GetID());
	sprintf(szname, "ForceLoad%02d", CountLoads<FENodalLoad>(fem)+1);
	pbc->SetName(szname);
	febio.AddParamCurve(pbc->GetLoadCurve(), lc);
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
void FEBioFormat2::ParseSurfaceLoad(FEStep* pstep, XMLTag& tag)
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
FESurface* FEBioFormat2::ParseLoadSurface(XMLTag& tag)
{
	// create a new surface
	FEBioModel::PartInstance& part = GetInstance();

	// see if the set is defined 
	if (tag.isempty())
	{
		// get the surface name
		const char* szset = tag.AttributeValue("set");

		// find the surface
		FESurface* ps = part.BuildFESurface(szset);
		if (ps == 0) throw XMLReader::InvalidAttributeValue(tag, "set", szset);

		return ps;
	}
	else
	{
		FEBioModel::Surface surf;
		// read the pressure data
		int nf[FEElement::MAX_NODES], N;
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

		FESurface* ps = part.BuildFESurface(surf);

		return ps;
	}
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadPressure(FEStep* pstep, XMLTag& tag)
{
	// create a new surface load
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();
	FEPressureLoad* pbc = new FEPressureLoad(&fem);
	pstep->AddComponent(pbc);

	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "PressureLoad%d", CountLoads<FEPressureLoad>(fem));
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
			febio.AddParamCurve(pbc->GetLoadCurve(), lc);
			double s; tag.value(s);
			pbc->SetLoad(s);
		}
		else if (tag == "surface")
		{
			FESurface* psurf = ParseLoadSurface(tag);
			pbc->SetItemList(psurf);
		}
		++tag;
	} while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadTraction(FEStep* pstep, XMLTag& tag)
{
	// create a new surface load
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();
	FESurfaceTraction* pbc = new FESurfaceTraction(&fem);
	pstep->AddComponent(pbc);

	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "TractionLoad%d", CountLoads<FESurfaceTraction>(fem));
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
		else if (tag == "surface")
		{
			FESurface* psurf = ParseLoadSurface(tag);
			pbc->SetItemList(psurf);
		}
		++tag;
	}
	while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadFluidTraction(FEStep* pstep, XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();

	// create a new surface load
	FEModel& fem = GetFEModel();
	FEFluidTraction* pbc = new FEFluidTraction(&fem);
	pstep->AddComponent(pbc);

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
		else if (tag == "surface")
		{
			FESurface* psurf = ParseLoadSurface(tag);
			pbc->SetItemList(psurf);
		}
		++tag;
	} 
	while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadFluidVelocity(FEStep* pstep, XMLTag& tag)
{
    FEBioModel& febio = GetFEBioModel();
    // create a new surface load
    FEModel& fem = GetFEModel();
    FEFluidVelocity* pbc = new FEFluidVelocity(&fem);
    pstep->AddComponent(pbc);
    
    // set the name
    char szname[256] = { 0 };
    sprintf(szname, "FluidVelocity%d", CountLoads<FEFluidVelocity>(fem));
    pbc->SetName(szname);
    
    // read the parameters
    ++tag;
    do
    {
        if (tag == "scale")
        {
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(pbc->GetLoadCurve(), lc);
        }
        else if (tag == "velocity")
        {
            vec3d t; tag.value(t);
            pbc->SetLoad(t);
        }
        else if (tag == "surface")
        {
            FESurface* psurf = ParseLoadSurface(tag);
            pbc->SetItemList(psurf);
        }
        ++tag;
    } while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadFluidNormalVelocity(FEStep* pstep, XMLTag& tag)
{
    FEBioModel& febio = GetFEBioModel();
    // create a new surface load
    FEModel& fem = GetFEModel();
    FEFluidNormalVelocity* pbc = new FEFluidNormalVelocity(&fem);
    pstep->AddComponent(pbc);
    
    // set the name
    char szname[256] = { 0 };
    sprintf(szname, "FluidNormalVelocity%d", CountLoads<FEFluidNormalVelocity>(fem));
    pbc->SetName(szname);
    
    // read the parameters
    ++tag;
    do
    {
        if (tag == "velocity")
        {
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(pbc->GetLoadCurve(), lc);
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
        else if (tag == "surface")
        {
            FESurface* psurf = ParseLoadSurface(tag);
            pbc->SetItemList(psurf);
        }
        ++tag;
    } while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadFluidRotationalVelocity(FEStep* pstep, XMLTag& tag)
{
    FEBioModel& febio = GetFEBioModel();
    // create a new surface load
    FEModel& fem = GetFEModel();
    FEFluidRotationalVelocity* pbc = new FEFluidRotationalVelocity(&fem);
    pstep->AddComponent(pbc);
    
    // set the name
    char szname[256] = { 0 };
    sprintf(szname, "FluidRotationalVelocity%d", CountLoads<FEFluidRotationalVelocity>(fem));
    pbc->SetName(szname);
    
    // read the parameters
    ++tag;
    do
    {
        if (tag == "angular_speed")
        {
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(pbc->GetLoadCurve(), lc);
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
            FESurface* psurf = ParseLoadSurface(tag);
            pbc->SetItemList(psurf);
        }
        ++tag;
    } while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadFluidFlowResistance(FEStep* pstep, XMLTag& tag)
{
    FEBioModel& febio = GetFEBioModel();
    // create a new surface load
    FEModel& fem = GetFEModel();
    FEFluidFlowResistance* pbc = new FEFluidFlowResistance(&fem);
    pstep->AddComponent(pbc);
    
    // set the name
    char szname[256] = { 0 };
    sprintf(szname, "FluidResistance%d", CountLoads<FEFluidFlowResistance>(fem));
    pbc->SetName(szname);
    
    // read the parameters
    ++tag;
    do
    {
        if (tag == "R")
        {
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(pbc->GetLoadCurve(), lc);
            double s; tag.value(s);
            pbc->SetLoad(s);
        }
        else if (tag == "pressure_offset")
        {
			int lc = tag.AttributeValue<int>("lc", 0) - 1;
			if (lc >= 0)
			{
				febio.AddParamCurve(pbc->GetPOLoadCurve(), lc);
			}
			double s; tag.value(s);
            pbc->SetPO(s);
        }
        else if (tag == "surface")
        {
            FESurface* psurf = ParseLoadSurface(tag);
            pbc->SetItemList(psurf);
        }
        ++tag;
    } while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadFluidBackflowStabilization(FEStep* pstep, XMLTag& tag)
{
    FEBioModel& febio = GetFEBioModel();
    // create a new surface load
    FEModel& fem = GetFEModel();
    FEFluidBackflowStabilization* pbc = new FEFluidBackflowStabilization(&fem);
    pstep->AddComponent(pbc);
    
    // set the name
    char szname[256] = { 0 };
    sprintf(szname, "BackflowStabilization%d", CountLoads<FEFluidBackflowStabilization>(fem));
    pbc->SetName(szname);
    
    // read the parameters
    ++tag;
    do
    {
        if (tag == "beta")
        {
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(pbc->GetLoadCurve(), lc);
            double s; tag.value(s);
            pbc->SetLoad(s);
        }
        else if (tag == "surface")
        {
            FESurface* psurf = ParseLoadSurface(tag);
            pbc->SetItemList(psurf);
        }
        ++tag;
    } while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadFluidTangentialStabilization(FEStep* pstep, XMLTag& tag)
{
    FEBioModel& febio = GetFEBioModel();
    // create a new surface load
    FEModel& fem = GetFEModel();
    FEFluidTangentialStabilization* pbc = new FEFluidTangentialStabilization(&fem);
    pstep->AddComponent(pbc);
    
    // set the name
    char szname[256] = { 0 };
    sprintf(szname, "TangentialStabilization%d", CountLoads<FEFluidTangentialStabilization>(fem));
    pbc->SetName(szname);
    
    // read the parameters
    ++tag;
    do
    {
        if (tag == "beta")
        {
            int lc = tag.Attribute("lc").value<int>() - 1;
            febio.AddParamCurve(pbc->GetLoadCurve(), lc);
            double s; tag.value(s);
            pbc->SetLoad(s);
        }
        else if (tag == "surface")
        {
            FESurface* psurf = ParseLoadSurface(tag);
            pbc->SetItemList(psurf);
        }
        ++tag;
    } while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadFSITraction(FEStep* pstep, XMLTag& tag)
{
    FEBioModel& febio = GetFEBioModel();
    // create a new surface load
    FEModel& fem = GetFEModel();
    FEFSITraction* pbc = new FEFSITraction(&fem);
    pstep->AddComponent(pbc);
    
    // set the name
    char szname[256] = { 0 };
    sprintf(szname, "FSInterfaceTraction%d", CountLoads<FEFSITraction>(fem));
    pbc->SetName(szname);
    
    // read the parameters
    ++tag;
    do
    {
        if (tag == "surface")
        {
            FESurface* psurf = ParseLoadSurface(tag);
            pbc->SetItemList(psurf);
        }
        ++tag;
    } while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadFluidFlux(FEStep* pstep, XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	// create a new surface load
	FEFluidFlux* pbc = new FEFluidFlux(&fem);
	pstep->AddComponent(pbc);

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
		else if (tag == "surface")
		{
			FESurface* psurf = ParseLoadSurface(tag);
			pbc->SetItemList(psurf);
		}
		++tag;
	} while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadSoluteFlux(FEStep* pstep, XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();

	FEModel& fem = GetFEModel();

	// create a new surface load
	FESoluteFlux* pbc = new FESoluteFlux(&fem);
	pstep->AddComponent(pbc);

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
			int lc = tag.Attribute("lc").value<int>() - 1;
			febio.AddParamCurve(pbc->GetLoadCurve(), lc);
			double s; tag.value(s);
			pbc->SetLoad(s);
		}
		else if (tag == "surface")
		{
			FESurface* psurf = ParseLoadSurface(tag);
			pbc->SetItemList(psurf);
		}
		++tag;
	} while (!tag.isend());
}


//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadConcentrationFlux(FEStep* pstep, XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();

	FEModel& fem = GetFEModel();

	// create a new surface load
	FEConcentrationFlux* pcf = new FEConcentrationFlux(&fem);
	pstep->AddComponent(pcf);

	// set the name
	char szname[256] = { 0 };
	sprintf(szname, "ConcentrationFlux%d", CountLoads<FEConcentrationFlux>(fem));
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
			febio.AddParamCurve(pcf->GetLoadCurve(), lc);
			double s; tag.value(s);
			pcf->SetFlux(s);
		}
		else if (tag == "surface")
		{
			FESurface* psurf = ParseLoadSurface(tag);
			pcf->SetItemList(psurf);
		}
		++tag;
	}
	while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadNormalTraction(FEStep* pstep, XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();

	FEModel& fem = GetFEModel();

	// create a new surface load
	FEBPNormalTraction* pbc = new FEBPNormalTraction(&fem);
	pstep->AddComponent(pbc);
	pbc->SetStep(pstep->GetID());

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
		else if (tag == "effective")
		{
			bool b; tag.value(b);
			pbc->SetMixtureFlag(b);
		}
		else if (tag == "traction")
		{
			int lc = tag.Attribute("lc").value<int>() - 1;
			febio.AddParamCurve(pbc->GetLoadCurve(), lc);
			double s; tag.value(s);
			pbc->SetLoad(s);
		}
		else if (tag == "surface")
		{
			FESurface* psurf = ParseLoadSurface(tag);
			pbc->SetItemList(psurf);
		}
		++tag;
	} while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadHeatFlux(FEStep* pstep, XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();

	FEModel& fem = GetFEModel();

	// create a new surface load
	FEHeatFlux* pbc = new FEHeatFlux(&fem);
	pstep->AddComponent(pbc);

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
		else if (tag == "surface")
		{
			FESurface* psurf = ParseLoadSurface(tag);
			pbc->SetItemList(psurf);
		}
		++tag;
	} while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseLoadConvectiveHeatFlux(FEStep* pstep, XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();

	FEModel& fem = GetFEModel();

	// create a new surface load
	FEConvectiveHeatFlux* pbc = new FEConvectiveHeatFlux(&fem);
	pstep->AddComponent(pbc);

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
		else if (tag == "surface")
		{
			FESurface* psurf = ParseLoadSurface(tag);
			pbc->SetItemList(psurf);
		}
		++tag;
	} while (!tag.isend());
}

//-----------------------------------------------------------------------------
//! Parses the body_load section.
void FEBioFormat2::ParseBodyLoad(FEStep* pstep, XMLTag& tag)
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
	FEModel& fem = GetFEModel();
	FEMesh* pm = &GetFEMesh();

	++tag;
	do
	{
		if (tag == "velocity")	// initial velocity BC
		{
			FENodeSet* pg = 0;
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
					if (pg == 0) pg = new FENodeSet(po);
					pg->add(id);
				}
				else ParseUnknownTag(tag);
				++tag;
			} while (!tag.isend());

			// create a new initial velocity BC
			FENodalVelocities* pbc = new FENodalVelocities(&fem, pg, v, m_pBCStep->GetID());
			char szname[64] = { 0 };
			sprintf(szname, "InitialVelocity%02d", CountICs<FENodalVelocities>(fem)+1);
			pbc->SetName(szname);
			m_pBCStep->AddComponent(pbc);
		}
		else if (tag == "concentration")	// initial concentration BC
		{
			FENodeSet* pg = 0;
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
					if (pg == 0) pg = new FENodeSet(po);
					pg->add(id);
				}
				else ParseUnknownTag(tag);
				++tag;
			} while (!tag.isend());

			// create a new initial concentration BC
			FEInitConcentration* pbc = new FEInitConcentration(&fem, pg, bc, c, m_pBCStep->GetID());
			char szname[64] = { 0 };
			sprintf(szname, "InitialConcentration%02d", CountICs<FEInitConcentration>(fem)+1);
			pbc->SetName(szname);
			m_pBCStep->AddComponent(pbc);
		}
		else if (tag == "fluid_pressure")	// initial fluid pressure
		{
			FENodeSet* pg = 0;
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
					if (pg == 0) pg = new FENodeSet(po);
					pg->add(id);
				}
				else ParseUnknownTag(tag);
				++tag;
			} while (!tag.isend());

			// create a new initial BC
			FEInitFluidPressure* pbc = new FEInitFluidPressure(&fem, pg, p, m_pBCStep->GetID());
			char szname[64] = { 0 };
			sprintf(szname, "InitialFluidPressure%02d", CountICs<FEInitFluidPressure>(fem)+1);
			pbc->SetName(szname);
			m_pBCStep->AddComponent(pbc);
		}
		else if (tag == "temperature")	// initial temperature
		{
			FENodeSet* pg = 0;
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
					if (pg == 0) pg = new FENodeSet(po);
					pg->add(id);
				}
				else ParseUnknownTag(tag);
				++tag;
			} while (!tag.isend());

			// create a new initial BC
			FEInitTemperature* pbc = new FEInitTemperature(&fem, pg, p, m_pBCStep->GetID());
			char szname[64] = { 0 };
			sprintf(szname, "InitialTemperature%02d", CountICs<FEInitTemperature>(fem)+1);
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
void FEBioFormat2::ParseContact(FEStep *pstep, XMLTag &tag)
{
	XMLAtt& atype = tag.Attribute("type");
	if      (atype == "sliding-node-on-facet"  ) ParseContactSliding    (pstep, tag);
	else if (atype == "sliding-facet-on-facet" ) ParseContactF2FSliding (pstep, tag);
	else if (atype == "sliding-biphasic"       ) ParseContactBiphasic   (pstep, tag);
	else if (atype == "sliding-biphasic-solute") ParseContactSolute     (pstep, tag);
	else if (atype == "sliding-multiphasic"    ) ParseContactMultiphasic(pstep, tag);
	else if (atype == "tied"                   ) ParseContactTied       (pstep, tag);
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
FESurface* FEBioFormat2::ParseContactSurface(XMLTag& tag, int format)
{
	FEBioModel& febio = GetFEBioModel();
	FEBioModel::PartInstance& part = GetInstance();
	GMeshObject* po = part.GetGObject();

	// see if the set is defined 
	if (tag.isempty())
	{
		// get the surface name
		const char* szset = tag.AttributeValue("set");

		// find the surface
		FESurface* psurf = part.BuildFESurface(szset);
		if (psurf == 0) throw XMLReader::InvalidAttributeValue(tag, "set", szset);

		return psurf;
	}
	else
	{
		FEBioModel::Surface surf;

		if (format == 0)
		{
			int nf[FEFace::MAX_NODES], N;

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

				FEElement* el = mesh.FindElementFromID(n[0]); assert(el);
				if (el)
				{
					int nf = el->Faces();
					if ((n[1] >= 1) && (n[1] <= nf))
					{
						FEFace f = el->GetFace(n[1] - 1);
						vector<int> node(f.Nodes());
						for (int i=0; i<f.Nodes(); ++i) node[i] = f.n[i];
						surf.m_face.push_back(node);
					}
				}

				++tag;
			}
			while (!tag.isend());
		}

		FEBioModel& febio = GetFEBioModel();
		FESurface *psurf = part.BuildFESurface(surf);
		return psurf;
	}
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseContactParams(XMLTag& tag, FEPairedInterface* pi, int nid)
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
				FESurface* ps = ParseContactSurface(tag, nfmt);
				if (ntype == 1)
				{
					sprintf(szbuf, "MasterSurface%02d", nid);
					ps->SetName(szn ? szn : szbuf);
					pi->SetMaster(ps);
				}
				else
				{
					sprintf(szbuf, "SlaveSurface%02d", nid);
					ps->SetName(szn ? szn : szbuf);
					pi->SetSlave(ps);
				}
			}
			else ParseUnknownTag(tag);
		}
		++tag;
	}
	while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseContactSliding(FEStep* pstep, XMLTag& tag)
{
	char szbuf[256];

	FEModel& fem = GetFEModel();

	// create new sliding interface
	FESlidingWithGapsInterface* pi = new FESlidingWithGapsInterface(&fem, pstep->GetID());

	int nid = CountInterfaces<FESlidingWithGapsInterface>(fem) +1;

	// get the (optional) contact name
	const char* szname = tag.AttributeValue("name", true);
	if (szname) sprintf(szbuf, "%s", szname);
	else sprintf(szbuf, "SlidingInterface%02d", nid);
	pi->SetName(szbuf);

	FESurface *pms = 0, *pss = 0;
	FEMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();

	ParseContactParams(tag, pi, nid);
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseContactF2FSliding(FEStep* pstep, XMLTag& tag)
{
	char szbuf[256];

	FEModel& fem = GetFEModel();

	// create new sliding interface
	FEFacetOnFacetInterface* pi = new FEFacetOnFacetInterface(&fem, pstep->GetID());

	int nid = CountInterfaces<FEFacetOnFacetInterface>(fem) +1;

	// get the (optional) contact name
	const char* szname = tag.AttributeValue("name", true);
	if (szname) sprintf(szbuf, "%s", szname);
	else sprintf(szbuf, "SlidingContact%02d", nid);
	pi->SetName(szbuf);

	ParseContactParams(tag, pi, nid);
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseContactBiphasic(FEStep* pstep, XMLTag& tag)
{
	FEModel& fem = GetFEModel();

	// create new contact interface
	FEPoroContact* pi = new FEPoroContact(&fem, pstep->GetID());

	int nid = CountInterfaces<FEPoroContact>(fem)+1;

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
void FEBioFormat2::ParseContactSolute(FEStep* pstep, XMLTag& tag)
{
	FEModel& fem = GetFEModel();
	FEMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();

	// create new interface
	FEPoroSoluteContact* pi = new FEPoroSoluteContact(&fem, pstep->GetID());

	int nid = CountInterfaces<FEPoroSoluteContact>(fem) +1;

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
void FEBioFormat2::ParseContactMultiphasic(FEStep* pstep, XMLTag& tag)
{
	FEModel& fem = GetFEModel();
	FEMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();

	// create new interface
	FEMultiphasicContact* pi = new FEMultiphasicContact(&fem, pstep->GetID());

	// set name
	int nid = CountInterfaces<FEMultiphasicContact>(fem) +1;
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
void FEBioFormat2::ParseContactTiedMultiphasic(FEStep* pstep, XMLTag& tag)
{
	FEModel& fem = GetFEModel();
	FEMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();

	// create new interface
	FETiedMultiphasicInterface* pi = new FETiedMultiphasicInterface(&fem, pstep->GetID());

	// set name
	int nid = CountInterfaces<FETiedMultiphasicInterface>(fem) +1;
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
void FEBioFormat2::ParseContactTied(FEStep *pstep, XMLTag &tag)
{
	FEModel& fem = GetFEModel();

	// create new interface
	FETiedInterface* pi = new FETiedInterface(&fem, pstep->GetID());

	// set name
	int nid = CountInterfaces<FETiedInterface>(fem) +1;
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
void FEBioFormat2::ParseContactTiedElastic(FEStep* pstep, XMLTag& tag)
{
	FEModel& fem = GetFEModel();

	// create new interface
	FETiedElasticInterface* pi = new FETiedElasticInterface(&fem, pstep->GetID());

	// set name
	int nid = CountInterfaces<FETiedElasticInterface>(fem) +1;
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
void FEBioFormat2::ParseContactSticky(FEStep *pstep, XMLTag &tag)
{
	FEModel& fem = GetFEModel();
	FEMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();

	// create new interface
	FEStickyInterface* pi = new FEStickyInterface(&fem, pstep->GetID());

	// set name
	int nid = CountInterfaces<FEStickyInterface>(fem) +1;
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
void FEBioFormat2::ParseContactPeriodic(FEStep *pstep, XMLTag &tag)
{
	FEModel& fem = GetFEModel();
	FEMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();

	// create new interface
	FEPeriodicBoundary* pi = new FEPeriodicBoundary(&fem, pstep->GetID());

	// set name
	int nid = CountInterfaces<FEPeriodicBoundary>(fem) +1;
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
void FEBioFormat2::ParseContactTC(FEStep *pstep, XMLTag &tag)
{
	FEModel& fem = GetFEModel();
	FEMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();

	// create new interface
	FETensionCompressionInterface* pi = new FETensionCompressionInterface(&fem, pstep->GetID());

	// set name
	int nid = CountInterfaces<FETensionCompressionInterface>(fem) +1;
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
void FEBioFormat2::ParseContactTiedPoro(FEStep *pstep, XMLTag &tag)
{
	FEModel& fem = GetFEModel();
	FEMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();

	// create new interface
	FETiedBiphasicInterface* pi = new FETiedBiphasicInterface(&fem, pstep->GetID());

	// set name
	int nid = CountInterfaces<FETiedBiphasicInterface>(fem) +1;
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
void FEBioFormat2::ParseRigidWall(FEStep* pstep, XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();
	FEMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();

	// create a new interface
	FERigidWallInterface* pci = new FERigidWallInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "RigidWall%02d", CountInterfaces<FERigidWallInterface>(fem) + 1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pci->SetName(szname);

	FESurface *pms = 0, *pss = 0;

	++tag;
	do
	{
		// read parameters
		if (tag == "laugon") { int n; tag.value(n); pci->SetBoolValue(FERigidWallInterface::LAUGON, (n == 0 ? false : true)); }
		else if (tag == "tolerance") { double f; tag.value(f); pci->SetFloatValue(FERigidWallInterface::ALTOL, f); }
		else if (tag == "penalty") { double f; tag.value(f); pci->SetFloatValue(FERigidWallInterface::PENALTY, f); }
		if (tag == "plane")
		{
			double n[4];
			tag.value(n, 4);
			pci->SetFloatValue(FERigidWallInterface::PA, n[0]);
			pci->SetFloatValue(FERigidWallInterface::PB, n[1]);
			pci->SetFloatValue(FERigidWallInterface::PC, n[2]);
			pci->SetFloatValue(FERigidWallInterface::PD, n[3]);

			// if some older formats the lc was set on the plane parameter, although now it should be on the offset parameter
			const char* szlc = tag.AttributeValue("lc", true);
			if (szlc)
			{
				febio.AddParamCurve(&pci->GetParam(FERigidWallInterface::OFFSET), atoi(szlc) - 1);
				pci->GetParam(FERigidWallInterface::OFFSET).SetFloatValue(1.0);
			}
		}
		else if (tag == "offset")
		{
			double s = 0.0;
			tag.value(s);

			const char* szlc = tag.AttributeValue("lc", true);
			if (szlc)
			{
				febio.AddParamCurve(&pci->GetParam(FERigidWallInterface::OFFSET), atoi(szlc) - 1);
				pci->GetParam(FERigidWallInterface::OFFSET).SetFloatValue(s);
			}
		}
		else if (tag == "surface")
		{
			const char* szn = tag.AttributeValue("name", true);

			FESurface* ps = ParseContactSurface(tag);
			pci->SetItemList(ps);

			if (szn) ps->SetName(szn);
		}
		++tag;
	} while (!tag.isend());

	// add interface to step
	pstep->AddComponent(pci);
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseContactRigid(FEStep *pstep, XMLTag &tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();
	FEMesh* pm = &GetFEMesh();
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

			FENode& node = pm->Node(id);
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
			FENodeSet* pn = new FENodeSet(po);
			//			pm->AddNodeSet(pn);
			sprintf(szbuf, "RigidNodeset%2d", CountInterfaces<FERigidInterface>(fem) + 1);
			pn->SetName(szbuf);

			int NN = pm->Nodes();
			for (int j = 0; j<NN; ++j)
			{
				FENode& node = pm->Node(j);
				if (node.m_ntag == id) pn->add(j);
			}

			// create the interface
			FERigidInterface* pi = new FERigidInterface(&fem, pmat, pn, pstep->GetID());
			if (sz) sprintf(szbuf, "%s", szname);
			else sprintf(szbuf, "RigidInterface%02d", i + 1);
			pi->SetName(szbuf);
			pstep->AddComponent(pi);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseContactJoint(FEStep *pstep, XMLTag &tag)
{
	FEBioModel& febio = GetFEBioModel();
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

	if (na >= 0) pi->m_pbodyA = febio.GetMaterial(na - 1);
	if (nb >= 0) pi->m_pbodyB = febio.GetMaterial(nb - 1);
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseConnector(FEStep *pstep, XMLTag &tag, const int rc)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	FERigidConnector* pi;
	char szname[256];

	switch (rc) {
	case 0:
		pi = new FERigidSphericalJoint(&fem, pstep->GetID());
		sprintf(szname, "RigidSphericalJoint%02d", CountConnectors<FERigidSphericalJoint>(fem) + 1);
		break;
	case 1:
		pi = new FERigidRevoluteJoint(&fem, pstep->GetID());
		sprintf(szname, "RigidrevoluteJoint%02d", CountConnectors<FERigidRevoluteJoint>(fem) +1);
		break;
	case 2:
		pi = new FERigidPrismaticJoint(&fem, pstep->GetID());
		sprintf(szname, "RigidPrismaticJoint%02d", CountConnectors<FERigidPrismaticJoint>(fem) +1);
		break;
	case 3:
		pi = new FERigidCylindricalJoint(&fem, pstep->GetID());
		sprintf(szname, "RigidCylindricalJoint%02d", CountConnectors<FERigidCylindricalJoint>(fem) +1);
		break;
	case 4:
		pi = new FERigidPlanarJoint(&fem, pstep->GetID());
		sprintf(szname, "RigidPlanarJoint%02d", CountConnectors<FERigidPlanarJoint>(fem) +1);
		break;
    case 5:
        pi = new FERigidLock(&fem, pstep->GetID());
        sprintf(szname, "RigidLock%02d", CountConnectors<FERigidLock>(fem) +1);
        break;
	case 6:
		pi = new FERigidSpring(&fem, pstep->GetID());
		sprintf(szname, "RigidSpring%02d", CountConnectors<FERigidSpring>(fem) +1);
		break;
	case 7:
		pi = new FERigidDamper(&fem, pstep->GetID());
		sprintf(szname, "RigidDamper%02d", CountConnectors<FERigidDamper>(fem) +1);
		break;
	case 8:
		pi = new FERigidAngularDamper(&fem, pstep->GetID());
		sprintf(szname, "RigidAngularDamper%02d", CountConnectors<FERigidAngularDamper>(fem) +1);
		break;
	case 9:
		pi = new FERigidContractileForce(&fem, pstep->GetID());
		sprintf(szname, "RigidContractileForce%02d", CountConnectors<FERigidContractileForce>(fem) +1);
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
			if (na >= 0) pi->m_rbA = febio.GetMaterial(na - 1)->GetID();
		}
		else if (tag == "body_b") {
			tag.value(nb);
			if (nb >= 0) pi->m_rbB = febio.GetMaterial(nb - 1)->GetID();
		}
		else
		{
			if (ReadParam(*pi, tag) == false) ParseUnknownTag(tag);
		}
		++tag;
	} while (!tag.isend());
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseSprings(FEStep *pstep, XMLTag &tag)
{
	FEBioModel& febio = GetFEBioModel();

	FEModel &fem = GetFEModel();
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
			GLinearSpring* ps = new GLinearSpring(n[0], n[1]);
			char szname[256];
			sprintf(szname, "Spring%02d", N);
			ps->SetName(szname);
			ps->GetParam(GLinearSpring::MP_E).SetFloatValue(E);
			pd = ps;
		}
		break;
	case FE_GENERAL_SPRING:
		{
			GGeneralSpring* pg = new GGeneralSpring(n[0], n[1]);
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
void FEBioFormat2::ParseLinearConstraint(FEStep* pstep, XMLTag& tag)
{
	FEModel &fem = GetFEModel();

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
void FEBioFormat2::ParseBodyForce(FEStep *pstep, XMLTag &tag)
{
	FEModel &fem = GetFEModel();

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
void FEBioFormat2::ParseHeatSource(FEStep *pstep, XMLTag &tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel &fem = GetFEModel();

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
			febio.AddParamCurve(phs->GetLoadCurve(), atoi(szlc) - 1);
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
bool FEBioFormat2::ParseConstraintSection(XMLTag& tag)
{
	if (tag.isleaf()) return true;

	FEStep* pstep = m_pBCStep;

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
			else ParseUnknownTag(tag);
		}
		else ParseUnknownTag(tag);
		++tag;
	} while (!tag.isend());

	return true;
}

//-----------------------------------------------------------------------------
void FEBioFormat2::ParseRigidConstraint(FEStep* pstep, XMLTag& tag)
{
	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	// get the material ID
	int nid = tag.Attribute("mat").value<int>() - 1;

	// get the rigid material
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
		if (tag == "initial_velocity")
		{
			FERigidVelocity* pv = new FERigidVelocity(&fem, pstep->GetID());
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
			FERigidAngularVelocity* pv = new FERigidAngularVelocity(&fem, pstep->GetID());
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
				if (pc == 0) pc = new FERigidFixed(&fem, pstep->GetID());
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
				if (lc > 0) febio.AddParamCurve(pf->GetLoadCurve(), lc - 1);
				else pf->GetLoadCurve()->Clear();
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
void FEBioFormat2::ParseVolumeConstraint(FEStep* pstep, XMLTag& tag)
{
	FEModel& fem = GetFEModel();

	// make sure there is something to read
	if (tag.isempty()) return;

	// create a new volume constraint
	FEVolumeConstraint* pi = new FEVolumeConstraint(&fem, pstep->GetID());
	pstep->AddComponent(pi);

	// get the (optional) contact name
	char szbuf[256];
	const char* szname = tag.AttributeValue("name", true);
	if (szname) sprintf(szbuf, "%s", szname);
	else sprintf(szbuf, "VolumeConstraint%02d", CountConstraints<FEVolumeConstraint>(fem) + 1);
	pi->SetName(szbuf);

	// get the mesh (need it for defining the surface)
	FEMesh* pm = &GetFEMesh();
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
				FESurface* ps = ParseContactSurface(tag);
				sprintf(szbuf, "VolumeConstraintSurface%02d", CountConstraints<FEVolumeConstraint>(fem));
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
void FEBioFormat2::ParseSymmetryPlane(FEStep* pstep, XMLTag& tag)
{
	FEModel& fem = GetFEModel();

	// make sure there is something to read
	if (tag.isempty()) return;

	// create a new volume constraint
	FESymmetryPlane* pi = new FESymmetryPlane(&fem, pstep->GetID());
	pstep->AddComponent(pi);

	// get the (optional) contact name
	char szbuf[256];
	const char* szname = tag.AttributeValue("name", true);
	if (szname) sprintf(szbuf, "%s", szname);
	else sprintf(szbuf, "SymmetrtPlane%02d", CountConstraints<FESymmetryPlane>(fem) +1);
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
				FESurface* ps = ParseContactSurface(tag);
				sprintf(szbuf, "SymmetryPlaneSurface%02d", CountConstraints<FESymmetryPlane>(fem));
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
void FEBioFormat2::ParseNrmlFldVlctSrf(FEStep* pstep, XMLTag& tag)
{
    FEModel& fem = GetFEModel();
    
    // make sure there is something to read
    if (tag.isempty()) return;
    
    // create a new volume constraint
    FENormalFlowSurface* pi = new FENormalFlowSurface(&fem, pstep->GetID());
    pstep->AddComponent(pi);
    
    // get the (optional) contact name
    char szbuf[256];
    const char* szname = tag.AttributeValue("name", true);
    if (szname) sprintf(szbuf, "%s", szname);
    else sprintf(szbuf, "NormalFlowSurface%02d", CountConstraints<FENormalFlowSurface>(fem) +1);
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
                FESurface* ps = ParseContactSurface(tag);
                sprintf(szbuf, "NormalFlowSurface%02d", CountConstraints<FENormalFlowSurface>(fem));
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
	FEModel& fem = GetFEModel();
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

//-----------------------------------------------------------------------------
FENodeSet* FEBioFormat2::ParseNodeSet(XMLTag& tag)
{
	GMeshObject* po = GetGObject();

	// create a new node set
	FENodeSet* pg = new FENodeSet(po);

	const char* szset = tag.AttributeValue("nset", true);
	if (szset)
	{
		// make sure this tag is empty
		if (tag.isempty() == false) throw XMLReader::InvalidValue(tag);

		// see if we can find the nodeset
		FENodeSet* ps = po->FindFENodeSet(szset);

		// make sure the set is found
		if (ps == 0) throw XMLReader::InvalidAttributeValue(tag, "nset", szset);

		// create a copy of this node set
		pg->Copy(ps);
	}
	else
	{
		// see if the name tag is defined
		const char* szname = tag.AttributeValue("name", true);
		if (szname) pg->SetName(szname);

		// loop over all nodes
		++tag;
		do
		{
			// get the node ID
			int n = tag.Attribute("id").value<int>();

			// assign the node to this group
			pg->add(n - 1);

			++tag;
		}
		while (!tag.isend());
	}

	return pg;
}
