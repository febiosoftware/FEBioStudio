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
#include "FEBioExport4.h"
#include <FEMLib/FERigidConstraint.h>
#include <FEMLib/FEInitialCondition.h>
#include <FEMLib/FESurfaceLoad.h>
#include <FEMLib/FEBodyLoad.h>
#include <FEMLib/FEDataMap.h>
#include <FEMLib/FEModelConstraint.h>
#include <FEMLib/FERigidLoad.h>
#include <GeomLib/GObject.h>
#include <MeshTools/GGroup.h>
#include <MeshTools/FENodeData.h>
#include <MeshTools/FESurfaceData.h>
#include <MeshTools/FEElementData.h>
#include <MeshTools/GModel.h>
#include <MeshTools/GModel.h>
#include <MeshTools/FEProject.h>
#include <FEBioLink/FEBioModule.h>
#include <memory>
#include <sstream>
#include <FECore/FETransform.h>

using std::stringstream;
using std::unique_ptr;

//-----------------------------------------------------------------------------
// defined in FEFEBioExport25.cpp
FENodeList* BuildNodeList(GFace* pf);
FENodeList* BuildNodeList(GPart* pg);
FENodeList* BuildNodeList(GNode* pn);
FENodeList* BuildNodeList(GEdge* pe);
FEFaceList* BuildFaceList(GFace* face);
const char* ElementTypeString(int ntype);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FEBioExport4::FEBioExport4(FEProject& prj) : FEBioExport(prj)
{
	m_writeNotes = true;
	m_exportEnumStrings = true;
	m_writeControlSection = true;
}

FEBioExport4::~FEBioExport4()
{
	Clear();
}

void FEBioExport4::Clear()
{
	FEBioExport::Clear();
	m_pSurf.clear();
	m_pNSet.clear();
	m_pESet.clear();
	m_ElSet.clear();
}

//----------------------------------------------------------------------------
FEBioExport4::Part* FEBioExport4::FindPart(GObject* po)
{
	for (size_t i = 0; i < m_Part.size(); ++i)
	{
		Part* pi = m_Part[i];
		if (po == pi->m_obj) return pi;
	}
	return 0;
}

//----------------------------------------------------------------------------
const char* FEBioExport4::GetSurfaceName(FEItemListBuilder* pl)
{
	int N = (int)m_pSurf.size();
	for (int i = 0; i < N; ++i)
		if (m_pSurf[i].m_list == pl) return m_pSurf[i].m_name.c_str();
	assert(false);
	return 0;
}

//----------------------------------------------------------------------------
string FEBioExport4::GetElementSetName(FEItemListBuilder* pl)
{
	int N = (int)m_pESet.size();
	for (int i = 0; i < N; ++i)
		if (m_pESet[i].m_list == pl) return m_pESet[i].m_name.c_str();
	assert(false);
	return "";
}

//----------------------------------------------------------------------------
string FEBioExport4::GetNodeSetName(FEItemListBuilder* pl)
{
	// search the nodesets first
	int N = (int)m_pNSet.size();
	for (int i = 0; i < N; ++i)
		if (m_pNSet[i].m_list == pl) return m_pNSet[i].m_name.c_str();

	// search the surfaces
	N = (int)m_pSurf.size();
	for (int i = 0; i < N; ++i)
		if (m_pSurf[i].m_list == pl)
		{
			string surfName = m_pSurf[i].m_name;
			return string("@surface:") + surfName;
		}

	// search the element sets
	N = (int)m_pESet.size();
	for (int i = 0; i < N; ++i)
		if (m_pESet[i].m_list == pl)
		{
			string setName = m_pESet[i].m_name;
			return string("@elem_set:") + setName;
		}

	assert(false);
	return "";
}

//-----------------------------------------------------------------------------
void FEBioExport4::AddNodeSet(const std::string& name, FEItemListBuilder* pl)
{
	// see if the node set is already defined
	for (int i = 0; i < m_pNSet.size(); ++i)
	{
		if (m_pNSet[i].m_name == name)
		{
			// add it, but mark it as duplicate
//			assert(false);
			m_pNSet.push_back(NamedItemList(name, pl, true));
			return;
		}
	}

	m_pNSet.push_back(NamedItemList(name, pl));
}

//-----------------------------------------------------------------------------
void FEBioExport4::AddSurface(const std::string& name, FEItemListBuilder* pl)
{
	// make sure this has not been added 
	for (int i = 0; i < m_pSurf.size(); ++i)
	{
		NamedItemList& surf = m_pSurf[i];
		if ((surf.m_list == pl) && (surf.m_name == name)) return;
	}
	m_pSurf.push_back(NamedItemList(string(name), pl));
}

//-----------------------------------------------------------------------------
void FEBioExport4::AddElemSet(const std::string& name, FEItemListBuilder* pl)
{
	assert(pl);
	if (pl == nullptr) return;
	m_pESet.push_back(NamedItemList(string(name), pl));
}

//-----------------------------------------------------------------------------
bool FEBioExport4::PrepareExport(FEProject& prj)
{
	if (FEBioExport::PrepareExport(prj) == false) return false;

	FSModel& fem = prj.GetFSModel();
	GModel& model = fem.GetModel();

	// make sure all steps (except the initial one)
	// are of the same type
	m_nsteps = fem.Steps();
	if (m_nsteps > 1)
	{
		FSStep* pstep = fem.GetStep(1);
		int ntype = pstep->GetType();
		for (int i = 2; i < m_nsteps; ++i)
		{
			if (fem.GetStep(i)->GetType() != ntype) return errf("All analysis steps must be of same type for this file format.");
		}
	}

	m_Part.push_back(new Part(nullptr));

	// Build the named lists
	BuildItemLists(prj);

	// see if we need to add a MeshData section
	m_bdata = false;
	if (model.ShellElements() > 0) m_bdata = true;	// for shell thicknesses
	for (int i = 0; i < fem.Materials(); ++i)
	{
		FSTransverselyIsotropic* pmat = dynamic_cast<FSTransverselyIsotropic*>(fem.GetMaterial(i)->GetMaterialProperties());
		if (pmat && (pmat->GetFiberMaterial()->m_naopt == FE_FIBER_USER)) m_bdata = true;
	}
	for (int i = 0; i < model.Objects(); ++i)
	{
		FEMesh* pm = model.Object(i)->GetFEMesh();
		for (int j = 0; j < pm->Elements(); ++j)
		{
			FEElement_& e = pm->ElementRef(j);
			if (e.m_Qactive) {
				m_bdata = true;
				break;
			}
		}
		if (pm->MeshDataFields() > 0) m_bdata = true;
	}

	// See if we have data maps
	if (fem.DataMaps() > 0) m_bdata = true;

	return true;
}

