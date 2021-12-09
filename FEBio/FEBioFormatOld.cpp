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
#include <MeshTools/GDiscreteObject.h>
#include <MeshTools/GModel.h>

FEBioFormatOld::FEBioFormatOld(FEBioImport* fileReader, FEBioInputModel& febio) : FEBioFormat(fileReader, febio)
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
	return (m_nAnalysis != -1);
}

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
	FEMesh* pm = part.GetFEMesh();

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
					FENode& node = pm->Node(i);
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
				int n[FEElement::MAX_NODES];
				for (int i = 0; i<elems; ++i)
				{
					// read element type
					FEElement& el = pm->Element(i);
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
							FEElement& el = pm->Element(id);

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
							FEElement& el = pm->Element(id);

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
							FEElement& el = pm->Element(id);
							if (!el.IsShell()) return false;
							tag.value(el.m_h, el.Nodes());
						}
						else if (tag == "area")
						{
							FEElement& el = pm->Element(id);
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
		FEElement& e = pm->Element(i);
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
void FEBioFormatOld::ParseBCFixed(FEStep* pstep, XMLTag &tag)
{
	char szname[256] = { 0 };
	int i;

	// count how many fixed nodes there are
	int nfix = 0;
	XMLTag t(tag); ++t;
	while (!t.isend()) { nfix++; ++t; }

	FEMesh* pm = &GetFEMesh();
	int N = pm->Nodes();
	vector<int> BC; BC.resize(N);
	for (i = 0; i<N; ++i) BC[i] = 0;

	FSModel& fem = GetFSModel();

	GMeshObject* po = GetGObject();

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
			FENodeSet* pg = new FENodeSet(po);

			// assign the nodes to this group
			for (int i = 0; i<N; ++i) if (BC[i] & ntype) { pg->add(i); BC[i] &= (~ntype); }

			// create the constraint
			if (ntype < 8)
			{
				FEFixedDisplacement* pbc = new FEFixedDisplacement(&fem, pg, ntype, pstep->GetID());
				sprintf(szname, "FixedDisplacement%02d", CountBCs<FEFixedDisplacement>(fem)+1);
				pbc->SetName(szname);
				pstep->AddComponent(pbc);
			}
			else if (ntype < 64)
			{
				ntype = ntype >> 3;
				FEFixedRotation* pbc = new FEFixedRotation(&fem, pg, ntype, pstep->GetID());
				sprintf(szname, "FixedRotation%02d", CountBCs<FEFixedRotation>(fem)+1);
				pbc->SetName(szname);
				pstep->AddComponent(pbc);
			}
			else if (ntype == 64)
			{
				FEFixedTemperature* pbc = new FEFixedTemperature(&fem, pg, 1, pstep->GetID());
				sprintf(szname, "FixedTemperature%02d", CountBCs<FEFixedTemperature>(fem)+1);
				pbc->SetName(szname);
				pstep->AddComponent(pbc);
			}
			else if (ntype == 128)
			{
				FEFixedFluidPressure* pbc = new FEFixedFluidPressure(&fem, pg, 1, pstep->GetID());
				sprintf(szname, "FixedFluidPressure%02d", CountBCs<FEFixedFluidPressure>(fem)+1);
				pbc->SetName(szname);
				pstep->AddComponent(pbc);
			}
			else
			{
				ntype = ntype >> 8;
				if (ntype < 256)
				{
					FEFixedConcentration* pbc = new FEFixedConcentration(&fem, pg, ntype, pstep->GetID());
					sprintf(szname, "FixedConcentration%02d", CountBCs<FEFixedConcentration>(fem)+1);
					pbc->SetName(szname);
					pstep->AddComponent(pbc);
				}
			}
		}
	} while (nbc);
}


