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

// FEFEBioExport2.cpp: implementation of the FEFEBioExport2 class.
//
//////////////////////////////////////////////////////////////////////

#include "FEBioExport25.h"
#include <FEMLib/FERigidConstraint.h>
#include <FEMLib/FEInitialCondition.h>
#include <FEMLib/FESurfaceLoad.h>
#include <FEMLib/FEBodyLoad.h>
#include <FEMLib/FEModelConstraint.h>
#include <GeomLib/GGroup.h>
#include <MeshLib/FEElementData.h>
#include <MeshLib/FESurfaceData.h>
#include <GeomLib/GModel.h>
#include <GeomLib/GObject.h>
#include <memory>
#include <sstream>
#include <FECore/FETransform.h>

using std::unique_ptr;
using std::stringstream;

//-----------------------------------------------------------------------------
FSNodeList* BuildNodeList(GFace* pf)
{
	if (pf == 0) return 0;
	GObject* po = dynamic_cast<GObject*>(pf->Object());
	FSMesh& m = *po->GetFEMesh();

	// clear the tags
	for (int i = 0; i<m.Nodes(); ++i) m.Node(i).m_ntag = 0;

	// then we need to identify which nodes are part of the nodelist
	int gid = pf->GetLocalID();

	// tag the nodes to be added to the list
	for (int i = 0; i<m.Faces(); ++i)
	{
		FSFace& f = m.Face(i);
		if (f.m_gid == gid)
		{
			int l = f.Nodes();
			for (int j = 0; j<l; ++j) m.Node(f.n[j]).m_ntag = 1;
		}
	}

	// create a new node list
	FSNodeList* ps = new FSNodeList();

	// next we add all the nodes
	for (int i = 0; i<m.Nodes(); ++i) if (m.Node(i).m_ntag == 1) ps->Add(&m, m.NodePtr(i));

	return ps;
}

//-----------------------------------------------------------------------------
FSNodeList* BuildNodeList(GPart* pg)
{
	if (pg == 0) return 0;
	GObject* po = dynamic_cast<GObject*>(pg->Object());
	FSMesh& m = *po->GetFEMesh();

	// clear the tags
	for (int i = 0; i<m.Nodes(); ++i) m.Node(i).m_ntag = 0;

	// then we need to identify which nodes are part of the nodelist
	int gid = pg->GetLocalID();

	// tag the nodes to be added to the list
	for (int i = 0; i<m.Elements(); ++i)
	{
		FSElement& el = m.Element(i);
		if (el.m_gid == gid)
		{
			int l = el.Nodes();
			for (int j = 0; j<l; ++j) m.Node(el.m_node[j]).m_ntag = 1;
		}
	}

	// create a new node list
	FSNodeList* ps = new FSNodeList();

	// next we add all the nodes
	for (int i = 0; i<m.Nodes(); ++i) if (m.Node(i).m_ntag == 1) ps->Add(&m, m.NodePtr(i));

	return ps;
}

//-----------------------------------------------------------------------------
FSNodeList* BuildNodeList(GNode* pn)
{
	if (pn == 0) return 0;
	FSNodeList* ps = new FSNodeList;
	GObject* po = dynamic_cast<GObject*>(pn->Object());
	FSMesh* pm = po->GetFEMesh();
	for (int j = 0; j<pm->Nodes(); ++j)
	{
		FSNode& node = pm->Node(j);
		if (node.m_gid == pn->GetLocalID())
		{
			ps->Add(pm, &node);
			break;
		}
	}
	return ps;
}

//-----------------------------------------------------------------------------
FSNodeList* BuildNodeList(GEdge* pe)
{
	if (pe == 0) return 0;
	GObject* po = dynamic_cast<GObject*>(pe->Object());
	FSMesh& m = *po->GetFEMesh();

	// clear the tags
	for (int i = 0; i < m.Nodes(); ++i) m.Node(i).m_ntag = 0;

	// then we need to identify which nodes are part of the nodelist
	int gid = pe->GetLocalID();

	// tag the nodes to be added to the list
	for (int i = 0; i < m.Edges(); ++i)
	{
		FSEdge& e = m.Edge(i);
		if (e.m_gid == gid)
		{
			int l = e.Nodes();
			for (int j = 0; j < l; ++j) m.Node(e.n[j]).m_ntag = 1;
		}
	}

	// create a new node list
	FSNodeList* ps = new FSNodeList();

	// next we add all the nodes
	for (int i = 0; i < m.Nodes(); ++i) if (m.Node(i).m_ntag == 1) ps->Add(&m, m.NodePtr(i));

	return ps;
}

