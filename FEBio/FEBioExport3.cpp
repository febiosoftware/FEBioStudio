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

#include "FEBioExport3.h"
#include <FEMLib/FEInitialCondition.h>
#include <FEMLib/FESurfaceLoad.h>
#include <FEMLib/FEBodyLoad.h>
#include <FEMLib/FEModelConstraint.h>
#include <FEMLib/FERigidLoad.h>
#include <GeomLib/GObject.h>
#include <GeomLib/GGroup.h>
#include <MeshLib/FENodeData.h>
#include <MeshLib/FESurfaceData.h>
#include <MeshLib/FEElementData.h>
#include <GeomLib/GModel.h>
#include <FEMLib/FSProject.h>
#include <FEBioLink/FEBioModule.h>
#include <memory>
#include <sstream>
#include <FECore/FETransform.h>

using namespace std;

//-----------------------------------------------------------------------------
// defined in FEFEBioExport25.cpp
FSNodeList* BuildNodeList(GFace* pf);
FSNodeList* BuildNodeList(GPart* pg);
FSNodeList* BuildNodeList(GNode* pn);
FSNodeList* BuildNodeList(GEdge* pe);
FEFaceList* BuildFaceList(GFace* face);
const char* ElementTypeString(int ntype);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FEBioExport3::FEBioExport3(FSProject& prj) : FEBioExport(prj)
{
	m_exportParts = false;
	m_useReactionMaterial2 = false;	// will be set to true for reaction-diffusion problems
	m_writeNotes = true;
	m_exportEnumStrings = true;
	m_writeControlSection = true;
	m_exportMesh = true;
}

FEBioExport3::~FEBioExport3()
{
	Clear();
}

void FEBioExport3::Clear()
{
	FEBioExport::Clear();
	m_pSurf.clear();
	m_pNSet.clear();
	m_pESet.clear();
	m_ElSet.clear();
}

//----------------------------------------------------------------------------
FEBioExport3::Part* FEBioExport3::FindPart(GObject* po)
{
	for (size_t i = 0; i<m_Part.size(); ++i)
	{
		Part* pi = m_Part[i];
		if (po == pi->m_obj) return pi;
	}
	return 0;
}

//----------------------------------------------------------------------------
const char* FEBioExport3::GetSurfaceName(FEItemListBuilder* pl)
{
	int N = (int)m_pSurf.size();
	for (int i = 0; i<N; ++i)
		if (m_pSurf[i].m_list == pl) return m_pSurf[i].m_name.c_str();
	assert(false);
	return 0;
}

//----------------------------------------------------------------------------
string FEBioExport3::GetElementSetName(FEItemListBuilder* pl)
{
	int N = (int)m_pESet.size();
	for (int i = 0; i<N; ++i)
		if (m_pESet[i].m_list == pl) return m_pESet[i].m_name.c_str();
	assert(false);
	return "";
}

//----------------------------------------------------------------------------
string FEBioExport3::GetNodeSetName(FEItemListBuilder* pl)
{
	// search the nodesets first
	int N = (int)m_pNSet.size();
	for (int i = 0; i<N; ++i)
		if (m_pNSet[i].m_list == pl) return m_pNSet[i].m_name.c_str();

	// search the surfaces
	N = (int)m_pSurf.size();
	for (int i = 0; i<N; ++i)
		if (m_pSurf[i].m_list == pl)
		{
			string surfName = m_pSurf[i].m_name;
			return string("@surface:") + surfName;
		}

	// search the element sets
	N = (int)m_pESet.size();
	for (int i=0; i<N; ++i)
		if (m_pESet[i].m_list == pl)
		{
			string setName = m_pESet[i].m_name;
			return string("@elem_set:") + setName;
		}

	assert(false);
	return "";
}

//-----------------------------------------------------------------------------
void FEBioExport3::AddNodeSet(const std::string& name, FEItemListBuilder* pl)
{
	if (m_exportParts)
	{
		switch (pl->Type())
		{
		case GO_NODE:
		{
			GNodeList* itemList = dynamic_cast<GNodeList*>(pl); assert(itemList);
			vector<GNode*> nodeList = itemList->GetNodeList();
			for (int i = 0; i<nodeList.size(); ++i)
			{
				GNode* node = nodeList[i];
				GObject* po = dynamic_cast<GObject*>(node->Object());
				Part* part = FindPart(po);
				NodeSet* ns = part->FindNodeSet(node->GetName());
				if (ns == 0) part->m_NSet.push_back(new NodeSet(node->GetName(), BuildNodeList(node)));
			}
		}
		break;
		case GO_EDGE:
		{
			GEdgeList* itemList = dynamic_cast<GEdgeList*>(pl); assert(itemList);
			vector<GEdge*> edgeList = itemList->GetEdgeList();
			for (int i = 0; i < edgeList.size(); ++i)
			{
				GEdge* edge = edgeList[i];
				GObject* po = dynamic_cast<GObject*>(edge->Object());
				Part* part = FindPart(po);
				NodeSet* ns = part->FindNodeSet(edge->GetName());
				if (ns == 0) part->m_NSet.push_back(new NodeSet(edge->GetName(), BuildNodeList(edge)));
			}
		}
		break;
		case GO_FACE:
		{
			GFaceList* itemList = dynamic_cast<GFaceList*>(pl); assert(itemList);
			vector<GFace*> faceList = itemList->GetFaceList();
			for (int i = 0; i<faceList.size(); ++i)
			{
				GFace* face = faceList[i];
				GObject* po = dynamic_cast<GObject*>(face->Object());
				Part* part = FindPart(po);
				NodeSet* ns = part->FindNodeSet(face->GetName());
				if (ns == 0) part->m_NSet.push_back(new NodeSet(face->GetName(), BuildNodeList(face)));
			}
		}
		break;
		case GO_PART:
		{
			GPartList* itemList = dynamic_cast<GPartList*>(pl); assert(itemList);
			vector<GPart*> partList = itemList->GetPartList();
			for (int i = 0; i<partList.size(); ++i)
			{
				GPart* pg = partList[i];
				GObject* po = dynamic_cast<GObject*>(pg->Object());
				Part* part = FindPart(po);
				NodeSet* ns = part->FindNodeSet(pg->GetName());
				if (ns == 0) part->m_NSet.push_back(new NodeSet(pg->GetName(), BuildNodeList(pg)));
			}
		}
		break;
		case FE_NODESET:
		{
			FSNodeSet* nset = dynamic_cast<FSNodeSet*>(pl); assert(nset);
			GObject* po = nset->GetGObject();
			Part* part = FindPart(po);
			NodeSet* ns = part->FindNodeSet(name);
			if (ns == 0) part->m_NSet.push_back(new NodeSet(name, nset->BuildNodeList()));
		}
		break;
		case FE_SURFACE:
		{
			FSSurface* face = dynamic_cast<FSSurface*>(pl); assert(face);
			GObject* po = face->GetGObject();
			Part* part = FindPart(po);
			NodeSet* ns = part->FindNodeSet(name);
			if (ns == 0) part->m_NSet.push_back(new NodeSet(name, face->BuildNodeList()));
		}
		break;
		default:
			assert(false);
		}
	}

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
void FEBioExport3::AddSurface(const std::string& name, FEItemListBuilder* pl)
{
	if (m_exportParts)
	{
		switch (pl->Type())
		{
		case GO_FACE:
		{
			GFaceList* itemList = dynamic_cast<GFaceList*>(pl); assert(itemList);
			vector<GFace*> faceList = itemList->GetFaceList();
			for (int i = 0; i<faceList.size(); ++i)
			{
				GFace* face = faceList[i];
				GObject* po = dynamic_cast<GObject*>(face->Object());
				Part* part = FindPart(po);
				Surface* s = part->FindSurface(face->GetName());
				if (s == 0) part->m_Surf.push_back(new Surface(face->GetName(), BuildFaceList(face)));
			}
		}
		break;
		case FE_SURFACE:
		{
			FSSurface* surf = dynamic_cast<FSSurface*>(pl); assert(surf);
			GObject* po = surf->GetGObject();
			Part* part = FindPart(po);
			Surface* s = part->FindSurface(name);
			if (s == 0) part->m_Surf.push_back(new Surface(name, surf->BuildFaceList()));
		}
		break;
		}
	}
	else
	{
		// make sure this has not been added 
		for (int i = 0; i<m_pSurf.size(); ++i)
		{
			NamedItemList& surf = m_pSurf[i];
			if ((surf.m_list == pl) && (surf.m_name == name)) return;
		}
	}

	m_pSurf.push_back(NamedItemList(string(name), pl));
}

//-----------------------------------------------------------------------------
void FEBioExport3::AddElemSet(const std::string& name, FEItemListBuilder* pl)
{
	assert(pl);
	if (pl == nullptr) return;
	if (m_exportParts)
	{
		switch (pl->Type())
		{
		case FE_ELEMSET:
		{
			FSElemSet* pg = dynamic_cast<FSElemSet*>(pl); assert(pg);
			GObject* po = pg->GetGObject();
			Part* part = FindPart(po);
			ElementList* es = part->FindElementSet(name);
			if (es == 0) part->m_ELst.push_back(new ElementList(name, pg->BuildElemList()));
		}
		break;
		default:
			assert(false);
		}
	}
	m_pESet.push_back(NamedItemList(string(name), pl));
}

//-----------------------------------------------------------------------------
bool FEBioExport3::PrepareExport(FSProject& prj)
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
		for (int i = 2; i<m_nsteps; ++i)
		{
			if (fem.GetStep(i)->GetType() != ntype) return errf("All analysis steps must be of same type for this file format.");
		}
	}

	// build the parts
	if (m_exportParts)
	{
		int nobj = model.Objects();
		for (int i = 0; i<nobj; ++i)
		{
			m_Part.push_back(new Part(model.Object(i)));
		}
	}
	if (m_exportMesh)
	{
		m_Part.push_back(new Part(nullptr));
	}

	// Build the named lists
	BuildItemLists(prj);

	// see if we need to add a MeshData section
	m_bdata = false;
	if (model.ShellElements() > 0) m_bdata = true;	// for shell thicknesses
	for (int i = 0; i<fem.Materials(); ++i)
	{
		// get the material properties
		GMaterial* gmat = fem.GetMaterial(i);
		FSMaterial* pmat = gmat->GetMaterialProperties();

		// see if we should write fibers
		// This is only done if the material specifies the "user" fiber property
		bool writeFibers = false;

		FSProperty* fiberProp = pmat->FindProperty("fiber");
		if (fiberProp && fiberProp->Size() == 1)
		{
			FSCoreBase* fib = fiberProp->GetComponent(0);
			if (strcmp(fib->GetTypeString(), "user") == 0)
			{
				m_bdata = true;
			}
		}
	}
	for (int i = 0; i<model.Objects(); ++i)
	{
		FSMesh* pm = model.Object(i)->GetFEMesh();
		for (int j = 0; j<pm->Elements(); ++j)
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
	if (fem.MeshDataGenerators() > 0) m_bdata = true;

	return true;
}

