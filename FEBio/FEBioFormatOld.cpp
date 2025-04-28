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
#include "FEBioFormatOld.h"
#include <FEMLib/FERigidConstraint.h>
#include <GeomLib/GMeshObject.h>
#include <FEMLib/FEInitialCondition.h>
#include <FEMLib/FEBodyLoad.h>
#include <FEMLib/FEModelConstraint.h>
#include <FEMLib/GDiscreteObject.h>
#include <GeomLib/GModel.h>
#include <FEBioLink/FEBioModule.h>
using namespace std;

FEBioFormatOld::FEBioFormatOld(FEBioFileImport* fileReader, FEBioInputModel& febio) : FEBioFormat(fileReader, febio)
{
}

FEBioFormatOld::~FEBioFormatOld()
{
}

bool FEBioFormatOld::ParseSection(XMLTag& tag)
{
	if      (tag == "Module"     ) ParseModuleSection    (tag);
	else if (tag == "Control"    ) ParseControlSection   (tag);
	else if (tag == "Material"   ) ParseMaterialSection  (tag);
	else if (tag == "Geometry"   ) ParseGeometrySection  (tag);
	else if (tag == "Boundary"   ) ParseBoundarySection  (tag);
	else if (tag == "Constraints") ParseConstraintSection(tag);
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
bool FEBioFormatOld::ParseModuleSection(XMLTag &tag)
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
	else if (atype == "poro")
	{
		m_nAnalysis = FE_STEP_BIPHASIC;
		FileReader()->AddLogEntry("poro module is obsolete. Use biphasic instead (line %d)", tag.currentLine());
	}
	else
	{
		m_nAnalysis = FE_STEP_MECHANICS;
		FileReader()->AddLogEntry("unknown module type. Assuming solid module (line %d)", tag.currentLine());
	}

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

	const char* sztype = atype.cvalue();
	int moduleId = FEBio::GetModuleId(sztype);
	if (moduleId < 0) { throw XMLReader::InvalidAttributeValue(tag, "type", sztype); }
	FileReader()->GetProject().SetModule(moduleId, false);

	return (m_nAnalysis != -1);}

//=============================================================================
//
//                                G E O M E T R Y
//
//=============================================================================

//-----------------------------------------------------------------------------
//  Parses the geometry section from the xml file (version 1.2 and before)
//
bool FEBioFormatOld::ParseGeometrySection(XMLTag& tag)
{
	// make sure the section is not empty
	if (tag.isleaf()) return true;

	// make sure we haven't been here before
	FEBioInputModel& febio = GetFEBioModel();
	if (febio.Parts() != 0) throw XMLReader::InvalidTag(tag);

	// Add the one-and-only part
	FEBioInputModel::Part& part = *febio.AddPart("Object01");

	// Add an instance
	assert(febio.Instances() == 0);
	FEBioInputModel::PartInstance* instance = new FEBioInputModel::PartInstance(&part);
	instance->SetName(part.GetName());
	febio.AddInstance(instance);

	// create a new mesh
	FSMesh* pm = part.GetFEMesh();

	// keep track of how many elements are assigned to each material
	// MAT[0] is the number of elements that don't have a material assigned
	int NMAT = febio.Materials();
	vector<int> MAT; MAT.assign(NMAT + 1, 0);
	vector<GMaterial*> PMAT; PMAT.assign(NMAT + 1, (GMaterial*)0);
	for (int i = 0; i<NMAT; ++i) PMAT[i + 1] = febio.GetMaterial(i);

	++tag;
	do
	{
		if (tag == "Nodes")
		{
			if (!tag.isleaf() && !tag.isempty())
			{
				// first we need to figure out how many nodes there are
				int nn = tag.children();

				// make sure there are nodes
				if (nn == 0) { delete pm; throw XMLReader::InvalidValue(tag); }

				// create nodes
				pm->Create(nn, 0);

				// read nodal coordinates
				++tag;
				for (int i = 0; i<nn; ++i)
				{
					FSNode& node = pm->Node(i);
					tag.value(node.r);
					++tag;
				}
			}
		}
		else if (tag == "Elements")
		{
			if (!tag.isleaf() && !tag.isempty())
			{
				// first we need to figure out how many elements there are
				int elems = tag.children();

				// make sure there are elements
				if (elems == 0) { delete pm; throw XMLReader::InvalidValue(tag); }

				// create elements
				pm->Create(0, elems);

				// read element data
				++tag;
				int n[FSElement::MAX_NODES];
				for (int i = 0; i<elems; ++i)
				{
					// read element type
					FSElement& el = pm->Element(i);
					if (tag == "hex8") el.SetType(FE_HEX8);
					else if (tag == "hex20") el.SetType(FE_HEX20);
					else if (tag == "hex27") el.SetType(FE_HEX27);
					else if (tag == "penta6") el.SetType(FE_PENTA6);
					else if (tag == "tet4") el.SetType(FE_TET4);
					else if (tag == "tet10") el.SetType(FE_TET10);
					else if (tag == "tet15") el.SetType(FE_TET15);
					else if (tag == "quad4") el.SetType(FE_QUAD4);
					else if (tag == "quad8") el.SetType(FE_QUAD8);
					else if (tag == "quad9") el.SetType(FE_QUAD9);
					else if (tag == "tri3") el.SetType(FE_TRI3);
					else if (tag == "tri6") el.SetType(FE_TRI6);
					else if (tag == "pyra5") el.SetType(FE_PYRA5);
                    else if (tag == "pyra13") el.SetType(FE_PYRA13);
					else throw XMLReader::InvalidTag(tag);

					// read nodes
					tag.value(n, el.Nodes());
					for (int j = 0; j<el.Nodes(); ++j) el.m_node[j] = n[j] - 1;

					// read material
					int nmat = tag.Attribute("mat").value<int>() - 1;

					// we'll create a new part for each material
					if (nmat >= 0) el.m_gid = nmat + 1; else el.m_gid = 0;
					MAT[el.m_gid]++;

					++tag;
				}
			}
		}
		else if (tag == "ElementData")
		{
			// read additional element data
			int i = 0;
			++tag;
			do
			{
				if (tag == "element")
				{
					// get the element ID
					int id = tag.Attribute("id").value<int>() - 1;

					// read the data
					++tag;
					do
					{
						if (tag == "fiber")
						{
							FSElement& el = pm->Element(id);

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
							FSElement& el = pm->Element(id);

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
							FSElement& el = pm->Element(id);
							if (!el.IsShell()) return false;
							tag.value(el.m_h, el.Nodes());
						}
						else if (tag == "area")
						{
							FSElement& el = pm->Element(id);
							if (!el.IsBeam()) return false;
							tag.value(el.m_a0);
						}
						else ParseUnknownTag(tag);

						++tag;
					} while (!tag.isend());
				}
				++tag;
			} while (!tag.isend());

		}
		else ParseUnknownTag(tag);

		++tag;
	} while (!tag.isend());

	// make sure there is something to process
	if ((pm->Nodes() == 0) || (pm->Elements() == 0))
	{
		delete pm;
		return true;
	}

	// see if there any elements that don't have a material assigned.
	if (MAT[0] > 0)
	{
		FileReader()->AddLogEntry("%d elements don't have a material assigned.", MAT[0]);
	}

	// reindex materials
	int n = 0;
	for (int i = 0; i<(int)MAT.size(); ++i)
	{
		if (MAT[i]>0) MAT[i] = n++; else MAT[i] = -1;
	}

	// assign group ID numbers. Groups ID's are assigned for
	// each material, considering only materials that are actually used
	for (int i = 0; i<pm->Elements(); ++i)
	{
		FSElement& e = pm->Element(i);
		e.m_gid = MAT[e.m_gid]; assert(e.m_gid >= 0);
	}

	// create domains 
	for (int i=0; i<(int) MAT.size(); ++i)
	{
		if (MAT[i] >= 0)
		{
			char sz[64] = {0};
			sprintf(sz, "Part%d", MAT[i] + 1);
			part.AddDomain(sz, MAT[i]);
		}
	}

	// don't forget to update the mesh data
	febio.UpdateGeometry();

	return true;
}

//=============================================================================
//
//                                B O U N D A R Y 
//
//=============================================================================