//-----------------------------------------------------------------------------
FEFaceList* BuildFaceList(GFace* face)
{
	FEFaceList* ps = new FEFaceList();
	GObject* po = dynamic_cast<GObject*>(face->Object());
	FSMesh& m = *po->GetFEMesh();
	int gid = face->GetLocalID();

	for (int i = 0; i<m.Faces(); ++i) if (m.Face(i).m_gid == gid) ps->Add(&m, m.FacePtr(i));

	return ps;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FEBioExport25::FEBioExport25(FSProject& prj) : FEBioExport(prj)
{
	m_exportParts = false;
	m_useReactionMaterial2 = false;	// will be set to true for reaction-diffusion problems
	m_writeNotes = true;
}

FEBioExport25::~FEBioExport25()
{
	Clear();
}

void FEBioExport25::Clear()
{
	FEBioExport::Clear();
	m_pSurf.clear();
	m_pNSet.clear();
	m_pESet.clear();
	m_ElSet.clear();
}

//----------------------------------------------------------------------------
FEBioExport25::Part* FEBioExport25::FindPart(GObject* po)
{
	for (size_t i=0; i<m_Part.size(); ++i)
	{
		Part* pi = m_Part[i];
		if (po == pi->m_obj) return pi;
	}
	return 0;
}

//----------------------------------------------------------------------------
const char* FEBioExport25::GetSurfaceName(FEItemListBuilder* pl)
{
	int N = (int) m_pSurf.size();
	for (int i=0; i<N; ++i)
		if (m_pSurf[i].second == pl) return m_pSurf[i].first.c_str();
	assert(false);
	return 0;
}

//----------------------------------------------------------------------------
const char* FEBioExport25::GetNodeSetName(FEItemListBuilder* pl)
{
	int N = (int) m_pNSet.size();
	for (int i=0; i<N; ++i)
		if (m_pNSet[i].second == pl) return m_pNSet[i].first.c_str();
	return 0;
}

//-----------------------------------------------------------------------------
void FEBioExport25::AddNodeSet(const std::string& name, FEItemListBuilder* pl)
{
	if (m_exportParts)
	{
		switch (pl->Type())
		{
		case GO_NODE:
			{
				GNodeList* itemList = dynamic_cast<GNodeList*>(pl); assert(itemList);
				vector<GNode*> nodeList = itemList->GetNodeList();
				for (int i=0; i<nodeList.size(); ++i)
				{
					GNode* node = nodeList[i];
					GObject* po = dynamic_cast<GObject*>(node->Object());
					Part* part = FindPart(po);
					NodeSet* ns = part->FindNodeSet(node->GetName());
					if (ns == 0) part->m_NSet.push_back(new NodeSet(node->GetName(), BuildNodeList(node)));
				}
			}
			break;
		case GO_FACE:
			{
				GFaceList* itemList = dynamic_cast<GFaceList*>(pl); assert(itemList);
				vector<GFace*> faceList = itemList->GetFaceList();
				for (int i=0; i<faceList.size(); ++i)
				{
					GFace* face = faceList[i];
					GObject* po = dynamic_cast<GObject*>(face->Object());
					Part* part = FindPart(po);
					NodeSet* ns = part->FindNodeSet(face->GetName());
					if (ns == 0) part->m_NSet.push_back(new NodeSet(face->GetName(), BuildNodeList(face)));
				}
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
		}
	}
	
	m_pNSet.push_back(NamedList(name, pl));
}

//-----------------------------------------------------------------------------
void FEBioExport25::AddSurface(const std::string& name, FEItemListBuilder* pl)
{
	if (m_exportParts)
	{
		switch (pl->Type())
		{
		case GO_FACE:
			{
				GFaceList* itemList = dynamic_cast<GFaceList*>(pl); assert(itemList);
				vector<GFace*> faceList = itemList->GetFaceList();
				for (int i=0; i<faceList.size(); ++i)
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
		for (int i=0; i<m_pSurf.size(); ++i)
		{
			NamedList& surf = m_pSurf[i];
			if ((surf.second == pl) && (surf.first == name)) return;
		}
	}

	m_pSurf.push_back(NamedList(string(name), pl));
}

//-----------------------------------------------------------------------------
void FEBioExport25::AddElemSet(const std::string& name, FEItemListBuilder* pl)
{
	if (pl) m_pESet.push_back(NamedList(string(name), pl));
}

//-----------------------------------------------------------------------------
bool FEBioExport25::PrepareExport(FSProject& prj)
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
		for (int i=2; i<m_nsteps; ++i)
		{
			if (fem.GetStep(i)->GetType() != ntype) return errf("All analysis steps must be of same type for this file format.");
		}		
	}
	
	// build the parts
	if (m_exportParts)
	{
		int nobj = model.Objects();
		for (int i=0; i<nobj; ++i)
		{
			m_Part.push_back(new Part(model.Object(i)));
		}
	}

	// Build the named nodeset list
	BuildNodeSetList(prj);

	// Build the named surface list
	BuildSurfaceList(prj);

	// build the element list
	BuildElemSetList(prj);

	// see if we need to add a MeshData section
	m_bdata = false;
	if (model.ShellElements() > 0) m_bdata = true;	// for shell thicknesses
	for (int i=0; i<fem.Materials(); ++i)
	{
		FSTransverselyIsotropic* pmat = dynamic_cast<FSTransverselyIsotropic*>(fem.GetMaterial(i)->GetMaterialProperties());
		if (pmat && (pmat->GetFiberMaterial()->m_naopt == FE_FIBER_USER)) m_bdata = true;
	}
	for (int i=0; i<model.Objects(); ++i)
	{
		FSMesh* pm = model.Object(i)->GetFEMesh();
		for (int j=0; j<pm->Elements(); ++j)
		{
			FEElement_& e = pm->ElementRef(j);
			if (e.m_Qactive) {
				m_bdata = true;
				break;
			}
		}
		if (pm->MeshDataFields() > 0) m_bdata = true;
	}

	return true;
}

//-----------------------------------------------------------------------------
void FEBioExport25::BuildSurfaceList(FSProject& prj)
{
	FSModel& fem = prj.GetFSModel();

	// get the named surfaces (loads)
	for (int i=0; i<fem.Steps(); ++i)
	{
		FSStep* pstep = fem.GetStep(i);
		for (int j=0; j<pstep->Loads(); ++j)
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
	char szbuf[256] = {0};
	for (int i=0; i<fem.Steps(); ++i)
	{
		FSStep* pstep = fem.GetStep(i);
		for (int j=0; j<pstep->Interfaces(); ++j)
		{
			FSInterface* pj = pstep->Interface(j);
			FSPairedInterface* pi = dynamic_cast<FSPairedInterface*>(pj);

			// Note: Don't export surfaces of tied-spring interfaces
			if (dynamic_cast<FSSpringTiedInterface*>(pi)) pi = 0;

			if (pi && pi->IsActive())
			{
				FEItemListBuilder* pms = pi->GetSecondarySurface();
				if (pms == 0) throw InvalidItemListBuilder(pi);

				string name = pms->GetName();				
				const char* szname = name.c_str();
				if ((szname==0) || (szname[0]==0))
				{
					sprintf(szbuf, "%s_master", pi->GetName().c_str());
					szname = szbuf;
				}
				AddSurface(szname,	pms);

				FEItemListBuilder* pss = pi->GetPrimarySurface();
				if (pss == 0) throw InvalidItemListBuilder(pi);
				
				name = pss->GetName();
				szname = name.c_str();
				if ((szname==0) || (szname[0]==0))
				{
					sprintf(szbuf, "%s_slave", pi->GetName().c_str());
					szname = szbuf;
				}
				AddSurface(szname,	pss);
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
			}
		}
	}

	// check all the (surface) plot variables
	CPlotDataSettings& plt = prj.GetPlotDataSettings();
	for (int i=0; i<plt.PlotVariables(); ++i)
	{
		CPlotVariable& var = plt.PlotVariable(i);
		if (var.domainType() == DOMAIN_SURFACE)
		{
			int ND = var.Domains();
			for (int j=0; j<ND; ++j)
			{
				FEItemListBuilder* pl = var.GetDomain(j);
				AddSurface(pl->GetName(), pl);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport25::BuildNodeSetList(FSProject& prj)
{
	FSModel& fem = prj.GetFSModel();

	// get the nodesets (bc's)
	for (int i=0; i<fem.Steps(); ++i)
	{
		FSStep* pstep = fem.GetStep(i);
		for (int j=0; j<pstep->BCs(); ++j)
		{
			FSBoundaryCondition* pl = pstep->BC(j);
			if (pl && pl->IsActive())
			{
				FEItemListBuilder* ps = pl->GetItemList();
				if (ps == 0) throw InvalidItemListBuilder(pl);

				string name = ps->GetName();
				if (name.empty()) name = pl->GetName();
				
				AddNodeSet(name, ps);
			}
		}
		for (int j=0; j<pstep->Loads(); ++j)
		{
			// this is only for nodal loads
			FSNodalDOFLoad* pl = dynamic_cast<FSNodalDOFLoad*>(pstep->Load(j));
			if (pl && pl->IsActive())
			{
				FEItemListBuilder* ps = pl->GetItemList();
				if (ps == 0) throw InvalidItemListBuilder(pl);

				string name = ps->GetName();
				if (name.empty()) name = pl->GetName();
				
				AddNodeSet(name, ps);
			}
		}
		for (int j = 0; j<pstep->ICs(); ++j)
		{
			// this is only for nodal loads
			FSInitialNodalDOF* pi = dynamic_cast<FSInitialNodalDOF*>(pstep->IC(j));
			if (pi && pi->IsActive())
			{
				FEItemListBuilder* ps = pi->GetItemList();
				if (ps == 0) throw InvalidItemListBuilder(pi);

				string name = ps->GetName();
				if (name.empty()) name = pi->GetName();

				AddNodeSet(name, ps);
			}
		}
		for (int j=0; j<pstep->Interfaces(); ++j)
		{
			FSRigidInterface* pri = dynamic_cast<FSRigidInterface*>(pstep->Interface(j));
			if (pri && pri->IsActive())
			{
				FEItemListBuilder* pitem = pri->GetItemList();
				if (pitem == 0) throw InvalidItemListBuilder(pri);

				string name = pitem->GetName();
				if (name.empty()) name = pri->GetName();
				
				AddNodeSet(name, pitem);
			}
		}
	}

	// check for node sets, used in log data
	GModel& mdl = fem.GetModel();
	CLogDataSettings& logData = prj.GetLogDataSettings();
	int ndata = logData.LogDataSize();
	for (int i = 0; i < ndata; ++i)
	{
		FSLogData& ld = logData.LogData(i);
		if (ld.Type() == FSLogData::LD_NODE)
		{
			FSLogNodeData& nd = dynamic_cast<FSLogNodeData&>(ld);
			FEItemListBuilder* pl = nd.GetItemList();
			if (pl) AddNodeSet(pl->GetName(), pl);
		}
	}

/*	// Node sets are already written in WriteGeometryNodeSets
	GModel& model = fem.GetModel();
	CLogDataSettings& log = prj.GetLogDataSettings();
	for (int i=0; i<log.LogDataSize(); ++i)
	{
		FELogData& di = log.LogData(i);
		if ((di.type == FELogData::LD_NODE) && (di.groupID != -1))
		{
			FEItemListBuilder* pg = model.FindNamedSelection(di.itemID);
			if (pg)
			{
				AddNodeSet(pg->GetName(), pg);
			} 
		}
	}	
*/
}

//-----------------------------------------------------------------------------
void FEBioExport25::BuildElemSetList(FSProject& prj)
{
	FSModel& fem = prj.GetFSModel();
	GModel& model = fem.GetModel();
	CLogDataSettings& log = prj.GetLogDataSettings();
	for (int i = 0; i<log.LogDataSize(); ++i)
	{
		FSLogData& di = log.LogData(i);
		if (di.Type() == FSLogData::LD_ELEM)
		{
			FSLogElemData& ed = dynamic_cast<FSLogElemData&>(di);
			FEItemListBuilder* pl = ed.GetItemList();
			if (pl) AddNodeSet(pl->GetName(), pl);
		}
	}

	// add user-selection
	if (m_exportSelections)
	{
		// model level parts
		int sets = model.PartLists();
		for (int i = 0; i < sets; ++i)
		{
			GPartList* pg = model.PartList(i);
			AddElemSet(pg->GetName(), pg);
		}

		// object level element sets
		int objs = model.Objects();
		for (int i = 0; i < objs; ++i)
		{
			GObject* po = model.Object(i);
			int sets = po->FEElemSets();
			for (int j = 0; j < sets; ++j)
			{
				FSElemSet* pg = po->GetFEElemSet(j);
				AddElemSet(pg->GetName(), pg);
			}
		}
	}
}

//-----------------------------------------------------------------------------
bool FEBioExport25::Write(const char* szfile)
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
			FSAnalysisStep* pstep = dynamic_cast<FSAnalysisStep*>(fem.GetStep(1));
			if (pstep == 0) return errf("Step 1 is not an analysis step.");
			ntype = pstep->GetType();
			if (pstep->BCs() + pstep->Loads() + pstep->Interfaces() + pstep->RigidConstraints() == 0) bsingle_step = true;
		}

		// open the file
		if (!m_xml.open(szfile)) return errf("Failed opening file %s", szfile);

		if (m_writeNotes)
		{
			m_xml.add_comment(mdl.GetInfo());
		}

		XMLElement el;

		// output root element
		el.name("febio_spec");
		el.add_attribute("version", "2.5");

		m_xml.add_branch(el);
		{
			// write the module section
			// Note that this format assumes that all steps are of the same type.
			// (This is verified in PrepareExport)
			if (m_nsteps > 1)
			{
				FSAnalysisStep* pstep = dynamic_cast<FSAnalysisStep*>(fem.GetStep(1));
				if (pstep == 0) return errf("Step 1 is not an analysis step.");
				if (m_section[FEBIO_MODULE]) WriteModuleSection(pstep);
			}

			// output control section
			// this section is only written for single-step analysis
			// for multi-step analysis, the control section is 
			// written separately for each step
			if (bsingle_step && (m_nsteps == 2))
			{
				FSAnalysisStep* pstep = dynamic_cast<FSAnalysisStep*>(fem.GetStep(1));
				if (pstep == 0) return errf("Step 1 is not an analysis step.");

				// write the control section
				if (m_section[FEBIO_CONTROL])
				{
					m_xml.add_branch("Control");
					{
						WriteControlSection(pstep);
					}
					m_xml.close_branch(); // Control
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

			// output geometry section
			if ((fem.GetModel().Objects() > 0) && (m_section[FEBIO_GEOMETRY]))
			{
				m_xml.add_branch("Geometry");
				{
					WriteGeometrySection();
//					WriteGeometrySection2();
				}
				m_xml.close_branch(); // Geometry
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
			int nbc = pstep->ActiveBCs() + pstep->Interfaces() + fem.GetModel().DiscreteObjects() + pstep->RigidConstraints();
			if ((nbc > 0) && (m_section[FEBIO_BOUNDARY]))
			{
				m_xml.add_branch("Boundary");
				{
					WriteBoundarySection(*pstep);
				}
				m_xml.close_branch(); // Boundary
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
			int nci = pstep->Interfaces();
			int nLC = pstep->LinearConstraints();
			if (((nci > 0)||(nLC > 0)) && (m_section[FEBIO_CONTACT]))
			{
				m_xml.add_branch("Contact");
				{
					WriteContactSection(*pstep);
				}
				m_xml.close_branch(); // Contact
			}

			// output constraints section
			int nnlc = CountConnectors<FSRigidConnector>(fem)
			+ CountInterfaces<FSRigidJoint>(fem)
			+ CountConstraints<FSModelConstraint>(fem);
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
					WriteInitialSection(*pstep);
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

			// loadcurve data
			if ((fem.LoadControllers() > 0) && (m_section[FEBIO_LOADDATA]))
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

			// step data
			// this is only written if there is more than one step
			// that defines BCs, loads or interfaces
			if (bsingle_step == false) WriteStepSection();
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
void FEBioExport25::WriteModuleSection(FSAnalysisStep* pstep)
{
	XMLElement t;
	t.name("Module");
	switch (pstep->GetType())
	{
	case FE_STEP_MECHANICS          : t.add_attribute("type", "solid"      ); break;
	case FE_STEP_HEAT_TRANSFER      : t.add_attribute("type", "heat"       ); break;
	case FE_STEP_BIPHASIC           : t.add_attribute("type", "biphasic"   ); break;
	case FE_STEP_BIPHASIC_SOLUTE   : t.add_attribute("type", "solute"     ); break;
	case FE_STEP_MULTIPHASIC        : t.add_attribute("type", "multiphasic"); break;
    case FE_STEP_FLUID              : t.add_attribute("type", "fluid"      ); break;
    case FE_STEP_FLUID_FSI          : t.add_attribute("type", "fluid-FSI"  ); break;
	case FE_STEP_REACTION_DIFFUSION : t.add_attribute("type", "reaction-diffusion"); m_useReactionMaterial2 = true; break;
	};

	m_xml.add_empty(t);
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteControlSection(FSAnalysisStep* pstep)
{
	STEP_SETTINGS& ops = pstep->GetSettings();
	int ntype = pstep->GetType();
	switch (ntype)
	{
	case FE_STEP_MECHANICS          : WriteSolidControlParams         (pstep); break;
	case FE_STEP_HEAT_TRANSFER      : WriteHeatTransferControlParams  (pstep); break;
	case FE_STEP_BIPHASIC           : WriteBiphasicControlParams      (pstep); break;
	case FE_STEP_BIPHASIC_SOLUTE   : WriteBiphasicSoluteControlParams(pstep); break;
	case FE_STEP_MULTIPHASIC        : WriteBiphasicSoluteControlParams(pstep); break;
    case FE_STEP_FLUID              : WriteFluidControlParams         (pstep); break;
    case FE_STEP_FLUID_FSI          : WriteFluidFSIControlParams      (pstep); break;
	case FE_STEP_REACTION_DIFFUSION : WriteReactionDiffusionControlParams(pstep); break;
	default:
		assert(false);
	}

	if (ops.plot_level != 1)
	{
		const char* sz[] = {"PLOT_NEVER", "PLOT_MAJOR_ITRS", "PLOT_MINOR_ITRS", "PLOT_MUST_POINTS", "PLOT_FINAL", "PLOT_AUGMENTATIONS", "PLOT_STEP_FINAL"};
		m_xml.add_leaf("plot_level", sz[ops.plot_level]);
	}

	if (ops.plot_stride != 1)
	{
		m_xml.add_leaf("plot_stride", ops.plot_stride);
	}
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteSolidControlParams(FSAnalysisStep* pstep)
{
	XMLElement el;
	STEP_SETTINGS& ops = pstep->GetSettings();

	m_xml.add_leaf("time_steps", ops.ntime);
	m_xml.add_leaf("step_size", ops.dt);
	m_xml.add_leaf("max_refs", ops.maxref);
	m_xml.add_leaf("max_ups", (ops.mthsol == 0 ? ops.ilimit : 0));
	m_xml.add_leaf("diverge_reform", ops.bdivref);
	m_xml.add_leaf("reform_each_time_step", ops.brefstep);

	// write the parameters
	WriteParamList(*pstep);

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

	el.name("analysis");
	el.add_attribute("type", (ops.nanalysis==0?"static":"dynamic"));
	m_xml.add_empty(el);

    if ((ops.nanalysis != 0) && ops.override_rhoi) {
        el.name("alpha");
        el.value(ops.alpha);
        m_xml.add_leaf(el);
        
        el.name("beta");
        el.value(ops.beta);
        m_xml.add_leaf(el);
        
        el.name("gamma");
        el.value(ops.gamma);
        m_xml.add_leaf(el);
    }
    
	if (ops.bminbw)
	{
		m_xml.add_leaf("optimize_bw", 1);
	}
    
    if (ops.nmatfmt != 0)
    {
        m_xml.add_leaf("symmetric_stiffness", (ops.nmatfmt == 1? 1: 0));
    }

	if (ops.neqscheme != 0)
	{
		m_xml.add_leaf("equation_scheme", ops.neqscheme);
	}
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteHeatTransferControlParams(FSAnalysisStep* pstep)
{
	XMLElement el;

	STEP_SETTINGS& ops = pstep->GetSettings();

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

	el.name("analysis");
	el.add_attribute("type", (ops.nanalysis==0?"static":"transient"));
	m_xml.add_empty(el);

	if (ops.bminbw)
	{
		m_xml.add_leaf("optimize_bw", 1);
	}
}


//-----------------------------------------------------------------------------
void FEBioExport25::WriteBiphasicControlParams(FSAnalysisStep* pstep)
{
	XMLElement el;
	STEP_SETTINGS& ops = pstep->GetSettings();

	m_xml.add_leaf("time_steps", ops.ntime);
	m_xml.add_leaf("step_size", ops.dt);
	m_xml.add_leaf("max_refs", ops.maxref);
	m_xml.add_leaf("max_ups", (ops.mthsol == 0 ? ops.ilimit : 0));
	m_xml.add_leaf("diverge_reform", ops.bdivref);
	m_xml.add_leaf("reform_each_time_step", ops.brefstep);

	// write the parameters
	WriteParamList(*pstep);

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

	if (ops.nanalysis == 0) 
	{
		XMLElement el;
		el.name("analysis");
		el.add_attribute("type", "steady-state");
		m_xml.add_empty(el);
	}

	if (ops.bminbw)
	{
		m_xml.add_leaf("optimize_bw", 1);
	}

	if (ops.nmatfmt != 0)
	{
		m_xml.add_leaf("symmetric_stiffness", (ops.nmatfmt == 1? 1: 0));
	}

	if (ops.neqscheme != 0)
	{
		m_xml.add_leaf("equation_scheme", ops.neqscheme);
	}
}


//-----------------------------------------------------------------------------
void FEBioExport25::WriteBiphasicSoluteControlParams(FSAnalysisStep* pstep)
{
	XMLElement el;
	STEP_SETTINGS& ops = pstep->GetSettings();

	m_xml.add_leaf("time_steps", ops.ntime);
	m_xml.add_leaf("step_size", ops.dt);
	m_xml.add_leaf("max_refs", ops.maxref);
	m_xml.add_leaf("max_ups", (ops.mthsol == 0 ? ops.ilimit : 0));
	m_xml.add_leaf("diverge_reform", ops.bdivref);
	m_xml.add_leaf("reform_each_time_step", ops.brefstep);

	// write the parameters
	WriteParamList(*pstep);

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

	if (ops.nanalysis == 0) 
	{
		XMLElement el;
		el.name("analysis");
		el.add_attribute("type", "steady-state");
		m_xml.add_empty(el);
	}

	if (ops.bminbw)
	{
		m_xml.add_leaf("optimize_bw", 1);
	}

	if (ops.nmatfmt != 0)
	{
		m_xml.add_leaf("symmetric_stiffness", (ops.nmatfmt == 1? 1: 0));
	}

	if (ops.neqscheme != 0)
	{
		m_xml.add_leaf("equation_scheme", ops.neqscheme);
	}
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteFluidControlParams(FSAnalysisStep* pstep)
{
    XMLElement el;
    STEP_SETTINGS& ops = pstep->GetSettings();
    
    m_xml.add_leaf("time_steps", ops.ntime);
    m_xml.add_leaf("step_size", ops.dt);
    m_xml.add_leaf("max_refs", ops.maxref);
    m_xml.add_leaf("max_ups", (ops.mthsol == 0 ? ops.ilimit : 0));
	m_xml.add_leaf("diverge_reform", ops.bdivref);
	m_xml.add_leaf("reform_each_time_step", ops.brefstep);

	// write the parameters
	WriteParamList(*pstep);

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
    
    if (ops.nanalysis == 0)
    {
        XMLElement el;
        el.name("analysis");
        el.add_attribute("type", "steady-state");
        m_xml.add_empty(el);
    }
    else
    {
        XMLElement el;
        el.name("analysis");
        el.add_attribute("type", "dynamic");
        m_xml.add_empty(el);
    }
    
    if (ops.bminbw)
    {
        m_xml.add_leaf("optimize_bw", 1);
    }
    
    if (ops.nmatfmt != 0)
    {
        m_xml.add_leaf("symmetric_stiffness", (ops.nmatfmt == 1? 1: 0));
    }
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteFluidFSIControlParams(FSAnalysisStep* pstep)
{
    XMLElement el;
    STEP_SETTINGS& ops = pstep->GetSettings();
    
    m_xml.add_leaf("time_steps", ops.ntime);
    m_xml.add_leaf("step_size", ops.dt);
    m_xml.add_leaf("max_refs", ops.maxref);
    m_xml.add_leaf("max_ups", (ops.mthsol == 0 ? ops.ilimit : 0));
    m_xml.add_leaf("diverge_reform", ops.bdivref);
    m_xml.add_leaf("reform_each_time_step", ops.brefstep);
    
    // write the parameters
    WriteParamList(*pstep);
    
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
    
    if (ops.nanalysis == 0)
    {
        XMLElement el;
        el.name("analysis");
        el.add_attribute("type", "steady-state");
        m_xml.add_empty(el);
    }
    else
    {
        XMLElement el;
        el.name("analysis");
        el.add_attribute("type", "dynamic");
        m_xml.add_empty(el);
    }
    
    if (ops.bminbw)
    {
        m_xml.add_leaf("optimize_bw", 1);
    }
    
    if (ops.nmatfmt != 0)
    {
        m_xml.add_leaf("symmetric_stiffness", (ops.nmatfmt == 1? 1: 0));
    }
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteReactionDiffusionControlParams(FSAnalysisStep* pstep)
{
	XMLElement el;
	STEP_SETTINGS& ops = pstep->GetSettings();

	m_xml.add_leaf("time_steps", ops.ntime);
	m_xml.add_leaf("step_size", ops.dt);
	m_xml.add_leaf("max_refs", ops.maxref);
	m_xml.add_leaf("max_ups", (ops.mthsol == 0 ? ops.ilimit : 0));

	// write the parameters
	WriteParamList(*pstep);

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

	if (ops.nanalysis == 0)
	{
		XMLElement el;
		el.name("analysis");
		el.add_attribute("type", "steady-state");
		m_xml.add_empty(el);
	}
	else
	{
		XMLElement el;
		el.name("analysis");
		el.add_attribute("type", "transient");
		m_xml.add_empty(el);
	}

	if (ops.nmatfmt != 0)
	{
		m_xml.add_leaf("symmetric_stiffness", (ops.nmatfmt == 1 ? 1 : 0));
	}
}

//-----------------------------------------------------------------------------

void FEBioExport25::WriteMaterialSection()
{
	XMLElement el;

	FSModel& s = *m_pfem;

	for (int i=0; i<s.Materials(); ++i)
	{
		GMaterial* pgm = s.GetMaterial(i);
		const string& name = pgm->GetName();

		if (m_writeNotes) m_xml.add_comment(pgm->GetInfo());

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
			errf("ERROR: Material %s does not have any properties.",  name.c_str());
			m_xml.add_leaf(el);
		}
	}
}

void FEBioExport25::WriteFiberMaterial(FSOldFiberMaterial& fiber)
{
	FSOldFiberMaterial& f = fiber;
	XMLElement el;
	el.name("fiber");
	if (f.m_naopt == FE_FIBER_LOCAL) 
	{
		el.add_attribute("type", "local");
		el.value(f.m_n,2);
		m_xml.add_leaf(el);
	}
	else if (f.m_naopt == FE_FIBER_CYLINDRICAL)
	{
		el.add_attribute("type", "cylindrical");
		m_xml.add_branch(el);
		{
			m_xml.add_leaf("center", f.m_r);
			m_xml.add_leaf("axis"  , f.m_a);
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
			m_xml.add_leaf("axis"  , f.m_a);
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
			m_xml.add_leaf("phi"  , f.m_phi  );
		}
		m_xml.close_branch();
	}
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteMaterialParams(FSMaterial* pm, bool topLevel)
{
	// only export non-persistent parameters for top-level materials
	m_exportNonPersistentParams = topLevel;

	// Write the parameters first
	WriteParamList(*pm);

	// reset flag
	m_exportNonPersistentParams = true;

	// if the material is transversely-isotropic, we need to write the fiber data as well
	FSOldFiberMaterial* fiber = dynamic_cast<FSOldFiberMaterial*>(pm);
	FSTransverselyIsotropic* ptiso = dynamic_cast<FSTransverselyIsotropic*>(pm);
	if (ptiso) fiber = ptiso->GetFiberMaterial();
	
	if (fiber)
	{
		WriteFiberMaterial(*fiber);
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
void FEBioExport25::WriteRigidMaterial(FSMaterial* pmat, XMLElement& el)
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
void FEBioExport25::WriteMaterial(FSMaterial* pm, XMLElement& el)
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
    
	// get the type string    
	const char* sztype = pm->GetTypeString();
	assert(sztype);

	// set the type attribute
    if (pm->Type() == FE_SOLUTE_MATERIAL)
    {
        FSSoluteMaterial* psm = dynamic_cast<FSSoluteMaterial*>(pm); assert(psm);
        el.add_attribute("sol", psm->GetSoluteIndex()+1);
    }
    else if (pm->Type() == FE_SBM_MATERIAL)
    {
        FSSBMMaterial* psb = dynamic_cast<FSSBMMaterial*>(pm); assert(psb);
        el.add_attribute("sbm", psb->GetSBMIndex()+1);
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
		case FSReactionSpecies::SOLUTE_SPECIES: el.add_attribute("sol", idx+1); break;
		case FSReactionSpecies::SBM_SPECIES   : el.add_attribute("sbm", idx + 1); break;
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
		case FSReactionSpecies::SBM_SPECIES   : el.add_attribute("sbm", idx + 1); break;
        default:
			assert(false);
        }
        m_xml.add_leaf(el);
        return;
    }
    else if (pm->Type() == FE_OSMO_WM)
    {
        FSOsmoWellsManning* pwm = dynamic_cast<FSOsmoWellsManning*>(pm); assert(pwm);
        el.add_attribute("type", sztype);
        m_xml.add_branch(el);
        {
            if (pm->Parameters()) WriteMaterialParams(pm);
            XMLElement cel("co_ion");
            cel.value(pwm->GetCoIonIndex()+1);
            m_xml.add_leaf(cel);
        }
        m_xml.close_branch();
        return;
    }
    else
        el.add_attribute("type", sztype);

	if ((pm->Parameters() == 0) && (pm->Properties() == 0))
	{
		m_xml.add_empty(el);
		return;
	}
    
	m_xml.add_branch(el);
	{
		// write the material parameters (if any)
		if (pm->Parameters()) WriteMaterialParams(pm, true);

		// write the components
		int NC = (int)pm->Properties();
		for (int i=0; i<NC; ++i)
		{
			FSProperty& mc = pm->GetProperty(i);
			for (int j=0; j<mc.Size(); ++j)
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
                    case FE_OSMO_WM: is_multi = true; break;
					}

					if (pc->ClassID() == FE_MAT_1DFUNC)
					{
						// we need some special handling for 1D functions
						FS1DPointFunction* f1d = dynamic_cast<FS1DPointFunction*>(pc);
						assert(f1d);
						WritePointCurve(f1d, el);
					}
					else
					{
						if ((pc->Properties() > 0) || is_multi) WriteMaterial(pc, el);
						else
						{
							el.add_attribute("type", pc->GetTypeString());

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
									bdone = true;
								}
							}

							if (bdone == false)
							{
								if (pc->Parameters() == 0)
								{
									m_xml.add_empty(el);
								}
								else
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
							}
						}
					}
				}
			}
		}
	}
	m_xml.close_branch();
}

//-----------------------------------------------------------------------------
void FEBioExport25::WritePointCurve(FS1DPointFunction* f1d, XMLElement& el)
{
	LoadCurve* plc = f1d->GetPointCurve();

	el.add_attribute("type", "point");
	m_xml.add_branch(el);
	{
		int ntype = plc->GetInterpolator();
		switch (ntype)
		{
		case PointCurve::LINEAR : m_xml.add_leaf("interpolate", "linear"); break;
		case PointCurve::STEP   : m_xml.add_leaf("interpolate", "step"); break;
		case PointCurve::SMOOTH : m_xml.add_leaf("interpolate", "smooth"); break;
        case PointCurve::CSPLINE: m_xml.add_leaf("interpolate", "cubic spline"); break;
        case PointCurve::CPOINTS: m_xml.add_leaf("interpolate", "control points"); break;
        case PointCurve::APPROX : m_xml.add_leaf("interpolate", "approximation"); break;
        case PointCurve::SMOOTH_STEP: m_xml.add_leaf("interpolate", "smooth step"); break;
		}

		int nextend = plc->GetExtendMode();
		switch (nextend)
		{
		case PointCurve::CONSTANT     : m_xml.add_leaf("extend", "constant"); break;
		case PointCurve::EXTRAPOLATE  : m_xml.add_leaf("extend", "extrapolate"); break;
		case PointCurve::REPEAT       : m_xml.add_leaf("extend", "repeat"); break;
		case PointCurve::REPEAT_OFFSET: m_xml.add_leaf("extend", "repeat offset"); break;
		}

		m_xml.add_branch("points");
		int n = plc->Points();
		for (int i = 0; i < n; ++i)
		{
			vec2d pi = plc->Point(i);
			double d[2] = { pi.x(), pi.y()};
			m_xml.add_leaf("pt", d, 2);
		}

		m_xml.close_branch();
	}
	m_xml.close_branch();
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteReactionMaterial(FSMaterial* pmat, XMLElement& el)
{
	const char* sztype = 0;
	switch (pmat->Type())
	{
	case FE_MASS_ACTION_FORWARD    : sztype = "mass-action-forward"    ; break;
	case FE_MASS_ACTION_REVERSIBLE : sztype = "mass-action-reversible" ; break;
	case FE_MICHAELIS_MENTEN       : sztype = "Michaelis-Menten"       ; break;
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
                        case FE_SBM_MATERIAL          : is_multi = true; break;
                        case FE_REACTANT_MATERIAL     : is_multi = true; break;
                        case FE_PRODUCT_MATERIAL      : is_multi = true; break;
                        case FE_SPECIES_MATERIAL      : is_multi = true; break;
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
void FEBioExport25::WriteReactionMaterial2(FSMaterial* pmat, XMLElement& el)
{
	// get the reaction material
	FSReactionMaterial* prm = dynamic_cast<FSReactionMaterial*>(pmat);
	assert(prm);
	if (prm == 0) return;

	// get the reaction type
	const char* sztype = 0;
	switch (pmat->Type())
	{
	case FE_MASS_ACTION_FORWARD: sztype = "mass action"     ; break;
	case FE_MICHAELIS_MENTEN   : sztype = "Michaelis-Menten"; break;
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
		for (int i=0; i<nreact; ++i)
		{
			FSReactantMaterial* ri = prm->Reactant(i);
			
			const char* sz = 0;
			int v = ri->GetCoef();
			int idx = ri->GetIndex();
			int type = ri->GetReactantType();
			if      (type == FSReactionSpecies::SOLUTE_SPECIES) sz = fem.GetSoluteData(idx).GetName().c_str();
			else if (type == FSReactionSpecies::SBM_SPECIES   ) sz = fem.GetSBMData(idx).GetName().c_str();
			else { assert(false); }

			if (v != 1) ss << v << "*" << sz;
			else ss << sz;
			if (i != nreact-1) ss << "+";
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
			if      (type == FSReactionSpecies::SOLUTE_SPECIES) sz = fem.GetSoluteData(idx).GetName().c_str();
			else if (type == FSReactionSpecies::SBM_SPECIES   ) sz = fem.GetSBMData(idx).GetName().c_str();
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
void FEBioExport25::WriteGeometrySection()
{
	if (m_exportParts)
		WriteGeometrySectionNew();
	else
		WriteGeometrySectionOld();
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteGeometrySectionOld()
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
void FEBioExport25::WriteGeometrySectionNew()
{
	FSModel& fem = *m_pfem;
	GModel& model = fem.GetModel();

	// reset counters
	m_ntotnodes = 0;
	m_ntotelem = 0;

	// write all parts
	size_t nparts = m_Part.size();
	for (int i=0; i<nparts; ++i)
	{
		Part* p = m_Part[i];
		const string& name = p->m_obj->GetName();

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
		Part* p = m_Part[i];
		GObject* po = p->m_obj;
		const string& name = po->GetName();
		XMLElement instance("Instance");
		instance.add_attribute("part", name.c_str());
		m_xml.add_branch(instance);
		{
			vec3d p = po->GetTransform().GetPosition();
			quatd q = po->GetTransform().GetRotation();
			vec3d s = po->GetTransform().GetScale();
			m_xml.add_leaf("scale", s);
			m_xml.add_leaf("rotate", q);
			m_xml.add_leaf("translate", p);
		}
		m_xml.close_branch();
	}

	// write the global node sets
	WriteGeometryNodeSetsNew();

	// write the global surfaces
	WriteGeometrySurfacesNew();

	// write the surface pairs
	WriteGeometrySurfacePairs();
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteGeometryObject(FEBioExport25::Part* part)
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
		WriteGeometryPart(pg, true);
	}

	// write all node sets
	for (int i=0; i<part->m_NSet.size(); ++i)
	{
		NodeSet* ns = part->m_NSet[i];
		WriteNodeSet(ns->m_name.c_str(), ns->m_nodeList);
	}

	// write all surfaces
	for (int i=0; i<part->m_Surf.size(); ++i)
	{
		Surface* s = part->m_Surf[i];
		XMLElement el("Surface");
		el.add_attribute("name", s->m_name.c_str());
		m_xml.add_branch(el);
		{
			FEFaceList* faceList = s->m_faceList;
			WriteSurfaceSection(*faceList);
		}
		m_xml.close_branch();
	}
}

//-----------------------------------------------------------------------------
bool FEBioExport25::WriteNodeSet(const string& name, FSNodeList* pl)
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
	for (int n=0; n<nn; ++n, pn++)
	{
		FSNode* pnode = pn->m_pi;
		if (pnode == 0) return false;
		m[n] = pnode->m_nid;
	}

	XMLElement el("NodeSet");
	el.add_attribute("name", name.c_str());
	m_xml.add_branch(el);
	{
		XMLElement nd("node");
		nd.add_attribute("id",0);
		for (int n=0; n<nn; ++n)
		{
			nd.set_attribute(0, m[n]);
			m_xml.add_empty(nd, false);
		}
	}
	m_xml.close_branch();
	return true;
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteGeometryNodeSetsNew()
{
	// Write the BC node sets
	int NS = (int)m_pNSet.size();
	for (int i = 0; i<NS; ++i)
	{
		FEItemListBuilder* pil = m_pNSet[i].second;
		const string& listName = m_pNSet[i].first;
		XMLElement tag("NodeSet");
		tag.add_attribute("name", m_pNSet[i].first.c_str());
		m_xml.add_branch(tag);
		switch (pil->Type())
		{
		case GO_NODE:
			{
				GNodeList* itemList = dynamic_cast<GNodeList*>(pil); assert(itemList);
				vector<GNode*> nodeList = itemList->GetNodeList();
				for (size_t i=0; i<nodeList.size(); ++i)
				{
					GNode* node = nodeList[i];
					GObject* po = dynamic_cast<GObject*>(node->Object());
					XMLElement nset("NodeSet");
					
					string name = string(po->GetName()) + "." + string(node->GetName());
					nset.add_attribute("node_set", name.c_str());
					m_xml.add_empty(nset);
				}
			}
			break;
		case GO_EDGE:
			{
				GEdgeList* itemList = dynamic_cast<GEdgeList*>(pil); assert(itemList);
				vector<GEdge*> edgeList = itemList->GetEdgeList();
				for (size_t i=0; i<edgeList.size(); ++i)
				{
					GEdge* edge = edgeList[i];
					GObject* po = dynamic_cast<GObject*>(edge->Object());
					XMLElement nset("NodeSet");
					
					string name = string(po->GetName()) + "." + string(edge->GetName());
					nset.add_attribute("node_set", name.c_str());
					m_xml.add_empty(nset);
				}
			}
			break;
		case GO_FACE:
			{
				GFaceList* itemList = dynamic_cast<GFaceList*>(pil); assert(itemList);
				vector<GFace*> faceList = itemList->GetFaceList();
				for (size_t i=0; i<faceList.size(); ++i)
				{
					GFace* face = faceList[i];
					GObject* po = dynamic_cast<GObject*>(face->Object());
					XMLElement nset("NodeSet");
					
					string name = string(po->GetName()) + "." + string(face->GetName());
					nset.add_attribute("node_set", name.c_str());
					m_xml.add_empty(nset);
				}
			}
			break;
		case GO_PART:
			{
				GPartList* itemList = dynamic_cast<GPartList*>(pil); assert(itemList);
				vector<GPart*> partList = itemList->GetPartList();
				for (size_t i=0; i<partList.size(); ++i)
				{
					GPart* part = partList[i];
					GObject* po = dynamic_cast<GObject*>(part->Object());
					XMLElement nset("NodeSet");
					
					string name = string(po->GetName()) + "." + string(part->GetName());
					nset.add_attribute("node_set", name.c_str());
					m_xml.add_empty(nset);
				}
			}
			break;
		case FE_SURFACE:
			{
				FSSurface* face = dynamic_cast<FSSurface*>(pil); assert(face);
				for (int i=0; i<m_Part.size(); ++i)
				{
					Part* part = m_Part[i];
					NodeSet* ns = part->FindNodeSet(m_pNSet[i].first.c_str());
					if (ns)
					{
						XMLElement nset("NodeSet");
						GObject* po = part->m_obj;
						string name = string(po->GetName()) + "." + listName;
						nset.add_attribute("node_set", name.c_str());
						m_xml.add_empty(nset);
						break;
					}
				}
			}
			break;
		}
		m_xml.close_branch();
	}
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteGeometryNodeSets()
{
	// Write the BC node sets
	int NS = (int) m_pNSet.size();
	for (int i=0; i<NS; ++i)
	{
		FEItemListBuilder* pil = m_pNSet[i].second;
		unique_ptr<FSNodeList> pl(pil->BuildNodeList());
		if (WriteNodeSet(m_pNSet[i].first.c_str(), pl.get()) == false)
		{
			throw InvalidItemListBuilder(pil);
		}
	}

	// Write the user-defined node sets
	if (m_exportSelections)
	{
		FSModel& fem = *m_pfem;
		GModel& model = fem.GetModel();

		// first, do model-level node sets
		for (int i = 0; i < model.NodeLists(); ++i)
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
		for (int i = 0; i < nobj; ++i)
		{
			GObject* po = model.Object(i);
			FSMesh* pm = po->GetFEMesh();
			if (pm)
			{
				int nset = po->FENodeSets();
				for (int j = 0; j < nset; ++j)
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
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteGeometrySurfaces()
{
	int NS = (int)m_pSurf.size();
	for (int i=0; i<NS; ++i)
	{
		FEItemListBuilder* pl = m_pSurf[i].second;
		FEFaceList* pfl = pl->BuildFaceList();
		if (pfl)
		{
			unique_ptr<FEFaceList> ps(pfl);
			XMLElement el("Surface");
			el.add_attribute("name", m_pSurf[i].first.c_str());
			m_xml.add_branch(el);
			{
				WriteSurfaceSection(*ps);
			}
			m_xml.close_branch();
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteGeometryElementSets()
{
	int NS = (int)m_pESet.size();
	for (int i = 0; i<NS; ++i)
	{
		FEItemListBuilder* pl = m_pESet[i].second;
		unique_ptr<FEElemList> ps(pl->BuildElemList());
		XMLElement el("ElementSet");
		el.add_attribute("name", m_pESet[i].first.c_str());
		m_xml.add_branch(el);
		{
			WriteElementList(*ps);
		}
		m_xml.close_branch();
	}
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteGeometrySurfacesNew()
{
	int NS = (int)m_pSurf.size();
	for (int i = 0; i<NS; ++i)
	{
		FEItemListBuilder* pl = m_pSurf[i].second;
		XMLElement el("Surface");
		string sname = m_pSurf[i].first;
		el.add_attribute("name", m_pSurf[i].first.c_str());
		m_xml.add_branch(el);
		{
			switch (pl->Type())
			{
			case GO_FACE:
				{
					GFaceList* itemList = dynamic_cast<GFaceList*>(pl); assert(itemList);
					vector<GFace*> faceList = itemList->GetFaceList();
					for (size_t i=0; i<faceList.size(); ++i)
					{
						GFace* face = faceList[i];
						GObject* po = dynamic_cast<GObject*>(face->Object());
						XMLElement nset("Surface");

						string name = string(po->GetName()) + "." + string(face->GetName());
						nset.add_attribute("surface", name.c_str());
						m_xml.add_empty(nset);
					}
				}
				break;
			case FE_SURFACE:
				{
					FSSurface* surf = dynamic_cast<FSSurface*>(pl); assert(surf);
					GObject* po = surf->GetGObject();

					string name = string(po->GetName()) + "." + sname;
					XMLElement nset("Surface");
					nset.add_attribute("surface", name.c_str());
					m_xml.add_empty(nset);
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
void FEBioExport25::WriteGeometrySurfacePairs()
{
	// get the named surfaces (paired interfaces)
	FSModel& fem = *m_pfem;
	for (int i=0; i<fem.Steps(); ++i)
	{
		FSStep* pstep = fem.GetStep(i);
		for (int j=0; j<pstep->Interfaces(); ++j)
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
					XMLElement master("master");
					master.add_attribute("surface", GetSurfaceName(pms));
					m_xml.add_empty(master);

					XMLElement slave("slave");
					slave.add_attribute("surface", GetSurfaceName(pss));
					m_xml.add_empty(slave);
				}
				m_xml.close_branch();
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Write the Nodes sections.
// One Nodes section is written for each object.
void FEBioExport25::WriteGeometryNodes()
{
	FSModel& s = *m_pfem;
	GModel& model = s.GetModel();

	XMLWriter::SetFloatFormat(XMLWriter::ScientificFormat);

	int n = 1;
	for (int i=0; i<model.Objects(); ++i)
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
			for (int j=0; j<pm->Nodes(); ++j, ++n)
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
	for (int i=0; i<model.DiscreteObjects(); ++i)
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

						XMLElement el("node");
						el.add_attribute("id", n++);
						el.value(r);
						m_xml.add_leaf(el);
					}
				}
				m_xml.close_branch();
			}
		}
	}

	XMLWriter::SetFloatFormat(XMLWriter::FixedFormat);
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteGeometryElements()
{
	FSModel& s = *m_pfem;
	GModel& model = s.GetModel();

	// reset element counter
	m_ntotelem = 0;

	// loop over all objects
	for (int i=0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);

		// loop over all parts
		int NP = po->Parts();
		for (int p=0; p<NP; ++p)
		{
			// get the part
			GPart* pg = po->Part(p);

			// write this part
			WriteGeometryPart(pg);
		}
	}
}

//-----------------------------------------------------------------------------
const char* ElementTypeString(int ntype)
{
	const char* sztype = 0;
	switch (ntype)
	{
	case FE_TET4  :	sztype = "tet4"  ; break;
	case FE_TET5  :	sztype = "tet5"  ; break;
	case FE_TET10 :	sztype = "tet10" ; break;
	case FE_TET15 :	sztype = "tet15" ; break;
	case FE_TET20 :	sztype = "tet20" ; break;
	case FE_PENTA6:	sztype = "penta6"; break;
	case FE_HEX8  :	sztype = "hex8"  ; break;
	case FE_HEX20 :	sztype = "hex20" ; break;
	case FE_HEX27 : sztype = "hex27" ; break;
	case FE_QUAD4 :	sztype = "quad4" ; break;
	case FE_TRI3  :	sztype = "tri3"  ; break;
    case FE_TRI6  :	sztype = "tri6"  ; break;
    case FE_QUAD8 :	sztype = "quad8" ; break;
    case FE_QUAD9 :	sztype = "quad9" ; break;
	case FE_PYRA5 :	sztype = "pyra5"; break;
	case FE_PENTA15: sztype = "penta15"; break;
    case FE_PYRA13 : sztype = "pyra13"; break;
	case FE_BEAM2  : sztype = "line2"; break;
	case FE_BEAM3  : sztype = "line3"; break;
	default:
		assert(false);
	}
	return sztype;
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteGeometryPart(GPart* pg, bool useMatNames)
{
	FSModel& s = *m_pfem;
	GModel& model = s.GetModel();
	GObject* po = dynamic_cast<GObject*>(pg->Object());
	FSCoreMesh* pm = po->GetFEMesh();
	int pid = pg->GetLocalID();

	// Parts must be split up by element type
	int NE = pm->Elements();
	int NEP = 0; // number of elements in part
	for (int i=0; i<NE; ++i) 
	{
		FEElement_& el = pm->ElementRef(i);
		if (el.m_gid == pid) { el.m_ntag = 1; NEP++; } else el.m_ntag = -1; 
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
	char szname[128] = {0};
	for (int i=0;ncount<NEP;++i)
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
			if (nmat > 0) 
			{
				if (useMatNames) xe.add_attribute("mat", pmat->GetName().c_str());
				else xe.add_attribute("mat", nmat);
			}

			if (nset == 0)
				sprintf(szname, "%s", pg->GetName().c_str());
			else 
				sprintf(szname, "%s__%d", pg->GetName().c_str(), nset+1);

			ElementSet es;
			es.mesh = pm;
			es.name = szname;
			es.matID = pg->GetMaterialID();

			xe.add_attribute("name", szname);
			m_xml.add_branch(xe);
			{
				XMLElement xej("elem");
				int n1 = xej.add_attribute("id",(int)0);

				for (int j=i; j<NE; ++j)
				{
					FEElement_& ej = pm->ElementRef(j);
					if ((ej.m_ntag == 1) && (ej.Type() == ntype))
					{
						int eid = m_ntotelem + ncount + 1;
						xej.set_attribute(n1, eid);
						int ne = ej.Nodes();
						assert(ne == el.Nodes());
						for (int k=0; k<ne; ++k) nn[k] = pm->Node(ej.m_node[k]).m_nid;
						xej.value(nn, ne);
						m_xml.add_leaf(xej, false);
						ej.m_ntag = -1;	// mark as processed
						ej.m_nid = eid;
						ncount++;

						es.elem.push_back(j);
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
void FEBioExport25::WriteGeometryDiscreteSets()
{
	FSModel& fem = *m_pfem;
	GModel& model = fem.GetModel();

	// write the discrete element sets
	for (int i=0; i<model.DiscreteObjects(); ++i)
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
				for (int n=0; n<N; ++n)
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

				int n[2], m[2] = {0};
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
						for (int j=0; j<N-2; ++j)
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
	for (int i=0; i<fem.Steps(); ++i)
	{
		FSStep* step = fem.GetStep(i);
		for (int j=0; j<step->Interfaces(); ++j)
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
					size_t N = L.size();
					for (int n = 0; n<N; ++n)
					{
						pair<int,int>& de = L[n];
						int m[2] = {de.first, de.second};
						m_xml.add_leaf("delem", m, 2);
					}
				}
				m_xml.close_branch();
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteMeshDataSection()
{
	WriteElementDataSection();
	WriteSurfaceDataSection();
	WriteEdgeDataSection();
	WriteNodeDataSection();
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteElementDataSection()
{
	WriteMeshDataShellThickness();

	WriteMeshDataMaterialFibers();

	WriteMeshDataMaterialAxes();

	WriteMeshDataFields();
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteSurfaceDataSection()
{
	FSModel& fem = *m_pfem;
	GModel& model = fem.GetModel();

	for (int i=0; i<model.Objects(); i++)
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

				if (sd.GetDataType() == FEMeshData::DATA_TYPE::DATA_SCALAR) tag.add_attribute("datatype", "scalar");
				else if (sd.GetDataType() == FEMeshData::DATA_TYPE::DATA_VEC3D) tag.add_attribute("datatype", "vector");

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
void FEBioExport25::WriteEdgeDataSection()
{
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteNodeDataSection()
{
}


//-----------------------------------------------------------------------------
void FEBioExport25::WriteMeshDataShellThickness()
{
	FSModel& fem = *m_pfem;
	GModel& model = fem.GetModel();
	for (int i=0; i<(int) m_ElSet.size(); ++i)
	{
		ElementSet& elset = m_ElSet[i];
		FSCoreMesh* pm = elset.mesh;

		// see if this mesh has shells
		bool bshell = false;
		for (int k=0; k<(int) elset.elem.size(); ++k)
		{
			FEElement_& el = pm->ElementRef(elset.elem[k]);
			if (el.IsShell()) { bshell = true; break; }
		}

		// write shell thickness data
		if (bshell)
		{
			XMLElement tag("ElementData");
			tag.add_attribute("var", "shell thickness");
			tag.add_attribute("elem_set", elset.name.c_str());
			m_xml.add_branch(tag);
			{
				XMLElement el("elem");
				int n1 = el.add_attribute("lid", 0);

				int nid = 1;
				for (int k = 0; k<(int)elset.elem.size(); ++k)
				{
					FEElement_& e = pm->ElementRef(elset.elem[k]);
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
void FEBioExport25::WriteMeshDataMaterialFibers()
{
	FSModel& fem = *m_pfem;

	// loop over all element sets
	size_t NSET = m_ElSet.size();
	for (int i = 0; i<NSET; ++i)
	{
		ElementSet& elSet = m_ElSet[i];
		FSCoreMesh* pm = elSet.mesh;
		GObject* po = pm->GetGObject();
		const Transform& T = po->GetTransform();

		GMaterial* pmat = fem.GetMaterialFromID(elSet.matID);
		FSTransverselyIsotropic* ptiso = 0;
		if (pmat) ptiso = dynamic_cast<FSTransverselyIsotropic*>(pmat->GetMaterialProperties());

		if (ptiso && (ptiso->GetFiberMaterial()->m_naopt == FE_FIBER_USER))
		{
			int NE = (int) elSet.elem.size();
			XMLElement tag("ElementData");
			tag.add_attribute("var", "fiber");
			tag.add_attribute("elem_set", elSet.name.c_str());
			m_xml.add_branch(tag);
			{
				XMLElement el("elem");
				int nid = el.add_attribute("lid", 0);
				for (int j=0; j<NE; ++j)
				{
					FEElement_& e = pm->ElementRef(elSet.elem[j]);
					vec3d a = T.LocalToGlobalNormal(e.m_fiber);
					el.set_attribute(nid, j+1);
					el.value(a);
					m_xml.add_leaf(el, false);
				}
			}
			m_xml.close_branch(); // elem_data
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteMeshDataMaterialAxes()
{
	// loop over all element sets
	size_t NSET = m_ElSet.size();
	for (int i=0; i<NSET; ++i)
	{
		ElementSet& elSet = m_ElSet[i];
		FSCoreMesh* pm = elSet.mesh;
		GObject* po = pm->GetGObject();
		const Transform& T = po->GetTransform();

		// make sure there is something to do
		bool bwrite = false;
		int NE = (int) elSet.elem.size();
		for (int j=0; j<NE; ++j)
		{
			FEElement_& el = pm->ElementRef(elSet.elem[j]);
			if (el.m_Qactive) { bwrite = true; break; }
		}

		// okay, let's get to work
		if (bwrite)
		{
			int n = 0;
			XMLElement tag("ElementData");
			tag.add_attribute("var", "mat_axis");
			tag.add_attribute("elem_set", elSet.name.c_str());
			m_xml.add_branch(tag);
			{
				XMLElement el("elem");
				int nid = el.add_attribute("lid", 0);

				for (int j=0; j<NE; ++j)
				{
					FEElement_& e = pm->ElementRef(elSet.elem[j]);
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
void FEBioExport25::WriteMeshDataFields()
{
	FSModel& fem = *m_pfem;
	GModel& model = fem.GetModel();
	for (int i=0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		FSMesh* pm = po->GetFEMesh();
		int NE = pm->Elements();

		int ND = pm->MeshDataFields();
		for (int n=0; n<ND; ++n)
		{
			FEElementData* elemData = dynamic_cast<FEElementData*>(pm->GetMeshDataField(n));
			if (elemData)
			{
				FEElementData& data = *elemData;

				const FSElemSet* pg = data.GetElementSet();

				double scale = data.GetScaleFactor();

				XMLElement tag("ElementData");
				tag.add_attribute("var", data.GetName().c_str());
				tag.add_attribute("elem_set", pg->GetName());
				if (scale != 1.0) tag.add_attribute("scale", scale);
				m_xml.add_branch(tag);
				{
					XMLElement el("elem");
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
		}
	}
}

//-----------------------------------------------------------------------------

void FEBioExport25::WriteBoundarySection(FSStep& s)
{
	// --- B O U N D A R Y   C O N D I T I O N S ---
	// fixed constraints
	WriteBCFixed(s);

	// prescribed displacements
	WriteBCPrescribed(s);

	// rigid contact 
	WriteBCRigid(s);

	// rigid body constraints
	WriteRigidConstraints(s);
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteContactSection(FSStep& s)
{
	// --- C O N T A C T ---
	for (int i=0; i<s.Interfaces(); ++i)
	{
		FSPairedInterface* pi = dynamic_cast<FSPairedInterface*> (s.Interface(i));
		if (pi && pi->IsActive())
		{
			if (m_writeNotes) m_xml.add_comment(pi->GetInfo());

			int ntype = pi->Type();
			switch (ntype)
			{
			case FE_SLIDING_INTERFACE: 
				{
					// NOTE: This interface is obsolete, but continues to be supported for now for backward compatibility.
					//       Note that we still use the old interface types. 
					FSSlidingInterface* ps = dynamic_cast<FSSlidingInterface*>(pi); assert(ps);
					int ntype = ps->GetIntValue(FSSlidingInterface::NTYPE);
					if      (ntype == 0) WriteContactInterface(s, "sliding_with_gaps" , pi);
					else if (ntype == 1) WriteContactInterface(s, "facet-to-facet sliding", pi);
				}
				break;
			case FE_SLIDING_WITH_GAPS        : WriteContactInterface(s, "sliding-node-on-facet"  , pi); break;
			case FE_FACET_ON_FACET_SLIDING   : WriteContactInterface(s, "sliding-facet-on-facet" , pi); break;
			case FE_TIED_INTERFACE           : WriteContactInterface(s, "tied-node-on-facet"     , pi); break;
			case FE_FACET_ON_FACET_TIED      : WriteContactInterface(s, "tied-facet-on-facet"    , pi); break;
			case FE_TENSCOMP_INTERFACE       : WriteContactInterface(s, "sliding-elastic"        , pi); break;
			case FE_PORO_INTERFACE           : WriteContactInterface(s, "sliding-biphasic"       , pi); break;
			case FE_PORO_SOLUTE_INTERFACE    : WriteContactInterface(s, "sliding-biphasic-solute", pi); break;
			case FE_MULTIPHASIC_INTERFACE    : WriteContactInterface(s, "sliding-multiphasic"    , pi); break;
			case FE_TIEDBIPHASIC_INTERFACE   : WriteContactInterface(s, "tied-biphasic"          , pi); break;
			case FE_TIEDMULTIPHASIC_INTERFACE: WriteContactInterface(s, "tied-multiphasic"       , pi); break;
			case FE_STICKY_INTERFACE         : WriteContactInterface(s, "sticky"                 , pi); break;
			case FE_PERIODIC_BOUNDARY        : WriteContactInterface(s, "periodic boundary"      , pi); break;
			case FE_TIED_ELASTIC_INTERFACE   : WriteContactInterface(s, "tied-elastic"           , pi); break;
			case FE_GAPHEATFLUX_INTERFACE    : WriteContactInterface(s, "gap heat flux"          , pi); break;
			case FE_CONTACTPOTENTIAL_CONTACT : WriteContactInterface(s, "contact potential"      , pi); break;
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

void FEBioExport25::WriteLoadsSection(FSStep& s)
{
	XMLElement el;

	// nodal forces
	WriteLoadNodal(s);

	// pressure forces
	WriteLoadPressure(s);

	// surface tractions
	WriteLoadTraction(s);

	// fluid flux
	WriteFluidFlux(s);

	// mixture normal traction
	WriteBPNormalTraction(s);

	// solute flux
	WriteSoluteFlux(s);

    // matching osmotic coefficient
    WriteMatchingOsmoticCoefficient(s);

	// concentration flux
	WriteConcentrationFlux(s);

	// heat flux
	WriteHeatFlux(s);

	// convective heat flux
	WriteConvectiveHeatFlux(s);

	// body loads
	WriteBodyLoads(s);

    // fluid tractions
    WriteFluidTraction(s);
    
    // fluid pressure loads
    WriteFluidPressureLoad(s);
    
    // fluid velocities
    WriteFluidVelocity(s);
    
    // fluid normal velocities
    WriteFluidNormalVelocity(s);
    
    // fluid rotational velocities
    WriteFluidRotationalVelocity(s);
    
    // fluid flow resistance
    WriteFluidFlowResistance(s);
    
    // fluid flow RCR
    WriteFluidFlowRCR(s);
    
    // fluid backflow stabilization
    WriteFluidBackflowStabilization(s);
    
    // fluid tangential stabilization
    WriteFluidTangentialStabilization(s);
    
    // fluid-FSI traction
    WriteFSITraction(s);
    
    // biphasic-FSI traction
    WriteBFSITraction(s);
    
}

//-----------------------------------------------------------------------------
// write discrete elements
//
void FEBioExport25::WriteDiscreteSection(FSStep& s)
{
	FSModel& fem = *m_pfem;
	GModel& model = fem.GetModel();

	// Write the discrete materials
	int n = 1;
	for (int i=0; i<model.DiscreteObjects(); ++i)
	{
		GLinearSpring* ps = dynamic_cast<GLinearSpring*>(model.DiscreteObject(i));
		if (ps)
		{
			XMLElement dmat("discrete_material");
			int n1 = dmat.add_attribute("id", 0);
			int n2 = dmat.add_attribute("name", "");
			int n3 = dmat.add_attribute("type", "");

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
			XMLElement dmat("discrete_material");
			int n1 = dmat.add_attribute("id", 0);
			int n2 = dmat.add_attribute("name", "");
			int n3 = dmat.add_attribute("type", "");

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
			XMLElement dmat("discrete_material");
			int n1 = dmat.add_attribute("id", 0);
			int n2 = dmat.add_attribute("name", "");
			int n3 = dmat.add_attribute("type", "");

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
	for (int i=0; i<s.Interfaces(); ++i)
	{
		XMLElement dmat("discrete_material");
		int n1 = dmat.add_attribute("id", 0);
		int n2 = dmat.add_attribute("name", "");
		int n3 = dmat.add_attribute("type", "");

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
	int n1 = disc.add_attribute("dmat", 0);
	int n2 = disc.add_attribute("discrete_set", "");
	n = 1;
	for (int i=0; i<model.DiscreteObjects(); ++i)
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
void FEBioExport25::WriteRigidJoint(FSStep& s)
{
	for (int i=0; i<s.Interfaces(); ++i)
	{
		// rigid joints
		FSRigidJoint* pj = dynamic_cast<FSRigidJoint*> (s.Interface(i));
		if (pj && pj->IsActive())
		{
			XMLElement ec("constraint");
			ec.add_attribute("type","rigid joint");
			const char* sz = pj->GetName().c_str();
            ec.add_attribute("name", sz);
			m_xml.add_branch(ec);
			{
				int na = (pj->m_pbodyA? pj->m_pbodyA->m_ntag:0);
				int nb = (pj->m_pbodyB? pj->m_pbodyB->m_ntag:0);

				m_xml.add_leaf("tolerance", pj->GetFloatValue(FSRigidJoint::TOL));
				m_xml.add_leaf("penalty"  , pj->GetFloatValue(FSRigidJoint::PENALTY));
				m_xml.add_leaf("body_a"   , na);
				m_xml.add_leaf("body_b"   , nb);

				vec3d v = pj->GetVecValue(FSRigidJoint::RJ);
				m_xml.add_leaf("joint"    , v);
			}
			m_xml.close_branch();
		}
	}
}

//-----------------------------------------------------------------------------
// write rigid walls
//
void FEBioExport25::WriteContactWall(FSStep& s)
{
	for (int i=0; i<s.Interfaces(); ++i)
	{
		FSRigidWallInterface* pw = dynamic_cast<FSRigidWallInterface*> (s.Interface(i));
		if (pw && pw->IsActive())
		{
			if (m_writeNotes) m_xml.add_comment(pw->GetInfo());
			XMLElement ec("contact");
			ec.add_attribute("type", "rigid_wall");
			const char* sz = pw->GetName().c_str();
            ec.add_attribute("name", sz);
			ec.add_attribute("surface", GetSurfaceName(pw->GetItemList()));
			m_xml.add_branch(ec);
			{
				m_xml.add_leaf("laugon", (pw->GetBoolValue(FSRigidWallInterface::LAUGON)? 1 : 0));
				m_xml.add_leaf("tolerance", pw->GetFloatValue(FSRigidWallInterface::ALTOL));
				m_xml.add_leaf("penalty", pw->GetFloatValue(FSRigidWallInterface::PENALTY));

				int lc = GetLC(&pw->GetParam(FSRigidWallInterface::OFFSET));
				XMLElement offset("offset");
				if (lc > 0) offset.add_attribute("lc", lc);
				offset.value(pw->GetFloatValue(FSRigidWallInterface::OFFSET));
				m_xml.add_leaf(offset);

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
void FEBioExport25::WriteContactSphere(FSStep& s)
{
	for (int i = 0; i<s.Interfaces(); ++i)
	{
		FSRigidSphereInterface* pw = dynamic_cast<FSRigidSphereInterface*> (s.Interface(i));
		if (pw && pw->IsActive())
		{
			if (m_writeNotes) m_xml.add_comment(pw->GetInfo());
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
void FEBioExport25::WriteContactInterface(FSStep& s, const char* sztype, FSPairedInterface* pi)
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
void FEBioExport25::WriteBCRigid(FSStep& s)
{
	for (int i=0; i<s.Interfaces(); ++i)
	{
		// rigid interfaces
		FSRigidInterface* pr = dynamic_cast<FSRigidInterface*> (s.Interface(i));
		if (pr && pr->IsActive())
		{
			if (m_writeNotes) m_xml.add_comment(pr->GetInfo());
			GMaterial* pm = pr->GetRigidBody();
			if (pm==0) throw RigidContactException();
			int rb = pm->m_ntag;

			FEItemListBuilder* pitem = pr->GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(pr);

			XMLElement ec("rigid");
			const char* sz = pr->GetName().c_str();
            ec.add_attribute("name", sz);
			ec.add_attribute("rb", rb);
			ec.add_attribute("node_set", GetNodeSetName(pitem));
			m_xml.add_empty(ec);
		}
	}
}

//-----------------------------------------------------------------------------
// write linear constraints
void FEBioExport25::WriteLinearConstraints(FSStep& s)
{
	const char* szbc[]={"x","y","z"};

	for (int i=0; i<s.LinearConstraints(); ++i)
	{
		FSLinearConstraintSet* pset = s.LinearConstraint(i);
		XMLElement ec("contact");
		ec.add_attribute("type", "linear constraint");
		m_xml.add_branch(ec);
		{
			m_xml.add_leaf("tol"    , pset->m_atol   );
			m_xml.add_leaf("penalty", pset->m_penalty);
			m_xml.add_leaf("maxaug" , pset->m_nmaxaug);

			int NC = (int) pset->m_set.size();
			for (int j=0; j<NC; ++j)
			{
				FSLinearConstraintSet::LinearConstraint& LC = pset->m_set[j];
				m_xml.add_branch("linear_constraint");
				{
					int ND = (int) LC.m_dof.size();
					XMLElement ed("node");
					int n1 = ed.add_attribute("id", 0);
					int n2 = ed.add_attribute("bc", 0);
					for (int n=0; n<ND; ++n)
					{
						FSLinearConstraintSet::LinearConstraint::DOF& dof = LC.m_dof[n];
						ed.set_attribute(n1, dof.node);
						ed.set_attribute(n2, szbc[dof.bc] );
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
void FEBioExport25::WriteConstraints(FSStep& s)
{
	for (int i = 0; i<s.Constraints(); ++i)
	{
		FSModelConstraint* pw = s.Constraint(i);
		if (pw && pw->IsActive())
		{
			if (m_writeNotes) m_xml.add_comment(pw->GetInfo());
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
void FEBioExport25::WriteBCFixed(FSStep &s)
{
	FSModel& fem = m_prj.GetFSModel();

	for (int i = 0; i<s.BCs(); ++i)
	{
		FSFixedDOF* pbc = dynamic_cast<FSFixedDOF*>(s.BC(i));
		if (pbc && pbc->IsActive())
		{
			if (m_writeNotes) m_xml.add_comment(pbc->GetInfo());

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
				XMLElement tag("fix");
				tag.add_attribute("bc", sbc.c_str());

				// write node set
				const char* szname = GetNodeSetName(pitem);
				tag.add_attribute("node_set", szname);
				m_xml.add_empty(tag);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Export prescribed boundary conditions
void FEBioExport25::WriteBCPrescribed(FSStep &s)
{
	FSModel& fem = m_prj.GetFSModel();
	for (int i=0; i<s.BCs(); ++i)
	{
		FSPrescribedDOF* pbc = dynamic_cast<FSPrescribedDOF*>(s.BC(i));
		if (pbc && pbc->IsActive())
		{
			if (m_writeNotes) m_xml.add_comment(pbc->GetInfo());

			FEDOFVariable& var = fem.Variable(pbc->GetVarID());
			const char* szbc = var.GetDOF(pbc->GetDOF()).symbol();

			FEItemListBuilder* pitem = pbc->GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(pbc);

			XMLElement e("prescribe");
			e.add_attribute("bc", szbc);
			e.add_attribute("node_set", GetNodeSetName(pitem));

			m_xml.add_branch(e);
			{
				WriteParamList(*pbc);
			}
			m_xml.close_branch();
		}
	}
}

//-----------------------------------------------------------------------------
// export nodal loads
//
void FEBioExport25::WriteLoadNodal(FSStep& s)
{
	char bc[][3] = {"x", "y", "z", "p", "c1", "c2", "c3", "c4", "c5", "c6"};

	for (int j=0; j<s.Loads(); ++j)
	{
		FSNodalDOFLoad* pbc = dynamic_cast<FSNodalDOFLoad*>(s.Load(j));
		if (pbc && pbc->IsActive())
		{
			if (m_writeNotes) m_xml.add_comment(pbc->GetInfo());

			FEItemListBuilder* pitem = pbc->GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(pbc);

			int l = pbc->GetDOF();
			int lc = GetLC(&pbc->GetParam(FSNodalDOFLoad::LOAD));

			XMLElement load("nodal_load");
			load.add_attribute("bc", bc[l]);
			load.add_attribute("node_set", GetNodeSetName(pitem));

			m_xml.add_branch(load);
			{
				XMLElement scale("scale");
				if (lc > 0) scale.add_attribute("lc", lc);
				scale.value(pbc->GetLoad());
				m_xml.add_leaf(scale);
			}
			m_xml.close_branch(); // nodal_load
		}
	}
}

//----------------------------------------------------------------------------
// Export pressure loads
//
void FEBioExport25::WriteLoadPressure(FSStep& s)
{
	for (int j=0; j<s.Loads(); ++j)
	{
		FSPressureLoad* pbc = dynamic_cast<FSPressureLoad*>(s.Load(j));
		if (pbc && pbc->IsActive())
		{
			if (m_writeNotes) m_xml.add_comment(pbc->GetInfo());

			// create the surface list
			FEItemListBuilder* pitem = pbc->GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(pbc);

			XMLElement load;
			load.name("surface_load");
			load.add_attribute("type", "pressure");
			load.add_attribute("surface", GetSurfaceName(pitem));
			m_xml.add_branch(load);
			{
				WriteParamList(*pbc);
			}
			m_xml.close_branch(); // surface_load
		}
	}
}

//----------------------------------------------------------------------------
// Export fluid flux
//
void FEBioExport25::WriteFluidFlux(FSStep& s)
{
	for (int j=0; j<s.Loads(); ++j)
	{
		FSFluidFlux* pbc = dynamic_cast<FSFluidFlux*>(s.Load(j));
		if (pbc && pbc->IsActive())
		{
			if (m_writeNotes) m_xml.add_comment(pbc->GetInfo());

			// create the surface list
			FEItemListBuilder* pitem = pbc->GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(pbc);

			XMLElement flux;
			flux.name("surface_load");
			flux.add_attribute("type", "fluidflux");
			flux.add_attribute("surface", GetSurfaceName(pitem));
			m_xml.add_branch(flux);
			{
				WriteParamList(*pbc);	
			}
			m_xml.close_branch(); // surface_load
		}
	}
}


//----------------------------------------------------------------------------
// Export mixture normal traction
//
void FEBioExport25::WriteBPNormalTraction(FSStep& s)
{
	for (int j=0; j<s.Loads(); ++j)
	{
		FSBPNormalTraction* pbc = dynamic_cast<FSBPNormalTraction*>(s.Load(j));
		if (pbc && pbc->IsActive())
		{
			if (m_writeNotes) m_xml.add_comment(pbc->GetInfo());

			FEItemListBuilder* pitem = pbc->GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(pbc);

			XMLElement flux;
			flux.name("surface_load");
			flux.add_attribute("type", "normal_traction");
			flux.add_attribute("surface", GetSurfaceName(pitem));
			m_xml.add_branch(flux);
			{
				WriteParamList(*pbc);
			}
			m_xml.close_branch(); // normal_traction
		}
	}
}

//----------------------------------------------------------------------------
// Export heat flux
//
void FEBioExport25::WriteHeatFlux(FSStep& s)
{
	for (int j=0; j<s.Loads(); ++j)
	{
		FSHeatFlux* pbc = dynamic_cast<FSHeatFlux*>(s.Load(j));
		if (pbc && pbc->IsActive())
		{
			if (m_writeNotes) m_xml.add_comment(pbc->GetInfo());

			FEItemListBuilder* pitem = pbc->GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(pbc);

			XMLElement flux("surface_load");
			flux.add_attribute("type", "heatflux");
			flux.add_attribute("surface", GetSurfaceName(pitem));
			m_xml.add_branch(flux);
			{
				WriteParamList(*pbc);
			}
			m_xml.close_branch(); // surface_load
		}
	}
}

//----------------------------------------------------------------------------
// Export matching osmotic coefficieny
//
void FEBioExport25::WriteMatchingOsmoticCoefficient(FSStep& s)
{
    for (int j=0; j<s.Loads(); ++j)
    {
        FSMatchingOsmoticCoefficient* pbc = dynamic_cast<FSMatchingOsmoticCoefficient*>(s.Load(j));
        if (pbc && pbc->IsActive())
        {
            if (m_writeNotes) m_xml.add_comment(pbc->GetInfo());

            FEItemListBuilder* pitem = pbc->GetItemList();
            if (pitem == 0) throw InvalidItemListBuilder(pbc);

            XMLElement moc;
            moc.name("surface_load");
            moc.add_attribute("type", "matching_osm_coef");
            moc.add_attribute("surface", GetSurfaceName(pitem));
            m_xml.add_branch(moc);
            {
                WriteParamList(*pbc);
            }
            m_xml.close_branch(); // normal_traction
        }
    }
}

//----------------------------------------------------------------------------
// Export convective heat flux
//
void FEBioExport25::WriteConvectiveHeatFlux(FSStep& s)
{
	for (int j=0; j<s.Loads(); ++j)
	{
		FSConvectiveHeatFlux* pbc = dynamic_cast<FSConvectiveHeatFlux*>(s.Load(j));
		if (pbc && pbc->IsActive())
		{
			if (m_writeNotes) m_xml.add_comment(pbc->GetInfo());

			FEItemListBuilder* pitem = pbc->GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(pbc);

			XMLElement flux("surface_load");
			flux.add_attribute("type", "convective_heatflux");
			flux.add_attribute("surface", GetSurfaceName(pitem));
			m_xml.add_branch(flux);
			{
				WriteParamList(*pbc);
			}
			m_xml.close_branch(); // surface_load
		}
	}
}

//----------------------------------------------------------------------------
// Export solute flux
//
void FEBioExport25::WriteSoluteFlux(FSStep& s)
{
	for (int j=0; j<s.Loads(); ++j)
	{
		FSSoluteFlux* pbc = dynamic_cast<FSSoluteFlux*>(s.Load(j));
		if (pbc && pbc->IsActive())
		{
			if (m_writeNotes) m_xml.add_comment(pbc->GetInfo());

			// get the item list builder
			FEItemListBuilder* pitem = pbc->GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(pbc);

			XMLElement flux;
			flux.name("surface_load");
			flux.add_attribute("type", "soluteflux");
			flux.add_attribute("surface", GetSurfaceName(pitem));
			m_xml.add_branch(flux);
			{
				WriteParamList(*pbc);
			}
			m_xml.close_branch(); // soluteflux
		}
	}
}

//----------------------------------------------------------------------------
void FEBioExport25::WriteConcentrationFlux(FSStep& s)
{
	for (int j = 0; j<s.Loads(); ++j)
	{
		FSConcentrationFlux* pcf = dynamic_cast<FSConcentrationFlux*>(s.Load(j));
		if (pcf && pcf->IsActive())
		{
			if (m_writeNotes) m_xml.add_comment(pcf->GetInfo());

			// get the item list builder
			FEItemListBuilder* pitem = pcf->GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(pcf);

			XMLElement flux;
			flux.name("surface_load");
			flux.add_attribute("type", "concentration flux");
			flux.add_attribute("surface", GetSurfaceName(pitem));
			m_xml.add_branch(flux);
			{
				WriteParamList(*pcf);
			}
			m_xml.close_branch();
		}
	}
}

//----------------------------------------------------------------------------
// Export pressure tractions
//
void FEBioExport25::WriteLoadTraction(FSStep& s)
{
	for (int j=0; j<s.Loads(); ++j)
	{
		FSSurfaceTraction* ptc = dynamic_cast<FSSurfaceTraction*>(s.Load(j));
		if (ptc && ptc->IsActive())
		{
			if (m_writeNotes) m_xml.add_comment(ptc->GetInfo());

			FEItemListBuilder* pitem = ptc->GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(ptc);

			XMLElement flux("surface_load");
			flux.add_attribute("type", "traction");
			flux.add_attribute("surface", GetSurfaceName(pitem));
			m_xml.add_branch(flux);
			{
				WriteParamList(*ptc);
			}
			m_xml.close_branch(); // surface_load
		}
	}
}

//----------------------------------------------------------------------------
// Export fluid tractions
//
void FEBioExport25::WriteFluidTraction(FSStep& s)
{
    for (int j=0; j<s.Loads(); ++j)
    {
        FSFluidTraction* ptc = dynamic_cast<FSFluidTraction*>(s.Load(j));
        if (ptc && ptc->IsActive())
        {
			if (m_writeNotes) m_xml.add_comment(ptc->GetInfo());

            FEItemListBuilder* pitem = ptc->GetItemList();
            if (pitem == 0) throw InvalidItemListBuilder(ptc);
            
            XMLElement flux("surface_load");
            flux.add_attribute("type", "fluid viscous traction");
			flux.add_attribute("surface", GetSurfaceName(pitem));
			m_xml.add_branch(flux);
            {
				WriteParamList(*ptc);
			}
            m_xml.close_branch(); // surface_load
        }
    }
}

//----------------------------------------------------------------------------
// Export fluid pressure loads
//
void FEBioExport25::WriteFluidPressureLoad(FSStep& s)
{
    for (int j=0; j<s.Loads(); ++j)
    {
        FSFluidPressureLoad* pbc = dynamic_cast<FSFluidPressureLoad*>(s.Load(j));
        if (pbc && pbc->IsActive())
        {
            if (m_writeNotes) m_xml.add_comment(pbc->GetInfo());
            
            // create the surface list
            FEItemListBuilder* pitem = pbc->GetItemList();
            if (pitem == 0) throw InvalidItemListBuilder(pbc);
            
            XMLElement load;
            load.name("surface_load");
            load.add_attribute("type", "fluid pressure");
            load.add_attribute("surface", GetSurfaceName(pitem));
            m_xml.add_branch(load);
            {
                WriteParamList(*pbc);
            }
            m_xml.close_branch(); // surface_load
        }
    }
}

//----------------------------------------------------------------------------
// Export fluid velocities
//
void FEBioExport25::WriteFluidVelocity(FSStep& s)
{
    for (int j=0; j<s.Loads(); ++j)
    {
        FSFluidVelocity* ptc = dynamic_cast<FSFluidVelocity*>(s.Load(j));
        if (ptc && ptc->IsActive())
        {
			if (m_writeNotes) m_xml.add_comment(ptc->GetInfo());

            FEItemListBuilder* pitem = ptc->GetItemList();
            if (pitem == 0) throw InvalidItemListBuilder(ptc);
            
            XMLElement flux("surface_load");
            flux.add_attribute("type", "fluid velocity");
            flux.add_attribute("surface", GetSurfaceName(pitem));
            m_xml.add_branch(flux);
            {
                XMLElement scl("scale");
				int lc = GetLC(&ptc->GetParam(FSFluidVelocity::LOAD));
				if (lc > 0) scl.add_attribute("lc", lc);
                scl.value(1.0);
                m_xml.add_leaf(scl);
                
                m_xml.add_leaf("velocity", ptc->GetLoad());
            }
            m_xml.close_branch(); // surface_load
        }
    }
}

//----------------------------------------------------------------------------
// Export fluid normal velocities
//
void FEBioExport25::WriteFluidNormalVelocity(FSStep& s)
{
    for (int j=0; j<s.Loads(); ++j)
    {
        FSFluidNormalVelocity* ptc = dynamic_cast<FSFluidNormalVelocity*>(s.Load(j));
        if (ptc && ptc->IsActive())
        {
			if (m_writeNotes) m_xml.add_comment(ptc->GetInfo());

            FEItemListBuilder* pitem = ptc->GetItemList();
            if (pitem == 0) throw InvalidItemListBuilder(ptc);
            
            XMLElement flux("surface_load");
            flux.add_attribute("type", "fluid normal velocity");
            flux.add_attribute("surface", GetSurfaceName(pitem));
            m_xml.add_branch(flux);
            {
                XMLElement load("velocity");

				int lc = GetLC(&ptc->GetParam(FSFluidNormalVelocity::LOAD));
				if (lc > 0) load.add_attribute("lc", lc);

                load.value(ptc->GetLoad());
                m_xml.add_leaf(load);
                
                XMLElement bp("prescribe_nodal_velocities");
                bp.value(ptc->GetBP());
                m_xml.add_leaf(bp);
                
                XMLElement bparab("parabolic");
                bparab.value(ptc->GetBParab());
                m_xml.add_leaf(bparab);
                
                XMLElement brimp("prescribe_rim_pressure");
                brimp.value(ptc->GetBRimP());
                m_xml.add_leaf(brimp);
                
            }
            m_xml.close_branch(); // surface_load
        }
    }
}

//----------------------------------------------------------------------------
// Export fluid rotational velocities
//
void FEBioExport25::WriteFluidRotationalVelocity(FSStep& s)
{
    for (int j=0; j<s.Loads(); ++j)
    {
        FSFluidRotationalVelocity* ptc = dynamic_cast<FSFluidRotationalVelocity*>(s.Load(j));
        if (ptc && ptc->IsActive())
        {
			if (m_writeNotes) m_xml.add_comment(ptc->GetInfo());

            FEItemListBuilder* pitem = ptc->GetItemList();
            if (pitem == 0) throw InvalidItemListBuilder(ptc);
            
            XMLElement flux("surface_load");
            flux.add_attribute("type", "fluid rotational velocity");
            flux.add_attribute("surface", GetSurfaceName(pitem));
            m_xml.add_branch(flux);
            {
                XMLElement load("angular_speed");

				int lc = GetLC(&ptc->GetParam(FSFluidRotationalVelocity::LOAD));
				if (lc > 0) load.add_attribute("lc", lc);

                load.value(ptc->GetLoad());
                m_xml.add_leaf(load);
                
                XMLElement axis("axis");
                axis.value(ptc->GetAxis());
                m_xml.add_leaf(axis);
                
                XMLElement origin("origin");
                origin.value(ptc->GetOrigin());
                m_xml.add_leaf(origin);
                
            }
            m_xml.close_branch(); // surface_load
        }
    }
}

//----------------------------------------------------------------------------
// Export fluid flow resistance
//
void FEBioExport25::WriteFluidFlowResistance(FSStep& s)
{
    for (int j=0; j<s.Loads(); ++j)
    {
        FSFluidFlowResistance* ptc = dynamic_cast<FSFluidFlowResistance*>(s.Load(j));
        if (ptc && ptc->IsActive())
        {
			if (m_writeNotes) m_xml.add_comment(ptc->GetInfo());

            FEItemListBuilder* pitem = ptc->GetItemList();
            if (pitem == 0) throw InvalidItemListBuilder(ptc);
            
            XMLElement flux("surface_load");
            flux.add_attribute("type", "fluid resistance");
            flux.add_attribute("surface", GetSurfaceName(pitem));
            m_xml.add_branch(flux);
            {
                XMLElement load("R");

				int lc = GetLC(&ptc->GetParam(FSFluidFlowResistance::LOAD));
				if (lc > 0) load.add_attribute("lc", lc);

                load.value(ptc->GetLoad());
                m_xml.add_leaf(load);
                
                XMLElement po("pressure_offset");
				
				int lcpo = GetLC(&ptc->GetParam(FSFluidFlowResistance::PO));
				if (lcpo > 0) po.add_attribute("lc", lcpo);
                
				po.value(ptc->GetPO());
                m_xml.add_leaf(po);
            }
            m_xml.close_branch(); // surface_load
        }
    }
}

//----------------------------------------------------------------------------
// Export fluid flow RCR
//
void FEBioExport25::WriteFluidFlowRCR(FSStep& s)
{
    for (int j=0; j<s.Loads(); ++j)
    {
        FSFluidFlowRCR* ptc = dynamic_cast<FSFluidFlowRCR*>(s.Load(j));
        if (ptc && ptc->IsActive())
        {
            if (m_writeNotes) m_xml.add_comment(ptc->GetInfo());

            FEItemListBuilder* pitem = ptc->GetItemList();
            if (pitem == 0) throw InvalidItemListBuilder(ptc);
            
            XMLElement flux("surface_load");
            flux.add_attribute("type", "fluid RCR");
            flux.add_attribute("surface", GetSurfaceName(pitem));
            m_xml.add_branch(flux);
            {
                XMLElement load("R");
				int lc = GetLC(&ptc->GetParam(FSFluidFlowRCR::LOAD));
				if (lc > 0) load.add_attribute("lc", lc);

                load.value(ptc->GetLoad());
                m_xml.add_leaf(load);
                
                XMLElement rd("Rd");

				int rdlc = GetLC(&ptc->GetParam(FSFluidFlowRCR::RD));
                if (rdlc > 0) rd.add_attribute("lc", rdlc);

                rd.value(ptc->GetRD());
                m_xml.add_leaf(rd);
                
                XMLElement co("capacitance");

				int colc = GetLC(&ptc->GetParam(FSFluidFlowRCR::CO)); 
				if (colc > 0) co.add_attribute("lc", colc);

                co.value(ptc->GetCO());
                m_xml.add_leaf(co);
                
                XMLElement po("pressure_offset");

				int polc = GetLC(&ptc->GetParam(FSFluidFlowRCR::PO)); 
				if (polc > 0) po.add_attribute("lc", polc);

                po.value(ptc->GetPO());
                m_xml.add_leaf(po);

                XMLElement ip("initial_pressure");

				int iplc = GetLC(&ptc->GetParam(FSFluidFlowRCR::IP));
				if (iplc > 0) po.add_attribute("lc", iplc);

                ip.value(ptc->GetIP());
                m_xml.add_leaf(ip);
            }
            m_xml.close_branch(); // surface_load
        }
    }
}

//----------------------------------------------------------------------------
// Export fluid backflow stabilization
//
void FEBioExport25::WriteFluidBackflowStabilization(FSStep& s)
{
    for (int j=0; j<s.Loads(); ++j)
    {
        FSFluidBackflowStabilization* ptc = dynamic_cast<FSFluidBackflowStabilization*>(s.Load(j));
        if (ptc && ptc->IsActive())
        {
			if (m_writeNotes) m_xml.add_comment(ptc->GetInfo());

            FEItemListBuilder* pitem = ptc->GetItemList();
            if (pitem == 0) throw InvalidItemListBuilder(ptc);
            
            XMLElement flux("surface_load");
            flux.add_attribute("type", "fluid backflow stabilization");
            flux.add_attribute("surface", GetSurfaceName(pitem));
            m_xml.add_branch(flux);
            {
				XMLElement load("beta");

				int lc = GetLC(&ptc->GetParam(FSFluidBackflowStabilization::LOAD));
                if (lc > 0) load.add_attribute("lc", lc);

                load.value(ptc->GetLoad());
                m_xml.add_leaf(load);
            }
            m_xml.close_branch(); // surface_load
        }
    }
}

//----------------------------------------------------------------------------
// Export fluid tangential stabilization
//
void FEBioExport25::WriteFluidTangentialStabilization(FSStep& s)
{
    for (int j=0; j<s.Loads(); ++j)
    {
        FSFluidTangentialStabilization* ptc = dynamic_cast<FSFluidTangentialStabilization*>(s.Load(j));
        if (ptc && ptc->IsActive())
        {
			if (m_writeNotes) m_xml.add_comment(ptc->GetInfo());

            FEItemListBuilder* pitem = ptc->GetItemList();
            if (pitem == 0) throw InvalidItemListBuilder(ptc);
            
            XMLElement flux("surface_load");
            flux.add_attribute("type", "fluid tangential stabilization");
            flux.add_attribute("surface", GetSurfaceName(pitem));
            m_xml.add_branch(flux);
            {
                XMLElement load("beta");

				int lc = GetLC(&ptc->GetParam(FSFluidTangentialStabilization::LOAD));
                if (lc > 0) load.add_attribute("lc", lc);

                load.value(ptc->GetLoad());
                m_xml.add_leaf(load);
            }
            m_xml.close_branch(); // surface_load
        }
    }
}

//----------------------------------------------------------------------------
// Export fluid-FSI traction
//
void FEBioExport25::WriteFSITraction(FSStep& s)
{
    for (int j=0; j<s.Loads(); ++j)
    {
        FSFSITraction* ptc = dynamic_cast<FSFSITraction*>(s.Load(j));
        if (ptc && ptc->IsActive())
        {
			if (m_writeNotes) m_xml.add_comment(ptc->GetInfo());

            FEItemListBuilder* pitem = ptc->GetItemList();
            if (pitem == 0) throw InvalidItemListBuilder(ptc);
            
            XMLElement flux("surface_load");
            flux.add_attribute("type", "fluid-FSI traction");
            flux.add_attribute("surface", GetSurfaceName(pitem));
            m_xml.add_branch(flux);
            m_xml.close_branch(); // surface_load
        }
    }
}

//----------------------------------------------------------------------------
// Export biphasic-FSI traction
//
void FEBioExport25::WriteBFSITraction(FSStep& s)
{
    for (int j=0; j<s.Loads(); ++j)
    {
        FSBFSITraction* ptc = dynamic_cast<FSBFSITraction*>(s.Load(j));
        if (ptc && ptc->IsActive())
        {
            if (m_writeNotes) m_xml.add_comment(ptc->GetInfo());
            
            FEItemListBuilder* pitem = ptc->GetItemList();
            if (pitem == 0) throw InvalidItemListBuilder(ptc);
            
            XMLElement flux("surface_load");
            flux.add_attribute("type", "biphasic-FSI traction");
            flux.add_attribute("surface", GetSurfaceName(pitem));
            m_xml.add_branch(flux);
            m_xml.close_branch(); // surface_load
        }
    }
}

//-----------------------------------------------------------------------------
// Export initial conditions
//
void FEBioExport25::WriteInitialSection(FSStep& s)
{
	FSModel& fem = m_prj.GetFSModel();

	// initial velocities
	for (int j=0; j<s.ICs(); ++j)
	{
		FSInitialCondition* pi = s.IC(j);
		if (pi && pi->IsActive())
		{
			if (m_writeNotes) m_xml.add_comment(pi->GetInfo());

			FEItemListBuilder* pitem = nullptr;
			
			if (dynamic_cast<FSInitialNodalDOF*>(pi))
			{
				pitem = pi->GetItemList();
				if (pitem == 0) throw InvalidItemListBuilder(pi);
			}

			switch (pi->Type())
			{
			case FE_INIT_NODAL_VELOCITIES      : WriteInitVelocity           (dynamic_cast<FSNodalVelocities  &>(*pi)); break;
			case FE_INIT_NODAL_SHELL_VELOCITIES: WriteInitShellVelocity      (dynamic_cast<FSNodalShellVelocities&>(*pi)); break;
            case FE_INIT_FLUID_PRESSURE        : WriteInitFluidPressure      (dynamic_cast<FSInitFluidPressure&>(*pi)); break;
            case FE_INIT_SHELL_FLUID_PRESSURE  : WriteInitShellFluidPressure (dynamic_cast<FSInitShellFluidPressure&>(*pi)); break;
			case FE_INIT_CONCENTRATION         : WriteInitConcentration      (dynamic_cast<FSInitConcentration&>(*pi)); break;
            case FE_INIT_SHELL_CONCENTRATION   : WriteInitShellConcentration (dynamic_cast<FSInitShellConcentration&>(*pi)); break;
			case FE_INIT_TEMPERATURE           : WriteInitTemperature        (dynamic_cast<FSInitTemperature  &>(*pi)); break;
            case FE_INIT_FLUID_DILATATION      : WriteInitFluidDilatation    (dynamic_cast<FSInitFluidDilatation&>(*pi)); break;
			case FE_INIT_PRESTRAIN             : WriteInitPrestrain          (dynamic_cast<FSInitPrestrain&>(*pi)); break;
			}
		}
	}

	// write rigid initial conditions
	for (int i=0; i<s.RigidConstraints(); ++i)
	{
		FSRigidConstraint* rc = s.RigidConstraint(i);
		if (rc->IsActive())
		{
			GMaterial* pgm = fem.GetMaterialFromID(rc->GetMaterialID());
			if (pgm == 0) throw MissingRigidBody(rc->GetName().c_str());
			if (pgm->GetMaterialProperties()->IsRigid() == false) throw InvalidMaterialReference();

			if (rc->Type() == FE_RIGID_INIT_VELOCITY)
			{
				FSRigidVelocity* rv = dynamic_cast<FSRigidVelocity*>(rc);
				XMLElement el("rigid_body");
				el.add_attribute("mat", pgm->m_ntag);
				m_xml.add_branch(el);
				{
					m_xml.add_leaf("initial_velocity", rv->GetVelocity());
				}
				m_xml.close_branch();
			}
			else if (rc->Type() == FE_RIGID_INIT_ANG_VELOCITY)
			{
				FSRigidAngularVelocity* rv = dynamic_cast<FSRigidAngularVelocity*>(rc);
				XMLElement el("rigid_body");
				el.add_attribute("mat", pgm->m_ntag);
				m_xml.add_branch(el);
				{
					m_xml.add_leaf("initial_angular_velocity", rv->GetVelocity());
				}
				m_xml.close_branch();
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteInitVelocity(FSNodalVelocities& iv)
{
	XMLElement el("init");
	int nbc = el.add_attribute("bc", "");
	el.add_attribute("node_set", GetNodeSetName(iv.GetItemList()));
	vec3d v = iv.GetVelocity();
	if (v.x != 0.0)
	{
		el.set_attribute(nbc, "vx");
		m_xml.add_branch(el, false);
		{
			m_xml.add_leaf("value", v.x);
		}
		m_xml.close_branch();
	}
	if (v.y != 0.0)
	{
		el.set_attribute(nbc, "vy");
		m_xml.add_branch(el, false);
		{
			m_xml.add_leaf("value", v.y);
		}
		m_xml.close_branch();
	}
	if (v.z != 0.0)
	{
		el.set_attribute(nbc, "vz");
		m_xml.add_branch(el, false);
		{
			m_xml.add_leaf("value", v.z);
		}
		m_xml.close_branch();
	}
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteInitShellVelocity(FSNodalShellVelocities& iv)
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
void FEBioExport25::WriteInitConcentration(FSInitConcentration& ic)
{
	char szbc[6][3] = {"c1", "c2", "c3", "c4", "c5", "c6"};
	int bc = ic.GetBC();
	XMLElement ec("init");
	ec.add_attribute("bc", szbc[bc]);
	ec.add_attribute("node_set", GetNodeSetName(ic.GetItemList()));
	m_xml.add_branch(ec);
	{
		m_xml.add_leaf("value", ic.GetValue());
	}
	m_xml.close_branch();
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteInitShellConcentration(FSInitShellConcentration& ic)
{
    char szbc[6][3] = {"d1", "d2", "d3", "d4", "d5", "d6"};
    int bc = ic.GetBC();
    XMLElement ec("init");
    ec.add_attribute("bc", szbc[bc]);
    ec.add_attribute("node_set", GetNodeSetName(ic.GetItemList()));
    m_xml.add_branch(ec);
    {
        m_xml.add_leaf("value", ic.GetValue());
    }
    m_xml.close_branch();
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteInitFluidPressure(FSInitFluidPressure& ip)
{
	XMLElement el("init");
	el.add_attribute("bc", "p");
	el.add_attribute("node_set", GetNodeSetName(ip.GetItemList()));
	m_xml.add_branch(el);
	{
		m_xml.add_leaf("value", ip.GetValue());
	}
	m_xml.close_branch();
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteInitShellFluidPressure(FSInitShellFluidPressure& ip)
{
    XMLElement el("init");
    el.add_attribute("bc", "q");
    el.add_attribute("node_set", GetNodeSetName(ip.GetItemList()));
    m_xml.add_branch(el);
    {
        m_xml.add_leaf("value", ip.GetValue());
    }
    m_xml.close_branch();
}
//-----------------------------------------------------------------------------
void FEBioExport25::WriteInitTemperature(FSInitTemperature&   it)
{
	XMLElement el("init");
	el.add_attribute("bc", "T");
	el.add_attribute("node_set", GetNodeSetName(it.GetItemList()));
	m_xml.add_branch(el);
	{
		m_xml.add_leaf("value", it.GetValue());
	}
	m_xml.close_branch();
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteInitFluidDilatation(FSInitFluidDilatation&   it)
{
    XMLElement el("init");
    el.add_attribute("bc", "ef");
    el.add_attribute("node_set", GetNodeSetName(it.GetItemList()));
    m_xml.add_branch(el);
    {
        m_xml.add_leaf("value", it.GetValue());
    }
    m_xml.close_branch();
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteInitPrestrain(FSInitPrestrain& ip)
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
void FEBioExport25::WriteBodyLoads(FSStep& s)
{
	for (int i = 0; i<s.Loads(); ++i)
	{
		FSBodyLoad* pbl = dynamic_cast<FSBodyLoad*>(s.Load(i));
		if (pbl && pbl->IsActive())
		{
			if (m_writeNotes) m_xml.add_comment(pbl->GetInfo());

			GPartList* pg = dynamic_cast<GPartList*>(pbl->GetItemList());

			vector<GPart*> partList;
			if (pg) partList = pg->GetPartList();

			if (partList.empty()) WriteBodyLoad(pbl, 0);
			else
			{
				for (int j=0; j<partList.size(); ++j) WriteBodyLoad(pbl, partList[j]);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteBodyLoad(FSBodyLoad* pbl, GPart* pg)
{
	FSConstBodyForce* pbf = dynamic_cast<FSConstBodyForce*>(pbl);
	if (pbf) WriteBodyForce(pbf, pg);

	FSHeatSource* phs = dynamic_cast<FSHeatSource*>(pbl);
	if (phs) WriteHeatSource(phs, pg);

	FSSBMPointSource* pps = dynamic_cast<FSSBMPointSource*>(pbl);
	if (pps)
	{
		XMLElement el("body_load");
		el.add_attribute("type", "sbm point source");
		if (pg) el.add_attribute("elem_set", pg->GetName());
		m_xml.add_branch(el);
		{
			WriteParamList(*pps);
		}
		m_xml.close_branch();	
	}

    FSCentrifugalBodyForce* pcs = dynamic_cast<FSCentrifugalBodyForce*>(pbl);
    if (pcs) WriteCentrifugalBodyForce(pcs, pg);
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteBodyForce(FSConstBodyForce* pbf, GPart* pg)
{
	XMLElement el("body_load");
	el.add_attribute("type", "const");
	if (pg) el.add_attribute("elem_set", pg->GetName());
	m_xml.add_branch(el);
	{
		char sz[3][2] = {"x", "y", "z"};
		XMLElement el;
		for (int i=0; i<3; ++i) 
		{
			el.name(sz[i]);
			int lc = GetLC(&pbf->GetParam(i));
			if (lc > 0) el.add_attribute("lc", lc);
			el.value(pbf->GetLoad(i));
			m_xml.add_leaf(el);
		}
	}
	m_xml.close_branch();
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteHeatSource(FSHeatSource* phs, GPart* pg)
{
	XMLElement el("body_load");
	el.add_attribute("type", "heat_source");
	if (pg) el.add_attribute("elem_set", pg->GetName());
	m_xml.add_branch(el);
	{
		WriteParamList(*phs);
	}
	m_xml.close_branch();
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteCentrifugalBodyForce(FSCentrifugalBodyForce* pcs, GPart* pg)
{
    XMLElement el("body_load");
    el.add_attribute("type", "centrifugal");
    if (pg) el.add_attribute("elem_set", pg->GetName());
    m_xml.add_branch(el);
    {
        WriteParamList(*pcs);
    }
    m_xml.close_branch();
}

//-----------------------------------------------------------------------------

void FEBioExport25::WriteGlobalsSection()
{
	XMLElement el;
	FSModel& fem = *m_pfem;

	if (fem.Parameters())
	{
		m_xml.add_branch("Constants");
		{
			int N = fem.Parameters();
			for (int i=0; i<N; ++i)
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
				for (int i=0; i<NS; ++i)
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
				for (int i=0; i<NS; ++i)
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

void FEBioExport25::WriteLoadDataSection()
{
	FSModel& fem = m_prj.GetFSModel();

	for (int i=0; i< fem.LoadControllers(); ++i)
	{
		XMLElement el;
		el.name("loadcurve");
		el.add_attribute("id", i+1);
/*
		switch (plc->GetType())
		{
		case LoadCurve::LC_STEP  : el.add_attribute("type", "step"  ); break;
		case LoadCurve::LC_LINEAR: el.add_attribute("type", "linear"); break;
		case LoadCurve::LC_SMOOTH: el.add_attribute("type", "smooth"); break;
        case LoadCurve::LC_CSPLINE: el.add_attribute("interpolate", "cubic spline"); break;
        case LoadCurve::LC_CPOINTS: el.add_attribute("interpolate", "control points"); break;
        case LoadCurve::LC_APPROX: el.add_attribute("interpolate", "approximation"); break;
		}

		switch (plc->GetExtend())
		{
//		case LoadCurve::EXT_CONSTANT     : el.add_attribute("extend", "constant"     ); break;
		case LoadCurve::EXT_EXTRAPOLATE  : el.add_attribute("extend", "extrapolate"  ); break;
		case LoadCurve::EXT_REPEAT       : el.add_attribute("extend", "repeat"       ); break;
		case LoadCurve::EXT_REPEAT_OFFSET: el.add_attribute("extend", "repeat offset"); break;
		}
*/
		double d[2];
		m_xml.add_branch(el);
		{
/*			for (int j = 0; j<plc->Size(); ++j)
			{
				LOADPOINT& pt = plc->Item(j);
				d[0] = pt.time;
				d[1] = pt.load;
				m_xml.add_leaf("point", d, 2);
			}
*/
		}
		m_xml.close_branch(); // loadcurve
	}
}

//-----------------------------------------------------------------------------

void FEBioExport25::WriteSurfaceSection(FEFaceList& s)
{
	XMLElement ef;
	int n = 1, nn[9];

	int NF = s.Size();
	FEFaceList::Iterator pf = s.First();

/*	
	FSAnalysisStep* pstep = dynamic_cast<FSAnalysisStep*>(m_pfem->GetStep(1));
	assert(pstep);
	STEP_SETTINGS& ops = pstep->GetSettings();
	if (ops.beface)
	{
		for (int i=0; i<NF; ++i, ++n, ++pf)
		{
			FSFace& face = *(pf->m_pi);
			FSCoreMesh* pm = pf->m_pm;
			FSElement& el = pm->Element(face.m_elem[0]);
			nn[0] = el.m_ntag;
			nn[1] = face.m_face+1;
	
			ef.name((face.m_nodes==3?"tri3":"quad4"));
			ef.add_attribute("id", n);
			ef.value(nn, 2);

			m_xml.add_leaf(ef);
		}
	}
	else
*/	{
		int nfn;
		for (int j=0; j<NF; ++j, ++n, ++pf)
		{
			if (pf->m_pi == 0) throw InvalidItemListBuilder(0);
			FSFace& face = *(pf->m_pi);
			FSCoreMesh* pm = pf->m_pm;
			nfn = face.Nodes();
			for (int k=0; k<nfn; ++k) nn[k] = pm->Node(face.n[k]).m_nid;
			switch(nfn)
			{
			case 3: ef.name("tri3" ); break;
			case 4: ef.name("quad4"); break;
			case 6: ef.name("tri6" ); break;
			case 7: ef.name("tri7" ); break;
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
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteElementList(FEElemList& el)
{
	int NE = el.Size();
	FEElemList::Iterator pe = el.First();
	for (int i=0; i<NE; ++i, ++pe)
	{
		FEElement_& el = *(pe->m_pi);
		XMLElement e("elem");
		e.add_attribute("id", el.m_nid);
		m_xml.add_empty(e);
	}
}

//-----------------------------------------------------------------------------
void FEBioExport25::WriteOutputSection()
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
		for (int i=0; i<N; ++i) if (plt.PlotVariable(i).isActive()) na++;

		if (na > 0)
		{
			m_xml.add_branch(p);
			{
				for (int i=0; i<N; ++i) 
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
								for (int n=0; n<v.Domains(); ++n)
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
			for (int i=0; i<N; ++i)
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

						FSLogConnectorData& cd = dynamic_cast<FSLogConnectorData&>(d);
						FSRigidConnector* rc = fem.GetRigidConnectorFromID(cd.GetConnectorID());
                        if (rc)
                        {
                            e.value(cd.GetConnectorID());
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

void FEBioExport25::WriteStepSection()
{
	// we've already written the initial step
	// so now we simply output all the analysis steps
	for (int i=1; i<m_pfem->Steps(); ++i)
	{
		FSAnalysisStep& s = dynamic_cast<FSAnalysisStep&>(*m_pfem->GetStep(i));
		if (m_writeNotes) m_xml.add_comment(s.GetInfo());

		XMLElement e;
		e.name("Step");
		if (s.GetName().empty() == false) e.add_attribute("name", s.GetName().c_str());

		m_xml.add_branch(e);
		{
			// output control section
			m_xml.add_branch("Control");
			{
				WriteControlSection(&s);
			}
			m_xml.close_branch(); // Control

			// initial conditions
			int nic = s.ICs();
			if (nic > 0)
			{
				m_xml.add_branch("Initial");
				{
					WriteInitialSection(s);
				}
				m_xml.close_branch(); // Initial
			}

			// output boundary section
			int nbc = s.BCs() + s.Interfaces() + s.RigidConstraints();
			if (nbc>0)
			{
				m_xml.add_branch("Boundary");
				{
					WriteBoundarySection(s);
				}
				m_xml.close_branch(); // Boundary
			}

			// output loads section
			int nlc = s.Loads();
			if (nlc>0)
			{
				m_xml.add_branch("Loads");
				{
					WriteLoadsSection(s);
				}
				m_xml.close_branch(); // Loads
			}

			// output contact section
			int nci = s.Interfaces();
			if (nci)
			{
				m_xml.add_branch("Contact");
				{
					WriteContactSection(s);
				}
				m_xml.close_branch();
			}

			// output constraint section
			int nnlc = s.RigidConstraints() + CountInterfaces<FSRigidJoint>(*m_pfem) + s.RigidConnectors() + s.Constraints();
			if (nnlc > 0)
			{
				m_xml.add_branch("Constraints");
				{
					WriteConstraintSection(s);
				}
				m_xml.close_branch(); // Constraints
			}
		}
		m_xml.close_branch(); // Step
	}
}

//-----------------------------------------------------------------------------

void FEBioExport25::WriteRigidConstraints(FSStep &s)
{
	const char* szbc[6] = { "x", "y", "z", "Rx", "Ry", "Rz" };

	for (int i=0; i<s.RigidConstraints(); ++i)
	{
		FSRigidConstraint* ps = s.RigidConstraint(i);
		if (ps->IsActive())
		{
			GMaterial* pgm = m_pfem->GetMaterialFromID(ps->GetMaterialID());
			if (pgm == 0) throw MissingRigidBody(ps->GetName().c_str());
			if (pgm->GetMaterialProperties()->IsRigid() == false) throw InvalidMaterialReference();

			if (ps->Type() == FE_RIGID_FIXED)
			{
				FSRigidFixed* rc = dynamic_cast<FSRigidFixed*>(ps);
				XMLElement el;
				el.name("rigid_body");
				el.add_attribute("mat", pgm->m_ntag);
				m_xml.add_branch(el);
				{
					for (int j = 0; j < 6; ++j)
						if (rc->GetDOF(j))
						{
							XMLElement el("fixed");
							el.add_attribute("bc", szbc[j]);
							m_xml.add_empty(el);
						}
				}
				m_xml.close_branch();
			}
			else if (ps->Type() == FE_RIGID_DISPLACEMENT)
			{
				FSRigidPrescribed* rc = dynamic_cast<FSRigidPrescribed*>(ps);
				XMLElement el;
				el.name("rigid_body");
				el.add_attribute("mat", pgm->m_ntag);
				m_xml.add_branch(el);
				{
					XMLElement el("prescribed");
					el.add_attribute("bc", szbc[rc->GetDOF()]);

					int lc = GetLC(&rc->GetParam(FSRigidPrescribed::VALUE));
					if (lc > 0) el.add_attribute("lc", lc);

					el.value(rc->GetValue());
					m_xml.add_leaf(el);
				}
				m_xml.close_branch();
			}
			else if (ps->Type() == FE_RIGID_FORCE)
			{
				FSRigidForce* rc = dynamic_cast<FSRigidForce*>(ps);
				XMLElement el;
				int forceType = rc->GetForceType();
				el.name("rigid_body");
				el.add_attribute("mat", pgm->m_ntag);
				m_xml.add_branch(el);
				{
					XMLElement el("force");
					el.add_attribute("bc", szbc[rc->GetDOF()]);

					int lc = GetLC(&rc->GetParam(FSRigidPrescribed::VALUE));
					if (lc > 0) el.add_attribute("lc", lc);

					el.value(rc->GetValue());
					if (forceType == 1) el.add_attribute("type", "follow");
					if (forceType == 2) el.add_attribute("type", "ramp");
					m_xml.add_leaf(el);
				}
				m_xml.close_branch();
			}
		}
	}
}

//-----------------------------------------------------------------------------
// write rigid connectors
//
void FEBioExport25::WriteConnectors(FSStep& s)
{
    for (int i=0; i<s.RigidConnectors(); ++i)
    {
        // rigid connectors
		FSRigidConnector* pj = s.RigidConnector(i);
        if (pj && pj->IsActive())
        {
			if (m_writeNotes) m_xml.add_comment(pj->GetInfo());

            XMLElement ec("constraint");
            if      (dynamic_cast<FSRigidSphericalJoint*  >(pj)) ec.add_attribute("type", "rigid spherical joint");
            else if (dynamic_cast<FSRigidRevoluteJoint*   >(pj)) ec.add_attribute("type", "rigid revolute joint");
            else if (dynamic_cast<FSRigidPrismaticJoint*  >(pj)) ec.add_attribute("type", "rigid prismatic joint");
            else if (dynamic_cast<FSRigidCylindricalJoint*>(pj)) ec.add_attribute("type", "rigid cylindrical joint");
            else if (dynamic_cast<FSRigidPlanarJoint*     >(pj)) ec.add_attribute("type", "rigid planar joint");
            else if (dynamic_cast<FSRigidLock*            >(pj)) ec.add_attribute("type", "rigid lock");
            else if (dynamic_cast<FSRigidSpring*          >(pj)) ec.add_attribute("type", "rigid spring");
            else if (dynamic_cast<FSRigidDamper*          >(pj)) ec.add_attribute("type", "rigid damper");
            else if (dynamic_cast<FSRigidAngularDamper*   >(pj)) ec.add_attribute("type", "rigid angular damper");
            else if (dynamic_cast<FSRigidContractileForce*>(pj)) ec.add_attribute("type", "rigid contractile force");
			else if (dynamic_cast<FSGenericRigidJoint*    >(pj)) ec.add_attribute("type", "generic rigid joint");
            else
                assert(false);
            
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

void FEBioExport25::WriteConstraintSection(FSStep &s)
{
	// some contact definitions are actually stored in the constraint section
	WriteConstraints(s);
    WriteConnectors(s);
	WriteRigidJoint(s);
}