//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseBCPrescribed(FEStep* pstep, XMLTag& tag)
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
	FSModel& fem = GetFSModel();
	char szname[256];
	vector<FEPrescribedDOF*> pBC(nns);
	vector<FENodeSet*> pNS(nns);
	nns = 0;
	FEMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();
	for (i = 0; i<lcmax; ++i)
	{
		for (j = 0; j<MAXBC; ++j)
		if (cc[j][i] >= 0)
		{
			// make a new group
			FENodeSet* pg = new FENodeSet(po);
			sprintf(szname, "Nodeset%02d", i + 1);
			pg->SetName(szname);

			// make a new boundary condition
			FEPrescribedDOF* pbc = 0;
			switch (j)
			{
			case 0:
				{
					FEPrescribedDisplacement * pd = new FEPrescribedDisplacement(&fem, pg, j, 1, pstep->GetID());
					pd->SetRelativeFlag(brel);
					pbc = pd;
				}
				break;
			case 1:
				{
					FEPrescribedDisplacement* pd = new FEPrescribedDisplacement(&fem, pg, j, 1, pstep->GetID());
					pd->SetRelativeFlag(brel);
					pbc = pd;
				}
				break;
			case 2:
				{
					FEPrescribedDisplacement* pd = new FEPrescribedDisplacement(&fem, pg, j, 1, pstep->GetID());
					pd->SetRelativeFlag(brel);
					pbc = pd;
				}
				break;
			case 3:
				{
					FEPrescribedTemperature* pd = new FEPrescribedTemperature(&fem, pg, 1, pstep->GetID());
					pd->SetRelativeFlag(brel);
					pbc = pd;
				}
				break;
			case 4:
				{
					FEPrescribedFluidPressure* pd = new FEPrescribedFluidPressure(&fem, pg, 1, pstep->GetID());
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
					FEPrescribedConcentration* pd = new FEPrescribedConcentration(&fem, pg, bc, 1.0, pstep->GetID());
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

		FEPrescribedDOF* pbc = pBC[ng];
		pbc->GetLoadCurve()->SetID(lc);
		pbc->SetScaleFactor(DC[i].s);
	}
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseForceLoad(FEStep *pstep, XMLTag &tag)
{
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
	vector<FENodalDOFLoad*> pFC(nns);
	vector<FENodeSet*> pNS(nns);
	FEMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();
	for (i = 0; i<lcmax; ++i)
	{
		for (j = 0; j<MAXBC; ++j)
		if (cc[j][i] >= 0)
		{
			FENodeSet* pg = new FENodeSet(po);
			sprintf(szname, "ForceNodeset%02d", i + 1);
			pg->SetName(szname);

			FENodalDOFLoad* pbc = new FENodalDOFLoad(&fem, pg, j, 1, pstep->GetID());
			sprintf(szname, "ForceLoad%02d", i + 1);
			pbc->SetName(szname);
			pFC[cc[j][i]] = pbc;
			pNS[cc[j][i]] = pg;
			pstep->AddComponent(pbc);
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

		pFC[ng]->GetLoadCurve()->SetID(lc);
		pFC[ng]->SetLoad(FC[i].s);
		pNS[ng]->add(n);
	}
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParsePressureLoad(FEStep *pstep, XMLTag &tag)
{
	int i;

	// count how many pressure cards there are
	int npc = tag.children();

	FEBioMesh& mesh = GetFEBioMesh();

	FEMesh* pm = &GetFEMesh();
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
			FEFace& face = pm->Face(m);
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
		FEFace& face = pm->Face(i);
		int lc = face.m_ntag;
		if (lc >= 0) nlc[lc] = 1;
	}

	int ns = 0;
	for (i = 0; i<nmax; ++i) if (nlc[i] > 0) nlc[i] = ns++;

	FSModel& fem = GetFSModel();
	GMeshObject* po = GetGObject();

	// let's create the surfaces
	vector<FEPressureLoad*> pPC(ns);
	vector<FESurface*> pSF(ns);
	char szname[256];
	int npr = CountLoads<FEPressureLoad>(fem);
	for (i = 0; i<ns; ++i)
	{
		FESurface* ps = new FESurface(po);
		sprintf(szname, "PressureSurface%02d", npr + i + 1);
		ps->SetName(szname);

		FEPressureLoad* pbc = new FEPressureLoad(&fem, ps, pstep->GetID());
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
		FEFace& face = pm->Face(i);
		if (face.m_ntag >= 0)
		{
			int n = nlc[face.m_ntag];
			FELoadCurve* plc = pPC[n]->GetLoadCurve();
			FESurface* ps = pSF[n];
			ps->add(i);
			plc->SetID(face.m_ntag);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseTractionLoad(FEStep* pstep, XMLTag& tag)
{
	// count how many traction cards there are
	int ntc = tag.children();

	FEBioMesh& mesh = GetFEBioMesh();
	FEMesh* pm = &GetFEMesh();
	for (int i = 0; i<pm->Faces(); ++i) pm->Face(i).m_ntag = -1;

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
			FEFace& face = pm->Face(m);
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
		FEFace& face = pm->Face(i);
		int lc = face.m_ntag;
		if (lc >= 0) nlc[lc] = 1;
	}

	int ns = 0;
	for (int i = 0; i<nmax; ++i) if (nlc[i] > 0) nlc[i] = ns++;

	GMeshObject* po = GetGObject();

	// let's create the surfaces
	vector<FESurfaceTraction*> pPC(ns);
	vector<FESurface*> pSF(ns);
	char szname[256];
	int ntl = CountLoads<FESurfaceTraction>(fem);
	for (int i = 0; i<ns; ++i)
	{
		FESurface* ps = new FESurface(po);
		sprintf(szname, "TractionSurface%02d", ntl + i + 1);
		ps->SetName(szname);

		FESurfaceTraction* pbc = new FESurfaceTraction(&fem, ps, pstep->GetID());
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
		FEFace& face = pm->Face(i);
		if (face.m_ntag >= 0)
		{
			int n = nlc[face.m_ntag];
			FELoadCurve* plc = pPC[n]->GetLoadCurve();
			FESurface* ps = pSF[n];
			ps->add(i);
			plc->SetID(face.m_ntag);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseFluidFlux(FEStep *pstep, XMLTag &tag)
{
	// count how many fluid flux cards there are
	int nfc = tag.children();

	FEBioMesh& mesh = GetFEBioMesh();
	FSModel& fem = GetFSModel();
	FEMesh* pm = &GetFEMesh();
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
			FEFace& face = pm->Face(m);
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
		FEFace& face = pm->Face(i);
		int lc = face.m_ntag;
		if (lc >= 0) nlc[lc] = 1;
	}

	int ns = 0;
	for (int i = 0; i<nmax; ++i) if (nlc[i] > 0) nlc[i] = ns++;

	GMeshObject* po = GetGObject();

	// let's create the surfaces
	vector<FEFluidFlux*> pPC(ns);
	vector<FESurface*> pSF(ns);
	char szname[256];
	int nfl = CountLoads<FEFluidFlux>(fem);
	for (int i = 0; i<ns; ++i)
	{
		FESurface* ps = new FESurface(po);
		sprintf(szname, "FluidFluxSurface%02d", nfl + i + 1);
		ps->SetName(szname);

		FEFluidFlux* pbc = new FEFluidFlux(&fem, ps, pstep->GetID());
		sprintf(szname, "FluidFlux%02d", nfl + i + 1);
		pbc->SetLinearFlag(ntype == 1);
		pbc->SetMixtureFlag(nflux == 1);
		pbc->SetLoad(scl);
		pbc->SetName(szname);
		pPC[i] = pbc;
		pSF[i] = ps;
		pstep->AddComponent(pbc);
	}

	// set the correct face group ID's
	for (int i = 0; i<pm->Faces(); ++i)
	{
		FEFace& face = pm->Face(i);
		if (face.m_ntag >= 0)
		{
			int n = nlc[face.m_ntag];
			FELoadCurve* plc = pPC[n]->GetLoadCurve();
			FESurface* ps = pSF[n];
			ps->add(i);
			plc->SetID(face.m_ntag);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseBPNormalTraction(FEStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();

	// count how many cards there are
	int npc = tag.children();

	FEBioMesh& mesh = GetFEBioMesh();
	FEMesh* pm = &GetFEMesh();
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
			FEFace& face = pm->Face(m);
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
		FEFace& face = pm->Face(i);
		int lc = face.m_ntag;
		if (lc >= 0) nlc[lc] = 1;
	}

	int ns = 0;
	for (int i = 0; i<nmax; ++i) if (nlc[i] > 0) nlc[i] = ns++;

	GMeshObject* po = GetGObject();

	// let's create the surfaces
	vector<FEBPNormalTraction*> pPC(ns);
	vector<FESurface*> pSF(ns);
	char szname[256];
	int ntl = CountLoads<FEBPNormalTraction>(fem);
	for (int i = 0; i<ns; ++i)
	{
		FESurface* ps = new FESurface(po);
		sprintf(szname, "NormalTractionSurface%02d", ntl + i + 1);
		ps->SetName(szname);

		FEBPNormalTraction* pbc = new FEBPNormalTraction(&fem, ps, pstep->GetID());
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
		FEFace& face = pm->Face(i);
		if (face.m_ntag >= 0)
		{
			int n = nlc[face.m_ntag];
			FELoadCurve* plc = pPC[n]->GetLoadCurve();
			FESurface* ps = pSF[n];
			ps->add(i);
			plc->SetID(face.m_ntag);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseHeatFlux(FEStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();

	// count how many heat flux cards there are
	int npc = tag.children();

	FEBioMesh& mesh = GetFEBioMesh();

	FEMesh* pm = &GetFEMesh();
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
			FEFace& face = pm->Face(m);
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
		FEFace& face = pm->Face(i);
		int lc = face.m_ntag;
		if (lc >= 0) nlc[lc] = 1;
	}

	int ns = 0;
	for (int i = 0; i<nmax; ++i) if (nlc[i] > 0) nlc[i] = ns++;

	GMeshObject* po = GetGObject();

	// let's create the surfaces
	vector<FEHeatFlux*> pPC(ns);
	vector<FESurface*> pSF(ns);
	char szname[256];
	int nhf = CountLoads<FEHeatFlux>(fem);
	for (int i = 0; i<ns; ++i)
	{
		FESurface* ps = new FESurface(po);
		sprintf(szname, "HeatFluxSurface%02d", nhf + i + 1);
		ps->SetName(szname);

		FEHeatFlux* pbc = new FEHeatFlux(&fem, ps, pstep->GetID());
		sprintf(szname, "HeatFlux%02d", nhf + i + 1);
		pbc->SetName(szname);
		pbc->SetLoad(scl);
		pPC[i] = pbc;
		pSF[i] = ps;
		pstep->AddComponent(pbc);
	}

	// set the correct face group ID's
	for (int i = 0; i<pm->Faces(); ++i)
	{
		FEFace& face = pm->Face(i);
		if (face.m_ntag >= 0)
		{
			int n = nlc[face.m_ntag];
			FELoadCurve* plc = pPC[n]->GetLoadCurve();
			FESurface* ps = pSF[n];
			ps->add(i);
			plc->SetID(face.m_ntag);
		}
	}
}


//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseSoluteFlux(FEStep *pstep, XMLTag &tag)
{
	// check the sol attribute
	int bc = 0;
	const char* szbc = tag.AttributeValue("sol", true);
	if (szbc) bc = atoi(szbc) - 1;

	// count how many solute flux cards there are
	int nsc = tag.children();

	FEBioMesh& mesh = GetFEBioMesh();
	FEMesh* pm = &GetFEMesh();
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
			FEFace& face = pm->Face(m);
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
		FEFace& face = pm->Face(i);
		int lc = face.m_ntag;
		if (lc >= 0) nlc[lc] = 1;
	}

	int ns = 0;
	for (int i = 0; i<nmax; ++i) if (nlc[i] > 0) nlc[i] = ns++;

	FSModel& fem = GetFSModel();
	GMeshObject* po = GetGObject();

	// let's create the surfaces
	vector<FESoluteFlux*> pPC(ns);
	vector<FESurface*> pSF(ns);
	char szname[256];
	int nsf = CountLoads<FESoluteFlux>(fem);
	for (int i = 0; i<ns; ++i)
	{
		FESurface* ps = new FESurface(po);
		sprintf(szname, "SoluteFluxSurface%02d", nsf + i + 1);
		ps->SetName(szname);

		FESoluteFlux* pbc = new FESoluteFlux(&fem, ps, pstep->GetID());
		sprintf(szname, "SoluteFlux%02d", nsf + i + 1);
		pbc->SetName(szname);
		pbc->SetBC(bc);
		pbc->SetLoad(scl);
		pPC[i] = pbc;
		pSF[i] = ps;
		pstep->AddComponent(pbc);
	}

	// set the correct face group ID's
	for (int i = 0; i<pm->Faces(); ++i)
	{
		FEFace& face = pm->Face(i);
		if (face.m_ntag >= 0)
		{
			int n = nlc[face.m_ntag];
			FELoadCurve* plc = pPC[n]->GetLoadCurve();
			FESurface* ps = pSF[n];
			ps->add(i);
			plc->SetID(face.m_ntag);
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
	GMeshObject* po = GetGObject();

	++tag;
	do
	{
		if (tag == "velocity")	// initial velocity BC
		{
			FENodeSet* pg = 0;
			FEMesh* pm = &GetFEMesh();
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
			sprintf(szname, "InitialVelocity%02d", CountBCs<FENodalVelocities>(fem) + 1);
			pbc->SetName(szname);
			m_pBCStep->AddComponent(pbc);
		}
		else if (tag == "concentration")	// initial concentration BC
		{
			FENodeSet* pg = 0;
			FEMesh* pm = &GetFEMesh();
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

			// create a new initial velocity BC
			FEInitConcentration* pbc = new FEInitConcentration(&fem, pg, bc, c, m_pBCStep->GetID());
			char szname[64] = { 0 };
			sprintf(szname, "InitialConcentration%02d", CountBCs<FEInitConcentration>(fem) + 1);
			pbc->SetName(szname);
			m_pBCStep->AddComponent(pbc);
		}
		else if (tag == "fluid_pressure")	// initial fluid pressure
		{
			FENodeSet* pg = 0;
			FEMesh* pm = &GetFEMesh();
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
			sprintf(szname, "InitialFluidPressure%02d", CountBCs<FEInitFluidPressure>(fem) + 1);
			pbc->SetName(szname);
			m_pBCStep->AddComponent(pbc);
		}
		else if (tag == "temperature")	// initial temperature
		{
			FENodeSet* pg = 0;
			FEMesh* pm = &GetFEMesh();
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
			sprintf(szname, "InitialTemperature%02d", CountBCs<FEInitTemperature>(fem) + 1);
			pbc->SetName(szname);
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
void FEBioFormatOld::ParseContact(FEStep *pstep, XMLTag &tag)
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
void FEBioFormatOld::ParseContactSurface(FESurface* ps, XMLTag& tag)
{
	FEMesh* pm = ps->GetMesh();
	GMeshObject* po = GetGObject();
	FEBioMesh& mesh = GetFEBioMesh();

	// see if the set is defined 
	if (tag.isempty())
	{
		// get the surface name
		const char* szset = tag.AttributeValue("set");

		// find the surface
		FESurface* psurf = po->FindFESurface(szset);
		if (psurf == 0) throw XMLReader::InvalidAttributeValue(tag, "set", szset);

		// copy the surface to the new surface
		ps->Copy(psurf);
	}
	else
	{
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
	}
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseContactSliding(FEStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();

	char szbuf[256];

	// create new sliding interface
	FESlidingWithGapsInterface* pi = new FESlidingWithGapsInterface(&fem, pstep->GetID());

	// get the (optional) contact name
	const char* szname = tag.AttributeValue("name", true);
	if (szname) sprintf(szbuf, "%s", szname);
	else sprintf(szbuf, "SlidingInterface%02d", CountInterfaces<FESlidingWithGapsInterface>(fem) +1);
	pi->SetName(szbuf);

	FESurface *pms = 0, *pss = 0;
	FEMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();

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
				FESurface* ps = new FESurface(po);
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

				// read the surface
				ParseContactSurface(ps, tag);
			}
			else ParseUnknownTag(tag);
		}
		++tag;
	} while (!tag.isend());

	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseContactF2FSliding(FEStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();

	char szbuf[256];

	// create new sliding interface
	FEFacetOnFacetInterface* pi = new FEFacetOnFacetInterface(&fem, pstep->GetID());

	// get the (optional) contact name
	const char* szname = tag.AttributeValue("name", true);
	if (szname) sprintf(szbuf, "%s", szname);
	else sprintf(szbuf, "SlidingContact%02d", CountInterfaces<FEFacetOnFacetInterface>(fem) +1);
	pi->SetName(szbuf);

	FESurface *pms = 0, *pss = 0;
	FEMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();

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
				FESurface* ps = new FESurface(po);
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

				// read the surface
				ParseContactSurface(ps, tag);
			}
			else ParseUnknownTag(tag);
		}
		++tag;
	} while (!tag.isend());

	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseContactBiphasic(FEStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();

	// create new contact interface
	FEPoroContact* pi = new FEPoroContact(&fem, pstep->GetID());

	// read the name
	char szname[256];
	sprintf(szname, "BiphasicContact%02d", CountInterfaces<FEPoroContact>(fem) + 1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	FESurface *pms = 0, *pss = 0;
	FEMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();

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

				FESurface* ps = new FESurface(po);
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

				// read the surface
				ParseContactSurface(ps, tag);
			}
		}
		++tag;
	} while (!tag.isend());

	// add interface to step
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseContactSolute(FEStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();

	// create new interface
	FEPoroSoluteContact* pi = new FEPoroSoluteContact(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "BiphasicSoluteContact%02d", CountInterfaces<FEPoroSoluteContact>(fem) + 1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	FESurface *pms = 0, *pss = 0;
	FEMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();

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

				FESurface* ps = new FESurface(po);

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

				// read the surface
				ParseContactSurface(ps, tag);
			}
		}
		++tag;
	} while (!tag.isend());

	// add interface to step
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseContactMultiphasic(FEStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();

	// create new interface
	FEMultiphasicContact* pi = new FEMultiphasicContact(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "MultiphasicContact%02d", CountInterfaces<FEMultiphasicContact>(fem) + 1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	FESurface *pms = 0, *pss = 0;
	FEMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();

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

				FESurface* ps = new FESurface(po);

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

				// read the surface
				ParseContactSurface(ps, tag);
			}
		}
		++tag;
	} while (!tag.isend());

	// add interface to step
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseContactTied(FEStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();

	// create new interface
	FETiedInterface* pi = new FETiedInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "TiedInterface%02d", CountInterfaces<FETiedInterface>(fem) + 1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	FEMesh* pm = &GetFEMesh();
	FESurface *pms = 0, *pss = 0;
	GMeshObject* po = GetGObject();

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

				FESurface* ps = new FESurface(po);

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

				// read the surface
				ParseContactSurface(ps, tag);
			}
		}
		++tag;
	} while (!tag.isend());

	// add interface to step
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseContactSticky(FEStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();

	// create new interface
	FEStickyInterface* pi = new FEStickyInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "StickyInterface%02d", CountInterfaces<FEStickyInterface>(fem) + 1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	FEMesh* pm = &GetFEMesh();
	FESurface *pms = 0, *pss = 0;
	GMeshObject* po = GetGObject();

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

				FESurface* ps = new FESurface(po);

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

				// read the surface
				ParseContactSurface(ps, tag);
			}
		}
		++tag;
	} while (!tag.isend());

	// add interface to step
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseContactPeriodic(FEStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();

	// create new interface
	FEPeriodicBoundary* pi = new FEPeriodicBoundary(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "PeriodicBoundary%02d", CountInterfaces<FEPeriodicBoundary>(fem) + 1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	FEMesh* pm = &GetFEMesh();
	FESurface *pms = 0, *pss = 0;
	GMeshObject* po = GetGObject();

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

				FESurface* ps = new FESurface(po);

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

				// read the surface
				ParseContactSurface(ps, tag);
			}
		}
		++tag;
	} while (!tag.isend());

	// add interface to step
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseContactTC(FEStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();

	// create new interface
	FETensionCompressionInterface* pi = new FETensionCompressionInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "TCInterface%02d", CountInterfaces<FETensionCompressionInterface>(fem) + 1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	FEMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();
	FESurface *pms = 0, *pss = 0;

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

				FESurface* ps = new FESurface(po);

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

				// read the surface
				ParseContactSurface(ps, tag);
			}
		}
		++tag;
	} while (!tag.isend());

	// add interface to step
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseContactTiedPoro(FEStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();

	// create new interface
	FETiedBiphasicInterface* pi = new FETiedBiphasicInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "TiedBiphasicInterface%02d", CountInterfaces<FETiedBiphasicInterface>(fem) + 1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pi->SetName(szname);

	FEMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();
	FESurface *pms = 0, *pss = 0;

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

				FESurface* ps = new FESurface(po);

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

				// read the surface
				ParseContactSurface(ps, tag);
			}
		}
		++tag;
	} while (!tag.isend());

	// add interface to step
	pstep->AddComponent(pi);
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseRigidWall(FEStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();

	// create a new interface
	FERigidWallInterface* pci = new FERigidWallInterface(&fem, pstep->GetID());

	// set name
	char szname[256];
	sprintf(szname, "RigidWall%02d", CountInterfaces<FERigidWallInterface>(fem) + 1);
	const char* szn = tag.AttributeValue("name", true);
	if (szn) strcpy(szname, szn);
	pci->SetName(szname);

	FEMesh* pm = &GetFEMesh();
	GMeshObject* po = GetGObject();
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

			const char* szlc = tag.AttributeValue("lc", true);
			if (szlc)
			{
				febio.AddParamCurve(&pci->GetParam(FERigidWallInterface::OFFSET), atoi(szlc) - 1);
			}
		}
		else if (tag == "surface")
		{
			const char* szn = tag.AttributeValue("name", true);

			FESurface* ps = new FESurface(po);
			pci->SetItemList(ps);
			if (szn) ps->SetName(szn);

			// read the surface
			ParseContactSurface(ps, tag);
		}
		++tag;
	} while (!tag.isend());

	// add interface to step
	pstep->AddComponent(pci);
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseContactRigid(FEStep *pstep, XMLTag &tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = GetFSModel();
	FEMesh* pm = &GetFEMesh();

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
	} while (!tag.isend());

	GMeshObject* po = GetGObject();
	FEMesh& mesh = GetFEMesh();
	int NN = mesh.Nodes();
	char szbuf[256];
	for (int i = 0; i<NMAT; ++i)
	{
		GMaterial* pmat = febio.GetMaterial(i);
		if (pmat && (pmat->m_ntag > 0))
		{
			int id = pmat->GetID();
			// create the node set
			FENodeSet* pn = new FENodeSet(po);

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
void FEBioFormatOld::ParseContactJoint(FEStep *pstep, XMLTag &tag)
{
	FEBioInputModel& febio = GetFEBioModel();

	FSModel& fem = GetFSModel();

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

	if (na >= 0) pi->m_pbodyA = febio.GetMaterial(na - 1);
	if (nb >= 0) pi->m_pbodyB = febio.GetMaterial(nb - 1);
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseSprings(FEStep *pstep, XMLTag &tag)
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
							  pg->GetParam(GGeneralSpring::MP_F).GetLoadCurve()->SetID(lc);
							  pd = pg;
	}
		break;
	}

	fem.GetModel().AddDiscreteObject(pd);
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseLinearConstraint(FEStep* pstep, XMLTag& tag)
{
	FSModel& fem = GetFSModel();

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

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseBodyForce(FEStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();

	FEConstBodyForce* pbl = new FEConstBodyForce(&fem, pstep->GetID());
	pstep->AddComponent(pbl);

	++tag;
	do
	{
		if (ReadParam(*pbl, tag) == false) ParseUnknownTag(tag);
		else ++tag;
	}
	while (!tag.isend());

	char szname[256] = { 0 };
	sprintf(szname, "BodyForce%02d", CountLoads<FEConstBodyForce>(fem));
	pbl->SetName(szname);
}

//-----------------------------------------------------------------------------
void FEBioFormatOld::ParseHeatSource(FEStep *pstep, XMLTag &tag)
{
	FSModel& fem = GetFSModel();

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
bool FEBioFormatOld::ParseConstraintSection(XMLTag& tag)
{
	if (tag.isleaf()) return true;

	FEBioInputModel& febio = GetFEBioModel();
	FSModel& fem = febio.GetFSModel();

	FEStep* pstep = m_pBCStep;

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
			assert(dynamic_cast<FERigidMaterial*>(pgm->GetMaterialProperties()));

			FERigidFixed* pc = 0; // fixed constraint

			++tag;
			do
			{
				if (tag == "trans_x")
				{
					int lc = tag.AttributeValue<int>("lc", 0);

					XMLAtt& atype = tag.Attribute("type");
					if (atype == "fixed")
					{
						if (pc == 0) pc = new FERigidFixed(&fem, pstep->GetID());
						pc->SetDOF(0, true);
					}
					else if (atype == "prescribed")
					{
						tag.value(v);
						FERigidDisplacement* pd = new FERigidDisplacement(0, matid, v, pstep->GetID());
						febio.AddParamCurve(pd->GetLoadCurve(), lc-1);

						sprintf(sz, "RigidDisplacement%02d", nrd++);
						pd->SetName(sz);
						pstep->AddRC(pd);
					}
					else if (atype == "force")
					{
						tag.value(v);
						FERigidForce* pd = new FERigidForce(0, matid, v, pstep->GetID());
						febio.AddParamCurve(pd->GetLoadCurve(), lc - 1);

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
						if (pc == 0) pc = new FERigidFixed(&fem, pstep->GetID());
						pc->SetDOF(1, true);
					}
					else if (atype == "prescribed")
					{
						tag.value(v);
						FERigidDisplacement* pd = new FERigidDisplacement(1, matid, v, pstep->GetID());
						febio.AddParamCurve(pd->GetLoadCurve(), lc - 1);

						sprintf(sz, "RigidDisplacement%02d", nrd++);
						pd->SetName(sz);
						pstep->AddRC(pd);
					}
					else if (atype == "force")
					{
						tag.value(v);
						FERigidForce* pd = new FERigidForce(1, matid, v, pstep->GetID());
						febio.AddParamCurve(pd->GetLoadCurve(), lc - 1);

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
						if (pc == 0) pc = new FERigidFixed(&fem, pstep->GetID());
						pc->SetDOF(2, true);
					}
					else if (atype == "prescribed")
					{
						tag.value(v);
						FERigidDisplacement* pd = new FERigidDisplacement(2, matid, v, pstep->GetID());
						febio.AddParamCurve(pd->GetLoadCurve(), lc - 1);

						sprintf(sz, "RigidDisplacement%02d", nrd++);
						pd->SetName(sz);
						pstep->AddRC(pd);
					}
					else if (atype == "force")
					{
						tag.value(v);
						FERigidForce* pd = new FERigidForce(2, matid, v, pstep->GetID());
						febio.AddParamCurve(pd->GetLoadCurve(), lc - 1);

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
						if (pc == 0) pc = new FERigidFixed(&fem, pstep->GetID());
						pc->SetDOF(3, true);
					}
					else if (atype == "prescribed")
					{
						tag.value(v);
						FERigidDisplacement* pd = new FERigidDisplacement(3, matid, v, pstep->GetID());
						febio.AddParamCurve(pd->GetLoadCurve(), lc - 1);

						sprintf(sz, "RigidDisplacement%02d", nrd++);
						pd->SetName(sz);
						pstep->AddRC(pd);
					}
					else if (atype == "force")
					{
						tag.value(v);
						FERigidForce* pd = new FERigidForce(3, matid, v, pstep->GetID());
						febio.AddParamCurve(pd->GetLoadCurve(), lc - 1);

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
						if (pc == 0) pc = new FERigidFixed(&fem, pstep->GetID());
						pc->SetDOF(4, true);
					}
					else if (atype == "prescribed")
					{
						tag.value(v);
						FERigidDisplacement* pd = new FERigidDisplacement(4, matid, v, pstep->GetID());
						febio.AddParamCurve(pd->GetLoadCurve(), lc - 1);

						sprintf(sz, "RigidDisplacement%02d", nrd++);
						pd->SetName(sz);
						pstep->AddRC(pd);
					}
					else if (atype == "force")
					{
						tag.value(v);
						FERigidForce* pd = new FERigidForce(4, matid, v, pstep->GetID());
						febio.AddParamCurve(pd->GetLoadCurve(), lc - 1);

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
						if (pc == 0) pc = new FERigidFixed(&fem, pstep->GetID());
						pc->SetDOF(5, true);
					}
					else if (atype == "prescribed")
					{
						tag.value(v);
						FERigidDisplacement* pd = new FERigidDisplacement(5, matid, v, pstep->GetID());
						febio.AddParamCurve(pd->GetLoadCurve(), lc - 1);

						sprintf(sz, "RigidDisplacement%02d", nrd++);
						pd->SetName(sz);
						pstep->AddRC(pd);
					}
					else if (atype == "force")
					{
						tag.value(v);
						FERigidForce* pd = new FERigidForce(5, matid, v, pstep->GetID());
						febio.AddParamCurve(pd->GetLoadCurve(), lc - 1);

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
void FEBioFormatOld::ParseRigidConstraint(FEStep* pstep, XMLTag& tag)
{
	FEBioInputModel& febio = GetFEBioModel();
	FSModel* fem = &febio.GetFSModel();
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

	++tag;
	do
	{
		if (tag == "initial_velocity")
		{
			FERigidVelocity* pv = new FERigidVelocity(fem, pstep->GetID());
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
			FERigidAngularVelocity* pv = new FERigidAngularVelocity(fem, pstep->GetID());
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
				if (pc == 0) pc = new FERigidFixed(fem, pstep->GetID());
				pc->SetDOF(nbc, true);
			}
			else if (tag == "prescribed")
			{
				int lc = tag.AttributeValue<int>("lc", 0);
				double v;
				tag.value(v);

				FERigidDisplacement* pd = new FERigidDisplacement(nbc, matid, v, pstep->GetID());
				febio.AddParamCurve(pd->GetLoadCurve(), lc - 1);

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

				FERigidForce* pd = new FERigidForce(nbc, matid, v, pstep->GetID());
				febio.AddParamCurve(pd->GetLoadCurve(), lc - 1);

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
void FEBioFormatOld::ParseVolumeConstraint(FEStep* pstep, XMLTag& tag)
{
	// make sure there is something to read
	if (tag.isempty()) return;

	FSModel& fem = GetFSModel();

	// create a new volume constraint
	FEVolumeConstraint* pi = new FEVolumeConstraint(&fem, pstep->GetID());
	pstep->AddComponent(pi);

	// get the (optional) contact name
	char szbuf[256];
	const char* szname = tag.AttributeValue("name", true);
	if (szname) sprintf(szbuf, "%s", szname);
	else sprintf(szbuf, "VolumeConstraint%02d", CountConstraints<FEVolumeConstraint>(fem));
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
				FESurface* ps = new FESurface(po);
				sprintf(szbuf, "VolumeConstraintSurface%02d", CountConstraints<FEVolumeConstraint>(fem));
				ps->SetName(szn ? szn : szbuf);

				// assign the surface
				pi->SetItemList(ps);

				// read the surface
				ParseContactSurface(ps, tag);
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

	m_nAnalysis = -1;

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
