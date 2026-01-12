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
#include <FEMLib/FEInitialCondition.h>
#include <FEMLib/FESurfaceLoad.h>
#include <FEMLib/FEBodyLoad.h>
#include <FEMLib/FEModelConstraint.h>
#include <FEMLib/FERigidLoad.h>
#include <GeomLib/GObject.h>
#include <GeomLib/GGroup.h>
#include <MeshLib/FSNodeData.h>
#include <MeshLib/FSSurfaceData.h>
#include <MeshLib/FSElementData.h>
#include <GeomLib/GModel.h>
#include <FEMLib/FSProject.h>
#include <FEBioLink/FEBioModule.h>
#include <memory>
#include <sstream>
#include <FECore/FETransform.h>
#include <GeomLib/GPartSection.h>
#include <FEMLib/FEElementFormulation.h>

using namespace std;

//-----------------------------------------------------------------------------
// defined in FEFEBioExport25.cpp
FSNodeList* BuildNodeList(GFace* pf);
FSNodeList* BuildNodeList(GPart* pg);
FSNodeList* BuildNodeList(GNode* pn);
FSNodeList* BuildNodeList(GEdge* pe);
FSFaceList* BuildFaceList(GFace* face);
const char* ElementTypeString(int ntype);

std::vector<FEBioExport4::Domain*> FEBioExport4::Part::GetDomainsFromGPart(GPart* pg)
{
	std::vector<FEBioExport4::Domain*> domainList;
	for (Domain* dom : m_Dom)
	{
		if (dom->m_pg == pg) domainList.push_back(dom);
	}
	return domainList;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FEBioExport4::FEBioExport4(FSProject& prj) : FEBioExport(prj)
{
	m_writeNotes = true;
	m_exportEnumStrings = true;
	m_writeControlSection = true;
	m_allowMixedParts = false;
	m_prg = nullptr;
}

FEBioExport4::~FEBioExport4()
{
	Clear();
}

void FEBioExport4::SetProgressTracker(ProgressTracker* prg)
{
	m_prg = prg;
}

void FEBioExport4::setProgress(double v)
{
	if (m_prg)
	{
		m_prg->pct = v;
		if (m_prg->cancel) throw CancelExport();
	}
}

void FEBioExport4::setProgress(double v, const char* sztask)
{
	if (m_prg)
	{
		m_prg->pct = v;
		m_prg->sztask = sztask;

		if (m_prg->cancel) throw CancelExport();
	}
}

void FEBioExport4::setProgressTask(const char* sztask)
{
	if (m_prg)
	{
		m_prg->sztask = sztask;
		if (m_prg->cancel) throw CancelExport();
	}
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
const char* FEBioExport4::GetSurfaceName(FSItemListBuilder* pl, bool allowPartLists)
{
	// find the surface
	int N = (int)m_pSurf.size();
	for (int i = 0; i < N; ++i)
		if (m_pSurf[i].m_list == pl) return m_pSurf[i].m_name.c_str();

	// see if it's a part list
	if (allowPartLists)
	{
		N = (int)m_pPSet.size();
		for (int i = 0; i < N; ++i)
		{
			NamedItemList& it = m_pPSet[i];
			if (it.m_list == pl)
			{
				it.m_extName = "@part_list:" + it.m_name;
				return it.m_extName.c_str();
			}
		}
	}

	assert(false);
	return 0;
}

//----------------------------------------------------------------------------
string FEBioExport4::GetElementSetName(FSItemListBuilder* pl)
{
	int N = (int)m_pESet.size();
	for (int i = 0; i < N; ++i)
		if (m_pESet[i].m_list == pl) return m_pESet[i].m_name.c_str();

	if (dynamic_cast<GPartList*>(pl))
	{
		if (pl->size() == 1)
		{
			std::vector<int> items = pl->CopyItems();
			int partId = *(items.begin());
			GPart* pg = m_pfem->GetModel().FindPart(partId); assert(pg);
			if (pg) return pg->GetName();
		}
		else
		{
			string name = "@part_list:" + pl->GetName();
			return name;
		}
	}
	else if (dynamic_cast<FSPartSet*>(pl))
	{
		FSPartSet* ps = dynamic_cast<FSPartSet*>(pl);
		if (pl->size() == 1)
		{
			std::vector<int> items = pl->CopyItems();
			GPart* pg = ps->GetPart(0); assert(pg);
			if (pg) return pg->GetName();
		}
		else
		{
			string name = "@part_list:" + pl->GetName();
			return name;
		}
	}

	assert(false);
	return "";
}

//----------------------------------------------------------------------------
string FEBioExport4::GetNodeSetName(FSItemListBuilder* pl)
{
	// search the nodesets first
	int N = (int)m_pNSet.size();
	for (int i = 0; i < N; ++i)
		if (m_pNSet[i].m_list == pl) return m_pNSet[i].m_name.c_str();

	// search the edgesets
	N = (int)m_pEdge.size();
	for (int i = 0; i < N; ++i)
		if (m_pEdge[i].m_list == pl)
		{
			string edgeName = m_pEdge[i].m_name;
			return string("@edge:") + edgeName;
		}

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

	// search the part lists
	N = (int)m_pPSet.size();
	for (int i = 0; i < N; ++i)
	{
		FSItemListBuilder* pli = m_pPSet[i].m_list;
		if (pli == pl)
		{
			string setName = m_pPSet[i].m_name;
			return string("@part_list:") + setName;
		}
	}

	assert(false);
	return "";
}

//-----------------------------------------------------------------------------
void FEBioExport4::AddNodeSet(const std::string& name, FSItemListBuilder* pl)
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
void FEBioExport4::AddEdgeSet(const std::string& name, FSItemListBuilder* pl)
{
	// see if the node set is already defined
	for (int i = 0; i < m_pEdge.size(); ++i)
	{
		if (m_pEdge[i].m_name == name)
		{
			// add it, but mark it as duplicate
//			assert(false);
			m_pEdge.push_back(NamedItemList(name, pl, true));
			return;
		}
	}

	m_pEdge.push_back(NamedItemList(name, pl));
}

const char* FEBioExport4::GetEdgeSetName(FSItemListBuilder* pl)
{
	// search the nodesets first
	int N = (int)m_pEdge.size();
	for (int i = 0; i < N; ++i)
		if (m_pEdge[i].m_list == pl) return m_pEdge[i].m_name.c_str();

	assert(false);
	return "";
}

//-----------------------------------------------------------------------------
void FEBioExport4::AddSurface(const std::string& name, FSItemListBuilder* pl)
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
void FEBioExport4::AddElemSet(const std::string& name, FSItemListBuilder* pl)
{
	assert(pl);
	if (pl == nullptr) return;

	// make sure we don't have it yet.
	for (int i = 0; i < m_pESet.size(); ++i)
	{
		NamedItemList& l = m_pESet[i];
		if ((l.m_list == pl) && (name == l.m_name)) return;
	}
	
	m_pESet.push_back(NamedItemList(string(name), pl));
}

//-----------------------------------------------------------------------------
void FEBioExport4::AddPartList(const std::string& name, FSItemListBuilder* pl)
{
	// make sure this has not been added 
	for (int i = 0; i < m_pPSet.size(); ++i)
	{
		NamedItemList& partList = m_pPSet[i];
		if ((partList.m_list == pl) && (partList.m_name == name)) return;
	}
	m_pPSet.push_back(NamedItemList(string(name), pl));
}

//-----------------------------------------------------------------------------
bool FEBioExport4::PrepareExport(FSProject& prj)
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

	// see if we have any shells with non-zero thickness
	if (model.ShellElements() > 0)
	{
		for (int i = 0; i < model.Objects(); ++i)
		{
			GObject* po = model.Object(i);
			if (po && po->GetFEMesh())
			{
				FSMesh* pm = po->GetFEMesh();
				for (int j = 0; j < pm->Elements(); ++j)
				{
					FSElement& el = pm->Element(j);
					if (el.IsShell())
					{
						int n = el.Nodes();
						for (int k = 0; k < n; ++k)
						{
							if (el.m_h[k] != 0.0)
							{
								m_bdata = true;
								break;
							}
						}
						if (m_bdata) break;
					}
				}
				if (m_bdata) break;
			}
		}
	}
	for (int i = 0; i < fem.Materials(); ++i)
	{
		// get the material properties
		GMaterial* gmat = fem.GetMaterial(i);
		FSMaterial* pmat = gmat->GetMaterialProperties();

		// see if we should write fibers
		// This is only done if the material specifies the "user" fiber property
		bool writeFibers = false;

		FSProperty* fiberProp = (pmat ? pmat->FindProperty("fiber") : nullptr);
		if (fiberProp && fiberProp->Size() == 1)
		{
			FSCoreBase* fib = fiberProp->GetComponent(0);
			if (fib && (strcmp(fib->GetTypeString(), "user") == 0))
			{
				m_bdata = true;
			}
		}
	}
	for (int i = 0; i < model.Objects(); ++i)
	{
		FSMesh* pm = model.Object(i)->GetFEMesh();
		for (int j = 0; j < pm->Elements(); ++j)
		{
			FSElement_& e = pm->ElementRef(j);
			if (e.m_Qactive) {
				m_bdata = true;
				break;
			}
		}
		if (pm->MeshDataFields() > 0) m_bdata = true;
	}

	// See if we have data maps
	if (fem.MeshDataGenerators() > 0) m_bdata = true;

	return true;
}

//-----------------------------------------------------------------------------
void FEBioExport4::BuildItemLists(FSProject& prj)
{
	FSModel& fem = prj.GetFSModel();
	GModel& model = fem.GetModel();

	// add the user-defined nodesets
	int nsets = model.NodeLists();
	for (int i = 0; i < nsets; ++i)
	{
		GNodeList* pnl = model.NodeList(i);
		AddNodeSet(pnl->GetName(), pnl);
	}

	// add the user-defined edges
	int esets = model.EdgeLists();
	for (int i = 0; i < esets; ++i)
	{
		GEdgeList* pel = model.EdgeList(i);
		AddEdgeSet(pel->GetName(), pel);
	}

	// add the user-defined surfaces
	int faces = model.FaceLists();
	for (int i = 0; i < faces; ++i)
	{
		GFaceList* pfl = model.FaceList(i);
		AddSurface(pfl->GetName(), pfl);
	}

	// add the user-defined part lists
	int parts = model.PartLists();
	for (int i = 0; i < parts; ++i)
	{
		GPartList* pl = model.PartList(i);
		AddPartList(pl->GetName(), pl);
	}

	// add the user-defined mesh selections
	int nobj = model.Objects();
	for (int i = 0; i < nobj; ++i)
	{
		GObject* po = model.Object(i);
		FSMesh* pm = po->GetFEMesh();
		if (pm)
		{
			int nsets = po->FENodeSets();
			for (int j = 0; j < nsets; ++j)
			{
				FSNodeSet* ps = po->GetFENodeSet(j);
				AddNodeSet(ps->GetName(), ps);
			}

			int esets = po->FEEdgeSets();
			for (int j = 0; j < esets; ++j)
			{
				FSEdgeSet* ps = po->GetFEEdgeSet(j);
				AddEdgeSet(ps->GetName(), ps);
			}

			int nsurf = po->FESurfaces();
			for (int j = 0; j < nsurf; ++j)
			{
				FSSurface* ps = po->GetFESurface(j);
				AddSurface(ps->GetName(), ps);
			}

			int neset = po->FEElemSets();
			for (int j = 0; j < neset; ++j)
			{
				FSElemSet* pg = po->GetFEElemSet(j);
				AddElemSet(pg->GetName(), pg);
			}

			int npset = po->FEPartSets();
			for (int j = 0; j < npset; ++j)
			{
				FSPartSet* pg = po->GetFEPartSet(j);
				AddPartList(pg->GetName(), pg);
			}
		}
	}

	// export item lists for mesh data
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		FSMesh* mesh = po->GetFEMesh();
		if (mesh)
		{
			int ND = mesh->MeshDataFields();
			for (int j = 0; j < ND; ++j)
			{
				FSMeshData* data = mesh->GetMeshDataField(j);

				switch (data->GetDataClass())
				{
				case NODE_DATA:
				{
					FSNodeData* map = dynamic_cast<FSNodeData*>(data); assert(map);
					FSItemListBuilder* pg = map->GetItemList();
					if (pg)
					{
						string name = pg->GetName();
						if (name.empty()) name = data->GetName();
						AddNodeSet(name, pg);
					}
				}
				break;
				case ELEM_DATA:
				{
					FSElementData* map = dynamic_cast<FSElementData*>(data); assert(map);
					FSElemSet* pg = map->GetElementSet();

					if (pg)
					{
						string name = pg->GetName();
						if (name.empty()) name = data->GetName();

						// It is possible that a FSElemSet has the same name as the domain
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
				case PART_DATA:
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
bool FEBioExport4::Write(const char* szfile)
{
	// get the project and model
	FSModel& fem = m_prj.GetFSModel();
	GModel& mdl = fem.GetModel();
	m_pfem = &fem;

	setProgress(0.0, nullptr);

	// prepare for export
	try
	{
		setProgress(0.0, "Preparing to export");
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
		bool bsingle_step = false;
		if (m_nsteps == 2)
		{
			FSStep* pstep = fem.GetStep(1);
			if (pstep->StepComponents() == 0) bsingle_step = true;
		}

		m_discreteSets.clear();

		// open the file
		if (!m_xml.open(szfile)) return errf("Failed opening file %s", szfile);

		if (m_writeNotes)
		{
			WriteNote(&mdl);
			if (bsingle_step)
			{
				FSStep& step = *fem.GetStep(1);
				WriteNote(&step);
			}
		}

		XMLElement el;

		// output root element
		el.name("febio_spec");
		el.add_attribute("version", "4.0");

		setProgress(0.0, "Writing file ...");

		m_xml.add_branch(el);
		{
			// write the module section
			if (WriteSection(FEBIO_MODULE)) WriteModuleSection(m_prj);

			// write Control section
			if (m_writeControlSection && (m_nsteps == 2) && bsingle_step)
			{
				if (WriteSection(FEBIO_CONTROL))
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
			if ((nvar > 0) && WriteSection(FEBIO_GLOBAL))
			{
				m_xml.add_branch("Globals");
				{
					WriteGlobalsSection();
				}
				m_xml.close_branch();
			}

			// output material section
			if ((fem.Materials() > 0) && (WriteSection(FEBIO_MATERIAL)))
			{
				m_xml.add_branch("Material");
				{
					WriteMaterialSection();
				}
				m_xml.close_branch(); // Material
			}

			// output Mesh section
			if ((fem.GetModel().Objects() > 0) && (WriteSection(FEBIO_GEOMETRY)))
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
			if (m_bdata && WriteSection(FEBIO_MESHDATA))
			{
				m_xml.add_branch("MeshData");
				{
					WriteMeshDataSection();
				}
				m_xml.close_branch(); // MeshData
			}

			// output mesh adaptor section
			int nma = pstep->MeshAdaptors();
			if ((nma > 0) && WriteSection(FEBIO_MESHADAPTOR))
			{
				m_xml.add_branch("MeshAdaptor");
				{
					WriteMeshAdaptorSection(*pstep);
				}
				m_xml.close_branch(); // MeshData
			}

			// output initial section
			int nic = pstep->ICs();
			if ((nic > 0) && (WriteSection(FEBIO_INITIAL)))
			{
				m_xml.add_branch("Initial");
				{
					WriteInitialSection(*pstep);
				}
				m_xml.close_branch(); // Initial
			}

			// output boundary section
			int nbc = pstep->BCs();
			if ((nbc > 0) && (WriteSection(FEBIO_BOUNDARY)))
			{
				m_xml.add_branch("Boundary");
				{
					WriteBoundarySection(*pstep);
				}
				m_xml.close_branch(); // Boundary
			}

			int nrc = pstep->RigidBCs() + pstep->RigidICs() + pstep->RigidConnectors() + pstep->RigidLoads();
			if ((nrc > 0) && (WriteSection(FEBIO_BOUNDARY)))
			{
				m_xml.add_branch("Rigid");
				{
					WriteRigidSection(*pstep);
				}
				m_xml.close_branch(); // Rigid
			}

			// output loads section
			int nlc = pstep->Loads();
			if ((nlc > 0) && (WriteSection(FEBIO_LOADS)))
			{
				m_xml.add_branch("Loads");
				{
					WriteLoadsSection(*pstep);
				}
				m_xml.close_branch(); // Boundary
			}

			// output constraints section
			int nnlc = CountConstraints<FSModelConstraint>(fem);
			if ((nnlc > 0) && (WriteSection(FEBIO_CONSTRAINTS)))
			{
				m_xml.add_branch("Constraints");
				{
					WriteConstraintSection(*pstep);
				}
				m_xml.close_branch();
			}

			// output contact
			int nci = pstep->Interfaces();
			if ((nci > 0) && (WriteSection(FEBIO_CONTACT)))
			{
				m_xml.add_branch("Contact");
				{
					WriteContactSection(*pstep);
				}
				m_xml.close_branch(); // Contact
			}

			// output discrete elements (the obsolete spring-tied interface generates springs as well)
			int nrb = fem.GetModel().DiscreteObjects();
			if ((nrb > 0) && (WriteSection(FEBIO_DISCRETE)))
			{
				m_xml.add_branch("Discrete");
				{
					WriteDiscreteSection(*pstep);
				}
				m_xml.close_branch(); // Discrete
			}

			// step data
			if (WriteSection(FEBIO_STEPS) && (m_nsteps > 1))
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
			if ((fem.LoadControllers() > 0) && (WriteSection(FEBIO_LOADDATA)))
			{
				m_xml.add_branch("LoadData");
				{
					WriteLoadDataSection();
				}
				m_xml.close_branch(); // LoadData
			}

			// Output data
			if (WriteSection(FEBIO_OUTPUT))
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
	catch (CancelExport)
	{
		return errf("Export was cancelled.");
	}
	catch (std::runtime_error e)
	{
		return errf(e.what());
	}
	catch (...)
	{
		return errf("An unknown exception has occurred.");
	}

	// close the file
	m_xml.close();

	return true;
}

//-----------------------------------------------------------------------------
// Write the MODULE section
void FEBioExport4::WriteModuleSection(FSProject& prj)
{
	int moduleId = prj.GetModule();
	const char* szmodName = FEBio::GetModuleName(moduleId);
	XMLElement t;
	t.name("Module");
	t.add_attribute("type", szmodName);

	if (prj.GetUnits() == 0)
		m_xml.add_empty(t);
	else
	{
		m_xml.add_branch(t);
		switch (prj.GetUnits())
		{
		case 2: m_xml.add_leaf("units", "SI"     ); break;
		case 3: m_xml.add_leaf("units", "mm-N-s" ); break;
		case 4: m_xml.add_leaf("units", "mm-kg-s"); break;
		case 5: m_xml.add_leaf("units", "um-nN-s"); break;
		case 6: m_xml.add_leaf("units", "CGS"    ); break;
        case 7: m_xml.add_leaf("units", "mm-g-s" ); break;
        case 8: m_xml.add_leaf("units", "mm-mg-s"); break;
		}
		m_xml.close_branch();
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteControlSection(FSStep& step)
{
	WriteParamList(step);
	for (int i = 0; i < step.Properties(); ++i)
	{
		FSProperty& prop = step.GetProperty(i);
		FSModelComponent* pc = dynamic_cast<FSModelComponent*>(prop.GetComponent());
		if (pc)
		{
			XMLElement el(prop.GetName().c_str());
			WriteModelComponent(pc, el);
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
			WriteModelComponent(pmat, el);
		}
		else
		{
			errf("ERROR: Material %s does not have any properties.", name.c_str());
			m_xml.add_leaf(el);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteModelComponent(FSModelComponent* pm, XMLElement& el)
{
	// handle mesh selections differently
	if (dynamic_cast<FSMeshSelection*>(pm))
	{
		FSMeshSelection* sel = dynamic_cast<FSMeshSelection*>(pm);
		const char* szname = nullptr;
		switch (sel->GetSuperClassID())
		{
		case FEEDGE_ID   : szname = GetEdgeSetName(sel->GetItemList()); break;
		case FESURFACE_ID: szname = GetSurfaceName(sel->GetItemList()); break;
		default:
			assert(false);
		}
		if (szname == nullptr) throw FEBioExportError();
		el.value(szname);
		m_xml.add_leaf(el);
		return;
	}

	// get the type string    
	const char* sztype = pm->GetTypeString(); assert(sztype);
	if (sztype && sztype[0])
	{
		// only add the type attribute if the tag name is different from the type string
		// there is an unfortunate coincidence that the nodal_load has the same type string
		// as tag name. We need to make an exception for this. 
		if ((strcmp(sztype, el.name()) != 0) || (strcmp(sztype, "nodal_load") == 0))
			el.add_attribute("type", sztype);
	}

	// see if there are any attribute parameters
	FSModel& fem = m_prj.GetFSModel();
	for (int i = 0; i < pm->Parameters(); ++i)
	{
		Param& p = pm->GetParam(i);
		if (p.GetFlags() & FS_PARAM_ATTRIBUTE)
		{
			switch (p.GetParamType())
			{
			case Param_CHOICE:
			case Param_INT:
			{
				if (p.GetEnumNames())
				{
//					int v = fem.GetEnumValue(p);
					const char* v = fem.GetEnumKey(p, false);
					if (v == nullptr) {
						errf("Invalid key for parameter %s", p.GetShortName());
						v = "(invalid)";
					}
					el.add_attribute(p.GetShortName(), v);
				}
				else el.add_attribute(p.GetShortName(), p.GetIntValue());
			}
			break;
			case Param_STRING:
			{
				std::string s = p.GetStringValue();
				el.add_attribute(p.GetShortName(), s);
			}
			break;
			default:
				assert(false);
			}
		}
	}

	if ((pm->Parameters() == 0) && (pm->Properties() == 0))
	{
		m_xml.add_empty(el);
		return;
	}

	if (pm->Properties() == 0)
	{
		// see if we can use the shorthand notation
		// In order to use this, there can only be one non-attribute parameter with
		// the same name as the type string. 
		Param* pp = nullptr;
		int nonattparam = 0;
		for (int i = 0; i < pm->Parameters(); ++i)
		{
			Param& p = pm->GetParam(i);
			if ((p.GetFlags() & FS_PARAM_ATTRIBUTE) == 0)
			{
				if (strcmp(p.GetShortName(), sztype) == 0)
				{
					assert(pp == nullptr);
					pp = &p;
				}
				nonattparam++;
			}
		}
		if (nonattparam != 1) pp = nullptr;

		// If there is only one parameter, and it has the same name
		// as the type, then write it all on a single line.
		if (pp)
		{
			bool useShortNotation = true;
			switch (pp->GetParamType())
			{
//			case Param_INT              : el.value(pp->GetIntValue   ()); break;
			case Param_FLOAT            : el.value(pp->GetFloatValue ()); break;
//			case Param_BOOL             : el.value(pp->GetBoolValue  ()); break;
//			case Param_VEC3D            : el.value(pp->GetVec3dValue ()); break;
//			case Param_VEC2I            : el.value(pp->GetVec2iValue ()); break;
//			case Param_MAT3D            : el.value(pp->GetMat3dValue ()); break;
//			case Param_MAT3DS           : el.value(pp->GetMat3dsValue()); break;
//			case Param_STD_VECTOR_INT   : el.value(pp->GetVectorIntValue()); break;
//			case Param_STD_VECTOR_DOUBLE: el.value(pp->GetVectorDoubleValue()); break;
//			case Param_ARRAY_INT        : el.value(pp->GetArrayIntValue()); break;
//			case Param_ARRAY_DOUBLE     : el.value(pp->GetArrayDoubleValue()); break;
//			case Param_MATH             : el.value(pp->GetMathString()); break;
//			case Param_STRING           : el.value(pp->GetStringValue()); break;
			default:
				useShortNotation = false;
			}

			if (useShortNotation)
			{
				m_xml.add_leaf(el);
				return;
			}
		}
	}

	m_xml.add_branch(el);
	{
		// write the parameters (if any)
		if (pm->Parameters())
		{
			WriteParamList(*pm);
		}

		// write the properties
		int NC = (int)pm->Properties();
		for (int i = 0; i < NC; ++i)
		{
			FSProperty& mc = pm->GetProperty(i);
			for (int j = 0; j < mc.Size(); ++j)
			{
				FSModelComponent* pc = dynamic_cast<FSModelComponent*>(pm->GetProperty(i, j));
				if (pc)
				{
					XMLElement eli(mc.GetName().c_str());

					// write the name (this is only used by a few features, such as specifies in the reaction-diffusion module)
					std::string name = pc->GetName();
					if (!name.empty())
					{
						eli.add_attribute("name", name.c_str());
					}

					WriteModelComponent(pc, eli);
				}
			}
		}
	}
	m_xml.close_branch();
}

// As of FBS 2.8, parts can deactivated. For these inactive parts, we don't write the nodes, elements, 
// or any geometry that references the inactive parts.
void FEBioExport4::ProcessParts()
{
	FSModel& s = *m_pfem;
	GModel& model = s.GetModel();

	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		FSCoreMesh* pm = po->GetFEMesh();

		// tag all nodes which will be exported. (Nodes of inactive parts are not exported)
		// first we tag all elements
		for (int j = 0; j < pm->Elements(); ++j)
		{
			FSElement_& el = pm->ElementRef(j);
			el.SetExport(true);
			if (el.m_gid >= 0)
			{
				GPart* pg = po->Part(el.m_gid);
				if (!pg->IsActive()) el.SetExport(false);
			}
		}

		// assume all nodes will be exported
		for (int j = 0; j < pm->Nodes(); ++j)
		{
			pm->Node(j).SetExport(true);
		}

		// tag all nodes of inactive parts
		for (int j = 0; j < pm->Elements(); ++j)
		{
			FSElement_& el = pm->ElementRef(j);
			if (!el.CanExport())
			{
				int ne = el.Nodes();
				for (int k = 0; k < ne; ++k) pm->Node(el.m_node[k]).SetExport(false);
			}
		}

		// tag all nodes of active parts
		// The reason why we first tag inactive nodes and then active nodes is so that nodes 
		// that are not connected to any parts (e.g. free node attached with discrete element) will still be exported.
		for (int j = 0; j < pm->Elements(); ++j)
		{
			FSElement_& el = pm->ElementRef(j);
			if (el.CanExport())
			{
				int ne = el.Nodes();
				for (int k = 0; k < ne; ++k) pm->Node(el.m_node[k]).SetExport(true);
			}
		}

		// process surface facets
		// facets are only exported if all its nodes are exported
		for (int j = 0; j < pm->Faces(); ++j)
		{
			FSFace& face = pm->Face(j);
			int nf = face.Nodes();
			face.SetExport(true);
			for (int k=0; k<nf; ++k)
			{
				if (!pm->Node(face.n[k]).CanExport()) face.SetExport(false);
			}
		}

		// process edges
		// edges are only exported if all its nodes are exported
		for (int j = 0; j < pm->Edges(); ++j)
		{
			FSEdge& edge = pm->Edge(j);
			int ne = edge.Nodes();
			edge.SetExport(true);
			for (int k = 0; k < ne; ++k)
			{
				if (!pm->Node(edge.n[k]).CanExport()) edge.SetExport(false);
			}
		}
	}
}

void FEBioExport4::WriteMeshSection() 
{
	// process active parts
	ProcessParts();
	
	// export the nodes
	WriteGeometryNodes();

	// Write the elements
	WriteMeshElements();

	// write the node sets
	WriteGeometryNodeSets();

	// write the edges
	WriteGeometryEdges();

	// write named surfaces
	WriteGeometrySurfaces();

	// write named element sets
	WriteGeometryElementSets();

	// write named part lists
	WriteGeometryPartLists();

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

			GSolidSection* section = dynamic_cast<GSolidSection*>(dom->m_pg->GetSection());
			if (section && section->GetElementFormulation())
			{
				FESolidFormulation* solid = section->GetElementFormulation();
				WriteModelComponent(solid, el);
			}
			else m_xml.add_empty(el);
		}
		else if (dom->m_elemClass == ELEM_SHELL)
		{
			GPart* pg = dom->m_pg;
			el.name("ShellDomain");
			el.add_attribute("name", dom->m_name);
			el.add_attribute("mat", dom->m_matName);

			GShellSection* section = dynamic_cast<GShellSection*>(pg->GetSection());
			if (section)
			{
				FEShellFormulation* shell = section->GetElementFormulation();
				if (shell) WriteModelComponent(shell, el);
				else
				{
					m_xml.add_branch(el);
					double h = section->shellThickness();
					m_xml.add_leaf("shell_thickness", h);
					m_xml.close_branch();
				}
			}
			else m_xml.add_empty(el);
		}
		else if (dom->m_elemClass == ELEM_BEAM)
		{
			GPart* pg = dom->m_pg;
			el.name("BeamDomain");
			el.add_attribute("name", dom->m_name);
			el.add_attribute("mat", dom->m_matName);

			GBeamSection* section = dynamic_cast<GBeamSection*>(pg->GetSection());
			if (section && section->GetElementFormulation())
			{
				FEBeamFormulation* beam = section->GetElementFormulation();
				WriteModelComponent(beam, el);
			}
			else m_xml.add_empty(el);
		}
		else
		{
			assert(false);
		}
	}
}

bool FEBioExport4::WriteNodeSet(const string& name, FSNodeList* pl)
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
	if (nn == 0) return false;

	FSNodeList::Iterator pn = pl->First();
	vector<int> m;
	m.reserve(pl->Size());
	for (int n = 0; n < nn; ++n, pn++)
	{
		FSNode* pnode = pn->m_pi;
		if (pnode == nullptr) return false;
		if (pnode->CanExport()) m.push_back(pnode->m_nid);
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
		FSItemListBuilder* pil = m_pNSet[i].m_list;
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
			FSNodeSet* nodeSet = dynamic_cast<FSNodeSet*>(pil); assert(nodeSet);
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
			FSSurface* face = dynamic_cast<FSSurface*>(pil); assert(face);
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
			FSItemListBuilder* pil = m_pNSet[i].m_list;
			unique_ptr<FSNodeList> pl(pil->BuildNodeList());
			if (WriteNodeSet(m_pNSet[i].m_name.c_str(), pl.get()) == false)
			{
				throw InvalidItemListBuilder(pil);
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
void FEBioExport4::WriteGeometryEdges()
{
	int NC = (int)m_pEdge.size();
	for (int i = 0; i < NC; ++i)
	{
		XMLElement el("Edge");
		el.add_attribute("name", m_pEdge[i].m_name.c_str());
		m_xml.add_branch(el);
		{
			WriteEdgeSection(m_pEdge[i]);
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
		FSItemListBuilder* pl = m_pESet[i].m_list;
		unique_ptr<FSElemList> ps(pl->BuildElemList());
		XMLElement el("ElementSet");
		el.add_attribute("name", m_pESet[i].m_name.c_str());

		int NE = ps->Size();
		vector<int> elemIds;
		FSElemList::Iterator pe = ps->First();
		for (int i = 0; i < NE; ++i, ++pe)
		{
			if (pe->m_pi)
			{
				FSElement_& el = *(pe->m_pi);
				if (el.CanExport())
					elemIds.push_back(el.m_nid);
			}
		}

		m_xml.add_leaf(el, elemIds);
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteGeometryPartLists()
{
	FEBioExport4::Part& part = *m_Part[0];

	GModel& mdl = m_pfem->GetModel();
	int NP = (int)m_pPSet.size();
	for (int i = 0; i < NP; ++i)
	{
		NamedItemList& it = m_pPSet[i];
		XMLElement el("PartList");
		el.add_attribute("name", it.m_name.c_str());
		std::stringstream ss;
		FSItemListBuilder* pl = it.m_list;

		// we don't write part lists that have only one part
		if (dynamic_cast<GPartList*>(pl))
		{
			std::vector<int> partIDs = pl->CopyItems();
			bool bfirst = true;
			for (int id : partIDs)
			{
				GPart* pg = mdl.FindPart(id); assert(pg);
				if (pg && pg->IsActive())
				{
					// since the domain can be split on export, we need to find all part with pg 
					// as its parent GPart
					std::vector<FEBioExport4::Domain*> domainList = part.GetDomainsFromGPart(pg);

					for (auto domain : domainList)
					{
						if (bfirst == false) ss << ","; else bfirst = false;
						ss << domain->m_name;
					}
				}
			}
		}
		else if (dynamic_cast<FSPartSet*>(pl))
		{
			FSPartSet* ps = dynamic_cast<FSPartSet*>(pl);
			bool bfirst = true;
			for (int n = 0; n<ps->size(); ++n)
			{
				if (bfirst == false) ss << ","; else bfirst = false;
				GPart* pg = ps->GetPart(n); assert(pg);
				if (pg && pg->IsActive()) ss << pg->GetName();
			}
		}
		else { assert(false); }
		string s = ss.str();
		el.value(s);
		m_xml.add_leaf(el);
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteGeometrySurfacesNew()
{
	int NS = (int)m_pSurf.size();
	for (int i = 0; i < NS; ++i)
	{
		FSItemListBuilder* pl = m_pSurf[i].m_list;
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
				FSSurface* surf = dynamic_cast<FSSurface*>(pl); assert(surf);
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
		FSItemListBuilder* pl = m_pESet[i].m_list;
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
			case FE_ELEMSET:
			{
				FSElemSet* part = dynamic_cast<FSElemSet*>(pl); assert(part);
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
				FSItemListBuilder* pms = pi->GetSecondarySurface();
				if (pms == 0) throw InvalidItemListBuilder(pi);

				FSItemListBuilder* pss = pi->GetPrimarySurface();
				if (pss == 0) throw InvalidItemListBuilder(pi);

				XMLElement el("SurfacePair");
				el.add_attribute("name", pi->GetName().c_str());
				m_xml.add_branch(el);
				{
					const char* szprimary   = GetSurfaceName(pss, true);
					const char* szsecondary = GetSurfaceName(pms, true);
					if ((szprimary == nullptr) || (szsecondary == nullptr))
						throw InvalidItemListBuilder(pi);

					m_xml.add_leaf("primary", szprimary);
					m_xml.add_leaf("secondary", szsecondary);
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
		FSCoreMesh* pm = po->GetFEMesh();

		// we only export this object if it has any nodes that need to be exported
		bool exportNodes = false;
		for (int j = 0; j < pm->Nodes(); ++j)
		{
			if (pm->Node(j).CanExport())
			{
				exportNodes = true;
				break;
			}
		}

		if (exportNodes)
		{
			XMLElement tagNodes("Nodes");
			const string& name = po->GetName();
			if (name.empty() == false) tagNodes.add_attribute("name", name.c_str());

			m_xml.add_branch(tagNodes);
			{
				XMLElement el("node");
				int nid = el.add_attribute("id", 0);
				for (int j = 0; j < pm->Nodes(); ++j)
				{
					FSNode& node = pm->Node(j);
					if (node.CanExport())
					{
						el.set_attribute(nid, node.m_nid);
						if (node.m_nid > n) n = node.m_nid + 1;
						vec3d r = po->GetTransform().LocalToGlobal(node.r);
						el.value(r);
						m_xml.add_leaf(el, false);
					}
				}
			}
			m_xml.close_branch();
		}
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

	int parts = model.Parts();
	if (parts == 0) parts = 1;
	int count = 0;

	// loop over all objects
	for (int i = 0; i < model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		FSMesh* pm = po->GetFEMesh();

		// loop over all parts
		int NP = po->Parts();
		std::vector<int> elemCount(NP, 0);
		for (int i=0; i<pm->Elements(); ++i)
		{
			FSElement& el = pm->Element(i);
			elemCount[el.m_gid]++;
		}
		std::vector< std::vector<int> > partElements(NP);
		for (int p = 0; p < NP; ++p)
		{
			partElements[p].reserve(elemCount[p]);
		}
		for (int i = 0; i < pm->Elements(); ++i)
		{
			FSElement& el = pm->Element(i);
			partElements[el.m_gid].push_back(i);
		}

		for (int p = 0; p < NP; ++p, count++)
		{
			setProgress((double)count / (double) parts);

			// get the part
			GPart* pg = po->Part(p);

			// see if it's active
			bool exportPart = pg->IsActive();

			// write this part if it's active
			if (exportPart)
			{
				if (m_allowMixedParts)
					WriteMixedElementsPart(part, pg, partElements[p], false, false);
				else
					WriteGeometryPart(part, pg, partElements[p], false, false);
			}
		}
	}
}

int degenerate_element_type(int type1, int type2)
{
	if (type1 == type2) return type1;
	if ((type1 == FE_TRI3  ) && (type2 == FE_QUAD4 )) return FE_QUAD4;
	if ((type1 == FE_QUAD4 ) && (type2 == FE_TRI3  )) return FE_QUAD4;
	if ((type1 == FE_TET4  ) && (type2 == FE_PENTA6)) return FE_PENTA6;
	if ((type1 == FE_PENTA6) && (type2 == FE_TET4  )) return FE_PENTA6;
	if ((type1 == FE_HEX8  ) && (type2 == FE_TET4  )) return FE_HEX8;
	if ((type1 == FE_TET4  ) && (type2 == FE_HEX8  )) return FE_HEX8;
	if ((type1 == FE_PENTA6) && (type2 == FE_HEX8  )) return FE_HEX8;
	if ((type1 == FE_HEX8  ) && (type2 == FE_PENTA6)) return FE_HEX8;
	return -1;
}

int get_degenerate_nodes(int dstType, int srcType, int* n)
{
	if (dstType == FE_QUAD4)
	{
		if (srcType == FE_QUAD4) return 4;
		if (srcType == FE_TRI3)
		{
			n[3] = n[2];
			return 4;
		}
	}
	if (dstType == FE_PENTA6)
	{
		if (srcType == FE_PENTA6) return 6;
		if (srcType == FE_TET4)
		{
			n[5] = n[4] = n[3];
			return 6;
		}
	}
	if (dstType == FE_HEX8)
	{
		if (srcType == FE_HEX8) return 8;
		if (srcType == FE_PENTA6)
		{
			n[7] = n[6] = n[5];
			n[5] = n[4];
			n[4] = n[3];
			n[3] = n[2];
			return 8;
		}
		if (srcType == FE_TET4)
		{
			n[7] = n[6] = n[5] = n[4] = n[3];
			n[3] = n[2];
			return 8;
		}
	}
	assert(false);
	return -1;
}

void FEBioExport4::WriteMixedElementsPart(Part* part, GPart* pg, std::vector<int>& elemList, bool writeMats, bool useMatNames)
{
	FSModel& s = *m_pfem;
	GModel& model = s.GetModel();
	GObject* po = dynamic_cast<GObject*>(pg->Object());
	FSCoreMesh* pm = po->GetFEMesh();
	int pid = pg->GetLocalID();

	// get the material
	int nmat = 0;
	GMaterial* pmat = s.GetMaterialFromID(pg->GetMaterialID());
	if (pmat) nmat = pmat->m_ntag;

	// figure out which type this part is
	int elemType = -1;
	int elemClass = -1;
	int NE = (int)elemList.size();
	for (int i = 0; i < NE; ++i)
	{
		FSElement_& el = pm->ElementRef(elemList[i]); assert(el.m_gid == pid);

		if (elemType == -1) elemType = el.Type();
		else if (el.Type() != elemType)
		{
			elemType = degenerate_element_type(elemType, el.Type());
			if (elemType == -1)
			{
				string err = "Part \"" + pg->GetName() + "\" has invalid degenerate elements.";
				throw std::runtime_error(err);
			}
		}

		if (elemClass == -1) elemClass = el.Class();
		else if (elemClass != el.Class()) throw FEBioExportError();
	}

	const char* sztype = ElementTypeString(elemType);
	if (sztype == nullptr) throw FEBioExportError();
	XMLElement xe("Elements");
	xe.add_attribute("type", sztype);
	if ((nmat > 0) && writeMats)
	{
		if (useMatNames) xe.add_attribute("mat", pmat->GetName().c_str());
		else xe.add_attribute("mat", nmat);
	}
	string name = pg->GetName();
	xe.add_attribute("name", name);

	ElementSet es;
	es.m_mesh = pm;
	es.m_name = name;
	es.m_matID = pg->GetMaterialID();

	// add a domain
	if (part)
	{
		FEBioExport4::Domain* dom = new FEBioExport4::Domain;
		dom->m_name = name;
		dom->m_pg = pg;
		if (pmat) dom->m_matName = pmat->GetName().c_str();
		part->m_Dom.push_back(dom);

		dom->m_elemClass = elemClass;
		dom->m_elemType = elemType;
	}

	int lastElemID = 0;
	int ncount = 0;
	m_xml.add_branch(xe);
	{
		// loop over unprocessed elements
		int nn[FSElement::MAX_NODES];
		for (int i = 0; i < NE; ++i)
		{
			FSElement_& el = pm->ElementRef(elemList[i]);
			if (el.m_nid <= lastElemID) throw FEBioExportError();
			lastElemID = el.m_nid;

			int ntype = el.Type();

			XMLElement xel("elem");
			xel.add_attribute("id", el.m_nid);

			int ne = el.Nodes();
			for (int k = 0; k < ne; ++k) nn[k] = pm->Node(el.m_node[k]).m_nid;

			if (el.Type() != elemType)
			{
				ne = get_degenerate_nodes(elemType, el.Type(), nn);
				if (ne == -1) throw FEBioExportError();
			}
					
			xel.value(nn, ne);
			m_xml.add_leaf(xel, false);
			ncount++;
			es.m_elem.push_back(elemList[i]);
		}
	}
	m_xml.close_branch();

	m_ElSet.push_back(es);

	// make sure this part has elements
	if (ncount == 0)
	{
		string err = "Part \"" + pg->GetName() + "\" has no elements.";
		throw std::runtime_error(err);
	}

	// update total element counter
	m_ntotelem += ncount;
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteGeometryPart(Part* part, GPart* pg, std::vector<int>& elemList, bool writeMats, bool useMatNames)
{
	FSModel& s = *m_pfem;
	GModel& model = s.GetModel();
	GObject* po = dynamic_cast<GObject*>(pg->Object());
	FSCoreMesh* pm = po->GetFEMesh();
	int pid = pg->GetLocalID();

	// Parts must be split up by element type
	int NEP = (int)elemList.size(); // number of elements in part
	for (int i = 0; i < NEP; ++i)
	{
		FSElement_& el = pm->ElementRef(elemList[i]); assert(el.m_gid == pid);
		el.m_ntag = 1;
	}

	// make sure this part has elements
	if (NEP == 0)
	{
		string err = "Part \"" + pg->GetName() + "\" has no elements.";
		throw std::runtime_error(err);
	}

	// get the material
	int nmat = 0;
	GMaterial* pmat = s.GetMaterialFromID(pg->GetMaterialID());
	if (pmat) nmat = pmat->m_ntag;

	// loop over unprocessed elements
	int nset = 0;
	int ncount = 0;
	int nn[FSElement::MAX_NODES];
	char szname[128] = { 0 };
	for (int i = 0; ncount < NEP; ++i)
	{
		FSElement_& el = pm->ElementRef(elemList[i]);
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
				dom->m_elemType = ntype;
			}

			xe.add_attribute("name", szname);
			m_xml.add_branch(xe);
			{
				XMLElement xej("elem");
				int n1 = xej.add_attribute("id", (int)0);
				int lastElemID = 0;

				for (int j = i; j < NEP; ++j)
				{
					FSElement_& ej = pm->ElementRef(elemList[j]);
					if ((ej.m_ntag == 1) && (ej.Type() == ntype))
					{
						if (ej.m_nid <= lastElemID) throw FEBioExportError();
						lastElemID = ej.m_nid;

						xej.set_attribute(n1, ej.m_nid);
						int ne = ej.Nodes();
						assert(ne == el.Nodes());
						for (int k = 0; k < ne; ++k) nn[k] = pm->Node(ej.m_node[k]).m_nid;
						xej.value(nn, ne);
						m_xml.add_leaf(xej, false);
						ej.m_ntag = -1;	// mark as processed
						ncount++;

						es.m_elem.push_back(elemList[j]);
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
		GDiscreteObject* pdo = model.DiscreteObject(i);
		if (pdo->IsActive())
		{
			GLinearSpring* ps = dynamic_cast<GLinearSpring*>(pdo);
			if (ps)
			{
				m_discreteSets.push_back(ps);

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
			GGeneralSpring* pg = dynamic_cast<GGeneralSpring*>(pdo);
			if (pg)
			{
				m_discreteSets.push_back(pg);

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
			GDiscreteSpringSet* pds = dynamic_cast<GDiscreteSpringSet*>(pdo);
			if (pds && (pds->size()))
			{
				vector<pair<int, int>> springNodes;
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
						FSNode* fn0 = po0->GetFENode(pn0->GetLocalID());
						FSNode* fn1 = po1->GetFENode(pn1->GetLocalID());
						if ((fn0 && fn0->CanExport()) && (fn1 && fn1->CanExport()))
						{
							n[0] = fn0->m_nid;
							n[1] = fn1->m_nid;
							springNodes.push_back({ n[0], n[1] });
						}
					}
				}

				if (!springNodes.empty())
				{
					m_discreteSets.push_back(pds);

					XMLElement el("DiscreteSet");
					el.add_attribute("name", pds->GetName().c_str());
					m_xml.add_branch(el);
					{
						int N = springNodes.size();
						for (int n = 0; n < N; ++n)
						{
							int m[2] = { springNodes[n].first, springNodes[n].second };
							m_xml.add_leaf("delem", m, 2);
						}
					}
					m_xml.close_branch();
				}
			}
			GDeformableSpring* ds = dynamic_cast<GDeformableSpring*>(pdo);
			if (ds)
			{
				m_discreteSets.push_back(ds);

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
	int N = fem.MeshDataGenerators();
	for (int i = 0; i < N; ++i)
	{
		FSMeshDataGenerator* pdm = fem.GetMeshDataGenerator(i);

		switch (pdm->Type())
		{
		case FE_FEBIO_NODEDATA_GENERATOR: WriteNodeDataGenerator(dynamic_cast<FSNodeDataGenerator*>(pdm)); break;
		case FE_FEBIO_EDGEDATA_GENERATOR: WriteEdgeDataGenerator(dynamic_cast<FSEdgeDataGenerator*>(pdm)); break;
		case FE_FEBIO_FACEDATA_GENERATOR: WriteFaceDataGenerator(dynamic_cast<FSFaceDataGenerator*>(pdm)); break;
		case FE_FEBIO_ELEMDATA_GENERATOR: WriteElemDataGenerator(dynamic_cast<FSElemDataGenerator*>(pdm)); break;
		case FE_CONST_FACEDATA_GENERATOR: WriteFaceDataGenerator(dynamic_cast<FSFaceDataGenerator*>(pdm)); break;
		default:
			assert(false);
		}
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
void FEBioExport4::WriteNodeDataGenerator(FSNodeDataGenerator* map)
{
	XMLElement meshData("NodeData");
	meshData.add_attribute("name", map->GetName());
	meshData.add_attribute("node_set", GetNodeSetName(map->GetItemList()));
	WriteModelComponent(map, meshData);
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteEdgeDataGenerator(FSEdgeDataGenerator* map)
{
	XMLElement meshData("EdgeData");
	meshData.add_attribute("name", map->GetName());
	//	meshData.add_attribute("edge_set", GetEdgeSetName(map->GetItemList()));
	WriteModelComponent(map, meshData);
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteFaceDataGenerator(FSFaceDataGenerator* map)
{
	const char* szsurf = GetSurfaceName(map->GetItemList());
	if (szsurf == nullptr) throw InvalidItemListBuilder(map);
	XMLElement meshData("SurfaceData");
	meshData.add_attribute("name", map->GetName());
	meshData.add_attribute("surface", GetSurfaceName(map->GetItemList()));
	WriteModelComponent(map, meshData);
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteElemDataGenerator(FSElemDataGenerator* map)
{
	XMLElement meshData("ElementData");
	meshData.add_attribute("name", map->GetName());
	meshData.add_attribute("elem_set", GetElementSetName(map->GetItemList()));
	WriteModelComponent(map, meshData);
}
//-----------------------------------------------------------------------------
void FEBioExport4::WriteMeshDataShellThickness()
{
	FSModel& fem = *m_pfem;
	GModel& model = fem.GetModel();
	for (int i = 0; i < (int)m_ElSet.size(); ++i)
	{
		ElementSet& elset = m_ElSet[i];
		FSCoreMesh* pm = elset.m_mesh;

		// see if this mesh has shells with nonzero thickness
		bool bshell = false;
		double hConst = 0.0;
		bool constShellThickness = true;
		for (int k = 0; k < (int)elset.m_elem.size(); ++k)
		{
			FSElement_& el = pm->ElementRef(elset.m_elem[k]);
			if (el.IsShell() && el.CanExport()) 
			{ 
				int n = el.Nodes();
				for (int l = 0; l < n; ++l)
				{
					if (el.m_h[l] != 0.0)
					{
						if (bshell == false)
						{
							bshell = true;
							hConst = el.m_h[l];
						}
						else if (hConst != el.m_h[l])
						{
							constShellThickness = false;
						}
					}
				}
			}
		}

		// write shell thickness data
		if (bshell)
		{
			XMLElement tag("ElementData");
			tag.add_attribute("type", "shell thickness");
			tag.add_attribute("elem_set", elset.m_name.c_str());
			m_xml.add_branch(tag);
			{
				XMLElement el("e");
				int n1 = el.add_attribute("lid", 0);

				int nid = 1;
				for (int k = 0; k < (int)elset.m_elem.size(); ++k)
				{
					FSElement_& e = pm->ElementRef(elset.m_elem[k]);
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
		FSCoreMesh* pm = elSet.m_mesh;
		GObject* po = pm->GetGObject();
		const Transform& T = po->GetTransform();

		// see if we should write fibers
		// This is only done if the material specifies the "user" fiber property
		bool writeFibers = false;

		// get the material properties
		GMaterial* gmat = fem.GetMaterialFromID(elSet.m_matID);
		if (gmat)
		{
			FSMaterial* pmat = gmat->GetMaterialProperties();

			FSProperty* fiberProp = pmat->FindProperty("fiber");
			if (fiberProp && fiberProp->Size() == 1)
			{
				FSCoreBase* fib = fiberProp->GetComponent(0);
				if (fib && (strcmp(fib->GetTypeString(), "user") == 0))
				{
					writeFibers = true;
				}
			}
		}

		if (writeFibers)
		{
			int NE = (int)elSet.m_elem.size();
			XMLElement tag("ElementData");
			tag.add_attribute("type", "fiber");
			tag.add_attribute("elem_set", elSet.m_name.c_str());
			m_xml.add_branch(tag);
			{
				XMLElement el("e");
				int nid = el.add_attribute("lid", 0);
				for (int j = 0; j < NE; ++j)
				{
					FSElement_& e = pm->ElementRef(elSet.m_elem[j]);
					if (e.CanExport())
					{
						vec3d a = T.LocalToGlobalNormal(e.m_fiber);
						el.set_attribute(nid, j + 1);
						el.value(a);
						m_xml.add_leaf(el, false);
					}
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
		FSCoreMesh* pm = elSet.m_mesh;
		GObject* po = pm->GetGObject();
		const Transform& T = po->GetTransform();

		// make sure there is something to do
		bool bwrite = false;
		int NE = (int)elSet.m_elem.size();
		for (int j = 0; j < NE; ++j)
		{
			FSElement_& el = pm->ElementRef(elSet.m_elem[j]);
			if (el.m_Qactive && el.CanExport()) { bwrite = true; break; }
		}

		// okay, let's get to work
		if (bwrite)
		{
			int n = 0;
			XMLElement tag("ElementData");
			tag.add_attribute("type", "mat_axis");
			tag.add_attribute("elem_set", elSet.m_name.c_str());
			m_xml.add_branch(tag);
			{
				XMLElement el("e");
				int nid = el.add_attribute("lid", 0);

				for (int j = 0; j < NE; ++j)
				{
					FSElement_& e = pm->ElementRef(elSet.m_elem[j]);
					if (e.m_Qactive && e.CanExport())
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
		FSMesh* pm = po->GetFEMesh();
		int NE = pm->Elements();

		int ND = pm->MeshDataFields();
		for (int n = 0; n < ND; ++n)
		{
			FSElementData* meshData = dynamic_cast<FSElementData*>(pm->GetMeshDataField(n));
			if (meshData)
			{
				FSElementData& data = *meshData;
				const FSElemSet* pg = data.GetElementSet();
				if (pg == nullptr) throw InvalidItemListBuilder(meshData);

				string name = pg->GetName();
				if (name.empty()) name = data.GetName();

				XMLElement tag("ElementData");
				tag.add_attribute("name", data.GetName().c_str());
				tag.add_attribute("elem_set", name);

				switch (meshData->GetDataType())
				{
				case DATA_SCALAR: break;
				case DATA_VEC3 : tag.add_attribute("data_type", "vec3"); break;
				case DATA_MAT3 : tag.add_attribute("data_type", "mat3"); break;
				default:
					assert(false);
				}

				m_xml.add_branch(tag);
				{
					double d[FSElementData::MAX_ITEM_SIZE];
					XMLElement el("e");
					int nid = el.add_attribute("lid", 0);
					FSItemListBuilder::ConstIterator it = pg->begin();
					int M = data.ItemSize();
					for (int j = 0; j < pg->size(); ++j, ++it)
					{
						int eid = *it;
						FSElement_& e = pm->ElementRef(eid);
						if (e.CanExport())
						{
							el.set_attribute(nid, j + 1);
							data.get(j, d);
							el.value(d, M);
							m_xml.add_leaf(el, false);
						}
					}
				}
				m_xml.close_branch();
			}
			FSPartData* partData = dynamic_cast<FSPartData*>(pm->GetMeshDataField(n));
			if (partData)
			{
				double v[FSElement::MAX_NODES] = { 0 };
				FSPartData& data = *partData;
				FSPartSet* partList = data.GetPartSet();
				FSElemList* elemList = data.BuildElemList();
				if (elemList == nullptr) throw InvalidItemListBuilder(partData);
				for (int np = 0; np < partList->size(); ++np)
				{
					int pid = (*partList)[np];
					GPart* pg = po->Part(pid);

					// It is possible that the part was split on export (if it had a mixed mesh)
					// If that is the case, we'll need to split the mesh data accordingly. 
					// we check for this by looping over all the exported domains that map to pg
					vector<FEBioExport4::Domain*>& domList = m_Part[0]->m_Dom;
					for (int l=0; l<domList.size(); ++l)
					{
						// see if the domain is a slice of pg
						FEBioExport4::Domain* dom = domList[l];
						if (dom->m_pg == pg)
						{
							XMLElement tag("ElementData");
							tag.add_attribute("name", data.GetName().c_str());
							tag.add_attribute("elem_set", dom->m_name);

							switch (partData->GetDataType())
							{
							case DATA_SCALAR: break;
							case DATA_VEC3 : tag.add_attribute("data_type", "vec3"); break;
							case DATA_MAT3 : tag.add_attribute("data_type", "mat3"); break;
							default:
								assert(false);
							}

							m_xml.add_branch(tag);
							{
								XMLElement el("e");
								int nid = el.add_attribute("lid", 0);
								int N = elemList->Size();
								FSElemList::Iterator it = elemList->First();
								int lid = 1;
								for (int j = 0; j < N; ++j, ++it)
								{
									FSElement_* pe = it->m_pi;
									if ((pe->m_gid == pid) && (pe->Type() == dom->m_elemType))
									{
										el.set_attribute(nid, lid++);

										if (partData->GetDataType() == DATA_SCALAR)
										{
											if (data.GetDataFormat() == DATA_ITEM)
											{
												el.value(data[j]);
											}
											else if (data.GetDataFormat() == DATA_MULT)
											{
												int nn = pe->Nodes();
												for (int k = 0; k < nn; ++k) v[k] = data.GetValue(j, k);
												el.value(v, nn);
											}
										}
										else if (partData->GetDataType() == DATA_VEC3)
										{
											// we only support DATA_ITEM format
											assert(data.GetDataFormat() == DATA_ITEM);
											vec3d v = data.getVec3d(j);
											el.value(v);
										}
										else if (partData->GetDataType() == DATA_MAT3)
										{
											// we only support DATA_ITEM format
											assert(data.GetDataFormat() == DATA_ITEM);
											mat3d v = data.getMat3d(j);
											el.value(v);
										}
										else assert(false);

										m_xml.add_leaf(el, false);
									}
								}
							}
							m_xml.close_branch();
						}
					}
				}
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
		FSMesh* mesh = model.Object(i)->GetFEMesh();

		for (int j = 0; j < mesh->MeshDataFields(); j++)
		{
			FSSurfaceData* surfData = dynamic_cast<FSSurfaceData*>(mesh->GetMeshDataField(j));
			if (surfData)
			{
				FSSurfaceData& sd = *surfData;

				XMLElement tag("SurfaceData");
				tag.add_attribute("name", sd.GetName().c_str());

				if      (sd.GetDataType() == DATA_SCALAR) tag.add_attribute("data_type", "scalar");
				else if (sd.GetDataType() == DATA_VEC3 ) tag.add_attribute("data_type", "vec3");

				tag.add_attribute("surface", sd.GetSurface()->GetName().c_str());

				m_xml.add_branch(tag);
				{
					XMLElement el("face");
					int n1 = el.add_attribute("lid", 0);

					int nid = 1;
					std::vector<double> data = sd.GetData();
					for (double d : data)
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
		FSMesh* mesh = model.Object(i)->GetFEMesh();

		for (int j = 0; j < mesh->MeshDataFields(); j++)
		{
			FSNodeData* nodeData = dynamic_cast<FSNodeData*>(mesh->GetMeshDataField(j));
			if (nodeData)
			{
				FSNodeData& nd = *nodeData;

				XMLElement tag("NodeData");
				tag.add_attribute("name", nd.GetName().c_str());

				if      (nd.GetDataType() == DATA_SCALAR) tag.add_attribute("data_type", "scalar");
				else if (nd.GetDataType() == DATA_VEC3 ) tag.add_attribute("data_type", "vec3"  );

				FSItemListBuilder* pitem = nd.GetItemList();
				tag.add_attribute("node_set", GetNodeSetName(pitem));

				m_xml.add_branch(tag);
				{
					XMLElement el("node");
					int n1 = el.add_attribute("lid", 0);

					int nid = 1;
					for (int i = 0; i < nd.Size(); ++i)
					{
						el.set_attribute(n1, nid++);

						if      (nd.GetDataType() == DATA_SCALAR) el.value(nd.getScalar(i));
						else if (nd.GetDataType() == DATA_VEC3 ) el.value(nd.getVec3d (i));

						m_xml.add_leaf(el, false);
					}
				}
				m_xml.close_branch();
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteMeshAdaptorSection(FSStep& s)
{
	FSModel& fem = *m_pfem;
	for (int i = 0; i < s.MeshAdaptors(); ++i)
	{
		FSMeshAdaptor* mda = s.MeshAdaptor(i);
		if (mda && mda->IsActive())
		{
			XMLElement el("mesh_adaptor");
			string name = mda->GetName();
			if (name.empty() == false) el.add_attribute("name", name);

			FSItemListBuilder* pi = mda->GetItemList();
			if (pi)
			{
				string elSet = GetElementSetName(pi);
				el.add_attribute("elem_set", elSet);
			}

			WriteModelComponent(mda, el);
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
	// rigid body bcs
	WriteRigidBCs(s);

	// rigid body ics
	WriteRigidICs(s);

	// rigid loads
	WriteRigidLoads(s);

	// rigid connectors
	WriteRigidConnectors(s);
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
			const char* sz = pi->GetName().c_str();

			XMLElement ec("contact");
			ec.add_attribute("name", sz);
			ec.add_attribute("surface_pair", sz);
			WriteModelComponent(pi, ec);
		}
	}
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
	for (GDiscreteObject* pdo : m_discreteSets)
	{
		if (pdo->IsActive())
		{
			GLinearSpring* ps = dynamic_cast<GLinearSpring*>(pdo);
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
			GGeneralSpring* pg = dynamic_cast<GGeneralSpring*>(pdo);
			if (pg)
			{
				dmat.set_attribute(n1, n++);
				dmat.set_attribute(n2, pg->GetName().c_str());
				dmat.set_attribute(n3, "nonlinear spring");
				m_xml.add_branch(dmat, false);
				{
					Param& p = pg->GetParam(GGeneralSpring::MP_F);
					double F = p.GetFloatValue();
					int lc = -1;// (p.GetLoadCurve() ? p.GetLoadCurve()->GetID() : -1);

					XMLElement f;
					f.name("force");
					f.value(F);
					if (lc > 0) f.add_attribute("lc", lc);
					m_xml.add_leaf(f);
				}
				m_xml.close_branch();
			}
			GDiscreteSpringSet* pds = dynamic_cast<GDiscreteSpringSet*>(pdo);
			if (pds && (pds->size()))
			{
				XMLElement dmat("discrete_material");
				dmat.add_attribute("id", n++);
				dmat.add_attribute("name", pds->GetName().c_str());
				FSDiscreteMaterial* dm = pds->GetMaterial();
				WriteModelComponent(dm, dmat);
			}
			GDeformableSpring* ds = dynamic_cast<GDeformableSpring*>(pdo);
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
	for (GDiscreteObject* pdo : m_discreteSets)
	{
		if (pdo->IsActive())
		{
			GLinearSpring* ps = dynamic_cast<GLinearSpring*>(pdo);
			if (ps)
			{
				disc.set_attribute(n1, n++);
				disc.set_attribute(n2, ps->GetName().c_str());
				m_xml.add_empty(disc, false);
			}
			GGeneralSpring* pg = dynamic_cast<GGeneralSpring*>(pdo);
			if (pg)
			{
				disc.set_attribute(n1, n++);
				disc.set_attribute(n2, pg->GetName().c_str());
				m_xml.add_empty(disc, false);
			}
			GDiscreteSpringSet* pds = dynamic_cast<GDiscreteSpringSet*>(pdo);
			if (pds && (pds->size()))
			{
				disc.set_attribute(n1, n++);
				disc.set_attribute(n2, pds->GetName().c_str());
				m_xml.add_empty(disc, false);
			}
			GDeformableSpring* ds = dynamic_cast<GDeformableSpring*>(pdo);
			if (ds)
			{
				disc.set_attribute(n1, n++);
				disc.set_attribute(n2, ds->GetName().c_str());
				m_xml.add_empty(disc, false);
			}
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
void FEBioExport4::WriteConstraints(FSStep& s)
{
	for (int i = 0; i < s.Constraints(); ++i)
	{
		FSModelConstraint* pw = s.Constraint(i);
		if (pw && pw->IsActive())
		{
			if (m_writeNotes) WriteNote(pw);

			XMLElement ec("constraint");
			const char* sz = pw->GetName().c_str();
			ec.add_attribute("name", sz);

			FSSurfaceConstraint* psf = dynamic_cast<FSSurfaceConstraint*>(pw);
			if (psf)
			{
				ec.add_attribute("surface", GetSurfaceName(pw->GetItemList()));
			}

			FSBodyConstraint* pv = dynamic_cast<FSBodyConstraint*>(pw);
			if (pv && pw->GetItemList())
			{
				ec.add_attribute("elem_set", GetElementSetName(pw->GetItemList()));
			}

			// write the constraint
			WriteModelComponent(pw, ec);
		}
	}
}


//-----------------------------------------------------------------------------
// Write the boundary conditions
void FEBioExport4::WriteBC(FSStep& s, FSBoundaryCondition* pbc)
{
	FSModel& fem = m_prj.GetFSModel();

	if (m_writeNotes) WriteNote(pbc);


	XMLElement tag("bc");
	tag.add_attribute("name", pbc->GetName());

	if (pbc->GetMeshItemType() != 0)
	{
		// get the item list
		FSItemListBuilder* pitem = pbc->GetItemList();
		if (pitem == 0) throw InvalidItemListBuilder(pbc);

		if (pbc->GetMeshItemType() == FE_FACE_FLAG)
		{
			// get surface name
			string surfName = GetSurfaceName(pitem);
			tag.add_attribute("surface", surfName);
		}
		else
		{
			// get node set name
			string nodeSetName = GetNodeSetName(pitem);
			tag.add_attribute("node_set", nodeSetName);
		}
	}

	// write the tag
	WriteModelComponent(pbc, tag);
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

			FSItemListBuilder* pitem = pbc->GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(pbc);

			XMLElement load("nodal_load");
			load.add_attribute("name", pbc->GetName());
			load.add_attribute("node_set", GetNodeSetName(pitem));
			WriteModelComponent(pbc, load);
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
			FSItemListBuilder* pitem = psl->GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(psl);

			XMLElement load;
			load.name("surface_load");
			load.add_attribute("name", psl->GetName());
			load.add_attribute("surface", GetSurfaceName(pitem));
			WriteModelComponent(psl, load);
		}
	}
}

//-----------------------------------------------------------------------------
// Export initial conditions
//
void FEBioExport4::WriteInitialSection(FSStep& s)
{
	// initial velocities
	for (int j = 0; j < s.ICs(); ++j)
	{
		FSInitialCondition* pic = s.IC(j);
		if (pic && pic->IsActive())
		{
			if (m_writeNotes) WriteNote(pic);

			XMLElement el("ic");
			el.add_attribute("name", pic->GetName().c_str());

			FSItemListBuilder* pitem = pic->GetItemList();
			if (pitem) el.add_attribute("node_set", GetNodeSetName(pitem));

			WriteModelComponent(pic, el);
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

			FSItemListBuilder* pitem = pbl->GetItemList();

			XMLElement el("body_load");

			if (pitem) el.add_attribute("elem_set", GetElementSetName(pitem));
			WriteModelComponent(pbl, el);
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
		int userParams = 0;
		m_xml.add_branch("Constants");
		{
			int N = fem.Parameters();
			for (int i = 0; i < N; ++i)
			{
				Param& p = fem.GetParam(i);
				if ((p.GetFlags() & FS_PARAM_USER) == 0)
					m_xml.add_leaf(p.GetShortName(), p.GetFloatValue());
				else
					userParams++;
			}
		}
		m_xml.close_branch();

		if (userParams > 0)
		{
			m_xml.add_branch("Variables");
			{
				int N = fem.Parameters();
				for (int i = 0; i < N; ++i)
				{
					Param& p = fem.GetParam(i);
					if ((p.GetFlags() & FS_PARAM_USER) != 0)
					{
						XMLElement el("var");
						el.add_attribute("name", p.GetShortName());
						el.value(p.GetFloatValue());
						m_xml.add_leaf(el);
					}
				}
			}
			m_xml.close_branch();
		}

		if (fem.Solutes() > 0)
		{
			m_xml.add_branch("Solutes");
			{
				int NS = fem.Solutes();
				for (int i = 0; i < NS; ++i)
				{
					SoluteData& s = fem.GetSoluteData(i);
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
					SoluteData& s = fem.GetSBMData(i);
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
	FSModel& fem = m_prj.GetFSModel();

	for (int i = 0; i < fem.LoadControllers(); ++i)
	{
		FSLoadController* plc = fem.GetLoadController(i);

		XMLElement el;
		el.name("load_controller");
		el.add_attribute("id", i + 1);
		if (plc->GetName().empty() == false) el.add_attribute("name", plc->GetName());
		WriteModelComponent(plc, el);
	}
}

//-----------------------------------------------------------------------------

void FEBioExport4::WriteSurfaceSection(FSFaceList& s)
{
	XMLElement ef;
	int n = 1, nn[9];

	int NF = s.Size();
	FSFaceList::Iterator pf = s.First();

	int nfn;
	for (int j = 0; j < NF; ++j, ++n, ++pf)
	{
		if (pf->m_pi == 0) throw InvalidItemListBuilder(0);
		FSFace& face = *(pf->m_pi);
		FSCoreMesh* pm = pf->m_pm;
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
	FSItemListBuilder* pl = l.m_list;
	FSFaceList* pfl = pl->BuildFaceList();
	if (pfl == nullptr) throw InvalidItemListBuilder(l.m_name);
	std::unique_ptr<FSFaceList> ps(pfl);

	XMLElement ef;
	int n = 1, nn[9];

	FSFaceList& s = *pfl;
	int NF = s.Size();
	FSFaceList::Iterator pf = s.First();

	for (int j = 0; j < NF; ++j, ++n, ++pf)
	{
		if (pf->m_pi == 0) throw InvalidItemListBuilder(l.m_name);
		FSFace& face = *(pf->m_pi);
		if (face.CanExport())
		{
			int nfn = face.Nodes();
			FSCoreMesh* pm = pf->m_pm;
			for (int k = 0; k < nfn; ++k) nn[k] = pm->Node(face.n[k]).m_nid;
			switch (nfn)
			{
			case  3: ef.name("tri3" ); break;
			case  4: ef.name("quad4"); break;
			case  6: ef.name("tri6" ); break;
			case  7: ef.name("tri7" ); break;
			case  8: ef.name("quad8"); break;
			case  9: ef.name("quad9"); break;
			case 10: ef.name("tri10"); break;
			default:
				assert(false);
			}
			ef.add_attribute("id", n);
			ef.value(nn, nfn);
			m_xml.add_leaf(ef);
		}
	}
}

void FEBioExport4::WriteEdgeSection(NamedItemList& l)
{
	FSItemListBuilder* pl = l.m_list;
	FSEdgeList* pel = pl->BuildEdgeList();
	if (pel == nullptr) throw InvalidItemListBuilder(l.m_name);
	std::unique_ptr<FSEdgeList> ps(pel);

	XMLElement ef;
	int n = 1, nn[3];

	FSEdgeList& e = *pel;
	int NE = e.Size();
	FSEdgeList::Iterator pe = e.First();

	for (int j = 0; j < NE; ++j, ++n, ++pe)
	{
		if (pe->m_pi == 0) throw InvalidItemListBuilder(l.m_name);
		FSEdge& edge = *(pe->m_pi);
		if (edge.CanExport())
		{
			FSCoreMesh* pm = pe->m_pm;
			int nen = edge.Nodes();
			for (int k = 0; k < nen; ++k) nn[k] = pm->Node(edge.n[k]).m_nid;
			switch (nen)
			{
			case 2: ef.name("line2"); break;
			case 3: ef.name("line3"); break;
			default:
				assert(false);
			}
			ef.add_attribute("id", n);
			ef.value(nn, nen);
			m_xml.add_leaf(ef);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteElementList(FSElemList& el)
{
	int NE = el.Size();
	FSElemList::Iterator pe = el.First();
	for (int i = 0; i < NE; ++i, ++pe)
	{
		FSElement_& el = *(pe->m_pi);
		if (el.CanExport())
		{
			XMLElement e("e");
			e.add_attribute("id", el.m_nid);
			m_xml.add_empty(e);
		}
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

		const char* szfmt = "febio";
		switch (plt.GetPlotFormat())
		{
		case CPlotDataSettings::PLOT_FEBIO: szfmt = "febio"; break;
		case CPlotDataSettings::PLOT_VTK  : szfmt = "vtk"; break;
		}
		p.add_attribute("type", szfmt);

		// count the nr of active plot variables
		int na = 0;
		for (int i = 0; i < N; ++i) if (plt.PlotVariable(i).isActive()) na++;

		if (na > 0)
		{
			m_xml.add_branch(p);
			{
				for (int i = 0; i < N; ++i)
				{
					CPlotVariable& v = plt.PlotVariable(i);
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
									FSItemListBuilder* pl = v.GetDomain(n);
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
				FSLogData& d = log.LogData(i);
				switch (d.Type())
				{
				case FSLogData::LD_NODE:
				{
					XMLElement e;
					e.name("node_data");
					e.add_attribute("data", d.GetDataString());

					if (d.GetFileName().empty() == false)
					{
						e.add_attribute("file", d.GetFileName());
					}

					FSLogNodeData& nd = dynamic_cast<FSLogNodeData&>(d);
					FSItemListBuilder* pg = nd.GetItemList();
					if (pg)
					{
						e.add_attribute("node_set", pg->GetName());
					}
					m_xml.add_empty(e);
				}
				break;
				case FSLogData::LD_FACE:
				{
					XMLElement e;
					e.name("face_data");
					e.add_attribute("data", d.GetDataString());

					if (d.GetFileName().empty() == false) e.add_attribute("file", d.GetFileName());

					FSLogFaceData& fd = dynamic_cast<FSLogFaceData&>(d);
					FSItemListBuilder* pg = fd.GetItemList();
					if (pg) e.add_attribute("surface", pg->GetName());

					m_xml.add_empty(e);
				}
				break;
				case FSLogData::LD_SURFACE:
				{
					XMLElement e;
					e.name("surface_data");
					e.add_attribute("data", d.GetDataString());

					if (d.GetFileName().empty() == false) e.add_attribute("file", d.GetFileName());

					FSLogSurfaceData& fd = dynamic_cast<FSLogSurfaceData&>(d);
					FSItemListBuilder* pg = fd.GetItemList();
					if (pg) e.add_attribute("surface", pg->GetName());

					m_xml.add_empty(e);
				}
				break;
				case FSLogData::LD_DOMAIN:
				{
					XMLElement e;
					e.name("domain_data");
					e.add_attribute("data", d.GetDataString());

					if (d.GetFileName().empty() == false) e.add_attribute("file", d.GetFileName());

					FSLogDomainData& fd = dynamic_cast<FSLogDomainData&>(d);
					FSItemListBuilder* pg = fd.GetItemList();
					if (pg) e.add_attribute("domain", pg->GetName());

					m_xml.add_empty(e);
				}
				break;
				case FSLogData::LD_ELEM:
				{
					XMLElement e;
					e.name("element_data");
					e.add_attribute("data", d.GetDataString());

					if (d.GetFileName().empty() == false)
					{
						e.add_attribute("file", d.GetFileName());
					}

					FSLogElemData& ed = dynamic_cast<FSLogElemData&>(d);
					FSItemListBuilder* pg = ed.GetItemList();
					if (pg)
					{
						e.add_attribute("elem_set", pg->GetName());
					}
					m_xml.add_empty(e);
				}
				break;
				case FSLogData::LD_RIGID:
				{
					XMLElement e;
					e.name("rigid_body_data");
					e.add_attribute("data", d.GetDataString());

					if (d.GetFileName().empty() == false)
					{
						e.add_attribute("file", d.GetFileName());
					}

					FSLogRigidData& rd = dynamic_cast<FSLogRigidData&>(d);
					GMaterial* pm = fem.GetMaterialFromID(rd.GetMatID());
					if (pm)
					{
						e.value(pm->m_ntag);
						m_xml.add_leaf(e);
					}
					else m_xml.add_empty(e);
				}
				break;
				case FSLogData::LD_CNCTR:
				{
					XMLElement e;
					e.name("rigid_connector_data");
					e.add_attribute("data", d.GetDataString());

					if (d.GetFileName().empty() == false)
					{
						e.add_attribute("file", d.GetFileName());
					}

					FSLogConnectorData& rd = dynamic_cast<FSLogConnectorData&>(d);
					FSRigidConnector* rc = fem.GetRigidConnectorFromID(rd.GetConnectorID());
					if (rc)
					{
						e.value(rd.GetConnectorID());
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
			m_xml.close_branch();

			// output initial section
			int nic = step.ICs();
			if (nic > 0)
			{
				m_xml.add_branch("Initial");
				{
					WriteInitialSection(step);
				}
				m_xml.close_branch();
			}

			// output boundary section
			int nbc = step.BCs();
			if (nbc > 0)
			{
				m_xml.add_branch("Boundary");
				{
					WriteBoundarySection(step);
				}
				m_xml.close_branch(); // Boundary
			}

			int nrc = step.RigidBCs() + step.RigidLoads() + step.RigidICs() + step.RigidConnectors();
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
			int nnlc = step.Constraints();
			if (nnlc > 0)
			{
				m_xml.add_branch("Constraints");
				{
					WriteConstraintSection(step);
				}
				m_xml.close_branch(); // Constraints
			}

			int nma = step.MeshAdaptors();
			if (nma > 0)
			{
				m_xml.add_branch("MeshAdaptor");
				{
					WriteMeshAdaptorSection(step);
				}
				m_xml.close_branch();
			}
		}
		m_xml.close_branch(); // step
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteRigidBCs(FSStep& s)
{
	for (int i = 0; i < s.RigidBCs(); ++i)
	{
		FSRigidBC* prc = s.RigidBC(i);
		if (prc && prc->IsActive())
		{
			if (m_writeNotes) WriteNote(prc);
			XMLElement el;
			el.name("rigid_bc");
			el.add_attribute("name", prc->GetName());
			WriteModelComponent(prc, el);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteRigidICs(FSStep& s)
{
	for (int i = 0; i < s.RigidICs(); ++i)
	{
		FSRigidIC* prc = s.RigidIC(i);
		if (prc && prc->IsActive())
		{
			if (m_writeNotes) WriteNote(prc);
			XMLElement el;
			el.name("rigid_ic");
			el.add_attribute("name", prc->GetName());
			WriteModelComponent(prc, el);
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
			WriteModelComponent(prl, el);
		}
	}
}

//-----------------------------------------------------------------------------
// write rigid connectors
//
void FEBioExport4::WriteRigidConnectors(FSStep& s)
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
			WriteModelComponent(prc, el);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport4::WriteConstraintSection(FSStep& s)
{
	// some contact definitions are actually stored in the constraint section
	WriteConstraints(s);
}
