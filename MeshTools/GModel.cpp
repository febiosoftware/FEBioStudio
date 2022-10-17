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
#include <sstream>
#include "GModel.h"
#include <GeomLib/GPrimitive.h>
#include <GeomLib/GMultiPatch.h>
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GSurfaceMeshObject.h>
#include <GeomLib/GCurveMeshObject.h>
#include <GeomLib/GOCCObject.h>
#include "GModifiedObject.h"
#include <FSCore/FSObjectList.h>
#include <MeshTools/GDiscreteObject.h>
#include <MeshTools/FEItemListBuilder.h>
#include <MeshTools/GGroup.h>
#include <MeshLib/FEMesh.h>
#include <FEMLib/FEAnalysisStep.h>
#include <GeomLib/MeshLayer.h>


using std::stringstream;
//=============================================================================
GNodeIterator::GNodeIterator(GModel& m) : m_mdl(m)
{
	reset();
}

void GNodeIterator::operator ++ ()
{
	if (m_node != -1)
	{
		m_node++;
		if (m_node >= m_mdl.Object(m_obj)->Nodes())
		{
			m_obj++;
			m_node = 0;

			if (m_obj >= m_mdl.Objects())
			{
				m_node = -1;
				m_obj = -1;
			}
		}
	}
}

GNode* GNodeIterator::operator -> ()
{
	if (m_node >= 0)
	{
		return m_mdl.Object(m_obj)->Node(m_node);
	}
	else
		return 0;
}

GNodeIterator::operator GNode* ()
{
	if (m_node >= 0)
	{
		return m_mdl.Object(m_obj)->Node(m_node);
	}
	else
		return 0;
}


bool GNodeIterator::isValid() const { return (m_node >= 0); }

void GNodeIterator::reset()
{
	if (m_mdl.Objects() > 0)
	{
		m_node = 0;
		m_obj = 0;
	}
	else m_node = m_obj = -1;
}

//=============================================================================
GPartIterator::GPartIterator(GModel& m) : m_mdl(m)
{
	reset();
}

void GPartIterator::operator ++ ()
{
	if (m_part != -1)
	{
		m_part++;
		if (m_part >= m_mdl.Object(m_obj)->Parts())
		{
			m_obj++;
			m_part = 0;

			if (m_obj >= m_mdl.Objects())
			{
				m_part = -1;
				m_obj = -1;
			}
		}
	}
}

GPart* GPartIterator::operator -> ()
{
	if (m_part >= 0)
	{
		return m_mdl.Object(m_obj)->Part(m_part);
	}
	else
		return nullptr;
}

GPartIterator::operator GPart* ()
{
	if (m_part >= 0)
	{
		return m_mdl.Object(m_obj)->Part(m_part);
	}
	else
		return 0;
}


bool GPartIterator::isValid() const { return (m_part >= 0); }

void GPartIterator::reset()
{
	if (m_mdl.Objects() > 0)
	{
		m_part = 0;
		m_obj = 0;
	}
	else m_part = m_obj = -1;
}

//=================================================================================================

class GModel::Imp
{
public:
	Imp()
	{
		m_mlm = nullptr;
		m_ps = nullptr;

        m_loadOnlyDiscrete = false;
	}

	~Imp()
	{
		delete m_mlm;
	}

	void ValidateNames(GObject* po);

public:
	GModel*				m_parent;
	FSModel*			m_ps;	//!< pointer to model
	BOX					m_box;	//!< bounding box

	MeshLayerManager*	m_mlm;

	FSObjectList<GObject>	m_Obj;	//!< list of objects

	FSObjectList<GPartList>	m_GPart;	//!< list of GPartGroup
	FSObjectList<GFaceList>	m_GFace;	//!< list of GFaceGroup
	FSObjectList<GEdgeList>	m_GEdge;	//!< list of GEdgeGroup
	FSObjectList<GNodeList>	m_GNode;	//!< list of GNodeGroup

	FSObjectList<GDiscreteObject>	m_Discrete;	//!< list of discrete objects

    bool m_loadOnlyDiscrete;
};

void GModel::Imp::ValidateNames(GObject* po)
{
	if (m_Obj.Size() == 0) return;

	// check object name
	string oldName = po->GetName();
	string newName = oldName;
	int n = 1;
	bool bok = false;
	while (bok == false)
	{
		bok = true;
		for (int i = 0; i < m_Obj.Size(); ++i)
		{
			GObject* poi = m_Obj[i];
			if (poi->GetName() == newName)
			{
				bok = false;
				stringstream ss;
				ss << oldName << "(" << n << ")";
				newName = ss.str();
				po->SetName(newName);
				n++;
				break;
			}
		}
	}

	// check part names of new object
	for (int i = 0; i < po->Parts(); ++i)
	{
		GPart* pg = po->Part(i);
		string oldName = pg->GetName();
		string newName = oldName;
		int n = 1;
		bool bok = false;
		while (bok == false)
		{
			bok = true;
			GPartIterator it(*m_parent);
			while (it.isValid())
			{
				GPart* pgi = it;
				if (pgi->GetName() == newName)
				{
					bok = false;
					stringstream ss;
					ss << oldName << "(" << n << ")";
					newName = ss.str();
					pg->SetName(newName);
					n++;
					break;
				}
				++it;
			}
		}
	}
}

//-----------------------------------------------------------------------------
GModel::GModel(FSModel* ps): imp(new GModel::Imp)
{
	SetName("Model");
	imp->m_parent = this;
	imp->m_ps = ps;
	imp->m_mlm = new MeshLayerManager(this);
	imp->m_mlm->AddLayer("Default");
}

//-----------------------------------------------------------------------------
GModel::~GModel(void)
{
	Clear();
	delete imp;
}

//-----------------------------------------------------------------------------
FSModel* GModel::GetFSModel()
{
	return imp->m_ps;
}

//-----------------------------------------------------------------------------
void GModel::Clear()
{
	// cleanup all objects
	imp->m_Obj.Clear();

	// clear mesh layers
	delete imp->m_mlm;
	imp->m_mlm = new MeshLayerManager(this);
	imp->m_mlm->AddLayer("Default");

	// cleanup all groups
	imp->m_GPart.Clear();
	imp->m_GFace.Clear();
	imp->m_GEdge.Clear();
	imp->m_GNode.Clear();

	// cleanup discrete objects
	imp->m_Discrete.Clear();
}

//-----------------------------------------------------------------------------
void GModel::ClearGroups()
{
	// cleanup all groups
	imp->m_GPart.Clear();
	imp->m_GFace.Clear();
	imp->m_GEdge.Clear();
	imp->m_GNode.Clear();
}

//-----------------------------------------------------------------------------
void GModel::ClearDiscrete()
{
	// cleanup discrete objects
	imp->m_Discrete.Clear();
}

//-----------------------------------------------------------------------------
int GModel::RemoveObject(GObject* po, bool deleteMeshList)
{
	// remove the object from the list
	size_t n = imp->m_Obj.Remove(po);

	// remove it from the mesh layers
	imp->m_mlm->RemoveObject(po, deleteMeshList);

	// update the bounding box
	UpdateBoundingBox();

	// note that we don't delete the actual object here.
	// we let the function that called this function deal with that
	return (int)n;
}

//-----------------------------------------------------------------------------