//-----------------------------------------------------------------------------
//  Parses the boundary section from the xml file
//
bool FEBioFormatOld::ParseBoundarySection(XMLTag& tag)
{
	if (tag.isleaf()) return true;

	++tag;
	do
	{
		if      (tag == "fix"            ) ParseBCFixed         (m_pBCStep, tag);
		else if (tag == "prescribe"      ) ParseBCPrescribed    (m_pBCStep, tag);
		else if (tag == "contact"        ) ParseContact         (m_pBCStep, tag);
		else if (tag == "spring"         ) ParseSprings         (m_pBCStep, tag);
		else if (tag == "force"          ) ParseForceLoad       (m_pBCStep, tag);
		else if (tag == "pressure"       ) ParsePressureLoad    (m_pBCStep, tag);
		else if (tag == "traction"       ) ParseTractionLoad    (m_pBCStep, tag);
		else if (tag == "fluidflux"      ) ParseFluidFlux       (m_pBCStep, tag);
		else if (tag == "normal_traction") ParseBPNormalTraction(m_pBCStep, tag);
		else if (tag == "heatflux"       ) ParseHeatFlux        (m_pBCStep, tag);
		else if (tag == "soluteflux"     ) ParseSoluteFlux      (m_pBCStep, tag);
		else ParseUnknownTag(tag);
		++tag;
	} 
	while (!tag.isend());

	return true;
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseBCFixed(FSStep* pstep, XMLTag &tag)
{
	char szname[256] = { 0 };
	int i;

	// count how many fixed nodes there are
	int nfix = 0;
	XMLTag t(tag); ++t;
	while (!t.isend()) { nfix++; ++t; }

	FSMesh* pm = &GetFEMesh();
	int N = pm->Nodes();
	vector<int> BC; BC.resize(N);
	for (i = 0; i<N; ++i) BC[i] = 0;

	FSModel& fem = GetFSModel();

	// read the prescribed data
	++tag;
	for (i = 0; i<nfix; ++i)
	{
		int n = tag.Attribute("id").value<int>() - 1;
		XMLAtt& abc = tag.Attribute("bc");

		// get the current BC
		int bc = BC[n];

		// OR-it with the new bc
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
		else if (abc == "c") bc |= (1 << 8);
		else if (abc == "c1") bc |= (1 << 8);
		else if (abc == "c2") bc |= (1 << 9);
		else if (abc == "c3") bc |= (1 << 10);
		else if (abc == "c4") bc |= (1 << 11);
		else if (abc == "c5") bc |= (1 << 12);
		else if (abc == "c6") bc |= (1 << 13);

		BC[n] = bc;

		++tag;
	}

	int nbc = 0;
	do
	{
		// find a non-zero bc
		nbc = 0;
		for (i = 0; i<N; ++i) if (BC[i] != 0) { nbc = BC[i]; break; }

		if (nbc)
		{
			// determine what kind of BC we want to add
			unsigned int ntype = 0;
			for (int i = 0; i <= 13; ++i) if (nbc&(1 << i)) { ntype = 1 << i; }

			// count all nodes that have this BC
			nfix = 0;
			for (int i = 0; i<N; ++i) if (BC[i] & ntype) ++nfix;

			// create a nodeset of this BC
			FSNodeSet* pg = new FSNodeSet(pm);
			pm->AddFENodeSet(pg);

			// assign the nodes to this group
			for (int i = 0; i<N; ++i) if (BC[i] & ntype) { pg->add(i); BC[i] &= (~ntype); }

			// create the constraint
			if (ntype < 8)
			{
				FSFixedDisplacement* pbc = new FSFixedDisplacement(&fem, pg, ntype, pstep->GetID());
				sprintf(szname, "FixedDisplacement%02d", CountBCs<FSFixedDisplacement>(fem)+1);
				pbc->SetName(szname);
				pg->SetName(szname);
				pstep->AddComponent(pbc);
			}
			else if (ntype < 64)
			{
				ntype = ntype >> 3;
				FSFixedRotation* pbc = new FSFixedRotation(&fem, pg, ntype, pstep->GetID());
				sprintf(szname, "FixedRotation%02d", CountBCs<FSFixedRotation>(fem)+1);
				pbc->SetName(szname);
				pg->SetName(szname);
				pstep->AddComponent(pbc);
			}
			else if (ntype == 64)
			{
				FSFixedTemperature* pbc = new FSFixedTemperature(&fem, pg, 1, pstep->GetID());
				sprintf(szname, "FixedTemperature%02d", CountBCs<FSFixedTemperature>(fem)+1);
				pbc->SetName(szname);
				pg->SetName(szname);
				pstep->AddComponent(pbc);
			}
			else if (ntype == 128)
			{
				FSFixedFluidPressure* pbc = new FSFixedFluidPressure(&fem, pg, 1, pstep->GetID());
				sprintf(szname, "FixedFluidPressure%02d", CountBCs<FSFixedFluidPressure>(fem)+1);
				pbc->SetName(szname);
				pg->SetName(szname);
				pstep->AddComponent(pbc);
			}
			else
			{
				ntype = ntype >> 8;
				if (ntype < 256)
				{
					FSFixedConcentration* pbc = new FSFixedConcentration(&fem, pg, ntype, pstep->GetID());
					sprintf(szname, "FixedConcentration%02d", CountBCs<FSFixedConcentration>(fem)+1);
					pbc->SetName(szname);
					pg->SetName(szname);
					pstep->AddComponent(pbc);
				}
			}
		}
	} while (nbc);
}


//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseBCPrescribed(FSStep* pstep, XMLTag& tag)
{
	// check for optional type attribute
	bool brel = false;
	const char* szr = tag.AttributeValue("type", true);
	if (szr && (strcmp(szr, "relative") == 0)) brel = true;

	int lcmax = 0, i, j;
	// count how many prescibed nodes there are
	int ndis = 0;
	XMLTag t(tag); ++t;
	while (!t.isend()) { ndis++; ++t; }

	// read the prescribed data
	vector<DISP_CARD> DC(ndis);
	++tag;
	for (i = 0; i<ndis; ++i)
	{
		int n = tag.Attribute("id").value<int>() - 1;
		XMLAtt& abc = tag.Attribute("bc");

		int bc = 0;
		if (abc == "x") bc = 0;
		else if (abc == "y") bc = 1;
		else if (abc == "z") bc = 2;
		else if (abc == "t") bc = 3;
		else if (abc == "p") bc = 4;
		else if (abc == "c") bc = 5;
		else if (abc == "c1") bc = 5;
		else if (abc == "c2") bc = 6;
		else if (abc == "c3") bc = 7;
		else if (abc == "c4") bc = 8;
		else if (abc == "c5") bc = 9;
		else if (abc == "c6") bc = 10;

		// get the loadcurve ID
		int lc = 0;
		XMLAtt* palc = tag.AttributePtr("lc");
		if (palc) lc = palc->value<int>();

		if (lc > lcmax) lcmax = lc;

		double s;
		tag.value(s);

		DC[i].node = n;
		DC[i].bc = bc;
		DC[i].lc = lc - 1;
		DC[i].s = s;

		++tag;
	}

	// first, let's see how many nodesets we need to create
	const int MAXBC = 11;
	vector<int> cc[MAXBC];
	for (i = 0; i<MAXBC; ++i)
	{
		cc[i].resize(lcmax);
		for (int j = 0; j<lcmax; ++j) cc[i][j] = -1;
	}

	for (i = 0; i<ndis; ++i)
	{
		int lc = DC[i].lc;
		int bc = DC[i].bc;
		cc[bc][lc] = 1;
	}

	int nns = 0;
	for (i = 0; i<lcmax; ++i)
	{
		for (int j = 0; j<MAXBC; ++j)
		if (cc[j][i] == 1) cc[j][i] = nns++;
	}

	// create the prescribed BC
	FEBioInputModel& feb = GetFEBioModel();
	FSModel& fem = GetFSModel();
	char szname[256];
	vector<FSPrescribedDOF*> pBC(nns);
	vector<FSNodeSet*> pNS(nns);
	nns = 0;
	FSMesh* pm = &GetFEMesh();
	for (i = 0; i<lcmax; ++i)
	{
		for (j = 0; j<MAXBC; ++j)
		if (cc[j][i] >= 0)
		{
			// make a new group
			FSNodeSet* pg = new FSNodeSet(pm);
			sprintf(szname, "Nodeset%02d", i + 1);
			pg->SetName(szname);
			pm->AddFENodeSet(pg);

			// make a new boundary condition
			FSPrescribedDOF* pbc = 0;
			switch (j)
			{
			case 0:
				{
					FSPrescribedDisplacement * pd = new FSPrescribedDisplacement(&fem, pg, j, 1, pstep->GetID());
					pd->SetRelativeFlag(brel);
					pbc = pd;
				}
				break;
			case 1:
				{
					FSPrescribedDisplacement* pd = new FSPrescribedDisplacement(&fem, pg, j, 1, pstep->GetID());
					pd->SetRelativeFlag(brel);
					pbc = pd;
				}
				break;
			case 2:
				{
					FSPrescribedDisplacement* pd = new FSPrescribedDisplacement(&fem, pg, j, 1, pstep->GetID());
					pd->SetRelativeFlag(brel);
					pbc = pd;
				}
				break;
			case 3:
				{
					FSPrescribedTemperature* pd = new FSPrescribedTemperature(&fem, pg, 1, pstep->GetID());
					pd->SetRelativeFlag(brel);
					pbc = pd;
				}
				break;
			case 4:
				{
					FSPrescribedFluidPressure* pd = new FSPrescribedFluidPressure(&fem, pg, 1, pstep->GetID());
					pd->SetRelativeFlag(brel);
					pbc = pd;
				}
				break;
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
				{
					int bc = j - 5;
					FSPrescribedConcentration* pd = new FSPrescribedConcentration(&fem, pg, bc, 1.0, pstep->GetID());
					pd->SetRelativeFlag(brel);
					pbc = pd;
				}
				break;
			}
			sprintf(szname, "PrescribedBC%02d", i + 1);
			pbc->SetName(szname);
			pBC[nns] = pbc;
			pNS[nns++] = pg;
			pstep->AddComponent(pbc);
		}
	}

	// assign nodes to node sets
	int ng, n, lc, bc;
	for (i = 0; i<ndis; ++i)
	{
		n = DC[i].node;
		lc = DC[i].lc;
		bc = DC[i].bc;

		ng = cc[bc][lc];
		assert(ng >= 0);

		pNS[ng]->add(n);

		FSPrescribedDOF* pbc = pBC[ng];
//		pbc->GetLoadCurve()->SetID(lc);
		pbc->SetScaleFactor(DC[i].s);
		if (DC[i].lc >= 0)
		{
			feb.AddParamCurve(pbc->GetParam("scale"), DC[i].lc);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseForceLoad(FSStep *pstep, XMLTag &tag)
{
	FEBioInputModel& feb = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// count how many prescibed nodes there are
	int ncnf = 0;
	XMLTag t(tag); ++t;
	while (!t.isend()) { ncnf++; ++t; }

	// read the prescribed data
	int lcmax = 0, i, j, n;
	vector<FORCE_CARD> FC(ncnf);
	++tag;
	for (i = 0; i<ncnf; ++i)
	{
		n = tag.Attribute("id").value<int>() - 1;

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

		int lc = tag.AttributeValue<int>("lc", 0);

		if (lc > lcmax) lcmax = lc;

		double s;
		tag.value(s);

		FC[i].node = n;
		FC[i].bc = bc;
		FC[i].lc = lc - 1;
		FC[i].s = s;

		++tag;
	}

	// first, let's see how many nodesets we need to create
	const int MAXBC = 10;
	vector<int> cc[MAXBC];
	for (j = 0; j<MAXBC; ++j)
	{
		cc[j].resize(lcmax);
		for (i = 0; i<lcmax; ++i) cc[j][i] = -1;
	}

	for (i = 0; i<ncnf; ++i)
	{
		int lc = FC[i].lc;
		int bc = FC[i].bc;
		cc[bc][lc] = 1;
	}

	int nns = 0;
	for (i = 0; i<lcmax; ++i)
	{
		for (int j = 0; j<MAXBC; ++j) if (cc[j][i] > 0) cc[j][i] = nns++;
	}

	// create the force loads
	char szname[256];
	vector<FSNodalDOFLoad*> pFC(nns);
	vector<FSNodeSet*> pNS(nns);
	FSMesh* pm = &GetFEMesh();
	for (i = 0; i<lcmax; ++i)
	{
		for (j = 0; j<MAXBC; ++j)
		if (cc[j][i] >= 0)
		{
			FSNodeSet* pg = new FSNodeSet(pm);
			pm->AddFENodeSet(pg);
			sprintf(szname, "ForceNodeset%02d", i + 1);
			pg->SetName(szname);

			FSNodalDOFLoad* pbc = new FSNodalDOFLoad(&fem, pg, j, 1, pstep->GetID());
			sprintf(szname, "ForceLoad%02d", i + 1);
			pbc->SetName(szname);
			pFC[cc[j][i]] = pbc;
			pNS[cc[j][i]] = pg;
			pstep->AddComponent(pbc);

			if (FC[i].lc >= 0)
			{
				feb.AddParamCurve(pbc->GetParam("scale"), FC[i].lc);
			}
		}
	}

	// assign nodes to node sets
	int ng;
	for (i = 0; i<ncnf; ++i)
	{
		n = FC[i].node;
		int lc = FC[i].lc;
		int bc = FC[i].bc;

		ng = cc[bc][lc];
		assert(ng >= 0);

//		pFC[ng]->GetLoadCurve()->SetID(lc);
		pFC[ng]->SetLoad(FC[i].s);
		pNS[ng]->add(n);
	}
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParsePressureLoad(FSStep *pstep, XMLTag &tag)
{
	int i;

	// count how many pressure cards there are
	int npc = tag.children();

	FEBioMesh& mesh = GetFEBioMesh();

	FSMesh* pm = &GetFEMesh();
	for (i = 0; i<pm->Faces(); ++i) pm->Face(i).m_ntag = -1;

	int ntype = 0;
	XMLAtt* patype = tag.AttributePtr("type");
	if (patype)
	{
		if (*patype == "nonlinear") ntype = 0;
		else if (*patype == "linear") ntype = 1;
	}

	// read the pressure data
	++tag;
	int nf[8], N = 0, m, nmax = 0;
	double scl = 1.0;
	for (i = 0; i<npc; ++i)
	{
		int lc = tag.AttributeValue<int>("lc", 0);

		if (tag == "quad4") N = 4;
		else if (tag == "tri3") N = 3;
		else if (tag == "tri6") N = 6;
		tag.value(nf, N);
		for (int j = 0; j<N; ++j) nf[j] = nf[j] - 1;
		if (N == 3) nf[3] = nf[2];

		// NOTE: This only works if all pressure cards use the
		//       same scale factor since in PreView, the scale
		//       factor is associated with the BC, not with each card
		XMLAtt* pa = tag.AttributePtr("scale");
		if (pa) scl = pa->value<double>();

		//find surface elements
		m = mesh.FindFace(nf, N);
		if (m >= 0)
		{
			FSFace& face = pm->Face(m);
			face.m_ntag = lc - 1;
			if (lc > nmax) nmax = lc;
		}
		else
		{
			// Oh,oh, we might have a problem!
			assert(false);
		}

		++tag;
	}

	// let's count the nr of surfaces we need
	vector<int> nlc(nmax);
	for (i = 0; i<nmax; ++i) nlc[i] = -1;

	for (i = 0; i<pm->Faces(); ++i)
	{
		FSFace& face = pm->Face(i);
		int lc = face.m_ntag;
		if (lc >= 0) nlc[lc] = 1;
	}

	int ns = 0;
	for (i = 0; i<nmax; ++i) if (nlc[i] > 0) nlc[i] = ns++;

	FSModel& fem = GetFSModel();

	FEBioInputModel& febio = GetFEBioModel();

	// let's create the surfaces
	vector<FSPressureLoad*> pPC(ns);
	vector<FSSurface*> pSF(ns);
	char szname[256];
	int npr = CountLoads<FSPressureLoad>(fem);
	for (i = 0; i<ns; ++i)
	{
		FSSurface* ps = new FSSurface(pm);
		sprintf(szname, "PressureSurface%02d", npr + i + 1);
		ps->SetName(szname);

		FSPressureLoad* pbc = new FSPressureLoad(&fem, ps, pstep->GetID());
		sprintf(szname, "PressureLoad%02d", npr + i + 1);
		pbc->SetLoad(scl);
		pbc->SetLinearFlag(ntype == 1);
		pbc->SetName(szname);
		pPC[i] = pbc;
		pSF[i] = ps;
		pstep->AddComponent(pbc);
	}

	// set the correct face group ID's
	for (i = 0; i<pm->Faces(); ++i)
	{
		FSFace& face = pm->Face(i);
		if (face.m_ntag >= 0)
		{
			int n = nlc[face.m_ntag];
			Param* p = &pPC[n]->GetParam(FSPressureLoad::LOAD);
			FSSurface* ps = pSF[n];
			ps->add(i);

			febio.AddParamCurve(p, face.m_ntag);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseTractionLoad(FSStep* pstep, XMLTag& tag)
{
	// count how many traction cards there are
	int ntc = tag.children();

	FEBioMesh& mesh = GetFEBioMesh();
	FSMesh* pm = &GetFEMesh();
	for (int i = 0; i<pm->Faces(); ++i) pm->Face(i).m_ntag = -1;

	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();
	// read the traction data
	++tag;
	int nf[8], N = 0, m, nmax = 0;
	vec3d tv(0, 0, 0);
	for (int i = 0; i<ntc; ++i)
	{
		int lc = tag.AttributeValue<int>("lc", 0);

		tv.x = tag.AttributeValue<double>("tx", 0.0);
		tv.y = tag.AttributeValue<double>("ty", 0.0);
		tv.z = tag.AttributeValue<double>("tz", 0.0);

		if (tag == "quad4") N = 4;
		else if (tag == "tri3") N = 3;
		else if (tag == "tri6") N = 6;
		tag.value(nf, N);
		for (int j = 0; j<N; ++j) nf[j] = nf[j] - 1;
		if (N == 3) nf[3] = nf[2];

		//find surface elements
		m = mesh.FindFace(nf, N);
		if (m >= 0)
		{
			FSFace& face = pm->Face(m);
			face.m_ntag = lc - 1;
			if (lc > nmax) nmax = lc;
		}
		else
		{
			// Oh,oh, we might have a problem!
			assert(false);
		}

		++tag;
	}

	// let's count the nr of surfaces we need
	vector<int> nlc(nmax);
	for (int i = 0; i<nmax; ++i) nlc[i] = -1;

	for (int i = 0; i<pm->Faces(); ++i)
	{
		FSFace& face = pm->Face(i);
		int lc = face.m_ntag;
		if (lc >= 0) nlc[lc] = 1;
	}

	int ns = 0;
	for (int i = 0; i<nmax; ++i) if (nlc[i] > 0) nlc[i] = ns++;

	// let's create the surfaces
	vector<FSSurfaceTraction*> pPC(ns);
	vector<FSSurface*> pSF(ns);
	char szname[256];
	int ntl = CountLoads<FSSurfaceTraction>(fem);
	for (int i = 0; i<ns; ++i)
	{
		FSSurface* ps = new FSSurface(pm);
		sprintf(szname, "TractionSurface%02d", ntl + i + 1);
		ps->SetName(szname);

		FSSurfaceTraction* pbc = new FSSurfaceTraction(&fem, ps, pstep->GetID());
		sprintf(szname, "TractionLoad%02d", ntl + i + 1);
		pbc->SetTraction(tv);
		pbc->SetName(szname);
		pPC[i] = pbc;
		pSF[i] = ps;
		pstep->AddComponent(pbc);
	}

	// set the correct face group ID's
	for (int i = 0; i<pm->Faces(); ++i)
	{
		FSFace& face = pm->Face(i);
		if (face.m_ntag >= 0)
		{
			int n = nlc[face.m_ntag];
			Param* p = &pPC[n]->GetParam(FSSurfaceTraction::LOAD);
			FSSurface* ps = pSF[n];
			ps->add(i);

			febio.AddParamCurve(p, face.m_ntag);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseFluidFlux(FSStep *pstep, XMLTag &tag)
{
	// count how many fluid flux cards there are
	int nfc = tag.children();

	FEBioMesh& mesh = GetFEBioMesh();
	FSModel& fem = GetFSModel();
	FSMesh* pm = &GetFEMesh();
	for (int i = 0; i<pm->Faces(); ++i) pm->Face(i).m_ntag = -1;

	int ntype = 0;
	XMLAtt* pat = tag.AttributePtr("type");
	if (pat)
	{
		if (*pat == "nonlinear") ntype = 0;
		else if (*pat == "linear") ntype = 1;
	}

	int nflux = 0;
	pat = tag.AttributePtr("flux");
	if (pat)
	{
		if (*pat == "fluid") nflux = 0;
		else if (*pat == "mixture") nflux = 1;
	}

	// read the fluxdata
	++tag;
	int nf[8], N = 0, m, nmax = 0;
	double scl = 1.0;
	for (int i = 0; i<nfc; ++i)
	{
		int lc = tag.AttributeValue<int>("lc", 0);

		if (tag == "quad4") N = 4;
		else if (tag == "tri3") N = 3;
		else if (tag == "tri6") N = 6;
		tag.value(nf, N);
		for (int j = 0; j<N; ++j) nf[j] = nf[j] - 1;
		if (N == 3) nf[3] = nf[2];

		// NOTE: This only works if all pressure cards use the
		//       same scale factor since in PreView, the scale
		//       factor is associated with the BC, not with each card
		XMLAtt* pa = tag.AttributePtr("scale");
		if (pa) scl = pa->value<double>();

		//find surface elements
		m = mesh.FindFace(nf, N);
		if (m >= 0)
		{
			FSFace& face = pm->Face(m);
			face.m_ntag = lc - 1;
			if (lc > nmax) nmax = lc;
		}
		else
		{
			// Oh,oh, we might have a problem!
			assert(false);
		}

		++tag;
	}

	// let's count the nr of surfaces we need
	vector<int> nlc(nmax);
	for (int i = 0; i<nmax; ++i) nlc[i] = -1;

	for (int i = 0; i<pm->Faces(); ++i)
	{
		FSFace& face = pm->Face(i);
		int lc = face.m_ntag;
		if (lc >= 0) nlc[lc] = 1;
	}

	int ns = 0;
	for (int i = 0; i<nmax; ++i) if (nlc[i] > 0) nlc[i] = ns++;

	// let's create the surfaces
	vector<FSFluidFlux*> pPC(ns);
	vector<FSSurface*> pSF(ns);
	char szname[256];
	int nfl = CountLoads<FSFluidFlux>(fem);
	for (int i = 0; i<ns; ++i)
	{
		FSSurface* ps = new FSSurface(pm);
		sprintf(szname, "FluidFluxSurface%02d", nfl + i + 1);
		ps->SetName(szname);

		FSFluidFlux* pbc = new FSFluidFlux(&fem, ps, pstep->GetID());
		sprintf(szname, "FluidFlux%02d", nfl + i + 1);
		pbc->SetLinearFlag(ntype == 1);
		pbc->SetMixtureFlag(nflux == 1);
		pbc->SetLoad(scl);
		pbc->SetName(szname);
		pPC[i] = pbc;
		pSF[i] = ps;
		pstep->AddComponent(pbc);
	}

	FEBioInputModel& febio = GetFEBioModel();

	// set the correct face group ID's
	for (int i = 0; i<pm->Faces(); ++i)
	{
		FSFace& face = pm->Face(i);
		if (face.m_ntag >= 0)
		{
			int n = nlc[face.m_ntag];
			Param* p = &pPC[n]->GetParam(FSFluidFlux::LOAD);
			FSSurface* ps = pSF[n];
			ps->add(i);

			febio.AddParamCurve(p, face.m_ntag);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseBPNormalTraction(FSStep *pstep, XMLTag &tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// count how many cards there are
	int npc = tag.children();

	FEBioMesh& mesh = GetFEBioMesh();
	FSMesh* pm = &GetFEMesh();
	for (int i = 0; i<pm->Faces(); ++i) pm->Face(i).m_ntag = -1;

	int ntype = 0;
	XMLAtt* pat = tag.AttributePtr("type");
	if (pat)
	{
		if (*pat == "nonlinear") ntype = 0;
		else if (*pat == "linear") ntype = 1;
	}

	int ntrac = 0;
	pat = tag.AttributePtr("traction");
	if (pat)
	{
		if (*pat == "effective") ntrac = 0;
		else if (*pat == "mixture") ntrac = 1;
	}

	// read the traction data
	++tag;
	int nf[8], N = 0, m, nmax = 0;
	double scl = 1.0;
	for (int i = 0; i<npc; ++i)
	{
		int lc = tag.AttributeValue<int>("lc", 0);

		if (tag == "quad4") N = 4;
		else if (tag == "tri3") N = 3;
		else if (tag == "tri6") N = 6;
		tag.value(nf, N);
		for (int j = 0; j<N; ++j) nf[j] = nf[j] - 1;
		if (N == 3) nf[3] = nf[2];

		// NOTE: This only works if all pressure cards use the
		//       same scale factor since in PreView, the scale
		//       factor is associated with the BC, not with each card
		XMLAtt* pa = tag.AttributePtr("scale");
		if (pa) scl = pa->value<double>();

		//find surface elements
		m = mesh.FindFace(nf, N);
		if (m >= 0)
		{
			FSFace& face = pm->Face(m);
			face.m_ntag = lc - 1;
			if (lc > nmax) nmax = lc;
		}
		else
		{
			// Oh,oh, we might have a problem!
		}

		++tag;
	}

	// let's count the nr of surfaces we need
	vector<int> nlc(nmax);
	for (int i = 0; i<nmax; ++i) nlc[i] = -1;

	for (int i = 0; i<pm->Faces(); ++i)
	{
		FSFace& face = pm->Face(i);
		int lc = face.m_ntag;
		if (lc >= 0) nlc[lc] = 1;
	}

	int ns = 0;
	for (int i = 0; i<nmax; ++i) if (nlc[i] > 0) nlc[i] = ns++;

	// let's create the surfaces
	vector<FSBPNormalTraction*> pPC(ns);
	vector<FSSurface*> pSF(ns);
	char szname[256];
	int ntl = CountLoads<FSBPNormalTraction>(fem);
	for (int i = 0; i<ns; ++i)
	{
		FSSurface* ps = new FSSurface(pm);
		sprintf(szname, "NormalTractionSurface%02d", ntl + i + 1);
		ps->SetName(szname);

		FSBPNormalTraction* pbc = new FSBPNormalTraction(&fem, ps, pstep->GetID());
		sprintf(szname, "NormalTraction%02d", ntl + i + 1);
		pbc->SetLoad(scl);
		pbc->SetLinearFlag(ntype == 1);
		pbc->SetMixtureFlag(ntrac == 1);
		pbc->SetName(szname);
		pPC[i] = pbc;
		pSF[i] = ps;
		pstep->AddComponent(pbc);
	}

	// set the correct face group ID's
	for (int i = 0; i<pm->Faces(); ++i)
	{
		FSFace& face = pm->Face(i);
		if (face.m_ntag >= 0)
		{
			int n = nlc[face.m_ntag];
			Param* p = &pPC[n]->GetParam(FSBPNormalTraction::LOAD);
			FSSurface* ps = pSF[n];
			ps->add(i);

			febio.AddParamCurve(p, face.m_ntag);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseHeatFlux(FSStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();

	// count how many heat flux cards there are
	int npc = tag.children();

	FEBioMesh& mesh = GetFEBioMesh();

	FSMesh* pm = &GetFEMesh();
	for (int i = 0; i<pm->Faces(); ++i) pm->Face(i).m_ntag = -1;

	// read the flux data
	++tag;
	int nf[8], N = 0, m, nmax = 0;
	double scl = 1.0;
	for (int i = 0; i<npc; ++i)
	{
		int lc = tag.AttributeValue<int>("lc", 0);

		if (tag == "quad4") N = 4;
		else if (tag == "tri3") N = 3;
		else if (tag == "tri6") N = 6;
		tag.value(nf, N);
		for (int j = 0; j<N; ++j) nf[j] = nf[j] - 1;
		if (N == 3) nf[3] = nf[2];

		// NOTE: This only works if all pressure cards use the
		//       same scale factor since in PreView, the scale
		//       factor is associated with the BC, not with each card
		XMLAtt* pa = tag.AttributePtr("scale");
		if (pa) scl = pa->value<double>();

		//find surface elements
		m = mesh.FindFace(nf, N);
		if (m >= 0)
		{
			FSFace& face = pm->Face(m);
			face.m_ntag = lc - 1;
			if (lc > nmax) nmax = lc;
		}
		else
		{
			// Oh,oh, we might have a problem!
		}

		++tag;
	}

	// let's count the nr of surfaces we need
	vector<int> nlc(nmax);
	for (int i = 0; i<nmax; ++i) nlc[i] = -1;

	for (int i = 0; i<pm->Faces(); ++i)
	{
		FSFace& face = pm->Face(i);
		int lc = face.m_ntag;
		if (lc >= 0) nlc[lc] = 1;
	}

	int ns = 0;
	for (int i = 0; i<nmax; ++i) if (nlc[i] > 0) nlc[i] = ns++;

	// let's create the surfaces
	vector<FSHeatFlux*> pPC(ns);
	vector<FSSurface*> pSF(ns);
	char szname[256];
	int nhf = CountLoads<FSHeatFlux>(fem);
	for (int i = 0; i<ns; ++i)
	{
		FSSurface* ps = new FSSurface(pm);
		sprintf(szname, "HeatFluxSurface%02d", nhf + i + 1);
		ps->SetName(szname);

		FSHeatFlux* pbc = new FSHeatFlux(&fem, ps, pstep->GetID());
		sprintf(szname, "HeatFlux%02d", nhf + i + 1);
		pbc->SetName(szname);
		pbc->SetLoad(scl);
		pPC[i] = pbc;
		pSF[i] = ps;
		pstep->AddComponent(pbc);
	}

	FEBioInputModel& febio = GetFEBioModel();

	// set the correct face group ID's
	for (int i = 0; i<pm->Faces(); ++i)
	{
		FSFace& face = pm->Face(i);
		if (face.m_ntag >= 0)
		{
			int n = nlc[face.m_ntag];
			Param* p = &pPC[n]->GetParam(FSHeatFlux::FLUX);
			FSSurface* ps = pSF[n];
			ps->add(i);

			febio.AddParamCurve(p, face.m_ntag);
		}
	}
}


//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseSoluteFlux(FSStep *pstep, XMLTag &tag)
{
	// check the sol attribute
	int bc = 0;
	const char* szbc = tag.AttributeValue("sol", true);
	if (szbc) bc = atoi(szbc) - 1;

	// count how many solute flux cards there are
	int nsc = tag.children();

	FEBioMesh& mesh = GetFEBioMesh();
	FSMesh* pm = &GetFEMesh();
	for (int i = 0; i<pm->Faces(); ++i) pm->Face(i).m_ntag = -1;

	// read the flux data
	++tag;
	int nf[8], N = 0, m, nmax = 0;
	double scl = 1.0;
	for (int i = 0; i<nsc; ++i)
	{
		int lc = tag.AttributeValue<int>("lc", 0);

		if (tag == "quad4") N = 4;
		else if (tag == "tri3") N = 3;
		else if (tag == "tri6") N = 6;
		tag.value(nf, N);
		for (int j = 0; j<N; ++j) nf[j] = nf[j] - 1;
		if (N == 3) nf[3] = nf[2];

		// NOTE: This only works if all pressure cards use the
		//       same scale factor since in PreView, the scale
		//       factor is associated with the BC, not with each card
		XMLAtt* pa = tag.AttributePtr("scale");
		if (pa) scl = pa->value<double>();

		//find surface elements
		m = mesh.FindFace(nf, N);
		if (m >= 0)
		{
			FSFace& face = pm->Face(m);
			face.m_ntag = lc - 1;
			if (lc > nmax) nmax = lc;
		}
		else
		{
			// Oh,oh, we might have a problem!
		}

		++tag;
	}

	// let's count the nr of surfaces we need
	vector<int> nlc(nmax);
	for (int i = 0; i<nmax; ++i) nlc[i] = -1;

	for (int i = 0; i<pm->Faces(); ++i)
	{
		FSFace& face = pm->Face(i);
		int lc = face.m_ntag;
		if (lc >= 0) nlc[lc] = 1;
	}

	int ns = 0;
	for (int i = 0; i<nmax; ++i) if (nlc[i] > 0) nlc[i] = ns++;

	FSModel& fem = GetFSModel();

	// let's create the surfaces
	vector<FSSoluteFlux*> pPC(ns);
	vector<FSSurface*> pSF(ns);
	char szname[256];
	int nsf = CountLoads<FSSoluteFlux>(fem);
	for (int i = 0; i<ns; ++i)
	{
		FSSurface* ps = new FSSurface(pm);
		sprintf(szname, "SoluteFluxSurface%02d", nsf + i + 1);
		ps->SetName(szname);

		FSSoluteFlux* pbc = new FSSoluteFlux(&fem, ps, pstep->GetID());
		sprintf(szname, "SoluteFlux%02d", nsf + i + 1);
		pbc->SetName(szname);
		pbc->SetBC(bc);
		pbc->SetLoad(scl);
		pPC[i] = pbc;
		pSF[i] = ps;
		pstep->AddComponent(pbc);
	}

	FEBioInputModel& febio = GetFEBioModel();

	// set the correct face group ID's
	for (int i = 0; i<pm->Faces(); ++i)
	{
		FSFace& face = pm->Face(i);
		if (face.m_ntag >= 0)
		{
			int n = nlc[face.m_ntag];
			Param* p = &pPC[n]->GetParam(FSSoluteFlux::LOAD);
			FSSurface* ps = pSF[n];
			ps->add(i);

			febio.AddParamCurve(p, face.m_ntag);
		}
	}
}


//=============================================================================
//
//                                L O A D S
//
//=============================================================================

//-----------------------------------------------------------------------------
bool FEBioFormatOld::ParseInitialSection(XMLTag& tag)
{
	FSModel& fem = GetFSModel();

	++tag;
	do
	{
		if (tag == "velocity")	// initial velocity BC
		{
			FSNodeSet* pg = 0;
			FSMesh* pm = &GetFEMesh();
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
					if (pg == 0) pg = new FSNodeSet(pm);
					pg->add(id);
				}
				else ParseUnknownTag(tag);
				++tag;
			} while (!tag.isend());

			// create a new initial velocity BC
			FSNodalVelocities* pbc = new FSNodalVelocities(&fem, pg, v, m_pBCStep->GetID());
			char szname[64] = { 0 };
			sprintf(szname, "InitialVelocity%02d", CountBCs<FSNodalVelocities>(fem) + 1);
			if (pg) { pg->SetName(szname); pm->AddFENodeSet(pg); }
			pbc->SetName(szname);
			m_pBCStep->AddComponent(pbc);
		}
		else if (tag == "concentration")	// initial concentration BC
		{
			FSNodeSet* pg = 0;
			FSMesh* pm = &GetFEMesh();
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
					if (pg == 0) pg = new FSNodeSet(pm);
					pg->add(id);
				}
				else ParseUnknownTag(tag);
				++tag;
			} while (!tag.isend());

			// create a new initial velocity BC
			FSInitConcentration* pbc = new FSInitConcentration(&fem, pg, bc, c, m_pBCStep->GetID());
			char szname[64] = { 0 };
			sprintf(szname, "InitialConcentration%02d", CountICs<FSInitConcentration>(fem) + 1);
			if (pg) { pg->SetName(szname); pm->AddFENodeSet(pg); }
			pbc->SetName(szname);
			m_pBCStep->AddComponent(pbc);
		}
		else if (tag == "fluid_pressure")	// initial fluid pressure
		{
			FSNodeSet* pg = 0;
			FSMesh* pm = &GetFEMesh();
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
					if (pg == 0) pg = new FSNodeSet(pm);
					pg->add(id);
				}
				else ParseUnknownTag(tag);
				++tag;
			} while (!tag.isend());

			// create a new initial BC
			FSInitFluidPressure* pbc = new FSInitFluidPressure(&fem, pg, p, m_pBCStep->GetID());
			char szname[64] = { 0 };
			sprintf(szname, "InitialFluidPressure%02d", CountBCs<FSInitFluidPressure>(fem) + 1);
			pbc->SetName(szname);
			if (pg) { pg->SetName(szname); pm->AddFENodeSet(pg); }
			m_pBCStep->AddComponent(pbc);
		}
		else if (tag == "temperature")	// initial temperature
		{
			FSNodeSet* pg = 0;
			FSMesh* pm = &GetFEMesh();
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
					if (pg == 0) pg = new FSNodeSet(pm);
					pg->add(id);
				}
				else ParseUnknownTag(tag);
				++tag;
			} while (!tag.isend());

			// create a new initial BC
			FSInitTemperature* pbc = new FSInitTemperature(&fem, pg, p, m_pBCStep->GetID());
			char szname[64] = { 0 };
			sprintf(szname, "InitialTemperature%02d", CountBCs<FSInitTemperature>(fem) + 1);
			pbc->SetName(szname);
			if (pg) { pg->SetName(szname); pm->AddFENodeSet(pg); }
			m_pBCStep->AddComponent(pbc);
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
void FEBioFormatOld::ParseContact(FSStep *pstep, XMLTag &tag)
{
	XMLAtt& atype = tag.Attribute("type");
	if      (atype == "sliding_with_gaps"          ) ParseContactSliding    (pstep, tag);
	else if (atype == "facet-to-facet sliding"     ) ParseContactF2FSliding (pstep, tag);
	else if (atype == "sliding2"                   ) ParseContactBiphasic   (pstep, tag);
	else if (atype == "sliding3"                   ) ParseContactSolute     (pstep, tag);
	else if (atype == "sliding-multiphasic"        ) ParseContactMultiphasic(pstep, tag);
	else if (atype == "tied"                       ) ParseContactTied       (pstep, tag);
	else if (atype == "sticky"                     ) ParseContactSticky     (pstep, tag);
	else if (atype == "periodic boundary"          ) ParseContactPeriodic   (pstep, tag);
	else if (atype == "rigid"                      ) ParseContactRigid      (pstep, tag);
	else if (atype == "rigid joint"                ) ParseContactJoint      (pstep, tag);
	else if (atype == "sliding-tension-compression") ParseContactTC         (pstep, tag);
	else if (atype == "tied-biphasic"              ) ParseContactTiedPoro   (pstep, tag);
	else if (atype == "rigid_wall"                 ) ParseRigidWall         (pstep, tag);
	else if (atype == "linear constraint"          ) ParseLinearConstraint  (pstep, tag);
	else ParseUnknownTag(tag);
}

//-----------------------------------------------------------------------------
FSSurface* FEBioFormatOld::ParseContactSurface(XMLTag& tag)
{
	FSMesh* pm = &GetFEMesh();
	FEBioMesh& mesh = GetFEBioMesh();

	// see if the set is defined 
	if (tag.isempty())
	{
		// get the surface name
		const char* szset = tag.AttributeValue("set");

		// find the surface
		FSSurface* psurf = pm->FindFESurface(szset);
		if (psurf == nullptr) throw XMLReader::InvalidAttributeValue(tag, "set", szset);

		return psurf;
	}
	else
	{
		FSSurface* ps = new FSSurface(pm);
		pm->AddFESurface(ps);

		// count nr of faces
		int faces = 0, N = 0, nf[8], m;
		XMLTag t(tag); ++t;
		while (!t.isend()) { faces++; ++t; }

		// read faces
		++tag;
		for (int i = 0; i<faces; ++i)
		{
			// read face data
			if (tag == "quad4") N = 4;
			else if (tag == "quad8") N = 8;
			else if (tag == "quad9") N = 9;
			else if (tag == "tri3") N = 3;
			else if (tag == "tri6") N = 6;
			else if (tag == "tri7") N = 7;
			else assert(false);

			tag.value(nf, N);
			for (int j = 0; j<N; ++j) --nf[j];
			if (N == 3) nf[3] = nf[2];

			// find slave surface elements
			m = mesh.FindFace(nf, N);
			if (m >= 0) ps->add(m);

			++tag;
		}

		return ps;
	}
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseContactSliding(FSStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();

	char szbuf[256];

	// create new sliding interface
	FSSlidingWithGapsInterface* pi = new FSSlidingWithGapsInterface(&fem, pstep->GetID());

	// get the (optional) contact name
	const char* szname = tag.AttributeValue("name", true);
	if (szname) sprintf(szbuf, "%s", szname);
	else sprintf(szbuf, "SlidingInterface%02d", CountInterfaces<FSSlidingWithGapsInterface>(fem) +1);
	pi->SetName(szbuf);

	FSSurface *pms = 0, *pss = 0;
	FSMesh* pm = &GetFEMesh();

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

				// create a new surface
				FSSurface* ps = ParseContactSurface(tag);
				if (ntype == 1)
				{
					pms = ps;
					if (szn) ps->SetName(szn);
					else {
						sprintf(szbuf, "SecondarySurface%02d", CountInterfaces<FSSlidingWithGapsInterface>(fem) + 1);
						ps->SetName(szbuf);
					}
					pi->SetSecondarySurface(ps);
				}
				else
				{
					pss = ps;
					if (szn) ps->SetName(szn);
					else {
						sprintf(szbuf, "PrimarySurface%02d", CountInterfaces<FSSlidingWithGapsInterface>(fem) + 1);
						ps->SetName(szbuf);
					}
					pi->SetPrimarySurface(ps);
				}
			}
			else ParseUnknownTag(tag);
		}
		++tag;
	} while (!tag.isend());

	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseContactF2FSliding(FSStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();

	char szbuf[256];

	// create new sliding interface
	FSFacetOnFacetInterface* pi = new FSFacetOnFacetInterface(&fem, pstep->GetID());

	// get the (optional) contact name
	const char* szname = tag.AttributeValue("name", true);
	if (szname) sprintf(szbuf, "%s", szname);
	else sprintf(szbuf, "SlidingContact%02d", CountInterfaces<FSFacetOnFacetInterface>(fem) +1);
	pi->SetName(szbuf);

	FSSurface *pms = 0, *pss = 0;
	FSMesh* pm = &GetFEMesh();

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

				// create a new surface
				FSSurface* ps = ParseContactSurface(tag);
				if (ntype == 1)
				{
					pms = ps;
					if (szn) ps->SetName(szn);
					pi->SetSecondarySurface(ps);
				}
				else
				{
					pss = ps;
					if (szn) ps->SetName(szn);
					pi->SetPrimarySurface(ps);
				}
			}
			else ParseUnknownTag(tag);
		}
		++tag;
	} while (!tag.isend());

	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseContactBiphasic(FSStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();

	// create new contact interface
	FSPoroContact* pi = new FSPoroContact(&fem, pstep->GetID());

	// read the name
	char szname[256];
	sprintf(szname, "BiphasicContact%02d", CountInterfaces<FSPoroContact>(fem) + 1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	FSSurface *pms = 0, *pss = 0;
	FSMesh* pm = &GetFEMesh();

	++tag;
	do
	{
		// Read parameters
		if (ReadParam(*pi, tag) == false)
		{
			if (tag == "surface")
			{
				const char* sztype = tag.AttributeValue("type");
				int ntype = 0;
				if (strcmp(sztype, "master") == 0) ntype = 1;
				else if (strcmp(sztype, "slave") == 0) ntype = 2;

				const char* szn = tag.AttributeValue("name", true);

				FSSurface* ps = ParseContactSurface(tag); assert(ps);
				if (ntype == 1)
				{
					pms = ps;
					if (szn) ps->SetName(szn);
					pi->SetSecondarySurface(ps);
				}
				else
				{
					pss = ps;
					if (szn) ps->SetName(szn);
					pi->SetPrimarySurface(ps);
				}
			}
		}
		++tag;
	} while (!tag.isend());

	// add interface to step
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseContactSolute(FSStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();

	// create new interface
	FSPoroSoluteContact* pi = new FSPoroSoluteContact(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "BiphasicSoluteContact%02d", CountInterfaces<FSPoroSoluteContact>(fem) + 1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	FSSurface *pms = 0, *pss = 0;
	FSMesh* pm = &GetFEMesh();

	++tag;
	do
	{
		// read parameters
		if (ReadParam(*pi, tag) == false)
		{
			if (tag == "surface")
			{
				const char* sztype = tag.AttributeValue("type");
				int ntype = 0;
				if (strcmp(sztype, "master") == 0) ntype = 1;
				else if (strcmp(sztype, "slave") == 0) ntype = 2;

				const char* szn = tag.AttributeValue("name", true);

				FSSurface* ps = ParseContactSurface(tag); assert(ps);
				if (ntype == 1)
				{
					pms = ps;
					if (szn) ps->SetName(szn);
					pi->SetSecondarySurface(ps);
				}
				else
				{
					pss = ps;
					if (szn) ps->SetName(szn);
					pi->SetPrimarySurface(ps);
				}
			}
		}
		++tag;
	} while (!tag.isend());

	// add interface to step
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseContactMultiphasic(FSStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();

	// create new interface
	FSMultiphasicContact* pi = new FSMultiphasicContact(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "MultiphasicContact%02d", CountInterfaces<FSMultiphasicContact>(fem) + 1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	FSSurface *pms = 0, *pss = 0;
	FSMesh* pm = &GetFEMesh();
	
	++tag;
	do
	{
		// read parameters
		if (ReadParam(*pi, tag) == false)
		{
			if (tag == "surface")
			{
				const char* sztype = tag.AttributeValue("type");
				int ntype = 0;
				if (strcmp(sztype, "master") == 0) ntype = 1;
				else if (strcmp(sztype, "slave") == 0) ntype = 2;

				const char* szn = tag.AttributeValue("name", true);

				FSSurface* ps = ParseContactSurface(tag); assert(ps);
				if (ntype == 1)
				{
					pms = ps;
					if (szn) ps->SetName(szn);
					pi->SetSecondarySurface(ps);
				}
				else
				{
					pss = ps;
					if (szn) ps->SetName(szn);
					pi->SetPrimarySurface(ps);
				}
			}
		}
		++tag;
	} while (!tag.isend());

	// add interface to step
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseContactTied(FSStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();

	// create new interface
	FSTiedInterface* pi = new FSTiedInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "TiedInterface%02d", CountInterfaces<FSTiedInterface>(fem) + 1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	FSMesh* pm = &GetFEMesh();
	FSSurface *pms = 0, *pss = 0;
	
	++tag;
	do
	{
		// read parameters
		if (ReadParam(*pi, tag) == false)
		{
			if (tag == "surface")
			{
				const char* sztype = tag.AttributeValue("type");
				int ntype = 0;
				if (strcmp(sztype, "master") == 0) ntype = 1;
				else if (strcmp(sztype, "slave") == 0) ntype = 2;

				const char* szn = tag.AttributeValue("name", true);

				FSSurface* ps = ParseContactSurface(tag); assert(ps);
				if (ntype == 1)
				{
					pms = ps;
					if (szn) ps->SetName(szn);
					pi->SetSecondarySurface(ps);
				}
				else
				{
					pss = ps;
					if (szn) ps->SetName(szn);
					pi->SetPrimarySurface(ps);
				}
			}
		}
		++tag;
	} while (!tag.isend());

	// add interface to step
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseContactSticky(FSStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();

	// create new interface
	FSStickyInterface* pi = new FSStickyInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "StickyInterface%02d", CountInterfaces<FSStickyInterface>(fem) + 1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	FSMesh* pm = &GetFEMesh();
	FSSurface *pms = 0, *pss = 0;

	++tag;
	do
	{
		// read parameters
		if (ReadParam(*pi, tag) == false)
		{
			if (tag == "surface")
			{
				const char* sztype = tag.AttributeValue("type");
				int ntype = 0;
				if (strcmp(sztype, "master") == 0) ntype = 1;
				else if (strcmp(sztype, "slave") == 0) ntype = 2;

				const char* szn = tag.AttributeValue("name", true);

				FSSurface* ps = ParseContactSurface(tag); assert(ps);
				if (ntype == 1)
				{
					pms = ps;
					if (szn) ps->SetName(szn);
					pi->SetSecondarySurface(ps);
				}
				else
				{
					pss = ps;
					if (szn) ps->SetName(szn);
					pi->SetPrimarySurface(ps);
				}
			}
		}
		++tag;
	} while (!tag.isend());

	// add interface to step
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseContactPeriodic(FSStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();

	// create new interface
	FSPeriodicBoundary* pi = new FSPeriodicBoundary(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "PeriodicBoundary%02d", CountInterfaces<FSPeriodicBoundary>(fem) + 1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	FSMesh* pm = &GetFEMesh();
	FSSurface *pms = 0, *pss = 0;

	++tag;
	do
	{
		// read parameters
		if (ReadParam(*pi, tag) == false)
		{
			if (tag == "surface")
			{
				const char* sztype = tag.AttributeValue("type");
				int ntype = 0;
				if (strcmp(sztype, "master") == 0) ntype = 1;
				else if (strcmp(sztype, "slave") == 0) ntype = 2;

				const char* szn = tag.AttributeValue("name", true);

				FSSurface* ps = ParseContactSurface(tag); assert(ps);
				if (ntype == 1)
				{
					pms = ps;
					if (szn) ps->SetName(szn);
					pi->SetSecondarySurface(ps);
				}
				else
				{
					pss = ps;
					if (szn) ps->SetName(szn);
					pi->SetPrimarySurface(ps);
				}
			}
		}
		++tag;
	} while (!tag.isend());

	// add interface to step
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseContactTC(FSStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();

	// create new interface
	FSTensionCompressionInterface* pi = new FSTensionCompressionInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "TCInterface%02d", CountInterfaces<FSTensionCompressionInterface>(fem) + 1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	FSMesh* pm = &GetFEMesh();
	FSSurface *pms = 0, *pss = 0;

	++tag;
	do
	{
		// read parameters
		if (ReadParam(*pi, tag) == false)
		{
			if (tag == "surface")
			{
				const char* sztype = tag.AttributeValue("type");
				int ntype = 0;
				if (strcmp(sztype, "master") == 0) ntype = 1;
				else if (strcmp(sztype, "slave") == 0) ntype = 2;

				const char* szn = tag.AttributeValue("name", true);

				FSSurface* ps = ParseContactSurface(tag); assert(ps);
				if (ntype == 1)
				{
					pms = ps;
					if (szn) ps->SetName(szn);
					pi->SetSecondarySurface(ps);
				}
				else
				{
					pss = ps;
					if (szn) ps->SetName(szn);
					pi->SetPrimarySurface(ps);
				}
			}
		}
		++tag;
	} while (!tag.isend());

	// add interface to step
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseContactTiedPoro(FSStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();

	// create new interface
	FSTiedBiphasicInterface* pi = new FSTiedBiphasicInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "TiedBiphasicInterface%02d", CountInterfaces<FSTiedBiphasicInterface>(fem) + 1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	FSMesh* pm = &GetFEMesh();
	FSSurface *pms = 0, *pss = 0;

	++tag;
	do
	{
		// read parameters
		if (ReadParam(*pi, tag) == false)
		{
			if (tag == "surface")
			{
				const char* sztype = tag.AttributeValue("type");
				int ntype = 0;
				if (strcmp(sztype, "master") == 0) ntype = 1;
				else if (strcmp(sztype, "slave") == 0) ntype = 2;

				const char* szn = tag.AttributeValue("name", true);

				FSSurface* ps = ParseContactSurface(tag); assert(ps);
				if (ntype == 1)
				{
					pms = ps;
					if (szn) ps->SetName(szn);
					pi->SetSecondarySurface(ps);
				}
				else
				{
					pss = ps;
					if (szn) ps->SetName(szn);
					pi->SetPrimarySurface(ps);
				}
			}
		}
		++tag;
	} while (!tag.isend());

	// add interface to step
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseRigidWall(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// create a new interface
	FSRigidWallInterface* pci = new FSRigidWallInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "RigidWall%02d", CountInterfaces<FSRigidWallInterface>(fem) + 1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pci->SetName(szname);

	FSMesh* pm = &GetFEMesh();
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

			const char* szlc = tag.AttributeValue("lc", true);
			if (szlc)
			{
				febio.AddParamCurve(&pci->GetParam(FSRigidWallInterface::OFFSET), atoi(szlc) - 1);
			}
		}
		else if (tag == "surface")
		{
			const char* szn = tag.AttributeValue("name", true);

			FSSurface* ps = ParseContactSurface(tag); assert(ps);
			pci->SetItemList(ps);
			if (szn) ps->SetName(szn);
		}
		++tag;
	} while (!tag.isend());

	// add interface to step
	pstep->AddComponent(pci);
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseContactRigid(FSStep *pstep, XMLTag &tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();
	FSMesh* pm = &GetFEMesh();

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
	} while (!tag.isend());

	FSMesh& mesh = GetFEMesh();
	int NN = mesh.Nodes();
	char szbuf[256];
	for (int i = 0; i<NMAT; ++i)
	{
		GMaterial* pmat = febio.GetMaterial(i);
		if (pmat && (pmat->m_ntag > 0))
		{
			int id = pmat->GetID();
			// create the node set
			FSNodeSet* pn = new FSNodeSet(pm);

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
			pn->SetName(szbuf);
			pm->AddFENodeSet(pn);
			pstep->AddComponent(pi);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseContactJoint(FSStep *pstep, XMLTag &tag)
{
	FEBioInputModel& febio = GetFEBioModel();

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

	if (na >= 0) pi->m_pbodyA = febio.GetMaterial(na - 1);
	if (nb >= 0) pi->m_pbodyB = febio.GetMaterial(nb - 1);
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseSprings(FSStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();
	GModel& gm = fem.GetModel();

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

	GMeshObject* po = GetGObject();
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
//							  pg->GetParam(GGeneralSpring::MP_F).GetLoadCurve()->SetID(lc);
							  pd = pg;
	}
		break;
	}

	fem.GetModel().AddDiscreteObject(pd);
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseLinearConstraint(FSStep* pstep, XMLTag& tag)
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

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseBodyForce(FSStep *pstep, XMLTag &tag)
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
void FEBioFormatOld::ParseHeatSource(FSStep *pstep, XMLTag &tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

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

			Param* p = &phs->GetParam(FSHeatSource::LOAD);
			febio.AddParamCurve(p, atoi(szlc) - 1);
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
bool FEBioFormatOld::ParseConstraintSection(XMLTag& tag)
{
	if (tag.isleaf()) return true;

	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = febio.GetFSModel();

	FSStep* pstep = m_pBCStep;

	static int nrd = 1;
	static int nrf = 1;
	char sz[256] = {0};
	double v;

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

			FSRigidFixed* pc = 0; // fixed constraint

			++tag;
			do
			{
				if (tag == "trans_x")
				{
					int lc = tag.AttributeValue<int>("lc", 0);

					XMLAtt& atype = tag.Attribute("type");
					if (atype == "fixed")
					{
						if (pc == 0) pc = new FSRigidFixed(&fem, pstep->GetID());
						pc->SetDOF(0, true);
					}
					else if (atype == "prescribed")
					{
						tag.value(v);
						FSRigidDisplacement* pd = new FSRigidDisplacement(0, matid, v, pstep->GetID());
						febio.AddParamCurve(&pd->GetParam(FSRigidDisplacement::VALUE), lc-1);

						sprintf(sz, "RigidDisplacement%02d", nrd++);
						pd->SetName(sz);
						pstep->AddRC(pd);
					}
					else if (atype == "force")
					{
						tag.value(v);
						FSRigidForce* pd = new FSRigidForce(0, matid, v, pstep->GetID());
						febio.AddParamCurve(&pd->GetParam(FSRigidDisplacement::VALUE), lc - 1);

						sprintf(sz, "RigidForce%02d", nrf++);
						pd->SetName(sz);
						pstep->AddRC(pd);
					}
				}
				else if (tag == "trans_y")
				{
					int lc = tag.AttributeValue<int>("lc", 0);

					XMLAtt& atype = tag.Attribute("type");
					if (atype == "fixed")
					{
						if (pc == 0) pc = new FSRigidFixed(&fem, pstep->GetID());
						pc->SetDOF(1, true);
					}
					else if (atype == "prescribed")
					{
						tag.value(v);
						FSRigidDisplacement* pd = new FSRigidDisplacement(1, matid, v, pstep->GetID());
						febio.AddParamCurve(&pd->GetParam(FSRigidDisplacement::VALUE), lc - 1);

						sprintf(sz, "RigidDisplacement%02d", nrd++);
						pd->SetName(sz);
						pstep->AddRC(pd);
					}
					else if (atype == "force")
					{
						tag.value(v);
						FSRigidForce* pd = new FSRigidForce(1, matid, v, pstep->GetID());
						febio.AddParamCurve(&pd->GetParam(FSRigidDisplacement::VALUE), lc - 1);

						sprintf(sz, "RigidForce%02d", nrf++);
						pd->SetName(sz);
						pstep->AddRC(pd);
					}
				}
				else if (tag == "trans_z")
				{
					int lc = tag.AttributeValue<int>("lc", 0);

					XMLAtt& atype = tag.Attribute("type");
					if (atype == "fixed")
					{
						if (pc == 0) pc = new FSRigidFixed(&fem, pstep->GetID());
						pc->SetDOF(2, true);
					}
					else if (atype == "prescribed")
					{
						tag.value(v);
						FSRigidDisplacement* pd = new FSRigidDisplacement(2, matid, v, pstep->GetID());
						febio.AddParamCurve(&pd->GetParam(FSRigidDisplacement::VALUE), lc - 1);

						sprintf(sz, "RigidDisplacement%02d", nrd++);
						pd->SetName(sz);
						pstep->AddRC(pd);
					}
					else if (atype == "force")
					{
						tag.value(v);
						FSRigidForce* pd = new FSRigidForce(2, matid, v, pstep->GetID());
						febio.AddParamCurve(&pd->GetParam(FSRigidDisplacement::VALUE), lc - 1);

						sprintf(sz, "RigidForce%02d", nrf++);
						pd->SetName(sz);
						pstep->AddRC(pd);
					}
				}
				else if (tag == "rot_x")
				{
					int lc = tag.AttributeValue<int>("lc", 0);

					XMLAtt& atype = tag.Attribute("type");
					if (atype == "fixed")
					{
						if (pc == 0) pc = new FSRigidFixed(&fem, pstep->GetID());
						pc->SetDOF(3, true);
					}
					else if (atype == "prescribed")
					{
						tag.value(v);
						FSRigidDisplacement* pd = new FSRigidDisplacement(3, matid, v, pstep->GetID());
						febio.AddParamCurve(&pd->GetParam(FSRigidDisplacement::VALUE), lc - 1);

						sprintf(sz, "RigidDisplacement%02d", nrd++);
						pd->SetName(sz);
						pstep->AddRC(pd);
					}
					else if (atype == "force")
					{
						tag.value(v);
						FSRigidForce* pd = new FSRigidForce(3, matid, v, pstep->GetID());
						febio.AddParamCurve(&pd->GetParam(FSRigidDisplacement::VALUE), lc - 1);

						sprintf(sz, "RigidForce%02d", nrf++);
						pd->SetName(sz);
						pstep->AddRC(pd);
					}
				}
				else if (tag == "rot_y")
				{
					int lc = tag.AttributeValue<int>("lc", 0);

					XMLAtt& atype = tag.Attribute("type");
					if (atype == "fixed")
					{
						if (pc == 0) pc = new FSRigidFixed(&fem, pstep->GetID());
						pc->SetDOF(4, true);
					}
					else if (atype == "prescribed")
					{
						tag.value(v);
						FSRigidDisplacement* pd = new FSRigidDisplacement(4, matid, v, pstep->GetID());
						febio.AddParamCurve(&pd->GetParam(FSRigidDisplacement::VALUE), lc - 1);

						sprintf(sz, "RigidDisplacement%02d", nrd++);
						pd->SetName(sz);
						pstep->AddRC(pd);
					}
					else if (atype == "force")
					{
						tag.value(v);
						FSRigidForce* pd = new FSRigidForce(4, matid, v, pstep->GetID());
						febio.AddParamCurve(&pd->GetParam(FSRigidDisplacement::VALUE), lc - 1);

						sprintf(sz, "RigidForce%02d", nrf++);
						pd->SetName(sz);
						pstep->AddRC(pd);
					}
				}
				else if (tag == "rot_z")
				{
					int lc = tag.AttributeValue<int>("lc", 0);

					XMLAtt& atype = tag.Attribute("type");
					if (atype == "fixed")
					{
						if (pc == 0) pc = new FSRigidFixed(&fem, pstep->GetID());
						pc->SetDOF(5, true);
					}
					else if (atype == "prescribed")
					{
						tag.value(v);
						FSRigidDisplacement* pd = new FSRigidDisplacement(5, matid, v, pstep->GetID());
						febio.AddParamCurve(&pd->GetParam(FSRigidDisplacement::VALUE), lc - 1);

						sprintf(sz, "RigidDisplacement%02d", nrd++);
						pd->SetName(sz);
						pstep->AddRC(pd);
					}
					else if (atype == "force")
					{
						tag.value(v);
						FSRigidForce* pd = new FSRigidForce(5, matid, v, pstep->GetID());
						febio.AddParamCurve(&pd->GetParam(FSRigidDisplacement::VALUE), lc - 1);

						sprintf(sz, "RigidForce%02d", nrf++);
						pd->SetName(sz);
						pstep->AddRC(pd);
					}
				}
				else ParseUnknownTag(tag);

				++tag;
			} while (!tag.isend());

			if (pc)
			{
				static int n = 1;
				char sz[256];
				sprintf(sz, "RigidFixed%02d", n++);
				pc->SetMaterialID(pgm ? pgm->GetID() : -1);
				pc->SetName(sz);
				pstep->AddRC(pc);
			}
		}
		else ParseUnknownTag(tag);
		++tag;
	}
	while (!tag.isend());

	return true;
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseRigidConstraint(FSStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel* fem = &febio.GetFSModel();
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

	++tag;
	do
	{
		if (tag == "initial_velocity")
		{
			FSRigidVelocity* pv = new FSRigidVelocity(fem, pstep->GetID());
			vec3d vi;
			tag.value(vi);
			pv->SetVelocity(vi);

			static int n = 1;
			if (hasName == false) sprintf(szname, "RigidVelocity%02d", n++);
			pv->SetName(szname);
			pstep->AddRC(pv);
		}
		else if (tag == "initial_angular_velocity")
		{
			FSRigidAngularVelocity* pv = new FSRigidAngularVelocity(fem, pstep->GetID());
			vec3d vi;
			tag.value(vi);
			pv->SetVelocity(vi);

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
				if (pc == 0) pc = new FSRigidFixed(fem, pstep->GetID());
				pc->SetDOF(nbc, true);
			}
			else if (tag == "prescribed")
			{
				int lc = tag.AttributeValue<int>("lc", 0);
				double v;
				tag.value(v);

				FSRigidDisplacement* pd = new FSRigidDisplacement(nbc, matid, v, pstep->GetID());
				febio.AddParamCurve(&pd->GetParam(FSRigidDisplacement::VALUE), lc - 1);

				static int n = 1;
				if (hasName == false) sprintf(szname, "RigidDisplacement%02d", n++);
				pd->SetName(szname);
				pstep->AddRC(pd);
			}
			else if (tag == "force")
			{
				int lc = tag.AttributeValue<int>("lc", 0);
				double v;
				tag.value(v);

				FSRigidForce* pd = new FSRigidForce(nbc, matid, v, pstep->GetID());
				febio.AddParamCurve(&pd->GetParam(FSRigidDisplacement::VALUE), lc - 1);

				static int n = 1;
				if (hasName == false) sprintf(szname, "RigidForce%02d", n++);
				pd->SetName(szname);
				pstep->AddRC(pd);
			}
			else ParseUnknownTag(tag);
		}

		++tag;
	} while (!tag.isend());

	if (pc)
	{
		static int n = 1;
		pc->SetMaterialID(matid);
		if (hasName == false) sprintf(szname, "RigidConstraint%02d", n++);
		pc->SetName(szname);
		pstep->AddRC(pc);
	}
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseVolumeConstraint(FSStep* pstep, XMLTag& tag)
{
	// make sure there is something to read
	if (tag.isempty()) return;

	FSModel& fem = GetFSModel();

	// create a new volume constraint
	FSVolumeConstraint* pi = new FSVolumeConstraint(&fem, pstep->GetID());
	pstep->AddComponent(pi);

	// get the (optional) contact name
	char szbuf[256];
	const char* szname = tag.AttributeValue("name", true);
	if (szname) sprintf(szbuf, "%s", szname);
	else sprintf(szbuf, "VolumeConstraint%02d", CountConstraints<FSVolumeConstraint>(fem));
	pi->SetName(szbuf);

	// get the mesh (need it for defining the surface)
	FSMesh* pm = &GetFEMesh();

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

				// process the surface
				FSSurface* ps = ParseContactSurface(tag);
				if (ps)
				{
					if (ps->GetName().empty())
					{
						sprintf(szbuf, "VolumeConstraintSurface%02d", CountConstraints<FSVolumeConstraint>(fem));
						ps->SetName(szn ? szn : szbuf);
					}

					// assign the surface
					pi->SetItemList(ps);
				}
			}
			else ParseUnknownTag(tag);
		}

		// go to the next tag
		++tag;
	} 
	while (!tag.isend());
}

//=============================================================================
//
//                                S T E P
//
//=============================================================================

//-----------------------------------------------------------------------------
bool FEBioFormatOld::ParseStepSection(XMLTag &tag)
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
		else ParseUnknownTag(tag);

		// go to the next tag
		++tag;
	} 
	while (!tag.isend());

	m_pstep = 0;
	m_pBCStep = 0;

	return true;
}