//-----------------------------------------------------------------------------
void FEBioExport4::BuildItemLists(FEProject& prj)
{
	FSModel& fem = prj.GetFSModel();

	// get the nodesets (bc's)
	for (int i = 0; i < fem.Steps(); ++i)
	{
		FSStep* pstep = fem.GetStep(i);
		for (int j = 0; j < pstep->BCs(); ++j)
		{
			FSBoundaryCondition* pl = pstep->BC(j);
			if (pl && pl->IsActive())
			{
				FEItemListBuilder* ps = pl->GetItemList();
				if (ps == 0) throw InvalidItemListBuilder(pl);

				string name = ps->GetName();
				if (name.empty()) name = pl->GetName();

				if ((ps->Type() == GO_FACE) || (ps->Type() == FE_SURFACE)) AddSurface(name, ps);
				else if (ps->Type() == GO_PART) AddElemSet(name, ps);
				else AddNodeSet(name, ps);
			}
		}
		for (int j = 0; j < pstep->Loads(); ++j)
		{
			// this is only for nodal loads
			FSNodalDOFLoad* pl = dynamic_cast<FSNodalDOFLoad*>(pstep->Load(j));
			if (pl && pl->IsActive())
			{
				FEItemListBuilder* ps = pl->GetItemList();
				if (ps == 0) throw InvalidItemListBuilder(pl);

				string name = ps->GetName();
				if (name.empty()) name = pl->GetName();

				if ((ps->Type() == GO_FACE) || (ps->Type() == FE_SURFACE)) AddSurface(name, ps);
				else if (ps->Type() == GO_PART) AddElemSet(name, ps);
				else AddNodeSet(name, ps);
			}

			FSBodyLoad* pbl = dynamic_cast<FSBodyLoad*>(pstep->Load(j));
			if (pbl && pbl->IsActive())
			{
				FEItemListBuilder* ps = pbl->GetItemList();
				if (ps)
				{
					string name = ps->GetName();
					if (name.empty()) name = pbl->GetName();
					if (ps->Type() == GO_PART) AddElemSet(name, ps);
				}
			}
		}
		for (int j = 0; j < pstep->ICs(); ++j)
		{
			// this is only for nodal loads
			FSInitialCondition* pi = pstep->IC(j);
			if (pi && pi->IsActive())
			{
				FEItemListBuilder* ps = pi->GetItemList();
				if (ps == 0) throw InvalidItemListBuilder(pi);

				string name = ps->GetName();
				if (name.empty()) name = pi->GetName();

				if ((ps->Type() == GO_FACE) || (ps->Type() == FE_SURFACE)) AddSurface(name, ps);
				else AddNodeSet(name, ps);
			}
		}
		for (int j = 0; j < pstep->Interfaces(); ++j)
		{
			FSRigidInterface* pri = dynamic_cast<FSRigidInterface*>(pstep->Interface(j));
			if (pri && pri->IsActive())
			{
				FEItemListBuilder* ps = pri->GetItemList();
				if (ps == 0) throw InvalidItemListBuilder(pri);

				string name = ps->GetName();
				if (name.empty()) name = pri->GetName();

				if ((ps->Type() == GO_FACE) || (ps->Type() == FE_SURFACE)) AddSurface(name, ps);
				else AddNodeSet(name, ps);
			}
		}
	}

	// get the named surfaces (loads)
	for (int i = 0; i < fem.Steps(); ++i)
	{
		FSStep* pstep = fem.GetStep(i);
		for (int j = 0; j < pstep->Loads(); ++j)
		{
			FSLoad* pl = pstep->Load(j);
			if (pl->IsActive())
			{
				// we need to exclude nodal loads and body loads
				if (dynamic_cast<FSNodalDOFLoad*>(pl)) pl = 0;
				if (dynamic_cast<FSBodyLoad*>(pl)) pl = 0;
				if (pl && pl->IsActive())
				{
					FEItemListBuilder* ps = pl->GetItemList();
					if (ps == 0) throw InvalidItemListBuilder(pl);

					string name = ps->GetName();
					if (name.empty()) name = pl->GetName();

					AddSurface(name, ps);
				}
			}
		}
	}

	// get the named surfaces (paired interfaces)
	char szbuf[256] = { 0 };
	for (int i = 0; i < fem.Steps(); ++i)
	{
		FSStep* pstep = fem.GetStep(i);
		for (int j = 0; j < pstep->Interfaces(); ++j)
		{
			FSInterface* pj = pstep->Interface(j);
			if (pj->IsActive())
			{
				FSPairedInterface* pi = dynamic_cast<FSPairedInterface*>(pj);

				// Note: Don't export surfaces of tied-spring interfaces
				if (dynamic_cast<FSSpringTiedInterface*>(pi)) pi = 0;

				if (pi && pi->IsActive())
				{
					FEItemListBuilder* pss = pi->GetPrimarySurface();
					if (pss == 0) throw InvalidItemListBuilder(pi);

					string name = pss->GetName();
					const char* szname = name.c_str();
					if ((szname == 0) || (szname[0] == 0))
					{
						sprintf(szbuf, "%s_primary", pi->GetName().c_str());
						szname = szbuf;
					}
					AddSurface(szname, pss);

					FEItemListBuilder* pms = pi->GetSecondarySurface();
					if (pms == 0) throw InvalidItemListBuilder(pi);

					name = pms->GetName();
					szname = name.c_str();
					if ((szname == 0) || (szname[0] == 0))
					{
						sprintf(szbuf, "%s_secondary", pi->GetName().c_str());
						szname = szbuf;
					}
					AddSurface(szname, pms);
				}

				FSRigidWallInterface* pw = dynamic_cast<FSRigidWallInterface*>(pj);
				if (pw && pw->IsActive())
				{
					FEItemListBuilder* pitem = pw->GetItemList();
					if (pitem == 0) throw InvalidItemListBuilder(pw);

					string name = pitem->GetName();
					if (name.empty()) name = pw->GetName();
					AddSurface(name, pitem);
				}

				FSRigidSphereInterface* prs = dynamic_cast<FSRigidSphereInterface*>(pj);
				if (prs && prs->IsActive())
				{
					FEItemListBuilder* pitem = prs->GetItemList();
					if (pitem == 0) throw InvalidItemListBuilder(prs);

					string name = pitem->GetName();
					if (name.empty()) name = prs->GetName();
					AddSurface(name, pitem);
				}
			}
		}

		for (int j = 0; j < pstep->Constraints(); ++j)
		{
			FSModelConstraint* pj = pstep->Constraint(j);
			if (pj->IsActive())
			{
				FSSurfaceConstraint* psf = dynamic_cast<FSSurfaceConstraint*>(pj);
				if (psf && psf->IsActive())
				{
					FEItemListBuilder* pi = psf->GetItemList();
					if (pi == 0) throw InvalidItemListBuilder(pi);

					string name = pi->GetName();
					if (name.empty()) name = psf->GetName();

					AddSurface(name, pi);
				}
			}
		}
	}

	// Write the user-defined surfaces
	if (m_exportSelections)
	{
		GModel& model = fem.GetModel();
		int faces = model.FaceLists();
		for (int i = 0; i < faces; ++i)
		{
			GFaceList* pfl = model.FaceList(i);
			AddSurface(pfl->GetName(), pfl);
		}

		int nobj = model.Objects();
		for (int i = 0; i < nobj; ++i)
		{
			GObject* po = model.Object(i);
			FEMesh* pm = po->GetFEMesh();
			if (pm)
			{
				int nsurf = po->FESurfaces();
				for (int j = 0; j < nsurf; ++j)
				{
					FESurface* ps = po->GetFESurface(j);
					AddSurface(ps->GetName(), ps);
				}

				int neset = po->FEParts();
				for (int j = 0; j < neset; ++j)
				{
					FEPart* pg = po->GetFEPart(j);
					AddElemSet(pg->GetName(), pg);
				}
			}
		}
	}

	// check all the (surface) plot variables
	CPlotDataSettings& plt = prj.GetPlotDataSettings();
	for (int i = 0; i < plt.PlotVariables(); ++i)
	{
		FEPlotVariable& var = plt.PlotVariable(i);
		if (var.domainType() == DOMAIN_SURFACE)
		{
			int ND = var.Domains();
			for (int j = 0; j < ND; ++j)
			{
				FEItemListBuilder* pl = var.GetDomain(j);
				AddSurface(pl->GetName(), pl);
			}
		}
	}

	GModel& model = fem.GetModel();
	CLogDataSettings& log = prj.GetLogDataSettings();
	for (int i = 0; i < log.LogDataSize(); ++i)
	{
		FELogData& di = log.LogData(i);
		if ((di.type == FELogData::LD_ELEM) && (di.groupID != -1))
		{
			FEItemListBuilder* pg = model.FindNamedSelection(di.groupID);
			if (pg)
			{
				AddElemSet(pg->GetName(), pg);
			}
		}
		if ((di.type == FELogData::LD_NODE) && (di.groupID != -1))
		{
			FEItemListBuilder* pg = model.FindNamedSelection(di.groupID);
			if (pg)
			{
				AddNodeSet(pg->GetName(), pg);
			}
		}
	}

	// export item lists for mesh data
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		FEMesh* mesh = po->GetFEMesh();
		if (mesh)
		{
			int ND = mesh->MeshDataFields();
			for (int j = 0; j < ND; ++j)
			{
				FEMeshData* data = mesh->GetMeshDataField(j);

				switch (data->GetDataClass())
				{
				case FEMeshData::NODE_DATA:
				{
					FENodeData* map = dynamic_cast<FENodeData*>(data); assert(map);
					FEItemListBuilder* pg = map->GetItemList();
					if (pg)
					{
						string name = pg->GetName();
						if (name.empty()) name = data->GetName();
						AddNodeSet(name, pg);
					}
				}
				break;
				case FEMeshData::ELEMENT_DATA:
				{
					FEElementData* map = dynamic_cast<FEElementData*>(data); assert(map);
					FEPart* pg = const_cast<FEPart*>(map->GetPart());

					if (pg)
					{
						string name = pg->GetName();
						if (name.empty()) name = data->GetName();

						// It is possible that a FEPart has the same name as the domain
						// from which it was created. In that case we don't want to 
						// write this element set.
						for (int j = 0; j < po->Parts(); ++j)
						{
							GPart* part = po->Part(j);
							if (part->GetName() == name)
							{
								pg = nullptr;
								break;
							}
						}
						if (pg) AddElemSet(name, pg);
					}
				}
				break;
				case FEMeshData::PART_DATA:
				{
					// We don't create element sets for part data since we already have the corresponding element sets
					// in the mesh. 
				}
				break;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
GPartList* FEBioExport4::BuildPartList(GMaterial* mat)
{
	// get the document
	FSModel& fem = m_prj.GetFSModel();
	GModel& mdl = fem.GetModel();

	GPartList* pl = new GPartList(&fem);

	// set the items
	int N = mdl.Parts();
	for (int i = 0; i < mdl.Parts(); ++i)
	{
		GPart* pg = mdl.Part(i);
		GMaterial* pgm = fem.GetMaterialFromID(pg->GetMaterialID());
		if (pgm && (pgm->GetID() == mat->GetID()))
		{
			pl->add(pg->GetID());
		}
	}

	return pl;
}

//-----------------------------------------------------------------------------
bool FEBioExport4::Write(const char* szfile)
{
	// get the project and model
	FSModel& fem = m_prj.GetFSModel();
	GModel& mdl = fem.GetModel();
	m_pfem = &fem;

	// prepare for export
	try
	{
		if (PrepareExport(m_prj) == false) return false;

		// get the initial step
		FSStep* pstep = fem.GetStep(0);

		// the format for single step versus multi-step
		// is slightly different, so we need to see if the 
		// model is single step or not.
		// The model is single step if it has only one 
		// analysis-step and if that step does not define
		// any BCs, Loads, interfaces or RCs.
		int ntype = -1;
		bool bsingle_step = (m_nsteps <= 1);
		if (m_nsteps == 2)
		{
			//			FSAnalysisStep* pstep = dynamic_cast<FSAnalysisStep*>(fem.GetStep(1));
			//			if (pstep == 0) return errf("Step 1 is not an analysis step.");
			ntype = pstep->GetType();
			if (pstep->BCs() + pstep->Loads() + pstep->ICs() + pstep->Interfaces() + pstep->LinearConstraints() + pstep->RigidConstraints() + pstep->RigidConnectors() == 0) bsingle_step = true;
		}

		// open the file
		if (!m_xml.open(szfile)) return errf("Failed opening file %s", szfile);

		if (m_writeNotes) WriteNote(&mdl);

		// set modified formats
		XMLElement::intFormat = "%d";

		XMLElement el;

		// output root element
		el.name("febio_spec");
		el.add_attribute("version", "4.0");

		m_xml.add_branch(el);
		{
			// write the module section
			if (m_section[FEBIO_MODULE]) WriteModuleSection(m_prj);

			// write Control section
			if (m_writeControlSection && (m_nsteps == 2) && bsingle_step)
			{
				if (m_section[FEBIO_CONTROL])
				{
					m_xml.add_branch("Control");
					{
						FSStep& step = *fem.GetStep(1);
						WriteControlSection(step);
					}
					m_xml.close_branch();
				}
			}

			// global variables
			int nvar = fem.Parameters();
			if ((nvar > 0) && m_section[FEBIO_GLOBAL])
			{
				m_xml.add_branch("Globals");
				{
					WriteGlobalsSection();
				}
				m_xml.close_branch();
			}

			// output material section
			if ((fem.Materials() > 0) && (m_section[FEBIO_MATERIAL]))
			{
				m_xml.add_branch("Material");
				{
					WriteMaterialSection();
				}
				m_xml.close_branch(); // Material
			}

			// output Mesh section
			if ((fem.GetModel().Objects() > 0) && (m_section[FEBIO_GEOMETRY]))
			{
				m_xml.add_branch("Mesh");
				{
					WriteMeshSection();
				}
				m_xml.close_branch(); // Mesh

				m_xml.add_branch("MeshDomains");
				{
					WriteMeshDomainsSection();
				}
				m_xml.close_branch(); // MeshDomains
			}

			// output mesh data section
			if (m_bdata && m_section[FEBIO_MESHDATA])
			{
				m_xml.add_branch("MeshData");
				{
					WriteMeshDataSection();
				}
				m_xml.close_branch(); // MeshData
			}

			// output boundary section
			int nbc = pstep->BCs();
			if ((nbc > 0) && (m_section[FEBIO_BOUNDARY]))
			{
				m_xml.add_branch("Boundary");
				{
					WriteBoundarySection(*pstep);
				}
				m_xml.close_branch(); // Boundary
			}

			int nrc = pstep->RigidConstraints() + pstep->RigidConnectors() + CountInterfaces<FSRigidJoint>(fem);
			if ((nrc > 0) && (m_section[FEBIO_BOUNDARY]))
			{
				m_xml.add_branch("Rigid");
				{
					WriteRigidSection(*pstep);
				}
				m_xml.close_branch(); // Rigid
			}

			// output loads section
			int nlc = pstep->Loads();
			if ((nlc > 0) && (m_section[FEBIO_LOADS]))
			{
				m_xml.add_branch("Loads");
				{
					WriteLoadsSection(*pstep);
				}
				m_xml.close_branch(); // Boundary
			}

			// output contact
			int nci = pstep->Interfaces() - CountInterfaces<FSRigidJoint>(fem);
			int nLC = pstep->LinearConstraints();
			if (((nci > 0) || (nLC > 0)) && (m_section[FEBIO_CONTACT]))
			{
				m_xml.add_branch("Contact");
				{
					WriteContactSection(*pstep);
				}
				m_xml.close_branch(); // Contact
			}

			// output constraints section
			int nnlc = CountConstraints<FSModelConstraint>(fem);
			if ((nnlc > 0) && (m_section[FEBIO_CONSTRAINTS]))
			{
				m_xml.add_branch("Constraints");
				{
					WriteConstraintSection(*pstep);
				}
				m_xml.close_branch();
			}

			// output initial section
			int nic = pstep->ICs() + pstep->RigidConstraints(FE_RIGID_INIT_VELOCITY) + pstep->RigidConstraints(FE_RIGID_INIT_ANG_VELOCITY);
			if ((nic > 0) && (m_section[FEBIO_INITIAL]))
			{
				m_xml.add_branch("Initial");
				{
					WriteInitialSection();
				}
				m_xml.close_branch(); // Initial
			}

			// output discrete elements (the obsolete spring-tied interface generates springs as well)
			int nrb = fem.GetModel().DiscreteObjects() + CountInterfaces<FSSpringTiedInterface>(fem);
			if ((nrb > 0) && (m_section[FEBIO_DISCRETE]))
			{
				m_xml.add_branch("Discrete");
				{
					WriteDiscreteSection(*pstep);
				}
				m_xml.close_branch(); // Discrete
			}

			// step data
			if (m_section[FEBIO_STEPS])
			{
				if ((m_writeControlSection == false) || (m_nsteps > 2) || (bsingle_step == false))
				{
					m_xml.add_branch("Step");
					{
						WriteStepSection();
					}
					m_xml.close_branch();
				}
			}

			// loadcurve data
			if ((m_pLC.size() > 0) && (m_section[FEBIO_LOADDATA]))
			{
				m_xml.add_branch("LoadData");
				{
					WriteLoadDataSection();
				}
				m_xml.close_branch(); // LoadData
			}

			// Output data
			if (m_section[FEBIO_OUTPUT])
			{
				m_xml.add_branch("Output");
				{
					WriteOutputSection();
				}
				m_xml.close_branch(); // Output
			}
		}
		m_xml.close_branch(); // febio_spec
	}
	catch (InvalidMaterialReference)
	{
		return errf("Invalid material reference.");
	}
	catch (InvalidItemListBuilder e)
	{
		const char* sz = "(unknown)";
		if (e.m_name.empty() == false) sz = e.m_name.c_str();
		return errf("Invalid reference to mesh item list when exporting:\n%s", sz);
	}
	catch (MissingRigidBody e)
	{
		return errf("No rigid body defined for rigid constraint %s", e.m_rbName.c_str());
	}
	catch (RigidContactException)
	{
		return errf("Missing rigid body in rigid contact definition.");
	}
	catch (...)
	{
		return errf("An unknown exception has occured.");
	}

	// close the file
	m_xml.close();

	return true;
}

//-----------------------------------------------------------------------------
// Write the MODULE section
void FEBioExport4::WriteModuleSection(FEProject& prj)
{
	int moduleId = prj.GetModule();
	const char* szmodName = FEBio::GetModuleName(moduleId);
	XMLElement t;
	t.name("Module");
	t.add_attribute("type", szmodName);
	m_xml.add_empty(t);
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteControlSection(FSStep& step)
{
	WriteParamList(step);
	for (int i = 0; i < step.ControlProperties(); ++i)
	{
		FSStepControlProperty& prop = step.GetControlProperty(i);
		FSStepComponent* pc = prop.m_prop;
		if (pc)
		{
			XMLElement el(prop.GetName().c_str());
			const char* sztype = pc->GetTypeString();
			if (sztype) el.add_attribute("type", sztype);
			m_xml.add_branch(el);
			WriteParamList(*pc);
			m_xml.close_branch();
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteMaterialSection()
{
	FSModel& fem = *m_pfem;
	for (int i = 0; i < fem.Materials(); ++i)
	{
		GMaterial* pgm = fem.GetMaterial(i);
		FSMaterial* pmat = pgm->GetMaterialProperties();

		if (m_writeNotes) WriteNote(pgm);

		const string& name = pgm->GetName();

		XMLElement el;
		el.name("material");
		el.add_attribute("id", pgm->m_ntag);
		el.add_attribute("name", name.c_str());

		if (pmat)
		{
			WriteMaterial(pmat, el);
		}
		else
		{
			errf("ERROR: Material %s does not have any properties.", name.c_str());
			m_xml.add_leaf(el);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteMaterial(FSMaterial* pm, XMLElement& el)
{
	// get the type string    
	const char* sztype = pm->GetTypeString(); assert(sztype);
	el.add_attribute("type", pm->GetTypeString());

	// see if there are any attribute parameters
	FSModel& fem = m_prj.GetFSModel();
	for (int i = 0; i < pm->Parameters(); ++i)
	{
		Param& p = pm->GetParam(i);
		if (p.GetFlags() & 0x01)
		{
			switch (p.GetParamType())
			{
			case Param_CHOICE:
			case Param_INT:
			{
				// this assumes that the value is actually the index into the enums. 
				// the enums may have an implied value associated that may differ from its index, so we need to acquire that.
				if (p.GetEnumNames())
				{
					int v = fem.GetEnumIntValue(p);
					el.add_attribute(p.GetShortName(), v);
				}
				else el.add_attribute(p.GetShortName(), p.GetIntValue());
			}
			break;
			}
		}
	}

	m_xml.add_branch(el);
	{
		// write the material parameters (if any)
		if (pm->Parameters())
		{
			WriteParamList(*pm);
		}

		// write the material properties
		int NC = pm->Properties();
		for (int i = 0; i < NC; ++i)
		{
			FSMaterialProperty& mc = pm->GetProperty(i);
			for (int j = 0; j < mc.Size(); ++j)
			{
				FSMaterial* pc = mc.GetMaterial(j);
				if (pc)
				{
					el.name(mc.GetName().c_str());
					WriteMaterial(pc, el);
				}
			}
		}
	}
	m_xml.close_branch();
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteMeshSection()
{
	// export the nodes
	WriteGeometryNodes();

	// Write the elements
	WriteMeshElements();

	// write the node sets
	WriteGeometryNodeSets();

	// write named surfaces
	WriteGeometrySurfaces();

	// write named element sets
	WriteGeometryElementSets();

	// write named surfaces pairs
	WriteGeometrySurfacePairs();

	// write discrete element sets
	WriteGeometryDiscreteSets();
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteMeshDomainsSection()
{
	Part* part = m_Part[0];
	for (int i = 0; i < part->m_Dom.size(); ++i)
	{
		Domain* dom = part->m_Dom[i];

		XMLElement el;
		if (dom->m_elemClass == ELEM_SOLID)
		{
			el.name("SolidDomain");
			el.add_attribute("name", dom->m_name);
			el.add_attribute("mat", dom->m_matName);
			m_xml.add_empty(el);
		}
		else if (dom->m_elemClass == ELEM_SHELL)
		{
			GPart* pg = dom->m_pg;
			el.name("ShellDomain");
			el.add_attribute("name", dom->m_name);
			el.add_attribute("mat", dom->m_matName);

			if (pg && pg->Parameters())
			{
				m_xml.add_branch(el);
				{
					WriteParamList(*pg);
				}
				m_xml.close_branch();
			}
			else m_xml.add_empty(el);
		}
		else
		{
			assert(false);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteGeometryObject(FEBioExport4::Part* part)
{
	GObject* po = part->m_obj;

	// get the mesh
	FECoreMesh* pm = po->GetFEMesh();

	// Write the nodes
	m_xml.add_branch("Nodes");
	{
		XMLElement el("node");
		int nid = el.add_attribute("id", 0);
		for (int j = 0; j < pm->Nodes(); ++j)
		{
			FENode& node = pm->Node(j);
			el.set_attribute(nid, ++m_ntotnodes);
			vec3d r = node.r;
			el.value(r);
			m_xml.add_leaf(el, false);
		}
	}
	m_xml.close_branch();

	// write all elements
	int NP = po->Parts();
	for (int p = 0; p < NP; ++p)
	{
		// get the part
		GPart* pg = po->Part(p);

		// write this part
		WriteGeometryPart(part, pg, false);
	}

	// write all node sets
	for (int i = 0; i < part->m_NSet.size(); ++i)
	{
		NodeSet* ns = part->m_NSet[i];
		WriteNodeSet(ns->m_name.c_str(), ns->m_nodeList);
	}

	// write all surfaces
	for (int i = 0; i < part->m_Surf.size(); ++i)
	{
		Surface* s = part->m_Surf[i];
		XMLElement el("Surface");
		el.add_attribute("name", s->m_name);
		m_xml.add_branch(el);
		{
			FEFaceList* faceList = s->m_faceList;
			WriteSurfaceSection(*faceList);
		}
		m_xml.close_branch();
	}

	// write all element sets
	for (int i = 0; i < part->m_ELst.size(); ++i)
	{
		ElementList* es = part->m_ELst[i];
		XMLElement el("ElementSet");
		el.add_attribute("name", es->m_name);
		m_xml.add_branch(el);
		{
			// TODO: This writes the old format. Need to fix it, if I still want this Geometry section. 
			assert(false);
			WriteElementList(*es->m_elemList);
		}
		m_xml.close_branch();
	}
}

//-----------------------------------------------------------------------------
bool FEBioExport4::WriteNodeSet(const string& name, FENodeList* pl)
{
	if (pl == 0)
	{
		// TODO: We can get here if the item list builder that generates this points to a non-existing list.
		// This can happen, for instance, if the user deleted the corresponding part or surface. These invalid
		// lists currently remain part of the model but would create null pointers here. 
		// For now, I'm returning true as if nothing is wrong, but obviously I need a better solution.
		return true;
	}

	int nn = pl->Size();
	FENodeList::Iterator pn = pl->First();
	vector<int> m(nn);
	for (int n = 0; n < nn; ++n, pn++)
	{
		FENode* pnode = pn->m_pi;
		if (pnode == 0) return false;
		m[n] = pnode->m_nid;
	}

	XMLElement el("NodeSet");
	el.add_attribute("name", name.c_str());
	m_xml.add_leaf(el, m);
	return true;
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteGeometryNodeSetsNew()
{
	// Write the BC node sets
	int NS = (int)m_pNSet.size();
	for (int i = 0; i < NS; ++i)
	{
		FEItemListBuilder* pil = m_pNSet[i].m_list;
		const string& listName = m_pNSet[i].m_name;
		XMLElement tag("NodeSet");
		tag.add_attribute("name", m_pNSet[i].m_name.c_str());
		m_xml.add_branch(tag);
		switch (pil->Type())
		{
		case GO_NODE:
		{
			GNodeList* itemList = dynamic_cast<GNodeList*>(pil); assert(itemList);
			vector<GNode*> nodeList = itemList->GetNodeList();
			for (size_t i = 0; i < nodeList.size(); ++i)
			{
				GNode* node = nodeList[i];
				GObject* po = dynamic_cast<GObject*>(node->Object());
				string name = string(po->GetName()) + "." + string(node->GetName());
				m_xml.add_leaf("node_set", name);
			}
		}
		break;
		case GO_EDGE:
		{
			GEdgeList* itemList = dynamic_cast<GEdgeList*>(pil); assert(itemList);
			vector<GEdge*> edgeList = itemList->GetEdgeList();
			for (size_t i = 0; i < edgeList.size(); ++i)
			{
				GEdge* edge = edgeList[i];
				GObject* po = dynamic_cast<GObject*>(edge->Object());
				string name = string(po->GetName()) + "." + string(edge->GetName());
				m_xml.add_leaf("node_set", name);
			}
		}
		break;
		case GO_FACE:
		{
			GFaceList* itemList = dynamic_cast<GFaceList*>(pil); assert(itemList);
			vector<GFace*> faceList = itemList->GetFaceList();
			for (size_t i = 0; i < faceList.size(); ++i)
			{
				GFace* face = faceList[i];
				GObject* po = dynamic_cast<GObject*>(face->Object());
				string name = string(po->GetName()) + "." + string(face->GetName());
				m_xml.add_leaf("node_set", name);
			}
		}
		break;
		case GO_PART:
		{
			GPartList* itemList = dynamic_cast<GPartList*>(pil); assert(itemList);
			vector<GPart*> partList = itemList->GetPartList();
			for (size_t i = 0; i < partList.size(); ++i)
			{
				GPart* part = partList[i];
				GObject* po = dynamic_cast<GObject*>(part->Object());
				string name = string(po->GetName()) + "." + string(part->GetName());
				m_xml.add_leaf("node_set", name);
			}
		}
		break;
		case FE_NODESET:
		{
			FENodeSet* nodeSet = dynamic_cast<FENodeSet*>(pil); assert(nodeSet);
			for (int i = 0; i < m_Part.size(); ++i)
			{
				Part* part = m_Part[i];
				NodeSet* ns = part->FindNodeSet(m_pNSet[i].m_name.c_str());
				if (ns)
				{
					GObject* po = part->m_obj;
					string name = string(po->GetName()) + "." + listName;
					m_xml.add_leaf("node_set", name);
					break;
				}
			}
		}
		break;
		case FE_SURFACE:
		{
			FESurface* face = dynamic_cast<FESurface*>(pil); assert(face);
			for (int i = 0; i < m_Part.size(); ++i)
			{
				Part* part = m_Part[i];
				NodeSet* ns = part->FindNodeSet(m_pNSet[i].m_name.c_str());
				if (ns)
				{
					GObject* po = part->m_obj;
					string name = string(po->GetName()) + "." + listName;
					m_xml.add_leaf("node_set", name);
					break;
				}
			}
		}
		break;
		default:
			assert(false);
		}
		m_xml.close_branch();
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteGeometryNodeSets()
{
	// Write the BC node sets
	int NS = (int)m_pNSet.size();
	for (int i = 0; i < NS; ++i)
	{
		if (m_pNSet[i].m_duplicate == false)
		{
			FEItemListBuilder* pil = m_pNSet[i].m_list;
			unique_ptr<FENodeList> pl(pil->BuildNodeList());
			if (WriteNodeSet(m_pNSet[i].m_name.c_str(), pl.get()) == false)
			{
				throw InvalidItemListBuilder(pil);
			}
		}
	}

	// Write the user-defined node sets
	FSModel& fem = *m_pfem;
	GModel& model = fem.GetModel();

	// first, do model-level node sets
	for (int i = 0; i < model.NodeLists(); ++i)
	{
		GNodeList* pg = model.NodeList(i);
		unique_ptr<FENodeList> pn(pg->BuildNodeList());
		if (WriteNodeSet(pg->GetName(), pn.get()) == false)
		{
			throw InvalidItemListBuilder(pg);
		}
	}

	// Then, do object-level node sets
	int nobj = model.Objects();
	for (int i = 0; i < nobj; ++i)
	{
		GObject* po = model.Object(i);
		FEMesh* pm = po->GetFEMesh();
		if (pm)
		{
			int nset = po->FENodeSets();
			for (int j = 0; j < nset; ++j)
			{
				FENodeSet* pns = po->GetFENodeSet(j);
				unique_ptr<FENodeList> pl(pns->BuildNodeList());
				if (WriteNodeSet(pns->GetName(), pl.get()) == false)
				{
					throw InvalidItemListBuilder(po);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteGeometrySurfaces()
{
	int NS = (int)m_pSurf.size();
	for (int i = 0; i < NS; ++i)
	{
		XMLElement el("Surface");
		el.add_attribute("name", m_pSurf[i].m_name.c_str());
		m_xml.add_branch(el);
		{
			WriteSurfaceSection(m_pSurf[i]);
		}
		m_xml.close_branch();
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteGeometryElementSets()
{
	int NS = (int)m_pESet.size();
	for (int i = 0; i < NS; ++i)
	{
		FEItemListBuilder* pl = m_pESet[i].m_list;
		unique_ptr<FEElemList> ps(pl->BuildElemList());
		XMLElement el("ElementSet");
		el.add_attribute("name", m_pESet[i].m_name.c_str());

		int NE = ps->Size();
		vector<int> elemIds;
		FEElemList::Iterator pe = ps->First();
		for (int i = 0; i < NE; ++i, ++pe)
		{
			FEElement_& el = *(pe->m_pi);
			elemIds.push_back(el.m_nid);
		}

		m_xml.add_leaf(el, elemIds);
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteGeometrySurfacesNew()
{
	int NS = (int)m_pSurf.size();
	for (int i = 0; i < NS; ++i)
	{
		FEItemListBuilder* pl = m_pSurf[i].m_list;
		XMLElement el("Surface");
		string sname = m_pSurf[i].m_name;
		el.add_attribute("name", m_pSurf[i].m_name.c_str());
		m_xml.add_branch(el);
		{
			switch (pl->Type())
			{
			case GO_FACE:
			{
				GFaceList* itemList = dynamic_cast<GFaceList*>(pl); assert(itemList);
				vector<GFace*> faceList = itemList->GetFaceList();
				for (size_t i = 0; i < faceList.size(); ++i)
				{
					GFace* face = faceList[i];
					GObject* po = dynamic_cast<GObject*>(face->Object());
					string name = string(po->GetName()) + "." + string(face->GetName());
					m_xml.add_leaf("surface", name);
				}
			}
			break;
			case FE_SURFACE:
			{
				FESurface* surf = dynamic_cast<FESurface*>(pl); assert(surf);
				GObject* po = surf->GetGObject();
				string name = string(po->GetName()) + "." + sname;
				m_xml.add_leaf("surface", name);
			}
			break;
			default:
				break;
			}
		}
		m_xml.close_branch();
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteGeometryElementSetsNew()
{
	int NS = (int)m_pESet.size();
	for (int i = 0; i < NS; ++i)
	{
		FEItemListBuilder* pl = m_pESet[i].m_list;
		XMLElement el("ElementSet");
		string sname = m_pESet[i].m_name;
		el.add_attribute("name", m_pESet[i].m_name.c_str());
		m_xml.add_branch(el);
		{
			switch (pl->Type())
			{
			case GO_PART:
			{
				GPartList* itemList = dynamic_cast<GPartList*>(pl); assert(itemList);
				vector<GPart*> partList = itemList->GetPartList();
				for (size_t i = 0; i < partList.size(); ++i)
				{
					GPart* pg = partList[i];
					GObject* po = dynamic_cast<GObject*>(pg->Object());
					string name = string(po->GetName()) + "." + string(pg->GetName());
					m_xml.add_leaf("elem_set", name);
				}
			}
			break;
			case FE_PART:
			{
				FEPart* part = dynamic_cast<FEPart*>(pl); assert(part);
				GObject* po = part->GetGObject();
				string name = string(po->GetName()) + "." + sname;
				m_xml.add_leaf("elem_set", name);
			}
			break;
			default:
				break;
			}
		}
		m_xml.close_branch();
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteGeometrySurfacePairs()
{
	// get the named surfaces (paired interfaces)
	FSModel& fem = *m_pfem;
	for (int i = 0; i < fem.Steps(); ++i)
	{
		FSStep* pstep = fem.GetStep(i);
		for (int j = 0; j < pstep->Interfaces(); ++j)
		{
			FSInterface* pj = pstep->Interface(j);
			FSPairedInterface* pi = dynamic_cast<FSPairedInterface*>(pj);

			// NOTE: don't export spring-tied interfaces!
			if (dynamic_cast<FSSpringTiedInterface*>(pi)) pi = 0;

			if (pi && pi->IsActive())
			{
				FEItemListBuilder* pms = pi->GetSecondarySurface();
				if (pms == 0) throw InvalidItemListBuilder(pi);

				FEItemListBuilder* pss = pi->GetPrimarySurface();
				if (pss == 0) throw InvalidItemListBuilder(pi);

				XMLElement el("SurfacePair");
				el.add_attribute("name", pi->GetName().c_str());
				m_xml.add_branch(el);
				{
					m_xml.add_leaf("primary", GetSurfaceName(pss));
					m_xml.add_leaf("secondary", GetSurfaceName(pms));
				}
				m_xml.close_branch();
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Write the Nodes sections.
// One Nodes section is written for each object.
void FEBioExport4::WriteGeometryNodes()
{
	FSModel& s = *m_pfem;
	GModel& model = s.GetModel();

	int n = 1;
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		FECoreMesh* pm = po->GetFEMesh();

		XMLElement tagNodes("Nodes");
		const string& name = po->GetName();
		if (name.empty() == false) tagNodes.add_attribute("name", name.c_str());

		m_xml.add_branch(tagNodes);
		{
			XMLElement el("node");
			int nid = el.add_attribute("id", 0);
			for (int j = 0; j < pm->Nodes(); ++j, ++n)
			{
				FENode& node = pm->Node(j);
				node.m_nid = n;
				el.set_attribute(nid, n);
				vec3d r = po->GetTransform().LocalToGlobal(node.r);
				el.value(r);
				m_xml.add_leaf(el, false);
			}
		}
		m_xml.close_branch();
	}

	// add the deformable springs
	for (int i = 0; i < model.DiscreteObjects(); ++i)
	{
		GDiscreteObject* po = model.DiscreteObject(i);
		if (po->GetType() == FE_DEFORMABLE_SPRING)
		{
			GDeformableSpring& spring = dynamic_cast<GDeformableSpring&>(*po);
			spring.m_ntag = -1;

			GNode& nodeA = *model.FindNode(spring.NodeID(0));
			GNode& nodeB = *model.FindNode(spring.NodeID(1));

			GObject& objA = dynamic_cast<GObject&>(*nodeA.Object());
			GObject& objB = dynamic_cast<GObject&>(*nodeB.Object());

			vec3d ra = objA.GetTransform().LocalToGlobal(nodeA.LocalPosition());
			vec3d rb = objB.GetTransform().LocalToGlobal(nodeB.LocalPosition());

			int ndivs = spring.Divisions();
			if (ndivs > 1)
			{
				spring.m_ntag = n;

				XMLElement tagNodes("Nodes");
				const string& name = po->GetName();
				if (name.empty() == false) tagNodes.add_attribute("name", name.c_str());
				m_xml.add_branch(tagNodes);
				{
					for (int j = 0; j < ndivs - 1; ++j)
					{
						double w = (double)(j + 1.0) / (double)ndivs;
						vec3d r = ra + (rb - ra) * w;

						XMLElement el("n");
						el.add_attribute("id", n++);
						el.value(r);
						m_xml.add_leaf(el);
					}
				}
				m_xml.close_branch();
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteMeshElements()
{
	FSModel& s = *m_pfem;
	GModel& model = s.GetModel();

	// reset element counter
	m_ntotelem = 0;

	Part* part = m_Part[0];

	// loop over all objects
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);

		// loop over all parts
		int NP = po->Parts();
		for (int p = 0; p < NP; ++p)
		{
			// get the part
			GPart* pg = po->Part(p);

			// write this part
			WriteGeometryPart(part, pg, false, false);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteGeometryElements(bool writeMats, bool useMatNames)
{
	FSModel& s = *m_pfem;
	GModel& model = s.GetModel();

	// reset element counter
	m_ntotelem = 0;

	// loop over all objects
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);

		// loop over all parts
		int NP = po->Parts();
		for (int p = 0; p < NP; ++p)
		{
			// get the part
			GPart* pg = po->Part(p);

			// write this part
			WriteGeometryPart(nullptr, pg, writeMats, useMatNames);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteGeometryPart(Part* part, GPart* pg, bool writeMats, bool useMatNames)
{
	FSModel& s = *m_pfem;
	GModel& model = s.GetModel();
	GObject* po = dynamic_cast<GObject*>(pg->Object());
	FECoreMesh* pm = po->GetFEMesh();
	int pid = pg->GetLocalID();

	// Parts must be split up by element type
	int NE = pm->Elements();
	int NEP = 0; // number of elements in part
	for (int i = 0; i < NE; ++i)
	{
		FEElement_& el = pm->ElementRef(i);
		if (el.m_gid == pid) { el.m_ntag = 1; NEP++; }
		else el.m_ntag = -1;
	}

	// make sure this part has elements
	if (NEP == 0) return;

	// get the material
	int nmat = 0;
	GMaterial* pmat = s.GetMaterialFromID(pg->GetMaterialID());
	if (pmat) nmat = pmat->m_ntag;

	// loop over unprocessed elements
	int nset = 0;
	int ncount = 0;
	int nn[FEElement::MAX_NODES];
	char szname[128] = { 0 };
	for (int i = 0; ncount < NEP; ++i)
	{
		FEElement_& el = pm->ElementRef(i);
		if (el.m_ntag == 1)
		{
			assert(el.m_gid == pid);
			int ntype = el.Type();
			const char* sztype = ElementTypeString(ntype);
			if (sztype == 0) throw FEBioExportError();
			XMLElement xe("Elements");
			if (sztype) xe.add_attribute("type", sztype);
			if ((nmat > 0) && writeMats)
			{
				if (useMatNames) xe.add_attribute("mat", pmat->GetName().c_str());
				else xe.add_attribute("mat", nmat);
			}

			if (nset == 0)
				sprintf(szname, "%s", pg->GetName().c_str());
			else
				sprintf(szname, "%s__%d", pg->GetName().c_str(), nset + 1);

			ElementSet es;
			es.m_mesh = pm;
			es.m_name = szname;
			es.m_matID = pg->GetMaterialID();

			// add a domain
			if (part)
			{
				FEBioExport4::Domain* dom = new FEBioExport4::Domain;
				dom->m_name = szname;
				dom->m_pg = pg;
				if (pmat) dom->m_matName = pmat->GetName().c_str();
				part->m_Dom.push_back(dom);

				dom->m_elemClass = el.Class();
			}

			xe.add_attribute("name", szname);
			m_xml.add_branch(xe);
			{
				XMLElement xej("elem");
				int n1 = xej.add_attribute("id", (int)0);

				for (int j = i; j < NE; ++j)
				{
					FEElement_& ej = pm->ElementRef(j);
					if ((ej.m_ntag == 1) && (ej.Type() == ntype))
					{
						int eid = m_ntotelem + ncount + 1;
						xej.set_attribute(n1, eid);
						int ne = ej.Nodes();
						assert(ne == el.Nodes());
						for (int k = 0; k < ne; ++k) nn[k] = pm->Node(ej.m_node[k]).m_nid;
						xej.value(nn, ne);
						m_xml.add_leaf(xej, false);
						ej.m_ntag = -1;	// mark as processed
						ej.m_nid = eid;
						ncount++;

						es.m_elem.push_back(j);
					}
				}
			}
			m_xml.close_branch();

			nset++;
			m_ElSet.push_back(es);
		}
	}

	// update total element counter
	m_ntotelem += ncount;
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteGeometryDiscreteSets()
{
	FSModel& fem = *m_pfem;
	GModel& model = fem.GetModel();

	// write the discrete element sets
	for (int i = 0; i < model.DiscreteObjects(); ++i)
	{
		GLinearSpring* ps = dynamic_cast<GLinearSpring*>(model.DiscreteObject(i));
		if (ps)
		{
			GNode* pn0 = model.FindNode(ps->m_node[0]);
			GNode* pn1 = model.FindNode(ps->m_node[1]);
			if (pn0 && pn1)
			{
				GObject* po0 = dynamic_cast<GObject*>(pn0->Object()); assert(po0);
				GObject* po1 = dynamic_cast<GObject*>(pn1->Object()); assert(po1);

				int n[2];
				n[0] = po0->GetFENode(pn0->GetLocalID())->m_nid;
				n[1] = po1->GetFENode(pn1->GetLocalID())->m_nid;

				XMLElement el("DiscreteSet");
				el.add_attribute("name", ps->GetName().c_str());
				m_xml.add_branch(el);
				{
					m_xml.add_leaf("delem", n, 2);
				}
				m_xml.close_branch();
			}
		}
		GGeneralSpring* pg = dynamic_cast<GGeneralSpring*>(model.DiscreteObject(i));
		if (pg)
		{
			GNode* pn0 = model.FindNode(pg->m_node[0]);
			GNode* pn1 = model.FindNode(pg->m_node[1]);
			if (pn0 && pn1)
			{
				GObject* po0 = dynamic_cast<GObject*>(pn0->Object());
				GObject* po1 = dynamic_cast<GObject*>(pn1->Object());

				int n[2];
				n[0] = po0->GetFENode(pn0->GetLocalID())->m_nid;
				n[1] = po1->GetFENode(pn1->GetLocalID())->m_nid;

				XMLElement el("DiscreteSet");
				el.add_attribute("name", pg->GetName().c_str());
				m_xml.add_branch(el);
				{
					m_xml.add_leaf("delem", n, 2);
				}
				m_xml.close_branch();
			}
		}
		GDiscreteSpringSet* pds = dynamic_cast<GDiscreteSpringSet*>(model.DiscreteObject(i));
		if (pds && (pds->size()))
		{
			XMLElement el("DiscreteSet");
			el.add_attribute("name", pds->GetName().c_str());
			m_xml.add_branch(el);
			{
				int N = pds->size();
				for (int n = 0; n < N; ++n)
				{
					GDiscreteElement& el = pds->element(n);
					GNode* pn0 = model.FindNode(el.Node(0));
					GNode* pn1 = model.FindNode(el.Node(1));
					if (pn0 && pn1)
					{
						GObject* po0 = dynamic_cast<GObject*>(pn0->Object());
						GObject* po1 = dynamic_cast<GObject*>(pn1->Object());

						int n[2];
						n[0] = po0->GetFENode(pn0->GetLocalID())->m_nid;
						n[1] = po1->GetFENode(pn1->GetLocalID())->m_nid;

						m_xml.add_leaf("delem", n, 2);
					}
				}
			}
			m_xml.close_branch();
		}
		GDeformableSpring* ds = dynamic_cast<GDeformableSpring*>(model.DiscreteObject(i));
		if (ds)
		{
			GNode* pn0 = model.FindNode(ds->NodeID(0));
			GNode* pn1 = model.FindNode(ds->NodeID(1));
			if (pn0 && pn1)
			{
				GObject* po0 = dynamic_cast<GObject*>(pn0->Object());
				GObject* po1 = dynamic_cast<GObject*>(pn1->Object());

				int n[2], m[2] = { 0 };
				n[0] = po0->GetFENode(pn0->GetLocalID())->m_nid;
				n[1] = po1->GetFENode(pn1->GetLocalID())->m_nid;

				XMLElement el("DiscreteSet");
				el.add_attribute("name", ds->GetName().c_str());
				m_xml.add_branch(el);
				{
					int N = ds->Divisions();
					if (N == 1) m_xml.add_leaf("delem", n, 2);
					else
					{
						int nid = ds->m_ntag; assert(nid != -1);
						m[0] = n[0]; m[1] = nid;
						m_xml.add_leaf("delem", m, 2);
						for (int j = 0; j < N - 2; ++j)
						{
							m[0] = nid++;
							m[1] = nid;
							m_xml.add_leaf("delem", m, 2);
						}
						m[0] = nid; m[1] = n[1];
						m_xml.add_leaf("delem", m, 2);
					}
				}
				m_xml.close_branch();
			}
		}
	}

	// write the spring-tied interfaces
	for (int i = 0; i < fem.Steps(); ++i)
	{
		FSStep* step = fem.GetStep(i);
		for (int j = 0; j < step->Interfaces(); ++j)
		{
			FSSpringTiedInterface* pst = dynamic_cast<FSSpringTiedInterface*>(step->Interface(j));
			if (pst && pst->IsActive())
			{
				vector<pair<int, int> > L;
				pst->BuildSpringList(L);

				XMLElement el("DiscreteSet");
				el.add_attribute("name", pst->GetName().c_str());
				m_xml.add_branch(el);
				{
					auto N = L.size();
					for (int n = 0; n < N; ++n)
					{
						pair<int, int>& de = L[n];
						int m[2] = { de.first, de.second };
						m_xml.add_leaf("delem", m, 2);
					}
				}
				m_xml.close_branch();
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteMeshDataSection()
{
	WriteElementDataSection();
	WriteSurfaceDataSection();
	WriteEdgeDataSection();
	WriteNodeDataSection();

	FSModel& fem = *m_pfem;
	int N = fem.DataMaps();
	for (int i = 0; i < N; ++i)
	{
		FEDataMapGenerator* map = fem.GetDataMap(i);
		WriteMeshData(map);
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteElementDataSection()
{
	WriteMeshDataShellThickness();

	WriteMeshDataMaterialFibers();

	WriteMeshDataMaterialAxes();

	WriteElementDataFields();
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteMeshData(FEDataMapGenerator* map)
{
	XMLElement meshData("ElementData");
	meshData.add_attribute("var", map->m_var);
	meshData.add_attribute("generator", map->m_generator);
	meshData.add_attribute("elem_set", map->m_elset);


	m_xml.add_branch(meshData);
	{
		FESurfaceToSurfaceMap* s2s = dynamic_cast<FESurfaceToSurfaceMap*>(map);
		if (s2s)
		{
			m_xml.add_leaf("bottom_surface", s2s->GetBottomSurface());
			m_xml.add_leaf("top_surface", s2s->GetTopSurface());

			Param* p = s2s->GetParam("function");
			XMLElement e("function");
			e.add_attribute("type", "point");
			m_xml.add_branch(e);
			{
				FELoadCurve& lc = *p->GetLoadCurve();
				m_xml.add_branch("points");
				{
					for (int i = 0; i < lc.Size(); ++i)
					{
						double v[2] = { lc[i].time, lc[i].load };
						m_xml.add_leaf("point", v, 2);
					}
				}
				m_xml.close_branch();
			}
			m_xml.close_branch();
		}
	}
	m_xml.close_branch();
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteMeshDataShellThickness()
{
	FSModel& fem = *m_pfem;
	GModel& model = fem.GetModel();
	for (int i = 0; i < (int)m_ElSet.size(); ++i)
	{
		ElementSet& elset = m_ElSet[i];
		FECoreMesh* pm = elset.m_mesh;

		// see if this mesh has shells
		bool bshell = false;
		for (int k = 0; k < (int)elset.m_elem.size(); ++k)
		{
			FEElement_& el = pm->ElementRef(elset.m_elem[k]);
			if (el.IsShell()) { bshell = true; break; }
		}

		// write shell thickness data
		if (bshell)
		{
			XMLElement tag("ElementData");
			tag.add_attribute("var", "shell thickness");
			tag.add_attribute("elem_set", elset.m_name.c_str());
			m_xml.add_branch(tag);
			{
				XMLElement el("e");
				int n1 = el.add_attribute("lid", 0);

				int nid = 1;
				for (int k = 0; k < (int)elset.m_elem.size(); ++k)
				{
					FEElement_& e = pm->ElementRef(elset.m_elem[k]);
					if (e.IsShell())
					{
						el.set_attribute(n1, nid++);
						el.value(e.m_h, e.Nodes());
						m_xml.add_leaf(el, false);
					}
				}
			}
			m_xml.close_branch();
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteMeshDataMaterialFibers()
{
	FSModel& fem = *m_pfem;

	// loop over all element sets
	auto NSET = m_ElSet.size();
	for (int i = 0; i < NSET; ++i)
	{
		ElementSet& elSet = m_ElSet[i];
		FECoreMesh* pm = elSet.m_mesh;
		GObject* po = pm->GetGObject();
		const Transform& T = po->GetTransform();

		GMaterial* pmat = fem.GetMaterialFromID(elSet.m_matID);
		FSTransverselyIsotropic* ptiso = 0;
		if (pmat) ptiso = dynamic_cast<FSTransverselyIsotropic*>(pmat->GetMaterialProperties());

		if (ptiso && (ptiso->GetFiberMaterial()->m_naopt == FE_FIBER_USER))
		{
			int NE = (int)elSet.m_elem.size();
			XMLElement tag("ElementData");
			tag.add_attribute("var", "fiber");
			tag.add_attribute("elem_set", elSet.m_name.c_str());
			m_xml.add_branch(tag);
			{
				XMLElement el("e");
				int nid = el.add_attribute("lid", 0);
				for (int j = 0; j < NE; ++j)
				{
					FEElement_& e = pm->ElementRef(elSet.m_elem[j]);
					vec3d a = T.LocalToGlobalNormal(e.m_fiber);
					el.set_attribute(nid, j + 1);
					el.value(a);
					m_xml.add_leaf(el, false);
				}
			}
			m_xml.close_branch(); // elem_data
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteMeshDataMaterialAxes()
{
	// loop over all element sets
	auto NSET = m_ElSet.size();
	for (int i = 0; i < NSET; ++i)
	{
		ElementSet& elSet = m_ElSet[i];
		FECoreMesh* pm = elSet.m_mesh;
		GObject* po = pm->GetGObject();
		const Transform& T = po->GetTransform();

		// make sure there is something to do
		bool bwrite = false;
		int NE = (int)elSet.m_elem.size();
		for (int j = 0; j < NE; ++j)
		{
			FEElement_& el = pm->ElementRef(elSet.m_elem[j]);
			if (el.m_Qactive) { bwrite = true; break; }
		}

		// okay, let's get to work
		if (bwrite)
		{
			int n = 0;
			XMLElement tag("ElementData");
			tag.add_attribute("var", "mat_axis");
			tag.add_attribute("elem_set", elSet.m_name.c_str());
			m_xml.add_branch(tag);
			{
				XMLElement el("e");
				int nid = el.add_attribute("lid", 0);

				for (int j = 0; j < NE; ++j)
				{
					FEElement_& e = pm->ElementRef(elSet.m_elem[j]);
					if (e.m_Qactive)
					{
						// e.m_Q is in local coordinates, so transform it to global coordinates
						mat3d& Q = e.m_Q;
						vec3d a(Q[0][0], Q[1][0], Q[2][0]);
						vec3d d(Q[0][1], Q[1][1], Q[2][1]);
						a = T.LocalToGlobalNormal(a);
						d = T.LocalToGlobalNormal(d);

						el.set_attribute(nid, ++n);
						m_xml.add_branch(el, false);
						{
							m_xml.add_leaf("a", a);
							m_xml.add_leaf("d", d);
						}
						m_xml.close_branch();
					}
				}
				m_xml.close_branch(); // elem_data
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteElementDataFields()
{
	FSModel& fem = *m_pfem;
	GModel& model = fem.GetModel();
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		FEMesh* pm = po->GetFEMesh();
		int NE = pm->Elements();

		int ND = pm->MeshDataFields();
		for (int n = 0; n < ND; ++n)
		{
			FEElementData* meshData = dynamic_cast<FEElementData*>(pm->GetMeshDataField(n));
			if (meshData)
			{
				FEElementData& data = *meshData;
				const FEPart* pg = data.GetPart();

				string name = pg->GetName();
				if (name.empty()) name = data.GetName();

				XMLElement tag("ElementData");
				tag.add_attribute("name", data.GetName().c_str());
				tag.add_attribute("elem_set", name);
				m_xml.add_branch(tag);
				{
					XMLElement el("e");
					int nid = el.add_attribute("lid", 0);
					FEItemListBuilder::ConstIterator it = pg->begin();
					for (int j = 0; j < pg->size(); ++j, ++it)
					{
						int eid = *it;
						FEElement_& e = pm->ElementRef(eid);
						el.set_attribute(nid, j + 1);
						el.value(data[j]);
						m_xml.add_leaf(el, false);
					}
				}
				m_xml.close_branch();
			}
			FEPartData* partData = dynamic_cast<FEPartData*>(pm->GetMeshDataField(n));
			if (partData)
			{
				double v[FEElement::MAX_NODES] = { 0 };
				FEPartData& data = *partData;
				GPartList* partList = data.GetPartList(&fem);
				std::vector<GPart*> partArray = partList->GetPartList();
				FEElemList* elemList = data.BuildElemList();
				for (int np = 0; np < partArray.size(); ++np)
				{
					GPart* pg = partArray[np];
					int pid = pg->GetLocalID();

					XMLElement tag("ElementData");
					tag.add_attribute("name", data.GetName().c_str());
					tag.add_attribute("elem_set", pg->GetName());
					m_xml.add_branch(tag);
					{
						XMLElement el("e");
						int nid = el.add_attribute("lid", 0);
						int N = elemList->Size();
						FEElemList::Iterator it = elemList->First();
						int lid = 1;
						for (int j = 0; j < N; ++j, ++it)
						{
							FEElement_* pe = it->m_pi;
							if (pe->m_gid == pid)
							{
								el.set_attribute(nid, lid++);

								if (data.GetDataFormat() == FEMeshData::DATA_ITEM)
								{
									el.value(data[j]);
								}
								else if (data.GetDataFormat() == FEMeshData::DATA_MULT)
								{
									int nn = pe->Nodes();
									for (int k = 0; k < nn; ++k) v[k] = data.GetValue(j, k);
									el.value(v, nn);
								}

								m_xml.add_leaf(el, false);
							}
						}
					}
					m_xml.close_branch();
				}
				delete partList;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteSurfaceDataSection()
{
	FSModel& fem = *m_pfem;
	GModel& model = fem.GetModel();

	for (int i = 0; i < model.Objects(); i++)
	{
		FEMesh* mesh = model.Object(i)->GetFEMesh();

		for (int j = 0; j < mesh->MeshDataFields(); j++)
		{
			FESurfaceData* surfData = dynamic_cast<FESurfaceData*>(mesh->GetMeshDataField(j));
			if (surfData)
			{
				FESurfaceData& sd = *surfData;

				XMLElement tag("SurfaceData");
				tag.add_attribute("name", sd.GetName().c_str());

				if (sd.GetDataType() == FEMeshData::DATA_TYPE::DATA_SCALAR) tag.add_attribute("data_type", "scalar");
				else if (sd.GetDataType() == FEMeshData::DATA_TYPE::DATA_VEC3D) tag.add_attribute("data_type", "vector");

				tag.add_attribute("surface", sd.getSurface()->GetName().c_str());

				m_xml.add_branch(tag);
				{
					XMLElement el("face");
					int n1 = el.add_attribute("lid", 0);

					int nid = 1;
					for (double d : *(sd.getData()))
					{
						el.set_attribute(n1, nid++);
						el.value(d);

						m_xml.add_leaf(el, false);
					}


				}
				m_xml.close_branch();
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteEdgeDataSection()
{
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteNodeDataSection()
{
	FSModel& fem = *m_pfem;
	GModel& model = fem.GetModel();

	for (int i = 0; i < model.Objects(); i++)
	{
		FEMesh* mesh = model.Object(i)->GetFEMesh();

		for (int j = 0; j < mesh->MeshDataFields(); j++)
		{
			FENodeData* nodeData = dynamic_cast<FENodeData*>(mesh->GetMeshDataField(j));
			if (nodeData)
			{
				FENodeData& nd = *nodeData;

				XMLElement tag("NodeData");
				tag.add_attribute("name", nd.GetName().c_str());

				if (nd.GetDataType() == FEMeshData::DATA_TYPE::DATA_SCALAR) tag.add_attribute("data_type", "scalar");
				else if (nd.GetDataType() == FEMeshData::DATA_TYPE::DATA_VEC3D) tag.add_attribute("data_type", "vector");

				FEItemListBuilder* pitem = nd.GetItemList();
				tag.add_attribute("node_set", GetNodeSetName(pitem));

				m_xml.add_branch(tag);
				{
					XMLElement el("node");
					int n1 = el.add_attribute("lid", 0);

					int nid = 1;
					for (int i = 0; i < nd.Size(); ++i)
					{
						el.set_attribute(n1, nid++);
						el.value(nd.get(i));

						m_xml.add_leaf(el, false);
					}
				}
				m_xml.close_branch();
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteBoundarySection(FSStep& s)
{
	FSModel& fem = m_prj.GetFSModel();
	for (int i = 0; i < s.BCs(); ++i)
	{
		FSBoundaryCondition* pbc = s.BC(i);
		if (pbc && pbc->IsActive())
		{
			WriteBC(s, pbc);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteRigidSection(FSStep& s)
{
	// rigid body constraints
	WriteRigidConstraints(s);

	// rigid loads
	WriteRigidLoads(s);

	// rigid connectors
	WriteConnectors(s);
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteContactSection(FSStep& s)
{
	// --- C O N T A C T ---
	for (int i = 0; i < s.Interfaces(); ++i)
	{
		FSPairedInterface* pi = dynamic_cast<FSPairedInterface*> (s.Interface(i));
		if (pi && pi->IsActive())
		{
			if (m_writeNotes) WriteNote(pi);

			XMLElement ec("contact");
			ec.add_attribute("type", pi->GetTypeString());
			const char* sz = pi->GetName().c_str();
			ec.add_attribute("name", sz);
			ec.add_attribute("surface_pair", sz);

			m_xml.add_branch(ec);
			{
				WriteParamList(*pi);
			}
			m_xml.close_branch();
		}
	}

	// linear constraints
	WriteLinearConstraints(s);
}

//-----------------------------------------------------------------------------
// Write the loads section
void FEBioExport4::WriteLoadsSection(FSStep& s)
{
	// nodal loads
	WriteNodalLoads(s);

	// surface loads
	WriteSurfaceLoads(s);

	// body loads
	WriteBodyLoads(s);
}

//-----------------------------------------------------------------------------
// write discrete elements
//
void FEBioExport4::WriteDiscreteSection(FSStep& s)
{
	FSModel& fem = *m_pfem;
	GModel& model = fem.GetModel();

	// Write the discrete materials
	XMLElement dmat("discrete_material");
	int n1 = dmat.add_attribute("id", 0);
	int n2 = dmat.add_attribute("name", "");
	int n3 = dmat.add_attribute("type", "");

	int n = 1;
	for (int i = 0; i < model.DiscreteObjects(); ++i)
	{
		GLinearSpring* ps = dynamic_cast<GLinearSpring*>(model.DiscreteObject(i));
		if (ps)
		{
			dmat.set_attribute(n1, n++);
			dmat.set_attribute(n2, ps->GetName().c_str());
			dmat.set_attribute(n3, "linear spring");
			m_xml.add_branch(dmat, false);
			{
				WriteParamList(*ps);
			}
			m_xml.close_branch();
		}
		GGeneralSpring* pg = dynamic_cast<GGeneralSpring*>(model.DiscreteObject(i));
		if (pg)
		{
			dmat.set_attribute(n1, n++);
			dmat.set_attribute(n2, pg->GetName().c_str());
			dmat.set_attribute(n3, "nonlinear spring");
			m_xml.add_branch(dmat, false);
			{
				Param& p = pg->GetParam(GGeneralSpring::MP_F);
				double F = p.GetFloatValue();
				int lc = (p.GetLoadCurve() ? p.GetLoadCurve()->GetID() : -1);

				XMLElement f;
				f.name("force");
				f.value(F);
				if (lc > 0) f.add_attribute("lc", lc);
				m_xml.add_leaf(f);
			}
			m_xml.close_branch();
		}
		GDiscreteSpringSet* pds = dynamic_cast<GDiscreteSpringSet*>(model.DiscreteObject(i));
		if (pds && (pds->size()))
		{
			XMLElement dmat("discrete_material");
			dmat.add_attribute("id", n++);
			dmat.add_attribute("name", pds->GetName().c_str());
			FSDiscreteMaterial* dm = pds->GetMaterial();
			WriteMaterial(dm, dmat);
		}
		GDeformableSpring* ds = dynamic_cast<GDeformableSpring*>(model.DiscreteObject(i));
		if (ds)
		{
			GDeformableSpring& spring = dynamic_cast<GDeformableSpring&>(*ds);
			dmat.set_attribute(n1, n++);
			dmat.set_attribute(n2, spring.GetName().c_str());
			dmat.set_attribute(n3, "linear spring");
			m_xml.add_branch(dmat, false);
			{
				WriteParamList(spring);
			}
			m_xml.close_branch();
		}
	}

	// Write the spring-tied interfaces
	for (int i = 0; i < s.Interfaces(); ++i)
	{
		FSSpringTiedInterface* pst = dynamic_cast<FSSpringTiedInterface*>(s.Interface(i));
		if (pst)
		{
			dmat.set_attribute(n1, n++);
			dmat.set_attribute(n2, pst->GetName().c_str());
			dmat.set_attribute(n3, "linear spring");
			m_xml.add_branch(dmat, false);
			{
				m_xml.add_leaf("E", pst->SpringConstant());
			}
			m_xml.close_branch();
		}
	}

	// write the discrete element sets
	XMLElement disc("discrete");
	n1 = disc.add_attribute("dmat", 0);
	n2 = disc.add_attribute("discrete_set", "");
	n = 1;
	for (int i = 0; i < model.DiscreteObjects(); ++i)
	{
		GLinearSpring* ps = dynamic_cast<GLinearSpring*>(model.DiscreteObject(i));
		if (ps)
		{
			disc.set_attribute(n1, n++);
			disc.set_attribute(n2, ps->GetName().c_str());
			m_xml.add_empty(disc, false);
		}
		GGeneralSpring* pg = dynamic_cast<GGeneralSpring*>(model.DiscreteObject(i));
		if (pg)
		{
			disc.set_attribute(n1, n++);
			disc.set_attribute(n2, pg->GetName().c_str());
			m_xml.add_empty(disc, false);
		}
		GDiscreteSpringSet* pds = dynamic_cast<GDiscreteSpringSet*>(model.DiscreteObject(i));
		if (pds && (pds->size()))
		{
			disc.set_attribute(n1, n++);
			disc.set_attribute(n2, pds->GetName().c_str());
			m_xml.add_empty(disc, false);
		}
		GDeformableSpring* ds = dynamic_cast<GDeformableSpring*>(model.DiscreteObject(i));
		if (ds)
		{
			disc.set_attribute(n1, n++);
			disc.set_attribute(n2, ds->GetName().c_str());
			m_xml.add_empty(disc, false);
		}
	}

	// write the spring-tied interfaces
	for (int i = 0; i < s.Interfaces(); ++i)
	{
		FSSpringTiedInterface* pst = dynamic_cast<FSSpringTiedInterface*>(s.Interface(i));
		if (pst)
		{
			disc.set_attribute(n1, n++);
			disc.set_attribute(n2, pst->GetName().c_str());
			m_xml.add_empty(disc, false);
		}
	}
}

//-----------------------------------------------------------------------------
// write linear constraints
void FEBioExport4::WriteLinearConstraints(FSStep& s)
{
	// This list should be consistent with the list of DOFs in FSModel::FSModel()
	const char* szbc[] = { "x", "y", "z", "u", "v", "w", "p", "T", "wx", "wy", "wz", "ef", "sx", "sy", "sz" };

	for (int i = 0; i < s.LinearConstraints(); ++i)
	{
		FSLinearConstraintSet* pset = s.LinearConstraint(i);
		XMLElement ec("contact");
		ec.add_attribute("type", "linear constraint");
		m_xml.add_branch(ec);
		{
			m_xml.add_leaf("tol", pset->m_atol);
			m_xml.add_leaf("penalty", pset->m_penalty);
			m_xml.add_leaf("maxaug", pset->m_nmaxaug);

			int NC = (int)pset->m_set.size();
			for (int j = 0; j < NC; ++j)
			{
				FSLinearConstraintSet::LinearConstraint& LC = pset->m_set[j];
				m_xml.add_branch("linear_constraint");
				{
					int ND = (int)LC.m_dof.size();
					XMLElement ed("node");
					int n1 = ed.add_attribute("id", 0);
					int n2 = ed.add_attribute("bc", 0);
					for (int n = 0; n < ND; ++n)
					{
						FSLinearConstraintSet::LinearConstraint::DOF& dof = LC.m_dof[n];
						ed.set_attribute(n1, dof.node);
						ed.set_attribute(n2, szbc[dof.bc]);
						ed.value(dof.s);
						m_xml.add_leaf(ed, false);
					}
				}
				m_xml.close_branch();
			}
		}
		m_xml.close_branch();
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteConstraints(FSStep& s)
{
	for (int i = 0; i < s.Constraints(); ++i)
	{
		FSModelConstraint* pw = s.Constraint(i);
		if (pw && pw->IsActive())
		{
			if (m_writeNotes) WriteNote(pw);

			XMLElement ec("constraint");
			ec.add_attribute("type", pw->GetTypeString());
			const char* sz = pw->GetName().c_str();
			ec.add_attribute("name", sz);

			FSSurfaceConstraint* psf = dynamic_cast<FSSurfaceConstraint*>(pw);
			if (psf)
			{
				ec.add_attribute("surface", GetSurfaceName(pw->GetItemList()));
			}

			m_xml.add_branch(ec);
			{
				WriteParamList(*pw);
			}
			m_xml.close_branch(); // constraint
		}
	}
}


//-----------------------------------------------------------------------------
// Write the boundary conditions
void FEBioExport4::WriteBC(FSStep& s, FSBoundaryCondition* pbc)
{
	FSModel& fem = m_prj.GetFSModel();

	if (m_writeNotes) WriteNote(pbc);

	// get the item list
	FEItemListBuilder* pitem = pbc->GetItemList();
	if (pitem == 0) throw InvalidItemListBuilder(pbc);

	// get node set name
	string nodeSetName = GetNodeSetName(pitem);

	XMLElement tag("bc");
	tag.add_attribute("name", pbc->GetName());
	tag.add_attribute("type", pbc->GetTypeString());
	tag.add_attribute("node_set", nodeSetName);

	// write the tag
	m_xml.add_branch(tag);
	{
		WriteParamList(*pbc);
	}
	m_xml.close_branch();
}

//-----------------------------------------------------------------------------
// export nodal loads
//
void FEBioExport4::WriteNodalLoads(FSStep& s)
{
	for (int j = 0; j < s.Loads(); ++j)
	{
		FSNodalLoad* pbc = dynamic_cast<FSNodalLoad*>(s.Load(j));
		if (pbc && pbc->IsActive())
		{
			if (m_writeNotes) WriteNote(pbc);

			FEItemListBuilder* pitem = pbc->GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(pbc);

			XMLElement load("nodal_load");
			load.add_attribute("name", pbc->GetName());
			load.add_attribute("type", pbc->GetTypeString());
			load.add_attribute("node_set", GetNodeSetName(pitem));

			m_xml.add_branch(load);
			{
				WriteParamList(*pbc);
			}
			m_xml.close_branch();
		}
	}
}

//----------------------------------------------------------------------------
// Export pressure loads
//
void FEBioExport4::WriteSurfaceLoads(FSStep& s)
{
	for (int j = 0; j < s.Loads(); ++j)
	{
		FSSurfaceLoad* psl = dynamic_cast<FSSurfaceLoad*>(s.Load(j));
		if (psl && psl->IsActive())
		{
			if (m_writeNotes) WriteNote(psl);

			// create the surface list
			FEItemListBuilder* pitem = psl->GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(psl);

			XMLElement load;
			load.name("surface_load");
			load.add_attribute("name", psl->GetName());
			load.add_attribute("type", psl->GetTypeString());
			load.add_attribute("surface", GetSurfaceName(pitem));
			m_xml.add_branch(load);
			{
				WriteParamList(*psl);
			}
			m_xml.close_branch(); // surface_load
		}
	}
}

//-----------------------------------------------------------------------------
// Export initial conditions
//
void FEBioExport4::WriteInitialSection()
{
	FSModel& fem = m_prj.GetFSModel();
	FSStep& s = *fem.GetStep(0);

	// initial velocities
	for (int j = 0; j < s.ICs(); ++j)
	{
		FSInitialCondition* pic = s.IC(j);
		if (pic && pic->IsActive())
		{
			if (m_writeNotes) WriteNote(pic);

			FEItemListBuilder* pitem = pic->GetItemList();
			if (pitem)
			{
				// TODO: Add item list
			}

			XMLElement el("ic");
			el.add_attribute("name", pic->GetName().c_str());
			el.add_attribute("type", pic->GetTypeString());
			m_xml.add_branch(el);
			{
				WriteParamList(*pic);
			}
			m_xml.close_branch();
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteBodyLoads(FSStep& s)
{
	for (int i = 0; i < s.Loads(); ++i)
	{
		FSBodyLoad* pbl = dynamic_cast<FSBodyLoad*>(s.Load(i));
		if (pbl && pbl->IsActive())
		{
			if (m_writeNotes) WriteNote(pbl);

			FEItemListBuilder* pitem = pbl->GetItemList();

			XMLElement el("body_load");
			el.add_attribute("type", pbl->GetTypeString());
			if (pitem) el.add_attribute("elem_set", GetElementSetName(pitem));
			m_xml.add_branch(el);
			{
				WriteParamList(*pbl);
			}
			m_xml.close_branch();
		}
	}
}

//-----------------------------------------------------------------------------

void FEBioExport4::WriteGlobalsSection()
{
	XMLElement el;
	FSModel& fem = *m_pfem;

	if (fem.Parameters())
	{
		m_xml.add_branch("Constants");
		{
			int N = fem.Parameters();
			for (int i = 0; i < N; ++i)
			{
				Param& p = fem.GetParam(i);
				m_xml.add_leaf(p.GetShortName(), p.GetFloatValue());
			}
		}
		m_xml.close_branch();

		if (fem.Solutes() > 0)
		{
			m_xml.add_branch("Solutes");
			{
				int NS = fem.Solutes();
				for (int i = 0; i < NS; ++i)
				{
					FESoluteData& s = fem.GetSoluteData(i);
					XMLElement el;
					el.name("solute");
					el.add_attribute("id", i + 1);
					el.add_attribute("name", s.GetName().c_str());
					m_xml.add_branch(el);
					{
						m_xml.add_leaf("charge_number", s.GetChargeNumber());
						m_xml.add_leaf("molar_mass", s.GetMolarMass());
						m_xml.add_leaf("density", s.GetDensity());
					}
					m_xml.close_branch();
				}
			}
			m_xml.close_branch();
		}

		if (fem.SBMs() > 0)
		{
			m_xml.add_branch("SolidBoundMolecules");
			{
				int NS = fem.SBMs();
				for (int i = 0; i < NS; ++i)
				{
					FESoluteData& s = fem.GetSBMData(i);
					XMLElement el;
					el.name("solid_bound");
					el.add_attribute("id", i + 1);
					el.add_attribute("name", s.GetName().c_str());
					m_xml.add_branch(el);
					{
						m_xml.add_leaf("charge_number", s.GetChargeNumber());
						m_xml.add_leaf("molar_mass", s.GetMolarMass());
						m_xml.add_leaf("density", s.GetDensity());
					}
					m_xml.close_branch();
				}
			}
			m_xml.close_branch();
		}
	}
}

//-----------------------------------------------------------------------------

void FEBioExport4::WriteLoadDataSection()
{
	for (int i = 0; i < (int)m_pLC.size(); ++i)
	{
		FELoadCurve* plc = m_pLC[i];

		XMLElement el;
		el.name("load_controller");
		el.add_attribute("id", i + 1);
		el.add_attribute("type", "loadcurve");

		double d[2];
		m_xml.add_branch(el);
		{
			switch (plc->GetType())
			{
			case FELoadCurve::LC_STEP: m_xml.add_leaf("interpolate", "STEP"); break;
			case FELoadCurve::LC_LINEAR: m_xml.add_leaf("interpolate", "LINEAR"); break;
			case FELoadCurve::LC_SMOOTH: m_xml.add_leaf("interpolate", "SMOOTH"); break;
			case FELoadCurve::LC_CSPLINE: m_xml.add_leaf("interpolate", "CUBIC SPLINE"); break;
			case FELoadCurve::LC_CPOINTS: m_xml.add_leaf("interpolate", "CONTROL POINTS"); break;
			case FELoadCurve::LC_APPROX: m_xml.add_leaf("interpolate", "APPROXIMATION"); break;
			}

			switch (plc->GetExtend())
			{
				//		case FELoadCurve::EXT_CONSTANT     : el.add_attribute("extend", "constant"     ); break;
			case FELoadCurve::EXT_EXTRAPOLATE: m_xml.add_leaf("extend", "EXTRAPOLATE"); break;
			case FELoadCurve::EXT_REPEAT: m_xml.add_leaf("extend", "REPEAT"); break;
			case FELoadCurve::EXT_REPEAT_OFFSET: m_xml.add_leaf("extend", "REPEAT OFFSET"); break;
			}

			m_xml.add_branch("points");
			{
				for (int j = 0; j < plc->Size(); ++j)
				{
					LOADPOINT& pt = plc->Item(j);
					d[0] = pt.time;
					d[1] = pt.load;
					m_xml.add_leaf("point", d, 2);
				}
			}
			m_xml.close_branch();
		}
		m_xml.close_branch(); // loadcurve
	}
}

//-----------------------------------------------------------------------------

void FEBioExport4::WriteSurfaceSection(FEFaceList& s)
{
	XMLElement ef;
	int n = 1, nn[9];

	int NF = s.Size();
	FEFaceList::Iterator pf = s.First();

	int nfn;
	for (int j = 0; j < NF; ++j, ++n, ++pf)
	{
		if (pf->m_pi == 0) throw InvalidItemListBuilder(0);
		FEFace& face = *(pf->m_pi);
		FECoreMesh* pm = pf->m_pm;
		nfn = face.Nodes();
		for (int k = 0; k < nfn; ++k) nn[k] = pm->Node(face.n[k]).m_nid;
		switch (nfn)
		{
		case 3: ef.name("tri3"); break;
		case 4: ef.name("quad4"); break;
		case 6: ef.name("tri6"); break;
		case 7: ef.name("tri7"); break;
		case 8: ef.name("quad8"); break;
		case 9: ef.name("quad9"); break;
		case 10: ef.name("tri10"); break;
		default:
			assert(false);
		}
		ef.add_attribute("id", n);
		ef.value(nn, nfn);
		m_xml.add_leaf(ef);
	}
}

void FEBioExport4::WriteSurfaceSection(NamedItemList& l)
{
	FEItemListBuilder* pl = l.m_list;
	FEFaceList* pfl = pl->BuildFaceList();
	if (pfl == nullptr) throw InvalidItemListBuilder(l.m_name);
	std::unique_ptr<FEFaceList> ps(pfl);

	XMLElement ef;
	int n = 1, nn[9];

	FEFaceList& s = *pfl;
	int NF = s.Size();
	FEFaceList::Iterator pf = s.First();

	int nfn;
	for (int j = 0; j < NF; ++j, ++n, ++pf)
	{
		if (pf->m_pi == 0) throw InvalidItemListBuilder(l.m_name);
		FEFace& face = *(pf->m_pi);
		FECoreMesh* pm = pf->m_pm;
		nfn = face.Nodes();
		for (int k = 0; k < nfn; ++k) nn[k] = pm->Node(face.n[k]).m_nid;
		switch (nfn)
		{
		case 3: ef.name("tri3"); break;
		case 4: ef.name("quad4"); break;
		case 6: ef.name("tri6"); break;
		case 7: ef.name("tri7"); break;
		case 8: ef.name("quad8"); break;
		case 9: ef.name("quad9"); break;
		case 10: ef.name("tri10"); break;
		default:
			assert(false);
		}
		ef.add_attribute("id", n);
		ef.value(nn, nfn);
		m_xml.add_leaf(ef);
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteElementList(FEElemList& el)
{
	int NE = el.Size();
	FEElemList::Iterator pe = el.First();
	for (int i = 0; i < NE; ++i, ++pe)
	{
		FEElement_& el = *(pe->m_pi);
		XMLElement e("e");
		e.add_attribute("id", el.m_nid);
		m_xml.add_empty(e);
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteOutputSection()
{
	CPlotDataSettings& plt = m_prj.GetPlotDataSettings();
	int N = plt.PlotVariables();
	if (N > 0)
	{
		XMLElement p;
		p.name("plotfile");
		p.add_attribute("type", "febio");

		// count the nr of active plot variables
		int na = 0;
		for (int i = 0; i < N; ++i) if (plt.PlotVariable(i).isActive()) na++;

		if (na > 0)
		{
			m_xml.add_branch(p);
			{
				for (int i = 0; i < N; ++i)
				{
					FEPlotVariable& v = plt.PlotVariable(i);
					if (v.isShown() && v.isActive())
					{
						if (v.Domains() == 0)
						{
							XMLElement e;
							e.name("var");
							e.add_attribute("type", v.name());
							m_xml.add_empty(e);
						}
						else
						{
							if (v.domainType() == DOMAIN_SURFACE)
							{
								for (int n = 0; n < v.Domains(); ++n)
								{
									FEItemListBuilder* pl = v.GetDomain(n);
									if (pl)
									{
										XMLElement e;
										e.name("var");
										e.add_attribute("type", v.name());
										e.add_attribute("surface", pl->GetName());
										m_xml.add_empty(e);
									}
								}
							}
							else
							{
								assert(false);
							}
						}
					}
				}

				if (m_compress) m_xml.add_leaf("compression", 1);
			}
			m_xml.close_branch();
		}
		else m_xml.add_empty(p);
	}

	FSModel& fem = m_prj.GetFSModel();
	GModel& mdl = fem.GetModel();
	CLogDataSettings& log = m_prj.GetLogDataSettings();
	N = log.LogDataSize();
	if (N > 0)
	{
		m_xml.add_branch("logfile");
		{
			for (int i = 0; i < N; ++i)
			{
				FELogData& d = log.LogData(i);
				switch (d.type)
				{
				case FELogData::LD_NODE:
				{
					XMLElement e;
					e.name("node_data");
					e.add_attribute("data", d.sdata);

					if (d.fileName.empty() == false)
					{
						e.add_attribute("file", d.fileName);
					}

					FEItemListBuilder* pg = mdl.FindNamedSelection(d.groupID);
					if (pg)
					{
						e.add_attribute("node_set", pg->GetName());
					}
					m_xml.add_empty(e);
				}
				break;
				case FELogData::LD_ELEM:
				{
					XMLElement e;
					e.name("element_data");
					e.add_attribute("data", d.sdata);

					if (d.fileName.empty() == false)
					{
						e.add_attribute("file", d.fileName);
					}

					FEItemListBuilder* pg = mdl.FindNamedSelection(d.groupID);
					if (pg)
					{
						e.add_attribute("elem_set", pg->GetName());
					}
					m_xml.add_empty(e);
				}
				break;
				case FELogData::LD_RIGID:
				{
					XMLElement e;
					e.name("rigid_body_data");
					e.add_attribute("data", d.sdata);

					if (d.fileName.empty() == false)
					{
						e.add_attribute("file", d.fileName);
					}

					GMaterial* pm = fem.GetMaterialFromID(d.matID);
					if (pm)
					{
						e.value(pm->m_ntag);
						m_xml.add_leaf(e);
					}
					else m_xml.add_empty(e);
				}
				break;
				case FELogData::LD_CNCTR:
				{
					XMLElement e;
					e.name("rigid_connector_data");
					e.add_attribute("data", d.sdata);

					if (d.fileName.empty() == false)
					{
						e.add_attribute("file", d.fileName);
					}

					FSRigidConnector* rc = fem.GetRigidConnectorFromID(d.rcID);
					if (rc)
					{
						e.value(d.rcID);
						m_xml.add_leaf(e);
					}
					else m_xml.add_empty(e);
				}
				break;
				}
			}
		}
		m_xml.close_branch();
	}
}

//-----------------------------------------------------------------------------

void FEBioExport4::WriteStepSection()
{
	// we've already written the initial step
	// so now we simply output all the analysis steps
	for (int i = 1; i < m_pfem->Steps(); ++i)
	{
		FSStep& step = *m_pfem->GetStep(i);

		if (m_writeNotes) WriteNote(&step);

		XMLElement e;
		e.name("step");
		e.add_attribute("id", i);
		if (step.GetName().empty() == false) e.add_attribute("name", step.GetName().c_str());

		m_xml.add_branch(e);
		{
			// output control section
			m_xml.add_branch("Control");
			{
				WriteControlSection(step);
			}
			m_xml.close_branch(); // Control

			// output boundary section
			int nbc = step.BCs() + step.Interfaces();
			if (nbc > 0)
			{
				m_xml.add_branch("Boundary");
				{
					WriteBoundarySection(step);
				}
				m_xml.close_branch(); // Boundary
			}

			int nrc = step.RigidConstraints();
			if (nrc > 0)
			{
				m_xml.add_branch("Rigid");
				{
					WriteRigidSection(step);
				}
				m_xml.close_branch();

			}

			// output loads section
			int nlc = step.Loads();
			if (nlc > 0)
			{
				m_xml.add_branch("Loads");
				{
					WriteLoadsSection(step);
				}
				m_xml.close_branch(); // Loads
			}

			// output contact section
			int nci = step.Interfaces();
			if (nci)
			{
				m_xml.add_branch("Contact");
				{
					WriteContactSection(step);
				}
				m_xml.close_branch();
			}

			// output constraint section
			int nnlc = step.RigidConstraints() + CountConstraints<FSModelConstraint>(*m_pfem) + CountInterfaces<FSRigidJoint>(*m_pfem);
			if (nnlc > 0)
			{
				m_xml.add_branch("Constraints");
				{
					WriteConstraintSection(step);
				}
				m_xml.close_branch(); // Constraints
			}
		}
		m_xml.close_branch(); // step
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteRigidConstraints(FSStep& s)
{
	for (int i = 0; i < s.RigidConstraints(); ++i)
	{
		FSRigidConstraint* prc = s.RigidConstraint(i);
		if (prc && prc->IsActive())
		{
			if (m_writeNotes) WriteNote(prc);

			XMLElement el;
			el.name("rigid_constraint");
			el.add_attribute("name", prc->GetName());
			el.add_attribute("type", prc->GetTypeString());
			m_xml.add_branch(el);
			{
				WriteParamList(*prc);
			}
			m_xml.close_branch();
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteRigidLoads(FSStep& s)
{
	for (int i = 0; i < s.RigidLoads(); ++i)
	{
		FSRigidLoad* prl = s.RigidLoad(i);
		if (prl && prl->IsActive())
		{
			if (m_writeNotes) WriteNote(prl);

			XMLElement el;
			el.name("rigid_load");
			el.add_attribute("name", prl->GetName());
			el.add_attribute("type", prl->GetTypeString());
			m_xml.add_branch(el);
			{
				WriteParamList(*prl);
			}
			m_xml.close_branch();
		}
	}
}

//-----------------------------------------------------------------------------
// write rigid connectors
//
void FEBioExport4::WriteConnectors(FSStep& s)
{
	for (int i = 0; i < s.RigidConnectors(); ++i)
	{
		// rigid connectors
		FSRigidConnector* prc = s.RigidConnector(i);
		if (prc && prc->IsActive())
		{
			if (m_writeNotes) WriteNote(prc);

			XMLElement el("rigid_connector");
			el.add_attribute("name", prc->GetName());
			el.add_attribute("type", prc->GetTypeString());

			GMaterial* pgA = m_pfem->GetMaterialFromID(prc->GetRigidBody1());
			if (pgA == 0) throw MissingRigidBody(prc->GetName().c_str());
			if (pgA->GetMaterialProperties()->IsRigid() == false) throw InvalidMaterialReference();

			GMaterial* pgB = m_pfem->GetMaterialFromID(prc->GetRigidBody2());
			if (pgB == 0) throw MissingRigidBody(prc->GetName().c_str());
			if (pgB->GetMaterialProperties()->IsRigid() == false) throw InvalidMaterialReference();

			m_xml.add_branch(el);
			{
				int na = pgA->m_ntag;
				int nb = pgB->m_ntag;
				m_xml.add_leaf("body_a", na);
				m_xml.add_leaf("body_b", nb);

				WriteParamList(*prc);
			}
			m_xml.close_branch();
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteConstraintSection(FSStep& s)
{
	// some contact definitions are actually stored in the constraint section
	WriteConstraints(s);
}