//-----------------------------------------------------------------------------
void FEBioExport3::BuildItemLists(FSProject& prj)
{
	FSModel& fem = prj.GetFSModel();

	// get the nodesets (bc's)
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FSStep* pstep = fem.GetStep(i);
		for (int j = 0; j<pstep->BCs(); ++j)
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
		for (int j = 0; j<pstep->Loads(); ++j)
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
		for (int j = 0; j<pstep->ICs(); ++j)
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
		for (int j = 0; j<pstep->Interfaces(); ++j)
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
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FSStep* pstep = fem.GetStep(i);
		for (int j = 0; j<pstep->Loads(); ++j)
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
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FSStep* pstep = fem.GetStep(i);
		for (int j = 0; j<pstep->Interfaces(); ++j)
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
		for (int i = 0; i<nobj; ++i)
		{
			GObject* po = model.Object(i);
			FSMesh* pm = po->GetFEMesh();
			if (pm)
			{
				int nsurf = po->FESurfaces();
				for (int j = 0; j<nsurf; ++j)
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
			}
		}
	}

	// check all the (surface) plot variables
	CPlotDataSettings& plt = prj.GetPlotDataSettings();
	for (int i = 0; i<plt.PlotVariables(); ++i)
	{
		CPlotVariable& var = plt.PlotVariable(i);
		if (var.domainType() == DOMAIN_SURFACE)
		{
			int ND = var.Domains();
			for (int j = 0; j<ND; ++j)
			{
				FEItemListBuilder* pl = var.GetDomain(j);
				AddSurface(pl->GetName(), pl);
			}
		}
	}

	GModel& model = fem.GetModel();
	CLogDataSettings& log = prj.GetLogDataSettings();
	for (int i = 0; i<log.LogDataSize(); ++i)
	{
		FSLogData& di = log.LogData(i);
		if (di.Type() == FSLogData::LD_ELEM)
		{
			FSLogElemData& ed = dynamic_cast<FSLogElemData&>(di);
			FEItemListBuilder* pg = ed.GetItemList();
			if (pg)
			{
				AddElemSet(pg->GetName(), pg);
			}
		}
		if (di.Type() == FSLogData::LD_NODE)
		{
			FSLogNodeData& nd = dynamic_cast<FSLogNodeData&>(di);
			FEItemListBuilder* pg = nd.GetItemList();
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
		FSMesh* mesh = po->GetFEMesh();
		if (mesh)
		{
			int ND = mesh->MeshDataFields();
			for (int j = 0; j < ND; ++j)
			{
				FEMeshData* data = mesh->GetMeshDataField(j);

				switch (data->GetDataClass())
				{
				case NODE_DATA:
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
				case ELEM_DATA:
				{
					FEElementData* map = dynamic_cast<FEElementData*>(data); assert(map);
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
bool FEBioExport3::Write(const char* szfile)
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
			FSStep* pstep = fem.GetStep(1);
			if (pstep->StepComponents() == 0) bsingle_step = true;
		}

		// open the file
		if (!m_xml.open(szfile)) return errf("Failed opening file %s", szfile);

		if (m_writeNotes) WriteNote(&mdl);

		// set modified formats
		XMLElement::intFormat = "%d";

		XMLElement el;

		// output root element
		el.name("febio_spec");
		el.add_attribute("version", "3.0");

		m_xml.add_branch(el);
		{
			// write the module section
			// Note that this format assumes that all steps are of the same type.
			// (This is verified in PrepareExport)
			if (m_nsteps > 1)
			{
				FSStep* pstep = fem.GetStep(1);
				if (WriteSection(FEBIO_MODULE)) WriteModuleSection(pstep);
			}

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

			// output geometry section
			if (m_exportMesh == false)
			{
				if ((fem.GetModel().Objects() > 0) && (WriteSection(FEBIO_GEOMETRY)))
				{
					m_xml.add_branch("Geometry");
					{
						WriteGeometrySection();
//						WriteGeometrySection2();
					}
					m_xml.close_branch(); // Geometry
				}
			}
			else
			{
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

			// output boundary section
			int nbc = pstep->BCs() + pstep->Interfaces() + fem.GetModel().DiscreteObjects();
			if ((nbc > 0) && (WriteSection(FEBIO_BOUNDARY)))
			{
				m_xml.add_branch("Boundary");
				{
					WriteBoundarySection(*pstep);
				}
				m_xml.close_branch(); // Boundary
			}

			int nrc = pstep->RigidBCs() + pstep->RigidICs() + pstep->RigidLoads() + pstep->RigidConnectors() + CountInterfaces<FSRigidJoint>(fem);
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

			// output contact
			int nci = pstep->Interfaces() - CountInterfaces<FSRigidJoint>(fem);
			int nLC = pstep->LinearConstraints();
			if (((nci > 0) || (nLC > 0)) && (WriteSection(FEBIO_CONTACT)))
			{
				m_xml.add_branch("Contact");
				{
					WriteContactSection(*pstep);
				}
				m_xml.close_branch(); // Contact
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

			// output initial section
			int nic = pstep->ICs() + pstep->RigidICs();
			if ((nic > 0) && (WriteSection(FEBIO_INITIAL)))
			{
				m_xml.add_branch("Initial");
				{
					WriteInitialSection();
				}
				m_xml.close_branch(); // Initial
			}

			// output discrete elements (the obsolete spring-tied interface generates springs as well)
			int nrb = fem.GetModel().DiscreteObjects() + CountInterfaces<FSSpringTiedInterface>(fem);
			if ((nrb > 0) && (WriteSection(FEBIO_DISCRETE)))
			{
				m_xml.add_branch("Discrete");
				{
					WriteDiscreteSection(*pstep);
				}
				m_xml.close_branch(); // Discrete
			}

			// step data
			if (WriteSection(FEBIO_STEPS))
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
void FEBioExport3::WriteModuleSection(FSStep* pstep)
{
	XMLElement t;
	t.name("Module");
	switch (pstep->GetType())
	{
	case FE_STEP_MECHANICS: t.add_attribute("type", "solid"); break;
	case FE_STEP_HEAT_TRANSFER: t.add_attribute("type", "heat"); break;
	case FE_STEP_BIPHASIC: t.add_attribute("type", "biphasic"); break;
	case FE_STEP_BIPHASIC_SOLUTE: t.add_attribute("type", "solute"); break;
	case FE_STEP_MULTIPHASIC: t.add_attribute("type", "multiphasic"); break;
	case FE_STEP_FLUID: t.add_attribute("type", "fluid"); break;
	case FE_STEP_FLUID_FSI: t.add_attribute("type", "fluid-FSI"); break;
	case FE_STEP_REACTION_DIFFUSION: t.add_attribute("type", "reaction-diffusion"); m_useReactionMaterial2 = true; break;
    case FE_STEP_POLAR_FLUID: t.add_attribute("type", "polar fluid"); break;
    case FE_STEP_FLUID_SOLUTES: t.add_attribute("type", "fluid-solutes"); break;
    case FE_STEP_THERMO_FLUID: t.add_attribute("type", "thermo-fluid"); break;
	case FE_STEP_FEBIO_ANALYSIS:
	{
		int mod = m_prj.GetModule();
		const char* szmod = FEBio::GetModuleName(mod);
		t.add_attribute("type", szmod);
	}
	break;
	};

	m_xml.add_empty(t);
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteControlSection(FSStep& step)
{
	FSAnalysisStep* analysis = dynamic_cast<FSAnalysisStep*>(&step);
	if (analysis)
	{
		STEP_SETTINGS& ops = analysis->GetSettings();
		int ntype = analysis->GetType();
		switch (ntype)
		{
		case FE_STEP_MECHANICS: WriteSolidControlParams(analysis); break;
		case FE_STEP_HEAT_TRANSFER: WriteHeatTransferControlParams(analysis); break;
		case FE_STEP_BIPHASIC: WriteBiphasicControlParams(analysis); break;
		case FE_STEP_BIPHASIC_SOLUTE: WriteBiphasicSoluteControlParams(analysis); break;
		case FE_STEP_MULTIPHASIC: WriteBiphasicSoluteControlParams(analysis); break;
		case FE_STEP_FLUID: WriteFluidControlParams(analysis); break;
		case FE_STEP_FLUID_FSI: WriteFluidFSIControlParams(analysis); break;
        case FE_STEP_POLAR_FLUID: WritePolarFluidControlParams(analysis); break;
		case FE_STEP_REACTION_DIFFUSION: WriteReactionDiffusionControlParams(analysis); break;
		default:
			assert(false);
		}
	}
	else
	{
		WriteParamList(step);
		for (int i = 0; i < step.Properties(); ++i)
		{
			FSProperty& prop = step.GetProperty(i);
			FSStepComponent* pc = dynamic_cast<FSStepComponent*>(prop.GetComponent(0));
			if (pc)
			{
				m_xml.add_branch(prop.GetName().c_str());
				WriteParamList(*pc);
				m_xml.close_branch();
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteSolidControlParams(FSAnalysisStep* pstep)
{
	XMLElement el;
	STEP_SETTINGS& ops = pstep->GetSettings();

	m_xml.add_leaf("analysis", (ops.nanalysis == 0 ? "STATIC" : "DYNAMIC"));
	m_xml.add_leaf("time_steps", ops.ntime);
	m_xml.add_leaf("step_size", ops.dt);

	m_xml.add_branch("solver");
	{
		m_xml.add_leaf("max_refs", ops.maxref);
		m_xml.add_leaf("max_ups", ops.ilimit);
		m_xml.add_leaf("diverge_reform", ops.bdivref);
		m_xml.add_leaf("reform_each_time_step", ops.brefstep);

		// write the parameters
		WriteParamList(*pstep);

		if ((ops.nanalysis != 0) && ops.override_rhoi) {
			m_xml.add_leaf("alpha", ops.alpha);
			m_xml.add_leaf("beta", ops.beta);
			m_xml.add_leaf("gamma", ops.gamma);
		}

		if (ops.nmatfmt != 0)
		{
			m_xml.add_leaf("symmetric_stiffness", (ops.nmatfmt == 1 ? 1 : 0));
		}

		if (ops.bminbw)
		{
			m_xml.add_leaf("optimize_bw", 1);
		}
	}
	m_xml.close_branch();

	if (ops.bauto)
	{
		LoadCurve* plc = pstep->GetMustPointLoadCurve();
		el.name("time_stepper");
		m_xml.add_branch(el);
		{
			m_xml.add_leaf("dtmin", ops.dtmin);

			el.name("dtmax");
			if (ops.bmust && plc)
				el.add_attribute("lc", plc->GetID());
			else el.value(ops.dtmax);
			m_xml.add_leaf(el);

			m_xml.add_leaf("max_retries", ops.mxback);
			m_xml.add_leaf("opt_iter", ops.iteopt);

			if (ops.ncut > 0) m_xml.add_leaf("aggressiveness", ops.ncut);
		}
		m_xml.close_branch();
	}

	if (ops.plot_level != 1)
	{
		const char* sz[] = { "PLOT_NEVER", "PLOT_MAJOR_ITRS", "PLOT_MINOR_ITRS", "PLOT_MUST_POINTS", "PLOT_FINAL", "PLOT_AUGMENTATIONS", "PLOT_STEP_FINAL" };
		m_xml.add_leaf("plot_level", sz[ops.plot_level]);
	}

	if (ops.plot_stride != 1)
	{
		m_xml.add_leaf("plot_stride", ops.plot_stride);
	}
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteHeatTransferControlParams(FSAnalysisStep* pstep)
{
	XMLElement el;

	STEP_SETTINGS& ops = pstep->GetSettings();

	m_xml.add_leaf("analysis", (ops.nanalysis == 0 ? "STATIC" : "TRANSIENT"));
	m_xml.add_leaf("time_steps", ops.ntime);
	m_xml.add_leaf("step_size", ops.dt);

	if (ops.bauto)
	{
		LoadCurve* plc = pstep->GetMustPointLoadCurve();
		el.name("time_stepper");
		m_xml.add_branch(el);
		{
			m_xml.add_leaf("dtmin", ops.dtmin);

			el.name("dtmax");
			if (ops.bmust && plc)
				el.add_attribute("lc", plc->GetID());
			else el.value(ops.dtmax);
			m_xml.add_leaf(el);

			m_xml.add_leaf("max_retries", ops.mxback);
			m_xml.add_leaf("opt_iter", ops.iteopt);

			if (ops.ncut > 0) m_xml.add_leaf("aggressiveness", ops.ncut);
		}
		m_xml.close_branch();
	}
}


//-----------------------------------------------------------------------------
void FEBioExport3::WriteBiphasicControlParams(FSAnalysisStep* pstep)
{
	XMLElement el;
	STEP_SETTINGS& ops = pstep->GetSettings();

	m_xml.add_leaf("analysis", (ops.nanalysis == 0 ? "STEADY-STATE" : "TRANSIENT"));
	m_xml.add_leaf("time_steps", ops.ntime);
	m_xml.add_leaf("step_size", ops.dt);

	m_xml.add_branch("solver");
	{
		m_xml.add_leaf("max_refs", ops.maxref);
		m_xml.add_leaf("max_ups", ops.ilimit);
		m_xml.add_leaf("diverge_reform", ops.bdivref);
		m_xml.add_leaf("reform_each_time_step", ops.brefstep);

		// write the parameters
		WriteParamList(*pstep);

		if (ops.bminbw)
		{
			m_xml.add_leaf("optimize_bw", 1);
		}

		if (ops.nmatfmt != 0)
		{
			m_xml.add_leaf("symmetric_stiffness", (ops.nmatfmt == 1 ? 1 : 0));
		}
	}
	m_xml.close_branch();

	if (ops.bauto)
	{
		LoadCurve* plc = pstep->GetMustPointLoadCurve();
		el.name("time_stepper");
		m_xml.add_branch(el);
		{
			m_xml.add_leaf("dtmin", ops.dtmin);

			el.name("dtmax");
			if (ops.bmust && plc)
				el.add_attribute("lc", plc->GetID());
			else el.value(ops.dtmax);
			m_xml.add_leaf(el);

			m_xml.add_leaf("max_retries", ops.mxback);
			m_xml.add_leaf("opt_iter", ops.iteopt);

			if (ops.ncut > 0) m_xml.add_leaf("aggressiveness", ops.ncut);
		}
		m_xml.close_branch();
	}

    if (ops.plot_level != 1)
    {
        const char* sz[] = { "PLOT_NEVER", "PLOT_MAJOR_ITRS", "PLOT_MINOR_ITRS", "PLOT_MUST_POINTS", "PLOT_FINAL", "PLOT_AUGMENTATIONS", "PLOT_STEP_FINAL" };
        m_xml.add_leaf("plot_level", sz[ops.plot_level]);
    }

	if (ops.plot_stride != 1)
	{
		m_xml.add_leaf("plot_stride", ops.plot_stride);
	}
}


//-----------------------------------------------------------------------------
void FEBioExport3::WriteBiphasicSoluteControlParams(FSAnalysisStep* pstep)
{
	XMLElement el;
	STEP_SETTINGS& ops = pstep->GetSettings();

	m_xml.add_leaf("analysis", (ops.nanalysis == 0 ? "STEADY-STATE" : "TRANSIENT"));
	m_xml.add_leaf("time_steps", ops.ntime);
	m_xml.add_leaf("step_size", ops.dt);

	m_xml.add_branch("solver");
	{
		m_xml.add_leaf("max_refs", ops.maxref);
		m_xml.add_leaf("max_ups", ops.ilimit);
		m_xml.add_leaf("diverge_reform", ops.bdivref);
		m_xml.add_leaf("reform_each_time_step", ops.brefstep);

		// write the parameters
		WriteParamList(*pstep);

		if (ops.bminbw)
		{
			m_xml.add_leaf("optimize_bw", 1);
		}

		if (ops.nmatfmt != 0)
		{
			m_xml.add_leaf("symmetric_stiffness", (ops.nmatfmt == 1 ? 1 : 0));
		}
	}
	m_xml.close_branch();

	if (ops.bauto)
	{
		LoadCurve* plc = pstep->GetMustPointLoadCurve();
		el.name("time_stepper");
		m_xml.add_branch(el);
		{
			m_xml.add_leaf("dtmin", ops.dtmin);

			el.name("dtmax");
			if (ops.bmust && plc)
				el.add_attribute("lc", plc->GetID());
			else el.value(ops.dtmax);
			m_xml.add_leaf(el);

			m_xml.add_leaf("max_retries", ops.mxback);
			m_xml.add_leaf("opt_iter", ops.iteopt);

			if (ops.ncut > 0) m_xml.add_leaf("aggressiveness", ops.ncut);
		}
		m_xml.close_branch();
	}

    if (ops.plot_level != 1)
    {
        const char* sz[] = { "PLOT_NEVER", "PLOT_MAJOR_ITRS", "PLOT_MINOR_ITRS", "PLOT_MUST_POINTS", "PLOT_FINAL", "PLOT_AUGMENTATIONS", "PLOT_STEP_FINAL" };
        m_xml.add_leaf("plot_level", sz[ops.plot_level]);
    }

	if (ops.plot_stride != 1)
	{
		m_xml.add_leaf("plot_stride", ops.plot_stride);
	}
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteFluidControlParams(FSAnalysisStep* pstep)
{
	XMLElement el;
	STEP_SETTINGS& ops = pstep->GetSettings();

	m_xml.add_leaf("analysis", (ops.nanalysis == 0 ? "STEADY-STATE" : "DYNAMIC"));
	m_xml.add_leaf("time_steps", ops.ntime);
	m_xml.add_leaf("step_size", ops.dt);

	m_xml.add_branch("solver");
	{
		m_xml.add_leaf("max_refs", ops.maxref);
		m_xml.add_leaf("max_ups", ops.ilimit);
		m_xml.add_leaf("diverge_reform", ops.bdivref);
		m_xml.add_leaf("reform_each_time_step", ops.brefstep);

		// write the parameters
		WriteParamList(*pstep);

		if (ops.bminbw)
		{
			m_xml.add_leaf("optimize_bw", 1);
		}

		if (ops.nmatfmt != 0)
		{
			m_xml.add_leaf("symmetric_stiffness", (ops.nmatfmt == 1 ? 1 : 0));
		}
	}
	m_xml.close_branch();

	if (ops.bauto)
	{
		LoadCurve* plc = pstep->GetMustPointLoadCurve();
		el.name("time_stepper");
		m_xml.add_branch(el);
		{
			m_xml.add_leaf("dtmin", ops.dtmin);

			el.name("dtmax");
			if (ops.bmust && plc)
				el.add_attribute("lc", plc->GetID());
			else el.value(ops.dtmax);
			m_xml.add_leaf(el);

			m_xml.add_leaf("max_retries", ops.mxback);
			m_xml.add_leaf("opt_iter", ops.iteopt);

			if (ops.ncut > 0) m_xml.add_leaf("aggressiveness", ops.ncut);
		}
		m_xml.close_branch();
	}

    if (ops.plot_level != 1)
    {
        const char* sz[] = { "PLOT_NEVER", "PLOT_MAJOR_ITRS", "PLOT_MINOR_ITRS", "PLOT_MUST_POINTS", "PLOT_FINAL", "PLOT_AUGMENTATIONS", "PLOT_STEP_FINAL" };
        m_xml.add_leaf("plot_level", sz[ops.plot_level]);
    }

	if (ops.plot_stride != 1)
	{
		m_xml.add_leaf("plot_stride", ops.plot_stride);
	}
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteFluidFSIControlParams(FSAnalysisStep* pstep)
{
	XMLElement el;
	STEP_SETTINGS& ops = pstep->GetSettings();

	m_xml.add_leaf("analysis", (ops.nanalysis == 0 ? "STEADY-STATE" : "DYNAMIC"));
	m_xml.add_leaf("time_steps", ops.ntime);
	m_xml.add_leaf("step_size", ops.dt);

	m_xml.add_branch("solver");
	{
		m_xml.add_leaf("max_refs", ops.maxref);
		m_xml.add_leaf("max_ups", ops.ilimit);
		m_xml.add_leaf("diverge_reform", ops.bdivref);
		m_xml.add_leaf("reform_each_time_step", ops.brefstep);

		// write the parameters
		WriteParamList(*pstep);

		if (ops.bminbw)
		{
			m_xml.add_leaf("optimize_bw", 1);
		}

		if (ops.nmatfmt != 0)
		{
			m_xml.add_leaf("symmetric_stiffness", (ops.nmatfmt == 1 ? 1 : 0));
		}
	}
	m_xml.close_branch();

	if (ops.bauto)
	{
		LoadCurve* plc = pstep->GetMustPointLoadCurve();
		el.name("time_stepper");
		m_xml.add_branch(el);
		{
			m_xml.add_leaf("dtmin", ops.dtmin);

			el.name("dtmax");
			if (ops.bmust && plc)
				el.add_attribute("lc", plc->GetID());
			else el.value(ops.dtmax);
			m_xml.add_leaf(el);

			m_xml.add_leaf("max_retries", ops.mxback);
			m_xml.add_leaf("opt_iter", ops.iteopt);

			if (ops.ncut > 0) m_xml.add_leaf("aggressiveness", ops.ncut);
		}
		m_xml.close_branch();
	}

    if (ops.plot_level != 1)
    {
        const char* sz[] = { "PLOT_NEVER", "PLOT_MAJOR_ITRS", "PLOT_MINOR_ITRS", "PLOT_MUST_POINTS", "PLOT_FINAL", "PLOT_AUGMENTATIONS", "PLOT_STEP_FINAL" };
        m_xml.add_leaf("plot_level", sz[ops.plot_level]);
    }

	if (ops.plot_stride != 1)
	{
		m_xml.add_leaf("plot_stride", ops.plot_stride);
	}
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteReactionDiffusionControlParams(FSAnalysisStep* pstep)
{
	XMLElement el;
	STEP_SETTINGS& ops = pstep->GetSettings();

	m_xml.add_leaf("analysis", (ops.nanalysis == 0 ? "STEADY-STATE" : "TRANSIENT"));
	m_xml.add_leaf("time_steps", ops.ntime);
	m_xml.add_leaf("step_size", ops.dt);

	m_xml.add_branch("solver");
	{
		m_xml.add_leaf("max_refs", ops.maxref);
		m_xml.add_leaf("max_ups", ops.ilimit);

		// write the parameters
		WriteParamList(*pstep);

		if (ops.nmatfmt != 0)
		{
			m_xml.add_leaf("symmetric_stiffness", (ops.nmatfmt == 1 ? 1 : 0));
		}
	}
	m_xml.close_branch();


	if (ops.bauto)
	{
		LoadCurve* plc = pstep->GetMustPointLoadCurve();
		el.name("time_stepper");
		m_xml.add_branch(el);
		{
			m_xml.add_leaf("dtmin", ops.dtmin);

			el.name("dtmax");
			if (ops.bmust && plc)
				el.add_attribute("lc", plc->GetID());
			else el.value(ops.dtmax);
			m_xml.add_leaf(el);

			m_xml.add_leaf("max_retries", ops.mxback);
			m_xml.add_leaf("opt_iter", ops.iteopt);

			if (ops.ncut > 0) m_xml.add_leaf("aggressiveness", ops.ncut);
		}
		m_xml.close_branch();
	}

    if (ops.plot_level != 1)
    {
        const char* sz[] = { "PLOT_NEVER", "PLOT_MAJOR_ITRS", "PLOT_MINOR_ITRS", "PLOT_MUST_POINTS", "PLOT_FINAL", "PLOT_AUGMENTATIONS", "PLOT_STEP_FINAL" };
        m_xml.add_leaf("plot_level", sz[ops.plot_level]);
    }

	if (ops.plot_stride != 1)
	{
		m_xml.add_leaf("plot_stride", ops.plot_stride);
	}
}

//-----------------------------------------------------------------------------
void FEBioExport3::WritePolarFluidControlParams(FSAnalysisStep* pstep)
{
    XMLElement el;
    STEP_SETTINGS& ops = pstep->GetSettings();
    
    m_xml.add_leaf("analysis", (ops.nanalysis == 0 ? "STEADY-STATE" : "DYNAMIC"));
    m_xml.add_leaf("time_steps", ops.ntime);
    m_xml.add_leaf("step_size", ops.dt);
    
    m_xml.add_branch("solver");
    {
        m_xml.add_leaf("max_refs", ops.maxref);
        m_xml.add_leaf("max_ups", ops.ilimit);
        m_xml.add_leaf("diverge_reform", ops.bdivref);
        m_xml.add_leaf("reform_each_time_step", ops.brefstep);
        
        // write the parameters
        WriteParamList(*pstep);
        
        if (ops.bminbw)
        {
            m_xml.add_leaf("optimize_bw", 1);
        }
        
        if (ops.nmatfmt != 0)
        {
            m_xml.add_leaf("symmetric_stiffness", (ops.nmatfmt == 1 ? 1 : 0));
        }
    }
    m_xml.close_branch();
    
    if (ops.bauto)
    {
        LoadCurve* plc = pstep->GetMustPointLoadCurve();
        el.name("time_stepper");
        m_xml.add_branch(el);
        {
            m_xml.add_leaf("dtmin", ops.dtmin);
            
            el.name("dtmax");
            if (ops.bmust && plc)
                el.add_attribute("lc", plc->GetID());
            else el.value(ops.dtmax);
            m_xml.add_leaf(el);
            
            m_xml.add_leaf("max_retries", ops.mxback);
            m_xml.add_leaf("opt_iter", ops.iteopt);
            
            if (ops.ncut > 0) m_xml.add_leaf("aggressiveness", ops.ncut);
        }
        m_xml.close_branch();
    }
    
    if (ops.plot_level != 1)
    {
        const char* sz[] = { "PLOT_NEVER", "PLOT_MAJOR_ITRS", "PLOT_MINOR_ITRS", "PLOT_MUST_POINTS", "PLOT_FINAL", "PLOT_AUGMENTATIONS", "PLOT_STEP_FINAL" };
        m_xml.add_leaf("plot_level", sz[ops.plot_level]);
    }
    
    if (ops.plot_stride != 1)
    {
        m_xml.add_leaf("plot_stride", ops.plot_stride);
    }
}

//-----------------------------------------------------------------------------

void FEBioExport3::WriteMaterialSection()
{
	XMLElement el;

	FSModel& s = *m_pfem;

	for (int i = 0; i<s.Materials(); ++i)
	{
		GMaterial* pgm = s.GetMaterial(i);

		if (m_writeNotes) WriteNote(pgm);

		const string& name = pgm->GetName();

		el.name("material");
		el.add_attribute("id", pgm->m_ntag);
		el.add_attribute("name", name.c_str());

		FSMaterial* pmat = pgm->GetMaterialProperties();
		if (pmat)
		{
			if (pmat->Type() == FE_RIGID_MATERIAL) WriteRigidMaterial(pmat, el);
			else WriteMaterial(pmat, el);
		}
		else
		{
			errf("ERROR: Material %s does not have any properties.", name.c_str());
			m_xml.add_leaf(el);
		}
	}
}

void FEBioExport3::WriteFiberMaterial(FSOldFiberMaterial& fiber)
{
	FSOldFiberMaterial& f = fiber;
	XMLElement el;
	el.name("fiber");
	if (f.m_naopt == FE_FIBER_LOCAL)
	{
		el.add_attribute("type", "local");
		el.value(f.m_n, 2);
		m_xml.add_leaf(el);
	}
	else if (f.m_naopt == FE_FIBER_CYLINDRICAL)
	{
		el.add_attribute("type", "cylindrical");
		m_xml.add_branch(el);
		{
			m_xml.add_leaf("center", f.m_r);
			m_xml.add_leaf("axis", f.m_a);
			m_xml.add_leaf("vector", f.m_d);
		}
		m_xml.close_branch();
	}
	else if (f.m_naopt == FE_FIBER_POLAR)
	{
		el.add_attribute("type", "polar");
		m_xml.add_branch(el);
		{
			m_xml.add_leaf("center", f.m_r);
			m_xml.add_leaf("axis", f.m_a);
			m_xml.add_leaf("radius1", f.m_R0);
			m_xml.add_leaf("vector1", f.m_d0);
			m_xml.add_leaf("radius2", f.m_R1);
			m_xml.add_leaf("vector2", f.m_d1);
		}
		m_xml.close_branch();
	}
	else if (f.m_naopt == FE_FIBER_SPHERICAL)
	{
		el.add_attribute("type", "spherical");
		m_xml.add_branch(el);
		{
			m_xml.add_leaf("center", f.m_r);
			m_xml.add_leaf("vector", f.m_d);
		}
		m_xml.close_branch();
	}
	else if (f.m_naopt == FE_FIBER_VECTOR)
	{
		el.add_attribute("type", "vector");
		el.value(f.m_a);
		m_xml.add_leaf(el);
	}
	else if (f.m_naopt == FE_FIBER_USER)
	{
		el.add_attribute("type", "user");
		m_xml.add_leaf(el);
	}
	else if (f.m_naopt == FE_FIBER_ANGLES)
	{
		el.add_attribute("type", "angles");
		m_xml.add_branch(el);
		{
			m_xml.add_leaf("theta", f.m_theta);
			m_xml.add_leaf("phi", f.m_phi);
		}
		m_xml.close_branch();
	}
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteMaterialParams(FSMaterial* pm, bool isTopLevel)
{
	// only export non-persistent parameters for top-level materials
	m_exportNonPersistentParams = isTopLevel;

	// Write the parameters first
	WriteParamList(*pm);

	// reset flag
	m_exportNonPersistentParams = true;

	// if the material is transversely-isotropic, we need to write the fiber data as well
	FSTransverselyIsotropic* ptiso = dynamic_cast<FSTransverselyIsotropic*>(pm);
	if (ptiso)
	{
		FSOldFiberMaterial& f = *(ptiso->GetFiberMaterial());
		WriteFiberMaterial(f);
	}

	// write the material axes (if any)
	if (pm->m_axes && (pm->m_axes->m_naopt > -1))
	{
		XMLElement el("mat_axis");
		XMLElement::intFormat = "%d";
		if (pm->m_axes->m_naopt == FE_AXES_LOCAL)
		{
			el.add_attribute("type", "local");
			el.value(pm->m_axes->m_n, 3);
			m_xml.add_leaf(el);
		}
		else if (pm->m_axes->m_naopt == FE_AXES_VECTOR)
		{
			el.add_attribute("type", "vector");
			m_xml.add_branch(el);
			{
				m_xml.add_leaf("a", pm->m_axes->m_a);
				m_xml.add_leaf("d", pm->m_axes->m_d);
			}
			m_xml.close_branch();
		}
        else if (pm->m_axes->m_naopt == FE_AXES_ANGLES)
        {
            el.add_attribute("type", "angles");
            m_xml.add_branch(el);
            {
                m_xml.add_leaf("theta", pm->m_axes->m_theta);
                m_xml.add_leaf("phi", pm->m_axes->m_phi);
            }
            m_xml.close_branch();
        }
		else if (pm->m_axes->m_naopt == FE_AXES_CYLINDRICAL)
		{
			el.add_attribute("type", "cylindrical");
			m_xml.add_branch(el);
			{
				m_xml.add_leaf("center", pm->m_axes->m_center);
				m_xml.add_leaf("axis", pm->m_axes->m_axis);
				m_xml.add_leaf("vector", pm->m_axes->m_vec);
			}
			m_xml.close_branch();
		}
		else if (pm->m_axes->m_naopt == FE_AXES_SPHERICAL)
		{
			el.add_attribute("type", "spherical");
			m_xml.add_branch(el);
			{
				m_xml.add_leaf("center", pm->m_axes->m_center);
				m_xml.add_leaf("vector", pm->m_axes->m_vec);
			}
			m_xml.close_branch();
		}
		XMLElement::setDefaultFormats();
	}
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteRigidMaterial(FSMaterial* pmat, XMLElement& el)
{
	FSModel& s = *m_pfem;

	FSRigidMaterial* pm = dynamic_cast<FSRigidMaterial*> (pmat);
	el.add_attribute("type", "rigid body");
	m_xml.add_branch(el);
	{
		m_xml.add_leaf("density", pm->GetFloatValue(FSRigidMaterial::MP_DENSITY));

		if (pm->GetBoolValue(FSRigidMaterial::MP_COM) == false)
		{
			vec3d v = pm->GetParam(FSRigidMaterial::MP_RC).GetVec3dValue();
			m_xml.add_leaf("center_of_mass", v);
		}

        if (pm->GetFloatValue(FSRigidMaterial::MP_E) != 0)
        {
            m_xml.add_leaf("E", pm->GetFloatValue(FSRigidMaterial::MP_E));
            m_xml.add_leaf("v", pm->GetFloatValue(FSRigidMaterial::MP_V));
        }
        
		if (pm->m_pid != -1)
		{
			GMaterial* ppm = s.GetMaterialFromID(pm->m_pid);
			assert(ppm);
			m_xml.add_leaf("parent_id", ppm->m_ntag);
		}
	}
	m_xml.close_branch();
}

//-----------------------------------------------------------------------------
void WriteAttributeParameters(XMLElement& el, FSMaterial* pm, FSModel& fem)
{
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
				if (p.GetEnumNames())
				{
					int v = fem.GetEnumValue(p);
					el.add_attribute(p.GetShortName(), v);
				}
				else el.add_attribute(p.GetShortName(), p.GetIntValue());
			}
			break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteMaterial(FSMaterial* pm, XMLElement& el)
{
	// redirect chemical reactions
	if ((pm->Type() == FE_MASS_ACTION_FORWARD) ||
		(pm->Type() == FE_MASS_ACTION_REVERSIBLE) ||
		(pm->Type() == FE_MICHAELIS_MENTEN))
	{
		if (m_useReactionMaterial2) WriteReactionMaterial2(pm, el);
		else WriteReactionMaterial(pm, el);
		return;
	}
    // redirect membrane reactions
    if ((pm->Type() == FE_MMASS_ACTION_FORWARD) ||
        (pm->Type() == FE_MMASS_ACTION_REVERSIBLE))
    {
        WriteMembraneReactionMaterial(pm, el);
        return;
    }

	// get the type string    
	const char* sztype = pm->GetTypeString();
	assert(sztype);

	// set the type attribute
	if (pm->Type() == FE_SOLUTE_MATERIAL)
	{
		FSSoluteMaterial* psm = dynamic_cast<FSSoluteMaterial*>(pm); assert(psm);
		el.add_attribute("sol", psm->GetSoluteIndex() + 1);
	}
	else if (pm->Type() == FE_SBM_MATERIAL)
	{
		FSSBMMaterial* psb = dynamic_cast<FSSBMMaterial*>(pm); assert(psb);
		el.add_attribute("sbm", psb->GetSBMIndex() + 1);
	}
	else if (pm->Type() == FE_SPECIES_MATERIAL)
	{
		FSSpeciesMaterial* psm = dynamic_cast<FSSpeciesMaterial*>(pm); assert(psm);

		int nsol = psm->GetSpeciesIndex();
		if (nsol >= 0)
		{
			FSModel& fem = *m_pfem;
			SoluteData& solute = fem.GetSoluteData(nsol);
			el.add_attribute("name", solute.GetName());
		}
	}
	else if (pm->Type() == FE_SOLID_SPECIES_MATERIAL)
	{
		FSSolidSpeciesMaterial* psm = dynamic_cast<FSSolidSpeciesMaterial*>(pm); assert(psm);
		int nsbm = psm->GetSBMIndex();
		if (nsbm >= 0)
		{
			FSModel& fem = *m_pfem;
			SoluteData& sbm = fem.GetSBMData(nsbm);
			el.add_attribute("name", sbm.GetName());
		}
	}
	else if (pm->Type() == FE_REACTANT_MATERIAL)
	{
		FSReactantMaterial* psb = dynamic_cast<FSReactantMaterial*>(pm); assert(psb);
		int idx = psb->GetIndex();
		int type = psb->GetReactantType();
		el.value(psb->GetCoef());
		switch (type)
		{
		case FSReactionSpecies::SOLUTE_SPECIES: el.add_attribute("sol", idx + 1); break;
		case FSReactionSpecies::SBM_SPECIES: el.add_attribute("sbm", idx + 1); break;
		default:
			assert(false);
		}
		m_xml.add_leaf(el);
		return;
	}
	else if (pm->Type() == FE_PRODUCT_MATERIAL)
	{
		FSProductMaterial* psb = dynamic_cast<FSProductMaterial*>(pm); assert(psb);
		int idx = psb->GetIndex();
		int type = psb->GetProductType();
		el.value(psb->GetCoef());
		switch (type)
		{
		case FSReactionSpecies::SOLUTE_SPECIES: el.add_attribute("sol", idx + 1); break;
		case FSReactionSpecies::SBM_SPECIES: el.add_attribute("sbm", idx + 1); break;
		default:
			assert(false);
		}
		m_xml.add_leaf(el);
		return;
	}
    else if (pm->Type() == FE_INT_REACTANT_MATERIAL)
    {
        FSInternalReactantMaterial* psb = dynamic_cast<FSInternalReactantMaterial*>(pm); assert(psb);
        int idx = psb->GetIndex();
        int type = psb->GetReactantType();
        el.value(psb->GetCoef());
        switch (type)
        {
            case FSReactionSpecies::SOLUTE_SPECIES: el.add_attribute("sol", idx + 1); break;
            default:
                assert(false);
        }
        m_xml.add_leaf(el);
        return;
    }
    else if (pm->Type() == FE_INT_PRODUCT_MATERIAL)
    {
        FSInternalProductMaterial* psb = dynamic_cast<FSInternalProductMaterial*>(pm); assert(psb);
        int idx = psb->GetIndex();
        int type = psb->GetProductType();
        el.value(psb->GetCoef());
        switch (type)
        {
            case FSReactionSpecies::SOLUTE_SPECIES: el.add_attribute("sol", idx + 1); break;
            default:
                assert(false);
        }
        m_xml.add_leaf(el);
        return;
    }
    else if (pm->Type() == FE_EXT_REACTANT_MATERIAL)
    {
        FSExternalReactantMaterial* psb = dynamic_cast<FSExternalReactantMaterial*>(pm); assert(psb);
        int idx = psb->GetIndex();
        int type = psb->GetReactantType();
        el.value(psb->GetCoef());
        switch (type)
        {
            case FSReactionSpecies::SOLUTE_SPECIES: el.add_attribute("sol", idx + 1); break;
            default:
                assert(false);
        }
        m_xml.add_leaf(el);
        return;
    }
    else if (pm->Type() == FE_EXT_PRODUCT_MATERIAL)
    {
        FSExternalProductMaterial* psb = dynamic_cast<FSExternalProductMaterial*>(pm); assert(psb);
        int idx = psb->GetIndex();
        int type = psb->GetProductType();
        el.value(psb->GetCoef());
        switch (type)
        {
            case FSReactionSpecies::SOLUTE_SPECIES: el.add_attribute("sol", idx + 1); break;
            default:
                assert(false);
        }
        m_xml.add_leaf(el);
        return;
    }
    else if (pm->Type() == FE_OSMO_WM)
    {
        FSOsmoWellsManning* pwm = dynamic_cast<FSOsmoWellsManning*>(pm); assert(pwm);
        el.add_attribute("co_ion", pwm->GetCoIonIndex()+1);
        m_xml.add_leaf(el);
        return;
    }
	else if (pm->Type() == FE_FNC1D_POINT)
	{
		FS1DPointFunction* pf1d = dynamic_cast<FS1DPointFunction*>(pm); assert(pf1d);
		LoadCurve* plc = pf1d->GetPointCurve();
		el.add_attribute("type", sztype);
		m_xml.add_branch(el);
		{
			if (plc)
			{
				m_xml.add_branch("points");
				int n = plc->Points();
				for (int i = 0; i < n; ++i)
				{
					vec2d p = plc->Point(i);
					double d[2] = { p.x(), p.y() };
					m_xml.add_leaf("pt", d, 2);
				}
				m_xml.close_branch();
			}
		}
		m_xml.close_branch();
		return;
	}
	else
	{
		if (strcmp(el.name(), sztype) != 0)
			el.add_attribute("type", sztype);
	}

	// see if there are any attribute parameters
	FSModel& fem = m_prj.GetFSModel();
	WriteAttributeParameters(el, pm, fem);

	m_xml.add_branch(el);
	{
		// write the material parameters (if any)
		if ((pm->Parameters() || (pm->m_axes != nullptr)))
		{
			// NOTE: This is a little hack, but if the parent material is 
			// a multi-material then we treat this property as top-level
			// so that non-persistent properties are written
			const FSMaterial* parentMat = pm->GetParentMaterial();
			bool topLevel = (parentMat == nullptr) || (dynamic_cast<const FSMultiMaterial*>(parentMat) != nullptr);
			WriteMaterialParams(pm, topLevel);
		}

		// write the components
		int NC = (int)pm->Properties();
		for (int i = 0; i<NC; ++i)
		{
			FSProperty& mc = pm->GetProperty(i);
			for (int j = 0; j<mc.Size(); ++j)
			{
				FSMaterial* pc = pm->GetMaterialProperty(i, j);
				if (pc)
				{
					el.name(mc.GetName().c_str());
					const string& name = pc->GetName();
					if (name.empty() == false) el.add_attribute("name", name.c_str());

					// TODO: some materials need to be treated as multi-materials
					//       although they technically aren't. I need to simplify this.
					bool is_multi = false;
					switch (pc->Type())
					{
					case FE_SBM_MATERIAL: is_multi = true; break;
					case FE_REACTANT_MATERIAL: is_multi = true; break;
					case FE_PRODUCT_MATERIAL: is_multi = true; break;
					case FE_SPECIES_MATERIAL: is_multi = true; break;
					case FE_SOLID_SPECIES_MATERIAL: is_multi = true; break;
					case FE_FNC1D_POINT: is_multi = true; break;
					}

					if ((pc->Properties() > 0) || is_multi) WriteMaterial(pc, el);
					else
					{
						// only add type if the string is different than the tag name
						const char* sztype2 = pc->GetTypeString();
						if (strcmp(el.name(), sztype2) != 0)
							el.add_attribute("type", sztype2);

						// write any attribute parameters
						WriteAttributeParameters(el, pc, fem);

						// We need some special formatting for some fiber generator materials
						bool bdone = false;
						if (pc->Parameters() == 1)
						{
							const char* sztype = pc->GetTypeString();
							Param& p = pc->GetParam(0);
							if (sztype && (strcmp(p.GetShortName(), sztype) == 0))
							{
								if (p.GetParamType() == Param_VEC2I)
								{
									el.value(p.GetVec2iValue());
									m_xml.add_leaf(el);
								}
								else if (p.GetParamType() == Param_VEC3D)
								{
									el.value(p.GetVec3dValue());
									m_xml.add_leaf(el);
								}
								else if (p.GetParamType() == Param_MAT3D)
								{
									el.value(p.GetMat3dValue());
									m_xml.add_leaf(el);
								}
								bdone = true;
							}
						}

						if (bdone == false)
						{
							if ((pc->Parameters() > 0) || ((pc->m_axes != nullptr) && (pc->m_axes->m_naopt != -1)))
							{
								m_xml.add_branch(el);
								{
									// NOTE: This is a little hack, but if the parent material is 
									// a multi-material then we treat this property as top-level
									// so that non-persistent properties are written
									bool topLevel = (dynamic_cast<FSMultiMaterial*>(pm) != nullptr);
									WriteMaterialParams(pc, topLevel);
								}
								m_xml.close_branch();
							}
							else m_xml.add_empty(el);
						}
					}
				}
			}
		}
	}
	m_xml.close_branch();
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteReactionMaterial(FSMaterial* pmat, XMLElement& el)
{
	const char* sztype = 0;
	switch (pmat->Type())
	{
	case FE_MASS_ACTION_FORWARD: sztype = "mass-action-forward"; break;
	case FE_MASS_ACTION_REVERSIBLE: sztype = "mass-action-reversible"; break;
	case FE_MICHAELIS_MENTEN: sztype = "Michaelis-Menten"; break;
	default:
		assert(false);
		return;
	}

	el.add_attribute("type", sztype);

	m_xml.add_branch(el);
	{
		// write the material parameters (if any)
		if (pmat->Parameters()) WriteMaterialParams(pmat);

		// write the components
		int NC = (int)pmat->Properties();
		for (int i = 0; i<NC; ++i)
		{
			FSProperty& mc = pmat->GetProperty(i);
			for (int j = 0; j<mc.Size(); ++j)
			{
				FSMaterial* pc = pmat->GetMaterialProperty(i, j);
				if (pc)
				{
					el.name(mc.GetName().c_str());
					const string& name = pc->GetName();
					if (name.empty() == false) el.add_attribute("name", name.c_str());

					bool is_multi = false;
					switch (pc->Type())
					{
					case FE_SBM_MATERIAL: is_multi = true; break;
					case FE_REACTANT_MATERIAL: is_multi = true; break;
					case FE_PRODUCT_MATERIAL: is_multi = true; break;
					case FE_SPECIES_MATERIAL: is_multi = true; break;
					case FE_SOLID_SPECIES_MATERIAL: is_multi = true; break;
					}

					if ((pc->Properties() > 0) || is_multi) WriteMaterial(pc, el);
					else
					{
						el.add_attribute("type", pc->GetTypeString());
						m_xml.add_branch(el);
						{
							WriteMaterialParams(pc);
						}
						m_xml.close_branch();
					}
				}
			}
		}
	}
	m_xml.close_branch();
}

//-----------------------------------------------------------------------------
// This is the format used by the reaction-diffusion module
void FEBioExport3::WriteReactionMaterial2(FSMaterial* pmat, XMLElement& el)
{
	// get the reaction material
	FSReactionMaterial* prm = dynamic_cast<FSReactionMaterial*>(pmat);
	assert(prm);
	if (prm == 0) return;

	// get the reaction type
	const char* sztype = 0;
	switch (pmat->Type())
	{
	case FE_MASS_ACTION_FORWARD: sztype = "mass action"; break;
	case FE_MICHAELIS_MENTEN: sztype = "Michaelis-Menten"; break;
	default:
		assert(false);
		return;
	}

	XMLElement r("reaction");
	r.add_attribute("type", sztype);
	m_xml.add_branch(r);

	FSModel& fem = *m_pfem;

	if (pmat->Type() == FE_MASS_ACTION_FORWARD)
	{
		// build the reaction equation from the product and reactant list
		stringstream ss;

		int nreact = prm->Reactants();
		for (int i = 0; i<nreact; ++i)
		{
			FSReactantMaterial* ri = prm->Reactant(i);

			const char* sz = 0;
			int v = ri->GetCoef();
			int idx = ri->GetIndex();
			int type = ri->GetReactantType();
			if (type == FSReactionSpecies::SOLUTE_SPECIES) sz = fem.GetSoluteData(idx).GetName().c_str();
			else if (type == FSReactionSpecies::SBM_SPECIES) sz = fem.GetSBMData(idx).GetName().c_str();
			else { assert(false); }

			if (v != 1) ss << v << "*" << sz;
			else ss << sz;
			if (i != nreact - 1) ss << "+";
		}

		ss << "->";

		int nprod = prm->Products();
		for (int i = 0; i<nprod; ++i)
		{
			FSProductMaterial* pi = prm->Product(i);

			const char* sz = 0;
			int v = pi->GetCoef();
			int idx = pi->GetIndex();
			int type = pi->GetProductType();
			if (type == FSReactionSpecies::SOLUTE_SPECIES) sz = fem.GetSoluteData(idx).GetName().c_str();
			else if (type == FSReactionSpecies::SBM_SPECIES) sz = fem.GetSBMData(idx).GetName().c_str();
			else { assert(false); }

			if (v != 1) ss << v << "*" << sz;
			else ss << sz;
			if (i != nreact - 1) ss << "+";
		}

		string s = ss.str();

		// write the reaction
		m_xml.add_leaf("equation", s.c_str());

		FSReactionRateConst* rate = dynamic_cast<FSReactionRateConst*>(prm->GetForwardRate());
		if (rate)
		{
			m_xml.add_leaf("rate_constant", rate->GetRateConstant());
		}
	}
	else if (pmat->Type() == FE_MICHAELIS_MENTEN)
	{
		assert(false);
	}

	m_xml.close_branch();
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteMembraneReactionMaterial(FSMaterial* pmat, XMLElement& el)
{
    const char* sztype = 0;
    switch (pmat->Type())
    {
        case FE_MMASS_ACTION_FORWARD: sztype = "membrane-mass-action-forward"; break;
        case FE_MMASS_ACTION_REVERSIBLE: sztype = "membrane-mass-action-reversible"; break;
        default:
            assert(false);
            return;
    }
    
    el.add_attribute("type", sztype);
    
    m_xml.add_branch(el);
    {
        // write the material parameters (if any)
        if (pmat->Parameters()) WriteMaterialParams(pmat);
        
        // write the components
        int NC = (int)pmat->Properties();
        for (int i = 0; i<NC; ++i)
        {
            FSProperty& mc = pmat->GetProperty(i);
            for (int j = 0; j<mc.Size(); ++j)
            {
				FSMaterial* pc = pmat->GetMaterialProperty(i, j);
                if (pc)
                {
                    el.name(mc.GetName().c_str());
                    const string& name = pc->GetName();
                    if (name.empty() == false) el.add_attribute("name", name.c_str());
                    
                    bool is_multi = false;
                    switch (pc->Type())
                    {
                        case FE_SBM_MATERIAL: is_multi = true; break;
                        case FE_REACTANT_MATERIAL: is_multi = true; break;
                        case FE_PRODUCT_MATERIAL: is_multi = true; break;
                        case FE_INT_REACTANT_MATERIAL: is_multi = true; break;
                        case FE_INT_PRODUCT_MATERIAL: is_multi = true; break;
                        case FE_EXT_REACTANT_MATERIAL: is_multi = true; break;
                        case FE_EXT_PRODUCT_MATERIAL: is_multi = true; break;
                        case FE_SPECIES_MATERIAL: is_multi = true; break;
                        case FE_SOLID_SPECIES_MATERIAL: is_multi = true; break;
                    }
                    
                    if ((pc->Properties() > 0) || is_multi) WriteMaterial(pc, el);
                    else
                    {
                        el.add_attribute("type", pc->GetTypeString());
                        m_xml.add_branch(el);
                        {
                            WriteMaterialParams(pc);
                        }
                        m_xml.close_branch();
                    }
                }
            }
        }
    }
    m_xml.close_branch();
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteGeometrySection()
{
	if (m_exportParts)
		WriteGeometrySectionNew();
	else
		WriteGeometrySectionOld();
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteGeometrySectionOld()
{
	// export the nodes
	WriteGeometryNodes();

	// Write the elements
	WriteGeometryElements();

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
void FEBioExport3::WriteMeshSection()
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
void FEBioExport3::WriteMeshDomainsSection()
{
	Part* part = m_Part[0];
	for (int i = 0; i < part->m_Dom.size(); ++i)
	{
		Domain* dom = part->m_Dom[i];

		XMLElement el;
		if (dom->m_elemClass == ELEM_SOLID)
		{
            GPart* pg = dom->m_pg;
			el.name("SolidDomain");
			el.add_attribute("name", dom->m_name);
			el.add_attribute("mat", dom->m_matName);
            if (pg && pg->Parameters())
            {
                // for solid domains, hide the shell node normal state
                pg->GetParam(0).SetState(0);
                if (pg->GetParam(1).GetBoolValue() == false)
                    m_xml.add_empty(el);
                else {
                    m_xml.add_branch(el);
                    {
                        WriteParamList(*pg);
                    }
                    m_xml.close_branch();
                }
            }
            else m_xml.add_empty(el);
		}
		else if (dom->m_elemClass == ELEM_SHELL)
		{
			GPart* pg = dom->m_pg;
			el.name("ShellDomain");
			el.add_attribute("name", dom->m_name);
			el.add_attribute("mat", dom->m_matName);

			if (pg && pg->Parameters())
			{
                // if there is no incompressibility constraint, hide parameters
                if (pg->GetParam(1).GetBoolValue() == false) {
                    pg->GetParam(1).SetState(0);
                    pg->GetParam(2).SetState(0);
                }
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
void FEBioExport3::WriteGeometrySectionNew()
{
	FSModel& fem = *m_pfem;
	GModel& model = fem.GetModel();

	// reset counters
	m_ntotnodes = 0;
	m_ntotelem = 0;

	// write all parts
	auto nparts = m_Part.size();
	for (int i = 0; i<nparts; ++i)
	{
		Part* p = m_Part[i];
		const string& name = p->m_obj->GetName();

		// make sure this part does not have any domains yet
		assert(p->m_Dom.empty());

		XMLElement tag("Part");
		tag.add_attribute("name", name.c_str());
		m_xml.add_branch(tag);
		{
			WriteGeometryObject(p);
		}
		m_xml.close_branch();
	}

	// write all instances
	for (int i = 0; i<nparts; ++i)
	{
		Part* part = m_Part[i];
		GObject* po = part->m_obj;
		const string& name = po->GetName();
		XMLElement instance("Instance");
		instance.add_attribute("part", name.c_str());
		m_xml.add_branch(instance);
		{
			vec3d p = po->GetTransform().GetPosition();
			quatd q = po->GetTransform().GetRotation();
			vec3d s = po->GetTransform().GetScale();
			m_xml.add_branch("transform");
			{
				m_xml.add_leaf("scale", s);
				m_xml.add_leaf("rotate", q);
				m_xml.add_leaf("translate", p);
			}
			m_xml.close_branch();

			// write all material assignments
			int ndom = (int)part->m_Dom.size();
			for (int j = 0; j < ndom; ++j)
			{
				FEBioExport3::Domain& dom = *part->m_Dom[j];
				// get the material
				XMLElement domain("domain");
				domain.add_attribute("name", dom.m_name);
				domain.add_attribute("mat", dom.m_matName);
				m_xml.add_empty(domain);
			}
		}
		m_xml.close_branch();
	}

	// write the global node sets
	WriteGeometryNodeSetsNew();

	// write the global surfaces
	WriteGeometrySurfacesNew();

	// write the global element sets
	WriteGeometryElementSetsNew();

	// write the surface pairs
	WriteGeometrySurfacePairs();

	// write discrete sets
	WriteGeometryDiscreteSets();
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteGeometryObject(FEBioExport3::Part* part)
{
	GObject* po = part->m_obj;

	// get the mesh
	FSCoreMesh* pm = po->GetFEMesh();

	// Write the nodes
	m_xml.add_branch("Nodes");
	{
		XMLElement el("node");
		int nid = el.add_attribute("id", 0);
		for (int j = 0; j<pm->Nodes(); ++j)
		{
			FSNode& node = pm->Node(j);
			el.set_attribute(nid, ++m_ntotnodes);
			vec3d r = node.r;
			el.value(r);
			m_xml.add_leaf(el, false);
		}
	}
	m_xml.close_branch();

	// write all elements
	int NP = po->Parts();
	for (int p = 0; p<NP; ++p)
	{
		// get the part
		GPart* pg = po->Part(p);

		// write this part
		WriteGeometryPart(part, pg, false);
	}

	// write all node sets
	for (int i = 0; i<part->m_NSet.size(); ++i)
	{
		NodeSet* ns = part->m_NSet[i];
		WriteNodeSet(ns->m_name.c_str(), ns->m_nodeList);
	}

	// write all surfaces
	for (int i = 0; i<part->m_Surf.size(); ++i)
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
			WriteElementList(*es->m_elemList);
		}
		m_xml.close_branch();
	}
}

//-----------------------------------------------------------------------------
bool FEBioExport3::WriteNodeSet(const string& name, FSNodeList* pl)
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
	FSNodeList::Iterator pn = pl->First();
	vector<int> m(nn);
	for (int n = 0; n<nn; ++n, pn++)
	{
		FSNode* pnode = pn->m_pi;
		if (pnode == 0) return false;
		m[n] = pnode->m_nid;
	}

	XMLElement el("NodeSet");
	el.add_attribute("name", name.c_str());
	m_xml.add_branch(el);
	{
		XMLElement nd("n");
		nd.add_attribute("id", 0);
		for (int n = 0; n<nn; ++n)
		{
			nd.set_attribute(0, m[n]);
			m_xml.add_empty(nd, false);
		}
	}
	m_xml.close_branch();
	return true;
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteGeometryNodeSetsNew()
{
	// Write the BC node sets
	int NS = (int)m_pNSet.size();
	for (int i = 0; i<NS; ++i)
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
			for (size_t i = 0; i<nodeList.size(); ++i)
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
			for (size_t i = 0; i<edgeList.size(); ++i)
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
			for (size_t i = 0; i<faceList.size(); ++i)
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
			for (size_t i = 0; i<partList.size(); ++i)
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
			for (int i = 0; i<m_Part.size(); ++i)
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
			for (int i = 0; i<m_Part.size(); ++i)
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
void FEBioExport3::WriteGeometryNodeSets()
{
	// Write the BC node sets
	int NS = (int)m_pNSet.size();
	for (int i = 0; i<NS; ++i)
	{
		if (m_pNSet[i].m_duplicate == false)
		{
			FEItemListBuilder* pil = m_pNSet[i].m_list;
			unique_ptr<FSNodeList> pl(pil->BuildNodeList());
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
	for (int i = 0; i<model.NodeLists(); ++i)
	{
		GNodeList* pg = model.NodeList(i);
		unique_ptr<FSNodeList> pn(pg->BuildNodeList());
		if (WriteNodeSet(pg->GetName(), pn.get()) == false)
		{
			throw InvalidItemListBuilder(pg);
		}
	}

	// Then, do object-level node sets
	int nobj = model.Objects();
	for (int i = 0; i<nobj; ++i)
	{
		GObject* po = model.Object(i);
		FSMesh* pm = po->GetFEMesh();
		if (pm)
		{
			int nset = po->FENodeSets();
			for (int j = 0; j<nset; ++j)
			{
				FSNodeSet* pns = po->GetFENodeSet(j);
				unique_ptr<FSNodeList> pl(pns->BuildNodeList());
				if (WriteNodeSet(pns->GetName(), pl.get()) == false)
				{
					throw InvalidItemListBuilder(po);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteGeometrySurfaces()
{
	int NS = (int)m_pSurf.size();
	for (int i = 0; i<NS; ++i)
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
void FEBioExport3::WriteGeometryElementSets()
{
	int NS = (int)m_pESet.size();
	for (int i = 0; i<NS; ++i)
	{
		FEItemListBuilder* pl = m_pESet[i].m_list;
		unique_ptr<FEElemList> ps(pl->BuildElemList());
		XMLElement el("ElementSet");
		el.add_attribute("name", m_pESet[i].m_name.c_str());
		m_xml.add_branch(el);
		{
			WriteElementList(*ps);
		}
		m_xml.close_branch();
	}
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteGeometrySurfacesNew()
{
	int NS = (int)m_pSurf.size();
	for (int i = 0; i<NS; ++i)
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
				for (size_t i = 0; i<faceList.size(); ++i)
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
void FEBioExport3::WriteGeometryElementSetsNew()
{
	int NS = (int)m_pESet.size();
	for (int i = 0; i<NS; ++i)
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
void FEBioExport3::WriteGeometrySurfacePairs()
{
	// get the named surfaces (paired interfaces)
	FSModel& fem = *m_pfem;
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FSStep* pstep = fem.GetStep(i);
		for (int j = 0; j<pstep->Interfaces(); ++j)
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
void FEBioExport3::WriteGeometryNodes()
{
	FSModel& s = *m_pfem;
	GModel& model = s.GetModel();

	int n = 1;
	for (int i = 0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		FSCoreMesh* pm = po->GetFEMesh();

		XMLElement tagNodes("Nodes");
		const string& name = po->GetName();
		if (name.empty() == false) tagNodes.add_attribute("name", name.c_str());

		m_xml.add_branch(tagNodes);
		{
			XMLElement el("node");
			int nid = el.add_attribute("id", 0);
			for (int j = 0; j<pm->Nodes(); ++j, ++n)
			{
				FSNode& node = pm->Node(j);
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
	for (int i = 0; i<model.DiscreteObjects(); ++i)
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
					for (int j = 0; j<ndivs - 1; ++j)
					{
						double w = (double)(j + 1.0) / (double)ndivs;
						vec3d r = ra + (rb - ra)*w;

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
void FEBioExport3::WriteMeshElements()
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
void FEBioExport3::WriteGeometryElements(bool writeMats, bool useMatNames)
{
	FSModel& s = *m_pfem;
	GModel& model = s.GetModel();

	// reset element counter
	m_ntotelem = 0;

	// loop over all objects
	for (int i = 0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);

		// loop over all parts
		int NP = po->Parts();
		for (int p = 0; p<NP; ++p)
		{
			// get the part
			GPart* pg = po->Part(p);

			// write this part
			WriteGeometryPart(nullptr, pg, writeMats, useMatNames);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteGeometryPart(Part* part, GPart* pg, bool writeMats, bool useMatNames)
{
	FSModel& s = *m_pfem;
	GModel& model = s.GetModel();
	GObject* po = dynamic_cast<GObject*>(pg->Object());
	FSCoreMesh* pm = po->GetFEMesh();
	int pid = pg->GetLocalID();

	// Parts must be split up by element type
	int NE = pm->Elements();
	int NEP = 0; // number of elements in part
	for (int i = 0; i<NE; ++i)
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
	int nn[FSElement::MAX_NODES];
	char szname[128] = { 0 };
	for (int i = 0; ncount<NEP; ++i)
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
			if (m_exportParts) es.m_name = po->GetName() + "." + szname;
			es.m_matID = pg->GetMaterialID();

			// add a domain
			if (part)
			{
				FEBioExport3::Domain* dom = new FEBioExport3::Domain;
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

				for (int j = i; j<NE; ++j)
				{
					FEElement_& ej = pm->ElementRef(j);
					if ((ej.m_ntag == 1) && (ej.Type() == ntype))
					{
						int eid = m_ntotelem + ncount + 1;
						xej.set_attribute(n1, eid);
						int ne = ej.Nodes();
						assert(ne == el.Nodes());
						for (int k = 0; k<ne; ++k) nn[k] = pm->Node(ej.m_node[k]).m_nid;
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
void FEBioExport3::WriteGeometryDiscreteSets()
{
	FSModel& fem = *m_pfem;
	GModel& model = fem.GetModel();

	// write the discrete element sets
	for (int i = 0; i<model.DiscreteObjects(); ++i)
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
				for (int n = 0; n<N; ++n)
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
						for (int j = 0; j<N - 2; ++j)
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
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FSStep* step = fem.GetStep(i);
		for (int j = 0; j<step->Interfaces(); ++j)
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
					for (int n = 0; n<N; ++n)
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
void FEBioExport3::WriteMeshDataSection()
{
	WriteElementDataSection();
	WriteSurfaceDataSection();
	WriteEdgeDataSection();
	WriteNodeDataSection();

	FSModel& fem = *m_pfem;
	int N = fem.MeshDataGenerators();
	for (int i=0; i<N; ++i)
	{
		FSMeshDataGenerator* pdm = fem.GetMeshDataGenerator(i);

		switch (pdm->Type())
		{
		case FE_FEBIO_NODEDATA_GENERATOR: WriteNodeDataGenerator(dynamic_cast<FSNodeDataGenerator*>(pdm)); break;
		case FE_FEBIO_EDGEDATA_GENERATOR: WriteEdgeDataGenerator(dynamic_cast<FSEdgeDataGenerator*>(pdm)); break;
		case FE_FEBIO_FACEDATA_GENERATOR: WriteFaceDataGenerator(dynamic_cast<FSFaceDataGenerator*>(pdm)); break;
		case FE_FEBIO_ELEMDATA_GENERATOR: WriteElemDataGenerator(dynamic_cast<FSElemDataGenerator*>(pdm)); break;
		default:
			assert(false);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteElementDataSection()
{
	WriteMeshDataShellThickness();

	WriteMeshDataMaterialFibers();

	WriteMeshDataMaterialAxes();

	WriteElementDataFields();
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteNodeDataGenerator(FSNodeDataGenerator* map)
{
	XMLElement meshData("NodeData");
	meshData.add_attribute("generator", map->GetTypeString());
	meshData.add_attribute("node_set", GetNodeSetName(map->GetItemList()));
	m_xml.add_branch(meshData);
	{
		WriteParamList(*map);
	}
	m_xml.close_branch();
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteEdgeDataGenerator(FSEdgeDataGenerator* map)
{
	XMLElement meshData("EdgeData");
	meshData.add_attribute("generator", map->GetTypeString());
//	meshData.add_attribute("edge_set", GetEdgeSetName(map->GetItemList()));
	m_xml.add_branch(meshData);
	{
		WriteParamList(*map);
	}
	m_xml.close_branch();
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteFaceDataGenerator(FSFaceDataGenerator* map)
{
	XMLElement meshData("SurfaceData");
	meshData.add_attribute("generator", map->GetTypeString());
	meshData.add_attribute("surface", GetSurfaceName(map->GetItemList()));
	m_xml.add_branch(meshData);
	{
		WriteParamList(*map);
	}
	m_xml.close_branch();
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteElemDataGenerator(FSElemDataGenerator* map)
{
	XMLElement meshData("ElementData");
	meshData.add_attribute("generator", map->GetTypeString());
	meshData.add_attribute("elem_set", GetElementSetName(map->GetItemList()));
	m_xml.add_branch(meshData);
	{
		WriteParamList(*map);
	}
	m_xml.close_branch();
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteMeshDataShellThickness()
{
	FSModel& fem = *m_pfem;
	GModel& model = fem.GetModel();
	for (int i = 0; i<(int)m_ElSet.size(); ++i)
	{
		ElementSet& elset = m_ElSet[i];
		FSCoreMesh* pm = elset.m_mesh;

		// see if this mesh has shells
		bool bshell = false;
		for (int k = 0; k<(int)elset.m_elem.size(); ++k)
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
				for (int k = 0; k<(int)elset.m_elem.size(); ++k)
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
void FEBioExport3::WriteMeshDataMaterialFibers()
{
	FSModel& fem = *m_pfem;

	// loop over all element sets
	auto NSET = m_ElSet.size();
	for (int i = 0; i<NSET; ++i)
	{
		ElementSet& elSet = m_ElSet[i];
		FSCoreMesh* pm = elSet.m_mesh;
		GObject* po = pm->GetGObject();
		const Transform& T = po->GetTransform();

		// get the material properties
		GMaterial* gmat = fem.GetMaterialFromID(elSet.m_matID);
		FSMaterial* pmat = gmat->GetMaterialProperties();

		// see if we should write fibers
		// This is only done if the material specifies the "user" fiber property
		bool writeFibers = false;

		FSProperty* fiberProp = pmat->FindProperty("fiber");
		if (fiberProp && fiberProp->Size() == 1)
		{
			FSCoreBase* fib = fiberProp->GetComponent(0);
			if (strcmp(fib->GetTypeString(), "user") == 0)
			{
				writeFibers = true;
			}
		}

		if (writeFibers) {
			int NE = (int)elSet.m_elem.size();
			XMLElement tag("ElementData");
			tag.add_attribute("var", "fiber");
			tag.add_attribute("elem_set", elSet.m_name.c_str());
			m_xml.add_branch(tag);
			{
				XMLElement el("e");
				int nid = el.add_attribute("lid", 0);
				for (int j = 0; j<NE; ++j)
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
void FEBioExport3::WriteMeshDataMaterialAxes()
{
	// loop over all element sets
	auto NSET = m_ElSet.size();
	for (int i = 0; i<NSET; ++i)
	{
		ElementSet& elSet = m_ElSet[i];
		FSCoreMesh* pm = elSet.m_mesh;
		GObject* po = pm->GetGObject();
		const Transform& T = po->GetTransform();

		// make sure there is something to do
		bool bwrite = false;
		int NE = (int)elSet.m_elem.size();
		for (int j = 0; j<NE; ++j)
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

				for (int j = 0; j<NE; ++j)
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
void FEBioExport3::WriteElementDataFields()
{
	FSModel& fem = *m_pfem;
	GModel& model = fem.GetModel();
	for (int i = 0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		FSMesh* pm = po->GetFEMesh();
		int NE = pm->Elements();

		int ND = pm->MeshDataFields();
		for (int n = 0; n<ND; ++n)
		{
			FEElementData* meshData = dynamic_cast<FEElementData*>(pm->GetMeshDataField(n));
			if (meshData)
			{
				FEElementData& data = *meshData;
				const FSElemSet* pg = data.GetElementSet();

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
				double v[FSElement::MAX_NODES] = { 0 };
				FEPartData& data = *partData;
				FSPartSet* partList = data.GetPartSet();
				FEElemList* elemList = data.BuildElemList();
				for (int np = 0; np < partList->size(); ++np)
				{
					int pid = (*partList)[np];
					GPart* pg = po->Part(pid);

					// A part could have been split into multiple Elements sections.
					// We need to do the same here.
					int NE = elemList->Size();
					FEElemList::Iterator it = elemList->First();
					for (int j = 0; j < NE; ++j, ++it) it->m_pi->m_ntag = 0;

					int nset = 0;
					int ecount = 0;
					char szname[128] = { 0 };
					do
					{
						if (nset == 0)
							sprintf(szname, "%s", pg->GetName().c_str());
						else
							sprintf(szname, "%s__%d", pg->GetName().c_str(), nset + 1);

						int etype = -1;
						FEElemList::Iterator it = elemList->First();
						for (int j=0; j<NE; ++j, ++it)
						{
							if (it->m_pi->m_ntag == 0)
							{
								etype = it->m_pi->Type();
								break;
							}
						}
						assert(etype != -1);

						XMLElement tag("ElementData");
						tag.add_attribute("name", data.GetName().c_str());
						tag.add_attribute("elem_set", szname);
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
								if ((pe->m_gid == pid) && (pe->Type() == etype))
								{
									el.set_attribute(nid, lid++);

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

									m_xml.add_leaf(el, false);
									ecount++;
									pe->m_ntag = 1;
								}
							}
						}
						m_xml.close_branch();
						nset++;
					}
					while (ecount < NE);
				}
				delete partList;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteSurfaceDataSection()
{
	FSModel& fem = *m_pfem;
	GModel& model = fem.GetModel();

	for (int i = 0; i<model.Objects(); i++)
	{
		FSMesh* mesh = model.Object(i)->GetFEMesh();

		for (int j = 0; j < mesh->MeshDataFields(); j++)
		{
			FESurfaceData* surfData = dynamic_cast<FESurfaceData*>(mesh->GetMeshDataField(j));
			if (surfData)
			{
				FESurfaceData& sd = *surfData;

				XMLElement tag("SurfaceData");
				tag.add_attribute("name", sd.GetName().c_str());

				if (sd.GetDataType() == DATA_SCALAR) tag.add_attribute("datatype", "scalar");
				else if (sd.GetDataType() == DATA_VEC3) tag.add_attribute("datatype", "vector");

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
void FEBioExport3::WriteEdgeDataSection()
{
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteNodeDataSection()
{
	FSModel& fem = *m_pfem;
	GModel& model = fem.GetModel();

	for (int i = 0; i<model.Objects(); i++)
	{
		FSMesh* mesh = model.Object(i)->GetFEMesh();

		for (int j = 0; j < mesh->MeshDataFields(); j++)
		{
			FENodeData* nodeData = dynamic_cast<FENodeData*>(mesh->GetMeshDataField(j));
			if (nodeData)
			{
				FENodeData& nd = *nodeData;

				XMLElement tag("NodeData");
				tag.add_attribute("name", nd.GetName().c_str());

				if      (nd.GetDataType() == DATA_SCALAR) tag.add_attribute("datatype", "scalar");
				else if (nd.GetDataType() == DATA_VEC3 ) tag.add_attribute("datatype", "vector");

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

void FEBioExport3::WriteBoundarySection(FSStep& s)
{
	FSModel& fem = m_prj.GetFSModel();

	for (int i = 0; i < s.BCs(); ++i)
	{
		FSBoundaryCondition* pbc = s.BC(i);
		if (pbc && pbc->IsActive())
		{
			if (dynamic_cast<FSFixedDOF*>(pbc)) WriteBCFixed(s, pbc);
			else if (dynamic_cast<FSPrescribedDOF*>(pbc)) WriteBCPrescribed(s, pbc);
			else WriteBC(s, pbc);
		}
	}

	// rigid contact 
	WriteBCRigid(s);

	// The fluid-rotational-velocity is a boundary condition in FEBio3
	for (int j = 0; j < s.Loads(); ++j)
	{
		FSSurfaceLoad* psl = dynamic_cast<FSSurfaceLoad*>(s.Load(j));
		if (psl && psl->IsActive() && (psl->Type() == FE_FLUID_ROTATIONAL_VELOCITY))
		{
			FEItemListBuilder* pitem = psl->GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(psl);

			stringstream ss;
			ss << "@surface:" << GetSurfaceName(pitem);
			string nodeSetName = ss.str();

			XMLElement e("bc");
			e.add_attribute("name", psl->GetName());
			e.add_attribute("type", "fluid rotational velocity");
			e.add_attribute("node_set", nodeSetName.c_str());
			m_xml.add_branch(e);
			{
				WriteParamList(*psl);
			}
			m_xml.close_branch();
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteRigidSection(FSStep& s)
{
	// rigid body constraints
	WriteRigidConstraints(s);

	// rigid connectors
	WriteConnectors(s);
	WriteRigidJoint(s);
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteContactSection(FSStep& s)
{
	// --- C O N T A C T ---
	for (int i = 0; i<s.Interfaces(); ++i)
	{
		FSPairedInterface* pi = dynamic_cast<FSPairedInterface*> (s.Interface(i));
		if (pi && pi->IsActive())
		{
			if (m_writeNotes) WriteNote(pi);

			int ntype = pi->Type();
			switch (ntype)
			{
			case FE_SLIDING_INTERFACE:
			{
				// NOTE: This interface is obsolete, but continues to be supported for now for backward compatibility.
				//       Note that we still use the old interface types. 
				FSSlidingInterface* ps = dynamic_cast<FSSlidingInterface*>(pi); assert(ps);
				int ntype = ps->GetIntValue(FSSlidingInterface::NTYPE);
				if (ntype == 0) WriteContactInterface(s, "sliding_with_gaps", pi);
				else if (ntype == 1) WriteContactInterface(s, "facet-to-facet sliding", pi);
			}
			break;
			case FE_SLIDING_WITH_GAPS: WriteContactInterface(s, "sliding-node-on-facet", pi); break;
			case FE_FACET_ON_FACET_SLIDING: WriteContactInterface(s, "sliding-facet-on-facet", pi); break;
			case FE_TIED_INTERFACE: WriteContactInterface(s, "tied-node-on-facet", pi); break;
			case FE_FACET_ON_FACET_TIED: WriteContactInterface(s, "tied-facet-on-facet", pi); break;
			case FE_TENSCOMP_INTERFACE: WriteContactInterface(s, "sliding-elastic", pi); break;
			case FE_PORO_INTERFACE: WriteContactInterface(s, "sliding-biphasic", pi); break;
			case FE_PORO_SOLUTE_INTERFACE: WriteContactInterface(s, "sliding-biphasic-solute", pi); break;
			case FE_MULTIPHASIC_INTERFACE: WriteContactInterface(s, "sliding-multiphasic", pi); break;
			case FE_TIEDBIPHASIC_INTERFACE: WriteContactInterface(s, "tied-biphasic", pi); break;
			case FE_TIEDMULTIPHASIC_INTERFACE: WriteContactInterface(s, "tied-multiphasic", pi); break;
			case FE_STICKY_INTERFACE: WriteContactInterface(s, "sticky", pi); break;
			case FE_PERIODIC_BOUNDARY: WriteContactInterface(s, "periodic boundary", pi); break;
			case FE_TIED_ELASTIC_INTERFACE: WriteContactInterface(s, "tied-elastic", pi); break;
			case FE_GAPHEATFLUX_INTERFACE: WriteContactInterface(s, "gap heat flux", pi); break;
			case FE_CONTACTPOTENTIAL_CONTACT: WriteContactInterface(s, "contact potential", pi); break;
			case FE_FEBIO_INTERFACE: WriteContactInterface(s, pi->GetTypeString(), pi); break;
			}
		}
	}

	// rigid walls
	WriteContactWall(s);

	// rigid spheres
	WriteContactSphere(s);

	// linear constraints
	WriteLinearConstraints(s);
}

//-----------------------------------------------------------------------------
// Write the loads section
void FEBioExport3::WriteLoadsSection(FSStep& s)
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
void FEBioExport3::WriteDiscreteSection(FSStep& s)
{
	FSModel& fem = *m_pfem;
	GModel& model = fem.GetModel();

	// Write the discrete materials
	XMLElement dmat("discrete_material");
	int n1 = dmat.add_attribute("id", 0);
	int n2 = dmat.add_attribute("name", "");
	int n3 = dmat.add_attribute("type", "");

	int n = 1;
	for (int i = 0; i<model.DiscreteObjects(); ++i)
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
				int lc = -1;// (p.GetLoadCurve() ? p.GetLoadCurve()->GetID() : -1);

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
	for (int i = 0; i<s.Interfaces(); ++i)
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
	for (int i = 0; i<model.DiscreteObjects(); ++i)
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
	for (int i = 0; i<s.Interfaces(); ++i)
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
// write rigid joints
//
void FEBioExport3::WriteRigidJoint(FSStep& s)
{
	for (int i = 0; i<s.Interfaces(); ++i)
	{
		// rigid joints
		FSRigidJoint* pj = dynamic_cast<FSRigidJoint*> (s.Interface(i));
		if (pj && pj->IsActive())
		{
			if (m_writeNotes) WriteNote(pj);

			XMLElement ec("rigid_connector");
			ec.add_attribute("type", "rigid joint");
			const char* sz = pj->GetName().c_str();
			ec.add_attribute("name", sz);
			m_xml.add_branch(ec);
			{
				int na = (pj->m_pbodyA ? pj->m_pbodyA->m_ntag : 0);
				int nb = (pj->m_pbodyB ? pj->m_pbodyB->m_ntag : 0);

				m_xml.add_leaf("tolerance", pj->GetFloatValue(FSRigidJoint::TOL));
				m_xml.add_leaf("penalty", pj->GetFloatValue(FSRigidJoint::PENALTY));
				m_xml.add_leaf("body_a", na);
				m_xml.add_leaf("body_b", nb);

				vec3d v = pj->GetVecValue(FSRigidJoint::RJ);
				m_xml.add_leaf("joint", v);
			}
			m_xml.close_branch();
		}
	}
}

//-----------------------------------------------------------------------------
// write rigid walls
//
void FEBioExport3::WriteContactWall(FSStep& s)
{
	for (int i = 0; i<s.Interfaces(); ++i)
	{
		FSRigidWallInterface* pw = dynamic_cast<FSRigidWallInterface*> (s.Interface(i));
		if (pw && pw->IsActive())
		{
			if (m_writeNotes) WriteNote(pw);

			XMLElement ec("contact");
			ec.add_attribute("type", "rigid_wall");
			const char* sz = pw->GetName().c_str();
			ec.add_attribute("name", sz);
			ec.add_attribute("surface", GetSurfaceName(pw->GetItemList()));
			m_xml.add_branch(ec);
			{
				WriteParam(pw->GetParam(FSRigidWallInterface::LAUGON));
				WriteParam(pw->GetParam(FSRigidWallInterface::ALTOL));
				WriteParam(pw->GetParam(FSRigidWallInterface::PENALTY));
				WriteParam(pw->GetParam(FSRigidWallInterface::OFFSET));

				XMLElement plane("plane");
				double a[4];
				a[0] = pw->GetFloatValue(FSRigidWallInterface::PA);
				a[1] = pw->GetFloatValue(FSRigidWallInterface::PB);
				a[2] = pw->GetFloatValue(FSRigidWallInterface::PC);
				a[3] = pw->GetFloatValue(FSRigidWallInterface::PD);
				plane.value(a, 4);
				m_xml.add_leaf(plane);
			}
			m_xml.close_branch();
		}
	}
}


//-----------------------------------------------------------------------------
// write rigid sphere contact
//
void FEBioExport3::WriteContactSphere(FSStep& s)
{
	for (int i = 0; i<s.Interfaces(); ++i)
	{
		FSRigidSphereInterface* pw = dynamic_cast<FSRigidSphereInterface*> (s.Interface(i));
		if (pw && pw->IsActive())
		{
			if (m_writeNotes) WriteNote(pw);

			XMLElement ec("contact");
			ec.add_attribute("type", "rigid sphere");
			const char* sz = pw->GetName().c_str();
			ec.add_attribute("name", sz);
			ec.add_attribute("surface", GetSurfaceName(pw->GetItemList()));
			m_xml.add_branch(ec);
			{
				m_xml.add_leaf("laugon", (pw->GetBoolValue(FSRigidSphereInterface::LAUGON) ? 1 : 0));
				m_xml.add_leaf("tolerance", pw->GetFloatValue(FSRigidSphereInterface::ALTOL));
				m_xml.add_leaf("penalty", pw->GetFloatValue(FSRigidSphereInterface::PENALTY));
				m_xml.add_leaf("radius", pw->Radius());
				m_xml.add_leaf("center", pw->Center());

/*				LoadCurve* lc[3];
				lc[0] = pw->GetLoadCurve(0);
				lc[1] = pw->GetLoadCurve(1);
				lc[2] = pw->GetLoadCurve(2);

				if (lc[0])
				{
					XMLElement el("ux");
					el.add_attribute("lc", lc[0]->GetID());
					el.value(1.0);
					m_xml.add_leaf(el);
				}
				if (lc[1])
				{
					XMLElement el("uy");
					el.add_attribute("lc", lc[1]->GetID());
					el.value(1.0);
					m_xml.add_leaf(el);
				}
				if (lc[2])
				{
					XMLElement el("uz");
					el.add_attribute("lc", lc[2]->GetID());
					el.value(1.0);
					m_xml.add_leaf(el);
				}
*/
			}
			m_xml.close_branch();
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteContactInterface(FSStep& s, const char* sztype, FSPairedInterface* pi)
{
	XMLElement ec("contact");
	ec.add_attribute("type", sztype);
	const char* sz = pi->GetName().c_str();
	ec.add_attribute("name", sz);
	ec.add_attribute("surface_pair", sz);

	m_xml.add_branch(ec);
	{
		WriteParamList(*pi);
	}
	m_xml.close_branch();
}

//-----------------------------------------------------------------------------
// write rigid interfaces
//
void FEBioExport3::WriteBCRigid(FSStep& s)
{
	for (int i = 0; i<s.Interfaces(); ++i)
	{
		// rigid interfaces
		FSRigidInterface* pr = dynamic_cast<FSRigidInterface*> (s.Interface(i));
		if (pr && pr->IsActive())
		{
			if (m_writeNotes) WriteNote(pr);

			GMaterial* pm = pr->GetRigidBody();
			if (pm == 0) throw RigidContactException();
			int rb = pm->m_ntag;

			FEItemListBuilder* pitem = pr->GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(pr);

			XMLElement ec("bc");
			ec.add_attribute("name", pr->GetName());
			ec.add_attribute("type", "rigid");
			ec.add_attribute("node_set", GetNodeSetName(pitem));
			m_xml.add_branch(ec);
			{
				m_xml.add_leaf("rb", rb);
			}
			m_xml.close_branch();
		}
	}
}

//-----------------------------------------------------------------------------
// write linear constraints
void FEBioExport3::WriteLinearConstraints(FSStep& s)
{
    // This list should be consistent with the list of DOFs in FSModel::FSModel()
	const char* szbc[] = { "x", "y", "z", "u", "v", "w", "p", "T", "wx", "wy", "wz", "ef", "sx", "sy", "sz" };

	for (int i = 0; i<s.LinearConstraints(); ++i)
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
			for (int j = 0; j<NC; ++j)
			{
				FSLinearConstraintSet::LinearConstraint& LC = pset->m_set[j];
				m_xml.add_branch("linear_constraint");
				{
					int ND = (int)LC.m_dof.size();
					XMLElement ed("node");
					int n1 = ed.add_attribute("id", 0);
					int n2 = ed.add_attribute("bc", 0);
					for (int n = 0; n<ND; ++n)
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
void FEBioExport3::WriteConstraints(FSStep& s)
{
	for (int i = 0; i<s.Constraints(); ++i)
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
// Write the fixed boundary conditions
void FEBioExport3::WriteBC(FSStep& s, FSBoundaryCondition* pbc)
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
// Write the fixed boundary conditions
void FEBioExport3::WriteBCFixed(FSStep &s, FSBoundaryCondition* febc)
{
	FSModel& fem = m_prj.GetFSModel();
	FSFixedDOF* pbc = dynamic_cast<FSFixedDOF*>(febc); assert(pbc);
	if (pbc == nullptr) return;

	if (m_writeNotes) WriteNote(pbc);

	// get the item list
	FEItemListBuilder* pitem = pbc->GetItemList();
	if (pitem == 0) throw InvalidItemListBuilder(pbc);

	// build the BC string
	int bc = pbc->GetBC();
	FEDOFVariable& var = fem.Variable(pbc->GetVarID());
	std::stringstream ss;
	int n = 0;
	for (int i = 0; i<var.DOFs(); ++i)
	{
		if (bc & (1 << i))
		{
			if (n != 0) ss << ",";
			ss << var.GetDOF(i).symbol();
			n++;
		}
	}
	std::string sbc = ss.str();

	// create the fix tag
	if (n > 0)
	{
		// get node set name
		string nodeSetName = GetNodeSetName(pitem);

		XMLElement tag("bc");
		tag.add_attribute("name", pbc->GetName());
		tag.add_attribute("type", "fix");
		tag.add_attribute("node_set", nodeSetName);

		// write the tag
		m_xml.add_branch(tag);
		{
			m_xml.add_leaf("dofs", sbc.c_str());
		}
		m_xml.close_branch();
	}
}

//-----------------------------------------------------------------------------
// Export prescribed boundary conditions
void FEBioExport3::WriteBCPrescribed(FSStep &s, FSBoundaryCondition* febc)
{
	FSModel& fem = m_prj.GetFSModel();
	FSPrescribedDOF* pbc = dynamic_cast<FSPrescribedDOF*>(febc); assert(febc);
	if (pbc == nullptr) return;

	if (m_writeNotes) WriteNote(pbc);

	FEDOFVariable& var = fem.Variable(pbc->GetVarID());
	const char* szbc = var.GetDOF(pbc->GetDOF()).symbol();

	FEItemListBuilder* pitem = pbc->GetItemList();
	if (pitem == 0) throw InvalidItemListBuilder(pbc);

	XMLElement e("bc");
	e.add_attribute("name", pbc->GetName());
	e.add_attribute("type", "prescribe");
	e.add_attribute("node_set", GetNodeSetName(pitem));
	m_xml.add_branch(e);
	{
		m_xml.add_leaf("dof", szbc);
		WriteParamList(*pbc);
	}
	m_xml.close_branch();
}

//-----------------------------------------------------------------------------
// export nodal loads
//

void FEBioExport3::WriteNodalLoads(FSStep& s)
{
	for (int j = 0; j < s.Loads(); ++j)
	{
		FSNodalLoad* pbc = dynamic_cast<FSNodalLoad*>(s.Load(j));
		if (pbc && pbc->IsActive())
		{
			if (pbc->Type() == FE_NODAL_DOF_LOAD) WriteDOFNodalLoad(s, pbc);
			else
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
}

void FEBioExport3::WriteDOFNodalLoad(FSStep& s, FSNodalLoad* pnl)
{
	char bc[][3] = { "x", "y", "z", "sx", "sy", "sz", "p", "c1", "c2", "c3", "c4", "c5", "c6" };

	FSNodalDOFLoad* pbc = dynamic_cast<FSNodalDOFLoad*>(pnl); assert(pbc);
	if (pbc == nullptr) return;

	if (m_writeNotes) WriteNote(pbc);

	FEItemListBuilder* pitem = pbc->GetItemList();
	if (pitem == 0) throw InvalidItemListBuilder(pbc);

	int l = pbc->GetDOF();

	XMLElement load("nodal_load");
	load.add_attribute("name", pbc->GetName());
	load.add_attribute("type", "nodal_load");
	load.add_attribute("node_set", GetNodeSetName(pitem));

	m_xml.add_branch(load);
	{
		m_xml.add_leaf("dof", bc[l]);
		WriteParam(pbc->GetParam(FSNodalDOFLoad::LOAD));
	}
	m_xml.close_branch(); // nodal_load
}

//----------------------------------------------------------------------------
// Export pressure loads
//
void FEBioExport3::WriteSurfaceLoads(FSStep& s)
{
	for (int j = 0; j<s.Loads(); ++j)
	{
		FSSurfaceLoad* pbc = dynamic_cast<FSSurfaceLoad*>(s.Load(j));
		if (pbc && pbc->IsActive())
		{
			if (m_writeNotes) WriteNote(pbc);

			switch (pbc->Type())
			{
			case FE_PRESSURE_LOAD            : WriteSurfaceLoad(s, pbc, "pressure"); break;
			case FE_SURFACE_TRACTION         : WriteSurfaceLoad(s, pbc, "traction"); break;
            case FE_SURFACE_FORCE            : WriteSurfaceLoad(s, pbc, "force"); break;
            case FE_BEARING_LOAD             : WriteSurfaceLoad(s, pbc, "bearing load"); break;
			case FE_FLUID_FLUX               : WriteSurfaceLoad(s, pbc, "fluidflux"); break;
			case FE_BP_NORMAL_TRACTION       : WriteSurfaceLoad(s, pbc, "normal_traction"); break;
            case FE_MATCHING_OSM_COEF        : WriteSurfaceLoad(s, pbc, "matching_osm_coef"); break;
			case FE_HEAT_FLUX                : WriteSurfaceLoad(s, pbc, "heatflux"); break;
			case FE_CONV_HEAT_FLUX           : WriteSurfaceLoad(s, pbc, "convective_heatflux"); break;
			case FE_FLUID_TRACTION           : WriteSurfaceLoad(s, pbc, "fluid viscous traction"); break;
			case FE_FLUID_FLOW_RESISTANCE    : WriteSurfaceLoad(s, pbc, "fluid resistance"); break;
            case FE_FLUID_FLOW_RCR           : WriteSurfaceLoad(s, pbc, "fluid RCR"); break;
			case FE_FLUID_TANGENTIAL_STABIL  : WriteSurfaceLoad(s, pbc, "fluid tangential stabilization"); break;
			case FE_FLUID_BACKFLOW_STABIL    : WriteSurfaceLoad(s, pbc, "fluid backflow stabilization"); break;
			case FE_FSI_TRACTION             : WriteSurfaceLoad(s, pbc, "fluid-FSI traction"); break;
            case FE_BFSI_TRACTION            : WriteSurfaceLoad(s, pbc, "biphasic-FSI traction"); break;
			case FE_FLUID_NORMAL_VELOCITY    : WriteSurfaceLoad(s, pbc, "fluid normal velocity"); break;
			case FE_FLUID_VELOCITY           : WriteSurfaceLoad(s, pbc, "fluid velocity"); break;
			case FE_SOLUTE_FLUX              : WriteSurfaceLoad(s, pbc, "soluteflux"); break;
            case FE_SOLUTE_NATURAL_FLUX      : WriteSurfaceLoad(s, pbc, "solute natural flux"); break;
			case FE_CONCENTRATION_FLUX       : WriteSurfaceLoad(s, pbc, "concentration flux"); break;
            case FE_FLUID_PRESSURE_LOAD      : WriteSurfaceLoad(s, pbc, "fluid pressure"); break;
			// NOTE: Fluid rotational velocity is a boundary condition in FEBio3!
			case FE_FLUID_ROTATIONAL_VELOCITY: break;// WriteSurfaceLoad(s, pbc, "fluid rotational velocity"); break;

			// NOTE: temporary hack!
			case FE_FEBIO_SURFACE_LOAD: WriteSurfaceLoad(s, pbc, pbc->GetTypeString()); break;
			default:
				assert(false);
			}
		}
	}
}

//----------------------------------------------------------------------------
void FEBioExport3::WriteSurfaceLoad(FSStep& s, FSSurfaceLoad* psl, const char* sztype)
{
	// create the surface list
	FEItemListBuilder* pitem = psl->GetItemList();
	if (pitem == 0) throw InvalidItemListBuilder(psl);

	XMLElement load;
	load.name("surface_load");
	load.add_attribute("name", psl->GetName());
	load.add_attribute("type", sztype);
	load.add_attribute("surface", GetSurfaceName(pitem));
	m_xml.add_branch(load);
	{
		WriteParamList(*psl);
	}
	m_xml.close_branch(); // surface_load
}

//-----------------------------------------------------------------------------
// Export initial conditions
//
void FEBioExport3::WriteInitialSection()
{
	FSModel& fem = m_prj.GetFSModel();
	FSStep& s = *fem.GetStep(0);

	// initial velocities
	for (int j = 0; j<s.ICs(); ++j)
	{
		FSInitialCondition* pi = s.IC(j);
		if (pi && pi->IsActive())
		{
			if (m_writeNotes) WriteNote(pi);

			FEItemListBuilder* pitem = nullptr;

			if (dynamic_cast<FSInitialNodalDOF*>(pi))
			{
				pitem = pi->GetItemList();
				if (pitem == 0) throw InvalidItemListBuilder(pi);
			}

			switch (pi->Type())
			{
			case FE_INIT_NODAL_VELOCITIES      : WriteInitVelocity(dynamic_cast<FSNodalVelocities  &>(*pi)); break;
			case FE_INIT_NODAL_SHELL_VELOCITIES: WriteInitShellVelocity(dynamic_cast<FSNodalShellVelocities&>(*pi)); break;
			case FE_INIT_FLUID_PRESSURE        : WriteInitFluidPressure(dynamic_cast<FSInitFluidPressure&>(*pi)); break;
			case FE_INIT_SHELL_FLUID_PRESSURE  : WriteInitShellFluidPressure(dynamic_cast<FSInitShellFluidPressure&>(*pi)); break;
			case FE_INIT_CONCENTRATION         : WriteInitConcentration(dynamic_cast<FSInitConcentration&>(*pi)); break;
			case FE_INIT_SHELL_CONCENTRATION   : WriteInitShellConcentration(dynamic_cast<FSInitShellConcentration&>(*pi)); break;
			case FE_INIT_TEMPERATURE           : WriteInitTemperature(dynamic_cast<FSInitTemperature  &>(*pi)); break;
            case FE_INIT_FLUID_DILATATION      : WriteInitFluidDilatation(dynamic_cast<FSInitFluidDilatation  &>(*pi)); break;
			case FE_INIT_PRESTRAIN             : WriteInitPrestrain(dynamic_cast<FSInitPrestrain&>(*pi)); break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteInitVelocity(FSNodalVelocities& iv)
{
	XMLElement el("ic");
	el.add_attribute("name", iv.GetName());
	el.add_attribute("type", "velocity");
	el.add_attribute("node_set", GetNodeSetName(iv.GetItemList()));
	m_xml.add_branch(el, false);
	{
		vec3d v = iv.GetVelocity();
		m_xml.add_leaf("value", v);
	}
	m_xml.close_branch();
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteInitShellVelocity(FSNodalShellVelocities& iv)
{
	XMLElement el("init");
	int nbc = el.add_attribute("bc", "");
	el.add_attribute("node_set", GetNodeSetName(iv.GetItemList()));
	vec3d v = iv.GetVelocity();
	if (v.x != 0.0)
	{
		el.set_attribute(nbc, "svx");
		m_xml.add_branch(el, false);
		{
			m_xml.add_leaf("value", v.x);
		}
		m_xml.close_branch();
	}
	if (v.y != 0.0)
	{
		el.set_attribute(nbc, "svy");
		m_xml.add_branch(el, false);
		{
			m_xml.add_leaf("value", v.y);
		}
		m_xml.close_branch();
	}
	if (v.z != 0.0)
	{
		el.set_attribute(nbc, "svz");
		m_xml.add_branch(el, false);
		{
			m_xml.add_leaf("value", v.z);
		}
		m_xml.close_branch();
	}
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteInitConcentration(FSInitConcentration& ic)
{
    char szbc[6];
	int bc = ic.GetBC();
    sprintf(szbc, "c%d",bc+1);
	XMLElement ec("ic");
	ec.add_attribute("type", "init_dof");
	ec.add_attribute("node_set", GetNodeSetName(ic.GetItemList()));
	m_xml.add_branch(ec);
	{
		m_xml.add_leaf("dof", szbc);
		m_xml.add_leaf("value", ic.GetValue());
	}
	m_xml.close_branch();
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteInitShellConcentration(FSInitShellConcentration& ic)
{
    char szbc[6];
	int bc = ic.GetBC();
    sprintf(szbc, "d%d",bc+1);
	XMLElement ec("ic");
	ec.add_attribute("type", "init_dof");
	ec.add_attribute("node_set", GetNodeSetName(ic.GetItemList()));
	m_xml.add_branch(ec);
	{
		m_xml.add_leaf("dof", szbc);
		m_xml.add_leaf("value", ic.GetValue());
	}
	m_xml.close_branch();
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteInitFluidPressure(FSInitFluidPressure& ip)
{
	XMLElement ec("ic");
	ec.add_attribute("type", "init_dof");
	ec.add_attribute("node_set", GetNodeSetName(ip.GetItemList()));
	m_xml.add_branch(ec);
	{
		m_xml.add_leaf("dof", "p");

		Param* pp = ip.GetParam("value"); assert(pp);
		WriteParam(*pp);
	}
	m_xml.close_branch();
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteInitShellFluidPressure(FSInitShellFluidPressure& ip)
{
	XMLElement ec("ic");
	ec.add_attribute("type", "init_dof");
	ec.add_attribute("node_set", GetNodeSetName(ip.GetItemList()));
	m_xml.add_branch(ec);
	{
		m_xml.add_leaf("dof", "q");
		m_xml.add_leaf("value", ip.GetValue());
	}
	m_xml.close_branch();
}
//-----------------------------------------------------------------------------
void FEBioExport3::WriteInitTemperature(FSInitTemperature&   it)
{
	XMLElement ec("ic");
	ec.add_attribute("type", "init_dof");
	ec.add_attribute("node_set", GetNodeSetName(it.GetItemList()));
	m_xml.add_branch(ec);
	{
		m_xml.add_leaf("dof", "T");
		m_xml.add_leaf("value", it.GetValue());
	}
	m_xml.close_branch();
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteInitFluidDilatation(FSInitFluidDilatation&   it)
{
	XMLElement ec("ic");
	ec.add_attribute("type", "init_dof");
	ec.add_attribute("node_set", GetNodeSetName(it.GetItemList()));
    m_xml.add_branch(ec);
    {
		m_xml.add_leaf("dof", "ef");
		m_xml.add_leaf("value", it.GetValue());
    }
    m_xml.close_branch();
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteInitPrestrain(FSInitPrestrain& ip)
{
	XMLElement el("ic");
	el.add_attribute("name", ip.GetName().c_str());
	el.add_attribute("type", "prestrain");
	m_xml.add_branch(el);
	{
		WriteParamList(ip);
	}
	m_xml.close_branch();
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteBodyLoads(FSStep& s)
{
	for (int i = 0; i<s.Loads(); ++i)
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

void FEBioExport3::WriteGlobalsSection()
{
	XMLElement el;
	FSModel& fem = *m_pfem;

	if (fem.Parameters())
	{
		m_xml.add_branch("Constants");
		{
			int N = fem.Parameters();
			for (int i = 0; i<N; ++i)
			{
				Param& p = fem.GetParam(i);
				m_xml.add_leaf(p.GetShortName(), p.GetFloatValue());
			}
		}
		m_xml.close_branch();

		if (fem.Solutes()>0)
		{
			m_xml.add_branch("Solutes");
			{
				int NS = fem.Solutes();
				for (int i = 0; i<NS; ++i)
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

		if (fem.SBMs()>0)
		{
			m_xml.add_branch("SolidBoundMolecules");
			{
				int NS = fem.SBMs();
				for (int i = 0; i<NS; ++i)
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

void FEBioExport3::WriteLoadDataSection()
{
	FSModel& fem = m_prj.GetFSModel();

	for (int i = 0; i<fem.LoadControllers(); ++i)
	{
		FSLoadController* plc = fem.GetLoadController(i);

		XMLElement el;
		el.name("load_controller");
		el.add_attribute("id", i + 1);
		el.add_attribute("type", plc->GetTypeString());
		if (plc->GetName().empty() == false) el.add_attribute("name", plc->GetName());

		m_xml.add_branch(el);
		{
			WriteParamList(*plc);
		}
		m_xml.close_branch();
	}
}

//-----------------------------------------------------------------------------

void FEBioExport3::WriteSurfaceSection(FEFaceList& s)
{
	XMLElement ef;
	int n = 1, nn[9];

	int NF = s.Size();
	FEFaceList::Iterator pf = s.First();

	int nfn;
	for (int j = 0; j<NF; ++j, ++n, ++pf)
	{
		if (pf->m_pi == 0) throw InvalidItemListBuilder(0);
		FSFace& face = *(pf->m_pi);
		FSCoreMesh* pm = pf->m_pm;
		nfn = face.Nodes();
		for (int k = 0; k<nfn; ++k) nn[k] = pm->Node(face.n[k]).m_nid;
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

void FEBioExport3::WriteSurfaceSection(NamedItemList& l)
{
	FEItemListBuilder* pl = l.m_list;
	FEFaceList* pfl = pl->BuildFaceList();
	if (pfl == nullptr) throw InvalidItemListBuilder(l.m_name);
	std::unique_ptr<FEFaceList> ps(pfl);

	XMLElement ef;
	int n = 1, nn[FSFace::MAX_NODES];

	FEFaceList& s = *pfl;
	int NF = s.Size();
	FEFaceList::Iterator pf = s.First();

	int nfn;
	for (int j = 0; j < NF; ++j, ++n, ++pf)
	{
		if (pf->m_pi == 0) throw InvalidItemListBuilder(l.m_name);
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

//-----------------------------------------------------------------------------
void FEBioExport3::WriteElementList(FEElemList& el)
{
	int NE = el.Size();
	FEElemList::Iterator pe = el.First();
	for (int i = 0; i<NE; ++i, ++pe)
	{
		FEElement_& el = *(pe->m_pi);
		XMLElement e("e");
		e.add_attribute("id", el.m_nid);
		m_xml.add_empty(e);
	}
}

//-----------------------------------------------------------------------------
void FEBioExport3::WriteOutputSection()
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
		for (int i = 0; i<N; ++i) if (plt.PlotVariable(i).isActive()) na++;

		if (na > 0)
		{
			m_xml.add_branch(p);
			{
				for (int i = 0; i<N; ++i)
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
								for (int n = 0; n<v.Domains(); ++n)
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
			for (int i = 0; i<N; ++i)
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
					FEItemListBuilder* pg = nd.GetItemList();
					if (pg)
					{
						e.add_attribute("node_set", pg->GetName());
					}
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
					FEItemListBuilder* pg = ed.GetItemList();
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

void FEBioExport3::WriteStepSection()
{
	// we've already written the initial step
	// so now we simply output all the analysis steps
	for (int i = 1; i<m_pfem->Steps(); ++i)
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
			if (nbc>0)
			{
				m_xml.add_branch("Boundary");
				{
					WriteBoundarySection(step);
				}
				m_xml.close_branch(); // Boundary
			}

			int nrc = step.RigidBCs() + step.RigidICs() + step.RigidLoads() + step.RigidConnectors();
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
			if (nlc>0)
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
			int nnlc = CountConstraints<FSModelConstraint>(*m_pfem) + CountInterfaces<FSRigidJoint>(*m_pfem);
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

void FEBioExport3::WriteRigidConstraints(FSStep &s)
{
	const char* szbc[6] = { "Rx", "Ry", "Rz", "Ru", "Rv", "Rw" };

	for (int i = 0; i < s.RigidBCs(); ++i)
	{
		FSRigidBC* ps = s.RigidBC(i);
		if (ps && ps->IsActive())
		{
			if (m_writeNotes) WriteNote(ps);

			const char* sztype = nullptr;
			if (strcmp(ps->GetTypeString(), "rigid_fixed") == 0) sztype = "fix";
			if (strcmp(ps->GetTypeString(), "rigid_prescribed") == 0) sztype = "prescribe";
			assert(sztype);


			XMLElement el;
			el.name("rigid_constraint");
			el.add_attribute("name", ps->GetName());
			el.add_attribute("type", sztype);
			m_xml.add_branch(el);
			WriteParamList(*ps);
			m_xml.close_branch();
		}
	}

	for (int i = 0; i < s.RigidLoads(); ++i)
	{
		FSRigidLoad* ps = s.RigidLoad(i);
		if (ps && ps->IsActive())
		{
			if (m_writeNotes) WriteNote(ps);

			XMLElement el("rigid_constraint");
			el.add_attribute("name", ps->GetName());
			el.add_attribute("type", "force");
			m_xml.add_branch(el);
			WriteParamList(*ps);
			m_xml.close_branch();
		}
	}

	for (int i = 0; i < s.RigidICs(); ++i)
	{
		FSRigidIC* ps = s.RigidIC(i);
		if (ps && ps->IsActive())
		{
			const char* sztype = nullptr;
			if (strcmp(ps->GetTypeString(), "initial_rigid_velocity") == 0) sztype = "rigid_velocity";
			if (strcmp(ps->GetTypeString(), "initial_rigid_angular_velocity") == 0) sztype = "rigid_angular_velocity";
			assert(sztype);

			XMLElement el("rigid_constraint");
			el.add_attribute("name", ps->GetName());
			el.add_attribute("type", sztype);
			m_xml.add_branch(el);
			WriteParamList(*ps);
			m_xml.close_branch();
		}
	}
}

//-----------------------------------------------------------------------------
// write rigid connectors
//
void FEBioExport3::WriteConnectors(FSStep& s)
{
	for (int i = 0; i<s.RigidConnectors(); ++i)
	{
		// rigid connectors
		FSRigidConnector* pj = s.RigidConnector(i);
		if (pj && pj->IsActive())
		{
			if (m_writeNotes) WriteNote(pj);

			XMLElement ec("rigid_connector");
			ec.add_attribute("name", pj->GetName());
			const char* sztype = pj->GetTypeString(); assert(sztype);
			ec.add_attribute("type", sztype);

			GMaterial* pgA = m_pfem->GetMaterialFromID(pj->GetRigidBody1());
			if (pgA == 0) throw MissingRigidBody(pj->GetName().c_str());
			if (pgA->GetMaterialProperties()->IsRigid() == false) throw InvalidMaterialReference();

			GMaterial* pgB = m_pfem->GetMaterialFromID(pj->GetRigidBody2());
			if (pgB == 0) throw MissingRigidBody(pj->GetName().c_str());
			if (pgB->GetMaterialProperties()->IsRigid() == false) throw InvalidMaterialReference();

			const char* sz = pj->GetName().c_str();
			ec.add_attribute("name", sz);
			m_xml.add_branch(ec);
			{
				int na = pgA->m_ntag;
				int nb = pgB->m_ntag;
				m_xml.add_leaf("body_a", na);
				m_xml.add_leaf("body_b", nb);

				WriteParamList(*pj);
			}
			m_xml.close_branch();
		}
	}
}

//-----------------------------------------------------------------------------

void FEBioExport3::WriteConstraintSection(FSStep &s)
{
	// some contact definitions are actually stored in the constraint section
	WriteConstraints(s);
}