void GModel::InsertObject(GObject* po, int n, bool updateManager)
{
	assert( (n>=0) && (n<=Objects()) );

	// insert the mesh to the list
	imp->m_Obj.Insert(n, po);

	// insert the object in the mesh layer manager
	if (updateManager) imp->m_mlm->InsertObject(n, po);

	// update bounding box
	UpdateBoundingBox();
}

//-----------------------------------------------------------------------------
int GModel::DiscreteObjects() { return (int) imp->m_Discrete.Size(); }

//-----------------------------------------------------------------------------
void GModel::AddDiscreteObject(GDiscreteObject* po) { imp->m_Discrete.Add(po); }

//-----------------------------------------------------------------------------
GDiscreteObject* GModel::DiscreteObject(int n)
{ 
	if (n < imp->m_Discrete.Size())
		return imp->m_Discrete[n];
	else
	{
		int nid = n - (int)imp->m_Discrete.Size();
		for (int i=0; i<(int)imp->m_Discrete.Size(); ++i)
		{
			GDiscreteElementSet* ps = dynamic_cast<GDiscreteElementSet*>(imp->m_Discrete[i]);
			if (ps)
			{
				int index = ps->FindElement(nid);
				if (index >= 0)
				{
					return &ps->element(index);
				}
			}
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------

int GModel::RemoveDiscreteObject(GDiscreteObject* po)
{
	// find where this object is located
	for (int i=0; i<DiscreteObjects(); ++i) 
	{
		GDiscreteObject* pdi = imp->m_Discrete[i];
		if (pdi == po)
		{
			// remove the mesh from the list
			imp->m_Discrete.Remove(pdi);
			return i;
		}
		else if (dynamic_cast<GDiscreteElementSet*>(pdi))
		{
			// The discrete object can also be a part of a set
			GDiscreteElementSet* pds = dynamic_cast<GDiscreteElementSet*>(pdi);

			// see if this belongs to the set
			int NE = pds->size();
			for (int j=0; j<NE; ++j)
			{
				GDiscreteObject* pj = &pds->element(j);
				if (pj == po)
				{
					pds->RemoveElement(j);
					return -1;
				}
			}
		}
	}
	return -1;
}

//-----------------------------------------------------------------------------

void GModel::InsertDiscreteObject(GDiscreteObject* po, int n)
{
	assert( (n>=0) && (n<=DiscreteObjects()) );

	// insert the mesh to the list
	imp->m_Discrete.Insert(n, po);
}

//-----------------------------------------------------------------------------
int GModel::FindDiscreteObjectIndex(GDiscreteObject* po)
{
	if (po == 0) return -1;

	int N = DiscreteObjects();
	for (int i = 0; i<N; ++i)
	{
		GDiscreteObject* pi = DiscreteObject(i);
		if (pi == po) return i;
	}
	return -1;
}

//-----------------------------------------------------------------------------
GDiscreteObject* GModel::FindDiscreteObject(const std::string& name)
{
	int N = DiscreteObjects();
	for (int i = 0; i<N; ++i)
	{
		GDiscreteObject* pi = DiscreteObject(i);
		if (pi->GetName() == name) return pi;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// return number of objects
int GModel::Objects() const
{ 
	return (int)imp->m_Obj.Size();
}

//-----------------------------------------------------------------------------
// return an object
GObject* GModel::Object(int n)
{ 
	return ((n >= 0) && (n<(int)imp->m_Obj.Size()) ? imp->m_Obj[n] : 0);
}

//-----------------------------------------------------------------------------
GObject* GModel::FindObject(int id)
{
	int N = Objects();
	for (int i=0; i<N; ++i) 
	{
		GObject* po = Object(i);
		if (po->GetID() == id) return po;
	}

	return 0;
}

//-----------------------------------------------------------------------------

GObject* GModel::FindObject(const string& name)
{
	int N = Objects();
	for (int i=0; i<N; ++i) 
	{
		GObject* po = imp->m_Obj[i];
		if (name == po->GetName()) return po;
	}

	return 0;
}

//-----------------------------------------------------------------------------

int GModel::FindObjectIndex(GObject* po)
{
	int N = Objects();
	for (int i=0; i<N; ++i)
	{
		if (imp->m_Obj[i] == po) return i;
	}

	return -1;
}

//-----------------------------------------------------------------------------

void GModel::ReplaceObject(int n, GObject* po)
{
	assert((n>=0) && (n<(int)imp->m_Obj.Size()));
	RemoveObject(Object(n));
	InsertObject(po, n);
}

//-----------------------------------------------------------------------------

void GModel::ReplaceObject(GObject* po, GObject* pn)
{
	int n = FindObjectIndex(po);
	ReplaceObject(n, pn);
}

//-----------------------------------------------------------------------------
void GModel::AddObject(GObject* po)
{ 
	// Make sure the object does not already exist
	if (imp->m_Obj.Find(po) >= 0)
	{
		assert(false);
		return;
	}

	// before we can add it, we must ensure that the names of this object
	// and all parts, surfaces, edges, and nodes are unique. 
	if (Objects() > 0) imp->ValidateNames(po);

	// add the object to the object list
	imp->m_Obj.Add(po);

	// add the object to the layers
	imp->m_mlm->AddObject(po);

	UpdateBoundingBox();
}

//-----------------------------------------------------------------------------

int GModel::Parts()
{
	int n = 0;
	for (int i=0; i<Objects(); ++i) n += Object(i)->Parts();
	return n;
}

//-----------------------------------------------------------------------------

GPart* GModel::Part(int n)
{
	int m = 0;
	for (int i=0; i<Objects(); ++i)
	{
		GObject* po = Object(i);
		for (int j=0; j<po->Parts(); ++j, ++m)
			if (n == m) return po->Part(j);
	}
	return 0;
}

//-----------------------------------------------------------------------------

GPart* GModel::FindPart(int nid)
{
	for (int i=0; i<Objects(); ++i)
	{
		GObject* po = Object(i);
		GPart* pg = po->FindPart(nid);
		if (pg) return pg;
	}
	return 0;
}


//-----------------------------------------------------------------------------

int GModel::Surfaces()
{
	int n = 0;
	for (int i=0; i<Objects(); ++i) n += Object(i)->Faces();
	return n;
}

//-----------------------------------------------------------------------------

GFace* GModel::Surface(int n)
{
	int nobj = Objects();
	for (int i=0; i<nobj; ++i)
	{
		GObject* po = Object(i);
		int nf = po->Faces();
		if (nf <= n) n -= nf;
		else return po->Face(n); 
	}
	return 0;
}

//-----------------------------------------------------------------------------

GFace* GModel::FindSurface(int nid)
{
	for (int i=0; i<Objects(); ++i)
	{
		GObject* po = Object(i);
		GFace* pg = po->FindFace(nid);
		if (pg) return pg;
	}
	return 0;
}

//-----------------------------------------------------------------------------

int GModel::Edges()
{
	int n = 0;
	for (int i=0; i<Objects(); ++i) n += Object(i)->Edges();
	return n;
}

//-----------------------------------------------------------------------------

GEdge* GModel::Edge(int n)
{
	int m = 0;
	for (int i=0; i<Objects(); ++i)
	{
		GObject* po = Object(i);
		int ne = po->Edges();
		if (ne <= n) n -= ne;
		else return po->Edge(n);
	}
	return 0;
}

//-----------------------------------------------------------------------------

GEdge* GModel::FindEdge(int nid)
{
	for (int i=0; i<Objects(); ++i)
	{
		GObject* po = Object(i);
		GEdge* pg = po->FindEdge(nid);
		if (pg) return pg;
	}
	return 0;
}

//-----------------------------------------------------------------------------

GEdge* GModel::FindEdgeFromName(const string& name)
{
	for (int i = 0; i<Objects(); ++i)
	{
		GObject* po = Object(i);
		int NE = po->Edges();
		for (int j=0; j<NE; ++j)
		{
			GEdge* pg = po->Edge(j);
			if (pg->GetName() == name) return pg;
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------

int GModel::Nodes()
{
	int n = 0;
	for (int i=0; i<Objects(); ++i) n += Object(i)->Nodes();
	return n;
}

//-----------------------------------------------------------------------------

GNode* GModel::Node(int n)
{
	int m = 0;
	for (int i=0; i<Objects(); ++i)
	{
		GObject* po = Object(i);
		int nn = po->Nodes();
		if (nn <= n) n -= nn;
		else return po->Node(n);
	}
	return 0;
}

//-----------------------------------------------------------------------------

GNode* GModel::FindNode(int nid)
{
	for (int i=0; i<Objects(); ++i)
	{
		GObject* po = Object(i);
		GNode* pg = po->FindNode(nid);
		if (pg) return pg;
	}
	return 0;
}

//-----------------------------------------------------------------------------

int GModel::FENodes()
{
	int nodes = 0;
	for (int i=0; i<(int)imp->m_Obj.Size(); ++i)
	{
		FSMesh* pm = imp->m_Obj[i]->GetFEMesh();
		if (pm) nodes += pm->Nodes();
	}
	return nodes;
}

//-----------------------------------------------------------------------------

int GModel::FEFaces()
{
	int faces = 0;
	for (int i=0; i<(int)imp->m_Obj.Size(); ++i)
	{
		FSMesh* pm = imp->m_Obj[i]->GetFEMesh();
		if (pm) faces += pm->Faces();
	}
	return faces;
}

//-----------------------------------------------------------------------------

int GModel::Elements()
{
	int elems = 0;
	for (int i=0; i<(int)imp->m_Obj.Size(); ++i)
	{
		FSMesh* pm = imp->m_Obj[i]->GetFEMesh();
		if (pm) elems += pm->Elements();
	}
	return elems;
}

//-----------------------------------------------------------------------------

int GModel::SolidElements()
{
	int elems = 0;
	for (int i=0; i<(int)imp->m_Obj.Size(); ++i)
	{
		FSMesh* pm = imp->m_Obj[i]->GetFEMesh();
		for (int j=0; j<pm->Elements(); ++j)
		{
			FSElement& el = pm->Element(j);
			if (el.IsSolid()) ++elems;
		}
	}
	return elems;
}

//-----------------------------------------------------------------------------

int GModel::ShellElements()
{
	int elems = 0;
	for (int i=0; i<(int)imp->m_Obj.Size(); ++i)
	{
		FSMesh* pm = imp->m_Obj[i]->GetFEMesh();
		for (int j=0; j<pm->Elements(); ++j)
		{
			FSElement& el = pm->Element(j);
			if (el.IsShell()) 
			{
				++elems;
			}
		}
	}
	return elems;
}


//-----------------------------------------------------------------------------

FSNodeSet* GModel::GetNodesetFromID(int id)
{
	int i, j;
	for (i=0; i<(int)imp->m_Obj.Size(); ++i)
	{
		GObject* po = imp->m_Obj[i];
		for (j=0; j<po->FENodeSets(); ++j)
		{
			FSNodeSet* pn = po->GetFENodeSet(j);
			if (pn->GetID() == id) return pn;
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------

FSSurface* GModel::GetSurfaceFromID(int id)
{
	int i, j;
	for (i=0; i<(int)imp->m_Obj.Size(); ++i)
	{
		GObject* po = imp->m_Obj[i];
		for (j=0; j<po->FESurfaces(); ++j)
		{
			FSSurface* ps = po->GetFESurface(j);
			if (ps->GetID() == id) return ps;
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------

BOX GModel::GetBoundingBox()
{
	return imp->m_box;
}

//-----------------------------------------------------------------------------
void GModel::UpdateBoundingBox()
{
	if (imp->m_Obj.Size() == 0)
	{
		imp->m_box = BOX(-1, -1, -1, 1, 1, 1);
	}
	else
	{
		imp->m_box = imp->m_Obj[0]->GetGlobalBox();
		for (int i = 1; i<(int)imp->m_Obj.Size(); ++i) imp->m_box += imp->m_Obj[i]->GetGlobalBox();
	}
}

//-----------------------------------------------------------------------------
// count named selections
int GModel::CountNamedSelections() const
{
	int nsel = 0;
	nsel += PartLists();
	nsel += FaceLists();
	nsel += EdgeLists();
	nsel += NodeLists();

	for (int i=0; i<Objects(); ++i)
	{
		const GObject* obj = imp->m_Obj[i];
		nsel += obj->FENodeSets();
		nsel += obj->FESurfaces();
		nsel += obj->FEEdgeSets();
		nsel += obj->FEParts();
	}

	return nsel;
}

//-----------------------------------------------------------------------------
FEItemListBuilder* GModel::FindNamedSelection(int nid)
{
	// don't bother looking if the ID is invalid
	if (nid < 0) return 0;

	int i, N;

	FEItemListBuilder* pg = 0;

	// search the GGroups
	N = PartLists();
	for (i = 0; i<N; ++i)
	{
		pg = PartList(i);
		if (pg->GetID() == nid) return pg;
	}

	N = FaceLists();
	for (i = 0; i<N; ++i)
	{
		pg = FaceList(i);
		if (pg->GetID() == nid) return pg;
	}

	N = EdgeLists();
	for (i = 0; i<N; ++i)
	{
		pg = EdgeList(i);
		if (pg->GetID() == nid) return pg;
	}

	N = NodeLists();
	for (i = 0; i<N; ++i)
	{
		pg = NodeList(i);
		if (pg->GetID() == nid) return pg;
	}

	// search all objects
	for (int n = 0; n<Objects(); ++n)
	{
		GObject* po = Object(n);
		FSMesh* pm = po->GetFEMesh();

		N = po->FEParts();
		for (i = 0; i<N; ++i)
		{
			pg = po->GetFEPart(i);
			if (pg->GetID() == nid) return pg;
		}

		N = po->FESurfaces();
		for (i = 0; i<N; ++i)
		{
			pg = po->GetFESurface(i);
			if (pg->GetID() == nid) return pg;
		}

		N = po->FENodeSets();
		for (i = 0; i<N; ++i)
		{
			pg = po->GetFENodeSet(i);
			if (pg->GetID() == nid) return pg;
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------
FEItemListBuilder* GModel::FindNamedSelection(const std::string& name)
{
	int i, N;
	FEItemListBuilder* pg = 0;

	// search the GGroups
	N = PartLists();
	for (i = 0; i<N; ++i)
	{
		pg = PartList(i);
		if (pg->GetName() == name) return pg;
	}

	N = FaceLists();
	for (i = 0; i<N; ++i)
	{
		pg = FaceList(i);
		if (pg->GetName() == name) return pg;
	}

	N = EdgeLists();
	for (i = 0; i<N; ++i)
	{
		pg = EdgeList(i);
		if (pg->GetName() == name) return pg;
	}

	N = NodeLists();
	for (i = 0; i<N; ++i)
	{
		pg = NodeList(i);
		if (pg->GetName() == name) return pg;
	}

	// search all objects
	for (int n = 0; n<Objects(); ++n)
	{
		GObject* po = Object(n);
		FSMesh* pm = po->GetFEMesh();

		N = po->FEParts();
		for (i = 0; i<N; ++i)
		{
			pg = po->GetFEPart(i);
			if (pg->GetName() == name) return pg;
		}

		N = po->FESurfaces();
		for (i = 0; i<N; ++i)
		{
			pg = po->GetFESurface(i);
			if (pg->GetName() == name) return pg;
		}

		N = po->FENodeSets();
		for (i = 0; i<N; ++i)
		{
			pg = po->GetFENodeSet(i);
			if (pg->GetName() == name) return pg;
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------
vector<FEItemListBuilder*> GModel::AllNamedSelections(int ntype)
{
	vector<FEItemListBuilder*> list;

	if (ntype == DOMAIN_PART)
	{
		for (int i = 0; i<PartLists(); ++i)
		{
			FEItemListBuilder* pg = PartList(i);
			list.push_back(pg);
		}
	}

	if (ntype == DOMAIN_SURFACE)
	{
		for (int i = 0; i<FaceLists(); ++i)
		{
			FEItemListBuilder* pg = FaceList(i);
			list.push_back(pg);
		}
	}

	if (ntype == DOMAIN_EDGE)
	{
		for (int i = 0; i<EdgeLists(); ++i)
		{
			FEItemListBuilder* pg = EdgeList(i);
			list.push_back(pg);
		}
	}

	if (ntype == DOMAIN_NODESET)
	{
		for (int i = 0; i<NodeLists(); ++i)
		{
			FEItemListBuilder* pg = NodeList(i);
			list.push_back(pg);
		}
	}

	// search all objects
	for (int n = 0; n<Objects(); ++n)
	{
		GObject* po = Object(n);
		FSMesh* pm = po->GetFEMesh();

		if (ntype == DOMAIN_PART)
		{
			for (int i = 0; i<po->FEParts(); ++i)
			{
				FEItemListBuilder* pg = po->GetFEPart(i);
				list.push_back(pg);
			}
		}

		if (ntype == DOMAIN_SURFACE)
		{
			for (int i = 0; i<po->FESurfaces(); ++i)
			{
				FEItemListBuilder*pg = po->GetFESurface(i);
				list.push_back(pg);
			}
		}

		if (ntype == DOMAIN_NODESET)
		{
			for (int i = 0; i<po->FENodeSets(); ++i)
			{
				FEItemListBuilder* pg = po->GetFENodeSet(i);
				list.push_back(pg);
			}
		}
	}

	return list;
}

//-----------------------------------------------------------------------------
void GModel::Save(OArchive &ar)
{
	ar.WriteChunk(CID_FEOBJ_INFO, GetInfo());

	// save the objects
	ar.BeginChunk(CID_OBJ_GOBJECTS);
	{
		for (int i=0; i<(int)imp->m_Obj.Size(); ++i)
		{
			GObject* po = imp->m_Obj[i];
			int ntype = po->GetType();
			ar.BeginChunk(ntype);
			{
				po->Save(ar);
			}
			ar.EndChunk();
		}
	}
	ar.EndChunk();

	// save mesh layers
	ar.BeginChunk(CID_MESH_LAYERS);
	{
		imp->m_mlm->Save(ar);
	}
	ar.EndChunk();

	// save the parts
	for (int i=0; i<(int)imp->m_GPart.Size(); ++i)
	{
		ar.BeginChunk(CID_OBJ_GPARTGROUP);
		{
			imp->m_GPart[i]->Save(ar);
		}
		ar.EndChunk();
	}

	// save the surfaces
	for (int i=0; i<(int)imp->m_GFace.Size(); ++i)
	{
		ar.BeginChunk(CID_OBJ_GFACEGROUP);
		{
			imp->m_GFace[i]->Save(ar);
		}
		ar.EndChunk();
	}

	// save the edges
	for (int i=0; i<(int)imp->m_GEdge.Size(); ++i)
	{
		ar.BeginChunk(CID_OBJ_GEDGEGROUP);
		{
			imp->m_GEdge[i]->Save(ar);
		}
		ar.EndChunk();
	}

	// save the nodes
	for (int i=0; i<(int)imp->m_GNode.Size(); ++i)
	{
		ar.BeginChunk(CID_OBJ_GNODEGROUP);
		{
			imp->m_GNode[i]->Save(ar);
		}
		ar.EndChunk();
	}

	// save the discrete objects
	if (imp->m_Discrete.Size() > 0)
	{
		ar.BeginChunk(CID_DISCRETE_OBJECT);
		{
			for (int i=0; i<(int)imp->m_Discrete.Size(); ++i)
			{
				int ntype = imp->m_Discrete[i]->GetType();
				ar.BeginChunk(ntype);
				{
					imp->m_Discrete[i]->Save(ar);
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
GObject* BuildObject(int ntype)
{
	GObject* po = 0;
	switch (ntype)
	{
	case GOBJECT			: po = new GObject(GOBJECT); break;
	case GBOX               : po = new GBox           (); break;
	case GCYLINDER          : po = new GCylinder      (); break;
	case GCYLINDER2         : po = new GCylinder2     (); break;
	case GSPHERE            : po = new GSphere        (); break;
//	case GSPRING            : po = new GSpring		   (); break;
	case GTORUS             : po = new GTorus         (); break;
	case GTUBE              : po = new GTube          (); break;
	case GTUBE2             : po = new GTube2         (); break;
	case GCONE              : po = new GCone          (); break;
	case GHOLLOW_SPHERE     : po = new GHollowSphere  (); break;
	case GDISC	            : po = new GDisc          (); break;
	case GPATCH	            : po = new GPatch         (); break;
	case GRING	            : po = new GRing          (); break;
//	case GSHELL_SPHERE      : po = new GShellSphere   (); break;
//	case GSHELL_TORUS       : po = new GShellTorus    (); break;
	case GSHELL_TUBE        : po = new GThinTube      (); break;
	case GCURVE             : po = new GObject        (GCURVE); break;
//	case GCURVE_CIRCLE      : po = new GCircle        (); break;
	case GMESH_OBJECT       : po = new GMeshObject    ((FSMesh*)0); break;
	case GMODIFIED_OBJECT   : po = new GModifiedObject(0); break;
	case GSLICE		        : po = new GSlice         (); break;
	case GTRUNC_ELLIPSOID   : po = new GTruncatedEllipsoid(); break;
	case GQUART_DOG_BONE    : po = new GQuartDogBone(); break;
	case GSOLIDARC          : po = new GSolidArc    (); break;
	case GCYLINDER_IN_BOX   : po = new GCylinderInBox(); break;
	case GSPHERE_IN_BOX     : po = new GSphereInBox  (); break;
	case GSURFACEMESH_OBJECT: po = new GSurfaceMeshObject(); break;
	case GCURVEMESH_OBJECT  : po = new GCurveMeshObject(); break;
	case GOCCOBJECT         : po = new GOCCObject(); break;
	case GOCC_BOTTLE        : po = new GOCCBottle(); break;
	case GOCC_BOX           : po = new GOCCBox(); break;
	case GCYLINDRICAL_PATCH : po = new GCylindricalPatch(); break;
	case GMULTI_BLOCK       : po = new GMultiBox(); break;
	case GMULTI_PATCH       : po = new GMultiPatch(); break;
	}

	assert(po);
	return po;
}

//-----------------------------------------------------------------------------
void GModel::Load(IArchive &ar)
{
	TRACE("GModel::Load");

	// re-allocate mesh layer manager
	delete imp->m_mlm;
	imp->m_mlm = new MeshLayerManager(this);
	imp->m_mlm->AddLayer("Default");

    if(imp->m_loadOnlyDiscrete)
    {
        while (IArchive::IO_OK == ar.OpenChunk())
        {
            int nid = ar.GetChunkID();

            switch (nid)
            {
                case CID_DISCRETE_OBJECT:
                {
                    LoadDiscrete(ar);
                }
                break;
            }

            ar.CloseChunk();
        }
    }
    else
    {
        while (IArchive::IO_OK == ar.OpenChunk())
        {
            int nid = ar.GetChunkID();

            switch (nid)
            {
            case CID_FEOBJ_INFO:
                {
                    string info;
                    ar.read(info);
                    SetInfo(info);
                }
            break;
            case CID_MESH_LAYERS:
                {
                    imp->m_mlm->Load(ar);
                }
                break;
            case CID_OBJ_GOBJECTS:
                {
                    while (IArchive::IO_OK == ar.OpenChunk())
                    {
                        int ntype = ar.GetChunkID();

                        // create a new geometry object
                        GObject* po = BuildObject(ntype);
                        if (po == 0) throw ReadError("error parsing CID_OBJ_GOBJECTS in GModel::Load");

                        // add object to the model
                        AddObject(po);

                        // load the object data
                        po->Load(ar);

                        ar.CloseChunk();
                    }
                }
                break;
            case CID_OBJ_GPARTGROUP:
                {
                    GPartList* pg = new GPartList(imp->m_ps);
                    pg->Load(ar);
                    AddPartList(pg);
                }
                break;
            case CID_OBJ_GFACEGROUP:
                {
                    GFaceList* pg = new GFaceList(imp->m_ps);
                    pg->Load(ar);
                    AddFaceList(pg);
                }
                break;
            case CID_OBJ_GEDGEGROUP:
                {
                    GEdgeList* pg = new GEdgeList(imp->m_ps);
                    pg->Load(ar);
                    AddEdgeList(pg);
                }
                break;
            case CID_OBJ_GNODEGROUP:
                {
                    GNodeList* pg = new GNodeList(imp->m_ps);
                    pg->Load(ar);
                    AddNodeList(pg);
                }
                break;
            case CID_DISCRETE_OBJECT:
                {
                    LoadDiscrete(ar);
                }
                break;
            }

            ar.CloseChunk();
        }
    }

	UpdateBoundingBox();
}

//-----------------------------------------------------------------------------

void GModel::LoadDiscrete(IArchive& ar)
{
	FSModel* fem = imp->m_ps;

    while (IArchive::IO_OK == ar.OpenChunk())
    {
        int ntype = ar.GetChunkID();

        // create a new geometry object
        GDiscreteObject* po = 0;
        switch (ntype)
        {
        case FE_DISCRETE_SPRING     : po = new GLinearSpring(this); break;
        case FE_GENERAL_SPRING      : po = new GGeneralSpring(this); break;
        case FE_DISCRETE_SPRING_SET : po = new GDiscreteSpringSet(this); break;
        case FE_LINEAR_SPRING_SET   : po = new GLinearSpringSet(this); break;
        case FE_NONLINEAR_SPRING_SET: po = new GNonlinearSpringSet(this); break;
        default:
            throw ReadError("error parsing CID_DISCRETE_OBJECT (GModel::Load)");
        }

        // load the object data
        po->Load(ar);

        // convert old objects to new format
        if (ntype == FE_LINEAR_SPRING_SET)
        {
            GDiscreteElementSet* ds = dynamic_cast<GDiscreteElementSet*>(po); assert(ds);
            GDiscreteSpringSet* pnew = new GDiscreteSpringSet(this);
            pnew->SetName(po->GetName());
            pnew->CopyDiscreteElementSet(ds);
            FSLinearSpringMaterial* mat = new FSLinearSpringMaterial(fem);
            mat->SetSpringConstant(po->GetFloatValue(GLinearSpringSet::MP_E));
            pnew->SetMaterial(mat);
            delete po;
            po = pnew;
        }
        else if (ntype == FE_NONLINEAR_SPRING_SET)
        {
            GDiscreteElementSet* ds = dynamic_cast<GDiscreteElementSet*>(po); assert(ds);
            GDiscreteSpringSet* pnew = new GDiscreteSpringSet(this);
            pnew->SetName(po->GetName());
            pnew->CopyDiscreteElementSet(ds);
            FSNonLinearSpringMaterial* mat = new FSNonLinearSpringMaterial(fem);
            // TODO: map F parameter
            pnew->SetMaterial(mat);
            delete po;
            po = pnew;
        }

        // add object to the model
        AddDiscreteObject(po);

        ar.CloseChunk();
    }
}

//-----------------------------------------------------------------------------

void GModel::AddPartList(GPartList *pg)
{
	imp->m_GPart.Add(pg);
}

//-----------------------------------------------------------------------------

void GModel::InsertPartList(int n, GPartList* pg)
{
	imp->m_GPart.Insert(n, pg);
}

//-----------------------------------------------------------------------------
int GModel::PartLists() const { return (int)imp->m_GPart.Size(); }

//-----------------------------------------------------------------------------
GPartList* GModel::PartList(int n) { return imp->m_GPart[n]; }

//-----------------------------------------------------------------------------

int GModel::RemovePartList(GPartList *pg)
{
	return imp->m_GPart.Remove(pg);
}

//-----------------------------------------------------------------------------

void GModel::AddFaceList(GFaceList *pg)
{
	imp->m_GFace.Add(pg);
}

//-----------------------------------------------------------------------------

void GModel::InsertFaceList(int n, GFaceList* pg)
{
	imp->m_GFace.Insert(n, pg);
}

//-----------------------------------------------------------------------------

int GModel::RemoveFaceList(GFaceList *pg)
{
	return imp->m_GFace.Remove(pg);
}

//-----------------------------------------------------------------------------
int GModel::FaceLists() const { return (int) imp->m_GFace.Size(); }

//-----------------------------------------------------------------------------
GFaceList* GModel::FaceList(int n) { return imp->m_GFace[n]; }

//-----------------------------------------------------------------------------

void GModel::AddEdgeList(GEdgeList *pg)
{
	imp->m_GEdge.Add(pg);
}

//-----------------------------------------------------------------------------
void GModel::InsertEdgeList(int n, GEdgeList* pg)
{
	imp->m_GEdge.Insert(n, pg);
}


//-----------------------------------------------------------------------------
int GModel::EdgeLists() const { return (int) imp->m_GEdge.Size(); }

//-----------------------------------------------------------------------------
GEdgeList* GModel::EdgeList(int n) { return imp->m_GEdge[n]; }

//-----------------------------------------------------------------------------

int GModel::RemoveEdgeList(GEdgeList *pg)
{
	return imp->m_GEdge.Remove(pg);
}

//-----------------------------------------------------------------------------

void GModel::AddNodeList(GNodeList *pg)
{
	imp->m_GNode.Add(pg);
}

//-----------------------------------------------------------------------------

void GModel::InsertNodeList(int n, GNodeList* pg)
{
	imp->m_GNode.Insert(n, pg);
}

//-----------------------------------------------------------------------------

int GModel::RemoveNodeList(GNodeList *pg)
{
	return imp->m_GNode.Remove(pg);
}

//-----------------------------------------------------------------------------
int GModel::NodeLists() const { return (int) imp->m_GNode.Size(); }

//-----------------------------------------------------------------------------
GNodeList* GModel::NodeList(int n) { return imp->m_GNode[n]; }

//-----------------------------------------------------------------------------
void GModel::RemoveNamedSelections()
{
	// remove all FE selections
	for (int i=0; i<(int)imp->m_Obj.Size(); ++i)
	{
		GObject& obj = *imp->m_Obj[i];
		obj.ClearFEGroups();
	}

	// remove all geometry selections
	ClearGroups();
}

//-----------------------------------------------------------------------------

template <class T> void clearList(FSObjectList<T>& l)
{
	if (l.IsEmpty()) return;
	for (int i=0; i<l.Size();)
	{
		if (l[i]->size() == 0)
		{
			l.Remove(l[i]);
		}
		else ++i;
	}
}

void GModel::RemoveEmptySelections()
{
	clearList(imp->m_GPart);
	clearList(imp->m_GFace);
	clearList(imp->m_GEdge);
	clearList(imp->m_GNode);

	for (int i=0; i<Objects(); ++i)
	{
		GObject* po = Object(i);
		po->RemoveEmptyFEGroups();
	}
}

//-----------------------------------------------------------------------------
void GModel::ShowObjects(const vector<int>& objList, bool bshow)
{
	if (bshow)
        for (int i : objList) Object(i)->Show();
	else
        for (int i : objList) Object(i)->Hide();
}

void GModel::ShowObject(GObject* po, bool bshow)
{
	if (bshow)
		po->Show();
	else
		po->Hide();
}

//-----------------------------------------------------------------------------
void GModel::SelectObjects(const vector<int>& objList)
{
    for (int i : objList)
	{
		if (Object(i)->IsVisible())
		{
			Object(i)->Select();
		}
		else
		{
			assert(false);
		}
	}
}

//-----------------------------------------------------------------------------
// show or hide a list of parts
void GModel::ShowParts(const vector<int>& partList, bool bshow, bool bselect)
{
	int N = (int)partList.size();
	for (int i = 0; i<N; ++i)
	{
		GPart* pg = FindPart(partList[i]);
		GObject* po = dynamic_cast<GObject*>(pg->Object()); assert(po);
		if (po) 
		{
			if (bshow) 
			{
				po->ShowPart(*pg, bshow);
				if (bselect) pg->Select();
			}
			else po->ShowPart(*pg, false);
		}
	}
}

//-----------------------------------------------------------------------------
// show or hide a list of parts
void GModel::ShowParts(vector<GPart*>& partList, bool bshow)
{
	for (int i=0; i<Objects(); ++i) Object(i)->m_ntag = 0;
	for (GPart* part : partList)
	{
		if (bshow) part->ShowItem(); else part->HideItem();
		GBaseObject* po = part->Object(); assert(po);
		if (po) po->m_ntag = 1;
	}

	for (int i=0; i<Objects(); ++i) 
	{
		GObject* po = Object(i);
		if (po->m_ntag == 1) po->UpdateItemVisibility();
	}
}

//-----------------------------------------------------------------------------
// show or hide a list of parts
void GModel::ShowParts(list<GPart*>& partList, bool bshow)
{
	for (int i = 0; i<Objects(); ++i) Object(i)->m_ntag = 0;
	for (GPart* part : partList)
	{
		if (bshow) part->ShowItem(); else part->HideItem();
		GBaseObject* po = part->Object(); assert(po);
		if (po) po->m_ntag = 1;
	}

	for (int i = 0; i<Objects(); ++i)
	{
		GObject* po = Object(i);
		if (po->m_ntag == 1) po->UpdateItemVisibility();
	}
}

//-----------------------------------------------------------------------------
void GModel::ShowPart(GPart* pg, bool bshow)
{
	if (pg == 0) return;

	GObject* po = dynamic_cast<GObject*>(pg->Object()); assert(po);
	if (po)
	{
		po->ShowPart(*pg, bshow);
	}
}

//-----------------------------------------------------------------------------
bool GModel::DeletePart(GPart* pg)
{
	if (pg == nullptr) return false;
	GBaseObject* po = pg->Object();
	if (po) return po->DeletePart(pg);
	else return false;
}

//-----------------------------------------------------------------------------
void GModel::ShowAllObjects()
{
	for (int i = 0; i<Objects(); ++i) ShowObject(Object(i));
}

//-----------------------------------------------------------------------------
void GModel::ShowAllParts(GObject* po)
{
	if (po) po->ShowAllParts();
}

//-----------------------------------------------------------------------------
GObject* GModel::CloneObject(GObject *po)
{
	// clone counter
	static int n = 1;

	// clone the object
	GObject* pco = po->Clone();
	if (pco == 0) return 0;

	// set a new name
	char sz[256];
	sprintf(sz, "Clone%02d", n++);
	pco->SetName(sz);

	// copy material assignments
	assert(pco->Parts() == po->Parts());
	if (pco->Parts() == po->Parts())
	{
		int NP = po->Parts();
		for (int i = 0; i<NP; ++i)
		{
			GPart* srcPart = po->Part(i);
			GPart* dstPart = pco->Part(i);

			dstPart->SetMaterialID(srcPart->GetMaterialID());
		}
	}

	return pco;
}

vector<GObject*> GModel::CloneGrid(GObject* po, int x0, int x1, int y0, int y1, int z0, int z1, double dx, double dy, double dz)
{
	// clone counter
	static int n = 1;

	// sanity checks
	if (x1 < x0) { int tmp = x0; x0 = x1; x1 = tmp; }
	if (y1 < y0) { int tmp = y0; y0 = y1; y1 = tmp; }
	if (z1 < z0) { int tmp = z0; z0 = z1; z1 = tmp; }

	// list of cloned objects
	vector<GObject*> newObjects;

	for (int i = x0; i <= x1; ++i)
		for (int j = y0; j <= y1; ++j)
			for (int k = z0; k <= z1; ++k)
				if ((i != 0) || (j != 0) || (k != 0))
				{
					// clone the object
					GObject* pco = po->Clone();
					if (pco == 0) return newObjects;

					// set a new name
					char sz[256];
					sprintf(sz, "GridClone%02d", n++);
					pco->SetName(sz);

					// copy material assignments
					assert(pco->Parts() == po->Parts());
					if (pco->Parts() == po->Parts())
					{
						int NP = po->Parts();
						for (int i = 0; i<NP; ++i)
						{
							GPart* srcPart = po->Part(i);
							GPart* dstPart = pco->Part(i);

							dstPart->SetMaterialID(srcPart->GetMaterialID());
						}
					}

					// apply transform
					pco->GetTransform().Translate(vec3d(i*dx, j*dy, k*dz));

					newObjects.push_back(pco);
				}

	return newObjects;
}

//-----------------------------------------------------------------------------
vector<GObject*> GModel::CloneRevolve(GObject* po, int count, double range, double spiral, const vec3d& center, const vec3d& axis, bool rotateClones)
{
	// clone counter
	static int n = 1;

	vector<GObject*> newObjects;

	// make sure there is work to do
	if (count <= 0) return newObjects;

	// get the source object's position
	vec3d r = po->GetTransform().GetPosition();

	vec3d normAxis = axis.Normalized();

	// create clones
	for (int i = 0; i<count; ++i)
	{
		double w = DEG2RAD*((i + 1) * range / (count + 1));

		double d = (i + 1)*spiral / count;

		quatd Q = quatd(w, axis);

		// clone the object
		GObject* pco = po->Clone();
		if (pco == 0) return newObjects;

		// set a new name
		char sz[256];
		sprintf(sz, "RevolveClone%02d", n++);
		pco->SetName(sz);

		// calculate new positions
		if (rotateClones) pco->GetTransform().Rotate(Q, center);
		else
		{
			vec3d pos = center + Q*(r - center);
			pco->GetTransform().SetPosition(pos);
		}
		pco->GetTransform().Translate(normAxis * d);

		// add the object
		newObjects.push_back(pco);
	}

	// all done
	return newObjects;
}

//-----------------------------------------------------------------------------
GObject* GModel::MergeSelectedObjects(GObjectSelection* sel, const string& newObjectName, bool weld, double tol)
{
	// make sure we have more than one object
	if (sel->Count() <= 1) return nullptr;

	// check for some special cases first.
	// See if one of the objects is a "discrete" object, an object that has only nodes and springs are attached to them
	// This is currently the case if the object's type is GOBJECT
	for (int i = 0; i<sel->Count(); ++i)
	{
		GObject* poi = sel->Object(i);
		if (poi->GetType() == GOBJECT)
		{
			// okay, we assume this is the case, so merge this object with the rest.
			vector<GObject*> discList;
			vector<GObject*> objList;
			for (int j=0; j<sel->Count(); ++j)
			{
				GObject* poj = sel->Object(j);
				if (poj->GetType() == GOBJECT) discList.push_back(poj);
				else objList.push_back(poj);
			}
			GObject* newObj = MergeDiscreteObject(discList, objList, tol); 
			if (newObj) newObj->SetName(newObjectName);
			return newObj;
		}
	}

	// If all objects are surface meshes, we'll create a new surface mesh object
	bool allSurfaces = true;
	for (int i = 0; i<sel->Count(); ++i)
	{
		GSurfaceMeshObject* po = dynamic_cast<GSurfaceMeshObject*>(sel->Object(i));
		if (po == 0)
		{
			allSurfaces = false;
			break;
		}
		else
		{
			// make sure this object has a surface mesh
			if (po->GetEditableMesh() == 0)
			{
				return nullptr;
			}
		}
	}

	if (allSurfaces == false)
	{
		// see if all objects are multiblocks
		bool allMultiBlocks = true;
		for (int i = 0; i < sel->Count(); ++i)
		{
			GMultiBox* mb = dynamic_cast<GMultiBox*>(sel->Object(i));
			if (mb == nullptr) {
				allMultiBlocks = false; break;
			}
		}

		// merge all multiblocks
		if (allMultiBlocks)
		{
			// create a new object by copying the first selected object
			GMultiBox* poa = dynamic_cast<GMultiBox*>(sel->Object(0)); assert(poa);
			GMultiBox* ponew = dynamic_cast<GMultiBox*>(poa->Clone());
			ponew->SetName(newObjectName.c_str());

			for (int i = 1; i < sel->Count(); ++i)
			{
				// get the next object
				GMultiBox* po = dynamic_cast<GMultiBox*>(sel->Object(i));

				// attach it
				ponew->Merge(*po);
			}

			return ponew;
		}

		// see if all objects are multi-patch
		bool allMultiPatch = true;
		for (int i = 0; i < sel->Count(); ++i)
		{
			GMultiPatch* mb = dynamic_cast<GMultiPatch*>(sel->Object(i));
			if (mb == nullptr) {
				allMultiPatch = false; break;
			}
		}

		// merge all multiblocks
		if (allMultiPatch)
		{
			// create a new object by copying the first selected object
			GMultiPatch* poa = dynamic_cast<GMultiPatch*>(sel->Object(0)); assert(poa);
			GMultiPatch* ponew = dynamic_cast<GMultiPatch*>(poa->Clone());
			ponew->SetName(newObjectName.c_str());

			for (int i = 1; i < sel->Count(); ++i)
			{
				// get the next object
				GMultiPatch* po = dynamic_cast<GMultiPatch*>(sel->Object(i));

				// attach it
				ponew->Merge(*po);
			}

			return ponew;
		}


		// make sure all objects have meshes
		for (int i = 0; i<sel->Count(); ++i)
		{
			GObject* po = sel->Object(i);
			FSMesh* pm = po->GetFEMesh();
			if (pm == 0)
			{
				return nullptr;
			}
		}
	}

	// this will be the new object
	GObject* newObject = 0;

	if (allSurfaces)
	{
		// create a new object by copying the first selected object
		GSurfaceMeshObject* poa = dynamic_cast<GSurfaceMeshObject*>(sel->Object(0)); assert(poa);
		GSurfaceMeshObject* ponew = new GSurfaceMeshObject(poa); newObject = ponew;
		ponew->SetName(newObjectName.c_str());
		ponew->CopyTransform(poa);

		for (int i = 1; i<sel->Count(); ++i)
		{
			// get the next object
			GSurfaceMeshObject* po = dynamic_cast<GSurfaceMeshObject*>(sel->Object(i));

			// attach it
			ponew->Attach(po, weld, tol);
		}
	}
	else
	{
		// create a new object by copying the first selected object
		GObject* poa = sel->Object(0); assert(poa);
		GMeshObject* ponew = new GMeshObject(poa); newObject = ponew;
		ponew->SetName(newObjectName.c_str());
		ponew->CopyTransform(poa);

		for (int i = 1; i<sel->Count(); ++i)
		{
			// get the next object
			GObject* po = sel->Object(i);

			// attach it
			ponew->Attach(po, weld, tol);
		}
	}
	assert(newObject);

	// the new object must have the same materials as the old objects
	// it is assumed that the number of partitions and their order did not change.
	int np = 0;
	for (int i = 0; i <sel->Count(); ++i)
	{
		GObject* po = sel->Object(i);
		for (int j = 0; j<po->Parts(); ++j, ++np)
		{
			assert(np < newObject->Parts());
			newObject->Part(np)->SetMaterialID(po->Part(j)->GetMaterialID());
		}
	}

	// TODO: Figure out a way to retain the original smoothing groups during merge.
	newObject->GetEditableMesh()->SmoothByPartition();

	return newObject;
}

//-----------------------------------------------------------------------------
GObject* GModel::MergeDiscreteObject(vector<GObject*> discreteObjects, vector<GObject*>& objList, double tol)
{
	// sanity checks
	if (discreteObjects.size() == 0) return 0;
	if (objList.size() == 0) return 0;

	// we're going to assume that we all the objects in the objList are GMeshObjects
	// create a new GMeshObject
	GMeshObject* newObj = new GMeshObject(objList[0]);
	newObj->CopyTransform(objList[0]);
	for (int i = 1; i<objList.size(); ++i)
	{
		// get the next object
		GObject* po = objList[i];
		// attach it
		newObj->Attach(po, false, 0.0);
	}

	// we'll need the mesh of the new object
	FSMesh* pm = newObj->GetFEMesh();

	// now, insert the nodes of the "discrete" objects
	for (int n=0; n<discreteObjects.size(); ++n)
	{
		GObject* po = discreteObjects[n];
		int N = po->Nodes();
		for (int i=0; i<N; ++i)
		{
			po->Node(i)->m_ntag = -1;

			// get the nodal position in the local coordinates of the new object
			vec3d r = po->Node(i)->Position();
			r = newObj->GetTransform().GlobalToLocal(r);

			// find the closest FE node
			int closestNode = -1;
			double Lmin = 0.0;
			int NN = pm->Nodes();
			for (int j=0; j<NN; ++j)
			{
				vec3d rj = pm->Node(j).r;
				double L = (r - rj).SqrLength();
				if ((closestNode == -1) || (L < Lmin))
				{
					closestNode = j;
					Lmin = L;
				}
			}

			// make sure it falls within the tolerance
			if ((tol == 0.0) || (Lmin < tol))
			{
				FSNode& node = pm->Node(closestNode);
				int nodeID = node.m_gid;

				// partition this node
				if (nodeID == -1)
				{
					nodeID = newObj->MakeGNode(closestNode);
				}
				else nodeID = newObj->Node(nodeID)->GetID();

				// store the new ID
				po->Node(i)->m_ntag = nodeID;
			}
		}
	}

	// Okay, now we need to update the springs to reference the new nodes
	int ND = DiscreteObjects();
	for (int i=0; i<ND; ++i)
	{
		GDiscreteElementSet* set = dynamic_cast<GDiscreteElementSet*>(DiscreteObject(i));
		if (set)
		{
			int NE = set->size();
			for (int j=0; j<NE; ++j)
			{
				GDiscreteElement& de = set->element(j);
				int nd[2] = {0};
				nd[0] = de.Node(0);
				nd[1] = de.Node(1);

				GNode* node[2] = {0};
				for (int k=0; k<2; ++k)
				{
					for (int n=0; n<discreteObjects.size(); ++n)
					{
						GObject* po = discreteObjects[n];
						GNode* nk = po->FindNode(nd[k]);
						if (nk)
						{
							node[k] = nk;
							break;
						}						
					}
					if (node[k]) nd[k] = node[k]->m_ntag;
				}
				de.SetNodes(nd[0], nd[1]);
			}
		}
	}

	// all done!
	return newObj;
}

//-----------------------------------------------------------------------------
// detach the discrete set
GObject* GModel::DetachDiscreteSet(GDiscreteElementSet* set)
{
	// make sure we have something to do
	assert(set);
	if ((set == 0) || (set->size() == 0)) return 0;

	GNodeIterator it(*this);
	while (it.isValid()) { it->m_ntag = -1; ++it; }

	// Identify the nodes that need to be copied
	int NE = set->size();
	for (int i=0; i<NE; ++i)
	{
		GDiscreteElement& de = set->element(i);
		int n0 = de.Node(0);
		int n1 = de.Node(1);

		GNode* node0 = FindNode(n0); assert(node0); if (node0 == 0) return 0;
		GNode* node1 = FindNode(n1); assert(node1); if (node1 == 0) return 0;

		node0->m_ntag = 1;
		node1->m_ntag = 1;
	}

	// fill a list
	it.reset();
	vector<GNode*> nodes;
	while (it.isValid())
	{
		if (it->m_ntag == 1) nodes.push_back(it);
		++it;
	}

	// Create a new generic object
	GObject* po = new GObject(GOBJECT);

	// add all the nodes
	for (int i=0; i<(int)nodes.size(); ++i)
	{
		GNode* ni = nodes[i];
		GNode* n = po->AddNode(ni->Position(), NODE_VERTEX, true);
		ni->m_ntag = n->GetID();
	}
	po->Update();

	// attach the springs to this new object
	for (int i=0; i<NE; ++i)
	{
		GDiscreteElement& de = set->element(i);
		int n0 = de.Node(0);
		int n1 = de.Node(1);

		GNode* node0 = FindNode(n0); assert(node0); if (node0 == 0) return 0;
		GNode* node1 = FindNode(n1); assert(node1); if (node1 == 0) return 0;

		de.SetNodes(node0->m_ntag, node1->m_ntag);
	}

	return po;
}

list<GPart*> GModel::FindPartsFromMaterial(int matId, bool bmatch)
{
	list<GPart*> partList;
	for (int i = 0; i < Objects(); ++i)
	{
		GObject* po = Object(i);
		for (int j = 0; j < po->Parts(); ++j)
		{
			GPart* pg = po->Part(j);
			if (bmatch)
			{
				if (pg->GetMaterialID() == matId) partList.push_back(pg);
			}
			else
			{
				if (pg->GetMaterialID() != matId) partList.push_back(pg);
			}
		}
	}
	return partList;
}

int GModel::MeshLayers() const
{
	return imp->m_mlm->Layers();
}

int GModel::GetActiveMeshLayer() const
{
	return imp->m_mlm->GetActiveLayer();
}

void GModel::SetActiveMeshLayer(int n)
{
	imp->m_mlm->SetActiveLayer(n);
}

int GModel::FindMeshLayer(const std::string& s)
{
	return imp->m_mlm->FindMeshLayer(s);
}

const std::string& GModel::GetMeshLayerName(int i) const
{
	return imp->m_mlm->GetLayerName(i);
}

bool GModel::AddMeshLayer(const std::string& layerName)
{
	return imp->m_mlm->AddLayer(layerName);
}

ObjectMeshList* GModel::GetObjectMeshList(GObject* po)
{
	return imp->m_mlm->GetObjectMeshList(po);
}

void GModel::InsertObjectMeshList(ObjectMeshList* oml)
{
	imp->m_mlm->InsertObjectMeshList(oml);
}

void GModel::DeleteMeshLayer(int n)
{
	imp->m_mlm->DeleteLayer(n);
}

MeshLayer* GModel::RemoveMeshLayer(int index) 
{
	return imp->m_mlm->RemoveMeshLayer(index);
}

void GModel::InsertMeshLayer(int index, MeshLayer* layer)
{
	imp->m_mlm->InsertMeshLayer(index, layer);
}

MeshLayerManager* GModel::GetMeshLayerManager()
{
	return imp->m_mlm;
}

void GModel::SetLoadOnlyDiscreteFlag(bool flag)
{
    imp->m_loadOnlyDiscrete = flag;
}
